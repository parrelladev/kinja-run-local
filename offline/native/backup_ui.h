// Native NGUI widgets cloned from the exact 1.9.1 home menu. Unity thread only.
namespace backup_ui {
struct Color { float r,g,b,a; };
uint32_t home_handle=0, dialog_handle=0;
constexpr const char* storage_path="Content/Page/Main/Top/StoragePanel";
constexpr const char* button_path="Content/Page/Main/Top/StoragePanel/imgStorageBg/btnSetting";

struct Ui {
    Roots roots;
    Klass tools=find_class("","NGUITools"), game=find_class("UnityEngine","GameObject"), transform=find_class("UnityEngine","Transform");
    Klass component=find_class("UnityEngine","Component"), widget=find_class("","UIWidget"), label=find_class("","UILabel"), sprite=find_class("","UISprite");
    Obj keep(Obj o){return o?roots.retain(o):nullptr;}
    Method add_child_method(const std::vector<std::string>& parameters){
        void* iterator=nullptr;Method result=nullptr;
        while(Method m=api.class_get_methods(tools,&iterator)){
            if(std::strcmp(api.method_get_name(m),"AddChild")!=0 || api.method_get_param_count(m)!=parameters.size() || type_name(api.method_get_return_type(m))!="UnityEngine.GameObject")continue;
            bool match=true;for(size_t i=0;i<parameters.size();i++)if(type_name(api.method_get_param(m,static_cast<uint32_t>(i)))!=parameters[i])match=false;
            if(match){if(result)return nullptr;result=m;}
        }
        return result;
    }
    Obj call(Method m,Obj self,void** args,const char* context){Obj o=nullptr;if(!m||!invoke(m,self,args,context,&o))return nullptr;return keep(o);}
    bool run(Method m,Obj self,void** args,const char* context){return m&&invoke(m,self,args,context);}
    Obj string(const char* text){return keep(api.string_new(text));}
    Obj go(Obj t){return call(find_method(component,"get_gameObject",{},true),t,nullptr,"Backup GameObject");}
    Obj trans(Obj o){return call(find_method(game,"get_transform",{}),o,nullptr,"Backup Transform");}
    Obj find(Obj parent,const char* path){Obj text=string(path);void* a[]={text};return call(find_method(transform,"Find",{"System.String"}),parent,a,"Backup child path");}
    Obj comp(Obj o,Klass k){Obj type=keep(api.type_get_object(api.class_get_type(k)));void* a[]={type};return call(find_method(game,"GetComponent",{"System.Type"}),o,a,"Backup component");}
    Obj add_component(Obj o,Klass k){Obj type=keep(api.type_get_object(api.class_get_type(k)));void* a[]={type};return call(find_method(game,"AddComponent",{"System.Type"}),o,a,"Backup add original component");}
    Obj child(Obj parent,const char* name){void* a[]={parent};Obj o=call(add_child_method({"UnityEngine.GameObject"}),nullptr,a,"Backup new child");return o&&rename(o,name)?o:nullptr;}
    bool rename(Obj o,const char* name){Obj text=string(name);void* a[]={text};return run(find_method(find_class("UnityEngine","Object"),"set_name",{"System.String"}),o,a,"Backup object name");}
    bool active(Obj o,bool value){uint8_t b=value;void* a[]={&b};return run(find_method(game,"SetActive",{"System.Boolean"}),o,a,"Backup visibility");}
    bool position(Obj o,float x,float y,float z=0){UnityVector3 v{x,y,z};void* a[]={&v};return run(find_method(transform,"set_localPosition",{"UnityEngine.Vector3"}),trans(o),a,"Backup local position");}
    bool dimensions(Obj w,int width,int height){void* a[]={&width,&height};return run(find_method(widget,"SetDimensions",{"System.Int32","System.Int32"}),w,a,"Backup original widget dimensions");}
    bool depth(Obj w,int n){void* a[]={&n};return run(find_method(widget,"set_depth",{"System.Int32"}),w,a,"Backup original widget depth");}
    bool color(Obj w,Color c){void* a[]={&c};return run(find_method(widget,"set_color",{"UnityEngine.Color"}),w,a,"Backup widget color");}
    bool int_property(Obj o,const char* method,int n){void* a[]={&n};return run(find_method(api.object_get_class(o),method,{"System.Int32"},true),o,a,"Backup original integer property");}
    bool collider(Obj o){
        Klass box=find_class("UnityEngine","BoxCollider");
        if(!comp(o,box) && !add_component(o,box))return false;
        uint8_t inactive=1;void* a[]={o,&inactive};return run(find_method(tools,"UpdateWidgetCollider",{"UnityEngine.GameObject","System.Boolean"}),nullptr,a,"Backup original collider bounds");
    }
    Obj clone(Obj parent,Obj source,const char* name){void* a[]={parent,source};Obj o=call(add_child_method({"UnityEngine.GameObject","UnityEngine.GameObject"}),nullptr,a,"Backup clone original widget");return o&&rename(o,name)?o:nullptr;}
    bool text(Obj w,const char* value,int size=24){
        Field no_translate=api.class_get_field_from_name(label,"WithoutTranslate");uint8_t yes=1;
        if(no_translate && type_name(api.field_get_type(no_translate))=="System.Boolean")api.field_set_value(w,no_translate,&yes);
        // Awake copies a nonempty language key back over cloned text on first activation.
        // Use original reflection SetValue for this reference field; no object offsets.
        Obj type=keep(api.type_get_object(api.class_get_type(label))),key=string("mLanguageKey");void* fa[]={key};
        Obj field=call(find_method(api.object_get_class(type),"GetField",{"System.String"},true),type,fa,"Backup cloned label language field");
        Obj empty=string("");void* set_field[]={w,empty};
        if(!field || !run(find_method(api.object_get_class(field),"SetValue",{"System.Object","System.Object"},true),field,set_field,"Backup clear cloned translation key"))return false;
        Obj s=string(value);uint8_t force=1;void* a[]={s,&force};return run(find_method(label,"SetText",{"System.String","System.Boolean"}),w,a,"Backup original forced label text")&&int_property(w,"set_fontSize",size);
    }
    bool bind(Obj button,Obj home){
        Klass listener=find_class("","UIEventListener"),delegates=find_class("System","Delegate");
        Method get=find_method(listener,"Get",{"UnityEngine.GameObject"});void* a[]={button};Obj l=call(get,nullptr,a,"Backup original click listener");
        Field callback=api.class_get_field_from_name(listener,"_onClick");
        if(!l || !callback)return false;
        Type delegate_type=api.field_get_type(callback);
        Obj reflection_type=keep(api.type_get_object(delegate_type)),method_name=string("OnClickStorageBg");
        void* args[]={reflection_type,home,method_name};
        Obj d=call(find_method(delegates,"CreateDelegate",{"System.Type","System.Object","System.String"}),nullptr,args,"Backup bind original managed method");
        if(!d)return false;
        Method setter=find_method(listener,"set_onClick",{type_name(delegate_type)});void* set_args[]={d};
        return run(setter,l,set_args,"Backup original onClick setter");
    }
    Obj add_sprite(Obj parent,Obj atlas,const char* name,const char* sprite_name,int width,int height,int order,float x,float y,Color tint={1,1,1,1}){
        Obj o=child(parent,name);if(!o)return nullptr;
        Obj s=string(sprite_name);void* a[]={o,atlas,s,&order};
        Obj w=call(find_method(tools,"AddSprite",{"UnityEngine.GameObject","INGUIAtlas","System.String","System.Int32"}),nullptr,a,"Backup original atlas sprite");
        if(!w || !dimensions(w,width,height) || !color(w,tint) || !position(o,x,y))return nullptr;
        if(std::strncmp(sprite_name,"common_bg",9)==0){int sliced=1;void* sa[]={&sliced};if(!run(find_method(find_class("","UIBasicSprite"),"set_type",{"UIBasicSprite.Type"}),w,sa,"Backup sliced original frame"))return nullptr;}
        return o;
    }
    Obj add_label(Obj parent,Obj template_label,const char* name,const char* value,int width,int height,int size,int order,float x,float y,bool plain=false){
        Obj o=clone(parent,template_label,name);if(!o)return nullptr;
        Obj w=comp(o,label);if(!w || !text(w,value,size) || !dimensions(w,width,height) || !depth(w,order) || !position(o,x,y))return nullptr;
        int center=2;void* a[]={&center};
        if(!run(find_method(label,"set_alignment",{"NGUIText.Alignment"}),w,a,"Backup centered original label"))return nullptr;
        if(plain){int effect=0;void* e[]={&effect};if(!run(find_method(label,"set_effectStyle",{"UILabel.Effect"}),w,e,"Backup body label style") || !color(w,{0.09f,0.13f,0.22f,1}))return nullptr;}
        return o;
    }
    bool sprite_name(Obj o,const char* value){Obj w=comp(o,sprite),s=string(value);void* a[]={s};return w&&run(find_method(sprite,"set_spriteName",{"System.String"}),w,a,"Backup original sprite selection");}
    bool button(Obj parent,Obj template_button,Obj home,const char* name,const char* caption,const char* background,float x,float y,int width=480,int height=108){
        Obj o=clone(parent,template_button,name);if(!o)return false;
        Obj w=comp(o,widget),t=trans(o),label_t=find(t,"txtSetting"),icon=find(t,"imgPower (1)");
        if(!w || !label_t || !position(o,x,y) || !sprite_name(o,background) || !dimensions(w,width,height) || !depth(w,30) || !collider(o) || !bind(o,home))return false;
        Obj label_go=go(label_t),lw=comp(label_go,label);
        if(!lw || !text(lw,caption,36) || !dimensions(lw,width-110,height-12) || !depth(lw,32) || !position(label_go,18,6))return false;
        if(icon){Obj icon_go=go(icon);if(!sprite_name(icon_go,name[14]=='E'?"rank_up":"rank_down") || !position(icon_go,-width*0.36f,5))return false;Obj iw=comp(icon_go,widget);if(!dimensions(iw,40,40) || !depth(iw,32))return false;}
        return active(o,true);
    }
};

bool bring_front(Ui& ui,Obj dialog){
    Klass panels=find_class("","UIPanel"),objects=find_class("UnityEngine","Object"),arrays=find_class("System","Array");
    Obj own=ui.comp(dialog,panels),type=ui.keep(api.type_get_object(api.class_get_type(panels)));void* args[]={type};
    Obj all=ui.call(find_method(objects,"FindObjectsOfType",{"System.Type"}),nullptr,args,"Backup active original panels");
    Obj boxed=all?ui.call(find_method(arrays,"get_Length",{}),all,nullptr,"Backup panel count"):nullptr;
    auto* length=boxed?static_cast<int32_t*>(api.object_unbox(boxed)):nullptr;
    if(!own || !length || *length<0 || *length>4096)return false;
    Field sorting=api.class_get_field_from_name(panels,"useSortingOrder");uint8_t enabled=1;
    if(!sorting || type_name(api.field_get_type(sorting))!="System.Boolean")return false;
    api.field_set_value(own,sorting,&enabled);
    int depth=0,order=0;
    for(int i=0;i<*length;i++){
        void* index[]={&i};Obj p=ui.call(find_method(arrays,"GetValue",{"System.Int32"}),all,index,"Backup original panel item");
        if(!p || p==own)continue;
        Obj bd=ui.call(find_method(panels,"get_depth",{}),p,nullptr,"Backup existing panel depth");
        Obj bo=ui.call(find_method(panels,"get_sortingOrder",{}),p,nullptr,"Backup existing panel order");
        if(!bd || !bo)return false;
        auto* d=static_cast<int32_t*>(api.object_unbox(bd));auto* o=static_cast<int32_t*>(api.object_unbox(bo));
        if(!d || !o)return false;depth=std::max(depth,*d);order=std::max(order,*o);
    }
    if(depth>1000000000 || order>1000000000)return false;depth+=100;order+=100;
    void* da[]={&depth};void* oa[]={&order};
    return ui.run(find_method(panels,"set_depth",{"System.Int32"}),own,da,"Backup modal front depth") &&
        ui.run(find_method(panels,"set_sortingOrder",{"System.Int32"}),own,oa,"Backup modal front sorting order");
}

bool initialize(Obj home){
    Ui ui;
    Obj root=ui.call(find_method(ui.component,"get_transform",{},true),home,nullptr,"Backup home transform");
    Obj source_t=root?ui.find(root,button_path):nullptr;if(!source_t)return false;
    Obj bg_t=ui.find(root,"Content/Page/Main/Top/StoragePanel/imgStorageBg");
    if(ui.find(bg_t,"OfflineBackup.Open"))return true;
    Obj source=ui.go(source_t),background=ui.go(bg_t);
    Obj clone=ui.clone(background,source,"OfflineBackup.Open");if(!clone)return false;
    Obj label_t=ui.find(ui.trans(clone),"txtSetting"),icon_t=ui.find(ui.trans(clone),"imgPower (1)");
    if(!label_t || !ui.text(ui.comp(ui.go(label_t),ui.label),"Backup") || !ui.bind(clone,home) || !ui.position(clone,2.193817f,45.3f))return false;
    if(icon_t && !ui.sprite_name(ui.go(icon_t),"img_load"))return false;
    if(!ui.dimensions(ui.comp(background,ui.widget),276,246) || !ui.active(clone,true))return false;
    // Expand around the original center; preserve original Config button and its event binding.
    Obj atlas=ui.call(find_method(ui.sprite,"get_atlas",{}),ui.comp(source,ui.sprite),nullptr,"Backup original Common_N atlas");
    Obj label_template=ui.go(ui.find(source_t,"txtSetting")),home_go=ui.go(root),dialog=ui.child(home_go,"OfflineBackup.Window");
    if(!atlas || !label_template || !dialog)return false;
    Obj panel=ui.add_component(dialog,find_class("","UIPanel"));int panel_depth=250;void* pd[]={&panel_depth};
    if(!panel || !ui.run(find_method(api.object_get_class(panel),"set_depth",{"System.Int32"}),panel,pd,"Backup original modal panel depth") || !ui.position(dialog,0,0,-300))return false;
    Obj mask=ui.add_sprite(dialog,atlas,"OfflineBackup.Close","common_bg1",4096,4096,0,0,0,{0,0,0,0.65f});
    if(!mask || !ui.collider(mask) || !ui.bind(mask,home))return false;
    Obj outer=ui.add_sprite(dialog,atlas,"Frame","common_bg1",592,850,10,0,0,{0.52f,0.52f,0.57f,1});
    Obj inner=ui.add_sprite(dialog,atlas,"Body","common_bg1",548,704,11,0,-43,{0.80f,0.91f,1,1});
    if(!outer || !inner || !ui.collider(outer))return false;
    if(!ui.add_label(dialog,label_template,"Title","BACKUP",500,80,48,20,0,352))return false;
    if(!ui.add_sprite(dialog,atlas,"RestoreIcon","img_load",120,90,20,0,225))return false;
    if(!ui.add_label(dialog,label_template,"Description","Salve e restaure seus dados\nde progresso.",480,98,28,20,0,125,true))return false;
    if(!ui.button(dialog,source,home,"OfflineBackup.Export","Exportar","bg_houpaihang2",0,-6))return false;
    if(!ui.button(dialog,source,home,"OfflineBackup.Import","Importar","bg_houpaihang3",0,-145))return false;
    if(!ui.add_label(dialog,label_template,"Footer","Exporte um backup para salvar seu\nprogresso e importe para restaurá-lo.",470,96,23,20,0,-294,true))return false;
    Obj close=ui.add_label(dialog,label_template,"OfflineBackup.Close","X",50,60,32,40,248,356);
    if(!close || !ui.collider(close) || !ui.bind(close,home) || !ui.active(dialog,false))return false;
    if(home_handle)api.gchandle_free(home_handle);if(dialog_handle)api.gchandle_free(dialog_handle);
    home_handle=api.gchandle_new(home,false);dialog_handle=api.gchandle_new(dialog,false);
    LOGI("Native backup NGUI menu and modal initialized from original StoragePanel widgets");
    return true;
}

void close(){Ui ui;Obj dialog=dialog_handle?ui.keep(api.gchandle_get_target(dialog_handle)):nullptr;if(dialog)ui.active(dialog,false);const std::string p="0\n";backup_java(backup_action,&p);}
void click(Obj home,Obj go,Method /*method*/){
    Ui ui;Obj name=go?ui.call(find_method(find_class("UnityEngine","Object"),"get_name",{}),go,nullptr,"Backup clicked object"):nullptr;
    std::string action;if(name){const uint16_t* chars=api.string_chars(name);for(int i=0;i<api.string_length(name);i++)if(chars[i]<128)action+=static_cast<char>(chars[i]);}
    if(action=="OfflineBackup.Open"){
        Obj root=ui.call(find_method(ui.component,"get_transform",{},true),home,nullptr,"Backup menu home");
        Obj storage=root?ui.find(root,storage_path):nullptr;if(storage)ui.active(ui.go(storage),false);
        Obj dialog=dialog_handle?ui.keep(api.gchandle_get_target(dialog_handle)):nullptr;
        if(dialog && ui.active(dialog,true) && bring_front(ui,dialog)){const std::string p="1\n";backup_java(backup_action,&p);LOGI("Native backup window opened");}
    }else if(action=="OfflineBackup.Close")close();
    else if(action=="OfflineBackup.Export"){
        std::string prefs;if(logical_backup_preferences(prefs)){const std::string payload="2\n"+prefs;backup_java(backup_action,&payload);}
        else {const std::string payload="4\nNão foi possível capturar o progresso do jogo.";backup_java(backup_action,&payload);}
    }else if(action=="OfflineBackup.Import"){const std::string p="3\n";backup_java(backup_action,&p);}
    else {
        // Original 1.9.1 OnClickStorageBg: GetGameObject(StoragePanel).SetActive(false).
        Obj root=ui.call(find_method(ui.component,"get_transform",{},true),home,nullptr,"Original storage close home");
        Obj storage=root?ui.find(root,storage_path):nullptr;if(storage)ui.active(ui.go(storage),false);
    }
}
} // namespace backup_ui
