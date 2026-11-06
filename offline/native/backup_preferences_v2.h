// Included inside the bridge namespace after its JSON/IL2CPP helpers.
// Format 2: integer/bool records retain {Key,Present,Value}; float records also
// carry Type="float" and Value is the exact unsigned IEEE754 binary32 bits.
struct BackupPreference {
    std::string key;
    bool obscured = false, boolean = false, floating = false;
    float fallback = 0;
};

std::vector<BackupPreference> backup_preferences() {
    std::vector<BackupPreference> result{{"1fight_fail_count",false,false},
        {"1drop_die_hint",false,false}, {"boxguide_close_1",false,false},
        {"_manga_state",true,true}, {"game_language",false,false},
        {"sound_volume",false,false,true,1.f}, {"music_volume",false,false,true,1.f},
        {"operation_sensitivity",false,false,true,0.33f}};
    for (int id : backup_region_ids) result.push_back({"fail_region_"+std::to_string(id),true,false});
    return result;
}

uint32_t backup_float_bits(float value) {
    uint32_t bits = 0;
    static_assert(sizeof(bits) == sizeof(value), "Backup requires IEEE754 binary32");
    std::memcpy(&bits,&value,sizeof(bits));
    return bits;
}

float backup_bits_float(uint32_t bits) {
    float value = 0;
    std::memcpy(&value,&bits,sizeof(value));
    return value;
}

struct BackupPreferenceState { BackupPreference definition; bool present = false; uint32_t value = 0; };

// Exact 1.9.1 Init.LanguageSet mapping, used only when no language was chosen.
int32_t backup_system_language(int32_t system) {
    static constexpr int32_t languages[]{6,5,2,2,2,2,11,10,4,3,2,2,2,2,8,2,9,2,2,2,7,2,12,15,2,13,0,1};
    if (system == 1) return 14;
    if (system == 6) return 0;
    return system >= 14 && system <= 41 ? languages[system-14] : 2;
}

