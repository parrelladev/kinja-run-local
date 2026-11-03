// Offline bootstrap adapter for the user's original Kinja Run 1.9.1 ARM64 client.
// No on-disk il2cpp code is modified. All managed calls run on the Unity thread.
#include <jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <algorithm>
#include <atomic>
#include <array>
#include <chrono>
#include <cctype>
#include <cerrno>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <ctime>
#include <fstream>
#include <iterator>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <utility>
#include <unordered_set>
#include <vector>
#include "fusion_batch_plan.h"
#include "account_progression.h"

#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "KinjaOffline", __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, "KinjaOffline", __VA_ARGS__)

namespace {
using Obj = void*;
using Klass = void*;
using Method = const void*;
using Type = const void*;
using Field = void*;
struct Api {
    void* (*domain_get)() = nullptr;
    const void** (*domain_get_assemblies)(void*, size_t*) = nullptr;
    const void* (*assembly_get_image)(const void*) = nullptr;
    const char* (*image_get_name)(const void*) = nullptr;
    Klass (*class_from_name)(const void*, const char*, const char*) = nullptr;
    Klass (*class_from_type)(Type) = nullptr;
    Klass (*class_get_parent)(Klass) = nullptr;
    const char* (*class_get_name)(Klass) = nullptr;
    const char* (*class_get_namespace)(Klass) = nullptr;
    Type (*class_get_type)(Klass) = nullptr;
    Method (*class_get_methods)(Klass, void**) = nullptr;
    const char* (*method_get_name)(Method) = nullptr;
    uint32_t (*method_get_param_count)(Method) = nullptr;
    Type (*method_get_param)(Method, uint32_t) = nullptr;
    Type (*method_get_return_type)(Method) = nullptr;
    char* (*type_get_name)(Type) = nullptr;
    Obj (*type_get_object)(Type) = nullptr;
    Klass (*object_get_class)(Obj) = nullptr;
    void* (*object_unbox)(Obj) = nullptr;
    int32_t (*string_length)(Obj) = nullptr;
    const uint16_t* (*string_chars)(Obj) = nullptr;
    Field (*class_get_field_from_name)(Klass, const char*) = nullptr;
    int32_t (*field_get_offset)(Field) = nullptr;
    void (*field_get_value)(Obj, Field, void*) = nullptr;
    Type (*field_get_type)(Field) = nullptr;
    void (*field_set_value)(Obj, Field, void*) = nullptr;
    Obj (*string_new)(const char*) = nullptr;
    Obj (*runtime_invoke)(Method, Obj, void**, Obj*) = nullptr;
    void* (*thread_attach)(void*) = nullptr;
    void (*thread_detach)(void*) = nullptr;
    uint32_t (*gchandle_new)(Obj, bool) = nullptr;
    Obj (*gchandle_get_target)(uint32_t) = nullptr;
    void (*gchandle_free)(uint32_t) = nullptr;
    void (*free)(void*) = nullptr;
    void (*format_exception)(Obj, char*, int) = nullptr;
    void (*format_stack_trace)(Obj, char*, int) = nullptr;
} api;

std::mutex config_mutex;
std::mutex status_mutex;
std::string files_dir;
AAssetManager* assets = nullptr;
jobject asset_owner = nullptr; // Global Java reference keeps AAssetManager alive.
std::atomic<bool> configured{false};
std::atomic<bool> hook_installed{false};
std::atomic<bool> initializing{false};
std::atomic<bool> combat_hooks_installed{false};
std::atomic<int> active_region{1};
std::atomic<int64_t> run_gold_payout{0};
std::atomic<int64_t> run_gold_credited{0};
std::atomic<int64_t> run_diamond_payout{0};
std::atomic<int64_t> run_diamond_credited{0};
std::atomic<bool> run_success_confirmed{false};
std::mutex run_item_mutex;
std::vector<std::pair<int, int>> pending_run_items;
std::unordered_set<int> seen_run_drop_instances;
int empty_book_rolls_this_run = 0;
uint32_t pending_end_response_handle = 0;
std::string last_error;
struct LocalTaskApi {
    Klass task_class = nullptr;
    Method create = nullptr;
    Method set_result = nullptr;
    Klass response_class = nullptr;
    Method json_method = nullptr;
};
LocalTaskApi enter_task;
LocalTaskApi kill_task;
LocalTaskApi end_task;
LocalTaskApi resurgence_task;
LocalTaskApi hangup_task;
LocalTaskApi reward_hangup_task;
LocalTaskApi talent_upgrade_task;
LocalTaskApi local_wear_equip_task;
LocalTaskApi local_equip_upgrade_task;
LocalTaskApi local_equip_onekey_upgrade_task;
LocalTaskApi local_equip_compound_task;
LocalTaskApi local_equip_onekey_compound_task;
std::atomic<bool> talent_upgrade_in_progress{false};
std::atomic<bool> wear_equip_in_progress{false};
std::atomic<bool> equip_upgrade_in_progress{false};
std::atomic<bool> equip_compound_in_progress{false};
std::atomic<bool> chest_purchase_in_progress{false};
std::atomic<bool> shop_coin_exchange_in_progress{false};
std::atomic<int64_t> last_chest_purchase_ms{0};
std::atomic<int32_t> last_chest_purchase_id{0};
struct UnityVector3 { float x, y, z; };
struct UnityVector4 { float x, y, z, w; };
struct SkillTriggerData { bool trigger_lightning, trigger_attack_skill; };
using DeductEnemyHpOriginal = bool (*)(Obj, int32_t, int64_t, UnityVector3, bool, int32_t,
                                      SkillTriggerData, Obj, int32_t, UnityVector3, int32_t,
                                      Obj, bool, bool, bool, bool, Method);
using DeductPlayerHpOriginal = void (*)(Obj, int32_t, bool, UnityVector3, bool, bool, int32_t, Method);
using TreasurePageUpdateDiamondOriginal = void (*)(Obj, Method);
using TreasurePageUpdateCoinOriginal = void (*)(Obj, Method);
using TreasurePageUpdateOriginal = void (*)(Obj, Method);
using TreasurePageUpdatePageOriginal = void (*)(Obj, Method);
using TreasurePageRefreshOriginal = void (*)(Obj, Method);
using BuyTreasureChestOriginal = void (*)(Obj, int32_t, bool, Obj, Method);
using TreasureBoxOnOpenByItemOriginal = void (*)(Obj, Obj, Method);
using MainPanelDoOpenOriginal = void (*)(Obj, Method);
using MainPanelOnClickFightOriginal = void (*)(Obj, Obj, Method);
DeductEnemyHpOriginal original_deduct_enemy_hp = nullptr;
DeductPlayerHpOriginal original_deduct_player_hp = nullptr;
TreasurePageUpdateDiamondOriginal original_treasure_page_update_diamond = nullptr;
TreasurePageUpdateCoinOriginal original_treasure_page_update_coin = nullptr;
TreasurePageUpdateOriginal original_treasure_page_update = nullptr;
TreasurePageUpdatePageOriginal original_treasure_page_update_page = nullptr;
TreasurePageRefreshOriginal original_treasure_page_refresh = nullptr;
BuyTreasureChestOriginal original_buy_treasure_chest = nullptr;
TreasureBoxOnOpenByItemOriginal original_treasure_box_on_open_by_item = nullptr;
using FusionSelectionOriginal = void (*)(Obj, Method);
FusionSelectionOriginal original_fusion_selection_update = nullptr;
thread_local int32_t chest_payment_context = 0; // 0=unknown, 3=item, 4=currency (1.9.1 enum)
MainPanelDoOpenOriginal original_main_panel_do_open = nullptr;
MainPanelOnClickFightOriginal original_main_panel_on_click_fight = nullptr;
std::atomic<unsigned> outgoing_damage_logs{0};
std::atomic<unsigned> incoming_damage_logs{0};
constexpr uintptr_t kAccountInitVa = 0x27972e0;
constexpr std::array<uint8_t, 16> kExpectedEntry = {
    0xf4,0x4f,0xbe,0xa9, 0xfd,0x7b,0x01,0xa9,
    0xfd,0x43,0x00,0x91, 0x14,0x18,0x01,0xb0
};

std::string escape_json(const std::string& input) {
    std::string result;
    const char* digits = "0123456789abcdef";
    for (unsigned char c : input) {
        if (c == '"' || c == '\\') { result += '\\'; result += c; }
        else if (c < 32) { result += "\\u00"; result += digits[c >> 4]; result += digits[c & 15]; }
        else result += c;
    }
    return result;
}

bool write_file(const std::string& path, const std::string& data) {
    const std::string temp = path + ".tmp";
    std::ofstream stream(temp, std::ios::binary | std::ios::trunc);
    if (!stream) return false;
    stream.write(data.data(), static_cast<std::streamsize>(data.size()));
    stream.close();
    if (!stream) { unlink(temp.c_str()); return false; }
    if (rename(temp.c_str(), path.c_str()) != 0) { unlink(temp.c_str()); return false; }
    return true;
}

void status(const char* stage, const std::string& detail) {
    std::lock_guard<std::mutex> lock(status_mutex);
    std::string body = "{\n  \"stage\":\"" + escape_json(stage) +
        "\",\n  \"detail\":\"" + escape_json(detail) +
        "\",\n  \"hook_installed\":" + (hook_installed.load() ? "true" : "false") +
        ",\n  \"unix_seconds\":" + std::to_string(std::time(nullptr)) + "\n}\n";
    if (!files_dir.empty()) write_file(files_dir + "/offline_bootstrap_status.json", body);
    LOGI("stage=%s %s", stage, detail.c_str());
}

bool fail(const std::string& detail) {
    last_error = detail;
    LOGE("%s", detail.c_str());
    return false;
}

template <typename T> bool symbol(void* library, const char* name, T& output) {
    output = reinterpret_cast<T>(dlsym(library, name));
    if (!output) { LOGE("Missing export: %s", name); return false; }
    return true;
}

bool load_api(void* library) {
    bool ok = true;
#define BIND(member) ok = symbol(library, "il2cpp_" #member, api.member) && ok
    BIND(domain_get); BIND(domain_get_assemblies); BIND(assembly_get_image); BIND(image_get_name);
    BIND(class_from_name); BIND(class_from_type); BIND(class_get_parent); BIND(class_get_name); BIND(class_get_namespace);
    BIND(class_get_type); BIND(class_get_methods); BIND(method_get_name); BIND(method_get_param_count);
    BIND(method_get_param); BIND(method_get_return_type); BIND(type_get_name); BIND(type_get_object);
    BIND(object_get_class); BIND(object_unbox); BIND(string_length); BIND(string_chars);
    BIND(class_get_field_from_name); BIND(field_get_type);
    BIND(field_get_offset); BIND(field_get_value);
    BIND(field_set_value); BIND(string_new); BIND(runtime_invoke); BIND(thread_attach); BIND(thread_detach);
    BIND(gchandle_new); BIND(gchandle_get_target); BIND(gchandle_free); BIND(free);
#undef BIND
    // These diagnostic exports are optional; managed exception class names still get logged.
    api.format_exception = reinterpret_cast<decltype(api.format_exception)>(dlsym(library, "il2cpp_format_exception"));
    api.format_stack_trace = reinterpret_cast<decltype(api.format_stack_trace)>(dlsym(library, "il2cpp_format_stack_trace"));
    return ok;
}

std::string type_name(Type type) {
    if (!type) return "<null-type>";
    char* text = api.type_get_name(type);
    std::string result = text ? text : "<unnamed-type>";
    if (text) api.free(text);
    return result;
}

std::string class_name(Klass klass) {
    const char* ns = klass ? api.class_get_namespace(klass) : nullptr;
    const char* name = klass ? api.class_get_name(klass) : nullptr;
    return std::string(ns && *ns ? ns : "") + (ns && *ns ? "." : "") + (name ? name : "<null-class>");
}

Klass find_class(const char* ns, const char* name) {
    void* domain = api.domain_get();
    if (!domain) { fail("IL2CPP domain is not initialized"); return nullptr; }
    size_t count = 0;
    const void** list = api.domain_get_assemblies(domain, &count);
    Klass found = nullptr;
    for (size_t i = 0; list && i < count; ++i) {
        const void* image = api.assembly_get_image(list[i]);
        Klass candidate = api.class_from_name(image, ns, name);
        if (!candidate) continue;
        if (found && found != candidate) {
            fail("Ambiguous class " + std::string(ns) + "." + name);
            return nullptr;
        }
        found = candidate;
        LOGI("Class %s.%s in %s", ns, name, api.image_get_name(image));
    }
    if (!found) fail("Missing class " + std::string(ns) + "." + name);
    return found;
}

Method find_method(Klass klass, const char* name, const std::vector<std::string>& parameters,
                   bool search_parents = false) {
    for (Klass current = klass; current; current = search_parents ? api.class_get_parent(current) : nullptr) {
        void* iterator = nullptr;
        Method found = nullptr;
        while (Method method = api.class_get_methods(current, &iterator)) {
            if (std::strcmp(api.method_get_name(method), name) != 0 ||
                api.method_get_param_count(method) != parameters.size()) continue;
            bool matches = true;
            for (size_t n = 0; n < parameters.size(); ++n) {
                if (type_name(api.method_get_param(method, static_cast<uint32_t>(n))) != parameters[n]) {
                    matches = false; break;
                }
            }
            if (!matches) continue;
            if (found && found != method) {
                fail("Ambiguous exact signature " + class_name(current) + "." + name);
                return nullptr;
            }
            found = method;
        }
        if (found) return found;
    }
    std::string signature = class_name(klass) + "." + name + "(";
    for (const auto& p : parameters) signature += p + ",";
    fail("Missing exact signature " + signature + ")");
    return nullptr;
}

void log_exception(Obj exception, const std::string& context) {
    std::array<char, 4096> text{};
    std::array<char, 8192> trace{};
    if (api.format_exception) api.format_exception(exception, text.data(), static_cast<int>(text.size()));
    if (api.format_stack_trace) api.format_stack_trace(exception, trace.data(), static_cast<int>(trace.size()));
    last_error = context + ": " + class_name(api.object_get_class(exception)) + " " + text.data();
    LOGE("Managed exception %s\n%s", last_error.c_str(), trace.data());
    // runtime_invoke captures managed exceptions before Unity can print them. Send
    // the same object through Unity's own logger to recover its managed call stack.
    Klass debug = find_class("UnityEngine", "Debug");
    Method log_method = debug ? find_method(debug, "LogException", {"System.Exception"}) : nullptr;
    if (log_method) {
        void* args[] = {exception};
        Obj nested = nullptr;
        api.runtime_invoke(log_method, nullptr, args, &nested);
        if (nested) LOGE("Unity Debug.LogException also threw %s", class_name(api.object_get_class(nested)).c_str());
    }
}

bool invoke(Method method, Obj instance, void** args, const std::string& context, Obj* output = nullptr) {
    if (!method) return false;
    Obj exception = nullptr;
    Obj result = api.runtime_invoke(method, instance, args, &exception);
    if (exception) { log_exception(exception, context); return false; }
    if (output) *output = result;
    LOGI("Invoke OK: %s", context.c_str());
    return true;
}

struct Roots {
    std::vector<uint32_t> handles;
    ~Roots() { for (auto handle : handles) api.gchandle_free(handle); }
    Obj retain(Obj obj) { if (obj) handles.push_back(api.gchandle_new(obj, false)); return obj; }
};

bool set_local_opening_functions(Method json_method, Roots& roots);

Obj singleton(Klass klass, Roots& roots) {
    Method getter = find_method(klass, "get_Instance", {}, true);
    Obj obj = nullptr;
    if (!getter || !invoke(getter, nullptr, nullptr, class_name(klass) + ".get_Instance", &obj)) return nullptr;
    if (!obj) { fail("Null singleton " + class_name(klass)); return nullptr; }
    return roots.retain(obj);
}

std::string read_asset(const std::string& path) {
    AAsset* asset = AAssetManager_open(assets, path.c_str(), AASSET_MODE_STREAMING);
    if (!asset) { fail("Missing packaged asset " + path); return {}; }
    const auto length = AAsset_getLength(asset);
    if (length <= 0 || length > 128 * 1024) { AAsset_close(asset); fail("Invalid seed length " + path); return {}; }
    std::string data(static_cast<size_t>(length), '\0');
    size_t read = 0;
    while (read < data.size()) {
        int n = AAsset_read(asset, data.data() + read, data.size() - read);
        if (n <= 0) break;
        read += static_cast<size_t>(n);
    }
    AAsset_close(asset);
    if (read != data.size()) { fail("Truncated packaged asset " + path); return {}; }
    return data;
}

std::string load_seed(const char* filename) {
    const std::string directory = files_dir + "/offline_save";
    if (mkdir(directory.c_str(), 0700) != 0 && errno != EEXIST) { fail("Cannot create offline_save directory"); return {}; }
    const std::string saved_path = directory + "/" + filename;
    std::ifstream saved(saved_path, std::ios::binary | std::ios::ate);
    if (saved) {
        auto length = saved.tellg();
        if (length <= 0 || length > 128 * 1024) { fail("Invalid saved seed length " + saved_path); return {}; }
        saved.seekg(0);
        std::string text(static_cast<size_t>(length), '\0');
        saved.read(text.data(), length);
        if (!saved) { fail("Cannot read saved seed " + saved_path); return {}; }
        LOGI("Loaded existing local seed %s", filename);
        return text;
    }
    std::string text = read_asset(std::string("offline/") + filename);
    if (text.empty()) return {};
    if (!write_file(saved_path, text)) { fail("Cannot persist initial seed " + saved_path); return {}; }
    LOGI("Persisted initial local seed %s", filename);
    return text;
}

Obj deserialize(Method method, Klass klass, const char* filename, Roots& roots) {
    std::string json = load_seed(filename);
    if (json.empty()) return nullptr;
    Obj text = roots.retain(api.string_new(json.c_str()));
    Obj reflection_type = roots.retain(api.type_get_object(api.class_get_type(klass)));
    if (!text || !reflection_type) { fail("Cannot create DeserializeObject arguments"); return nullptr; }
    void* args[] = {text, reflection_type};
    Obj result = nullptr;
    if (!invoke(method, nullptr, args, "JsonConvert.DeserializeObject " + class_name(klass), &result)) return nullptr;
    if (!result || api.object_get_class(result) != klass) {
        fail("DeserializeObject returned wrong/null class for " + class_name(klass)); return nullptr;
    }
    return roots.retain(result);
}

Obj deserialize_local_payload(Method method, Klass klass, const std::string& json, Roots& roots) {
    if (!method || !klass || json.empty()) { fail("Invalid local JSON response payload"); return nullptr; }
    Obj text = roots.retain(api.string_new(json.c_str()));
    Obj reflection_type = roots.retain(api.type_get_object(api.class_get_type(klass)));
    if (!text || !reflection_type) { fail("Cannot create local response DeserializeObject arguments"); return nullptr; }
    void* args[] = {text, reflection_type};
    Obj result = nullptr;
    if (!invoke(method, nullptr, args, "JsonConvert.DeserializeObject local HangUp response", &result)) return nullptr;
    if (!result || api.object_get_class(result) != klass) {
        fail("DeserializeObject returned wrong/null class for local HangUp response");
        return nullptr;
    }
    return roots.retain(result);
}

size_t json_member_value(const std::string& json, const std::string& key, size_t from = 0) {
    const std::string needle = "\"" + key + "\"";
    size_t pos = json.find(needle, from);
    if (pos == std::string::npos) return pos;
    pos = json.find(':', pos + needle.size());
    if (pos == std::string::npos) return pos;
    do { ++pos; } while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\t' || json[pos] == '\r' || json[pos] == '\n'));
    return pos;
}

bool json_read_int(const std::string& json, const std::string& key, int64_t& value, size_t from = 0) {
    size_t pos = json_member_value(json, key, from);
    if (pos == std::string::npos) return false;
    char* end = nullptr;
    long long parsed = std::strtoll(json.c_str() + pos, &end, 10);
    if (end == json.c_str() + pos) return false;
    value = static_cast<int64_t>(parsed);
    return true;
}

bool json_read_bool(const std::string& json, const std::string& key, bool& value, size_t from = 0) {
    size_t pos = json_member_value(json, key, from);
    if (pos == std::string::npos) return false;
    if (json.compare(pos, 4, "true") == 0) { value = true; return true; }
    if (json.compare(pos, 5, "false") == 0) { value = false; return true; }
    return false;
}

bool json_set_scalar(std::string& json, const std::string& key, const std::string& value, size_t from = 0) {
    size_t pos = json_member_value(json, key, from);
    if (pos == std::string::npos) return false;
    size_t end = pos;
    if (end < json.size() && json[end] == '"') {
        ++end;
        while (end < json.size()) {
            if (json[end++] == '"' && json[end - 2] != '\\') break;
        }
    } else {
        while (end < json.size() && json[end] != ',' && json[end] != '}' && json[end] != ']' &&
               json[end] != ' ' && json[end] != '\r' && json[end] != '\n' && json[end] != '\t') ++end;
    }
    if (end <= pos) return false;
    json.replace(pos, end - pos, value);
    return true;
}

bool json_set_array(std::string& json, const std::string& key, const std::string& value) {
    size_t start = json_member_value(json, key);
    if (start == std::string::npos || start >= json.size() || json[start] != '[' || value.empty() || value[0] != '[')
        return false;
    int depth = 0;
    bool quoted = false, escaped = false;
    size_t end = start;
    for (; end < json.size(); ++end) {
        char c = json[end];
        if (quoted) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') quoted = false;
            continue;
        }
        if (c == '"') quoted = true;
        else if (c == '[') ++depth;
        else if (c == ']' && --depth == 0) { ++end; break; }
    }
    if (depth != 0 || end <= start) return false;
    json.replace(start, end - start, value);
    return true;
}

bool json_set_int(std::string& json, const std::string& key, int64_t value, size_t from = 0) {
    return json_set_scalar(json, key, std::to_string(value), from);
}

bool json_set_bool(std::string& json, const std::string& key, bool value, size_t from = 0) {
    return json_set_scalar(json, key, value ? "true" : "false", from);
}

size_t json_object_end(const std::string& json, size_t start) {
    if (start >= json.size() || json[start] != '{') return std::string::npos;
    int depth = 0;
    bool quoted = false, escaped = false;
    for (size_t i = start; i < json.size(); ++i) {
        char c = json[i];
        if (quoted) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') quoted = false;
            continue;
        }
        if (c == '"') quoted = true;
        else if (c == '{') ++depth;
        else if (c == '}' && --depth == 0) return i + 1;
    }
    return std::string::npos;
}

bool json_region(std::string& json, int region_id, int64_t score, bool passed,
                 bool* was_passed, int64_t* old_score) {
    size_t scan = 0;
    while ((scan = json.find("\"RegionId\"", scan)) != std::string::npos) {
        size_t start = json.rfind('{', scan);
        if (start == std::string::npos) return false;
        size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        int64_t id = -1;
        if (json_read_int(json.substr(start, end - start), "RegionId", id) && id == region_id) {
            std::string object = json.substr(start, end - start);
            bool old_pass = false;
            int64_t best = 0;
            if (!json_read_bool(object, "IsPass", old_pass) || !json_read_int(object, "MaxScore", best)) return false;
            if (was_passed) *was_passed = old_pass;
            if (old_score) *old_score = best;
            if (passed) json_set_bool(object, "IsPass", true);
            if (score > best) json_set_int(object, "MaxScore", score);
            json.replace(start, end - start, object);
            return true;
        }
        scan = end;
    }
    return false;
}

bool json_ensure_region_record(std::string& json, int region_id) {
    size_t scan = 0;
    while ((scan = json.find("\"RegionId\"", scan)) != std::string::npos) {
        size_t start = json.rfind('{', scan);
        if (start == std::string::npos) return false;
        size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        int64_t id = -1;
        if (json_read_int(json.substr(start, end - start), "RegionId", id) && id == region_id) return true;
        scan = end;
    }

    size_t array = json_member_value(json, "RegionsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t end = array + 1;
    int object_depth = 0;
    bool quoted = false, escaped = false;
    for (; end < json.size(); ++end) {
        char c = json[end];
        if (quoted) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') quoted = false;
            continue;
        }
        if (c == '"') quoted = true;
        else if (c == '{') ++object_depth;
        else if (c == '}') --object_depth;
        else if (c == ']' && object_depth == 0) break;
    }
    if (end >= json.size()) return false;
    const bool empty = json.find_first_not_of(" \t\r\n", array + 1) == end;
    const std::string record = "{\"RegionId\":" + std::to_string(region_id) +
        ",\"IsPass\":false,\"MaxScore\":0,\"ConsecutiveLostCount\":0,\"DifficultPass\":[false,false]}";
    json.insert(end, std::string(empty ? "" : ",") + record);
    return true;
}

bool unlock_next_region(std::string& json, int region_id, bool& newly_unlocked) {
    int64_t max_region = 0;
    if (!json_read_int(json, "MaxRegion", max_region)) return false;
    const int next_region = region_id + 1;
    newly_unlocked = max_region < next_region;
    if (newly_unlocked &&
        (!json_set_int(json, "MaxRegion", next_region) ||
         !json_set_int(json, "SelectRegion", next_region))) return false;
    return json_ensure_region_record(json, next_region);
}

bool json_add_item(std::string& json, const char* array_name, int item_id, int count,
                   const char* id_key, const char* count_key) {
    size_t array = json_member_value(json, array_name);
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t end = array + 1;
    int object_depth = 0;
    bool quoted = false, escaped = false;
    for (; end < json.size(); ++end) {
        char c = json[end];
        if (quoted) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') quoted = false;
            continue;
        }
        if (c == '"') quoted = true;
        else if (c == '{') ++object_depth;
        else if (c == '}') --object_depth;
        else if (c == ']' && object_depth == 0) break;
    }
    if (end >= json.size()) return false;
    size_t scan = array + 1;
    while ((scan = json.find(std::string("\"") + id_key + "\"", scan)) != std::string::npos && scan < end) {
        size_t object_start = json.rfind('{', scan);
        if (object_start == std::string::npos || object_start <= array) break;
        size_t object_end = json_object_end(json, object_start);
        if (object_end == std::string::npos || object_end > end) break;
        int64_t existing_id = -1, existing_count = 0;
        if (json_read_int(json.substr(object_start, object_end - object_start), id_key, existing_id) && existing_id == item_id) {
            std::string object = json.substr(object_start, object_end - object_start);
            if (!json_read_int(object, count_key, existing_count) || !json_set_int(object, count_key, existing_count + count)) return false;
            json.replace(object_start, object_end - object_start, object);
            return true;
        }
        scan = object_end;
    }
    const bool empty = json.find_first_not_of(" \t\r\n", array + 1) == end;
    std::string item = "{\"" + std::string(id_key) + "\":" + std::to_string(item_id) +
        ",\"" + count_key + "\":" + std::to_string(count) + "}";
    json.insert(end, std::string(empty ? "" : ",") + item);
    return true;
}

