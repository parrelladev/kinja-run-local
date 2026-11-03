// Included inside the bridge namespace after the JSON/IL2CPP helpers.
struct LocalHeroRecord {
    int id = 0, level = 0, star = 0;
    std::string json;
};

bool read_saved_heroes(const std::string& text, std::vector<LocalHeroRecord>& heroes, int& equipped) {
    int64_t selected = 0;
    size_t cursor = json_member_value(text, "HerosInfo");
    if (!json_read_int(text, "EquipHeroId", selected) || selected <= 0 || selected > INT32_MAX ||
        cursor == std::string::npos || text[cursor] != '[') return fail("Malformed local hero list/selection");
    heroes.clear();
    std::unordered_set<int> seen;
    for (++cursor; cursor < text.size();) {
        while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor]))) ++cursor;
        if (cursor < text.size() && text[cursor] == ']') {
            equipped = static_cast<int>(selected);
            return seen.count(equipped) != 0 || fail("Selected hero is not owned in saved data");
        }
        size_t end = json_object_end(text, cursor);
        int64_t id = 0, level = 0, star = 0;
        if (cursor >= text.size() || text[cursor] != '{' || end == std::string::npos)
            return fail("Malformed local hero record");
        std::string record = text.substr(cursor,end-cursor);
        if (!json_read_int(record,"HeroId",id) || !json_read_int(record,"Level",level) || !json_read_int(record,"Star",star) ||
            id <= 0 || id > INT32_MAX || level < 1 || level > 1000 || star < 0 || star > 1000 ||
            json_member_value(record,"HeroTalent") == std::string::npos || !seen.insert(static_cast<int>(id)).second)
            return fail("Invalid or duplicate local hero identity/progression");
        heroes.push_back({static_cast<int>(id),static_cast<int>(level),static_cast<int>(star),record});
        cursor = end;
        while (cursor < text.size() && std::isspace(static_cast<unsigned char>(text[cursor]))) ++cursor;
        if (cursor < text.size() && text[cursor] == ',') { ++cursor; continue; }
        if (cursor >= text.size() || text[cursor] != ']') return fail("Malformed hero list separator");
    }
    return fail("Unterminated local hero list");
}

bool replace_saved_heroes(std::string& text, const std::vector<LocalHeroRecord>& heroes, int equipped) {
    std::string list = "[";
    for (const auto& hero : heroes) { if (list.size() > 1) list += ','; list += hero.json; }
    return json_set_array(text,"HerosInfo",list+"]") && json_set_int(text,"EquipHeroId",equipped);
}

bool hero_cfg_int(Obj object, const char* name, int& value) {
    Field field = object ? api.class_get_field_from_name(api.object_get_class(object),name) : nullptr;
    if (!field) return fail(std::string("Missing original hero config field ")+name);
    api.field_get_value(object,field,&value);
    return true;
}

bool repair_legacy_hero_stars(Roots& roots) {
    std::string text = load_seed("seed_heroes.json");
    std::vector<LocalHeroRecord> heroes;
    int selected = 0;
    if (!read_saved_heroes(text,heroes,selected)) return false;
    bool changed = false;
    Klass klass = find_class("","HeroManager");
    Obj manager = klass ? singleton(klass,roots) : nullptr;
    Method get = klass ? find_method(klass,"GetHeroStarCfgOne",{"System.Int32","System.Int32"}) : nullptr;
    if (!manager || !get) return false;
    for (auto& hero : heroes) {
        if (hero.star != 0) continue;
        int32_t id = hero.id, star = 1;
        Obj cfg = nullptr;
        void* args[] = {&id,&star};
        int limit = 0;
        if (!invoke(get,manager,args,"Validate minimum original hero star",&cfg) || !cfg ||
            !hero_cfg_int(cfg,"lv_limit",limit) || hero.level > limit || !json_set_int(hero.json,"Star",1))
            return fail("Legacy star-zero hero cannot be normalized safely");
        roots.retain(cfg);
        LOGI("Normalized legacy hero=%d star=0->1 level=%d; original minimum, no fragments charged",hero.id,hero.level);
        hero.star = 1;
        changed = true;
    }
    return !changed || (replace_saved_heroes(text,heroes,selected) &&
        write_fusion_snapshot(files_dir+"/offline_save/seed_heroes.json",text));
}