bool logical_backup_preferences(std::string& snapshot, const std::string* checkpoint = nullptr) {
    Roots roots;
    Klass plain = find_class("UnityEngine","PlayerPrefs");
    Klass obscured = find_class("CodeStage.AntiCheat.Storage","ObscuredPrefs");
    Klass config = find_class("","ConfigTool");
    Field language = config ? api.class_get_field_from_name(config,"CurrentLanguageIndex") : nullptr;
    if (!plain || !obscured || !language || !api.field_static_get_value || !api.field_static_set_value ||
        type_name(api.field_get_type(language)) != "System.Int32")
        return fail("Manual backup original settings APIs unavailable");
    int32_t effective_language = 0;
    api.field_static_get_value(language,&effective_language);
    if (effective_language < 0 || effective_language > 15) return fail("Original language index is invalid");

    std::vector<BackupPreferenceState> states;
    for (const auto& entry : backup_preferences()) {
        BackupPreferenceState state{entry,false,0};
        if (checkpoint) {
            const std::string needle = "\"Key\":\""+entry.key+"\"";
            size_t pos = checkpoint->find(needle);
            if (pos == std::string::npos || checkpoint->find(needle,pos+needle.size()) != std::string::npos)
                return fail("Missing or duplicate manual preference checkpoint");
            size_t end = checkpoint->find('}',pos);
            int64_t number = 0;
            if (end == std::string::npos) return false;
            std::string record = checkpoint->substr(pos,end-pos);
            const bool typed_float = record.find("\"Type\":\"float\"") != std::string::npos;
            if (!json_read_bool(record,"Present",state.present) || !json_read_int(record,"Value",number) ||
                number < 0 || static_cast<uint64_t>(number) > UINT32_MAX || typed_float != entry.floating ||
                (!entry.floating && record.find("\"Type\"") != std::string::npos))
                return fail("Invalid typed manual preference checkpoint");
            state.value = static_cast<uint32_t>(number);
            if (entry.floating) {
                float value = backup_bits_float(state.value);
                if (!std::isfinite(value) || value < 0 || value > 1 ||
                    (!state.present && state.value != backup_float_bits(entry.fallback)))
                    return fail("Invalid manual floating point preference");
            } else if (state.value > (entry.boolean ? 1u : entry.key == "game_language" ? 15u : INT32_MAX) ||
                (!state.present && entry.key != "game_language" && state.value != 0))
                return fail("Invalid manual integer preference");
        }
        states.push_back(state);
    }
    int32_t destination_language = effective_language;
    if (checkpoint) {
        Method system_language = find_method(find_class("UnityEngine","Application"),"get_systemLanguage",{});
        Obj boxed = nullptr;
        if (!system_language || !invoke(system_language,nullptr,nullptr,"Read destination system language",&boxed) || !boxed)
            return false;
        roots.retain(boxed);
        auto* system = static_cast<int32_t*>(api.object_unbox(boxed));
        if (!system) return false;
        destination_language = backup_system_language(*system);
    }
    // Resolve every method and validate all records above before the first Set/Delete call.
    struct PreferenceMethods { Method has, get, set, remove; };
    std::vector<PreferenceMethods> methods;
    for (const auto& state : states) {
        const auto& entry = state.definition;
        Klass klass = entry.obscured ? obscured : plain;
        const char* scalar = entry.floating ? "System.Single" : entry.boolean ? "System.Boolean" : "System.Int32";
        PreferenceMethods m{find_method(klass,"HasKey",{"System.String"},true),
            find_method(klass,entry.floating?"GetFloat":entry.boolean?"GetBool":"GetInt",{"System.String",scalar},true),
            find_method(klass,entry.floating?"SetFloat":entry.boolean?"SetBool":"SetInt",{"System.String",scalar},true),
            find_method(klass,"DeleteKey",{"System.String"},true)};
        if (!m.has || !m.get || !m.set || !m.remove) return false;
        methods.push_back(m);
    }
    Klass audio_class = nullptr;
    Obj audio = nullptr;
    Method audio_setters[2]{}, audio_getters[2]{};
    if (checkpoint) {
        audio_class = find_class("","AudioManager");
        audio = audio_class ? singleton(audio_class,roots) : nullptr;
        if (!audio) return false;
        for (int i=0;i<2;++i) {
            audio_setters[i] = find_method(audio_class,i?"set_MusicVolume":"set_SoundVolume",{"System.Single"});
            audio_getters[i] = find_method(audio_class,i?"get_MusicVolume":"get_SoundVolume",{});
            if (!audio_setters[i] || !audio_getters[i]) return false;
        }
    }

    snapshot = "{\"Values\":[";
    for (size_t i=0;i<states.size();++i) {
        auto& state = states[i];
        const auto& entry = state.definition;
        const auto& m = methods[i];
        Obj key = roots.retain(api.string_new(entry.key.c_str()));
        if (!key) return false;
        void* key_args[]{key};
        auto presence = [&](bool& result) {
            Obj boxed = nullptr;
            if (!invoke(m.has,nullptr,key_args,"Manual backup preference presence",&boxed) || !boxed) return false;
            roots.retain(boxed);
            auto raw = static_cast<uint8_t*>(api.object_unbox(boxed));
            if (!raw) return false;
            result = *raw != 0;
            return true;
        };
        if (checkpoint) {
            int32_t integer = static_cast<int32_t>(state.value);
            uint8_t boolean = state.value != 0;
            float floating = backup_bits_float(state.value);
            void* args[]{key,entry.floating?static_cast<void*>(&floating):
                entry.boolean?static_cast<void*>(&boolean):static_cast<void*>(&integer)};
            if (!(state.present ? invoke(m.set,nullptr,args,"Restore typed manual preference") :
                invoke(m.remove,nullptr,key_args,"Remove absent manual preference"))) return false;
            if (entry.key == "game_language") {
                if (!state.present) integer = destination_language;
                api.field_static_set_value(language,&integer);
                int32_t readback = -1;
                api.field_static_get_value(language,&readback);
                if (readback != integer) return fail("Original language cache readback mismatch");
            }
            if (entry.key == "sound_volume" || entry.key == "music_volume") {
                int index = entry.key == "music_volume" ? 1 : 0;
                void* volume_args[]{&floating};
                Obj boxed = nullptr;
                if (!invoke(audio_setters[index],audio,volume_args,"Refresh original audio cache") ||
                    !invoke(audio_getters[index],audio,nullptr,"Read original audio cache",&boxed) || !boxed) return false;
                roots.retain(boxed);
                auto raw = static_cast<float*>(api.object_unbox(boxed));
                if (!raw || backup_float_bits(*raw) != state.value) return fail("Original audio cache readback mismatch");
                // Original setters also persist the value. Preserve the checkpoint's absence.
                if (!state.present && !invoke(m.remove,nullptr,key_args,"Preserve absent audio setting")) return false;
            }
        }
        bool present = false;
        if (!presence(present) || (checkpoint && present != state.present))
            return fail("Manual preference presence readback mismatch");
        uint32_t actual = entry.floating ? backup_float_bits(entry.fallback) :
            entry.key == "game_language" ? static_cast<uint32_t>(effective_language) : 0;
        if (present) {
            int32_t default_int = 0;
            uint8_t default_bool = 0;
            float default_float = entry.fallback;
            void* get_args[]{key,entry.floating?static_cast<void*>(&default_float):
                entry.boolean?static_cast<void*>(&default_bool):static_cast<void*>(&default_int)};
            Obj boxed = nullptr;
            if (!invoke(m.get,nullptr,get_args,"Read typed manual preference",&boxed) || !boxed) return false;
            roots.retain(boxed);
            void* raw = api.object_unbox(boxed);
            if (!raw) return false;
            actual = entry.floating ? backup_float_bits(*static_cast<float*>(raw)) :
                entry.boolean ? (*static_cast<uint8_t*>(raw) != 0) : static_cast<uint32_t>(*static_cast<int32_t*>(raw));
        } else if (checkpoint) actual = state.value;
        if (entry.floating) {
            float value = backup_bits_float(actual);
            if (!std::isfinite(value) || value < 0 || value > 1) return fail("Original volume/sensitivity outside supported range");
        } else if (actual > (entry.boolean ? 1u : entry.key == "game_language" ? 15u : INT32_MAX))
            return fail("Original integer preference outside supported range");
        if (checkpoint && actual != state.value) return fail("Manual typed preference value readback mismatch");
        if (snapshot.back() != '[') snapshot += ',';
        snapshot += "{\"Key\":\""+entry.key+"\",\"Present\":"+(present?"true":"false");
        if (entry.floating) snapshot += ",\"Type\":\"float\"";
        snapshot += ",\"Value\":"+std::to_string(actual)+"}";
    }
    snapshot += "]}";
    if (checkpoint) for (Klass klass : {plain,obscured}) {
        Method save = find_method(klass,"Save",{},true);
        if (!save || !invoke(save,nullptr,nullptr,"Save restored typed manual preferences")) return false;
    }
    return true;
}
