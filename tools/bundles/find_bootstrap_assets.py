"""Bounded static scan of shipped scene and GameObject bundles for local/dev hooks."""
import pathlib,sys,json,re
sys.path.insert(0,str(pathlib.Path(__file__).parent/'python-libs'))
import UnityPy
from extract_bundles import ROOT,OUT,plain
desired={'LittleLevelTestIniter','LittleLevelTest','BulletShooterTest','WeaponTest','ServerConnect'}
term=re.compile(r'offline|server|isdebug|testmode|localmode|mock|useTLS|ProductionEnvironment',re.I)
bundles=ROOT/'assets'/'AssetBundles'/'Android'
db=json.loads((OUT/'asset_database.json').read_text())
scene_bundles={d['AssetBundleName'] for d in db if d['RootAssetName'].endswith('.unity')}
paths=sorted(p for p in bundles.glob('*.ht') if p.name.startswith('clientgameobjects') or p.name in scene_bundles)
report={'scope':'all clientgameobjects*.ht plus every .unity bundle in the original resource database','scanned':[],'script_matches':[],'component_matches':[],'errors':[]}
for n,p in enumerate(paths):
    try:
        env=UnityPy.load(str(p)); objects=list(env.objects)
        scripts={}
        for o in objects:
            if o.type.name=='MonoScript':
                s=o.read(); scripts[o.path_id]=s.m_ClassName
                if s.m_ClassName in desired:
                    report['script_matches'].append({'bundle':p.name,'file':o.assets_file.name,'path_id':o.path_id,'offset':o.byte_start,'class':s.m_ClassName})
        for o in objects:
            if o.type.name!='MonoBehaviour':continue
            children=o.serialized_type.node.m_Children if o.serialized_type and o.serialized_type.node else []
            field_matches=[c.m_Name for c in children if term.search(c.m_Name)]
            if field_matches or set(scripts.values())&desired:
                tree=o.read_typetree()
                script=tree.get('m_Script',{}); class_name=scripts.get(script.get('m_PathID'))
                if field_matches or class_name in desired:
                    report['component_matches'].append({'bundle':p.name,'file':o.assets_file.name,'path_id':o.path_id,'offset':o.byte_start,'size':o.byte_size,'class':class_name,'fields':field_matches,'tree':plain(tree)})
        report['scanned'].append(p.name)
    except Exception as err:report['errors'].append({'bundle':p.name,'error':str(err)})
    if n%30==0:print(n+1,'of',len(paths),p.name,flush=True)
    (OUT/'bootstrap_asset_scan.json').write_text(json.dumps(report,indent=2,ensure_ascii=True),encoding='utf-8')
print('COMPLETE',len(report['scanned']),'matches',len(report['script_matches']),len(report['component_matches']),'errors',len(report['errors']))