bool local_hero_function_open(bool& opened, Roots& roots) {
    opened = false;
    Klass cfg_class = find_class("","FunctionCfg");
    Method get = cfg_class ? find_method(cfg_class,"GetCfg",{"System.Int32"}) : nullptr;
    int32_t key = 107;
    Obj cfg = nullptr;
    void* args[] = {&key};
    int type = -1, threshold = 0, enabled = 0;
    if (!get || !invoke(get,nullptr,args,"FunctionCfg.GetCfg hero progression",&cfg) || !cfg) return false;
    roots.retain(cfg);
    if (!hero_cfg_int(cfg,"unlock_type",type) || !hero_cfg_int(cfg,"unlock_lv",threshold) ||
        !hero_cfg_int(cfg,"enable",enabled) || type != 1 || threshold < 3 || enabled != 1)
        return fail("Unsupported original hero unlock configuration");
    std::string regions = load_seed("seed_regions.json");
    int64_t maximum = 0;
    if (!json_read_int(regions,"MaxRegion",maximum)) return false;
    if (maximum < threshold) return true;
    const int required_clear = threshold-1;
    bool passed = false;
    if (!json_region(regions,required_clear,0,false,&passed,nullptr))
        return fail("Reached hero unlock region is absent from save");
    opened = passed;
    LOGI("Local hero unlock function107 threshold=%d requiredClearRegion=%d passed=%d",threshold,required_clear,opened);
    return true;
}

LocalTaskApi hero_tasks[4];
std::atomic<bool> hero_busy{false};
const char* hero_files[] = {"seed_heroes.json","seed_items.json","seed_player.json"};

bool recover_hero_transaction() {
    const std::string dir = files_dir+"/offline_save/";
    if (access((dir+"hero_pending").c_str(),F_OK) != 0) return true;
    std::string old[3];
    for (int i=0;i<3;++i)
        if (!read_local_text(dir+"hero_"+std::to_string(i)+".before",old[i]) || old[i].empty())
            return fail("Hero recovery snapshot missing; refusing login");
    for (int i=0;i<3;++i)
        if (!write_fusion_snapshot(dir+hero_files[i],old[i])) return false;
    if (unlink((dir+"hero_pending").c_str()) != 0 || !sync_fusion_directory(dir)) return false;
    LOGI("Recovered interrupted local hero transaction");
    return true;
}

bool hero_int_result(Method method, Obj self, void** args, int& value, Roots& roots, bool obscured=false) {
    Obj boxed=nullptr;
    if (!method || !invoke(method,self,args,"Original hero integer readback",&boxed) || !boxed) return false;
    roots.retain(boxed);
    if (obscured) {
        Method decrypt=find_method(api.object_get_class(boxed),"GetDecrypted",{});
        void* raw=api.object_unbox(boxed);
        Obj plain=nullptr;
        if (!raw || !decrypt || !invoke(decrypt,raw,nullptr,"Hero ObscuredInt.GetDecrypted",&plain) || !plain) return false;
        boxed=roots.retain(plain);
    }
    void* raw=api.object_unbox(boxed);
    if (!raw) return false;
    value=*static_cast<int32_t*>(raw);
    return true;
}

