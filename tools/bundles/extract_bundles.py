"""Read-only Unity bundle inventory and extracted TextAssets from the supplied APK.

Never loads .NET classes or runs game code. All output is under analysis/bundles.
"""
import sys, json, hashlib, collections, pathlib
sys.path.insert(0, str(pathlib.Path(__file__).parent / 'python-libs'))
import UnityPy

ROOT = pathlib.Path(r'C:\Users\parrella\AppData\Local\Temp\rea-extracted-df1badd0-2245-4966-b856-d921c70678f9')
OUT = pathlib.Path(__file__).parents[2] / 'analysis' / 'bundles'
OUT.mkdir(parents=True, exist_ok=True)

def plain(obj):
    if isinstance(obj, bytes): return {'bytes': len(obj), 'sha256': hashlib.sha256(obj).hexdigest()}
    if isinstance(obj, dict): return {str(k):plain(v) for k,v in obj.items()}
    if isinstance(obj, (list,tuple)): return [plain(v) for v in obj]
    if isinstance(obj, (str,int,float,bool)) or obj is None: return obj
    return str(obj)

def run(paths):
    records=json.loads((OUT/'inventory.json').read_text()) if (OUT/'inventory.json').exists() else []
    for p in paths:
        try: label=str(p.relative_to(ROOT)).replace('\\','/')
        except ValueError: label='assets/bin/Data/'+p.name+' [joined split0 + split1]'
        item={'apk_path':label,'size':p.stat().st_size,'sha256':hashlib.sha256(p.read_bytes()).hexdigest()}
        try:
            env=UnityPy.load(str(p))
            item['containers'] = list(env.container.keys())
            item['objects']=[]
            for obj in env.objects:
                row={'path_id':obj.path_id,'type':obj.type.name,'serialized_file':obj.assets_file.name,'offset_in_serialized_file':obj.byte_start,'size':obj.byte_size}
                if obj.type.name in ('TextAsset','MonoScript','GameObject','MonoBehaviour','AssetBundle','ResourceManager'):
                    try:
                        data=obj.read()
                        row['name']=getattr(data,'m_Name',None)
                        if obj.type.name=='TextAsset':
                            raw=data.m_Script.encode('utf-8','surrogateescape') if isinstance(data.m_Script,str) else data.m_Script
                            row['payload_size']=len(raw)
                            row['payload_sha256']=hashlib.sha256(raw).hexdigest()
                            target=OUT/'textassets'/p.name/(str(obj.path_id)+'_'+row['name']+'.bytes')
                            target.parent.mkdir(parents=True,exist_ok=True)
                            target.write_bytes(raw)
                            row['extracted_payload']=str(target.relative_to(OUT)).replace('\\','/')
                        elif obj.type.name=='MonoScript':
                            row['class']=data.m_ClassName
                            row['namespace']=data.m_Namespace
                            row['assembly']=data.m_AssemblyName
                        elif obj.type.name=='MonoBehaviour':
                            try: row['tree']=plain(obj.read_typetree())
                            except Exception as err: row['tree_error']=str(err)
                        elif obj.type.name=='AssetBundle':
                            row['bundle_name']=data.m_Name
                    except Exception as err: row['error']=str(err)
                item['objects'].append(row)
            item['counts']=dict(collections.Counter(o['type'] for o in item['objects']))
            print(p.name, item['counts'], flush=True)
        except Exception as err:
            item['error']=str(err)
            print(p.name,'ERROR',err,flush=True)
        records=[r for r in records if r['apk_path']!=item['apk_path']]
        records.append(item)
        (OUT/'inventory.json').write_text(json.dumps(records,ensure_ascii=True,indent=2),encoding='utf-8')

if __name__=='__main__':
    bundles=ROOT/'assets'/'AssetBundles'/'Android'
    selected=sorted(p for p in bundles.glob('*.ht') if p.name.startswith(('clientdata','clientleveldata','clientresourcesdatabase','clientloading','clientmain','clientrelink')))
    selected += [ROOT/'assets'/'bin'/'Data'/name for name in ['globalgamemanagers','level0','level1','sharedassets0.assets','sharedassets1.assets'] if (ROOT/'assets'/'bin'/'Data'/name).exists()]
    run(selected)