bool json_get_item_count(const std::string& json, int item_id, int64_t& result) {
    size_t array = json_member_value(json, "ItemsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t scan = array + 1;
    while ((scan = json.find("\"ItemId\"", scan)) != std::string::npos) {
        size_t start = json.rfind('{', scan);
        if (start == std::string::npos || start < array) return false;
        size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        const std::string object = json.substr(start, end - start);
        int64_t id = -1;
        if (!json_read_int(object, "ItemId", id)) return false;
        if (id == item_id) return json_read_int(object, "Count", result);
        scan = end;
    }
    result = 0;
    return true;
}

#ifdef KINJA_OFFLINE_QA_MAX_PROFILE
bool json_raise_item_count(std::string& json, int item_id, int target) {
    size_t array = json_member_value(json, "ItemsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t scan = array + 1;
    while ((scan = json.find("\"ItemId\"", scan)) != std::string::npos) {
        size_t start = json.rfind('{', scan);
        if (start == std::string::npos || start < array) return false;
        size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        std::string object = json.substr(start, end - start);
        int64_t id = -1, count = 0;
        if (!json_read_int(object, "ItemId", id) || !json_read_int(object, "Count", count)) return false;
        if (id == item_id) {
            if (count >= target) return true;
            if (!json_set_int(object, "Count", target)) return false;
            json.replace(start, end - start, object);
            return true;
        }
        scan = end;
    }
    return json_add_item(json, "ItemsInfo", item_id, target, "ItemId", "Count");
}

bool apply_qa_max_profile() {
    constexpr int32_t kQaBalance = 1000000;
    const std::string marker_path = files_dir + "/offline_save/qa_max_profile_applied.json";
    std::ifstream marker(marker_path, std::ios::binary);
    if (marker) {
        LOGI("QA max profile already prepared; preserving subsequent gameplay spends");
        return true;
    }
    std::string player = load_seed("seed_player.json");
    std::string items = load_seed("seed_items.json");
    if (player.empty() || items.empty()) return false;
    bool changed = false;
    for (const char* key : {"Coin", "Diamond", "Exp"}) {
        int64_t current = 0;
        if (!json_read_int(player, key, current) || current < 0) return false;
        if (current < kQaBalance) {
            if (!json_set_int(player, key, kQaBalance)) return false;
            changed = true;
        }
    }
    int64_t player_level = 0;
    if (!json_read_int(player, "Level", player_level)) return false;
    if (player_level < 100 && !json_set_int(player, "Level", 100)) return false;
    if (player_level < 100) changed = true;
    // Every bag item from ItemCfg is stocked to a practical QA quantity.
    // One million exercises all local purchase paths while leaving int32 headroom.
    constexpr int kBagItemIds[] = {
        1001,1002,1004,3001,3002,4001,5001,6001,7001,10001,10002,10003,
        20001,20002,20003,20004,20005,20006,40001,701001,704002,704003,
        8010101,8010102,8010103,8020101,8020102,8020103,8020104,8020105,
        8020106,8020107,8030101,8030102,8030103,8030104,8030105,8030106,
        8030107,9010101,9010102,9010103,9020101,9020102,9020103,9020104,
        9020105,9020106,9020107,9030101,9030102,9030103,9030104,9030105,
        9030106,9030107
    };
    for (int id : kBagItemIds) {
        if (!json_raise_item_count(items, id, kQaBalance)) return false;
    }
    if (changed && !write_file(files_dir + "/offline_save/seed_player.json", player)) return false;
    if (!write_file(files_dir + "/offline_save/seed_items.json", items)) return false;
    if (!write_file(marker_path, "{\"Profile\":\"qa-max-v1\",\"Balance\":1000000}\n")) return false;
    LOGI("QA max profile applied idempotently: currencies and %zu bag item types >= %d",
         sizeof(kBagItemIds) / sizeof(kBagItemIds[0]), kQaBalance);
    return true;
}
#endif

struct EquipReward { int id; int level; int quality; int uid = 0; int part = 0; };

bool json_has_item(const std::string& json, const char* array_name, int item_id,
                   const char* id_key) {
    size_t array = json_member_value(json, array_name);
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t scan = array + 1;
    while ((scan = json.find(std::string("\"") + id_key + "\"", scan)) != std::string::npos) {
        size_t start = json.rfind('{', scan);
        if (start == std::string::npos || start < array) return false;
        size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        int64_t id = -1;
        if (json_read_int(json.substr(start, end - start), id_key, id) && id == item_id) return true;
        scan = end;
    }
    return false;
}

bool json_has_equipment(const std::string& json, const EquipReward& equip) {
    size_t array = json_member_value(json, "EquipsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t scan = array + 1;
    while ((scan = json.find("\"EquipId\"", scan)) != std::string::npos) {
        size_t start = json.rfind('{', scan);
        if (start == std::string::npos || start < array) return false;
        size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        std::string object = json.substr(start, end - start);
        int64_t id = -1, level = -1, quality = -1;
        if (json_read_int(object, "EquipId", id) &&
            (json_read_int(object, "Level", level) || json_read_int(object, "EquipLevel", level)) &&
            (json_read_int(object, "Quality", quality) || json_read_int(object, "EquipQuality", quality)) && id == equip.id &&
            level == equip.level && quality == equip.quality) return true;
        scan = end;
    }
    return false;
}

bool json_add_equipment(std::string& json, const EquipReward& equip) {
    size_t array = json_member_value(json, "EquipsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    if (equip.uid <= 0 && json_has_equipment(json, equip)) return true;
    size_t end = array + 1;
    int object_depth = 0;
    bool quoted = false, escaped = false;
    for (; end < json.size(); ++end) {
        char c = json[end];
        if (quoted) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') quoted = false;
            continue;
        }
        if (c == '"') quoted = true;
        else if (c == '{') ++object_depth;
        else if (c == '}') --object_depth;
        else if (c == ']' && object_depth == 0) break;
    }
    if (end >= json.size()) return false;
    const bool empty = json.find_first_not_of(" \t\r\n", array + 1) == end;
    std::string record = "{\"EquipId\":" + std::to_string(equip.id) +
        ",\"Uid\":" + std::to_string(equip.uid) +
        ",\"Level\":" + std::to_string(equip.level) +
        ",\"Quality\":" + std::to_string(equip.quality) +
        ",\"Part\":" + std::to_string(equip.part) +
        ",\"IsWear\":false}";
    json.insert(end, std::string(empty ? "" : ",") + record);
    return true;
}

bool json_remove_equipment(std::string& json, const EquipReward& equip, bool& removed) {
    removed = false;
    size_t array = json_member_value(json, "EquipsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t scan = array + 1;
    while ((scan = json.find("\"EquipId\"", scan)) != std::string::npos) {
        size_t start = json.rfind('{', scan);
        if (start == std::string::npos || start < array) return false;
        size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        std::string object = json.substr(start, end - start);
        int64_t id = -1, level = -1, quality = -1;
        if (json_read_int(object, "EquipId", id) && json_read_int(object, "EquipLevel", level) &&
            json_read_int(object, "EquipQuality", quality) && id == equip.id &&
            level == equip.level && quality == equip.quality) {
            size_t erase_start = start, erase_end = end;
            size_t after = erase_end;
            while (after < json.size() && (json[after] == ' ' || json[after] == '\t' || json[after] == '\r' || json[after] == '\n')) ++after;
            if (after < json.size() && json[after] == ',') erase_end = after + 1;
            else {
                size_t before = erase_start;
                while (before > array + 1 && (json[before - 1] == ' ' || json[before - 1] == '\t' || json[before - 1] == '\r' || json[before - 1] == '\n')) --before;
                if (before > array + 1 && json[before - 1] == ',') erase_start = before - 1;
            }
            json.erase(erase_start, erase_end - erase_start);
            removed = true;
            return true;
        }
        scan = end;
    }
    return true;
}

bool fight_won(int stage, Roots& roots, bool& passed) {
    Klass data_class = find_class("", "FightDataManager");
    if (!data_class) return false;
    Obj manager = singleton(data_class, roots);
    Method get_current = find_method(data_class, "get_CurrentRegion", {});
    Method get_count = find_method(data_class, "get_RegionCount", {});
    if (!manager || !get_current || !get_count) return false;
    Obj current_box = nullptr, count_box = nullptr;
    if (!invoke(get_current, manager, nullptr, "FightDataManager.get_CurrentRegion at finish", &current_box) || !current_box)
        return false;
    roots.retain(current_box);
    if (!invoke(get_count, manager, nullptr, "FightDataManager.get_RegionCount at finish", &count_box) ||
        !current_box || !count_box || !api.object_unbox(current_box) || !api.object_unbox(count_box)) return false;
    roots.retain(count_box);
    const int current_region = *static_cast<int*>(api.object_unbox(current_box));
    const int region_count = *static_cast<int*>(api.object_unbox(count_box));
    if (current_region != stage || region_count <= 0) return fail("EndGame stage does not match FightDataManager region counters");
    // The original client advances CurrentRegion one past RegionCount only
    // after the final segment; FightClear itself is filled from the response.
    passed = current_region > region_count;
    LOGI("Local completion inference currentRegion=%d regionCount=%d passed=%d", current_region, region_count, passed);
    return true;
}

bool add_local_reward(Obj item_manager, int item_id, int count, const char* reward_id, Roots& roots,
                      const char* way = "offline_fight") {
    Klass manager_class = api.object_get_class(item_manager);
    Method add = find_method(manager_class, "AddLocalItem", {"System.Int32", "System.Int32", "System.String", "System.String"});
    if (!add) return false;
    Obj way_text = roots.retain(api.string_new(way));
    Obj reward = roots.retain(api.string_new(reward_id));
    if (!way_text || !reward) return fail("Cannot allocate AddLocalItem string arguments");
    void* args[] = {&item_id, &count, way_text, reward};
    return invoke(add, item_manager, args, "ItemManager.AddLocalItem offline fight reward");
}

Obj make_end_response(int region, bool passed, bool new_score, bool first_win, bool new_chapter,
                      int gold_reward, int diamond_reward,
                      const std::vector<std::pair<int, int>>& rewards,
                      const std::vector<EquipReward>& equipment_rewards, Roots& roots) {
    std::string json = load_seed("seed_end_game.json");
    if (json.empty() || !json_set_int(json, "RegionId", region) ||
        !json_set_int(json, "Gold", gold_reward) || !json_set_int(json, "Diamond", diamond_reward) ||
        !json_set_bool(json, "IsPass", passed) || !json_set_bool(json, "IsNewScore", new_score) ||
        !json_set_bool(json, "IsNewStage", first_win) || !json_set_bool(json, "IsNewChapter", new_chapter)) {
        fail("Cannot prepare local end-game response JSON"); return nullptr;
    }
    for (const auto& reward : rewards) {
        if (!json_add_item(json, "Items", reward.first, reward.second, "ItemId", "ItemCount")) {
            fail("Cannot add configured reward to local end-game response"); return nullptr;
        }
    }
    if (!equipment_rewards.empty()) {
        std::string ids = "[";
        for (size_t i = 0; i < equipment_rewards.size(); ++i) {
            if (i) ids += ",";
            ids += std::to_string(equipment_rewards[i].id);
        }
        ids += "]";
        if (!json_set_array(json, "EquipsUid", ids)) {
            fail("Cannot add equipment IDs to local end-game response"); return nullptr;
        }
    }
    Obj text = roots.retain(api.string_new(json.c_str()));
    Obj reflection_type = roots.retain(api.type_get_object(api.class_get_type(end_task.response_class)));
    if (!text || !reflection_type) { fail("Cannot create end-game response deserialization arguments"); return nullptr; }
    void* args[] = {text, reflection_type};
    Obj response = nullptr;
    if (!invoke(end_task.json_method, nullptr, args, "JsonConvert.DeserializeObject local EndGame", &response) ||
        !response || api.object_get_class(response) != end_task.response_class) {
        fail("EndGame JSON returned the wrong or null response class"); return nullptr;
    }
    return roots.retain(response);
}

bool persist_local_clear(int region, int64_t score, bool passed, bool& first_win, bool& new_score,
                         bool& milestone, bool& new_chapter,
                         std::vector<std::pair<int, int>>& rewards,
                         std::vector<EquipReward>& equipment_rewards) {
    std::string regions = load_seed("seed_regions.json");
    std::string items = load_seed("seed_items.json");
    if (regions.empty() || items.empty()) return false;
    bool was_passed = false;
    int64_t old_score = 0, old_progress = 0;
    if (!json_read_int(regions, "MaxRegionProgress", old_progress) ||
        !json_region(regions, region, score, passed, &was_passed, &old_score)) {
        return fail("Local region save does not contain the active region");
    }
    first_win = passed && !was_passed;
    new_score = score > old_score;
    if (passed && !unlock_next_region(regions, region, new_chapter))
        return fail("Cannot unlock the next local region");
    // Local region 2 is displayed Chapter 1; local region 3 is Chapter 2.
    // The shipped GameRegionCfg defines region 3's silver chest and 100% gear.
    milestone = passed && region == 2 && old_progress < 100;
    if (milestone && !json_set_int(regions, "MaxRegionProgress", 100)) return fail("Cannot save local region progress");
    if (first_win && region == 2) rewards.emplace_back(100001, 1);
    if (first_win && region == 3) rewards.emplace_back(100002, 1);
    if (milestone) rewards.emplace_back(40001, 2);
    for (const auto& reward : rewards) {
        if (!json_add_item(items, "ItemsInfo", reward.first, reward.second, "ItemId", "Count"))
            return fail("Cannot update saved offline inventory");
        LOGI("Local reward persisted ItemId=%d Count=%d", reward.first, reward.second);
    }
    if (!write_file(files_dir + "/offline_save/seed_regions.json", regions) ||
        !write_file(files_dir + "/offline_save/seed_items.json", items))
        return fail("Cannot persist local region and inventory rewards");
    return true;
}

bool apply_local_reward_items(const std::vector<std::pair<int, int>>& rewards, Roots& roots) {
    if (rewards.empty()) return true;
    Klass item_class = find_class("", "ItemManager");
    Obj manager = item_class ? singleton(item_class, roots) : nullptr;
    if (!manager) return false;
    for (const auto& reward : rewards) {
        const std::string reward_id = reward.first == 40001 ? "10013" : std::to_string(reward.first);
        if (!add_local_reward(manager, reward.first, reward.second, reward_id.c_str(), roots)) return false;
    }
    return true;
}

bool refresh_local_regions(Roots& roots) {
    Klass region_class = find_class("ET", "G2C_RegionData");
    Klass scene_class = find_class("", "SceneDataManager");
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Method callback = scene_class ? find_method(scene_class, "OnGetSceneProgress", {"ET.G2C_RegionData"}) : nullptr;
    Method deserialize_method = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    Obj manager = scene_class ? singleton(scene_class, roots) : nullptr;
    std::string json = load_seed("seed_regions.json");
    if (!region_class || !callback || !deserialize_method || !manager || json.empty()) return false;
    Obj text = roots.retain(api.string_new(json.c_str()));
    Obj reflection_type = roots.retain(api.type_get_object(api.class_get_type(region_class)));
    if (!text || !reflection_type) return false;
    void* deserialize_args[] = {text, reflection_type};
    Obj response = nullptr;
    if (!invoke(deserialize_method, nullptr, deserialize_args, "Deserialize local region save", &response) || !response) return false;
    roots.retain(response);
    void* callback_args[] = {response};
    return invoke(callback, manager, callback_args, "SceneDataManager.OnGetSceneProgress local completion");
}

struct Delivery {
    const char* model;
    const char* filename;
    const char* manager;
    const char* callback;
    Klass model_class = nullptr;
    Klass manager_class = nullptr;
    Method callback_method = nullptr;
    Obj message = nullptr;
    Obj receiver = nullptr;
};

bool configure_task(LocalTaskApi& result, Klass helper, const char* method_name,
                    const std::vector<std::string>& inputs, const char* response_name) {
    Method source = find_method(helper, method_name, inputs);
    if (!source) return false;
    result.task_class = api.class_from_type(api.method_get_return_type(source));
    if (!result.task_class) return fail(std::string("Cannot resolve closed task type for ") + method_name);
    result.create = find_method(result.task_class, "Create", {"System.Boolean"});
    std::string result_type = "System.Boolean";
    if (response_name) {
        result.response_class = find_class("ET", response_name);
        Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
        if (!result.response_class || !json_class) return false;
        result.json_method = find_method(json_class, "DeserializeObject", {"System.String", "System.Type"});
        if (!result.json_method) return false;
        result_type = std::string("ET.") + response_name;
    }
    result.set_result = find_method(result.task_class, "SetResult", {result_type});
    if (!result.create || !result.set_result) return false;
    LOGI("Prepared original closed task factory for FightHelper.%s: %s", method_name,
         type_name(api.method_get_return_type(source)).c_str());
    return true;
}

Obj complete_local_task(const LocalTaskApi& task_api, Obj response, bool boolean_result, Roots& roots) {
    bool from_pool = false;
    void* create_args[] = {&from_pool};
    Obj task = nullptr;
    if (!invoke(task_api.create, nullptr, create_args, "Original ETTask.Create(false)", &task) || !task)
        return nullptr;
    roots.retain(task);
    void* result_args[] = {task_api.response_class ? response : static_cast<void*>(&boolean_result)};
    if (!invoke(task_api.set_result, task, result_args, "Original ETTask.SetResult local reply")) return nullptr;
    return task;
}

struct HangUpLocalState {
    int64_t last_claim = 0;
    int64_t quick_return_day = 0;
    int quick_return_count = 0;
};

bool save_hangup_state(const HangUpLocalState& state) {
    const std::string path = files_dir + "/offline_save/hangup_local_state.json";
    const std::string json = "{\"LastClaim\":" + std::to_string(state.last_claim) +
        ",\"QuickReturnDay\":" + std::to_string(state.quick_return_day) +
        ",\"QuickReturnCount\":" + std::to_string(state.quick_return_count) + "}\n";
    if (!write_file(path, json)) return fail("Cannot atomically save local HangUp state");
    return true;
}

bool load_hangup_state(int64_t now, HangUpLocalState& state) {
    const std::string path = files_dir + "/offline_save/hangup_local_state.json";
    std::ifstream saved(path, std::ios::binary);
    const int64_t today = now / 86400;
    if (!saved) {
        // One hour is granted once when this local feature is introduced, which
        // makes an upgraded profile immediately testable without resetting it.
        state.last_claim = now - 3600;
        state.quick_return_day = today;
        state.quick_return_count = 0;
        if (!save_hangup_state(state)) return false;
        LOGI("Created local HangUp ledger with one-hour initial accrual");
        return true;
    }
    const std::string json((std::istreambuf_iterator<char>(saved)), std::istreambuf_iterator<char>());
    int64_t count = 0;
    if (!json_read_int(json, "LastClaim", state.last_claim) ||
        !json_read_int(json, "QuickReturnDay", state.quick_return_day) ||
        !json_read_int(json, "QuickReturnCount", count) || count < 0 || count > 3) {
        return fail("Malformed local HangUp ledger; refusing to reset claim progress");
    }
    state.quick_return_count = static_cast<int>(count);
    bool changed = false;
    if (state.last_claim <= 0 || state.last_claim > now) {
        state.last_claim = now;
        changed = true;
    }
    if (state.quick_return_day != today) {
        state.quick_return_day = today;
        state.quick_return_count = 0;
        changed = true;
    }
    if (changed && !save_hangup_state(state)) return false;
    return true;
}

bool get_hangup_rates(int& chapter, int& hour_gold, int& hour_exp, int& update_period) {
    chapter = 0;
    hour_gold = hour_exp = 0;
    update_period = 600;
    Klass scene_class = find_class("", "SceneDataManager");
    Obj scene = nullptr;
    // Resolve the singleton with a local root scope; all this code runs on the Unity thread.
    Roots roots;
    if (!scene_class) return false;
    scene = singleton(scene_class, roots);
    Field selected_region_field = api.class_get_field_from_name(scene_class, "CurSelectRegion");
    int selected_region = 0;
    if (!scene || !selected_region_field) return fail("Cannot read SceneDataManager.CurSelectRegion for HangUp rates");
    api.field_get_value(scene, selected_region_field, &selected_region);
    if (selected_region <= 0) return fail("Invalid selected region for local HangUp rates");

    Klass cfg_class = find_class("", "TimeGainCfg");
    Method get_cfg = cfg_class ? find_method(cfg_class, "GetCfg", {"System.Int32"}) : nullptr;
    if (!get_cfg) return false;
    const int candidates[] = {selected_region > 1 ? selected_region - 1 : selected_region,
                              selected_region};
    Obj cfg = nullptr;
    for (size_t i = 0; i < 2 && !cfg; ++i) {
        int32_t key = candidates[i];
        void* args[] = {&key};
        if (!invoke(get_cfg, nullptr, args, "TimeGainCfg.GetCfg local HangUp rate", &cfg)) return false;
        if (cfg) chapter = key;
    }
    if (!cfg) return fail("No TimeGainCfg exists for selected local region");
    Klass actual_cfg_class = api.object_get_class(cfg);
    for (const auto& entry : {std::pair<const char*, int*> {"hour_gold", &hour_gold},
                              std::pair<const char*, int*> {"hour_exp", &hour_exp},
                              std::pair<const char*, int*> {"time", &update_period}}) {
        Field field = api.class_get_field_from_name(actual_cfg_class, entry.first);
        if (!field) return fail(std::string("Missing TimeGainCfg field ") + entry.first);
        api.field_get_value(cfg, field, entry.second);
    }
    if (hour_gold < 0 || hour_exp < 0 || update_period <= 0)
        return fail("Invalid TimeGainCfg HangUp rates or update interval");
    return true;
}

bool account_progress(int level, int64_t exp, kinja::AccountProgress& result, Roots& roots) {
    // Read the existing 1.9.1 table through IL2CPP; no copied curve or new RVAs.
    Klass cfg_class = find_class("", "AccountLevelCfg");
    Method all = cfg_class ? find_method(cfg_class, "GetAllCfg", {}) : nullptr;
    Method get = cfg_class ? find_method(cfg_class, "GetCfg", {"System.Int32"}) : nullptr;
    Obj dictionary = nullptr, boxed_count = nullptr;
    if (!all || !get || !invoke(all, nullptr, nullptr, "AccountLevelCfg.GetAllCfg", &dictionary) || !dictionary)
        return fail("Cannot resolve original account level table");
    roots.retain(dictionary);
    Method count = find_method(api.object_get_class(dictionary), "get_Count", {});
    if (!count || !invoke(count, dictionary, nullptr, "AccountLevelCfg count", &boxed_count) || !boxed_count)
        return false;
    roots.retain(boxed_count);
    void* count_value = api.object_unbox(boxed_count);
    int n = count_value ? *static_cast<int32_t*>(count_value) : 0;
    if (n < 1 || n > 1000) return fail("Invalid original account level table size");
    std::vector<int> costs;
    for (int32_t key = 1; key <= n; ++key) {
        Obj cfg = nullptr;
        void* args[] = {&key};
        if (!invoke(get, nullptr, args, "AccountLevelCfg.GetCfg progression", &cfg) || !cfg)
            return fail("Account level table is not contiguous");
        roots.retain(cfg);
        Klass klass = api.object_get_class(cfg);
        Field level_field = api.class_get_field_from_name(klass, "level");
        Field cost_field = api.class_get_field_from_name(klass, "cost_exp");
        int configured_level = 0, cost = 0;
        if (!level_field || !cost_field) return fail("Missing original account level fields");
        api.field_get_value(cfg, level_field, &configured_level);
        api.field_get_value(cfg, cost_field, &cost);
        if (configured_level != key || cost <= 0) return fail("Invalid original account level row");
        costs.push_back(cost);
    }
    return kinja::advance_account(level, exp, costs, result) || fail("Invalid saved account level/EXP");
}

bool repair_saved_account_progress(Roots& roots) {
    std::string player = load_seed("seed_player.json");
    int64_t level = 0, exp = -1;
    if (!json_read_int(player, "Level", level) || !json_read_int(player, "Exp", exp) ||
        level < 1 || level > INT32_MAX) return fail("Cannot read saved account progression");
    kinja::AccountProgress next{};
    if (!account_progress(static_cast<int>(level), exp, next, roots)) return false;
    if (next.level == level && next.exp == exp) return true;
    if (!json_set_int(player, "Level", next.level) || !json_set_int(player, "Exp", next.exp) ||
        !write_file(files_dir + "/offline_save/seed_player.json", player))
        return fail("Cannot persist normalized account progression");
    LOGI("Local account progression repaired level=%lld->%d exp=%lld->%d; existing EXP only",
         (long long)level, next.level, (long long)exp, next.exp);
    return true;
}

bool grant_local_currency_reward(int gold, int exp, Roots& roots, int diamonds = 0) {
    Klass manager_class = find_class("", "ItemManager");
    Obj manager = manager_class ? singleton(manager_class, roots) : nullptr;
    if (!manager_class || !manager) return fail("Cannot resolve original ItemManager for local currency reward");
    Method get_count = find_method(manager_class, "GetOwnItemNum", {"System.Int32"});
    auto read_count = [&](int id, int& result, const char* context) -> bool {
        Obj boxed = nullptr;
        void* count_args[] = {&id};
        if (!get_count || !invoke(get_count, manager, count_args, context, &boxed) || !boxed)
            return false;
        roots.retain(boxed);
        void* raw = api.object_unbox(boxed);
        if (!raw) return false;
        result = *static_cast<int32_t*>(raw);
        return true;
    };
    int gold_before = 0, exp_before = 0, diamonds_before = 0;
    if (!read_count(101, gold_before, "ItemManager.GetOwnItemNum gold before local reward") ||
        !read_count(104, exp_before, "ItemManager.GetOwnItemNum exp before local reward") ||
        !read_count(102, diamonds_before, "ItemManager.GetOwnItemNum diamonds before local reward")) return false;
    if (gold_before < 0 || exp_before < 0 || diamonds_before < 0 || gold > INT32_MAX - gold_before ||
        exp > INT32_MAX - exp_before || diamonds > INT32_MAX - diamonds_before)
        return fail("Invalid current inventory count or overflow while applying local currency reward");

    // IDs 101, 102 and 104 are backed by PlayerDataManager.PlayerGold/
    // PlayerDiamond/PlayerExp in
    // this client. The item update callback only processes server item rows and
    // does not mutate those two special currencies. Build each original
    // ObscuredInt value through its conversion operator, then call the original
    // PlayerDataManager setters on the Unity thread.
    Klass player_class = find_class("", "PlayerDataManager");
    Obj player = player_class ? singleton(player_class, roots) : nullptr;
    Klass obscured_class = find_class("CodeStage.AntiCheat.ObscuredTypes", "ObscuredInt");
    Method to_obscured = obscured_class ? find_method(obscured_class, "op_Implicit", {"System.Int32"}) : nullptr;
    Method set_gold = player_class ? find_method(player_class, "set_PlayerGold", {"CodeStage.AntiCheat.ObscuredTypes.ObscuredInt"}) : nullptr;
    Method set_exp = player_class ? find_method(player_class, "set_PlayerExp", {"CodeStage.AntiCheat.ObscuredTypes.ObscuredInt"}) : nullptr;
    Method set_level = player_class ? find_method(player_class, "set_PlayerLevel", {"CodeStage.AntiCheat.ObscuredTypes.ObscuredInt"}) : nullptr;
    Method get_level = player_class ? find_method(player_class, "get_PlayerLevel", {}) : nullptr;
    Method decrypt = obscured_class ? find_method(obscured_class, "GetDecrypted", {}) : nullptr;
    Method set_diamonds = player_class ? find_method(player_class, "set_PlayerDiamond", {"CodeStage.AntiCheat.ObscuredTypes.ObscuredInt"}) : nullptr;
    Method sync = player_class ? find_method(player_class, "SyncValueToShow", {}) : nullptr;
    if (!player || !obscured_class || !to_obscured || !set_gold || !set_exp || !set_diamonds || !sync ||
        !set_level || !get_level || !decrypt)
        return fail("Cannot resolve original PlayerDataManager currency setters/refresh");
    auto read_level = [&](int& level) -> bool {
        Obj obscured = nullptr, boxed = nullptr;
        if (!invoke(get_level, player, nullptr, "PlayerDataManager.get_PlayerLevel local progression", &obscured) || !obscured)
            return false;
        roots.retain(obscured);
        if (!invoke(decrypt, obscured, nullptr, "ObscuredInt.GetDecrypted account level", &boxed) || !boxed)
            return false;
        roots.retain(boxed);
        void* value = api.object_unbox(boxed);
        if (!value) return false;
        level = *static_cast<int32_t*>(value);
        return level > 0;
    };
    int level_before = 0;
    if (!read_level(level_before)) return fail("Cannot read live account level");
    std::string player_json = load_seed("seed_player.json");
    int64_t saved_gold = -1, saved_exp = -1, saved_diamonds = -1, saved_level = -1;
    if (player_json.empty() || !json_read_int(player_json, "Coin", saved_gold) ||
        !json_read_int(player_json, "Exp", saved_exp) || !json_read_int(player_json, "Diamond", saved_diamonds) ||
        !json_read_int(player_json, "Level", saved_level))
        return fail("Cannot read existing player fields before local currency mutation");
    if (saved_gold != gold_before || saved_exp != exp_before || saved_diamonds != diamonds_before || saved_level != level_before)
        return fail("Local player save differs from live currency/level; refusing to overwrite unrelated progress");
    kinja::AccountProgress progress{level_before, exp_before};
    if (exp < 0) return fail("Negative account EXP rewards are unsupported");
    if (exp > 0 && !account_progress(level_before, static_cast<int64_t>(exp_before) + exp, progress, roots))
        return false;
    const int32_t new_gold = gold_before + gold;
    const int32_t new_exp = progress.exp;
    const int32_t new_diamonds = diamonds_before + diamonds;
    auto set_value = [&](int32_t value, Method setter, const char* context) -> bool {
        void* conversion_args[] = {&value};
        Obj boxed = nullptr;
        if (!invoke(to_obscured, nullptr, conversion_args, "ObscuredInt.op_Implicit local currency value", &boxed) || !boxed)
            return false;
        roots.retain(boxed);
        void* unboxed = api.object_unbox(boxed);
        if (!unboxed) return fail("Cannot unbox original ObscuredInt for local currency setter");
        void* setter_args[] = {unboxed};
        return invoke(setter, player, setter_args, context);
    };
    auto rollback = [&]() {
        bool ok = set_value(gold_before, set_gold, "PlayerDataManager rollback gold");
        ok = set_value(exp_before, set_exp, "PlayerDataManager rollback EXP") && ok;
        ok = set_value(level_before, set_level, "PlayerDataManager rollback level") && ok;
        ok = set_value(diamonds_before, set_diamonds, "PlayerDataManager rollback diamonds") && ok;
        ok = invoke(sync, player, nullptr, "PlayerDataManager.SyncValueToShow rollback") && ok;
        if (!ok) LOGE("Local currency/level rollback failed; restart required to reload unchanged save");
    };
    if (!set_value(new_gold, set_gold, "PlayerDataManager.set_PlayerGold local reward") ||
        !set_value(new_exp, set_exp, "PlayerDataManager.set_PlayerExp local reward") ||
        (progress.level != level_before && !set_value(progress.level, set_level, "PlayerDataManager.set_PlayerLevel local reward")) ||
        !set_value(new_diamonds, set_diamonds, "PlayerDataManager.set_PlayerDiamond local reward") ||
        !invoke(sync, player, nullptr, "PlayerDataManager.SyncValueToShow local reward")) { rollback(); return false; }
    int gold_after = 0, exp_after = 0, diamonds_after = 0, level_after = 0;
    const bool have_gold = read_count(101, gold_after, "ItemManager.GetOwnItemNum gold after local reward");
    const bool have_exp = read_count(104, exp_after, "ItemManager.GetOwnItemNum exp after local reward");
    const bool have_diamonds = read_count(102, diamonds_after, "ItemManager.GetOwnItemNum diamonds after local reward");
    LOGI("Local currency readback gold101=%s%d diamonds102=%s%d exp104=%s%d expectedGold=%d expectedDiamonds=%d expectedExp=%d",
         have_gold ? "" : "unavailable/", have_gold ? gold_after : 0,
         have_diamonds ? "" : "unavailable/", have_diamonds ? diamonds_after : 0,
         have_exp ? "" : "unavailable/", have_exp ? exp_after : 0,
         new_gold, new_diamonds, new_exp);
    if (!have_gold || !have_exp || !have_diamonds || gold_after != gold_before + gold ||
        exp_after != new_exp || diamonds_after != diamonds_before + diamonds ||
        !read_level(level_after) || level_after != progress.level) {
        rollback();
        return fail("Original currency setters did not apply the complete local reward");
    }
    if (!json_set_int(player_json, "Coin", gold_after) || !json_set_int(player_json, "Exp", exp_after) ||
        !json_set_int(player_json, "Level", level_after) ||
        !json_set_int(player_json, "Diamond", diamonds_after) ||
        !write_file(files_dir + "/offline_save/seed_player.json", player_json)) {
        rollback();
        return fail("Cannot persist currency to the existing local player save");
    }
    LOGI("Persisted local currency without replacing other player fields: Coin=%d Diamond=%d Exp=%d",
         gold_after, diamonds_after, exp_after);
    if (level_after != level_before) {
        LOGI("Local account level advanced level=%d->%d exp=%d+%d->%d", level_before, level_after, exp_before, exp, exp_after);
        Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
        Method json_method = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
        // Currency is already committed. A menu-refresh failure must not turn
        // this into a failed reward that can be claimed a second time.
        if (!json_method || !set_local_opening_functions(json_method, roots))
            LOGE("Account level saved; menu refresh requires restart: %s", last_error.c_str());
    }
    return true;
}

// A purchase updates two seed files. The journal lets bootstrap finish a
// purchase interrupted between the Coin and TalentLevel atomic file writes.
const std::string talent_purchase_journal_path() {
    return files_dir + "/offline_save/talent_purchase_pending.json";
}

bool reconcile_pending_talent_purchase() {
    const std::string path = talent_purchase_journal_path();
    std::ifstream input(path, std::ios::binary);
    if (!input) return true;
    const std::string journal((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    int64_t currency_id = 101, talent_kind = 0;
    int64_t currency_before = -1, currency_after = -1, talent_before = -1, talent_after = -1, cost = -1;
    const bool modern = json_read_int(journal, "CurrencyId", currency_id);
    if (!json_read_int(journal, modern ? "CurrencyBefore" : "GoldBefore", currency_before) ||
        !json_read_int(journal, modern ? "CurrencyAfter" : "GoldAfter", currency_after) ||
        !json_read_int(journal, "TalentBefore", talent_before) ||
        !json_read_int(journal, "TalentAfter", talent_after) ||
        !json_read_int(journal, "Cost", cost) ||
        (modern && !json_read_int(journal, "TalentKind", talent_kind)) ||
        (currency_id != 101 && currency_id != 40001) || talent_kind < 0 || talent_kind > 1 ||
        currency_before < 0 || currency_after < 0 || cost <= 0 ||
        currency_before - cost != currency_after || talent_before < 0 ||
        talent_after != talent_before + 1)
        return fail("Malformed pending talent purchase; preserving the local save and journal");
    std::string player = load_seed("seed_player.json");
    std::string talents = load_seed("seed_talents.json");
    std::string items = currency_id == 40001 ? load_seed("seed_items.json") : std::string();
    int64_t saved_currency = -1, saved_talent = -1;
    const char* talent_field = talent_kind == 1 ? "ExtraTalentLevel" : "TalentLevel";
    const bool currency_ok = currency_id == 101 ? json_read_int(player, "Coin", saved_currency) :
        (!items.empty() && json_get_item_count(items, 40001, saved_currency));
    if (player.empty() || talents.empty() || !currency_ok ||
        !json_read_int(talents, talent_field, saved_talent))
        return fail("Cannot read saved values for pending talent purchase");
    if ((saved_currency != currency_before && saved_currency != currency_after) ||
        (saved_talent != talent_before && saved_talent != talent_after))
        return fail("Pending talent purchase conflicts with saved progress; refusing recovery");
    // Before/before means the process stopped before its first mutation.
    if (saved_currency == currency_before && saved_talent == talent_before) {
        if (unlink(path.c_str()) != 0) return fail("Cannot remove unapplied talent purchase journal");
        LOGI("Discarded unapplied local talent purchase journal");
        return true;
    }
    if (saved_currency != currency_after) {
        const bool saved = currency_id == 101 ?
            (json_set_int(player, "Coin", currency_after) && write_file(files_dir + "/offline_save/seed_player.json", player)) :
            (json_add_item(items, "ItemsInfo", 40001, static_cast<int>(-cost), "ItemId", "Count") &&
             write_file(files_dir + "/offline_save/seed_items.json", items));
        if (!saved) return fail("Cannot finish currency debit from pending talent purchase");
    }
    if (saved_talent != talent_after &&
        (!json_set_int(talents, talent_field, talent_after) ||
         !write_file(files_dir + "/offline_save/seed_talents.json", talents)))
        return fail("Cannot finish talent level from pending talent purchase");
    if (unlink(path.c_str()) != 0) return fail("Cannot remove completed talent purchase journal");
    LOGI("Recovered pending local talent purchase currency=%d balance=%lld field=%s level=%lld",
         static_cast<int>(currency_id), static_cast<long long>(currency_after), talent_field,
         static_cast<long long>(talent_after));
    return true;
}

bool read_original_item_count(Obj item_manager, Klass item_class, int item_id, int& count, Roots& roots) {
    Method getter = find_method(item_class, "GetOwnItemNum", {"System.Int32"});
    if (!getter) return false;
    void* args[] = {&item_id};
    Obj boxed = nullptr;
    if (!invoke(getter, item_manager, args, "ItemManager.GetOwnItemNum talent purchase", &boxed) || !boxed)
        return false;
    roots.retain(boxed);
    void* value = api.object_unbox(boxed);
    if (!value) return fail("Cannot unbox original item count for talent purchase");
    count = *static_cast<int32_t*>(value);
    return true;
}

void treasure_box_on_open_by_item_hook(Obj self, Obj go, Method method) {
    if (!original_treasure_box_on_open_by_item) return;
    int32_t previous_context = chest_payment_context;
    int32_t state = 0;
    int32_t chest_id = 0;
    if (self) {
        Klass widget_class = api.object_get_class(self);
        Field state_field = widget_class ? api.class_get_field_from_name(widget_class, "_openTreasureBoxState") : nullptr;
        Field cfg_field = widget_class ? api.class_get_field_from_name(widget_class, "Cfg") : nullptr;
        if (state_field) api.field_get_value(self, state_field, &state);
        if (cfg_field) {
            Obj cfg = nullptr;
            api.field_get_value(self, cfg_field, &cfg);
            Klass cfg_class = cfg ? api.object_get_class(cfg) : nullptr;
            Field id_field = cfg_class ? api.class_get_field_from_name(cfg_class, "id") : nullptr;
            if (id_field) api.field_get_value(cfg, id_field, &chest_id);
        }
    }
    chest_payment_context = (state == 3 || state == 4) ? state : 0;
    LOGI("TreasureBoxItem.OnOpenByItem route state=%d chest=%d", state, chest_id);
    // This original 1.9.1 handler synchronously calls ShopManager.BuyTreasureChest.
    // Keep the payment choice scoped to that Unity-thread callback chain only.
    original_treasure_box_on_open_by_item(self, go, method);
    chest_payment_context = previous_context;
}

int local_calendar_day() {
    const std::time_t now = std::time(nullptr);
    std::tm local{};
    if (!localtime_r(&now, &local)) return 0;
    return (local.tm_year + 1900) * 10000 + (local.tm_mon + 1) * 100 + local.tm_mday;
}

bool read_local_text(const std::string& path, std::string& value) {
    std::ifstream input(path, std::ios::binary);
    if (!input) return false;
    value.assign(std::istreambuf_iterator<char>(input), std::istreambuf_iterator<char>());
    return static_cast<bool>(input) || input.eof();
}

const std::string chest_purchase_root() { return files_dir + "/offline_save/"; }
bool recover_interrupted_chest_purchase() {
    const std::string root = chest_purchase_root();
    std::ifstream pending(root + "chest_purchase_pending", std::ios::binary);
    if (!pending) return true;
    const std::array<std::pair<const char*, const char*>, 3> files{{
        {"seed_player.json", "chest_player.before"},
        {"seed_items.json", "chest_items.before"},
        {"seed_equipment.json", "chest_equipment.before"}
    }};
    for (const auto& file : files) {
        std::string snapshot;
        if (!read_local_text(root + file.second, snapshot) || snapshot.empty() ||
            !write_file(root + file.first, snapshot))
            return fail(std::string("Cannot recover interrupted chest purchase snapshot: ") + file.first);
    }
    unlink((root + "chest_purchase_pending").c_str());
    for (const auto& file : files) unlink((root + file.second).c_str());
    LOGI("Recovered interrupted local chest transaction from pre-purchase snapshots");
    return true;
}

bool begin_chest_purchase_journal(const std::string& player, const std::string& items,
                                  const std::string& equips) {
    const std::string root = chest_purchase_root();
    if (!write_file(root + "chest_player.before", player) ||
        !write_file(root + "chest_items.before", items) ||
        !write_file(root + "chest_equipment.before", equips) ||
        !write_file(root + "chest_purchase_pending", "pending\n"))
        return fail("Cannot create local chest transaction recovery journal");
    return true;
}

void finish_chest_purchase_journal() {
    const std::string root = chest_purchase_root();
    unlink((root + "chest_purchase_pending").c_str());
    unlink((root + "chest_player.before").c_str());
    unlink((root + "chest_items.before").c_str());
    unlink((root + "chest_equipment.before").c_str());
}

std::string daily_shop_claim_path() { return files_dir + "/offline_save/shop_daily_claim.json"; }
std::string daily_shop_pending_path() { return files_dir + "/offline_save/shop_daily_claim_pending.json"; }

bool reconcile_shop_daily_claim(Roots& roots) {
    const std::string pending_path = daily_shop_pending_path();
    std::string pending;
    if (read_local_text(pending_path, pending)) {
        int64_t date = 0, before = -1, after = -1, saved_gold = -1;
        std::string player = load_seed("seed_player.json");
        if (pending.empty() || !json_read_int(pending, "Date", date) ||
            !json_read_int(pending, "Before", before) || !json_read_int(pending, "After", after) ||
            !json_read_int(player, "Coin", saved_gold) || date <= 0 || before < 0 || after != before + 4000)
            return fail("Invalid pending Shop daily claim journal");
        if (saved_gold == before) {
            if (!grant_local_currency_reward(4000, 0, roots))
                return fail("Could not recover prepared Shop daily claim");
        } else if (saved_gold != after) {
            return fail("Pending Shop daily claim conflicts with saved gold; refusing duplicate grant");
        }
        const std::string claimed = "{\"Date\":" + std::to_string(date) +
            ",\"Claimed\":true,\"Amount\":4000}\n";
        if (!write_file(daily_shop_claim_path(), claimed) || unlink(pending_path.c_str()) != 0)
            return fail("Could not finalize Shop daily claim journal");
        LOGI("Recovered local Shop daily claim date=%lld gold=%lld->%lld",
             static_cast<long long>(date), static_cast<long long>(before), static_cast<long long>(after));
    }
    return true;
}

bool set_shop_daily_claim_count(int count, Roots& roots) {
    Klass player_class = find_class("", "PlayerDataManager");
    Obj player = player_class ? singleton(player_class, roots) : nullptr;
    Field count_field = player_class ? api.class_get_field_from_name(player_class, "AdBuyCoinCount") : nullptr;
    if (!player || !count_field || type_name(api.field_get_type(count_field)) != "System.Int32")
        return fail("Cannot resolve original PlayerDataManager.AdBuyCoinCount field");
    api.field_set_value(player, count_field, &count);
    return true;
}

bool perform_shop_daily_claim(Obj page, Roots& roots) {
    if (!reconcile_shop_daily_claim(roots)) return false;
    const int today = local_calendar_day();
    if (!today) return fail("Cannot read local calendar for Shop daily claim");
    std::string state;
    int64_t last_date = 0;
    if (read_local_text(daily_shop_claim_path(), state)) json_read_int(state, "Date", last_date);
    const int used = last_date == today ? 1 : 0;
    if (!set_shop_daily_claim_count(used, roots)) return false;
    if (used) {
        LOGI("Local Shop daily gold claim already used date=%d", today);
        return true;
    }
    std::string player = load_seed("seed_player.json");
    int64_t saved_gold = -1;
    if (player.empty() || !json_read_int(player, "Coin", saved_gold) || saved_gold < 0 || saved_gold > INT32_MAX - 4000)
        return fail("Cannot validate local gold before Shop daily claim");
    Klass item_class = find_class("", "ItemManager");
    Obj item_manager = item_class ? singleton(item_class, roots) : nullptr;
    int live_gold = -1;
    if (!item_manager || !read_original_item_count(item_manager, item_class, 101, live_gold, roots) ||
        live_gold != saved_gold)
        return fail("Live and saved gold differ; refusing Shop daily claim");
    const std::string journal = "{\"Date\":" + std::to_string(today) +
        ",\"Before\":" + std::to_string(saved_gold) +
        ",\"After\":" + std::to_string(saved_gold + 4000) + "}\n";
    if (!write_file(daily_shop_pending_path(), journal)) return fail("Cannot persist Shop daily claim journal");
    if (!grant_local_currency_reward(4000, 0, roots)) {
        unlink(daily_shop_pending_path().c_str());
        return false;
    }
    if (!reconcile_shop_daily_claim(roots)) return false;
    if (!set_shop_daily_claim_count(1, roots)) return false;
    LOGI("Local Shop daily claim granted gold=4000 date=%d", today);
    (void)page;
    return true;
}

bool perform_shop_coin_exchange(const char* config_key, int expected_gold, int expected_diamonds, Roots& roots) {
    // Read the reward and price from the original 1.9.1 Variable config instead
    // of trusting button text or duplicating mutable table data.
    Klass variable_class = find_class("", "Variable");
    Method get_cfg = variable_class ? find_method(variable_class, "GetCfg", {"System.String"}) : nullptr;
    if (!get_cfg) return fail("Cannot resolve original Variable.GetCfg for Shop exchange");
    Obj key = roots.retain(api.string_new(config_key));
    Obj cfg = nullptr;
    void* cfg_args[] = {key};
    if (!key || !invoke(get_cfg, nullptr, cfg_args, "Variable.GetCfg Shop gem-to-gold exchange", &cfg) || !cfg)
        return fail("Original Shop exchange config is unavailable");
    roots.retain(cfg);
    Field values_field = api.class_get_field_from_name(api.object_get_class(cfg), "int_array_value");
    Obj values = nullptr;
    if (!values_field) return fail("Variable config has no int_array_value field");
    api.field_get_value(cfg, values_field, &values);
    if (!values)
        return fail("Shop exchange config does not contain reward and price values");
    roots.retain(values);
    Klass array_class = find_class("System", "Array");
    Method get_value = array_class ? find_method(array_class, "GetValue", {"System.Int32"}, true) : nullptr;
    if (!get_value) return fail("Cannot resolve System.Array.GetValue(Int32) for Shop config");
    auto array_int = [&](int32_t index, int32_t& value) -> bool {
        Obj boxed = nullptr;
        void* args[] = {&index};
        if (!invoke(get_value, values, args, "Read Shop exchange Variable int_array_value", &boxed) || !boxed)
            return false;
        roots.retain(boxed);
        void* raw = api.object_unbox(boxed);
        if (!raw || type_name(api.class_get_type(api.object_get_class(boxed))) != "System.Int32") return false;
        value = *static_cast<int32_t*>(raw);
        return true;
    };
    int32_t reward_gold = 0, cost_diamonds = 0;
    if (!array_int(0, reward_gold) || !array_int(1, cost_diamonds) ||
        reward_gold != expected_gold || cost_diamonds != expected_diamonds ||
        reward_gold <= 0 || cost_diamonds <= 0)
        return fail("Shop exchange config differs from the validated 1.9.1 offer; refusing purchase");

    Klass item_class = find_class("", "ItemManager");
    Obj item_manager = item_class ? singleton(item_class, roots) : nullptr;
    int live_gold = -1, live_diamonds = -1;
    if (!item_manager || !read_original_item_count(item_manager, item_class, 101, live_gold, roots) ||
        !read_original_item_count(item_manager, item_class, 102, live_diamonds, roots))
        return fail("Cannot read live gold and gems for Shop exchange");
    std::string player = load_seed("seed_player.json");
    int64_t saved_gold = -1, saved_diamonds = -1;
    if (player.empty() || !json_read_int(player, "Coin", saved_gold) ||
        !json_read_int(player, "Diamond", saved_diamonds) || saved_gold < 0 || saved_diamonds < 0 ||
        saved_gold != live_gold || saved_diamonds != live_diamonds)
        return fail("Live Shop currencies differ from the existing save; refusing exchange");
    if (live_diamonds < cost_diamonds)
        return fail("Insufficient gems for this Shop gold exchange; no currency was changed");
    if (live_gold > INT32_MAX - reward_gold)
        return fail("Shop gold exchange would exceed the supported balance; no currency was changed");

    if (!grant_local_currency_reward(reward_gold, 0, roots, -cost_diamonds))
        return fail("Shop gem-to-gold transaction failed; see currency readback/rollback logs");
    LOGI("Local Shop gem-to-gold exchange committed config=%s gems=%d->%d gold=%d->%d",
         config_key, live_diamonds, live_diamonds - cost_diamonds, live_gold, live_gold + reward_gold);
    return true;
}

int next_saved_equip_uid(const std::string& json) {
    int64_t highest = 0;
    size_t scan = 0;
    while ((scan = json.find("\"Uid\"", scan)) != std::string::npos) {
        int64_t uid = 0;
        if (json_read_int(json, "Uid", uid, scan) && uid > highest) highest = uid;
        ++scan;
    }
    return highest >= INT32_MAX ? 0 : static_cast<int>(highest + 1);
}

bool refresh_local_chest_inventory(Roots& roots) {
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Klass item_msg_class = find_class("ET", "G2C_ItemInfo");
    Klass equip_msg_class = find_class("ET", "G2C_EquipData");
    Klass item_manager_class = find_class("", "ItemManager");
    Klass equip_manager_class = find_class("", "EquipManager");
    Method deserialize_method = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    Method on_items = item_manager_class ? find_method(item_manager_class, "OnItemInit", {"ET.G2C_ItemInfo"}) : nullptr;
    Method on_equips = equip_manager_class ? find_method(equip_manager_class, "OnEquipDataInitResp", {"ET.G2C_EquipData"}) : nullptr;
    Obj item_manager = item_manager_class ? singleton(item_manager_class, roots) : nullptr;
    Obj equip_manager = equip_manager_class ? singleton(equip_manager_class, roots) : nullptr;
    if (!deserialize_method || !item_msg_class || !equip_msg_class || !on_items || !on_equips || !item_manager || !equip_manager)
        return fail("Cannot resolve original inventory callbacks for local chest refresh");
    auto deliver = [&](const char* filename, Klass model, Method callback, Obj receiver, const char* label) -> bool {
        const std::string json = load_seed(filename);
        if (json.empty()) return false;
        Obj text = roots.retain(api.string_new(json.c_str()));
        Obj type = roots.retain(api.type_get_object(api.class_get_type(model)));
        if (!text || !type) return false;
        void* args[] = {text, type};
        Obj response = nullptr;
        if (!invoke(deserialize_method, nullptr, args, std::string("Deserialize ") + label, &response) ||
            !response || api.object_get_class(response) != model) return false;
        roots.retain(response);
        void* callback_args[] = {response};
        return invoke(callback, receiver, callback_args, std::string("Original ") + label);
    };
    return deliver("seed_items.json", item_msg_class, on_items, item_manager, "ItemManager.OnItemInit chest refresh") &&
           deliver("seed_equipment.json", equip_msg_class, on_equips, equip_manager, "EquipManager.OnEquipDataInitResp chest refresh");
}

bool find_equipment_record(std::string& json, int uid, std::string& record,
                           size_t* record_start = nullptr, size_t* record_end = nullptr);

#ifdef KINJA_OFFLINE_QA_FUSION_REPEATS
bool append_equipment_clone(std::string& json, const std::string& source_record, int uid) {
    size_t array = json_member_value(json, "EquipsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[' || uid <= 0) return false;
    size_t end = array + 1;
    int object_depth = 0;
    bool quoted = false, escaped = false;
    for (; end < json.size(); ++end) {
        const char c = json[end];
        if (quoted) {
            if (escaped) escaped = false;
            else if (c == '\\') escaped = true;
            else if (c == '"') quoted = false;
            continue;
        }
        if (c == '"') quoted = true;
        else if (c == '{') ++object_depth;
        else if (c == '}') --object_depth;
        else if (c == ']' && object_depth == 0) break;
    }
    if (end >= json.size()) return false;
    std::string clone = source_record;
    if (!json_set_int(clone, "Uid", uid) || !json_set_bool(clone, "IsWear", false)) return false;
    const bool empty = json.find_first_not_of(" \t\r\n", array + 1) == end;
    json.insert(end, std::string(empty ? "" : ",") + clone);
    return true;
}

bool apply_qa_fusion_repeats(Obj equip_manager, Roots& roots) {
    const std::string marker_path = files_dir + "/offline_save/qa_fusion_repeats_v2.json";
    std::string marker;
    const bool has_marker = read_local_text(marker_path, marker);
    if (has_marker && marker.find("\"Status\":\"done\"") != std::string::npos) {
        LOGI("QA fusion repeats already prepared; preserving subsequent fusion gameplay");
        return true;
    }

    const std::string equips_path = files_dir + "/offline_save/seed_equipment.json";
    std::string equipment = load_seed("seed_equipment.json");
    if (equipment.empty() || !equip_manager) return fail("QA fusion repeats cannot read the loaded equipment save");
    Klass manager_class = api.object_get_class(equip_manager);
    Method can_compound = find_method(manager_class, "EquipCanCompound", {"System.Int32"});
    Method relate_compound = find_method(manager_class, "EquipRelateCompound", {"System.Int32"});
    Klass quality_class = find_class("", "EquipQualityCfg");
    Method get_quality_cfg = quality_class ? find_method(quality_class, "GetCfg", {"System.Int32"}, true) : nullptr;
    if (!can_compound || !relate_compound || !get_quality_cfg)
        return fail("QA fusion repeats cannot resolve original 1.9.1 eligibility/config APIs");

    int target_uid = 0, target_id = 0, target_quality = 0;
    int first_clone_uid = 0;
    bool target_can_compound = false, target_relate_compound = false;
    if (has_marker) {
        int64_t uid = 0, id = 0, quality = 0;
        if (marker.find("\"Status\":\"pending\"") == std::string::npos ||
            !json_read_int(marker, "Uid", uid) || !json_read_int(marker, "EquipId", id) ||
            !json_read_int(marker, "Quality", quality) || uid <= 0 || id <= 0 || quality <= 0 ||
            uid > INT32_MAX || id > INT32_MAX || quality >= INT32_MAX)
            return fail("QA fusion repeat journal is malformed; refusing to alter equipment");
        target_uid = static_cast<int>(uid);
        target_id = static_cast<int>(id);
        target_quality = static_cast<int>(quality);
    } else {
        const size_t array = json_member_value(equipment, "EquipsInfo");
        size_t scan = array == std::string::npos ? equipment.size() : array + 1;
        while (scan < equipment.size()) {
            const size_t uid_pos = equipment.find("\"Uid\"", scan);
            if (uid_pos == std::string::npos) break;
            const size_t start = equipment.rfind('{', uid_pos);
            const size_t end = start == std::string::npos ? std::string::npos : json_object_end(equipment, start);
            if (start == std::string::npos || start < array || end == std::string::npos)
                return fail("QA fusion repeat scan found malformed equipment JSON");
            const std::string record = equipment.substr(start, end - start);
            int64_t uid = 0, id = 0, quality = 0;
            bool wearing = true;
            if (!json_read_int(record, "Uid", uid) || !json_read_int(record, "EquipId", id) ||
                !json_read_int(record, "Quality", quality) || !json_read_bool(record, "IsWear", wearing))
                return fail("QA fusion repeat scan found an incomplete equipment record");
            if (uid > 0 && id > 0 && quality > 0 && quality < INT32_MAX && !wearing) {
                int32_t uid_arg = static_cast<int32_t>(uid);
                void* can_args[] = {&uid_arg};
                Obj can_result = nullptr, relate_result = nullptr;
                const bool can_called = invoke(can_compound, equip_manager, can_args, "EquipManager.EquipCanCompound QA repeat selection", &can_result) && can_result;
                const bool relate_called = invoke(relate_compound, equip_manager, can_args, "EquipManager.EquipRelateCompound QA repeat selection", &relate_result) && relate_result;
                if (can_called) roots.retain(can_result);
                if (relate_called) roots.retain(relate_result);
                const bool can_value = can_called && api.object_unbox(can_result) && *static_cast<bool*>(api.object_unbox(can_result));
                const bool relate_value = relate_called && api.object_unbox(relate_result) && *static_cast<bool*>(api.object_unbox(relate_result));
                // The account currently has no piece that is already compoundable.
                // The explicit QA request is to create repeated copies of an existing
                // unequipped item; keep the 1.9.1 API results for post-refresh evidence,
                // but do not bypass these APIs in the actual fusion hook.
                {
                        int32_t next_quality = static_cast<int32_t>(quality + 1);
                        void* cfg_args[] = {&next_quality};
                        Obj next_cfg = nullptr;
                        if (invoke(get_quality_cfg, nullptr, cfg_args, "EquipQualityCfg.GetCfg QA repeat selection", &next_cfg) && next_cfg) {
                            roots.retain(next_cfg);
                            Field quality_field = api.class_get_field_from_name(api.object_get_class(next_cfg), "quality");
                            int32_t configured = 0;
                            if (quality_field) api.field_get_value(next_cfg, quality_field, &configured);
                            if (configured == next_quality) {
                                if (quality > target_quality) {
                                    target_uid = static_cast<int>(uid);
                                    target_id = static_cast<int>(id);
                                    target_quality = static_cast<int>(quality);
                                    target_can_compound = can_value;
                                    target_relate_compound = relate_value;
                                }
                            }
                        }
                    }
                }
            scan = end;
        }
        if (target_uid == 0) {
            LOGI("QA fusion repeats skipped: original EquipManager found no eligible saved equipment");
            return true;
        }
        marker = "{\"Status\":\"pending\",\"Uid\":" + std::to_string(target_uid) +
            ",\"EquipId\":" + std::to_string(target_id) +
            ",\"Quality\":" + std::to_string(target_quality) + ",\"TargetCount\":7}\n";
        if (!write_file(marker_path, marker)) return fail("QA fusion repeat journal could not be created");
        LOGI("QA fusion repeat highest-quality target selected uid=%d equipId=%d quality=%d preCopyCanCompound=%d preCopyRelateCompound=%d",
             target_uid, target_id, target_quality, target_can_compound ? 1 : 0, target_relate_compound ? 1 : 0);
    }

    std::string source_record;
    if (!find_equipment_record(equipment, target_uid, source_record))
        return fail("QA fusion repeat source UID is no longer present");
    size_t array = json_member_value(equipment, "EquipsInfo");
    size_t scan = array + 1;
    int eligible_count = 0;
    while ((scan = equipment.find("\"EquipId\"", scan)) != std::string::npos) {
        size_t start = equipment.rfind('{', scan);
        size_t end = start == std::string::npos ? std::string::npos : json_object_end(equipment, start);
        if (start == std::string::npos || start < array || end == std::string::npos)
            return fail("QA fusion repeat count found malformed equipment JSON");
        std::string record = equipment.substr(start, end - start);
        int64_t id = 0, quality = 0;
        bool wearing = true;
        if (!json_read_int(record, "EquipId", id) || !json_read_int(record, "Quality", quality) ||
            !json_read_bool(record, "IsWear", wearing)) return fail("QA fusion repeat count found incomplete equipment data");
        if (id == target_id && quality == target_quality && !wearing) ++eligible_count;
        scan = end;
    }
    constexpr int kTargetCount = 7;
    int added = 0;
    while (eligible_count < kTargetCount) {
        int uid = next_saved_equip_uid(equipment);
        if (!uid || !append_equipment_clone(equipment, source_record, uid))
            return fail("QA fusion repeat could not append a unique local equipment copy");
        if (first_clone_uid == 0) first_clone_uid = uid;
        ++eligible_count;
        ++added;
    }
    if (added && !write_file(equips_path, equipment)) return fail("QA fusion repeat equipment save failed");
    if (!write_file(marker_path, "{\"Status\":\"done\",\"Uid\":" + std::to_string(target_uid) +
                    ",\"EquipId\":" + std::to_string(target_id) + ",\"Quality\":" +
                    std::to_string(target_quality) + ",\"CopiesAdded\":" + std::to_string(added) + "}\n"))
        return fail("QA fusion repeat completion marker could not be committed");

    if (added) {
        Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
        Klass equip_data_class = find_class("ET", "G2C_EquipData");
        Method deserialize = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
        Method on_equip_data = manager_class ? find_method(manager_class, "OnEquipDataInitResp", {"ET.G2C_EquipData"}) : nullptr;
        Obj text = roots.retain(api.string_new(equipment.c_str()));
        Obj type = equip_data_class ? roots.retain(api.type_get_object(api.class_get_type(equip_data_class))) : nullptr;
        Obj response = nullptr;
        void* args[] = {text, type};
        if (!deserialize || !on_equip_data || !text || !type ||
            !invoke(deserialize, nullptr, args, "Deserialize QA fusion repeats", &response) || !response ||
            api.object_get_class(response) != equip_data_class) return fail("QA fusion repeat inventory model could not be refreshed");
        roots.retain(response);
        void* callback_args[] = {response};
        if (!invoke(on_equip_data, equip_manager, callback_args, "EquipManager.OnEquipDataInitResp QA fusion repeats"))
            return fail("Original EquipManager callback failed after QA fusion repeat save");
    }
    int32_t root_arg = target_uid, material_arg = first_clone_uid;
    void* root_args[] = {&root_arg};
    void* material_args[] = {&material_arg};
    Obj post_root = nullptr, post_material = nullptr;
    bool root_eligible = false, material_eligible = false;
    if (invoke(can_compound, equip_manager, root_args, "EquipManager.EquipCanCompound QA post-refresh", &post_root) && post_root) {
        roots.retain(post_root);
        void* value = api.object_unbox(post_root);
        root_eligible = value && *static_cast<bool*>(value);
    }
    if (first_clone_uid > 0 && invoke(relate_compound, equip_manager, material_args,
                                      "EquipManager.EquipRelateCompound QA post-refresh", &post_material) && post_material) {
        roots.retain(post_material);
        void* value = api.object_unbox(post_material);
        material_eligible = value && *static_cast<bool*>(value);
    }
    LOGI("QA fusion original post-refresh eligibility uid=%d canCompound=%d cloneUid=%d relateCompound=%d",
         target_uid, root_eligible ? 1 : 0, first_clone_uid, material_eligible ? 1 : 0);
    LOGI("QA fusion repeats committed from existing saved equipment equipId=%d quality=%d uid=%d copiesAdded=%d totalUnequippedCopies=%d; no currencies/items changed",
         target_id, target_quality, target_uid, added, eligible_count);
    return true;
}
#endif

bool persist_local_equipment_wear(int part, int uid, Roots& roots) {
    std::string equips = load_seed("seed_equipment.json");
    size_t array = json_member_value(equips, "EquipsInfo");
    if (equips.empty() || array == std::string::npos || equips[array] != '[')
        return fail("Local equipment save has no EquipsInfo list");
    bool found = uid == 0;
    size_t scan = array + 1;
    while ((scan = equips.find("\"Uid\"", scan)) != std::string::npos) {
        size_t start = equips.rfind('{', scan);
        if (start == std::string::npos || start < array) return fail("Malformed local equipment record");
        size_t end = json_object_end(equips, start);
        if (end == std::string::npos) return fail("Malformed local equipment object");
        std::string object = equips.substr(start, end - start);
        int64_t record_uid = -1, record_part = -1;
        if (!json_read_int(object, "Uid", record_uid) || !json_read_int(object, "Part", record_part))
            return fail("Local equipment record is missing Uid or Part");
        if (record_part == part) {
            const bool wearing = uid > 0 && record_uid == uid;
            if (wearing) found = true;
            if (!json_set_bool(object, "IsWear", wearing))
                return fail("Local equipment record is missing IsWear");
            equips.replace(start, end - start, object);
            scan = start + object.size();
        } else scan = end;
    }
    if (!found) return fail("Requested equipment UID does not belong to the selected part");
    const std::string path = files_dir + "/offline_save/seed_equipment.json";
    std::string before;
    read_local_text(path, before);
    if (!write_file(path, equips)) return fail("Could not persist local equipped item");
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Klass message_class = find_class("ET", "G2C_EquipData");
    Klass manager_class = find_class("", "EquipManager");
    Method deserialize_method = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    Method on_equips = manager_class ? find_method(manager_class, "OnEquipDataInitResp", {"ET.G2C_EquipData"}) : nullptr;
    Obj manager = manager_class ? singleton(manager_class, roots) : nullptr;
    Obj text = roots.retain(api.string_new(equips.c_str()));
    Obj type = message_class ? roots.retain(api.type_get_object(api.class_get_type(message_class))) : nullptr;
    Obj response = nullptr;
    void* deserialize_args[] = {text, type};
    if (!message_class || !deserialize_method || !on_equips || !manager || !text || !type ||
        !invoke(deserialize_method, nullptr, deserialize_args, "Deserialize local equipment wear", &response) ||
        !response || api.object_get_class(response) != message_class) {
        if (!before.empty()) write_file(path, before);
        return fail("Could not refresh original EquipManager after local wear");
    }
    roots.retain(response);
    void* callback_args[] = {response};
    if (!invoke(on_equips, manager, callback_args, "EquipManager.OnEquipDataInitResp local wear")) {
        if (!before.empty()) write_file(path, before);
        return false;
    }
    LOGI("Local equipment wear persisted part=%d uid=%d", part, uid);
    return true;
}

const std::string equip_upgrade_txn_path() {
    return files_dir + "/offline_save/equip_upgrade_pending";
}

bool restore_equip_upgrade_snapshot() {
    const std::string root = files_dir + "/offline_save/";
    const std::array<std::pair<const char*, const char*>, 3> files{{
        {"seed_equipment.json", "equip_upgrade_equipment.before"},
        {"seed_items.json", "equip_upgrade_items.before"},
        {"seed_player.json", "equip_upgrade_player.before"}
    }};
    for (const auto& entry : files) {
        std::string before;
        if (!read_local_text(root + entry.second, before) || before.empty() ||
            !write_file(root + entry.first, before))
            return fail(std::string("Cannot restore interrupted equipment-upgrade save: ") + entry.first);
    }
    unlink((root + "equip_upgrade_pending").c_str());
    for (const auto& entry : files) unlink((root + entry.second).c_str());
    LOGI("Recovered interrupted local equipment upgrade by restoring the pre-transaction save");
    return true;
}

bool reconcile_pending_equip_upgrade() {
    std::ifstream marker(equip_upgrade_txn_path(), std::ios::binary);
    if (!marker) return true;
    return restore_equip_upgrade_snapshot();
}

bool find_equipment_record(std::string& json, int uid, std::string& record,
                           size_t* record_start, size_t* record_end) {
    const size_t array = json_member_value(json, "EquipsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t scan = array + 1;
    while ((scan = json.find("\"Uid\"", scan)) != std::string::npos) {
        const size_t start = json.rfind('{', scan);
        if (start == std::string::npos || start < array) return false;
        const size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        std::string candidate = json.substr(start, end - start);
        int64_t candidate_uid = -1;
        if (!json_read_int(candidate, "Uid", candidate_uid)) return false;
        if (candidate_uid == uid) {
            record = candidate;
            if (record_start) *record_start = start;
            if (record_end) *record_end = end;
            return true;
        }
        scan = end;
    }
    return false;
}

bool find_item_count(const std::string& json, int item_id, int64_t& count) {
    const size_t array = json_member_value(json, "ItemsInfo");
    if (array == std::string::npos || array >= json.size() || json[array] != '[') return false;
    size_t scan = array + 1;
    while ((scan = json.find("\"ItemId\"", scan)) != std::string::npos) {
        const size_t start = json.rfind('{', scan);
        if (start == std::string::npos || start < array) return false;
        const size_t end = json_object_end(json, start);
        if (end == std::string::npos) return false;
        const std::string row = json.substr(start, end - start);
        int64_t id = -1;
        if (!json_read_int(row, "ItemId", id)) return false;
        if (id == item_id) return json_read_int(row, "Count", count);
        scan = end;
    }
    count = 0;
    return true;
}

bool refresh_local_equip_upgrade(const std::string& equips, const std::string& items,
                                 Roots& roots, bool rollback) {
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Klass equip_data_class = find_class("ET", rollback ? "G2C_EquipData" : "G2C_EquipDataUpdate");
    Klass item_data_class = find_class("ET", "G2C_ItemInfo");
    Klass equip_manager_class = find_class("", "EquipManager");
    Klass item_manager_class = find_class("", "ItemManager");
    Method deserialize = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    Method equip_callback = equip_manager_class ? find_method(equip_manager_class,
        rollback ? "OnEquipDataInitResp" : "OnEquipDataUpdateResp",
        {rollback ? "ET.G2C_EquipData" : "ET.G2C_EquipDataUpdate"}) : nullptr;
    Method item_callback = item_manager_class ? find_method(item_manager_class, "OnItemInit", {"ET.G2C_ItemInfo"}) : nullptr;
    Obj equip_manager = equip_manager_class ? singleton(equip_manager_class, roots) : nullptr;
    Obj item_manager = item_manager_class ? singleton(item_manager_class, roots) : nullptr;
    if (!deserialize || !equip_data_class || !item_data_class || !equip_callback || !item_callback ||
        !equip_manager || !item_manager)
        return fail("Cannot resolve original equipment/item callbacks for local upgrade");
    auto decode = [&](const std::string& json, Klass model, const char* context, Obj& result) -> bool {
        Obj text = roots.retain(api.string_new(json.c_str()));
        Obj type = roots.retain(api.type_get_object(api.class_get_type(model)));
        if (!text || !type) return false;
        void* args[] = {text, type};
        return invoke(deserialize, nullptr, args, context, &result) && result &&
               api.object_get_class(result) == model;
    };
    Obj equip_data = nullptr, item_data = nullptr;
    if (!decode(equips, equip_data_class, "Deserialize local equipment-upgrade callback", equip_data) ||
        !decode(items, item_data_class, "Deserialize local equipment item callback", item_data)) return false;
    roots.retain(equip_data);
    roots.retain(item_data);
    void* equip_args[] = {equip_data};
    if (!invoke(equip_callback, equip_manager, equip_args, "Original EquipManager equipment-upgrade callback"))
        return false;
    void* item_args[] = {item_data};
    return invoke(item_callback, item_manager, item_args, "Original ItemManager equipment-upgrade callback");
}

struct LocalEquipUpgradePlan {
    int equip_uid = 0;
    int equip_id = 0;
    int quality = 0;
    int level_before = 0;
    int level_after = 0;
    int item_id = 0;
    int item_cost = 0;
    int gold_cost = 0;
};

bool calculate_local_equip_upgrade(int uid, bool one_key, LocalEquipUpgradePlan& plan,
                                   std::string& equipment_json, std::string& item_json,
                                   std::string& player_json, Roots& roots) {
    equipment_json = load_seed("seed_equipment.json");
    item_json = load_seed("seed_items.json");
    player_json = load_seed("seed_player.json");
    std::string saved_record;
    if (uid <= 0 || equipment_json.empty() || item_json.empty() || player_json.empty() ||
        !find_equipment_record(equipment_json, uid, saved_record))
        return fail("Selected equipment UID is not present in the existing local save");
    int64_t saved_uid = -1, saved_id = -1, saved_quality = -1, saved_level = -1, saved_gold = -1;
    if (!json_read_int(saved_record, "Uid", saved_uid) ||
        !json_read_int(saved_record, "EquipId", saved_id) ||
        (!json_read_int(saved_record, "Quality", saved_quality) && !json_read_int(saved_record, "EquipQuality", saved_quality)) ||
        (!json_read_int(saved_record, "Level", saved_level) && !json_read_int(saved_record, "EquipLevel", saved_level)) ||
        !json_read_int(player_json, "Coin", saved_gold) || saved_uid != uid || saved_id <= 0 ||
        saved_quality <= 0 || saved_level <= 0 || saved_gold < 0)
        return fail("Local equipment/player save lacks valid upgrade fields");

    Klass equip_manager_class = find_class("", "EquipManager");
    Klass item_manager_class = find_class("", "ItemManager");
    Klass player_class = find_class("", "PlayerDataManager");
    Klass upgrade_cfg_class = find_class("", "EquipUpgradeCfg");
    if (!equip_manager_class || !item_manager_class || !player_class || !upgrade_cfg_class)
        return fail("Missing exact 1.9.1 equipment-upgrade metadata types");
    Obj equip_manager = singleton(equip_manager_class, roots);
    Obj item_manager = singleton(item_manager_class, roots);
    Method get_equip = find_method(equip_manager_class, "GetEquipInfoByUid", {"System.Int32"});
    Method get_limit = find_method(equip_manager_class, "GetOwnEquipLevelLimit", {"System.Int32"});
    Method get_cfg = find_method(upgrade_cfg_class, "GetCfg", {"System.Int32"}, true);
    Method get_item_count = find_method(item_manager_class, "GetOwnItemNum", {"System.Int32"});
    Method equip_cfg = find_method(equip_manager_class, "GetEquipCfgByIdAndLvAndQuality",
                                   {"System.Int32", "System.Int32", "System.Int32"});
    if (!equip_manager || !item_manager || !get_equip || !get_limit || !get_cfg || !get_item_count || !equip_cfg)
        return fail("Missing exact 1.9.1 equipment-upgrade APIs");
    void* equip_args[] = {&uid};
    Obj boxed_equip = nullptr;
    if (!invoke(get_equip, equip_manager, equip_args, "EquipManager.GetEquipInfoByUid local upgrade", &boxed_equip) || !boxed_equip)
        return fail("Selected equipment is missing from original EquipManager");
    roots.retain(boxed_equip);
    Klass boxed_class = api.object_get_class(boxed_equip);
    auto read_struct_int = [&](const char* field_name, int32_t& value) -> bool {
        Field field = boxed_class ? api.class_get_field_from_name(boxed_class, field_name) : nullptr;
        if (!field) return false;
        api.field_get_value(boxed_equip, field, &value);
        return true;
    };
    int32_t live_uid = 0, live_id = 0, live_quality = 0, live_level = 0;
    if (!read_struct_int("Uid", live_uid) || !read_struct_int("Id", live_id) ||
        !read_struct_int("Quality", live_quality) || !read_struct_int("Level", live_level) ||
        live_uid != uid || live_id != saved_id || live_quality != saved_quality || live_level != saved_level)
        return fail("Live EquipManager state does not match the selected local save record");
    int32_t max_level = 0;
    void* limit_args[] = {&uid};
    Obj boxed_limit = nullptr;
    if (!invoke(get_limit, equip_manager, limit_args, "EquipManager.GetOwnEquipLevelLimit local upgrade", &boxed_limit) || !boxed_limit)
        return false;
    roots.retain(boxed_limit);
    void* limit_value = api.object_unbox(boxed_limit);
    if (!limit_value) return false;
    max_level = *static_cast<int32_t*>(limit_value);
    if (max_level <= live_level || max_level > 1000) return fail("Selected equipment is already at its configured level limit");

    plan.equip_uid = uid;
    plan.equip_id = live_id;
    plan.quality = live_quality;
    plan.level_before = live_level;
    plan.level_after = live_level;
    for (int level = live_level; level < max_level; ++level) {
        if (!one_key && level > live_level) break;
        int32_t cfg_level = level;
        void* cfg_args[] = {&cfg_level};
        Obj cfg = nullptr;
        if (!invoke(get_cfg, nullptr, cfg_args, "EquipUpgradeCfg.GetCfg local equipment upgrade", &cfg) || !cfg)
            break;
        roots.retain(cfg);
        Klass cfg_actual_class = api.object_get_class(cfg);
        Field coin_field = api.class_get_field_from_name(cfg_actual_class, "cost_coin");
        Field item_count_field = api.class_get_field_from_name(cfg_actual_class, "cost_item_num");
        int32_t coin_cost = 0, item_count = 0;
        if (!coin_field || !item_count_field) return fail("EquipUpgradeCfg is missing level cost fields");
        api.field_get_value(cfg, coin_field, &coin_cost);
        api.field_get_value(cfg, item_count_field, &item_count);
        int32_t id = live_id, quality = live_quality, equip_level = level;
        void* equip_cfg_args[] = {&id, &equip_level, &quality};
        Obj exact_equip_cfg = nullptr;
        if (!invoke(equip_cfg, equip_manager, equip_cfg_args, "EquipManager.GetEquipCfgByIdAndLvAndQuality local upgrade", &exact_equip_cfg) || !exact_equip_cfg)
            return fail("Missing equipment-specific upgrade material config");
        roots.retain(exact_equip_cfg);
        Field upgrade_item_field = api.class_get_field_from_name(api.object_get_class(exact_equip_cfg), "upgrade_item");
        int32_t item_id = 0;
        if (!upgrade_item_field) return fail("EquipCfg is missing its configured upgrade_item");
        api.field_get_value(exact_equip_cfg, upgrade_item_field, &item_id);
        if (coin_cost <= 0 || item_count <= 0 || item_id <= 0) break;
        if (plan.item_id && plan.item_id != item_id)
            return fail("One-click upgrade crosses multiple material IDs; refusing a partial/ambiguous transaction");
        int64_t live_gold = 0, live_items = 0, saved_items = 0;
        auto read_live_count = [&](int item, int64_t& count) -> bool {
            Obj boxed = nullptr;
            void* args[] = {&item};
            if (!invoke(get_item_count, item_manager, args, "ItemManager.GetOwnItemNum equipment upgrade", &boxed) || !boxed)
                return false;
            roots.retain(boxed);
            void* value = api.object_unbox(boxed);
            if (!value) return false;
            count = *static_cast<int32_t*>(value);
            return count >= 0;
        };
        int gold_id = 101;
        if (!read_live_count(gold_id, live_gold) || !read_live_count(item_id, live_items) ||
            !find_item_count(item_json, item_id, saved_items) ||
            live_gold != saved_gold || live_items != saved_items)
            return fail("Live currency/material counts differ from the existing local save");
        if (plan.gold_cost > INT32_MAX - coin_cost || plan.item_cost > INT32_MAX - item_count)
            return fail("Equipment upgrade total exceeds safe integer range");
        if (static_cast<int64_t>(plan.gold_cost) + coin_cost > live_gold ||
            static_cast<int64_t>(plan.item_cost) + item_count > live_items) {
            if (plan.level_after == plan.level_before)
                return fail("Insufficient configured coin or book for the next equipment level");
            break;
        }
        if (!plan.item_id) plan.item_id = item_id;
        plan.gold_cost += coin_cost;
        plan.item_cost += item_count;
        ++plan.level_after;
    }
    return plan.level_after > plan.level_before;
}

bool mutate_equipment_upgrade_json(std::string& equipment_json, int uid, int new_level) {
    std::string record;
    size_t start = 0, end = 0;
    if (!find_equipment_record(equipment_json, uid, record, &start, &end)) return false;
    const std::string field = json_member_value(record, "Level") != std::string::npos ? "Level" : "EquipLevel";
    if (!json_set_int(record, field, new_level)) return false;
    equipment_json.replace(start, end - start, record);
    return true;
}

bool apply_local_equip_upgrade(int uid, bool one_key, Roots& roots) {
    if (equip_compound_in_progress.load()) return fail("Equipment upgrade refused during fusion transaction");
    std::string equipment_before, items_before, player_before;
    LocalEquipUpgradePlan plan;
    if (!calculate_local_equip_upgrade(uid, one_key, plan, equipment_before, items_before, player_before, roots))
        return false;
    int64_t old_gold = -1, old_item_count = -1;
    if (!json_read_int(player_before, "Coin", old_gold) ||
        !find_item_count(items_before, plan.item_id, old_item_count) || old_gold < plan.gold_cost || old_item_count < plan.item_cost)
        return fail("Cannot verify saved gold/book balance before local equipment upgrade");
    std::string equipment_after = equipment_before;
    std::string items_after = items_before;
    if (!mutate_equipment_upgrade_json(equipment_after, uid, plan.level_after) ||
        !json_add_item(items_after, "ItemsInfo", plan.item_id, -plan.item_cost, "ItemId", "Count"))
        return fail("Cannot prepare local equipment/material save update");

    const std::string root = files_dir + "/offline_save/";
    const std::array<std::pair<const char*, const std::string*>, 3> snapshots{{
        {"equip_upgrade_equipment.before", &equipment_before},
        {"equip_upgrade_items.before", &items_before},
        {"equip_upgrade_player.before", &player_before}
    }};
    for (const auto& entry : snapshots)
        if (!write_file(root + entry.first, *entry.second)) return fail("Cannot write local equipment-upgrade recovery snapshot");
    if (!write_file(equip_upgrade_txn_path(), "pending\n")) return fail("Cannot mark local equipment-upgrade transaction");
    if (!write_file(root + "seed_equipment.json", equipment_after) ||
        !write_file(root + "seed_items.json", items_after)) {
        restore_equip_upgrade_snapshot();
        return fail("Could not persist equipment/material state; transaction restored");
    }
    if (!grant_local_currency_reward(-plan.gold_cost, 0, roots)) {
        restore_equip_upgrade_snapshot();
        return false;
    }
    if (!refresh_local_equip_upgrade(equipment_after, items_after, roots, false)) {
        grant_local_currency_reward(plan.gold_cost, 0, roots);
        restore_equip_upgrade_snapshot();
        refresh_local_equip_upgrade(equipment_before, items_before, roots, true);
        return false;
    }
    int live_gold = -1, live_items = -1;
    Obj item_manager = singleton(find_class("", "ItemManager"), roots);
    Klass item_manager_class = find_class("", "ItemManager");
    Method get_count = item_manager_class ? find_method(item_manager_class, "GetOwnItemNum", {"System.Int32"}) : nullptr;
    auto read_live = [&](int item_id, int& count) -> bool {
        Obj boxed = nullptr;
        void* args[] = {&item_id};
        if (!get_count || !invoke(get_count, item_manager, args, "ItemManager.GetOwnItemNum equipment-upgrade verification", &boxed) || !boxed) return false;
        roots.retain(boxed);
        void* value = api.object_unbox(boxed);
        if (!value) return false;
        count = *static_cast<int32_t*>(value);
        return true;
    };
    if (!read_live(101, live_gold) || !read_live(plan.item_id, live_items) ||
        live_gold != old_gold - plan.gold_cost || live_items != old_item_count - plan.item_cost) {
        grant_local_currency_reward(plan.gold_cost, 0, roots);
        restore_equip_upgrade_snapshot();
        refresh_local_equip_upgrade(equipment_before, items_before, roots, true);
        return fail("Original manager callbacks did not apply the complete equipment-upgrade transaction");
    }
    // Removing the marker commits all three save files. If the process is interrupted
    // before this point, bootstrap restores the snapshots instead of keeping a partial spend.
    if (unlink(equip_upgrade_txn_path().c_str()) != 0) {
        grant_local_currency_reward(plan.gold_cost, 0, roots);
        restore_equip_upgrade_snapshot();
        refresh_local_equip_upgrade(equipment_before, items_before, roots, true);
        return fail("Cannot commit equipment-upgrade journal; save restored");
    }
    for (const auto& entry : snapshots) unlink((root + entry.first).c_str());
    LOGI("Local equipment upgrade complete uid=%d oneKey=%d level=%d->%d gold=%d books item=%d count=%d",
         uid, one_key ? 1 : 0, plan.level_before, plan.level_after, plan.gold_cost, plan.item_id, plan.item_cost);
    return true;
}

bool collect_local_combat_drop(int enemy_id, int enemy_instance_id) {
    {
        std::lock_guard<std::mutex> lock(run_item_mutex);
        if (enemy_instance_id > 0 && !seen_run_drop_instances.insert(enemy_instance_id).second) {
            LOGI("Duplicate combat drop callback suppressed enemyInstance=%d", enemy_instance_id);
            return true;
        }
    }
    Roots roots;
    Klass monster_class = find_class("", "Monster");
    Method get_monster = monster_class ? find_method(monster_class, "GetCfg", {"System.Int32"}, true) : nullptr;
    void* monster_args[] = {&enemy_id};
    Obj monster = nullptr;
    if (!get_monster || !invoke(get_monster, nullptr, monster_args, "Monster.GetCfg local combat drop", &monster) || !monster) {
        LOGI("No local monster drop config for enemy=%d; no item granted", enemy_id);
        return true;
    }
    roots.retain(monster);
    Field reward_field = api.class_get_field_from_name(api.object_get_class(monster), "reward_id");
    int32_t reward_id = 0;
    if (!reward_field) return fail("Monster config has no reward_id field");
    api.field_get_value(monster, reward_field, &reward_id);
    if (reward_id <= 0) return true;
    Klass rewards_class = find_class("", "RewardManager");
    Obj rewards = rewards_class ? singleton(rewards_class, roots) : nullptr;
    Method random_reward = rewards_class ? find_method(rewards_class, "RandomRewardPack", {"System.Int32", "System.Boolean"}) : nullptr;
    bool calc_buff = false;
    void* reward_args[] = {&reward_id, &calc_buff};
    Obj boxed_pack = nullptr;
    if (!rewards || !random_reward || !invoke(random_reward, rewards, reward_args,
            "RewardManager.RandomRewardPack local monster drop", &boxed_pack) || !boxed_pack) {
        LOGE("Could not resolve configured local drop rewardId=%d", reward_id);
        return false;
    }
    roots.retain(boxed_pack);
    Klass pack_class = api.object_get_class(boxed_pack);
    Field item_id_field = api.class_get_field_from_name(pack_class, "ItemId");
    Field item_num_field = api.class_get_field_from_name(pack_class, "ItemNum");
    int32_t item_id = 0, item_num = 0;
    if (!item_id_field || !item_num_field) return fail("RewardPack is missing item fields");
    api.field_get_value(boxed_pack, item_id_field, &item_id);
    api.field_get_value(boxed_pack, item_num_field, &item_num);
    bool offline_book_pity = false;
    if (item_id == 0 && item_num > 0 && reward_id >= 100020 && reward_id <= 100990 && reward_id % 10 == 0) {
        std::lock_guard<std::mutex> lock(run_item_mutex);
        ++empty_book_rolls_this_run;
        if (empty_book_rolls_this_run >= 10) {
            // Common combat tables devote 96% of their weight to ItemId 0.
            // Keep the configured book IDs/counts but guarantee one after ten
            // empty eligible rolls so offline progression does not stall.
            item_id = 20001 + ((enemy_instance_id % 6 + 6) % 6);
            item_num = 1;
            empty_book_rolls_this_run = 0;
            offline_book_pity = true;
        }
    } else if (item_id >= 20001 && item_id <= 20006) {
        std::lock_guard<std::mutex> lock(run_item_mutex);
        empty_book_rolls_this_run = 0;
    }
    LOGI("Configured combat drop rolled enemy=%d rewardId=%d item=%d count=%d",
         enemy_id, reward_id, item_id, item_num);
    if (offline_book_pity)
        LOGI("Offline book pity queued after 10 empty eligible rolls item=%d count=1; victory required", item_id);
    if (item_id > 0 && item_num > 0) {
        std::lock_guard<std::mutex> lock(run_item_mutex);
        auto existing = std::find_if(pending_run_items.begin(), pending_run_items.end(),
            [item_id](const auto& row) { return row.first == item_id; });
        if (existing == pending_run_items.end()) pending_run_items.emplace_back(item_id, item_num);
        else existing->second += item_num;
        LOGI("Configured combat drop queued enemy=%d rewardId=%d item=%d count=%d",
             enemy_id, reward_id, item_id, item_num);
    }
    return true;
}

bool local_chest_purchase(Obj shop, int chest_id, bool free, Obj effect_point, Method original_method,
                          int payment_state, Roots& roots) {
    // Verified 1.9.1 local gem offers: bronze single (101), diamond ten-draw (108),
    // and the alternate diamond single (115). The ten-draw count and 2200-gem
    // price are read from TreasureBoxCfg below; rewards still use its original pool.
    if (free || (chest_id != 101 && chest_id != 108 && chest_id != 115)) {
        LOGI("Refused unsupported or ad chest request id=%d free=%d", chest_id, free ? 1 : 0);
        return false;
    }
    Klass box_class = find_class("", "TreasureBoxCfg");
    Method get_box = box_class ? find_method(box_class, "GetCfg", {"System.Int32"}) : nullptr;
    int32_t requested_id = chest_id;
    void* cfg_args[] = {&requested_id};
    Obj box = nullptr;
    if (!get_box || !invoke(get_box, nullptr, cfg_args, "TreasureBoxCfg.GetCfg local chest", &box) || !box)
        return false;
    roots.retain(box);
    Klass actual_box_class = api.object_get_class(box);
    Field diamond_field = api.class_get_field_from_name(actual_box_class, "cost_diamond");
    Field open_item_field = api.class_get_field_from_name(actual_box_class, "open_item_id");
    Field draws_field = api.class_get_field_from_name(actual_box_class, "item_nums");
    int32_t cost = 0;
    int32_t open_item_id = 0, draw_count = 0;
    if (!diamond_field || !open_item_field || !draws_field) return false;
    api.field_get_value(box, diamond_field, &cost);
    api.field_get_value(box, open_item_field, &open_item_id);
    api.field_get_value(box, draws_field, &draw_count);
    const bool pay_with_item = payment_state == 3;
    const int payment_item_cost = draw_count;
    if (draw_count <= 0 || draw_count > 10 || (pay_with_item && open_item_id <= 0))
        return fail("Local chest has invalid configured item payment");
    if (!pay_with_item && cost <= 0) return fail("Local gem chest has invalid configured price");
    Klass item_class = find_class("", "ItemManager");
    Obj items_manager = item_class ? singleton(item_class, roots) : nullptr;
    int diamonds = -1, payment_item_balance = -1;
    if (!items_manager || !read_original_item_count(items_manager, item_class, 102, diamonds, roots)) return false;
    if (pay_with_item && !read_original_item_count(items_manager, item_class, open_item_id, payment_item_balance, roots))
        return false;
    if ((!pay_with_item && diamonds < cost) || (pay_with_item && payment_item_balance < payment_item_cost)) {
        if (pay_with_item) {
            LOGI("Local chest insufficient key count chest=%d item=%d balance=%d cost=%d; forwarding original insufficient-item UI",
                 chest_id, open_item_id, payment_item_balance, payment_item_cost);
            if (original_buy_treasure_chest) original_buy_treasure_chest(shop, chest_id, false, effect_point, original_method);
            return false;
        }
        LOGI("Local chest purchase insufficient gems chest=%d balance=%d cost=%d; invoking original insufficient-balance UI branch",
             chest_id, diamonds, cost);
        if (original_buy_treasure_chest) original_buy_treasure_chest(shop, chest_id, false, effect_point, original_method);
        return false;
    }
    const std::string player_before = load_seed("seed_player.json");
    const std::string items_before = load_seed("seed_items.json");
    const std::string equips_before = load_seed("seed_equipment.json");
    int64_t saved_diamonds = -1, saved_payment_item = -1;
    if (player_before.empty() || items_before.empty() || equips_before.empty() ||
        !json_read_int(player_before, "Diamond", saved_diamonds) || saved_diamonds != diamonds ||
        (pay_with_item && (!json_get_item_count(items_before, open_item_id, saved_payment_item) ||
                           saved_payment_item != payment_item_balance)))
        return fail("Chest purchase refused because live payment balance differs from existing save");

    Klass content_class = find_class("", "TBContentCfg");
    Method get_content = content_class ? find_method(content_class, "GetCfg", {"System.Int32"}) : nullptr;
    Klass equip_class = find_class("", "EquipCfg");
    Method get_equip = equip_class ? find_method(equip_class, "GetCfg", {"System.Int32"}) : nullptr;
    if (!get_content || !get_equip) return false;
    const int first_cfg = chest_id == 101 ? 1001 : 1175;
    const int last_cfg = chest_id == 101 ? 1036 : 1240;
    const int expected_box_id = chest_id == 101 ? 1011 : 1021;
    Field box_id_field = api.class_get_field_from_name(content_class, "box_id");
    Field weight_field = api.class_get_field_from_name(content_class, "base_weight");
    if (!box_id_field || !weight_field) return false;
    std::vector<std::pair<int32_t, int32_t>> weighted_content;
    int64_t total_weight = 0;
    for (int32_t cfg_id = first_cfg; cfg_id <= last_cfg; ++cfg_id) {
        void* cfg_args[] = {&cfg_id};
        Obj cfg = nullptr;
        if (!invoke(get_content, nullptr, cfg_args, "TBContentCfg.GetCfg local chest pool", &cfg) || !cfg) return false;
        roots.retain(cfg);
        int32_t pool_id = 0, weight = 0;
        api.field_get_value(cfg, box_id_field, &pool_id);
        api.field_get_value(cfg, weight_field, &weight);
        if (pool_id != expected_box_id || weight < 0) return fail("TBContentCfg differs from the verified 1.9.1 chest pool");
        if (weight > 0) { weighted_content.emplace_back(cfg_id, weight); total_weight += weight; }
    }
    if (weighted_content.empty() || total_weight <= 0) return fail("Configured local chest reward pool is empty");
    static std::atomic<uint64_t> roll_counter{0};
    std::mt19937_64 rng(static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count()) ^ roll_counter.fetch_add(1));
    std::vector<int32_t> content_ids;
    for (int32_t draw = 0; draw < draw_count; ++draw) {
        int64_t roll = static_cast<int64_t>(rng() % static_cast<uint64_t>(total_weight));
        for (const auto& candidate : weighted_content) {
            if (roll < candidate.second) { content_ids.push_back(candidate.first); break; }
            roll -= candidate.second;
        }
    }
    if (static_cast<int32_t>(content_ids.size()) != draw_count) return fail("Weighted chest selection missed a configured draw");
    const int32_t way = 4;
    std::string items_after = items_before;
    if (pay_with_item && !json_add_item(items_after, "ItemsInfo", open_item_id, -payment_item_cost,
                                        "ItemId", "Count"))
        return fail("Cannot prepare configured chest-key debit");
    std::string equips_after = equips_before;
    int next_uid = next_saved_equip_uid(equips_after);
    std::string content_json = "[";
    for (size_t index = 0; index < content_ids.size(); ++index) {
        int32_t content_id = content_ids[index];
        if (content_id <= 0) return fail("OpenBoxResp returned an invalid content ID");
        if (index) content_json += ",";
        content_json += std::to_string(content_id);
        void* content_args[] = {&content_id};
        Obj content = nullptr;
        if (!invoke(get_content, nullptr, content_args, "TBContentCfg.GetCfg chest reward", &content) || !content) return false;
        roots.retain(content);
        Klass actual_content = api.object_get_class(content);
        int32_t type = 0, item_id = 0, quality = 0, count = 0;
        for (const auto& field : {std::pair<const char*, int32_t*> {"type", &type}, {"item_id", &item_id},
                                  {"quality", &quality}, {"count", &count}}) {
            Field f = api.class_get_field_from_name(actual_content, field.first);
            if (!f) return fail(std::string("Missing TBContentCfg field ") + field.first);
            api.field_get_value(content, f, field.second);
        }
        if (item_id <= 0 || count <= 0) return fail("Configured chest reward has invalid item/count");
        if (type == 1) {
            void* equip_args[] = {&item_id};
            Obj equip_cfg = nullptr;
            if (!invoke(get_equip, nullptr, equip_args, "EquipCfg.GetCfg chest equipment", &equip_cfg) || !equip_cfg) return false;
            roots.retain(equip_cfg);
            Klass actual_equip = api.object_get_class(equip_cfg);
            Field part_field = api.class_get_field_from_name(actual_equip, "part");
            int32_t part = 0;
            if (!part_field || next_uid <= 0) return fail("Cannot allocate valid local equipment identity");
            api.field_get_value(equip_cfg, part_field, &part);
            for (int32_t copy = 0; copy < count; ++copy) {
                if (!json_add_equipment(equips_after, {item_id, 1, quality, next_uid++, part}))
                    return fail("Cannot persist configured chest equipment reward");
            }
        } else if (!json_add_item(items_after, "ItemsInfo", item_id, count, "ItemId", "Count")) {
            return fail("Cannot persist configured chest item reward");
        }
    }
    content_json += "]";
    const std::string player_path = files_dir + "/offline_save/seed_player.json";
    const std::string items_path = files_dir + "/offline_save/seed_items.json";
    const std::string equips_path = files_dir + "/offline_save/seed_equipment.json";
    const int32_t reported_cost = pay_with_item ? payment_item_cost : cost;
    const std::string response_json = "{\"Way\":" + std::to_string(way) + ",\"ChestId\":" +
        std::to_string(chest_id) + ",\"Cost\":" + std::to_string(reported_cost) + ",\"ContentIds\":" +
        content_json + ",\"ExtraReward\":[],\"ServerPackItem\":[]}";
    Klass response_class = find_class("ET", "G2C_BuyChestMsg");
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Method deserialize_method = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    Klass shop_manager_class = find_class("", "ShopManager");
    Method on_bought = shop_manager_class ? find_method(shop_manager_class, "OnBuyTreasureChestSucceed", {"ET.G2C_BuyChestMsg"}) : nullptr;
    if (!response_class || !deserialize_method || !on_bought) return false;
    Obj response_text = roots.retain(api.string_new(response_json.c_str()));
    Obj response_type = roots.retain(api.type_get_object(api.class_get_type(response_class)));
    void* response_args[] = {response_text, response_type};
    Obj response = nullptr;
    if (!response_text || !response_type || !invoke(deserialize_method, nullptr, response_args,
            "Deserialize local G2C_BuyChestMsg", &response) || !response || api.object_get_class(response) != response_class)
        return false;
    roots.retain(response);

    // Debit through original currency setters. Prepare complete reward saves first,
    // then atomically replace each existing seed; on any failure restore all files.
    if (!begin_chest_purchase_journal(player_before, items_before, equips_before)) return false;
    if (pay_with_item) {
        if (!write_file(items_path, items_after) || !refresh_local_chest_inventory(roots)) {
            write_file(player_path, player_before);
            write_file(items_path, items_before);
            write_file(equips_path, equips_before);
            refresh_local_chest_inventory(roots);
            finish_chest_purchase_journal();
            return fail("Could not commit chest-key debit; restored saved inventory");
        }
    } else if (!grant_local_currency_reward(0, 0, roots, -cost)) return false;
    if (!pay_with_item) {
        // Write full item rewards below after the currency debit.
    }
    if (!write_file(items_path, items_after) || !write_file(equips_path, equips_after)) {
        if (pay_with_item) refresh_local_chest_inventory(roots);
        else grant_local_currency_reward(0, 0, roots, cost);
        write_file(player_path, player_before);
        write_file(items_path, items_before);
        write_file(equips_path, equips_before);
        refresh_local_chest_inventory(roots);
        finish_chest_purchase_journal();
        return fail("Could not commit chest rewards; restored saved balance and inventory");
    }
    void* callback_args[] = {response};
    if (!invoke(on_bought, shop, callback_args, "ShopManager.OnBuyTreasureChestSucceed local purchase") ||
        !refresh_local_chest_inventory(roots)) {
        if (pay_with_item) refresh_local_chest_inventory(roots);
        else grant_local_currency_reward(0, 0, roots, cost);
        write_file(player_path, player_before);
        write_file(items_path, items_before);
        write_file(equips_path, equips_before);
        refresh_local_chest_inventory(roots);
        finish_chest_purchase_journal();
        return fail("Original chest reward/UI callback failed; rolled back local purchase");
    }
    finish_chest_purchase_journal();
    if (pay_with_item)
        LOGI("Local chest purchase committed route=item chest=%d item=%d count=%d->%d contentIds=%s",
             chest_id, open_item_id, payment_item_balance, payment_item_balance - payment_item_cost, content_json.c_str());
    else
        LOGI("Local chest purchase committed route=currency chest=%d gems=%d->%d contentIds=%s", chest_id, diamonds,
             diamonds - cost, content_json.c_str());
    return true;
}

void local_buy_treasure_chest_hook(Obj shop, int32_t chest_id, bool free, Obj effect_point, Method original_method) {
    if (equip_compound_in_progress.load()) { LOGE("Chest purchase refused during fusion transaction"); return; }
    if (chest_purchase_in_progress.exchange(true)) {
        LOGE("Concurrent/reentrant chest purchase refused chest=%d", chest_id);
        return;
    }
    const auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
    const int64_t previous_ms = last_chest_purchase_ms.load();
    if (last_chest_purchase_id.load() == chest_id && previous_ms > 0 &&
        now_ms >= previous_ms && now_ms - previous_ms < 650) {
        chest_purchase_in_progress.store(false);
        LOGI("Duplicate chest tap suppressed chest=%d delta_ms=%lld", chest_id,
             static_cast<long long>(now_ms - previous_ms));
        return;
    }
    last_chest_purchase_id.store(chest_id);
    last_chest_purchase_ms.store(now_ms);
    Roots roots;
    local_chest_purchase(shop, chest_id, free, effect_point, original_method, chest_payment_context, roots);
    chest_purchase_in_progress.store(false);
}

Obj local_talent_upgrade_hook(int32_t talent_id, Method /*original_method*/) {
    Roots roots;
    if (!talent_upgrade_task.create || !talent_upgrade_task.set_result)
        return nullptr;
    if (talent_upgrade_in_progress.exchange(true)) {
        LOGE("Concurrent local talent purchase refused");
        return complete_local_task(talent_upgrade_task, nullptr, false, roots);
    }
    auto finish = [&](bool success) -> Obj {
        talent_upgrade_in_progress.store(false);
        return complete_local_task(talent_upgrade_task, nullptr, success, roots);
    };
    if (!reconcile_pending_talent_purchase()) return finish(false);
    Klass cfg_class = find_class("", "TalentCfg");
    Klass talent_class = find_class("", "TalentManager");
    Klass item_class = find_class("", "ItemManager");
    Klass response_class = find_class("ET", "G2C_TalentUpgrade");
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Method get_cfg = cfg_class ? find_method(cfg_class, "GetCfg", {"System.Int32"}) : nullptr;
    Method get_level = talent_class ? find_method(talent_class, "get_CurUpgradeNum", {}) : nullptr;
    Method get_extra_level = talent_class ? find_method(talent_class, "get_CurExtraUpgradeNum", {}) : nullptr;
    Method on_upgrade = talent_class ? find_method(talent_class, "OnTalentDataResp", {"ET.G2C_TalentUpgrade"}) : nullptr;
    Method on_data = talent_class ? find_method(talent_class, "OnTalentDataResp", {"ET.G2C_TalentData"}) : nullptr;
    Method deserialize_method = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    Obj talent_manager = talent_class ? singleton(talent_class, roots) : nullptr;
    Obj item_manager = item_class ? singleton(item_class, roots) : nullptr;
    if (!get_cfg || !get_level || !get_extra_level || !on_upgrade || !on_data ||
        !deserialize_method || !talent_manager || !item_manager || !response_class)
        return finish(false);
    void* cfg_args[] = {&talent_id};
    Obj cfg = nullptr;
    if (!invoke(get_cfg, nullptr, cfg_args, "TalentCfg.GetCfg local purchase", &cfg) || !cfg)
        return finish(false);
    int32_t item_id = 0, cost = 0, talent_type = -1, required_player_level = 0;
    Klass actual_cfg_class = api.object_get_class(cfg);
    Field item_field = api.class_get_field_from_name(actual_cfg_class, "item");
    Field cost_field = api.class_get_field_from_name(actual_cfg_class, "item_num");
    Field type_field = api.class_get_field_from_name(actual_cfg_class, "type");
    Field level_limit_field = api.class_get_field_from_name(actual_cfg_class, "level_limit");
    if (!item_field || !cost_field || !type_field || !level_limit_field) return finish(false);
    api.field_get_value(cfg, item_field, &item_id);
    api.field_get_value(cfg, cost_field, &cost);
    api.field_get_value(cfg, type_field, &talent_type);
    api.field_get_value(cfg, level_limit_field, &required_player_level);
    const int currency_id = talent_type == 0 ? 101 : (talent_type == 1 ? 40001 : 0);
    if (currency_id == 0 || item_id != currency_id || cost <= 0 || required_player_level <= 0) {
        LOGE("Unsupported local talent cost id=%d currency=%d amount=%d", talent_id, item_id, cost);
        return finish(false);
    }
    std::string player_json = load_seed("seed_player.json");
    std::string talent_json = load_seed("seed_talents.json");
    std::string items_json = currency_id == 40001 ? load_seed("seed_items.json") : std::string();
    int64_t saved_gold = -1, saved_level = -1, extra_level = -1, player_level = -1, saved_crystals = -1;
    if (player_json.empty() || talent_json.empty() ||
        !json_read_int(player_json, "Coin", saved_gold) ||
        !json_read_int(player_json, "Level", player_level) ||
        !json_read_int(talent_json, "TalentLevel", saved_level) ||
        !json_read_int(talent_json, "ExtraTalentLevel", extra_level) ||
        saved_gold < 0 || saved_level < 0 || saved_level >= INT32_MAX || extra_level < 0 ||
        player_level < required_player_level ||
        (currency_id == 40001 && (items_json.empty() || !json_get_item_count(items_json, 40001, saved_crystals))))
        return finish(false);
    const int64_t saved_balance = currency_id == 101 ? saved_gold : saved_crystals;
    int live_balance = -1;
    if (!read_original_item_count(item_manager, item_class, currency_id, live_balance, roots) || live_balance != saved_balance)
        return finish(false);
    Obj boxed_level = nullptr;
    Method selected_level_getter = talent_type == 0 ? get_level : get_extra_level;
    if (!invoke(selected_level_getter, talent_manager, nullptr,
                talent_type == 0 ? "TalentManager.get_CurUpgradeNum local purchase" :
                                   "TalentManager.get_CurExtraUpgradeNum local purchase", &boxed_level) ||
        !boxed_level)
        return finish(false);
    roots.retain(boxed_level);
    void* level_value = api.object_unbox(boxed_level);
    if (!level_value) return finish(false);
    int32_t live_level = *static_cast<int32_t*>(level_value);
    const int64_t expected_level = talent_type == 0 ? saved_level : extra_level;
    if (live_level != expected_level || live_balance < cost ||
        (talent_type == 1 && talent_id != 2001 + extra_level)) {
        LOGI("Local talent purchase refused id=%d currency=%d balance=%d cost=%d expectedLevel=%lld liveLevel=%d",
             talent_id, currency_id, live_balance, cost, static_cast<long long>(expected_level), live_level);
        return finish(false);
    }
    std::string response_json = "{\"RpcId\":0,\"Error\":0,\"Message\":\"offline\",\"TalentLevel\":" +
        std::to_string(saved_level + (talent_type == 0 ? 1 : 0)) + ",\"ExtraTalentLevel\":" +
        std::to_string(extra_level + (talent_type == 1 ? 1 : 0)) + "}";
    Obj response_text = roots.retain(api.string_new(response_json.c_str()));
    Obj response_type = roots.retain(api.type_get_object(api.class_get_type(response_class)));
    void* response_args[] = {response_text, response_type};
    Obj response = nullptr;
    if (!response_text || !response_type ||
        !invoke(deserialize_method, nullptr, response_args, "Deserialize local talent purchase reply", &response) ||
        !response || api.object_get_class(response) != response_class)
        return finish(false);
    roots.retain(response);
    const int64_t balance_after = saved_balance - cost;
    const int64_t talent_before = talent_type == 0 ? saved_level : extra_level;
    const int64_t talent_after = talent_before + 1;
    const std::string journal = "{\"TalentId\":" + std::to_string(talent_id) +
        ",\"CurrencyId\":" + std::to_string(currency_id) +
        ",\"CurrencyBefore\":" + std::to_string(saved_balance) +
        ",\"CurrencyAfter\":" + std::to_string(balance_after) +
        ",\"TalentKind\":" + std::to_string(talent_type) +
        ",\"TalentBefore\":" + std::to_string(talent_before) +
        ",\"TalentAfter\":" + std::to_string(talent_after) +
        ",\"Cost\":" + std::to_string(cost) + "}\n";
    if (!write_file(talent_purchase_journal_path(), journal))
        return finish(false);
    bool currency_committed = false;
    if (currency_id == 101) {
        currency_committed = grant_local_currency_reward(-cost, 0, roots);
    } else {
        std::string items_after = items_json;
        currency_committed = json_add_item(items_after, "ItemsInfo", 40001, -cost, "ItemId", "Count") &&
            write_file(files_dir + "/offline_save/seed_items.json", items_after) &&
            refresh_local_chest_inventory(roots);
    }
    if (!currency_committed) {
        // The currency helper rolls back its live setters if persistence fails.
        if (currency_id == 40001) {
            write_file(files_dir + "/offline_save/seed_items.json", items_json);
            refresh_local_chest_inventory(roots);
        }
        // Keep the journal only if the saved values did change unexpectedly.
        reconcile_pending_talent_purchase();
        return finish(false);
    }
    void* upgrade_args[] = {response};
    if (!invoke(on_upgrade, talent_manager, upgrade_args, "TalentManager.OnTalentDataResp local purchase")) {
        bool refunded = currency_id == 101 ? grant_local_currency_reward(cost, 0, roots) :
            (write_file(files_dir + "/offline_save/seed_items.json", items_json) && refresh_local_chest_inventory(roots));
        if (refunded) reconcile_pending_talent_purchase();
        else LOGE("Talent callback failed and currency rollback failed; preserving purchase journal for bootstrap");
        return finish(false);
    }
    if (!json_set_int(talent_json, talent_type == 0 ? "TalentLevel" : "ExtraTalentLevel", talent_after) ||
        !write_file(files_dir + "/offline_save/seed_talents.json", talent_json)) {
        Klass data_class = find_class("ET", "G2C_TalentData");
        Obj old_text = roots.retain(api.string_new(load_seed("seed_talents.json").c_str()));
        Obj old_type = data_class ? roots.retain(api.type_get_object(api.class_get_type(data_class))) : nullptr;
        if (old_text && old_type) {
            void* old_args[] = {old_text, old_type};
            Obj old_response = nullptr;
            if (invoke(deserialize_method, nullptr, old_args, "Deserialize talent rollback data", &old_response) && old_response) {
                roots.retain(old_response);
                void* rollback_args[] = {old_response};
                invoke(on_data, talent_manager, rollback_args, "TalentManager.OnTalentDataResp rollback");
            }
        }
        bool refunded = currency_id == 101 ? grant_local_currency_reward(cost, 0, roots) :
            (write_file(files_dir + "/offline_save/seed_items.json", items_json) && refresh_local_chest_inventory(roots));
        if (refunded) reconcile_pending_talent_purchase();
        else LOGE("Talent save failed and currency rollback failed; preserving purchase journal for bootstrap");
        return finish(false);
    }
    if (unlink(talent_purchase_journal_path().c_str()) != 0)
        LOGE("Completed talent purchase journal cleanup failed; bootstrap will reconcile it");
    LOGI("Local talent purchase complete id=%d currency=%d cost=%d balance=%lld->%lld field=%s level=%lld->%lld",
         talent_id, currency_id, cost, static_cast<long long>(saved_balance), static_cast<long long>(balance_after),
         talent_type == 0 ? "TalentLevel" : "ExtraTalentLevel",
         static_cast<long long>(talent_before), static_cast<long long>(talent_after));
    return finish(true);
}

Obj local_wear_equip_hook(int32_t part, int32_t uid, Method /*original_method*/) {
    Roots roots;
    if (!local_wear_equip_task.create || !local_wear_equip_task.set_result) return nullptr;
    if (equip_compound_in_progress.load())
        return complete_local_task(local_wear_equip_task, nullptr, false, roots);
    if (wear_equip_in_progress.exchange(true)) {
        LOGE("Concurrent/reentrant local equipment wear refused part=%d uid=%d", part, uid);
        return complete_local_task(local_wear_equip_task, nullptr, false, roots);
    }
    const bool success = persist_local_equipment_wear(part, uid, roots);
    wear_equip_in_progress.store(false);
    return complete_local_task(local_wear_equip_task, nullptr, success, roots);
}

Obj local_equip_upgrade_hook(int32_t uid, Method /*original_method*/) {
    Roots roots;
    if (!local_equip_upgrade_task.create || !local_equip_upgrade_task.set_result) return nullptr;
    if (equip_upgrade_in_progress.exchange(true)) {
        LOGE("Concurrent/reentrant local equipment upgrade refused uid=%d", uid);
        return complete_local_task(local_equip_upgrade_task, nullptr, false, roots);
    }
    const bool success = apply_local_equip_upgrade(uid, false, roots);
    equip_upgrade_in_progress.store(false);
    return complete_local_task(local_equip_upgrade_task, nullptr, success, roots);
}

Obj local_equip_onekey_upgrade_hook(int32_t uid, Method /*original_method*/) {
    Roots roots;
    if (!local_equip_onekey_upgrade_task.create || !local_equip_onekey_upgrade_task.set_result) return nullptr;
    if (equip_upgrade_in_progress.exchange(true)) {
        LOGE("Concurrent/reentrant local one-key equipment upgrade refused uid=%d", uid);
        return complete_local_task(local_equip_onekey_upgrade_task, nullptr, false, roots);
    }
    const bool success = apply_local_equip_upgrade(uid, true, roots);
    equip_upgrade_in_progress.store(false);
    return complete_local_task(local_equip_onekey_upgrade_task, nullptr, success, roots);
}

bool remove_equipment_uid(std::string& json, int uid) {
    std::string record;
    size_t start = 0, end = 0;
    if (uid <= 0 || !find_equipment_record(json, uid, record, &start, &end)) return false;
    const size_t array = json_member_value(json, "EquipsInfo");
    if (array == std::string::npos || start < array || end <= start) return false;
    size_t erase_start = start, erase_end = end;
    size_t after = end;
    while (after < json.size() && (json[after] == ' ' || json[after] == '\t' ||
           json[after] == '\r' || json[after] == '\n')) ++after;
    if (after < json.size() && json[after] == ',') erase_end = after + 1;
    else {
        size_t before = start;
        while (before > array + 1 && (json[before - 1] == ' ' || json[before - 1] == '\t' ||
               json[before - 1] == '\r' || json[before - 1] == '\n')) --before;
        if (before > array + 1 && json[before - 1] == ',') erase_start = before - 1;
    }
    json.erase(erase_start, erase_end - erase_start);
    return true;
}

bool local_compound_equip(int equip_uid, int cost_uid1, int cost_uid2, Roots& roots) {
    if (!reconcile_pending_equip_upgrade()) return false;
    if (equip_upgrade_in_progress.load() || chest_purchase_in_progress.load())
        return fail("Fusion refused while another inventory transaction is active");
    std::string before = load_seed("seed_equipment.json");
    if (before.empty()) return fail("Local equipment save is unavailable for fusion");
    std::unordered_set<int64_t> saved_uids;
    size_t scan = json_member_value(before, "EquipsInfo");
    if (scan == std::string::npos) return fail("Fusion save has no equipment list");
    while ((scan = before.find("\"Uid\"", scan)) != std::string::npos) {
        size_t start = before.rfind('{', scan), end = start == std::string::npos ? start : json_object_end(before, start);
        int64_t uid = 0;
        if (end == std::string::npos || !json_read_int(before.substr(start, end - start), "Uid", uid) ||
            uid <= 0 || uid > INT32_MAX || !saved_uids.insert(uid).second)
            return fail("Invalid or duplicate equipment UID in save; fusion refused before consumption");
        scan = end;
    }
    std::string root_record, cost1_record, cost2_record;
    if (equip_uid <= 0 || cost_uid1 <= 0 || cost_uid2 < 0 || equip_uid == cost_uid1 ||
        equip_uid == cost_uid2 || (cost_uid2 && cost_uid1 == cost_uid2) ||
        !find_equipment_record(before, equip_uid, root_record) ||
        !find_equipment_record(before, cost_uid1, cost1_record) ||
        (cost_uid2 && !find_equipment_record(before, cost_uid2, cost2_record)))
        return fail("Fusion UIDs are missing, duplicated, or no longer in the saved inventory");

    auto record_value = [](const std::string& record, const char* primary, const char* fallback,
                           int64_t& value) {
        return json_read_int(record, primary, value) || (fallback && json_read_int(record, fallback, value));
    };
    int64_t root_id = 0, root_quality = 0, root_level = 0;
    int64_t c1_id = 0, c1_quality = 0, c1_level = 0;
    int64_t c2_id = 0, c2_quality = 0, c2_level = 0;
    bool root_worn = false, c1_worn = false, c2_worn = false;
    if (!record_value(root_record, "EquipId", nullptr, root_id) ||
        !record_value(root_record, "Quality", "EquipQuality", root_quality) ||
        !record_value(root_record, "Level", "EquipLevel", root_level) ||
        !json_read_bool(root_record, "IsWear", root_worn) ||
        !record_value(cost1_record, "EquipId", nullptr, c1_id) ||
        !record_value(cost1_record, "Quality", "EquipQuality", c1_quality) ||
        !record_value(cost1_record, "Level", "EquipLevel", c1_level) ||
        !json_read_bool(cost1_record, "IsWear", c1_worn) ||
        (cost_uid2 && (!record_value(cost2_record, "EquipId", nullptr, c2_id) ||
        !record_value(cost2_record, "Quality", "EquipQuality", c2_quality) ||
        !record_value(cost2_record, "Level", "EquipLevel", c2_level) ||
        !json_read_bool(cost2_record, "IsWear", c2_worn))) ||
        root_id <= 0 || c1_id <= 0 || root_quality <= 0 || root_level <= 0 ||
        c1_level <= 0 || (cost_uid2 && (c2_id <= 0 || c2_level <= 0)) ||
        c1_worn != 0 || c2_worn != 0)
        return fail("Fusion items do not satisfy the saved 1.9.1 identity, quality, and unequipped checks");
    if (root_quality >= INT32_MAX) return fail("Fusion would exceed the supported equipment quality");

    Klass equip_manager_class = find_class("", "EquipManager");
    Klass quality_cfg_class = find_class("", "EquipQualityCfg");
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Klass response_class = find_class("ET", "G2C_CompoundEquip");
    Klass equip_data_class = find_class("ET", "G2C_EquipData");
    Method get_equip = equip_manager_class ? find_method(equip_manager_class, "GetEquipInfoByUid", {"System.Int32"}) : nullptr;
    Method can_compound = equip_manager_class ? find_method(equip_manager_class, "EquipCanCompound", {"System.Int32"}) : nullptr;
    Method relate_compound = equip_manager_class ? find_method(equip_manager_class, "EquipRelateCompound", {"System.Int32"}) : nullptr;
    Method rebuild = equip_manager_class ? find_method(equip_manager_class, "UpdateEquipCompoundRedPoint", {}) : nullptr;
    Method is_worn = equip_manager_class ? find_method(equip_manager_class, "CheckEquipIsWear", {"System.Int32"}) : nullptr;
    Method equip_cfg = equip_manager_class ? find_method(equip_manager_class, "GetEquipCfgByUid", {"System.Int32"}) : nullptr;
    Method get_quality_cfg = quality_cfg_class ? find_method(quality_cfg_class, "GetCfg", {"System.Int32"}, true) : nullptr;
    Method deserialize_method = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    Method on_compound = equip_manager_class ? find_method(equip_manager_class, "OnGetCompoundEquip", {"ET.G2C_CompoundEquip"}) : nullptr;
    Method on_equip_data = equip_manager_class ? find_method(equip_manager_class, "OnEquipDataInitResp", {"ET.G2C_EquipData"}) : nullptr;
    Obj equip_manager = equip_manager_class ? singleton(equip_manager_class, roots) : nullptr;
    if (!equip_manager || !get_equip || !can_compound || !relate_compound || !rebuild || !is_worn || !equip_cfg || !get_quality_cfg || !deserialize_method || !response_class ||
        !equip_data_class || !on_compound || !on_equip_data)
        return fail("Cannot resolve exact 1.9.1 fusion validation and original callback APIs");

    auto live_matches = [&](int uid, int64_t id, int64_t quality, int64_t level) -> bool {
        void* args[] = {&uid};
        Obj boxed = nullptr;
        if (!invoke(get_equip, equip_manager, args, "EquipManager.GetEquipInfoByUid local fusion", &boxed) || !boxed)
            return false;
        roots.retain(boxed);
        Klass actual = api.object_get_class(boxed);
        int32_t live_uid = 0, live_id = 0, live_quality = 0, live_level = 0;
        Field f_uid = actual ? api.class_get_field_from_name(actual, "Uid") : nullptr;
        Field f_id = actual ? api.class_get_field_from_name(actual, "Id") : nullptr;
        Field f_quality = actual ? api.class_get_field_from_name(actual, "Quality") : nullptr;
        Field f_level = actual ? api.class_get_field_from_name(actual, "Level") : nullptr;
        if (!f_uid || !f_id || !f_quality || !f_level) return false;
        api.field_get_value(boxed, f_uid, &live_uid);
        api.field_get_value(boxed, f_id, &live_id);
        api.field_get_value(boxed, f_quality, &live_quality);
        api.field_get_value(boxed, f_level, &live_level);
        return live_uid == uid && live_id == id && live_quality == quality && live_level == level;
    };
    if (!live_matches(equip_uid, root_id, root_quality, root_level) ||
        !live_matches(cost_uid1, c1_id, c1_quality, c1_level) ||
        (cost_uid2 && !live_matches(cost_uid2, c2_id, c2_quality, c2_level)))
        return fail("Live EquipManager inventory differs from saved fusion items; no items consumed");

    bool bool_calls_ok = true;
    auto original_bool = [&](Method method, int uid, const char* label) -> bool {
        void* args[] = {&uid};
        Obj boxed = nullptr;
        if (!invoke(method, equip_manager, args, label, &boxed) || !boxed) { bool_calls_ok = false; return false; }
        roots.retain(boxed);
        void* value = api.object_unbox(boxed);
        if (!value) { bool_calls_ok = false; return false; }
        return *static_cast<bool*>(value);
    };
    if (!invoke(rebuild, equip_manager, nullptr, "EquipManager.UpdateEquipCompoundRedPoint fusion preflight")) return false;
    if (original_bool(is_worn, equip_uid, "EquipManager.CheckEquipIsWear fusion root") != root_worn ||
        original_bool(is_worn, cost_uid1, "EquipManager.CheckEquipIsWear fusion material 1") ||
        (cost_uid2 && original_bool(is_worn, cost_uid2, "EquipManager.CheckEquipIsWear fusion material 2")))
        return fail("Saved/live equipment wear state differs or a material is equipped; no consumption");
    if (!original_bool(can_compound, equip_uid, "EquipManager.EquipCanCompound local fusion root") ||
        !original_bool(relate_compound, cost_uid1, "EquipManager.EquipRelateCompound local fusion material 1") ||
        (cost_uid2 && !original_bool(relate_compound, cost_uid2, "EquipManager.EquipRelateCompound local fusion material 2")))
        return fail("Original EquipManager marks this root or one of the selected materials as ineligible");
    if (!bool_calls_ok) return fail("Original fusion eligibility/wear API could not be read; no consumption");

    // CheckCompound and the selector use CURRENT quality.cost=[identityKind,materialQuality,count].
    // kind 1 means same EquipId; kind 2 means same part. The UI passes UID2=0 for one material.
    int32_t current_quality = static_cast<int32_t>(root_quality);
    void* current_args[] = {&current_quality};
    Obj current_cfg = nullptr, costs = nullptr;
    if (!invoke(get_quality_cfg, nullptr, current_args, "EquipQualityCfg.GetCfg fusion recipe", &current_cfg) || !current_cfg)
        return false;
    roots.retain(current_cfg);
    Field cost_field = api.class_get_field_from_name(api.object_get_class(current_cfg), "cost");
    if (!cost_field) return fail("Original quality configuration has no cost recipe");
    api.field_get_value(current_cfg, cost_field, &costs);
    if (!costs) return fail("Original fusion recipe is null");
    roots.retain(costs);
    Method array_value = find_method(api.object_get_class(costs), "GetValue", {"System.Int32"}, true);
    int32_t recipe[3]{};
    for (int32_t i = 0; i < 3; ++i) {
        Obj boxed = nullptr;
        void* args[] = {&i};
        if (!array_value || !invoke(array_value, costs, args, "System.Array.GetValue fusion recipe", &boxed) || !boxed || !api.object_unbox(boxed))
            return fail("Cannot read original fusion recipe through managed array API");
        roots.retain(boxed);
        recipe[i] = *static_cast<int32_t*>(api.object_unbox(boxed));
    }
    auto part_for_uid = [&](int uid, int32_t& part) -> bool {
        Obj cfg = nullptr;
        void* args[] = {&uid};
        if (!invoke(equip_cfg, equip_manager, args, "EquipManager.GetEquipCfgByUid fusion part", &cfg) || !cfg) return false;
        roots.retain(cfg);
        Field field = api.class_get_field_from_name(api.object_get_class(cfg), "part");
        if (!field) return false;
        api.field_get_value(cfg, field, &part);
        return part > 0;
    };
    int32_t root_part = 0, part1 = 0, part2 = 0;
    if ((recipe[0] != 1 && recipe[0] != 2) || recipe[2] != (cost_uid2 ? 2 : 1) ||
        c1_quality != recipe[1] || (cost_uid2 && c2_quality != recipe[1]) ||
        !part_for_uid(equip_uid, root_part) || !part_for_uid(cost_uid1, part1) ||
        (cost_uid2 && !part_for_uid(cost_uid2, part2)) || root_part != part1 ||
        (cost_uid2 && root_part != part2) ||
        (recipe[0] == 1 && (root_id != c1_id || (cost_uid2 && root_id != c2_id))))
        return fail("Selected fusion UIDs do not satisfy the original current-quality cost recipe");
    int64_t saved_part = 0;
    if (!json_read_int(root_record, "Part", saved_part) || saved_part != root_part ||
        !json_read_int(cost1_record, "Part", saved_part) || saved_part != part1 ||
        (cost_uid2 && (!json_read_int(cost2_record, "Part", saved_part) || saved_part != part2)))
        return fail("Saved fusion equipment parts differ from original EquipCfg; no consumption");
    LOGI("Fusion preflight root=%s material1=%s material2=%s recipe=%d,%d,%d",
         root_record.c_str(), cost1_record.c_str(), cost2_record.c_str(), recipe[0], recipe[1], recipe[2]);

    int32_t next_quality = static_cast<int32_t>(root_quality + 1);
    void* quality_args[] = {&next_quality};
    Obj quality_cfg = nullptr;
    if (!invoke(get_quality_cfg, nullptr, quality_args, "EquipQualityCfg.GetCfg local fusion", &quality_cfg) || !quality_cfg)
        return fail("The next equipment quality is absent from the original 1.9.1 quality table");
    roots.retain(quality_cfg);
    int32_t configured_quality = 0;
    Field quality_field = api.class_get_field_from_name(api.object_get_class(quality_cfg), "quality");
    if (!quality_field) return fail("EquipQualityCfg lacks the original quality field");
    api.field_get_value(quality_cfg, quality_field, &configured_quality);
    if (configured_quality != next_quality) return fail("EquipQualityCfg returned an unexpected fusion quality");

    std::string after = before;
    std::string updated_root;
    size_t root_start = 0, root_end = 0;
    if (!find_equipment_record(after, equip_uid, updated_root, &root_start, &root_end) ||
        !json_set_int(updated_root, "Quality", next_quality))
        return fail("Cannot prepare the resulting equipment record");
    after.replace(root_start, root_end - root_start, updated_root);
    if (!remove_equipment_uid(after, cost_uid1) || (cost_uid2 && !remove_equipment_uid(after, cost_uid2)))
        return fail("Cannot prepare removal of both selected fusion materials");

    const std::string root = files_dir + "/offline_save/";
    const std::array<std::pair<const char*, const char*>, 3> snapshots{{
        {"seed_equipment.json", "equip_upgrade_equipment.before"},
        {"seed_items.json", "equip_upgrade_items.before"},
        {"seed_player.json", "equip_upgrade_player.before"}
    }};
    auto rollback_fusion = [&]() {
        if (!restore_equip_upgrade_snapshot()) return false;
        return refresh_local_chest_inventory(roots);
    };

    const std::string response_json = "{\"RpcId\":0,\"Error\":0,\"Message\":\"offline\",\"EquipUid\":" +
        std::to_string(equip_uid) + ",\"CostEquips\":[{\"EquipId\":" + std::to_string(c1_id) +
        ",\"EquipLevel\":" + std::to_string(c1_level) + ",\"EquipQuality\":" + std::to_string(c1_quality) +
        "}" + (cost_uid2 ? ",{\"EquipId\":" + std::to_string(c2_id) + ",\"EquipLevel\":" + std::to_string(c2_level) +
        ",\"EquipQuality\":" + std::to_string(c2_quality) + "}" : "") + "]}";
    Obj text = roots.retain(api.string_new(response_json.c_str()));
    Obj type = roots.retain(api.type_get_object(api.class_get_type(response_class)));
    void* deserialize_args[] = {text, type};
    Obj response = nullptr;
    if (!text || !type || !invoke(deserialize_method, nullptr, deserialize_args,
                                  "Deserialize original G2C_CompoundEquip local response", &response) ||
        !response || api.object_get_class(response) != response_class) {
        return fail("Could not build original 1.9.1 equipment-fusion response; no items consumed");
    }
    roots.retain(response);
    const std::string& final_save = after;
    Obj data_text = roots.retain(api.string_new(final_save.c_str()));
    Obj data_type = roots.retain(api.type_get_object(api.class_get_type(equip_data_class)));
    void* data_args[] = {data_text, data_type};
    Obj data_response = nullptr;
    if (final_save.empty() || !data_text || !data_type ||
        !invoke(deserialize_method, nullptr, data_args, "Deserialize final fused equipment save", &data_response) ||
        !data_response || api.object_get_class(data_response) != equip_data_class) {
        return fail("Could not build final equipment inventory refresh; no items consumed");
    }
    roots.retain(data_response);
    std::string items_before = load_seed("seed_items.json"), player_before = load_seed("seed_player.json");
    if (load_seed("seed_equipment.json") != before || items_before.empty() || player_before.empty())
        return fail("Save changed or unavailable during fusion preflight; no items consumed");
    for (const auto& entry : snapshots) {
        const std::string& current = std::strcmp(entry.first, "seed_equipment.json") == 0 ? before :
            (std::strcmp(entry.first, "seed_items.json") == 0 ? items_before : player_before);
        if (!write_file(root + entry.second, current)) {
            for (const auto& saved : snapshots) unlink((root + saved.second).c_str());
            return fail("Cannot snapshot existing save before equipment fusion");
        }
    }
    if (!write_file(equip_upgrade_txn_path(), "{\"kind\":\"compound\",\"rootUid\":" +
                    std::to_string(equip_uid) + "}\n") || !write_file(root + "seed_equipment.json", after)) {
        rollback_fusion();
        return fail("Could not commit local equipment fusion save; state rolled back");
    }
    void* refresh_args[] = {data_response};
    if (!invoke(on_equip_data, equip_manager, refresh_args, "EquipManager.OnEquipDataInitResp local fusion refresh")) {
        rollback_fusion();
        return fail("Could not refresh original equipment inventory after fusion; saved fusion was rolled back");
    }
    std::string check = load_seed("seed_equipment.json"), missing;
    auto live_missing = [&](int uid) -> bool {
        Obj boxed = nullptr;
        void* args[] = {&uid};
        if (!invoke(get_equip, equip_manager, args, "Fusion consumed UID readback", &boxed) || !boxed) return false;
        roots.retain(boxed);
        Field field = api.class_get_field_from_name(api.object_get_class(boxed), "Uid");
        if (!field) return false;
        int32_t found = uid;
        api.field_get_value(boxed, field, &found);
        return found == 0;
    };
    if (check != after || load_seed("seed_items.json") != items_before || load_seed("seed_player.json") != player_before ||
        !live_matches(equip_uid, root_id, next_quality, root_level) || !live_missing(cost_uid1) ||
        (cost_uid2 && !live_missing(cost_uid2)) ||
        find_equipment_record(check, cost_uid1, missing) ||
        (cost_uid2 && find_equipment_record(check, cost_uid2, missing))) {
        rollback_fusion();
        return fail("Fusion result readback failed; inventory and save rolled back");
    }
    void* callback_args[] = {response};
    if (!invoke(on_compound, equip_manager, callback_args, "EquipManager.OnGetCompoundEquip local fusion") ||
        unlink(equip_upgrade_txn_path().c_str()) != 0) {
        rollback_fusion();
        return fail("Fusion callback or commit failed; inventory and save rolled back");
    }
    for (const auto& entry : snapshots) unlink((root + entry.second).c_str());
    LOGI("Fusion result saved=%s part=%d wear=%d readback=verified", updated_root.c_str(), root_part, root_worn);
    LOGI("Local equipment fusion committed root=%d quality=%lld->%d consumed=%d,%d level=%lld costs=none",
         equip_uid, static_cast<long long>(root_quality), next_quality, cost_uid1, cost_uid2,
         static_cast<long long>(root_level));
    return true;
}

Obj local_equip_compound_hook(int32_t equip_uid, int32_t cost_uid1, int32_t cost_uid2, Method) {
    Roots roots;
    if (!local_equip_compound_task.create || !local_equip_compound_task.set_result) return nullptr;
    if (equip_compound_in_progress.exchange(true)) {
        LOGE("Concurrent/reentrant equipment fusion refused root=%d costs=%d,%d", equip_uid, cost_uid1, cost_uid2);
        return complete_local_task(local_equip_compound_task, nullptr, false, roots);
    }
    const bool success = local_compound_equip(equip_uid, cost_uid1, cost_uid2, roots);
    equip_compound_in_progress.store(false);
    return complete_local_task(local_equip_compound_task, nullptr, success, roots);
}

// Independent collective response and atomic commit. Selection is an explicitly
// authorized local policy, not ProtocolManager's incompatible legacy transaction.
bool sync_fusion_directory(const std::string& directory) {
    const int fd = open(directory.c_str(), O_RDONLY | O_DIRECTORY | O_CLOEXEC);
    if (fd < 0) return false;
    const bool success = fsync(fd) == 0;
    close(fd);
    return success;
}

bool write_fusion_snapshot(const std::string& path, const std::string& data) {
    if (!write_file(path, data)) return false;
    const int fd = open(path.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd < 0) return false;
    const bool success = fsync(fd) == 0;
    close(fd);
    return success && sync_fusion_directory(path.substr(0, path.find_last_of('/')));
}

bool local_onekey_compound(Roots& roots, bool dry_run) {
    if (!dry_run && !reconcile_pending_equip_upgrade()) return false;
    if (equip_upgrade_in_progress.load() || chest_purchase_in_progress.load() || wear_equip_in_progress.load())
        return fail("Collective fusion refused during another inventory transaction");
    std::string before = load_seed("seed_equipment.json");
    const std::string items_before = load_seed("seed_items.json"), player_before = load_seed("seed_player.json");
    if (before.empty() || items_before.empty() || player_before.empty()) return fail("Collective fusion save unavailable");
    std::vector<FusionBatchItem> originals;
    size_t scan = json_member_value(before, "EquipsInfo");
    if (scan == std::string::npos || before[scan] != '[') return fail("Collective fusion equipment array missing");
    for (++scan; scan < before.size();) {
        while (scan < before.size() && std::isspace(static_cast<unsigned char>(before[scan]))) ++scan;
        if (scan < before.size() && before[scan] == ']') break;
        if (scan >= before.size() || before[scan] != '{') return fail("Malformed collective equipment array");
        const size_t end = json_object_end(before, scan);
        if (end == std::string::npos) return false;
        const std::string record = before.substr(scan, end - scan);
        FusionBatchItem item;
        auto read = [&](const char* key, int& value) {
            int64_t wide = 0;
            if (!json_read_int(record, key, wide) || wide <= 0 || wide > INT32_MAX) return false;
            value = static_cast<int>(wide); return true;
        };
        if (!read("Uid", item.uid) || !read("EquipId", item.id) || !read("Quality", item.quality) ||
            !read("Level", item.level) || !read("Part", item.part) || !json_read_bool(record, "IsWear", item.worn))
            return fail("Collective fusion refuses incomplete/noncanonical equipment fields");
        originals.push_back(item);
        scan = end;
        while (scan < before.size() && std::isspace(static_cast<unsigned char>(before[scan]))) ++scan;
        if (scan < before.size() && before[scan] == ',') ++scan;
        else if (scan >= before.size() || before[scan] != ']') return false;
    }
    if (scan >= before.size() || before[scan] != ']') return false;
    auto planned = originals;
    std::vector<FusionBatchStep> steps;
    if (!plan_fusion_batch(planned, steps)) return fail("Collective fusion invalid/duplicate UID or item data");

    Klass manager_class = find_class("", "EquipManager"), quality_class = find_class("", "EquipQualityCfg");
    Klass response_class = find_class("ET", "G2C_OneKeyCompoundEquip"), data_class = find_class("ET", "G2C_EquipData");
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    Obj manager = manager_class ? singleton(manager_class, roots) : nullptr;
    Method get = manager_class ? find_method(manager_class, "GetEquipInfoByUid", {"System.Int32"}) : nullptr;
    Method worn = manager_class ? find_method(manager_class, "CheckEquipIsWear", {"System.Int32"}) : nullptr;
    Method cfg = manager_class ? find_method(manager_class, "GetEquipCfgByIdAndLvAndQuality",
        {"System.Int32", "System.Int32", "System.Int32"}) : nullptr;
    Method quality_cfg = quality_class ? find_method(quality_class, "GetCfg", {"System.Int32"}, true) : nullptr;
    Method rebuild = manager_class ? find_method(manager_class, "UpdateEquipCompoundRedPoint", {}) : nullptr;
    Method can = manager_class ? find_method(manager_class, "EquipCanCompound", {"System.Int32"}) : nullptr;
    Method relate = manager_class ? find_method(manager_class, "EquipRelateCompound", {"System.Int32"}) : nullptr;
    Method list_method = manager_class ? find_method(manager_class, "GetOwnEquips",
        {"System.Boolean", "System.Boolean", "System.Int32", "EquipSortType"}) : nullptr;
    Method on_data = manager_class ? find_method(manager_class, "OnEquipDataInitResp", {"ET.G2C_EquipData"}) : nullptr;
    Method on_success = manager_class ? find_method(manager_class, "OnGetOneKeyCompoundEquip", {"ET.G2C_OneKeyCompoundEquip"}) : nullptr;
    Method json = json_class ? find_method(json_class, "DeserializeObject", {"System.String", "System.Type"}) : nullptr;
    if (!manager || !get || !worn || !cfg || !quality_cfg || !rebuild || !can || !relate || !list_method ||
        !on_data || !on_success || !json || !response_class || !data_class)
        return fail("Collective fusion exact original APIs unresolved");
    auto bool_value = [&](Method method, int uid, bool& value) {
        Obj boxed = nullptr; void* args[] = {&uid};
        if (!invoke(method, manager, args, "Collective fusion original eligibility/wear", &boxed) || !boxed) return false;
        roots.retain(boxed); void* unboxed = api.object_unbox(boxed);
        if (!unboxed) return false;
        value = *static_cast<bool*>(unboxed); return true;
    };
    auto live_matches = [&](const FusionBatchItem& item) {
        int uid = item.uid; Obj boxed = nullptr; void* args[] = {&uid};
        if (!invoke(get, manager, args, "Collective fusion inventory readback", &boxed) || !boxed) return false;
        roots.retain(boxed);
        auto field_value = [&](const char* name, int expected) {
            Field field = api.class_get_field_from_name(api.object_get_class(boxed), name);
            int actual = -1; if (!field) return false;
            api.field_get_value(boxed, field, &actual); return actual == expected;
        };
        if (item.removed) return field_value("Uid", 0);
        bool live_worn = false;
        return field_value("Uid", item.uid) && field_value("Id", item.id) && field_value("Quality", item.quality) &&
            field_value("Level", item.level) && field_value("Part", item.part) &&
            bool_value(worn, item.uid, live_worn) && live_worn == item.worn;
    };
    auto live_count_matches = [&](size_t expected) {
        bool sort = false, without_same = false; int max = -1, sort_type = 1;
        void* args[] = {&sort, &without_same, &max, &sort_type}; Obj list = nullptr;
        if (!invoke(list_method, manager, args, "Collective fusion live inventory count", &list) || !list) return false;
        roots.retain(list); Method count = find_method(api.object_get_class(list), "get_Count", {}, true);
        Obj boxed = nullptr;
        if (!count || !invoke(count, list, nullptr, "Collective fusion List count", &boxed) || !boxed) return false;
        roots.retain(boxed); void* value = api.object_unbox(boxed);
        return value && *static_cast<int*>(value) == static_cast<int>(expected);
    };
    if (!live_count_matches(originals.size())) return fail("Collective fusion saved/live inventory counts differ");
    for (const auto& item : originals) {
        if (!live_matches(item)) return fail("Collective fusion saved/live UID, fields or wear differ");
        int id = item.id, level = item.level, quality = item.quality; Obj config = nullptr;
        void* args[] = {&id, &level, &quality};
        if (!invoke(cfg, manager, args, "Collective fusion original item config", &config) || !config) return false;
        roots.retain(config); int part = 0;
        Field field = api.class_get_field_from_name(api.object_get_class(config), "part");
        if (!field) return false;
        api.field_get_value(config, field, &part);
        if (part != item.part) return fail("Collective fusion saved part differs from original config");
    }
    if (!invoke(rebuild, manager, nullptr, "Collective fusion original cache rebuild")) return false;
    for (int quality = 1; quality <= 4; ++quality) {
        Obj config = nullptr; void* args[] = {&quality};
        if (!invoke(quality_cfg, nullptr, args, "Collective fusion original quality config", &config) || !config) return false;
        roots.retain(config); int actual_quality = 0;
        Field field = api.class_get_field_from_name(api.object_get_class(config), "quality");
        if (!field) return false;
        api.field_get_value(config, field, &actual_quality);
        if (actual_quality != quality) return false;
        if (quality == 4) continue;
        Obj costs = nullptr; field = api.class_get_field_from_name(api.object_get_class(config), "cost");
        if (!field) return false;
        api.field_get_value(config, field, &costs);
        if (!costs) return false;
        roots.retain(costs); Method value = find_method(api.object_get_class(costs), "GetValue", {"System.Int32"}, true);
        const int expected[] = {1, quality, 2};
        for (int index = 0; index < 3; ++index) {
            Obj boxed = nullptr; void* value_args[] = {&index};
            if (!value || !invoke(value, costs, value_args, "Collective fusion original recipe", &boxed) || !boxed) return false;
            roots.retain(boxed); void* unboxed = api.object_unbox(boxed);
            if (!unboxed || *static_cast<int*>(unboxed) != expected[index])
                return fail("Collective fusion configured recipe differs from authorized policy");
        }
    }
    // Initial saved groups must agree with the original caches. Cascade-only
    // groups are planned from the validated recipe; they do not yet exist live.
    for (const auto& item : originals) {
        if (item.worn || item.quality > 3) continue;
        size_t copies = 0;
        for (const auto& other : originals)
            if (!other.worn && other.id == item.id && other.quality == item.quality) ++copies;
        if (copies < 3) continue;
        bool eligible = false, material = false;
        if (!bool_value(can, item.uid, eligible) || !bool_value(relate, item.uid, material) || !eligible || !material)
            return fail("Collective fusion original caches refuse an initial planned group");
    }
    int64_t gold = -1, gems = -1, exp = -1;
    if (!json_read_int(player_before, "Coin", gold) || !json_read_int(player_before, "Diamond", gems) ||
        !json_read_int(player_before, "Exp", exp)) return fail("Collective fusion balance baseline unavailable");
    LOGI("OneKey preflight dryRun=%d policy=offline-triples-max-level-v1 inventory=%zu steps=%zu gold=%lld gems=%lld exp=%lld",
        dry_run, originals.size(), steps.size(), (long long)gold, (long long)gems, (long long)exp);
    for (const auto& item : originals)
        LOGI("OneKey baseline uid=%d id=%d quality=%d level=%d part=%d wear=%d", item.uid, item.id,
            item.quality, item.level, item.part, item.worn);
    for (const auto& step : steps)
        LOGI("OneKey plan root=%d id=%d quality=%d->%d level=%d part=%d consumed=%d(q%d/lv%d),%d(q%d/lv%d) wear=0",
            step.root.uid, step.root.id, step.root.quality, step.root.quality + 1, step.root.level, step.root.part,
            step.materials[0].uid, step.materials[0].quality, step.materials[0].level,
            step.materials[1].uid, step.materials[1].quality, step.materials[1].level);
    if (steps.empty()) return dry_run ? true : fail("Collective fusion has no eligible unequipped triples");
    std::string after = before, result_uids, cost_json;
    size_t remaining = 0, results = 0;
    for (const auto& item : planned) {
        if (item.removed) {
            if (!remove_equipment_uid(after, item.uid)) return false;
            continue;
        }
        ++remaining;
        if (!item.changed) continue;
        size_t start = 0, end = 0; std::string record;
        if (!find_equipment_record(after, item.uid, record, &start, &end) || !json_set_int(record, "Quality", item.quality)) return false;
        after.replace(start, end - start, record);
        if (!result_uids.empty()) result_uids += ",";
        result_uids += std::to_string(item.uid); ++results;
        LOGI("OneKey prepared result=%s dryRun=%d", record.c_str(), dry_run);
    }
    for (const auto& step : steps) for (const auto& material : step.materials) {
        if (!cost_json.empty()) cost_json += ",";
        cost_json += "{\"EquipId\":" + std::to_string(material.id) + ",\"EquipLevel\":" + std::to_string(material.level) +
            ",\"EquipQuality\":" + std::to_string(material.quality) + "}";
    }
    if (remaining + steps.size() * 2 != originals.size() || results == 0) return false;
    auto model = [&](Klass klass, const std::string& payload) -> Obj {
        Obj text = roots.retain(api.string_new(payload.c_str()));
        Obj type = roots.retain(api.type_get_object(api.class_get_type(klass))); Obj response = nullptr;
        void* args[] = {text, type};
        if (!text || !type || !invoke(json, nullptr, args, "Collective fusion prepare original model", &response) ||
            !response || api.object_get_class(response) != klass) return nullptr;
        return roots.retain(response);
    };
    const std::string response_json = "{\"RpcId\":0,\"Error\":0,\"Message\":\"offline\",\"EquipUids\":[" +
        result_uids + "],\"CostEquips\":[" + cost_json + "]}";
    Obj response = model(response_class, response_json);
    Obj data = model(data_class, after);
    if (!response || !data || load_seed("seed_equipment.json") != before ||
        load_seed("seed_items.json") != items_before || load_seed("seed_player.json") != player_before)
        return fail("Collective fusion preparation failed or save changed; no consumption");
    auto list_count = [&](const char* field_name, size_t expected) {
        Field field = api.class_get_field_from_name(response_class, field_name); Obj list = nullptr;
        if (!field) return false;
        api.field_get_value(response, field, &list);
        if (!list) return false;
        roots.retain(list); Method count = find_method(api.object_get_class(list), "get_Count", {}, true);
        Obj boxed = nullptr;
        if (!count || !invoke(count, list, nullptr, "Collective fusion response list contract", &boxed) || !boxed) return false;
        roots.retain(boxed); void* value = api.object_unbox(boxed);
        return value && *static_cast<int*>(value) == static_cast<int>(expected);
    };
    if (!list_count("EquipUids", results) || !list_count("CostEquips", steps.size() * 2))
        return fail("Original collective response list contract mismatch; no consumption");
    LOGI("OneKey models ready dryRun=%d results=%zu costs=%zu inventory=%zu", dry_run, results, steps.size()*2, remaining);
    // Full baseline avoids logd's burst suppression. Keep diagnostic copies in
    // app-private storage, never external/shared storage or inventory seed inputs.
    const std::string diagnostic_dir = files_dir + "/";
    std::string step_json;
    for (const auto& step : steps) {
        if (!step_json.empty()) step_json += ",";
        step_json += "{\"rootUid\":" + std::to_string(step.root.uid) + ",\"qualityBefore\":" +
            std::to_string(step.root.quality) + ",\"qualityAfter\":" + std::to_string(step.root.quality + 1) +
            ",\"materialUids\":[" + std::to_string(step.materials[0].uid) + "," + std::to_string(step.materials[1].uid) + "]}";
    }
    const std::string report = "{\"policy\":\"offline-triples-max-level-v1\",\"dryRun\":" +
        std::string(dry_run ? "true" : "false") + ",\"steps\":[" + step_json + "],\"equipmentBefore\":" + before +
        ",\"itemsBefore\":" + items_before + ",\"playerBefore\":" + player_before +
        ",\"equipmentProjected\":" + after + ",\"responseProjected\":" + response_json + "}";
    const std::string diagnostic_path = diagnostic_dir + (dry_run ? "kinja_onekey_dryrun.json" : "kinja_onekey_preflight.json");
    if (!write_file(diagnostic_path, report)) return fail("Collective fusion full baseline could not be recorded; no consumption");
    LOGI("OneKey diagnostic=%s dryRun=%d bytes=%zu", diagnostic_path.c_str(), dry_run, report.size());
    if (dry_run) return true; // No journal/save write, inventory mutation or success callback.
    const std::string directory = files_dir + "/offline_save/";
    const std::array<std::pair<const char*, const std::string*>, 3> snapshots{{
        {"equip_upgrade_equipment.before", &before}, {"equip_upgrade_items.before", &items_before},
        {"equip_upgrade_player.before", &player_before}}};
    for (const auto& entry : snapshots) {
        if (!write_fusion_snapshot(directory + entry.first, *entry.second)) {
            for (const auto& snapshot : snapshots) unlink((directory + snapshot.first).c_str());
            return fail("Collective fusion snapshot failed; no consumption");
        }
    }
    auto rollback = [&]() {
        const char* filenames[] = {"seed_equipment.json", "seed_items.json", "seed_player.json"};
        bool restored = true;
        for (size_t i = 0; i < snapshots.size(); ++i)
            if (!write_fusion_snapshot(directory + filenames[i], *snapshots[i].second)) restored = false;
        const bool refreshed = restored && refresh_local_chest_inventory(roots);
        // Keep snapshots/journal if memory restoration fails; do not erase the
        // recovery evidence before all original inventory managers are restored.
        if (refreshed && (unlink(equip_upgrade_txn_path().c_str()) == 0 || errno == ENOENT) &&
            sync_fusion_directory(directory))
            for (const auto& entry : snapshots) unlink((directory + entry.first).c_str());
        LOGE("OneKey rollback save=%d memory=%d", restored, refreshed);
        return false;
    };
    if (!write_fusion_snapshot(equip_upgrade_txn_path(), "{\"kind\":\"onekey-compound\",\"policy\":\"offline-triples-max-level-v1\"}\n") ||
        !write_fusion_snapshot(directory + "seed_equipment.json", after)) return rollback();
    void* data_args[] = {data};
    if (!invoke(on_data, manager, data_args, "Collective fusion original inventory refresh") ||
        !live_count_matches(remaining)) return rollback();
    for (const auto& item : planned) if (!live_matches(item)) return rollback();
    if (load_seed("seed_equipment.json") != after || load_seed("seed_items.json") != items_before ||
        load_seed("seed_player.json") != player_before) return rollback();
    void* callback_args[] = {response};
    if (!invoke(on_success, manager, callback_args, "EquipManager.OnGetOneKeyCompoundEquip offline batch") ||
        load_seed("seed_equipment.json") != after || load_seed("seed_items.json") != items_before ||
        load_seed("seed_player.json") != player_before || unlink(equip_upgrade_txn_path().c_str()) != 0 ||
        !sync_fusion_directory(directory)) return rollback();
    for (const auto& entry : snapshots) unlink((directory + entry.first).c_str());
    LOGI("OneKey committed steps=%zu consumed=%zu results=[%s] inventory=%zu costs=none readback=verified",
        steps.size(), steps.size() * 2, result_uids.c_str(), remaining);
    if (!write_file(diagnostic_dir + "kinja_onekey_last_commit.json", "{\"readbackVerified\":true,\"transaction\":" + report + "}"))
        LOGE("OneKey committed successfully but the optional last-commit diagnostic export failed");
    return true;
}

Obj local_equip_onekey_compound_hook(Method) {
    Roots roots;
    if (!local_equip_onekey_compound_task.create || !local_equip_onekey_compound_task.set_result) return nullptr;
    if (equip_compound_in_progress.exchange(true)) {
        LOGE("Concurrent/reentrant collective fusion refused");
        return complete_local_task(local_equip_onekey_compound_task, nullptr, false, roots);
    }
    const bool success = local_onekey_compound(roots, false);
    equip_compound_in_progress.store(false);
    return complete_local_task(local_equip_onekey_compound_task, nullptr, success, roots);
}

void log_fusion_selection_update(Obj self, Method method) {
    if (!original_fusion_selection_update) return;
    original_fusion_selection_update(self, method);
    if (!self) return;
    Roots roots;
    roots.retain(self);
    Field field = api.class_get_field_from_name(api.object_get_class(self), "_selected");
    Obj selected = nullptr;
    if (!field) return;
    api.field_get_value(self, field, &selected);
    if (!selected) return;
    roots.retain(selected);
    Klass klass = api.object_get_class(selected);
    Method count = find_method(klass, "get_Count", {});
    Method item = find_method(klass, "get_Item", {"System.Int32"});
    Obj boxed = nullptr;
    if (!count || !item || !invoke(count, selected, nullptr, "Fusion selected UID count", &boxed) || !boxed || !api.object_unbox(boxed)) return;
    roots.retain(boxed);
    int32_t n = *static_cast<int32_t*>(api.object_unbox(boxed));
    if (n < 0 || n > 3) { LOGE("Unexpected original fusion selection count=%d", n); return; }
    std::string ids = "[";
    for (int32_t i = 0; i < n; ++i) {
        void* args[] = {&i};
        boxed = nullptr;
        if (!invoke(item, selected, args, "Fusion selected UID", &boxed) || !boxed || !api.object_unbox(boxed)) return;
        roots.retain(boxed);
        if (i) ids += ",";
        ids += std::to_string(*static_cast<int32_t*>(api.object_unbox(boxed)));
    }
    LOGI("Fusion UI selected=%s] (root first; observation only)", ids.c_str());
}

bool settle_completed_run_currency(Roots& roots, const char* source) {
    if (!run_success_confirmed.load()) return true;
    const int64_t gold_total = run_gold_payout.load();
    const int64_t gold_credited = run_gold_credited.load();
    const int64_t diamond_total = run_diamond_payout.load();
    const int64_t diamond_credited = run_diamond_credited.load();
    const int64_t gold_delta = gold_total - gold_credited;
    const int64_t diamond_delta = diamond_total - diamond_credited;
    if (gold_delta <= 0 && diamond_delta <= 0) return true;
    if (gold_delta < 0 || diamond_delta < 0 || gold_delta > INT32_MAX || diamond_delta > INT32_MAX ||
        !grant_local_currency_reward(static_cast<int>(gold_delta), 0, roots, static_cast<int>(diamond_delta))) {
        LOGE("Local run currency settlement failed source=%s gold=%lld/%lld diamonds=%lld/%lld",
             source, static_cast<long long>(gold_total), static_cast<long long>(gold_credited),
             static_cast<long long>(diamond_total), static_cast<long long>(diamond_credited));
        return false;
    }
    run_gold_credited.store(gold_total);
    run_diamond_credited.store(diamond_total);
    LOGI("Local run currency settled source=%s gold=%lld delta=%lld diamonds=%lld delta=%lld",
         source, static_cast<long long>(gold_total), static_cast<long long>(gold_delta),
         static_cast<long long>(diamond_total), static_cast<long long>(diamond_delta));
    return true;
}

std::string hangup_items_json(int gold, int exp) {
    std::string items = "[";
    if (gold > 0) items += "{\"ItemId\":101,\"ItemCount\":" + std::to_string(gold) + "}";
    if (gold > 0 && exp > 0) items += ",";
    if (exp > 0) items += "{\"ItemId\":104,\"ItemCount\":" + std::to_string(exp) + "}";
    items += "]";
    return items;
}

std::string hangup_data_json(const std::string& items, const HangUpLocalState& state,
                             int64_t now, int update_period) {
    const int64_t day_start = (now / 86400) * 86400;
    return "{\"RpcId\":0,\"Error\":0,\"Message\":\"\",\"StartTime\":" +
        std::to_string(state.last_claim) + ",\"LastUpdateTime\":" + std::to_string(state.last_claim) +
        ",\"NextUpdateTime\":" + std::to_string(now + update_period) +
        ",\"AdCount\":" + std::to_string(state.quick_return_count) +
        ",\"DiamondCount\":0,\"Items\":" + items +
        ",\"LastRefreshTime\":" + std::to_string(day_start) +
        ",\"PowerCount\":0,\"equips\":[]}";
}

std::string hangup_reward_json(int16_t reward_type, int error, const std::string& message,
                               const std::string& items, const std::string& rewards,
                               const HangUpLocalState& state, int64_t now, int update_period) {
    const int64_t day_start = (now / 86400) * 86400;
    return "{\"RpcId\":0,\"Error\":" + std::to_string(error) +
        ",\"Message\":\"" + escape_json(message) + "\",\"RewardType\":" +
        std::to_string(reward_type) + ",\"StartTime\":" + std::to_string(state.last_claim) +
        ",\"LastUpdateTime\":" + std::to_string(state.last_claim) +
        ",\"NextUpdateTime\":" + std::to_string(now + update_period) +
        ",\"AdCount\":" + std::to_string(state.quick_return_count) +
        ",\"DiamondCount\":0,\"Items\":" + items + ",\"RewardItems\":" + rewards +
        ",\"CostDiamond\":0,\"PowerCount\":0,\"CostPower\":0," 
        "\"RewardEquipsUid\":[],\"ExtraItems\":[],\"LastRefreshTime\":" +
        std::to_string(day_start) + "}";
}

Obj enter_fight_hook(int32_t region_id, int32_t enemy_count, Method) {
    Roots roots;
    active_region.store(region_id);
    run_gold_payout.store(0);
    run_gold_credited.store(0);
    run_diamond_payout.store(0);
    run_diamond_credited.store(0);
    run_success_confirmed.store(false);
    {
        std::lock_guard<std::mutex> lock(run_item_mutex);
        pending_run_items.clear();
        seen_run_drop_instances.clear();
        empty_book_rolls_this_run = 0;
    }
    if (pending_end_response_handle && api.gchandle_free) {
        api.gchandle_free(pending_end_response_handle);
        pending_end_response_handle = 0;
    }
    LOGI("Local EnterFight region=%d enemyCount=%d; caller retains original attack calculation", region_id, enemy_count);
    Obj task = complete_local_task(enter_task, nullptr, true, roots);
    if (!task) status("fight_failed", last_error);
    return task;
}

Obj resurgence_hook(bool is_ad, Method) {
    Roots roots;
    LOGI("FightHelper.Resurgence replaced with local response isAd=%d", is_ad ? 1 : 0);
    Obj response = deserialize(resurgence_task.json_method, resurgence_task.response_class,
                               "seed_resurgence.json", roots);
    if (!response) { status("fight_failed", last_error); return nullptr; }
    // The normal network path dispatches this response through
    // ProtocolManager.HandleResurgence, which calls OnResurgenceSucceed.
    // Returning a completed local task alone skips the gameplay side effect:
    // the original coroutine closes the revive UI, but leaves the hero dead.
    // GameSceneManager belongs to the global namespace in this APK.
    Klass scene_manager = find_class("", "GameSceneManager");
    Method on_resurgence = scene_manager
        ? find_method(scene_manager, "OnResurgenceSucceed", {"ET.G2C_Resurgence"}) : nullptr;
    Obj manager = scene_manager ? singleton(scene_manager, roots) : nullptr;
    if (!on_resurgence || !manager) {
        status("fight_failed", last_error.empty() ? "Cannot resolve GameSceneManager revive handler" : last_error);
        return nullptr;
    }
    void* revive_args[] = {response};
    if (!invoke(on_resurgence, manager, revive_args, "GameSceneManager.OnResurgenceSucceed local revive")) {
        status("fight_failed", last_error);
        return nullptr;
    }
    Obj task = complete_local_task(resurgence_task, response, false, roots);
    if (!task) status("fight_failed", last_error);
    return task;
}

Obj hangup_data_hook(Method) {
    Roots roots;
    const int64_t now = static_cast<int64_t>(std::time(nullptr));
    HangUpLocalState state{};
    int chapter = 0, hour_gold = 0, hour_exp = 0, update_period = 600;
    if (!load_hangup_state(now, state) || !get_hangup_rates(chapter, hour_gold, hour_exp, update_period)) {
        LOGE("Local HangUp accrual unavailable; returning empty response: %s", last_error.c_str());
        Obj response = deserialize(hangup_task.json_method, hangup_task.response_class,
                                   "seed_hangup.json", roots);
        if (!response) { status("offline_feature_failed", last_error); return nullptr; }
        return complete_local_task(hangup_task, response, false, roots);
    }
    const int64_t elapsed = std::max<int64_t>(0, std::min<int64_t>(now - state.last_claim, 6 * 60 * 60));
    const int gold = static_cast<int>((static_cast<int64_t>(hour_gold) * elapsed) / 3600);
    const int exp = static_cast<int>((static_cast<int64_t>(hour_exp) * elapsed) / 3600);
    std::string items = hangup_items_json(gold, exp);
    std::string json = hangup_data_json(items, state, now, update_period);
    LOGI("Local HangUp data chapter=%d elapsed=%llds gold=%d exp=%d quickReturns=%d/3",
         chapter, static_cast<long long>(elapsed), gold, exp, state.quick_return_count);
    Obj response = deserialize_local_payload(hangup_task.json_method, hangup_task.response_class, json, roots);
    if (!response) { status("offline_feature_failed", last_error); return nullptr; }
    Obj task = complete_local_task(hangup_task, response, false, roots);
    if (!task) status("offline_feature_failed", last_error);
    return task;
}

Obj reward_hangup_hook(int16_t reward_type, Method) {
    Roots roots;
    const int64_t now = static_cast<int64_t>(std::time(nullptr));
    HangUpLocalState state{};
    int chapter = 0, hour_gold = 0, hour_exp = 0, update_period = 600;
    bool state_ok = load_hangup_state(now, state);
    bool rates_ok = get_hangup_rates(chapter, hour_gold, hour_exp, update_period);
    int error = 0;
    int gold = 0, exp = 0;
    if (!state_ok || !rates_ok) {
        error = 1;
    } else if (reward_type == 0) {
        const int64_t elapsed = std::max<int64_t>(0, std::min<int64_t>(now - state.last_claim, 6 * 60 * 60));
        gold = static_cast<int>((static_cast<int64_t>(hour_gold) * elapsed) / 3600);
        exp = static_cast<int>((static_cast<int64_t>(hour_exp) * elapsed) / 3600);
        if (gold == 0 && exp == 0) error = 1;
        else state.last_claim = now;
    } else if (reward_type == 1) {
        if (state.quick_return_count >= 3) error = 1;
        else {
            // The original localization calls this a six-hour quick idle reward.
            gold = hour_gold * 6;
            exp = hour_exp * 6;
            state.last_claim = now;
            ++state.quick_return_count;
        }
    } else {
        // Diamond/power quick-return routes have no offline purchase/cost flow.
        error = 1;
    }

    if (!error && !grant_local_currency_reward(gold, exp, roots)) {
        error = 1;
        gold = exp = 0;
    }
    if (!error && !save_hangup_state(state)) {
        error = 1;
        gold = exp = 0;
        fail("Cannot persist HangUp claim state after applying reward; inspect for duplicate risk");
    }
    const std::string items = "[]";
    const std::string rewards = error ? "[]" : hangup_items_json(gold, exp);
    std::string json = hangup_reward_json(reward_type, error, error ?
        "Esta opção não está disponível offline; nenhum custo foi aplicado." : "",
        items, rewards, state, now, update_period);
    LOGI("Local HangUp reward type=%d chapter=%d error=%d gold=%d exp=%d quickReturns=%d/3",
         reward_type, chapter, error, gold, exp, state.quick_return_count);
    Obj response = deserialize_local_payload(reward_hangup_task.json_method,
                                             reward_hangup_task.response_class, json, roots);
    if (!response) { status("offline_feature_failed", last_error); return nullptr; }
    Obj task = complete_local_task(reward_hangup_task, response, false, roots);
    if (!task) status("offline_feature_failed", last_error);
    return task;
}

bool deduct_enemy_hp_hook(Obj self, int32_t enemy_id, int64_t damage, UnityVector3 point,
                          bool is_element_crit, int32_t source_type, SkillTriggerData trigger,
                          Obj hit_resource, int32_t attenuation_type, UnityVector3 euler_angles,
                          int32_t attack_element, Obj enemy_buff_ids, bool force_crit,
                          bool disabled_emit, bool main_attack, bool without_anything, Method method) {
    const int64_t original_damage = damage;
    if (active_region.load() == 3 && damage > 0) damage = damage < 9999 ? 9999 : damage;
    unsigned log_index = outgoing_damage_logs.fetch_add(1);
    if (active_region.load() == 3 && log_index < 8)
        LOGI("Chapter 2 applied hero hit enemy=%d damage=%lld->%lld source=%d",
             enemy_id, static_cast<long long>(original_damage), static_cast<long long>(damage), source_type);
    return original_deduct_enemy_hp(self, enemy_id, damage, point, is_element_crit, source_type,
                                    trigger, hit_resource, attenuation_type, euler_angles,
                                    attack_element, enemy_buff_ids, force_crit, disabled_emit,
                                    main_attack, without_anything, method);
}

void deduct_player_hp_hook(Obj self, int32_t damage, bool force_kill, UnityVector3 damage_pos,
                           bool is_trap, bool show_hp_popup, int32_t entity_id, Method method) {
    const int32_t original_damage = damage;
    if (active_region.load() == 3 && damage > 0 && !force_kill) damage = (damage + 9) / 10;
    unsigned log_index = incoming_damage_logs.fetch_add(1);
    if (active_region.load() == 3 && log_index < 8)
        LOGI("Chapter 2 incoming hit entity=%d damage=%d->%d forceKill=%d trap=%d",
             entity_id, original_damage, damage, force_kill ? 1 : 0, is_trap ? 1 : 0);
    original_deduct_player_hp(self, damage, force_kill, damage_pos, is_trap,
                              show_hp_popup, entity_id, method);
}

Obj kill_enemy_hook(int32_t enemy_id, int32_t enemy_instance_id, bool is_boss, bool is_elite, Method) {
    Roots roots;
    LOGI("Local KillEnemy enemy=%d instance=%d boss=%d elite=%d", enemy_id, enemy_instance_id, is_boss, is_elite);
    Obj response = deserialize(kill_task.json_method, kill_task.response_class, "seed_kill_enemy.json", roots);
    if (!response) { status("fight_failed", last_error); return nullptr; }
    // The original G2C_KillEnemy carries the immediate gold payout. Offline
    // seed replies used zero, so the live battle coin counter could never grow.
    const int32_t gold = is_boss ? 50 : (is_elite ? 25 : 5);
    const int32_t diamonds = gold / 2; // Integer division floors each kill payout independently.
    Method set_gold = find_method(api.object_get_class(response), "set_Gold", {"System.Int32"}, true);
    void* gold_args[] = {const_cast<int32_t*>(&gold)};
    if (!set_gold || !invoke(set_gold, response, gold_args, "G2C_KillEnemy.set_Gold local payout")) {
        status("fight_failed", last_error.empty() ? "Cannot set local kill gold payout" : last_error);
        return nullptr;
    }
    LOGI("Local kill payout enemy=%d gold=%d", enemy_id, gold);
    const int64_t accumulated = run_gold_payout.fetch_add(gold) + gold;
    const int64_t accumulated_diamonds = run_diamond_payout.fetch_add(diamonds) + diamonds;
    LOGI("Local run payout accumulated gold=%lld diamonds=%lld eventDiamonds=%d",
         static_cast<long long>(accumulated), static_cast<long long>(accumulated_diamonds), diamonds);
    if (!collect_local_combat_drop(enemy_id, enemy_instance_id))
        LOGE("Configured item drop lookup failed for enemy=%d: %s", enemy_id, last_error.c_str());
    if (run_success_confirmed.load() && pending_end_response_handle && api.gchandle_get_target) {
        Obj end_response = api.gchandle_get_target(pending_end_response_handle);
        Klass response_class = find_class("ET", "G2C_EndGame");
        Method set_end_gold = response_class ? find_method(response_class, "set_Gold", {"System.Int32"}, true) : nullptr;
        if (end_response && set_end_gold) {
            roots.retain(end_response);
            const int32_t final_gold = static_cast<int32_t>(std::min<int64_t>(accumulated, INT32_MAX));
            const int32_t final_diamonds = static_cast<int32_t>(std::min<int64_t>(accumulated_diamonds, INT32_MAX));
            void* end_gold_args[] = {const_cast<int32_t*>(&final_gold)};
            if (invoke(set_end_gold, end_response, end_gold_args, "G2C_EndGame.set_Gold include late kill")) {
                Method set_end_diamond = find_method(response_class, "set_Diamond", {"System.Int32"}, true);
                void* end_diamond_args[] = {const_cast<int32_t*>(&final_diamonds)};
                if (set_end_diamond && invoke(set_end_diamond, end_response, end_diamond_args,
                                              "G2C_EndGame.set_Diamond include late kill"))
                    LOGI("Local EndGame response updated after late kill gold=%d diamonds=%d",
                         final_gold, final_diamonds);
            }
        } else {
            LOGE("Could not update retained local EndGame response after late kill");
        }
    }
    if (run_success_confirmed.load() && !settle_completed_run_currency(roots, "late_kill_reply"))
        status("fight_failed", last_error.empty() ? "Could not persist late local combat gold" : last_error);
    // Zero attack overrides preserve values already computed by the original caller.
    Obj task = complete_local_task(kill_task, response, false, roots);
    if (!task) status("fight_failed", last_error);
    return task;
}

Obj end_game_hook(int32_t stage, int64_t score, int32_t bud_count, int32_t perfect_count,
                  int32_t skill_count, Method) {
    Roots roots;
    LOGI("Local EndGame stage=%d score=%lld buds=%d perfect=%d skills=%d", stage,
         static_cast<long long>(score), bud_count, perfect_count, skill_count);
    int32_t region = active_region.load();
    bool passed = false, first_win = false, new_score = false, milestone = false, new_chapter = false;
    if (!fight_won(stage, roots, passed)) { status("fight_failed", last_error); return nullptr; }
    run_success_confirmed.store(passed);
    std::vector<std::pair<int, int>> rewards;
    std::vector<EquipReward> equipment_rewards;
    if (passed) {
        std::lock_guard<std::mutex> lock(run_item_mutex);
        rewards.insert(rewards.end(), pending_run_items.begin(), pending_run_items.end());
    }
    if (!persist_local_clear(region, score, passed, first_win, new_score, milestone, new_chapter,
                             rewards, equipment_rewards)) {
        status("fight_failed", last_error); return nullptr;
    }
    int gold_reward = 0, diamond_reward = 0;
    if (passed) {
        const int64_t accumulated = run_gold_payout.load();
        gold_reward = static_cast<int>(std::min<int64_t>(accumulated, INT32_MAX));
        const int64_t accumulated_diamonds = run_diamond_payout.load();
        diamond_reward = static_cast<int>(std::min<int64_t>(accumulated_diamonds, INT32_MAX));
    }
    Obj response = make_end_response(region, passed, new_score, first_win, new_chapter,
                                     gold_reward, diamond_reward, rewards, equipment_rewards, roots);
    if (!response) { status("fight_failed", last_error); return nullptr; }
    if (pending_end_response_handle && api.gchandle_free) api.gchandle_free(pending_end_response_handle);
    pending_end_response_handle = passed ? api.gchandle_new(response, false) : 0;
    if (passed && !pending_end_response_handle)
        LOGE("Could not retain local EndGame response for late kill total update");
    if (!settle_completed_run_currency(roots, "end_game")) {
        status("fight_failed", last_error.empty() ? "Could not persist local combat gold" : last_error);
        return nullptr;
    }
    LOGI("Local EndGame settlement passed=%d gold=%d diamonds=%d exp=0",
         passed ? 1 : 0, gold_reward, diamond_reward);
    if (!apply_local_reward_items(rewards, roots)) { status("fight_failed", last_error); return nullptr; }
    if (passed) {
        std::lock_guard<std::mutex> lock(run_item_mutex);
        pending_run_items.clear();
    }
    if (passed && !refresh_local_regions(roots)) {
        status("fight_failed", last_error.empty() ? "Could not refresh original region manager from local save" : last_error);
        return nullptr;
    }
    // SetOpeningFunc is an original list setter, not a one-shot login task.
    // Refresh after the persisted clear, before the original result returns to
    // the home UI. A menu failure must not discard an already settled victory.
    if (passed && !set_local_opening_functions(end_task.json_method, roots))
        LOGE("Post-clear menu refresh failed; victory remains saved: %s", last_error.c_str());
    write_file(files_dir + "/offline_save/last_fight.json", "{\"region\":" + std::to_string(region) +
               ",\"stage\":" + std::to_string(stage) + ",\"score\":" + std::to_string(score) +
               ",\"passed\":" + (passed ? "true" : "false") + ",\"first_win\":" + (first_win ? "true" : "false") +
               ",\"milestone_100\":" + (milestone ? "true" : "false") + "}\n");
    Obj task = complete_local_task(end_task, response, false, roots);
    if (task) {
        std::string detail = "Original EndGame completed offline; passed=" + std::to_string(passed) +
            " firstWin=" + std::to_string(first_win) + " milestone100=" + std::to_string(milestone) +
            " newChapter=" + std::to_string(new_chapter) + " itemStacks=" + std::to_string(rewards.size()) +
            " equipmentRewards=" + std::to_string(equipment_rewards.size());
        status("fight_reply", detail);
    }
    else status("fight_failed", last_error);
    return task;
}

bool configure_combat() {
    Klass helper = find_class("ET", "FightHelper");
    return helper &&
        configure_task(enter_task, helper, "EnterFight", {"System.Int32", "System.Int32"}, nullptr) &&
        configure_task(kill_task, helper, "KillEnemy", {"System.Int32", "System.Int32", "System.Boolean", "System.Boolean"}, "G2C_KillEnemy") &&
        configure_task(resurgence_task, helper, "Resurgence", {"System.Boolean"}, "G2C_Resurgence") &&
        configure_task(end_task, helper, "EndGame", {"System.Int32", "System.Int64", "System.Int32", "System.Int32", "System.Int32"}, "G2C_EndGame");
}

bool repair_saved_chapter_progress() {
    std::string regions = load_seed("seed_regions.json");
    if (regions.empty()) return false;
    bool region_two_passed = false;
    if (!json_region(regions, 2, 0, false, &region_two_passed, nullptr))
        return fail("Saved local region data is missing region 2");
    if (!region_two_passed) return true;

    int64_t max_progress = 0;
    if (!json_read_int(regions, "MaxRegionProgress", max_progress)) return false;
    bool unlocked = false;
    if (!unlock_next_region(regions, 2, unlocked) || !json_set_int(regions, "MaxRegionProgress", 100))
        return fail("Cannot repair saved chapter unlock after prior clear");
    if (unlocked || max_progress < 100) {
        if (!write_file(files_dir + "/offline_save/seed_regions.json", regions))
            return fail("Cannot persist repaired chapter unlock");
        LOGI("Repaired prior region 2 clear: chapter 2 unlocked; MaxRegion=3 SelectRegion=3");
    }
    return true;
}

bool repair_saved_chapter2_rewards() {
    std::string regions = load_seed("seed_regions.json");
    std::string items = load_seed("seed_items.json");
    std::string equipment = load_seed("seed_equipment.json");
    std::string player = load_seed("seed_player.json");
    if (regions.empty() || items.empty() || equipment.empty() || player.empty()) return false;
    bool chapter2_passed = false;
    if (!json_region(regions, 3, 0, false, &chapter2_passed, nullptr)) return true;
    if (!chapter2_passed) return true;

    // Region 2 is Chapter 1 in this client. Older local saves could advance
    // MaxRegion without persisting its IsPass flag, leaving chapter-gated menus locked.
    if (!json_ensure_region_record(regions, 2))
        return fail("Cannot ensure Chapter 1 progress record for menu unlocks");
    bool chapter1_passed = false;
    if (!json_region(regions, 2, 0, false, &chapter1_passed, nullptr))
        return fail("Cannot read Chapter 1 progress for menu unlocks");
    const bool repaired_chapter1_pass = !chapter1_passed;
    if (repaired_chapter1_pass && !json_region(regions, 2, 0, true, nullptr, nullptr))
        return fail("Cannot repair Chapter 1 pass flag for menu unlocks");

    bool changed = false;
    bool added_chest = false, removed_invalid_equipment = false, promoted_player_level = false;
    changed = repaired_chapter1_pass;
    if (!json_has_item(items, "ItemsInfo", 100002, "ItemId")) {
        if (!json_add_item(items, "ItemsInfo", 100002, 1, "ItemId", "Count"))
            return fail("Cannot repair missing Chapter 2 silver chest");
        changed = added_chest = true;
    }
    // Previous experimental builds inserted this incomplete record. EquipManager
    // requires more fields and crashes during OnEquipDataInitResp when it sees it.
    const EquipReward invalid_experimental_gear{1001, 2, 1};
    if (!json_remove_equipment(equipment, invalid_experimental_gear, removed_invalid_equipment))
        return fail("Cannot remove invalid experimental Chapter 2 equipment record");
    changed = changed || removed_invalid_equipment;
    // FunctionCfg 101-103 require Chapter 1/2 completion and account level 2.
    // The retired server supplied the account EXP/level-up; local clears did not.
    int64_t player_level = 1;
    if (!json_read_int(player, "Level", player_level))
        return fail("Cannot read local account level for Chapter 2 menu unlocks");
    if (player_level < 2) {
        if (!json_set_int(player, "Level", 2))
            return fail("Cannot unlock Chapter 2 menus by advancing local account level");
        changed = promoted_player_level = true;
    }
    if (!changed) return true;
    if (!write_file(files_dir + "/offline_save/seed_items.json", items) ||
        !write_file(files_dir + "/offline_save/seed_equipment.json", equipment) ||
        !write_file(files_dir + "/offline_save/seed_player.json", player) ||
        !write_file(files_dir + "/offline_save/seed_regions.json", regions))
        return fail("Cannot persist repaired Chapter 2 reward data");
    LOGI("Repaired Chapter 2 progression: silverChest=%d removedInvalidExperimentalEquipment=%d accountLevel2=%d chapter1PassFlag=%d",
         added_chest ? 1 : 0, removed_invalid_equipment ? 1 : 0, promoted_player_level ? 1 : 0,
         repaired_chapter1_pass ? 1 : 0);
    return true;
}

bool enable_offline_perks() {
    std::string player = load_seed("seed_player.json");
    if (player.empty()) return false;
    const std::string before = player;
    // PlayerStateType constants in the shipped client: SkipAd="8", InfinitePower="9",
    // AdditionalResurrection="10". Ad callbacks are also short-circuited below.
    if (!json_set_array(player, "PlayerStatus", "[\"8\",\"9\",\"10\"]") ||
        !json_set_array(player, "PlayerStatusTime", "[4102444800,4102444800,4102444800]"))
        return fail("Cannot persist offline SkipAd/InfinitePower/AdditionalResurrection statuses");
    if (player != before && !write_file(files_dir + "/offline_save/seed_player.json", player))
        return fail("Cannot save offline perks to local player profile");
    LOGI("Offline perks enabled: SkipAd=8 InfinitePower=9 AdditionalResurrection=10 until 2100-01-01");
    return true;
}

bool log_offline_perk_statuses(Obj player_manager, Roots& roots) {
    Klass player_class = api.object_get_class(player_manager);
    Method check = find_method(player_class, "CheckPlayerStatusValue", {"System.String"});
    if (!check) return false;
    for (const char* id : {"8", "9", "10"}) {
        Obj key = roots.retain(api.string_new(id));
        if (!key) return false;
        void* args[] = {key};
        Obj boxed = nullptr;
        if (!invoke(check, player_manager, args, "PlayerDataManager.CheckPlayerStatusValue", &boxed) || !boxed)
            return false;
        roots.retain(boxed);
        void* value = api.object_unbox(boxed);
        if (!value) return false;
        LOGI("Offline player status id=%s value=%d", id, *static_cast<int32_t*>(value));
    }
    return true;
}

// The original server sends FuncManager.SetOpeningFunc with the feature IDs
// the account has unlocked. Without that response the home screen keeps its
// navigation locks even though local chapter and level saves satisfy the
// shipped FunctionCfg requirements. Rebuild only the progression-based,
// offline-capable portion of that response from the local save.
bool read_local_menu_progress(std::string& regions, bool& chapter1_passed, bool& chapter2_passed) {
    chapter1_passed = chapter2_passed = false;
    int64_t max_region = 0;
    if (!json_read_int(regions, "MaxRegion", max_region) || max_region < 2 ||
        !json_region(regions, 2, 0, false, &chapter1_passed, nullptr))
        return fail("Cannot read reached Chapter 1 progression for menu unlocks");
    // A new account only has regions 1/2. An unreached Chapter 2 is locked,
    // not a corrupt save; do not create a record or mark anything completed.
    if (!json_has_item(regions, "RegionsInfo", 3, "RegionId")) {
        if (max_region >= 3) return fail("Reached Chapter 2 progress record is missing");
        LOGI("Menu progression: Chapter 2 not reached (MaxRegion=%lld); menus remain locked", (long long)max_region);
        return true;
    }
    return json_region(regions, 3, 0, false, &chapter2_passed, nullptr) ||
        fail("Cannot read existing Chapter 2 progress for menu unlocks");
}

bool set_local_opening_functions(Method json_method, Roots& roots) {
    std::string regions = load_seed("seed_regions.json");
    std::string player = load_seed("seed_player.json");
    int64_t level = 1;
    if (regions.empty() || player.empty() || !json_read_int(player, "Level", level))
        return fail("Cannot read local progression for menu unlocks");

    bool chapter1_passed = false;
    bool chapter2_passed = false;
    if (!read_local_menu_progress(regions, chapter1_passed, chapter2_passed)) return false;

    std::vector<int> opened;
    if (chapter1_passed) opened.push_back(101); // Talent: challenge chapter 1.
    if (level >= 2) { opened.push_back(102); opened.push_back(103); } // Equipment and shop.
    if (chapter2_passed) opened.push_back(116); // Hang-up rewards: pass chapter 2.
    // Server-only and purchase-driven entries (guild, first charge, cards,
    // funds, chapter gifts and hero offers) stay out of the offline menu list.

    std::string json = "[";
    for (size_t i = 0; i < opened.size(); ++i) {
        if (i) json += ",";
        json += std::to_string(opened[i]);
    }
    json += "]";

    Klass manager_class = find_class("", "FuncManager");
    if (!manager_class) return false;
    Method set_opening = nullptr;
    void* iterator = nullptr;
    while (Method candidate = api.class_get_methods(manager_class, &iterator)) {
        if (std::strcmp(api.method_get_name(candidate), "SetOpeningFunc") == 0 &&
            api.method_get_param_count(candidate) == 1) {
            set_opening = candidate;
            break;
        }
    }
    if (!set_opening) return fail("Missing FuncManager.SetOpeningFunc(List<int>)");
    Type list_type = api.method_get_param(set_opening, 0);
    Obj list_reflection_type = list_type ? api.type_get_object(list_type) : nullptr;
    Obj json_text = api.string_new(json.c_str());
    if (!list_reflection_type || !json_text) return fail("Cannot create local opening-function list");
    roots.retain(list_reflection_type);
    roots.retain(json_text);
    void* deserialize_args[] = {json_text, list_reflection_type};
    Obj list = nullptr;
    if (!invoke(json_method, nullptr, deserialize_args, "JsonConvert.DeserializeObject local opening functions", &list) || !list)
        return fail("Cannot deserialize local opening-function list");
    roots.retain(list);
    Obj manager = singleton(manager_class, roots);
    if (!manager) return false;
    void* set_args[] = {list};
    if (!invoke(set_opening, manager, set_args, "FuncManager.SetOpeningFunc local progression")) return false;
    std::string detail = "level=" + std::to_string(level) + " chapter1=" +
        (chapter1_passed ? "passed" : "locked") + " chapter2=" +
        (chapter2_passed ? "passed" : "locked") + " functionIds=" + json;
    LOGI("Local menu progression: %s", detail.c_str());
    return true;
}

void log_fusion_baseline(Obj manager, Roots& roots, const char* stage, bool rebuild_cache) {
    Klass klass = manager ? api.object_get_class(manager) : nullptr;
    Method can = klass ? find_method(klass, "EquipCanCompound", {"System.Int32"}) : nullptr;
    Method relate = klass ? find_method(klass, "EquipRelateCompound", {"System.Int32"}) : nullptr;
    Method get = klass ? find_method(klass, "GetEquipInfoByUid", {"System.Int32"}) : nullptr;
    Method refresh = klass ? find_method(klass, "UpdateEquipCompoundRedPoint", {}) : nullptr;
    if (!manager || !can || !relate || !get || !refresh) { LOGE("Fusion baseline APIs unavailable"); return; }
    if (rebuild_cache && !invoke(refresh, manager, nullptr, "EquipManager.UpdateEquipCompoundRedPoint after local function unlocks")) return;
    std::string equipment = load_seed("seed_equipment.json");
    size_t scan = json_member_value(equipment, "EquipsInfo");
    if (scan == std::string::npos) return;
    while ((scan = equipment.find("\"Uid\"", scan)) != std::string::npos) {
        size_t start = equipment.rfind('{', scan), end = start == std::string::npos ? start : json_object_end(equipment, start);
        if (end == std::string::npos) return;
        std::string record = equipment.substr(start, end - start);
        int64_t saved_uid = 0;
        if (!json_read_int(record, "Uid", saved_uid) || saved_uid <= 0 || saved_uid > INT32_MAX) return;
        int32_t uid = static_cast<int32_t>(saved_uid);
        void* args[] = {&uid};
        Obj can_box = nullptr, relate_box = nullptr, live = nullptr;
        bool ok = invoke(can, manager, args, "Fusion baseline root eligibility", &can_box) && can_box &&
                  invoke(relate, manager, args, "Fusion baseline material eligibility", &relate_box) && relate_box &&
                  invoke(get, manager, args, "Fusion baseline live equipment", &live) && live;
        if (can_box) roots.retain(can_box);
        if (relate_box) roots.retain(relate_box);
        if (live) roots.retain(live);
        int32_t fields[4]{};
        const char* names[] = {"Uid", "Id", "Quality", "Level"};
        for (size_t i = 0; ok && i < 4; ++i) {
            Field field = api.class_get_field_from_name(api.object_get_class(live), names[i]);
            if (!field) { ok = false; break; }
            api.field_get_value(live, field, &fields[i]);
        }
        void* c = can_box ? api.object_unbox(can_box) : nullptr;
        void* r = relate_box ? api.object_unbox(relate_box) : nullptr;
        LOGI("Fusion baseline stage=%s saved=%s liveUid=%d liveId=%d liveQuality=%d liveLevel=%d can=%d relate=%d readable=%d",
             stage, record.c_str(), fields[0], fields[1], fields[2], fields[3], c ? *static_cast<bool*>(c) : -1,
             r ? *static_cast<bool*>(r) : -1, ok);
        scan = end;
    }
    std::string player = load_seed("seed_player.json");
    int64_t gold = -1, gems = -1, exp = -1;
    json_read_int(player, "Coin", gold); json_read_int(player, "Diamond", gems); json_read_int(player, "Exp", exp);
    LOGI("Fusion baseline stage=%s savedGold=%lld savedGems=%lld savedExp=%lld count=%zu",
         stage, static_cast<long long>(gold), static_cast<long long>(gems), static_cast<long long>(exp), equipment.size());
}

bool initialize_original_player(Obj pdm) {
    last_error.clear();
    Roots roots;
    roots.retain(pdm);
    Klass player_class = api.object_get_class(pdm);
    if (class_name(player_class) != "PlayerDataManager") return fail("Hook receiver is not PlayerDataManager");
    for (const char* field_name : {"_allData", "_propertySaveEventDic", "_playerStatusDic", "_intValueDic", "_floatValueDic", "_boolValueDic", "_stringValueDic"}) {
        Field field = api.class_get_field_from_name(player_class, field_name);
        Obj value = nullptr;
        if (field) api.field_get_value(pdm, field, &value);
        LOGI("PDM field %s offset=0x%x value=%p", field_name, field ? api.field_get_offset(field) : -1, value);
    }
    Klass json_class = find_class("Newtonsoft.Json", "JsonConvert");
    if (!json_class) return false;
    if (!reconcile_pending_talent_purchase()) return false;
    if (!reconcile_pending_equip_upgrade()) return false;
    if (!recover_interrupted_chest_purchase()) return false;
    if (!repair_saved_chapter_progress()) return false;
    // Persist missed clear rewards before deserializing the normal seed models.
    if (!repair_saved_chapter2_rewards()) return false;
    // Normalize existing earned EXP before creating the original player model.
    // Idempotent: retain only the unspent balance and never demote legacy levels.
    if (!repair_saved_account_progress(roots)) return false;
    if (!enable_offline_perks()) return false;
#ifdef KINJA_OFFLINE_QA_MAX_PROFILE
    if (!apply_qa_max_profile()) return fail("Could not prepare the authorized QA resource profile");
#endif
    // Required exact overload. The two-parameter (string, JsonSerializerSettings) overload is NOT accepted.
    Method json_method = find_method(json_class, "DeserializeObject", {"System.String", "System.Type"});
    if (!json_method) return false;

    std::array<Delivery, 8> deliveries{{
        {"G2C_ItemInfo", "seed_items.json", "ItemManager", "OnItemInit"},
        {"G2C_EquipData", "seed_equipment.json", "EquipManager", "OnEquipDataInitResp"},
        {"G2C_TalentData", "seed_talents.json", "TalentManager", "OnTalentDataResp"},
        {"G2C_HeroInfo", "seed_heroes.json", "HeroManager", "HeroDataInit"},
        {"G2C_RegionData", "seed_regions.json", "SceneDataManager", "OnGetSceneProgress"},
        {"G2C_PlayerVariable", "seed_guide.json", "PlayerGuideManager", "OnGetGuideData"},
        {"G2C_PlayerData", "seed_player.json", "PlayerDataManager", "OnPlayerValueDataResp"},
        {"G2C_ShopChestState", "seed_shop_chests.json", "ShopManager", "OnGetChestState"}
    }};

    // Resolve every exact class/method and deserialize every payload before the first state mutation.
    for (auto& delivery : deliveries) {
        delivery.model_class = find_class("ET", delivery.model);
        delivery.manager_class = std::strcmp(delivery.manager, "PlayerDataManager") == 0 ? player_class : find_class("", delivery.manager);
        if (!delivery.model_class || !delivery.manager_class) return false;
        delivery.callback_method = find_method(delivery.manager_class, delivery.callback, {std::string("ET.") + delivery.model});
        if (!delivery.callback_method) return false;
        delivery.message = deserialize(json_method, delivery.model_class, delivery.filename, roots);
        if (!delivery.message) return false;
        delivery.receiver = std::strcmp(delivery.manager, "PlayerDataManager") == 0 ? pdm : singleton(delivery.manager_class, roots);
        if (!delivery.receiver) return false;
    }
    Method set_uid = find_method(player_class, "set_Uid", {"System.Int32"});
    Method set_player_uid = find_method(player_class, "set_PlayerUid", {"System.Int32"});
    Method complete_task = find_method(player_class, "CompleteLoginTask", {});
    if (!set_uid || !set_player_uid || !complete_task) return false;

    // Set a real local PID using the original ObscuredLong conversion rather than its raw representation.
    Klass protocol_class = find_class("", "ProtocolManager");
    Klass obscured_long = find_class("CodeStage.AntiCheat.ObscuredTypes", "ObscuredLong");
    if (!protocol_class || !obscured_long) return false;
    Obj protocol = singleton(protocol_class, roots);
    Method implicit_long = find_method(obscured_long, "op_Implicit", {"System.Int64"});
    Method update_time = find_method(protocol_class, "UpdateServerTime", {"System.Double"});
    Field player_id = api.class_get_field_from_name(protocol_class, "_playerId");
    if (!protocol || !implicit_long || !update_time || !player_id) return fail("Missing original PID/time API");
    if (type_name(api.field_get_type(player_id)) != "CodeStage.AntiCheat.ObscuredTypes.ObscuredLong" ||
        type_name(api.method_get_return_type(implicit_long)) != "CodeStage.AntiCheat.ObscuredTypes.ObscuredLong")
        return fail("Original _playerId field/conversion type mismatch");

    int uid = 1;
    int64_t pid = 1;
    void* int_args[] = {&uid};
    void* long_args[] = {&pid};
    Obj boxed_pid = nullptr;
    if (!invoke(implicit_long, nullptr, long_args, "ObscuredLong.op_Implicit local PID", &boxed_pid) || !boxed_pid)
        return fail("Cannot create original ObscuredLong local PID");
    roots.retain(boxed_pid);
    api.field_set_value(protocol, player_id, api.object_unbox(boxed_pid));
    if (!invoke(set_uid, pdm, int_args, "PlayerDataManager.set_Uid") ||
        !invoke(set_player_uid, pdm, int_args, "PlayerDataManager.set_PlayerUid")) return false;
    double now_seconds = static_cast<double>(std::time(nullptr));
    void* time_args[] = {&now_seconds};
    if (!invoke(update_time, protocol, time_args, "ProtocolManager.UpdateServerTime local clock")) return false;

    for (auto& delivery : deliveries) {
        void* args[] = {delivery.message};
        if (!invoke(delivery.callback_method, delivery.receiver, args,
                    std::string(delivery.manager) + "." + delivery.callback)) return false;
    }
#ifdef KINJA_OFFLINE_QA_FUSION_REPEATS
    // QA-only: clone one existing item that the original 1.9.1 manager marks
    // eligible, then refresh via its original callback on the Unity thread.
    if (!apply_qa_fusion_repeats(deliveries[1].receiver, roots)) return false;
#endif
    log_fusion_baseline(deliveries[1].receiver, roots, "before-function-unlocks", false);
    if (!set_local_opening_functions(json_method, roots)) return false;
    // Original DoUpdateEquipData calculates eligibility only while FuncIsOpen(102).
    // Inventory was loaded earlier; rebuild its caches after the opening-function response.
    log_fusion_baseline(deliveries[1].receiver, roots, "after-function-unlocks", true);
    if (!local_onekey_compound(roots, true)) LOGE("Collective fusion read-only baseline unavailable: %s", last_error.c_str());
    if (!log_offline_perk_statuses(pdm, roots)) return false;
    // OnPlayerValueDataResp completes the data task. This replaces the second account/session task.
    if (!invoke(complete_task, pdm, nullptr, "PlayerDataManager.CompleteLoginTask local session")) return false;
    return true;
}

void account_init_hook(Obj pdm, Method /*original_method*/) {
    if (initializing.exchange(true)) { LOGE("Recursive/concurrent AccountInit blocked"); return; }
    status("initializing", "Original AccountInit redirected to local response models on Unity thread");
    bool ok = pdm && initialize_original_player(pdm);
    if (ok) status("initialized", "Original player callbacks completed; gameplay still requires runtime validation");
    else status("failed", last_error.empty() ? "Null PlayerDataManager receiver" : last_error);
    initializing.store(false);
}

// The home screen prewarms the optional online union/guild panel. Its asset
// expects live union service state and aborts the whole warmup when offline.
// Keep the original UI prefab and all solo panels; skip only that online setup.
void skip_union_online_panel_awake(Obj /*self*/, Method /*original_method*/) {
    LOGI("Skipped online-only UIUnionPanel.DontHaveUnionPartAwake in offline mode");
}
void skip_union_online_panel_refresh(Obj /*self*/, Method /*original_method*/) {
    LOGI("Skipped online-only UIUnionPanel refresh in offline mode");
}

// Rewarded videos have no offline equivalent. Resolve their original callback
// immediately as Complete, so revive and other optional ad rewards work locally.
void complete_offline_ad_hook(Obj /*self*/, Obj /*ad_position*/, Obj callback,
                              int32_t /*chapter*/, Method /*original_method*/) {
    if (!callback) { LOGI("Skipped rewarded ad without a callback"); return; }
    Roots roots;
    roots.retain(callback);
    Klass callback_class = api.object_get_class(callback);
    Method invoke_callback = find_method(callback_class, "Invoke", {"AdFinishType"});
    if (!invoke_callback) { LOGE("Could not resolve offline rewarded-ad callback: %s", last_error.c_str()); return; }
    int32_t complete = 1; // AdFinishType.Complete
    void* args[] = {&complete};
    if (invoke(invoke_callback, callback, args, "offline rewarded-ad callback Complete"))
        LOGI("Rewarded ad replaced by immediate local completion");
}

// The original page prefab ships six enabled, pre-rendered real-money cards.
// Run its normal update, then deactivate the exact paid-offer section discovered
// in the 1.9.1 UITreasureBoxPage bundle hierarchy. Chest UI is a sibling.
void hide_offline_paid_shop_section(Obj self, Method original_method) {
    if (original_treasure_page_update_diamond)
        original_treasure_page_update_diamond(self, original_method);
    if (!self) { LOGE("Cannot hide paid Shop section: null UITreasureBoxPage"); return; }

    static Method get_transform = nullptr;
    static Method find_child = nullptr;
    static Method get_game_object = nullptr;
    static Method set_active = nullptr;
    Klass component = find_class("UnityEngine", "Component");
    if (!component) { LOGE("Cannot resolve UnityEngine.Component for paid Shop section"); return; }
    if (!get_transform) get_transform = find_method(component, "get_transform", {}, true);
    if (!get_transform) { LOGE("Cannot resolve Component.get_transform for paid Shop section"); return; }

    Roots roots;
    roots.retain(self);
    Obj transform = nullptr;
    if (!invoke(get_transform, self, nullptr, "UITreasureBoxPage.get_transform", &transform) || !transform) return;
    roots.retain(transform);
    Klass transform_class = api.object_get_class(transform);
    if (!find_child) find_child = transform_class ? find_method(transform_class, "Find", {"System.String"}, true) : nullptr;
    if (!find_child) { LOGE("Cannot resolve Transform.Find(String) for paid Shop section"); return; }
    Obj path = roots.retain(api.string_new("obj/ContentPanel/Top/BuyDiamond"));
    if (!path) { LOGE("Cannot allocate the known paid Shop prefab path"); return; }
    void* find_args[] = {path};
    Obj section_transform = nullptr;
    if (!invoke(find_child, transform, find_args, "Find paid Shop prefab section", &section_transform)) return;
    if (!section_transform) { LOGE("Paid Shop prefab section path was not found; leaving UI visible"); return; }
    roots.retain(section_transform);
    if (!get_game_object) get_game_object = find_method(transform_class, "get_gameObject", {}, true);
    if (!get_game_object) { LOGE("Cannot resolve Component.get_gameObject for paid Shop section"); return; }
    Obj section_object = nullptr;
    if (!invoke(get_game_object, section_transform, nullptr, "Paid Shop section.get_gameObject", &section_object) || !section_object) return;
    roots.retain(section_object);
    Klass game_object_class = api.object_get_class(section_object);
    if (!set_active) set_active = game_object_class ? find_method(game_object_class, "SetActive", {"System.Boolean"}) : nullptr;
    if (!set_active) { LOGE("Cannot resolve GameObject.SetActive(Boolean) for paid Shop section"); return; }
    uint8_t inactive = 0;
    void* active_args[] = {&inactive};
    if (invoke(set_active, section_object, active_args, "Hide paid Shop section"))
        LOGI("Hidden prefab BuyDiamond section (six real-money cards); local chest section remains enabled");

    // Keep the gold-offer group visible. Its daily ad claim and both gem
    // exchanges are now handled locally by their version-checked click hooks.
    LOGI("Kept offline gold offers visible; daily claim and gem exchanges have local handlers");
}

// The 1.9.1 prefab includes a configured TenDiamondBox/TreasureBoxItem node
// beside the single-draw boxes. UpdateTreasurePage already populates its ID108
// model; only the prefab node is inactive. Reveal that original node after the
// game's update finishes, then move the following local-gold section and its
// bottom marker down by one card row through Unity's Transform API. Recalculate
// the NGUI scroll bounds so the existing offers remain reachable by scrolling.
void configure_local_ten_draw_chest(Obj self) {
    if (!self) { LOGE("Cannot reveal local ten-draw chest: null UITreasureBoxPage"); return; }

    static Method get_transform = nullptr;
    static Method find_child = nullptr;
    static Method get_game_object = nullptr;
    static Method set_active = nullptr;
    Klass component = find_class("UnityEngine", "Component");
    if (!component) { LOGE("Cannot resolve UnityEngine.Component for ten-draw chest"); return; }
    if (!get_transform) get_transform = find_method(component, "get_transform", {}, true);
    if (!get_transform) { LOGE("Cannot resolve Component.get_transform for ten-draw chest"); return; }

    Roots roots;
    roots.retain(self);
    Obj transform = nullptr;
    if (!invoke(get_transform, self, nullptr, "UITreasureBoxPage.get_transform for ten-draw chest", &transform) || !transform) return;
    roots.retain(transform);
    Klass transform_class = api.object_get_class(transform);
    if (!find_child) find_child = transform_class ? find_method(transform_class, "Find", {"System.String"}, true) : nullptr;
    if (!find_child) { LOGE("Cannot resolve Transform.Find(String) for ten-draw chest"); return; }
    Obj path = roots.retain(api.string_new("obj/ContentPanel/Top/ChestBox/TenDiamondBox"));
    if (!path) { LOGE("Cannot allocate original ten-draw prefab path"); return; }
    void* find_args[] = {path};
    Obj chest_transform = nullptr;
    if (!invoke(find_child, transform, find_args, "Find original TenDiamondBox prefab node", &chest_transform)) return;
    if (!chest_transform) { LOGE("Original TenDiamondBox prefab node not found; leaving Shop unchanged"); return; }
    roots.retain(chest_transform);
    if (!get_game_object) get_game_object = find_method(transform_class, "get_gameObject", {}, true);
    if (!get_game_object) { LOGE("Cannot resolve Component.get_gameObject for ten-draw chest"); return; }
    Obj chest_object = nullptr;
    if (!invoke(get_game_object, chest_transform, nullptr, "TenDiamondBox.get_gameObject", &chest_object) || !chest_object) return;
    roots.retain(chest_object);
    Klass game_object_class = api.object_get_class(chest_object);
    if (!set_active) set_active = game_object_class ? find_method(game_object_class, "SetActive", {"System.Boolean"}) : nullptr;
    if (!set_active) { LOGE("Cannot resolve GameObject.SetActive(Boolean) for ten-draw chest"); return; }
    uint8_t active = 1;
    void* active_args[] = {&active};
    if (invoke(set_active, chest_object, active_args, "Reveal configured TenDiamondBox")) {
        LOGI("Revealed original TenDiamondBox prefab node; original UpdateTreasurePage supplied configured chest ID108");
        // The exact 1.9.1 prefab gives TenDiamondBox a 387-unit height. Add a
        // 33-unit gap by moving the next section down 420 units. These values
        // are positions from the shipped prefab, not object-memory offsets.
        static Method get_local_position = nullptr;
        static Method set_local_position = nullptr;
        if (!get_local_position)
            get_local_position = find_method(transform_class, "get_localPosition", {}, true);
        if (!set_local_position)
            set_local_position = find_method(transform_class, "set_localPosition", {"UnityEngine.Vector3"}, true);
        if (!get_local_position || !set_local_position) {
            LOGE("Cannot resolve Transform localPosition APIs; leaving Shop section geometry unchanged");
            return;
        }
        struct LayoutNode { const char* path; size_t slot; Obj transform; UnityVector3 position; bool move; };
        LayoutNode nodes[] = {
            {"obj/ContentPanel/Top/BuyCoin", 0, nullptr, {}, false},
            {"obj/ContentPanel/Top/imgDownFill", 1, nullptr, {}, false},
        };
        constexpr float kShiftDown = 420.0f;
        constexpr float kTolerance = 2.0f;
        static Obj last_layout_page = nullptr;
        static float last_applied_y[] = {NAN, NAN};
        if (last_layout_page != self) {
            last_layout_page = self;
            last_applied_y[0] = NAN;
            last_applied_y[1] = NAN;
        }
        bool layout_ready = true;
        for (auto& node : nodes) {
            Obj node_path = roots.retain(api.string_new(node.path));
            if (!node_path) { layout_ready = false; break; }
            void* node_find_args[] = {node_path};
            Obj node_transform = nullptr;
            if (!invoke(find_child, transform, node_find_args,
                        std::string("Find ten-draw following layout node ") + node.path,
                        &node_transform) || !node_transform) {
                layout_ready = false;
                break;
            }
            node.transform = roots.retain(node_transform);
            Obj boxed_position = nullptr;
            if (!invoke(get_local_position, node.transform, nullptr,
                        std::string("Transform.get_localPosition ") + node.path,
                        &boxed_position) || !boxed_position) {
                layout_ready = false;
                break;
            }
            roots.retain(boxed_position);
            auto* value = static_cast<UnityVector3*>(api.object_unbox(boxed_position));
            if (!value) { layout_ready = false; break; }
            node.position = *value;
            // Page refresh can change this node's prefab position (for example,
            // BuyCoin is y=-3032 in the asset and y=-790 after the game's own
            // refresh). Reapply only when Unity has changed its local position
            // since our last write; repeated updates at the same position do
            // not accumulate the layout shift.
            node.move = !std::isfinite(last_applied_y[node.slot]) ||
                std::fabs(node.position.y - last_applied_y[node.slot]) > kTolerance;
            LOGI("Ten-draw layout node %s current y=%.2f lastApplied=%.2f move=%d",
                 node.path, node.position.y, last_applied_y[node.slot], node.move ? 1 : 0);
        }
        if (!layout_ready) return;
        for (auto& node : nodes) {
            if (!node.move) continue;
            node.position.y -= kShiftDown;
            void* position_args[] = {&node.position};
            if (!invoke(set_local_position, node.transform, position_args,
                        std::string("Transform.set_localPosition ") + node.path))
                return;
            last_applied_y[node.slot] = node.position.y;
            LOGI("Moved %s down %.0f Unity units for TenDiamondBox: y=%.2f",
                 node.path, kShiftDown, node.position.y);
        }

        Klass scroll_view_class = find_class("", "UIScrollView");
        Method get_component = find_method(component, "GetComponent", {"System.Type"}, true);
        Method update_scrollbars = scroll_view_class
            ? find_method(scroll_view_class, "UpdateScrollbars", {"System.Boolean"}, true) : nullptr;
        if (!scroll_view_class || !get_component || !update_scrollbars) {
            LOGE("Cannot recalculate NGUI bounds after adding ten-draw row");
            return;
        }
        Obj content_path = roots.retain(api.string_new("obj/ContentPanel"));
        if (!content_path) return;
        void* content_find_args[] = {content_path};
        Obj content_transform = nullptr;
        if (!invoke(find_child, transform, content_find_args,
                    "Find Shop ContentPanel for scroll-bound refresh", &content_transform) || !content_transform)
            return;
        roots.retain(content_transform);
        Obj scroll_type = roots.retain(api.type_get_object(api.class_get_type(scroll_view_class)));
        void* component_args[] = {scroll_type};
        Obj scroll_view = nullptr;
        if (!scroll_type || !invoke(get_component, content_transform, component_args,
                                    "ContentPanel.GetComponent UIScrollView after ten-draw layout",
                                    &scroll_view) || !scroll_view) {
            LOGE("ContentPanel UIScrollView missing; ten-draw row is visible but scroll bounds may be stale");
            return;
        }
        roots.retain(scroll_view);
        uint8_t recalculate = 1;
        void* scrollbar_args[] = {&recalculate};
        if (invoke(update_scrollbars, scroll_view, scrollbar_args,
                   "UIScrollView.UpdateScrollbars(true) after ten-draw layout")) {
            // NGUI may clamp/reposition its content while refreshing bounds.
            // Re-read the managed Transform values after that operation and
            // apply the row gap once more if the original layout was restored.
            for (auto& node : nodes) {
                Obj boxed_position = nullptr;
                if (!invoke(get_local_position, node.transform, nullptr,
                            std::string("Verify post-scroll localPosition ") + node.path,
                            &boxed_position) || !boxed_position) return;
                roots.retain(boxed_position);
                auto* value = static_cast<UnityVector3*>(api.object_unbox(boxed_position));
                if (!value) return;
                UnityVector3 corrected = *value;
                if (!std::isfinite(last_applied_y[node.slot]) ||
                    std::fabs(corrected.y - last_applied_y[node.slot]) > kTolerance) {
                    const float before_y = corrected.y;
                    corrected.y -= kShiftDown;
                    void* corrected_args[] = {&corrected};
                    if (!invoke(set_local_position, node.transform, corrected_args,
                                std::string("Reapply ten-draw spacing after scroll refresh ") + node.path))
                        return;
                    last_applied_y[node.slot] = corrected.y;
                    LOGI("Reapplied %s after NGUI bounds refresh: y %.2f -> %.2f",
                         node.path, before_y, corrected.y);
                }
            }
            LOGI("Ten-draw row laid out; moved local gold offers and refreshed Shop scroll bounds");
        }

        // The 1.9.1 ContentPanel clip extends under the fixed resource HUD when
        // scrolled. Reduce only its upper edge by 160 Unity units, preserving the
        // lower edge so the bottom offers remain reachable in the same scroll.
        Klass panel_class = find_class("", "UIPanel");
        Method get_base_clip = panel_class ? find_method(panel_class, "get_baseClipRegion", {}) : nullptr;
        Method set_base_clip = panel_class ? find_method(panel_class, "set_baseClipRegion", {"UnityEngine.Vector4"}) : nullptr;
        Method invalidate_clipping = panel_class ? find_method(panel_class, "InvalidateClipping", {}) : nullptr;
        if (!panel_class || !get_base_clip || !set_base_clip || !invalidate_clipping) {
            LOGE("Cannot resolve UIPanel baseClipRegion APIs; Shop HUD overlap remains");
            return;
        }
        Obj panel_type = roots.retain(api.type_get_object(api.class_get_type(panel_class)));
        Obj panel_box = nullptr;
        void* panel_component_args[] = {panel_type};
        if (!panel_type || !invoke(get_component, content_transform, panel_component_args,
                                   "ContentPanel.GetComponent UIPanel for HUD-safe clip", &panel_box) || !panel_box) {
            LOGE("ContentPanel UIPanel missing; cannot constrain Shop under fixed HUD");
            return;
        }
        roots.retain(panel_box);
        Obj boxed_clip = nullptr;
        if (!invoke(get_base_clip, panel_box, nullptr, "UIPanel.get_baseClipRegion Shop", &boxed_clip) || !boxed_clip)
            return;
        roots.retain(boxed_clip);
        auto* clip_value = static_cast<UnityVector4*>(api.object_unbox(boxed_clip));
        if (!clip_value) return;
        UnityVector4 clip = *clip_value;
        static Obj last_clip_page = nullptr;
        static UnityVector4 last_applied_clip{};
        static bool has_last_clip = false;
        if (last_clip_page != self) { last_clip_page = self; has_last_clip = false; }
        const auto same_clip = [](const UnityVector4& a, const UnityVector4& b) {
            return std::fabs(a.x-b.x)<0.5f && std::fabs(a.y-b.y)<0.5f &&
                   std::fabs(a.z-b.z)<0.5f && std::fabs(a.w-b.w)<0.5f;
        };
        if (!has_last_clip || !same_clip(clip, last_applied_clip)) {
            const float old_top = clip.y + clip.w * 0.5f;
            const float old_bottom = clip.y - clip.w * 0.5f;
            constexpr float kHudClearance = 160.0f;
            const float new_top = old_top - kHudClearance;
            clip.y = (new_top + old_bottom) * 0.5f;
            clip.w = new_top - old_bottom;
            void* clip_args[] = {&clip};
            if (!invoke(set_base_clip, panel_box, clip_args, "UIPanel.set_baseClipRegion Shop HUD-safe clip"))
                return;
            if (!invoke(invalidate_clipping, panel_box, nullptr, "UIPanel.InvalidateClipping Shop HUD-safe clip"))
                return;
            Obj verified_clip_box = nullptr;
            if (!invoke(get_base_clip, panel_box, nullptr, "UIPanel.get_baseClipRegion Shop readback", &verified_clip_box) ||
                !verified_clip_box) return;
            roots.retain(verified_clip_box);
            auto* verified_clip = static_cast<UnityVector4*>(api.object_unbox(verified_clip_box));
            if (!verified_clip || !same_clip(*verified_clip, clip)) {
                fail("UIPanel baseClipRegion readback differs after Shop HUD-safe clip");
                return;
            }
            last_applied_clip = clip;
            has_last_clip = true;
            LOGI("Shop ContentPanel clip top inset %.0f: top %.2f->%.2f bottom %.2f preserved height %.2f",
                 kHudClearance, old_top, new_top, old_bottom, clip.w);
        } else {
            LOGI("Shop ContentPanel HUD-safe clip already applied top=%.2f bottom=%.2f", clip.y + clip.w*0.5f,
                 clip.y - clip.w*0.5f);
        }
        if (invoke(update_scrollbars, scroll_view, scrollbar_args,
                   "UIScrollView.UpdateScrollbars(true) after HUD-safe clip"))
            LOGI("Shop scrollbars refreshed after viewport clip adjustment");
    }
}

void show_local_ten_draw_chest(Obj self, Method original_method) {
    if (original_treasure_page_update)
        original_treasure_page_update(self, original_method);
    configure_local_ten_draw_chest(self);
}

void refresh_local_ten_draw_chest(Obj self, Method original_method) {
    if (original_treasure_page_refresh)
        original_treasure_page_refresh(self, original_method);
    // DoRefreshPage is the original post-update selector in 1.9.1. Running
    // after it keeps our layout after the game's own section positioning.
    configure_local_ten_draw_chest(self);
}

void update_page_local_ten_draw_chest(Obj self, Method original_method) {
    if (original_treasure_page_update_page)
        original_treasure_page_update_page(self, original_method);
    // UpdatePage is the parent page's 1.9.1 sequence for rebuilding all shop
    // sections. Apply the original ten-draw node and spacing after that pass.
    configure_local_ten_draw_chest(self);
}

bool set_infinite_power_cost_indicator(Obj self) {
    if (!self) return fail("Cannot update home power cost indicator: null UIMainPanel");
    static Method get_transform = nullptr;
    static Method find_child = nullptr;
    static Method get_game_object = nullptr;
    static Method set_active = nullptr;
    Klass component = find_class("UnityEngine", "Component");
    if (!component) return false;
    if (!get_transform) get_transform = find_method(component, "get_transform", {}, true);
    if (!get_transform) return false;

    Roots roots;
    roots.retain(self);
    Obj transform = nullptr;
    if (!invoke(get_transform, self, nullptr, "UIMainPanel.get_transform for infinite energy", &transform) || !transform)
        return false;
    roots.retain(transform);
    Klass transform_class = api.object_get_class(transform);
    if (!find_child) find_child = transform_class ? find_method(transform_class, "Find", {"System.String"}, true) : nullptr;
    if (!find_child) return false;
    Obj path = roots.retain(api.string_new(
        "Content/Page/Main/Bottom/LevelHandle/Unclear/btnUnclearFight/imgPower"));
    if (!path) return fail("Cannot allocate the verified home power indicator path");
    void* find_args[] = {path};
    Obj indicator_transform = nullptr;
    if (!invoke(find_child, transform, find_args, "Find home fight power indicator", &indicator_transform)) return false;
    if (!indicator_transform) {
        LOGE("Home fight power indicator path was not found; its prefab value remains visible");
        return false;
    }
    roots.retain(indicator_transform);
    if (!get_game_object) get_game_object = find_method(transform_class, "get_gameObject", {}, true);
    if (!get_game_object) return false;
    Obj indicator = nullptr;
    if (!invoke(get_game_object, indicator_transform, nullptr, "Home power indicator.get_gameObject", &indicator) || !indicator)
        return false;
    roots.retain(indicator);
    Klass game_object_class = api.object_get_class(indicator);
    if (!set_active) set_active = game_object_class ? find_method(game_object_class, "SetActive", {"System.Boolean"}) : nullptr;
    if (!set_active) return false;
    uint8_t inactive = 0;
    void* active_args[] = {&inactive};
    if (!invoke(set_active, indicator, active_args, "Hide home stage power cost")) return false;
    LOGI("Hidden prefab imgPower (static x5); unlimited-power icon remains in the resource bar");
    return true;
}

bool read_player_power(int32_t& power) {
    Roots roots;
    Klass manager_class = find_class("", "PlayerDataManager");
    if (!manager_class) return false;
    Obj manager = singleton(manager_class, roots);
    Method get_power = find_method(manager_class, "get_PlayerPower", {});
    if (!manager || !get_power) return false;
    Obj obscured_power = nullptr;
    if (!invoke(get_power, manager, nullptr, "PlayerDataManager.get_PlayerPower for QA", &obscured_power) || !obscured_power)
        return false;
    roots.retain(obscured_power);
    Klass obscured_int_class = find_class("CodeStage.AntiCheat.ObscuredTypes", "ObscuredInt");
    Method decrypt = obscured_int_class ? find_method(obscured_int_class, "GetDecrypted", {}) : nullptr;
    if (!decrypt) return false;
    Obj boxed_power = nullptr;
    if (!invoke(decrypt, obscured_power, nullptr, "ObscuredInt.GetDecrypted for QA power readback", &boxed_power) || !boxed_power)
        return false;
    void* unboxed_power = api.object_unbox(boxed_power);
    if (!unboxed_power) return false;
    power = *static_cast<int32_t*>(unboxed_power);
    return true;
}

void hide_offline_mail_entries(Obj self) {
    if (!self) return;
    Klass component = find_class("UnityEngine", "Component");
    Klass transform_class = find_class("UnityEngine", "Transform");
    Klass object_class = find_class("UnityEngine", "Object");
    if (!component || !transform_class || !object_class || !api.string_length || !api.string_chars) {
        LOGE("Cannot resolve Unity APIs for hiding offline mail navigation");
        return;
    }
    Method get_transform = find_method(component, "get_transform", {}, true);
    Method get_child = find_method(transform_class, "GetChild", {"System.Int32"}, true);
    Method get_child_count = find_method(transform_class, "get_childCount", {}, true);
    Method get_game_object = find_method(transform_class, "get_gameObject", {}, true);
    Method get_name = find_method(object_class, "get_name", {}, true);
    if (!get_transform || !get_child || !get_child_count || !get_game_object || !get_name) {
        LOGE("Cannot resolve Transform traversal APIs for hiding offline mail navigation");
        return;
    }
    Roots roots;
    roots.retain(self);
    Obj root = nullptr;
    if (!invoke(get_transform, self, nullptr, "UIMainPanel.get_transform for offline mail hide", &root) || !root)
        return;
    roots.retain(root);
    std::vector<Obj> pending{root};
    std::vector<Obj> hidden;
    Method set_active = nullptr;
    while (!pending.empty()) {
        Obj transform = pending.back();
        pending.pop_back();
        roots.retain(transform);
        Obj game_object = nullptr;
        if (!invoke(get_game_object, transform, nullptr, "Transform.get_gameObject offline mail scan", &game_object) || !game_object)
            continue;
        roots.retain(game_object);
        Obj name = nullptr;
        if (invoke(get_name, game_object, nullptr, "GameObject.get_name offline mail scan", &name) && name) {
            roots.retain(name);
            const int32_t length = api.string_length(name);
            const uint16_t* chars = api.string_chars(name);
            std::string ascii;
            if (chars && length > 0 && length < 256) {
                ascii.reserve(static_cast<size_t>(length));
                for (int32_t i = 0; i < length; ++i) {
                    const uint16_t ch = chars[i];
                    ascii.push_back(ch < 128 ? static_cast<char>(ch) : '?');
                }
                std::string lower = ascii;
                std::transform(lower.begin(), lower.end(), lower.begin(),
                    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
                const bool mail_button = lower == "btnmail";
                const bool mail_notice = lower.find("mail") != std::string::npos &&
                    (lower.find("red") != std::string::npos || lower.find("point") != std::string::npos ||
                     lower.find("notice") != std::string::npos || lower.find("badge") != std::string::npos);
                if (mail_button || mail_notice) hidden.push_back(game_object);
            }
        }
        Obj boxed_count = nullptr;
        if (!invoke(get_child_count, transform, nullptr, "Transform.get_childCount offline mail scan", &boxed_count) || !boxed_count)
            continue;
        roots.retain(boxed_count);
        void* count_value = api.object_unbox(boxed_count);
        if (!count_value) continue;
        const int32_t count = *static_cast<int32_t*>(count_value);
        if (count < 0 || count > 512) continue;
        for (int32_t i = 0; i < count; ++i) {
            Obj child = nullptr;
            void* args[] = {&i};
            if (invoke(get_child, transform, args, "Transform.GetChild offline mail scan", &child) && child)
                pending.push_back(child);
        }
    }
    if (hidden.empty()) {
        LOGI("No mail button/red-point nodes found in UIMainPanel hierarchy");
        return;
    }
    Klass game_object_class = find_class("UnityEngine", "GameObject");
    set_active = game_object_class ? find_method(game_object_class, "SetActive", {"System.Boolean"}) : nullptr;
    if (!set_active) {
        LOGE("Cannot resolve GameObject.SetActive for offline mail navigation");
        return;
    }
    uint8_t inactive = 0;
    for (Obj game_object : hidden) {
        void* args[] = {&inactive};
        if (invoke(set_active, game_object, args, "Hide offline mail entry or notification"))
            LOGI("Hidden backend-only mail UI node in UIMainPanel");
    }
}

void center_chapter_button_label(Obj self) {
    if (!self) return;
    Klass component = find_class("UnityEngine", "Component");
    Klass transform_class = find_class("UnityEngine", "Transform");
    Klass label_class = find_class("", "UILabel");
    Klass widget_class = find_class("", "UIWidget");
    Klass rect_class = find_class("", "UIRect");
    Klass game_object_class = find_class("UnityEngine", "GameObject");
    if (!component || !transform_class || !label_class || !widget_class || !rect_class || !game_object_class) return;
    Method get_transform = find_method(component, "get_transform", {}, true);
    Method find_child = find_method(transform_class, "Find", {"System.String"}, true);
    Method get_parent = find_method(transform_class, "get_parent", {}, true);
    Method get_game_object = find_method(component, "get_gameObject", {}, true);
    Method get_component = find_method(component, "GetComponent", {"System.Type"}, true);
    Method set_anchor = find_method(rect_class, "SetAnchor", {
        "UnityEngine.GameObject", "System.Single", "System.Single", "System.Single", "System.Single"
    }, true);
    auto find_setter = [&](Klass klass, const char* name) -> Method {
        for (Klass current = klass; current; current = api.class_get_parent(current)) {
            void* iterator = nullptr;
            while (Method candidate = api.class_get_methods(current, &iterator))
                if (std::strcmp(api.method_get_name(candidate), name) == 0 && api.method_get_param_count(candidate) == 1)
                    return candidate;
        }
        return nullptr;
    };
    Method set_pivot = find_setter(widget_class, "set_pivot");
    Method set_alignment = find_setter(label_class, "set_alignment");
    if (!get_transform || !find_child || !get_parent || !get_game_object || !get_component ||
        !set_anchor || !set_pivot || !set_alignment) return;
    Roots roots;
    roots.retain(self);
    Obj root = nullptr;
    if (!invoke(get_transform, self, nullptr, "UIMainPanel.get_transform for chapter button label", &root) || !root) return;
    roots.retain(root);
    Obj path = roots.retain(api.string_new("Content/Page/Main/Bottom/LevelHandle/Unclear/btnUnclearFight/txtRegionNameInBtn"));
    if (!path) return;
    void* find_args[] = {path};
    Obj text_transform = nullptr;
    if (!invoke(find_child, root, find_args, "Transform.Find exact chapter button label path", &text_transform) || !text_transform) {
        LOGI("Chapter button label path is absent; alignment left unchanged");
        return;
    }
    roots.retain(text_transform);
    Obj label_type = roots.retain(api.type_get_object(api.class_get_type(label_class)));
    void* component_args[] = {label_type};
    Obj label = nullptr;
    if (!label_type || !invoke(get_component, text_transform, component_args, "Transform.GetComponent UILabel chapter button", &label) || !label) {
        LOGE("Chapter button label path exists without UILabel; alignment left unchanged");
        return;
    }
    roots.retain(label);
    Obj button_transform = nullptr;
    if (!invoke(get_parent, text_transform, nullptr, "Transform.get_parent chapter button label", &button_transform) || !button_transform) {
        LOGE("Chapter button label has no parent; anchor alignment left unchanged");
        return;
    }
    roots.retain(button_transform);
    Obj button_game_object = nullptr;
    if (!invoke(get_game_object, button_transform, nullptr, "Transform.get_gameObject chapter button", &button_game_object) || !button_game_object) {
        LOGE("Chapter button parent has no GameObject; anchor alignment left unchanged");
        return;
    }
    roots.retain(button_game_object);
    float anchor_left = 0.0f, anchor_bottom = 0.0f, anchor_right = 1.0f, anchor_top = 1.0f;
    void* anchor_args[] = {button_game_object, &anchor_left, &anchor_bottom, &anchor_right, &anchor_top};
    int32_t center_pivot = 4; // UIWidget.Pivot.Center in exact 1.9.1 metadata.
    int32_t center_alignment = 2; // NGUIText.Alignment.Center in exact 1.9.1 metadata.
    const bool anchor_ok = invoke(set_anchor, label, anchor_args, "UIRect.SetAnchor chapter label to button bounds");
    void* pivot_args[] = {&center_pivot};
    void* alignment_args[] = {&center_alignment};
    const bool pivot_ok = invoke(set_pivot, label, pivot_args, "UIWidget.set_pivot chapter button", nullptr);
    const bool alignment_ok = invoke(set_alignment, label, alignment_args, "UILabel.set_alignment chapter button", nullptr);
    LOGI("Chapter button label alignment applied anchor=%d pivot=%d alignment=%d", anchor_ok, pivot_ok, alignment_ok);
}

void main_panel_do_open_hook(Obj self, Method method) {
    if (original_main_panel_do_open) original_main_panel_do_open(self, method);
    set_infinite_power_cost_indicator(self);
    hide_offline_mail_entries(self);
    center_chapter_button_label(self);
}

void main_panel_on_click_fight_hook(Obj self, Obj go, Method method) {
    int32_t before = 0;
    bool have_before = read_player_power(before);
    if (original_main_panel_on_click_fight) original_main_panel_on_click_fight(self, go, method);
    int32_t after = 0;
    bool have_after = read_player_power(after);
    if (have_before && have_after)
        LOGI("QA energy entry measurement: PlayerPower before=%d after=%d delta=%d", before, after, after - before);
    else
        LOGE("QA energy entry measurement unavailable: PlayerDataManager getter/conversion failed");
}

void block_offline_purchase(Obj /*self*/, int32_t iap_id, Method /*original_method*/) {
    LOGI("Blocked real-money purchase request in offline mode (iapId=%d)", iap_id);
}

void update_shop_daily_claim_ui(Obj page, Method method) {
    Roots roots;
    if (!reconcile_shop_daily_claim(roots)) LOGE("Shop daily claim recovery failed: %s", last_error.c_str());
    const int today = local_calendar_day();
    std::string state;
    int64_t last_date = 0;
    if (today && read_local_text(daily_shop_claim_path(), state)) json_read_int(state, "Date", last_date);
    if (!set_shop_daily_claim_count(last_date == today && today ? 1 : 0, roots))
        LOGE("Could not synchronize local Shop daily claim counter: %s", last_error.c_str());
    if (original_treasure_page_update_coin)
        original_treasure_page_update_coin(page, method);
}

void block_shop_ad_coin(Obj self, Obj /*button*/, Method /*original_method*/) {
    Roots roots;
    static std::atomic<bool> claim_in_progress{false};
    if (claim_in_progress.exchange(true)) {
        LOGI("Ignored reentrant Shop daily claim click");
        return;
    }
    const bool ok = perform_shop_daily_claim(self, roots);
    if (ok) {
        if (original_treasure_page_update_coin) original_treasure_page_update_coin(self, nullptr);
    } else {
        LOGE("Local Shop daily claim failed: %s", last_error.c_str());
    }
    claim_in_progress.store(false);
}

void local_shop_coin_exchange(Obj self, const char* config_key, int gold, int gems) {
    if (shop_coin_exchange_in_progress.exchange(true)) {
        LOGI("Ignored reentrant Shop gem-to-gold exchange click config=%s", config_key);
        return;
    }
    Roots roots;
    roots.retain(self);
    const bool ok = perform_shop_coin_exchange(config_key, gold, gems, roots);
    if (ok) {
        if (original_treasure_page_update_coin)
            original_treasure_page_update_coin(self, nullptr);
        LOGI("Shop exchange UI refresh requested config=%s", config_key);
    } else {
        LOGE("Local Shop gem-to-gold exchange failed config=%s: %s", config_key, last_error.c_str());
    }
    shop_coin_exchange_in_progress.store(false);
}

void block_shop_coin_exchange_lot(Obj self, Obj /*button*/, Method /*original_method*/) {
    local_shop_coin_exchange(self, "diamond_buy_lot_coin", 20000, 100);
}

void block_shop_coin_exchange_huge(Obj self, Obj /*button*/, Method /*original_method*/) {
    local_shop_coin_exchange(self, "diamond_buy_huge_coin", 118888, 500);
}

bool mapping_protection(uintptr_t address, int& protection) {
    std::ifstream maps("/proc/self/maps");
    std::string line;
    while (std::getline(maps, line)) {
        unsigned long long start = 0, end = 0;
        char permissions[5]{};
        if (std::sscanf(line.c_str(), "%llx-%llx %4s", &start, &end, permissions) != 3) continue;
        if (address < start || address + kExpectedEntry.size() > end) continue;
        protection = (permissions[0] == 'r' ? PROT_READ : 0) |
                     (permissions[1] == 'w' ? PROT_WRITE : 0) |
                     (permissions[2] == 'x' ? PROT_EXEC : 0);
        return (protection & PROT_READ) && (protection & PROT_EXEC);
    }
    return false;
}

bool patch_entry(uintptr_t base, uintptr_t method_va, const std::array<uint8_t,16>& expected,
                 const char* name, uintptr_t replacement) {
#if !defined(__aarch64__)
    (void)base; (void)method_va; (void)expected; (void)name; (void)replacement;
    return fail("This adapter only supports the original ARM64 client");
#else
    uintptr_t entry = base + method_va;
    int old_protection = 0;
    if (!mapping_protection(entry, old_protection)) return fail(std::string(name) + " entry is not in readable executable memory");
    if (std::memcmp(reinterpret_cast<void*>(entry), expected.data(), expected.size()) != 0) {
        std::ostringstream bytes;
        const auto* memory = reinterpret_cast<const uint8_t*>(entry);
        const char* hex = "0123456789abcdef";
        for (size_t i = 0; i < expected.size(); ++i) { bytes << hex[memory[i] >> 4] << hex[memory[i] & 15]; }
        return fail(std::string(name) + " runtime prologue mismatch; refusing hook: " + bytes.str());
    }
    const long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) return fail("Cannot determine page size");
    uintptr_t page = entry & ~(static_cast<uintptr_t>(page_size) - 1);
    uintptr_t end = (entry + 16 + page_size - 1) & ~(static_cast<uintptr_t>(page_size) - 1);
    size_t length = end - page;
    if (mprotect(reinterpret_cast<void*>(page), length, old_protection | PROT_WRITE) != 0)
        return fail(std::string("mprotect write permission rejected for ") + name);
    // This is installed before AccountInit executes; no instruction from the entry is relocated.
    const uint32_t branch[2] = {0x58000051, 0xd61f0220}; // LDR x17,#8; BR x17.
    const uint64_t destination = replacement;
    std::memcpy(reinterpret_cast<void*>(entry + 8), &destination, sizeof(destination));
    std::memcpy(reinterpret_cast<void*>(entry), branch, sizeof(branch));
    __builtin___clear_cache(reinterpret_cast<char*>(entry), reinterpret_cast<char*>(entry + 16));
    if (mprotect(reinterpret_cast<void*>(page), length, old_protection) != 0) {
        // Restore original entry if protection restoration fails; never leave a half-installed hook.
        std::memcpy(reinterpret_cast<void*>(entry), expected.data(), expected.size());
        __builtin___clear_cache(reinterpret_cast<char*>(entry), reinterpret_cast<char*>(entry + 16));
        mprotect(reinterpret_cast<void*>(page), length, old_protection);
        return fail("Cannot restore original executable page protection");
    }
    LOGI("Installed %s memory hook at %p (base=%p)", name, reinterpret_cast<void*>(entry), reinterpret_cast<void*>(base));
    return true;
#endif
}

void* make_entry_trampoline(uintptr_t base, uintptr_t method_va,
                            const std::array<uint8_t,16>& expected) {
#if !defined(__aarch64__)
    (void)base; (void)method_va; (void)expected;
    return nullptr;
#else
    const uintptr_t entry = base + method_va;
    const long page_size = sysconf(_SC_PAGESIZE);
    if (page_size <= 0) return nullptr;
    void* trampoline = mmap(nullptr, static_cast<size_t>(page_size), PROT_READ | PROT_WRITE,
                            MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (trampoline == MAP_FAILED) return nullptr;
    auto* code = static_cast<uint8_t*>(trampoline);
    std::memcpy(code, expected.data(), expected.size());
    const uint32_t branch[2] = {0x58000051, 0xd61f0220}; // LDR x17,#8; BR x17.
    const uint64_t resume = entry + expected.size();
    std::memcpy(code + expected.size(), branch, sizeof(branch));
    std::memcpy(code + expected.size() + sizeof(branch), &resume, sizeof(resume));
    __builtin___clear_cache(reinterpret_cast<char*>(code), reinterpret_cast<char*>(code + 32));
    if (mprotect(trampoline, static_cast<size_t>(page_size), PROT_READ | PROT_EXEC) != 0) {
        munmap(trampoline, static_cast<size_t>(page_size));
        return nullptr;
    }
    return trampoline;
#endif
}

bool install_hook(void* library) {
    void* init_export = dlsym(library, "il2cpp_init");
    Dl_info info{};
    if (!init_export || dladdr(init_export, &info) == 0 || !info.dli_fbase ||
        !info.dli_fname || std::string(info.dli_fname).find("libil2cpp.so") == std::string::npos)
        return fail("Cannot resolve original libil2cpp ASLR base from il2cpp_init");
    uintptr_t base = reinterpret_cast<uintptr_t>(info.dli_fbase);
    if (!patch_entry(base, kAccountInitVa, kExpectedEntry, "AccountInit", reinterpret_cast<uintptr_t>(&account_init_hook))) return false;
    const std::array<uint8_t,16> main_panel_do_open = {
        0xfc,0x6f,0xba,0xa9, 0xfa,0x67,0x01,0xa9, 0xf8,0x5f,0x02,0xa9, 0xf6,0x57,0x03,0xa9
    };
    void* main_open_trampoline = make_entry_trampoline(base, 0xfd1bb8, main_panel_do_open);
    if (main_open_trampoline) {
        original_main_panel_do_open = reinterpret_cast<MainPanelDoOpenOriginal>(main_open_trampoline);
        if (!patch_entry(base, 0xfd1bb8, main_panel_do_open, "UIMainPanel.DoOpen",
                         reinterpret_cast<uintptr_t>(&main_panel_do_open_hook)))
            original_main_panel_do_open = nullptr;
    } else LOGE("Could not allocate UIMainPanel.DoOpen trampoline; static power indicator may remain visible");
    const std::array<uint8_t,16> main_panel_fight = {
        0xff,0x83,0x01,0xd1, 0xf7,0x13,0x00,0xf9, 0xf6,0x57,0x03,0xa9, 0xf4,0x4f,0x04,0xa9
    };
    void* main_fight_trampoline = make_entry_trampoline(base, 0xfda1fc, main_panel_fight);
    if (main_fight_trampoline) {
        original_main_panel_on_click_fight = reinterpret_cast<MainPanelOnClickFightOriginal>(main_fight_trampoline);
        if (!patch_entry(base, 0xfda1fc, main_panel_fight, "UIMainPanel.OnClickFight",
                         reinterpret_cast<uintptr_t>(&main_panel_on_click_fight_hook)))
            original_main_panel_on_click_fight = nullptr;
    } else LOGE("Could not allocate UIMainPanel.OnClickFight trampoline; QA energy readback unavailable");
    const std::array<uint8_t,16> union_awake = {0xfa,0x67,0xbb,0xa9,0xf8,0x5f,0x01,0xa9,0xf6,0x57,0x02,0xa9,0xf4,0x4f,0x03,0xa9};
    if (!patch_entry(base, 0x186c74c, union_awake, "UIUnionPanel.DontHaveUnionPartAwake", reinterpret_cast<uintptr_t>(&skip_union_online_panel_awake)))
        LOGE("Could not disable optional online union-panel warmup: %s", last_error.c_str());
    const std::array<uint8_t,16> union_info = {0xff,0x43,0x01,0xd1,0xf5,0x13,0x00,0xf9,0xf4,0x4f,0x03,0xa9,0xfd,0x7b,0x04,0xa9};
    if (!patch_entry(base, 0x1870870, union_info, "UIUnionPanel.InitDontHaveUnionPartInfo", reinterpret_cast<uintptr_t>(&skip_union_online_panel_refresh)))
        LOGE("Could not disable optional online union info: %s", last_error.c_str());
    const std::array<uint8_t,16> union_refresh = {0xf5,0x0f,0x1d,0xf8,0xf4,0x4f,0x01,0xa9,0xfd,0x7b,0x02,0xa9,0xfd,0x83,0x00,0x91};
    if (!patch_entry(base, 0x186e9b8, union_refresh, "UIUnionPanel.UpdateDontHaveUnionPage", reinterpret_cast<uintptr_t>(&skip_union_online_panel_refresh)))
        LOGE("Could not disable optional online union refresh: %s", last_error.c_str());
    const std::array<uint8_t,16> rewarded_ad = {0xf7,0x0f,0x1c,0xf8,0xf6,0x57,0x01,0xa9,0xf4,0x4f,0x02,0xa9,0xfd,0x7b,0x03,0xa9};
    if (!patch_entry(base, 0xF08120, rewarded_ad, "AdManager.ShowRewardAdWithCallback", reinterpret_cast<uintptr_t>(&complete_offline_ad_hook)))
        LOGE("Could not replace rewarded ads with local completion: %s", last_error.c_str());
    const std::array<uint8_t,16> update_treasure_page = {
        0xf5,0x0f,0x1d,0xf8, 0xf4,0x4f,0x01,0xa9,
        0xfd,0x7b,0x02,0xa9, 0xfd,0x83,0x00,0x91
    };
    void* treasure_page_trampoline = make_entry_trampoline(base, 0x13df2cc, update_treasure_page);
    if (treasure_page_trampoline) {
        original_treasure_page_update = reinterpret_cast<TreasurePageUpdateOriginal>(treasure_page_trampoline);
        if (!patch_entry(base, 0x13df2cc, update_treasure_page, "UITreasureBoxPage.UpdateTreasurePage",
                         reinterpret_cast<uintptr_t>(&show_local_ten_draw_chest))) {
            original_treasure_page_update = nullptr;
            LOGE("Could not reveal original 10-draw chest prefab node: %s", last_error.c_str());
        }
    } else LOGE("Could not allocate UpdateTreasurePage trampoline; ten-draw chest remains hidden");
    const std::array<uint8_t,16> update_treasure_page_parent = {
        0xff,0x83,0x02,0xd1, 0xe9,0x23,0x06,0x6d,
        0xf5,0x3b,0x00,0xf9, 0xf4,0x4f,0x08,0xa9
    };
    void* treasure_page_parent_trampoline = make_entry_trampoline(base, 0x13deca0, update_treasure_page_parent);
    if (treasure_page_parent_trampoline) {
        original_treasure_page_update_page = reinterpret_cast<TreasurePageUpdatePageOriginal>(treasure_page_parent_trampoline);
        if (!patch_entry(base, 0x13deca0, update_treasure_page_parent, "UITreasureBoxPage.UpdatePage",
                         reinterpret_cast<uintptr_t>(&update_page_local_ten_draw_chest))) {
            original_treasure_page_update_page = nullptr;
            LOGE("Could not install post-page-update ten-draw layout hook: %s", last_error.c_str());
        }
    } else LOGE("Could not allocate UpdatePage trampoline; ten-draw parent-page layout hook unavailable");
    // The original DoRefreshPage selects and refreshes the lower Shop sections
    // after UpdateTreasurePage. Hook its exact 1.9.1 prologue too, so the local
    // ten-draw spacing is applied after all original page positioning.
    const std::array<uint8_t,16> refresh_treasure_page = {
        0xf3,0x0f,0x1e,0xf8, 0xfd,0x7b,0x01,0xa9,
        0xfd,0x43,0x00,0x91, 0xf3,0x03,0x00,0xaa
    };
    void* treasure_page_refresh_trampoline = make_entry_trampoline(base, 0x13e0ed0, refresh_treasure_page);
    if (treasure_page_refresh_trampoline) {
        original_treasure_page_refresh = reinterpret_cast<TreasurePageRefreshOriginal>(treasure_page_refresh_trampoline);
        if (!patch_entry(base, 0x13e0ed0, refresh_treasure_page, "UITreasureBoxPage.DoRefreshPage",
                         reinterpret_cast<uintptr_t>(&refresh_local_ten_draw_chest))) {
            original_treasure_page_refresh = nullptr;
            LOGE("Could not install post-refresh ten-draw layout hook: %s", last_error.c_str());
        }
    } else LOGE("Could not allocate DoRefreshPage trampoline; ten-draw post-refresh layout unavailable");
    const std::array<uint8_t,16> update_buy_diamond = {
        0xff,0xc3,0x01,0xd1, 0xfc,0x6f,0x01,0xa9,
        0xfa,0x67,0x02,0xa9, 0xf8,0x5f,0x03,0xa9
    };
    void* buy_diamond_trampoline = make_entry_trampoline(base, 0x13dfb00, update_buy_diamond);
    if (buy_diamond_trampoline) {
        original_treasure_page_update_diamond = reinterpret_cast<TreasurePageUpdateDiamondOriginal>(buy_diamond_trampoline);
        if (!patch_entry(base, 0x13dfb00, update_buy_diamond, "UITreasureBoxPage.UpdateBuyDiamond",
                         reinterpret_cast<uintptr_t>(&hide_offline_paid_shop_section))) {
            original_treasure_page_update_diamond = nullptr;
            LOGE("Could not hide the original paid Shop prefab section: %s", last_error.c_str());
        }
    } else LOGE("Could not allocate UpdateBuyDiamond trampoline; paid Shop section hiding unavailable");
    const std::array<uint8_t,16> update_buy_coin = {
        0xff,0x83,0x02,0xd1, 0xfc,0x6f,0x04,0xa9,
        0xfa,0x67,0x05,0xa9, 0xf8,0x5f,0x06,0xa9
    };
    void* buy_coin_trampoline = make_entry_trampoline(base, 0x13dfdf0, update_buy_coin);
    if (buy_coin_trampoline) {
        original_treasure_page_update_coin = reinterpret_cast<TreasurePageUpdateCoinOriginal>(buy_coin_trampoline);
        if (!patch_entry(base, 0x13dfdf0, update_buy_coin, "UITreasureBoxPage.UpdateBuyCoin",
                         reinterpret_cast<uintptr_t>(&update_shop_daily_claim_ui))) {
            original_treasure_page_update_coin = nullptr;
            LOGE("Could not synchronize local Shop daily claim counter: %s", last_error.c_str());
        }
    } else LOGE("Could not allocate UpdateBuyCoin trampoline; Shop daily count may be stale until claimed");
    const std::array<uint8_t,16> purchase = {
        0xff,0x83,0x04,0xd1, 0xfc,0x67,0x0d,0xa9,
        0xf8,0x5f,0x0e,0xa9, 0xf6,0x57,0x0f,0xa9
    };
    if (!patch_entry(base, 0x2aa0120, purchase, "PurchaseManager.Purchase",
                     reinterpret_cast<uintptr_t>(&block_offline_purchase)))
        LOGE("Could not block external purchase requests: %s", last_error.c_str());
    const std::array<uint8_t,16> buy_treasure_chest = {
        0xf7,0x0f,0x1c,0xf8, 0xf6,0x57,0x01,0xa9,
        0xf4,0x4f,0x02,0xa9, 0xfd,0x7b,0x03,0xa9
    };
    const std::array<uint8_t,16> treasure_open_by_item = {
        0xff,0xc3,0x01,0xd1, 0xf5,0x23,0x00,0xf9,
        0xf4,0x4f,0x05,0xa9, 0xfd,0x7b,0x06,0xa9
    };
    void* open_by_item_trampoline = make_entry_trampoline(base, 0x1341a9c, treasure_open_by_item);
    if (open_by_item_trampoline) {
        original_treasure_box_on_open_by_item = reinterpret_cast<TreasureBoxOnOpenByItemOriginal>(open_by_item_trampoline);
        if (!patch_entry(base, 0x1341a9c, treasure_open_by_item, "TreasureBoxItem.OnOpenByItem",
                         reinterpret_cast<uintptr_t>(&treasure_box_on_open_by_item_hook))) {
            original_treasure_box_on_open_by_item = nullptr;
            LOGE("Could not install payment-route context hook for TreasureBoxItem.OnOpenByItem");
        }
    } else LOGE("Could not allocate OnOpenByItem trampoline; chest payment route remains currency-only");
    void* buy_chest_trampoline = make_entry_trampoline(base, 0x26d8ad0, buy_treasure_chest);
    if (buy_chest_trampoline) {
        original_buy_treasure_chest = reinterpret_cast<BuyTreasureChestOriginal>(buy_chest_trampoline);
        if (!patch_entry(base, 0x26d8ad0, buy_treasure_chest, "ShopManager.BuyTreasureChest",
                         reinterpret_cast<uintptr_t>(&local_buy_treasure_chest_hook))) {
            original_buy_treasure_chest = nullptr;
            LOGE("Could not install local gem-chest purchase hook: %s", last_error.c_str());
        }
    } else LOGE("Could not allocate BuyTreasureChest trampoline; local chest purchase unavailable");
    const std::array<uint8_t,16> shop_ad_coin = {
        0xf7,0x0f,0x1c,0xf8, 0xf6,0x57,0x01,0xa9,
        0xf4,0x4f,0x02,0xa9, 0xfd,0x7b,0x03,0xa9
    };
    if (!patch_entry(base, 0x13e0f80, shop_ad_coin, "UITreasureBoxPage.OnClickAdCoin",
                     reinterpret_cast<uintptr_t>(&block_shop_ad_coin)))
        LOGE("Could not block offline Shop ad-gold click: %s", last_error.c_str());
    const std::array<uint8_t,16> shop_lot_coin = {
        0xff,0x43,0x01,0xd1, 0xf6,0x57,0x02,0xa9,
        0xf4,0x4f,0x03,0xa9, 0xfd,0x7b,0x04,0xa9
    };
    if (!patch_entry(base, 0x13e145c, shop_lot_coin, "UITreasureBoxPage.OnClickBuyLotCoin",
                     reinterpret_cast<uintptr_t>(&block_shop_coin_exchange_lot)))
        LOGE("Could not install local Shop gem-to-gold click (lot): %s", last_error.c_str());
    const std::array<uint8_t,16> shop_huge_coin = {
        0xff,0x43,0x01,0xd1, 0xf6,0x57,0x02,0xa9,
        0xf4,0x4f,0x03,0xa9, 0xfd,0x7b,0x04,0xa9
    };
    if (!patch_entry(base, 0x13e1730, shop_huge_coin, "UITreasureBoxPage.OnClickBuyHugeCoin",
                     reinterpret_cast<uintptr_t>(&block_shop_coin_exchange_huge)))
        LOGE("Could not install local Shop gem-to-gold click (huge): %s", last_error.c_str());
    Klass talent_helper = find_class("ET", "TalentHelper");
    if (talent_helper && configure_task(talent_upgrade_task, talent_helper, "UpgradeTalent",
                                        {"System.Int32"}, nullptr)) {
        const std::array<uint8_t,16> upgrade_talent = {
            0xff,0x43,0x01,0xd1, 0xf4,0x4f,0x03,0xa9,
            0xfd,0x7b,0x04,0xa9, 0xfd,0x03,0x01,0x91
        };
        if (!patch_entry(base, 0x28fd814, upgrade_talent, "TalentHelper.UpgradeTalent",
                         reinterpret_cast<uintptr_t>(&local_talent_upgrade_hook)))
            LOGE("Could not replace online talent upgrade: %s", last_error.c_str());
    } else LOGE("Could not configure local talent upgrade task: %s", last_error.c_str());
    Klass equip_helper = find_class("ET", "EquipHelper");
    if (equip_helper && configure_task(local_wear_equip_task, equip_helper, "WearEquip",
                                       {"EquipPart", "System.Int32"}, nullptr)) {
        const std::array<uint8_t,16> wear_equip_helper = {
            0xff,0x43,0x01,0xd1, 0xf5,0x13,0x00,0xf9,
            0xf4,0x4f,0x03,0xa9, 0xfd,0x7b,0x04,0xa9
        };
        if (!patch_entry(base, 0x2c2d014, wear_equip_helper, "EquipHelper.WearEquip",
                         reinterpret_cast<uintptr_t>(&local_wear_equip_hook)))
            LOGE("Could not replace online equipment-wear helper: %s", last_error.c_str());
    } else LOGE("Could not configure local equipment-wear task: %s", last_error.c_str());
    if (equip_helper && configure_task(local_equip_upgrade_task, equip_helper, "UpgradeEquip",
                                       {"System.Int32"}, nullptr)) {
        const std::array<uint8_t,16> upgrade_equip_helper = {
            0xff,0x43,0x01,0xd1, 0xf4,0x4f,0x03,0xa9,
            0xfd,0x7b,0x04,0xa9, 0xfd,0x03,0x01,0x91
        };
        if (!patch_entry(base, 0x2c2d20c, upgrade_equip_helper, "EquipHelper.UpgradeEquip",
                         reinterpret_cast<uintptr_t>(&local_equip_upgrade_hook)))
            LOGE("Could not replace online equipment upgrade helper: %s", last_error.c_str());
    } else LOGE("Could not configure local equipment-upgrade task: %s", last_error.c_str());
    if (equip_helper && configure_task(local_equip_onekey_upgrade_task, equip_helper, "OneKeyUpgradeEquip",
                                       {"System.Int32"}, nullptr)) {
        const std::array<uint8_t,16> one_key_upgrade_equip_helper = {
            0xff,0x43,0x01,0xd1, 0xf4,0x4f,0x03,0xa9,
            0xfd,0x7b,0x04,0xa9, 0xfd,0x03,0x01,0x91
        };
        if (!patch_entry(base, 0x2c2d300, one_key_upgrade_equip_helper, "EquipHelper.OneKeyUpgradeEquip",
                         reinterpret_cast<uintptr_t>(&local_equip_onekey_upgrade_hook)))
            LOGE("Could not replace online one-key equipment helper: %s", last_error.c_str());
    } else LOGE("Could not configure local one-key equipment-upgrade task: %s", last_error.c_str());
    if (equip_helper && configure_task(local_equip_compound_task, equip_helper, "CompoundEquip",
                                       {"System.Int32", "System.Int32", "System.Int32"}, nullptr)) {
        const std::array<uint8_t,16> compound_equip_helper = {
            0xff,0x83,0x01,0xd1, 0xf6,0x57,0x03,0xa9,
            0xf4,0x4f,0x04,0xa9, 0xfd,0x7b,0x05,0xa9
        };
        if (!patch_entry(base, 0x2c2d3f4, compound_equip_helper, "EquipHelper.CompoundEquip",
                         reinterpret_cast<uintptr_t>(&local_equip_compound_hook)))
            LOGE("Could not replace online equipment fusion helper: %s", last_error.c_str());
    } else LOGE("Could not configure local equipment-fusion task: %s", last_error.c_str());
    if (equip_helper && configure_task(local_equip_onekey_compound_task, equip_helper, "OneKeyCompoundEquip", {}, nullptr)) {
        const std::array<uint8_t,16> entry = {
            0xff,0x03,0x01,0xd1, 0xf3,0x13,0x00,0xf9,
            0xfd,0x7b,0x03,0xa9, 0xfd,0xc3,0x00,0x91
        };
        if (!patch_entry(base, 0x2c2d50c, entry, "EquipHelper.OneKeyCompoundEquip offline collective policy",
                         reinterpret_cast<uintptr_t>(&local_equip_onekey_compound_hook)))
            LOGE("Could not replace online collective fusion helper: %s", last_error.c_str());
    } else LOGE("Could not configure collective equipment-fusion task: %s", last_error.c_str());
    Klass fusion_ui = find_class("", "UICompoundEquipTips");
    Method selection_update = fusion_ui ? find_method(fusion_ui, "UpdateCompoundList", {}) : nullptr;
    const std::array<uint8_t,16> fusion_selection_entry = {
        0xff,0x43,0x07,0xd1, 0xea,0xab,0x00,0xfd,
        0xe9,0x23,0x16,0x6d, 0xfc,0x6f,0x17,0xa9
    };
    if (selection_update) {
        void* trampoline = make_entry_trampoline(base, 0x12f8530, fusion_selection_entry);
        if (trampoline) {
            original_fusion_selection_update = reinterpret_cast<FusionSelectionOriginal>(trampoline);
            if (!patch_entry(base, 0x12f8530, fusion_selection_entry, "UICompoundEquipTips.UpdateCompoundList UID observation",
                             reinterpret_cast<uintptr_t>(&log_fusion_selection_update)))
                original_fusion_selection_update = nullptr;
        }
    }
    Klass hangup_helper = find_class("ET", "HangUpHelper");
    if (hangup_helper && configure_task(hangup_task, hangup_helper, "GetHangUpData", {}, "G2C_HangUpData")) {
        const std::array<uint8_t,16> get_hangup_data = {
            0xff,0x03,0x01,0xd1, 0xf3,0x13,0x00,0xf9,
            0xfd,0x7b,0x03,0xa9, 0xfd,0xc3,0x00,0x91
        };
        if (!patch_entry(base, 0x2964eb0, get_hangup_data, "HangUpHelper.GetHangUpData",
                         reinterpret_cast<uintptr_t>(&hangup_data_hook)))
            LOGE("Could not replace online HangUp data request: %s", last_error.c_str());
    } else LOGE("Could not configure local HangUp response: %s", last_error.c_str());
    if (hangup_helper && configure_task(reward_hangup_task, hangup_helper, "RewardHangUp", {"System.Int16"}, "G2C_RewardHangUp")) {
        const std::array<uint8_t,16> reward_hangup = {
            0xff,0x43,0x01,0xd1, 0xf4,0x4f,0x03,0xa9,
            0xfd,0x7b,0x04,0xa9, 0xfd,0x03,0x01,0x91
        };
        if (!patch_entry(base, 0x2964f98, reward_hangup, "HangUpHelper.RewardHangUp",
                         reinterpret_cast<uintptr_t>(&reward_hangup_hook)))
            LOGE("Could not replace online HangUp reward request: %s", last_error.c_str());
    } else LOGE("Could not configure local HangUp reward response: %s", last_error.c_str());
    hook_installed.store(true);
    // Optional combat transport adapters use exact original ARM64 prologues from the restored APK.
    if (configure_combat()) {
        const std::array<uint8_t,16> enter = {0xff,0x43,0x01,0xd1,0xf5,0x13,0x00,0xf9,0xf4,0x4f,0x03,0xa9,0xfd,0x7b,0x04,0xa9};
        const std::array<uint8_t,16> kill = {0xff,0xc3,0x01,0xd1,0xf7,0x1b,0x00,0xf9,0xf6,0x57,0x04,0xa9,0xf4,0x4f,0x05,0xa9};
        const std::array<uint8_t,16> end = {0xff,0x03,0x02,0xd1,0xf8,0x5f,0x04,0xa9,0xf6,0x57,0x05,0xa9,0xf4,0x4f,0x06,0xa9};
        if (patch_entry(base, 0x29c3728, enter, "FightHelper.EnterFight", reinterpret_cast<uintptr_t>(&enter_fight_hook)) &&
            patch_entry(base, 0x29c4abc, kill, "FightHelper.KillEnemy", reinterpret_cast<uintptr_t>(&kill_enemy_hook)) &&
            patch_entry(base, 0x29c3f44, end, "FightHelper.EndGame", reinterpret_cast<uintptr_t>(&end_game_hook)))
            combat_hooks_installed.store(true);
        else LOGE("Combat hook installation incomplete; %s", last_error.c_str());
    } else LOGE("Combat metadata API unavailable; core bootstrap remains installed: %s", last_error.c_str());
    const std::array<uint8_t,16> resurgence = {0xff,0x43,0x01,0xd1,0xf4,0x4f,0x03,0xa9,0xfd,0x7b,0x04,0xa9,0xfd,0x03,0x01,0x91};
    if (!patch_entry(base, 0x29c54c0, resurgence, "FightHelper.Resurgence", reinterpret_cast<uintptr_t>(&resurgence_hook)))
        LOGE("Could not replace online resurgence transport with local response: %s", last_error.c_str());
    // Apply the Chapter 2 assist at the actual damage application points. The
    // previous ResetData/ResetProperty stat writes were not evidence of combat damage.
    const std::array<uint8_t,16> deduct_enemy = {
        0xef,0x3b,0xb6,0x6d, 0xed,0x33,0x01,0x6d,
        0xeb,0x2b,0x02,0x6d, 0xe9,0x23,0x03,0x6d
    };
    void* enemy_trampoline = make_entry_trampoline(base, 0x27C9D4C, deduct_enemy);
    if (enemy_trampoline) {
        original_deduct_enemy_hp = reinterpret_cast<DeductEnemyHpOriginal>(enemy_trampoline);
        if (!patch_entry(base, 0x27C9D4C, deduct_enemy, "FightDataManager.DeductEnemyHp",
                         reinterpret_cast<uintptr_t>(&deduct_enemy_hp_hook)))
            LOGE("Could not install Chapter 2 hero damage hook: %s", last_error.c_str());
    } else LOGE("Could not allocate DeductEnemyHp trampoline; hero damage assist unavailable");
    const std::array<uint8_t,16> deduct_player = {
        0xee,0x0f,0x16,0xfc, 0xed,0x33,0x01,0x6d,
        0xeb,0x2b,0x02,0x6d, 0xe9,0x23,0x03,0x6d
    };
    void* player_trampoline = make_entry_trampoline(base, 0x27E027C, deduct_player);
    if (player_trampoline) {
        original_deduct_player_hp = reinterpret_cast<DeductPlayerHpOriginal>(player_trampoline);
        if (!patch_entry(base, 0x27E027C, deduct_player, "FightDataManager.DeductPlayerHp",
                         reinterpret_cast<uintptr_t>(&deduct_player_hp_hook)))
            LOGE("Could not install Chapter 2 enemy damage hook: %s", last_error.c_str());
    } else LOGE("Could not allocate DeductPlayerHp trampoline; enemy damage reduction unavailable");
    return true;
}

void wait_for_game() {
    status("waiting", "Waiting for original libil2cpp and initialized domain");
    void* library = nullptr;
    // RTLD_NOLOAD observes the library; it never starts IL2CPP from the background worker.
    for (int attempt = 0; attempt < 1800; ++attempt) {
        library = dlopen("libil2cpp.so", RTLD_NOW | RTLD_NOLOAD);
        if (library) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!library) { status("failed", "Original libil2cpp was not loaded within 180 seconds"); return; }
    if (!load_api(library)) { status("failed", "Missing required IL2CPP export; AccountInit remains unchanged"); return; }
    // Unity 2020 begins loading libil2cpp before il2cpp_init has completed. Polling
    // domain_get during that window races Unity's own thread joins on Android 16.
    // Give the original engine startup a quiet window before the first IL2CPP call.
    status("waiting_unity", "Original IL2CPP library loaded; waiting for Unity startup before metadata access");
    std::this_thread::sleep_for(std::chrono::seconds(2));
    void* domain = nullptr;
    for (int attempt = 0; attempt < 1800; ++attempt) {
        domain = api.domain_get();
        size_t count = 0;
        if (domain && api.domain_get_assemblies(domain, &count) && count) break;
        domain = nullptr;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    if (!domain) { status("failed", "IL2CPP domain did not become ready within 180 seconds"); return; }
    void* thread = api.thread_attach(domain);
    if (!thread) { status("failed", "Cannot attach offline worker to IL2CPP domain"); return; }
    // The worker resolves metadata only. Singleton creation and all gameplay callbacks happen in the hook.
    bool ready = find_class("", "PlayerDataManager") && find_class("Newtonsoft.Json", "JsonConvert");
    if (ready && install_hook(library)) status("hooked", combat_hooks_installed.load() ?
        "Original AccountInit and EnterFight/KillEnemy/EndGame helpers redirected; waiting for Unity initialization" :
        "Original AccountInit redirected; combat transport hooks are unavailable, see KinjaOffline log");
    else status("failed", last_error.empty() ? "Required game metadata unavailable" : last_error);
    api.thread_detach(thread);
    // Retain dlopen reference for the lifetime of pointers stored in api/the patched entry.
}
} // namespace

extern "C" JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM*, void*) {
    LOGI("Offline bridge loaded; awaiting configure(filesDir, AssetManager)");
    return JNI_VERSION_1_6;
}

extern "C" JNIEXPORT void JNICALL
Java_local_kinja_OfflineActivity_configure(JNIEnv* env, jclass, jstring directory, jobject manager) {
    std::lock_guard<std::mutex> lock(config_mutex);
    if (configured.load()) { LOGI("Bridge already configured in this process"); return; }
    if (!directory || !manager) { LOGE("configure requires filesDir and AssetManager"); return; }
    const char* path = env->GetStringUTFChars(directory, nullptr);
    if (!path) return;
    files_dir = path;
    env->ReleaseStringUTFChars(directory, path);
    if (files_dir.empty() || files_dir[0] != '/') { LOGE("Invalid filesDir path"); return; }
    asset_owner = env->NewGlobalRef(manager);
    assets = AAssetManager_fromJava(env, manager);
    if (!asset_owner || !assets) { LOGE("Cannot retain Android AssetManager"); return; }
    configured.store(true);
    LOGI("Configured local files directory %s", files_dir.c_str());
#if defined(KINJA_OFFLINE_DIAGNOSTIC_NO_WORKER)
    // Diagnostic build: retain JNI/asset setup, but create no worker and call no IL2CPP API.
    status("diagnostic_no_worker", "JNI configured; worker and all IL2CPP calls/hooks are disabled");
#else
    std::thread(wait_for_game).detach();
#endif
}