// operation: equip, acquire with fragments, level, star.
bool local_hero_action(int operation, int32_t id, Roots& roots) {
    bool opened=false;
    const std::string dir=files_dir+"/offline_save/";
    if (!local_hero_function_open(opened,roots) || !opened || id<=0 ||
        access((dir+"hero_pending").c_str(),F_OK)==0 || equip_upgrade_in_progress.load() ||
        equip_compound_in_progress.load() || wear_equip_in_progress.load() || chest_purchase_in_progress.load())
        return fail("Hero action locked or another transaction pending");
    std::string before[3]={load_seed(hero_files[0]),load_seed(hero_files[1]),load_seed(hero_files[2])};
    std::vector<LocalHeroRecord> heroes;
    int selected=0;
    int64_t gold=0;
    if (!read_saved_heroes(before[0],heroes,selected) || !json_read_int(before[2],"Coin",gold) || gold<0 || gold>INT32_MAX) return false;
    Klass hc=find_class("","HeroManager"), ic=find_class("","ItemManager"), pc=find_class("","PlayerDataManager");
    Obj hm=hc?singleton(hc,roots):nullptr, im=ic?singleton(ic,roots):nullptr, pm=pc?singleton(pc,roots):nullptr;
    Method get=hc?find_method(hc,"GetHeroInfo",{"System.Int32"}):nullptr;
    Method equip=hc?find_method(hc,"get_EquipHeroId",{}):nullptr;
    Method cfg_get=hc?find_method(hc,"GetHeroCfg",{"System.Int32"}):nullptr;
    Method star_get=hc?find_method(hc,"GetHeroStarCfgOne",{"System.Int32","System.Int32"}):nullptr;
    if (!hm || !im || !pm || !get || !equip || !cfg_get || !star_get) return false;
    auto matches=[&](const std::vector<LocalHeroRecord>& expected,int chosen) {
        int actual=0;
        Obj all=nullptr;
        Field all_field=api.class_get_field_from_name(hc,"AllHeroInfo");
        if (!all_field) return false;
        api.field_get_value(hm,all_field,&all);
        if (!all) return false;
        roots.retain(all);
        if (!hero_int_result(find_method(api.object_get_class(all),"get_Count",{},true),all,nullptr,actual,roots) ||
            actual!=static_cast<int>(expected.size())) return false;
        if (!hero_int_result(equip,hm,nullptr,actual,roots,true) || actual!=chosen) return false;
        for (const auto& hero:expected) {
            int32_t key=hero.id; void* args[]={&key}; Obj live=nullptr;
            if (!invoke(get,hm,args,"HeroManager.GetHeroInfo readback",&live) || !live) return false;
            roots.retain(live);
            for (const auto& field:std::array<std::pair<const char*,int>,3>{{{"get_HeroId",hero.id},{"get_Level",hero.level},{"get_Star",hero.star}}})
                if (!hero_int_result(find_method(api.object_get_class(live),field.first,{}),live,nullptr,actual,roots) || actual!=field.second) return false;
        }
        return true;
    };
    if (!matches(heroes,selected)) return fail("Hero live/save progression mismatch");
    const auto old_heroes=heroes;
    auto target=std::find_if(heroes.begin(),heroes.end(),[&](const LocalHeroRecord& h){return h.id==id;});
    const bool owned=target!=heroes.end();
    if ((operation==1)==owned) return fail("Hero ownership does not permit requested action");
    Obj cfg=nullptr; void* id_args[]={&id};
    if (!invoke(cfg_get,hm,id_args,"HeroManager.GetHeroCfg local action",&cfg) || !cfg) return false;
    roots.retain(cfg);
    int item=0,cost=0,coins=0,next_selected=selected;
    if (operation==0) next_selected=id;
    if (operation==1) {
        int enabled=0;
        if (!hero_cfg_int(cfg,"can_receive",enabled) || enabled!=1 || !hero_cfg_int(cfg,"fragment_id",item) ||
            !hero_cfg_int(cfg,"fragment_num",cost) || cost<=0) return false;
        int32_t initial_star=1; void* initial_args[]={&id,&initial_star}; Obj initial_cfg=nullptr;
        if (!invoke(star_get,hm,initial_args,"Validate acquired hero minimum star",&initial_cfg) || !initial_cfg) return false;
        roots.retain(initial_cfg);
        heroes.push_back({id,1,1,"{\"HeroId\":"+std::to_string(id)+",\"Level\":1,\"Star\":1,\"HeroTalent\":[]}"});
    }
    if (operation>=2) {
        int32_t star=target->star; Obj current=nullptr; void* sa[]={&id,&star};
        if (star<1 || !invoke(star_get,hm,sa,"Current original hero star configuration",&current) || !current) return false;
        roots.retain(current);
        if (operation==2) {
            int limit=0;
            if (!hero_cfg_int(current,"lv_limit",limit) || target->level>=limit) return fail("Hero level cap reached");
            Klass exp=find_class("","HeroExpCfg"); Method exp_get=exp?find_method(exp,"GetCfg",{"System.Int32"}):nullptr;
            int32_t level=target->level; void* la[]={&level}; Obj price=nullptr;
            if (!exp_get || !invoke(exp_get,nullptr,la,"HeroExpCfg.GetCfg local level cost",&price) || !price) return false;
            roots.retain(price);
            if (!hero_cfg_int(price,"coin_cost",coins) || !hero_cfg_int(price,"item_id",item) || !hero_cfg_int(price,"item_cost",cost)) return false;
            ++target->level;
            if (!json_set_int(target->json,"Level",target->level)) return false;
        } else {
            int32_t next=star+1; void* na[]={&id,&next}; Obj next_cfg=nullptr;
            if (!invoke(star_get,hm,na,"Next original hero star configuration",&next_cfg) || !next_cfg) return fail("Hero star cap reached");
            roots.retain(next_cfg);
            if (!hero_cfg_int(cfg,"fragment_id",item) || !hero_cfg_int(current,"item_cost",cost)) return false;
            ++target->star;
            if (!json_set_int(target->json,"Star",target->star)) return false;
        }
    }
    int64_t count=0; int live_gold=0,live_count=0;
    if (coins<0 || cost<0 || (operation!=0 && (item<=0 || cost<=0)) || gold<coins ||
        !read_original_item_count(im,ic,101,live_gold,roots) || live_gold!=gold) return fail("Hero gold invalid, insufficient or desynchronized");
    if (item && (!json_get_item_count(before[1],item,count) || count<cost || count>INT32_MAX ||
        !read_original_item_count(im,ic,item,live_count,roots) || live_count!=count)) return fail("Hero materials insufficient or desynchronized");
    std::string after[3]={before[0],before[1],before[2]};
    if (!replace_saved_heroes(after[0],heroes,next_selected) || !json_set_int(after[2],"Coin",gold-coins) ||
        (item && !json_add_item(after[1],"ItemsInfo",item,-cost,"ItemId","Count"))) return false;
    Klass jc=find_class("Newtonsoft.Json","JsonConvert");
    Method json=jc?find_method(jc,"DeserializeObject",{"System.String","System.Type"}):nullptr;
    auto model=[&](const char* name,const std::string& text)->Obj {
        Klass c=find_class("ET",name); return c&&json?deserialize_local_payload(json,c,text,roots):nullptr;
    };
    Obj update_new=model("G2C_PlayerHeroDataUpdate",after[0]), update_old=model("G2C_PlayerHeroDataUpdate",before[0]);
    Obj init_old=model("G2C_HeroInfo",before[0]), items_new=model("G2C_ItemInfo",after[1]), items_old=model("G2C_ItemInfo",before[1]);
    Klass handler_class=find_class("ET","G2C_PlayerHeroDataUpdateHandler"), activator=find_class("System","Activator");
    Method create=activator?find_method(activator,"CreateInstance",{"System.Type"}):nullptr;
    Method run=handler_class?find_method(handler_class,"Run",{"ET.Session","ET.G2C_PlayerHeroDataUpdate"}):nullptr;
    Obj type=handler_class?roots.retain(api.type_get_object(api.class_get_type(handler_class))):nullptr, handler=nullptr;
    void* ca[]={type};
    Method init=find_method(hc,"HeroDataInit",{"ET.G2C_HeroInfo"});
    Method on_items=find_method(ic,"OnItemInit",{"ET.G2C_ItemInfo"});
    Method sync=find_method(pc,"SyncValueToShow",{});
    Method set_gold=find_method(pc,"set_PlayerGold",{"CodeStage.AntiCheat.ObscuredTypes.ObscuredInt"});
    Klass oi=find_class("CodeStage.AntiCheat.ObscuredTypes","ObscuredInt");
    Method convert=oi?find_method(oi,"op_Implicit",{"System.Int32"}):nullptr;
    auto box=[&](int32_t value)->Obj { Obj out=nullptr; void* a[]={&value}; return convert&&invoke(convert,nullptr,a,"Hero currency box",&out)?roots.retain(out):nullptr; };
    Obj gold_old=box(gold),gold_new=box(gold-coins);
    if (!update_new || !update_old || !init_old || !items_new || !items_old || !run || !create || !type || !init ||
        !on_items || !sync || !set_gold || !gold_old || !gold_new || !invoke(create,nullptr,ca,"Create original hero update handler",&handler) || !handler) return false;
    roots.retain(handler);
    auto apply=[&](Obj update,Obj items,Obj currency,bool restoring) {
        bool ok=true;
        if (restoring) {
            void* a[]={init_old}; ok=invoke(init,hm,a,"HeroDataInit rollback");
            Klass scene=find_class("","GameSceneManager"); Obj sm=scene?singleton(scene,roots):nullptr;
            Method warm=scene?find_method(scene,"WarmupFightBuff",{}):nullptr;
            ok=(sm && warm && invoke(warm,sm,nullptr,"Restore original hero combat cache"))&&ok;
        }
        void* raw=api.object_unbox(currency); if (!raw) return false;
        void* ga[]={raw},*ia[]={items},*ha[]={nullptr,update};
        ok=invoke(set_gold,pm,ga,"Hero set_PlayerGold")&&ok;
        ok=invoke(on_items,im,ia,"Hero OnItemInit")&&ok;
        ok=invoke(run,handler,ha,"Original hero data update and UI event")&&ok;
        ok=invoke(sync,pm,nullptr,"Hero SyncValueToShow")&&ok;
        return ok;
    };
    for (int i=0;i<3;++i) if (!write_fusion_snapshot(dir+"hero_"+std::to_string(i)+".before",before[i])) return false;
    if (!write_fusion_snapshot(dir+"hero_pending","hero transaction\n")) return false;
    auto rollback=[&]() {
        bool ok=true;
        for (int i=0;i<3;++i) ok=write_fusion_snapshot(dir+hero_files[i],before[i])&&ok;
        ok=apply(update_old,items_old,gold_old,true)&&ok;
        ok=matches(old_heroes,selected)&&ok;
        if (ok) { if (unlink((dir+"hero_pending").c_str())!=0 || !sync_fusion_directory(dir)) ok=false; }
        LOGE("Hero action rollback complete=%d; recovery journal kept on failure",ok);
        return false;
    };
    for (int i=0;i<3;++i) if (!write_fusion_snapshot(dir+hero_files[i],after[i])) return rollback();
    if (!apply(update_new,items_new,gold_new,false) || !matches(heroes,next_selected) ||
        !read_original_item_count(im,ic,101,live_gold,roots) || live_gold!=gold-coins ||
        (item && (!read_original_item_count(im,ic,item,live_count,roots) || live_count!=count-cost))) return rollback();
    for (int i=0;i<3;++i) if (load_seed(hero_files[i])!=after[i]) return rollback();
    if (unlink((dir+"hero_pending").c_str())!=0) return rollback();
    if (!sync_fusion_directory(dir)) { write_fusion_snapshot(dir+"hero_pending","uncertain commit\n"); return rollback(); }
    for (int i=0;i<3;++i) unlink((dir+"hero_"+std::to_string(i)+".before").c_str());
    LOGI("Hero committed operation=%d id=%d selected=%d gold=%lld->%d item=%d count=%lld->%d",operation,id,next_selected,(long long)gold,live_gold,item,(long long)count,live_count);
    return true;
}
Obj hero_action_hook(int operation,int32_t id) {
    Roots roots;
    if (hero_busy.exchange(true)) return complete_local_task(hero_tasks[operation],nullptr,false,roots);
    const bool ok=local_hero_action(operation,id,roots);
    hero_busy.store(false);
    return complete_local_task(hero_tasks[operation],nullptr,ok,roots);
}
Obj hero_equip_hook(int32_t id,Method) { return hero_action_hook(0,id); }
Obj hero_acquire_hook(int32_t id,Method) { return hero_action_hook(1,id); }
Obj hero_level_hook(int32_t id,Method) { return hero_action_hook(2,id); }
Obj hero_star_hook(int32_t id,Method) { return hero_action_hook(3,id); }
