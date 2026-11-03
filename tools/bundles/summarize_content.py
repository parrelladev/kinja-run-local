"""Create original-content evidence summary and tutorial dependency closure."""
import json, pathlib, collections
BASE=pathlib.Path(__file__).parents[2]/'analysis'/'bundles'
ROOT=pathlib.Path(r'C:\Users\parrella\AppData\Local\Temp\rea-extracted-df1badd0-2245-4966-b856-d921c70678f9')
inventory=json.loads((BASE/'inventory.json').read_text())
tables=json.loads((BASE/'table_index.json').read_text())
resource_bundle=next(i for i in inventory if i['apk_path'].endswith('clientresourcesdatabase.ht'))
resources=next(o['tree']['databases'] for o in resource_bundle['objects'] if 'tree' in o)
(BASE/'asset_database.json').write_text(json.dumps(resources,ensure_ascii=True,indent=2),encoding='utf-8')
bundle_names={p.name.lower() for p in (ROOT/'assets'/'AssetBundles'/'Android').iterdir() if p.is_file()}
refs={name.lower() for d in resources for name in [d['AssetBundleName']]+d['DepedencyAssetBundleNames']}
by_alias={d['AliasFilePath']:d for d in resources}
by_path={d['FilePath']:d for d in resources}
def table(suffix):
    p=next((BASE/'tables'/'clientdata.ht').glob('*_'+suffix+'.json'))
    return p,list(json.loads(p.read_text()))
def clean(v):
    if isinstance(v,dict):return {k:clean(x) for k,x in v.items() if not k.startswith('$')}
    if isinstance(v,list):return [clean(x) for x in v]
    return v
region_file,regions=table('GameRegionCfgData'); region=next(x for x in regions if x['id']==1)
stage_file,stages=table('GameStageCfgData'); stage=next(x for x in stages if x['id']==region['array'][0])
group_file,groups=table('BrickRandomGroupCfgCfgData'); selected_groups=[x for x in groups if x['id'] in stage['block_group']]
matrix_file,matrix=table('BlockMatrixCfgCfgData'); matrix_ids={x for g in selected_groups if g['use_matrix'] for x in g['bricks']}
selected_matrix=[x for x in matrix if x['id'] in matrix_ids]
brick_ids={stage[k] for k in ['enter_block','finish_block','start_block','loop_block','boss_block','merge_block'] if stage[k]}
brick_ids|={x for g in selected_groups if not g['use_matrix'] for x in g['bricks']}
brick_ids|={x for g in selected_groups if g['boss_brick'] for x in [g['boss_brick']]}
brick_ids|={x for m in selected_matrix for x in m['bricks']}
text_index={o.get('name'):(i,o) for i in inventory for o in i.get('objects',[]) if o['type']=='TextAsset'}
payload_to_json={t['payload']:t.get('json') for t in tables}
bricks=[]; widget_paths=set()
for bid in sorted(brick_ids):
    found=text_index.get(str(bid))
    if not found:bricks.append({'uid':bid,'missing':True});continue
    bundle,obj=found; jp=payload_to_json[obj['extracted_payload']]
    data=json.loads((BASE/jp).read_text()); widgets=data.get('Data') or []
    paths={w['WidgetSourcePath'] for w in widgets if w.get('WidgetSourcePath')}
    widget_paths|=paths
    bricks.append({'uid':bid,'bundle':bundle['apk_path'],'path_id':obj['path_id'],'offset_in_serialized_file':obj['offset_in_serialized_file'],'json':jp,'widget_count':len(widgets),'widget_source_paths':sorted(paths)})
resolved=[by_path[p] for p in sorted(widget_paths) if p in by_path]
missing_widget_paths=sorted(widget_paths-set(by_path))
seed={'status':'static evidence only; not an executable offline profile','birth':clean(table('BirthCfgData')[1]),'region':clean(region),'stage':clean(stage),'brick_groups':clean(selected_groups),'matrix':clean(selected_matrix),'bricks':bricks,'widget_assets':resolved,'unresolved_widget_paths':missing_widget_paths}
(BASE/'tutorial_content_seed.json').write_text(json.dumps(seed,ensure_ascii=True,indent=2),encoding='utf-8')
summary={'inventory_files':len(inventory),'textassets':sum(i.get('counts',{}).get('TextAsset',0) for i in inventory),'tables_parsed':len(tables),'table_errors':[{k:v for k,v in i.items() if k in ['payload','error','consumed','payload_size']} for i in tables if 'error'in i],'tables_with_full_byte_consumption':sum(i.get('consumed')==i.get('payload_size') for i in tables),'clientdata_tables':sum('clientdata.ht/' in i['payload'] for i in tables),'asset_database_entries':len(resources),'referenced_bundle_names':len(refs),'missing_bundles':sorted(refs-bundle_names),'tutorial_bricks':len(bricks),'tutorial_missing_bricks':[i for i in bricks if i.get('missing')],'tutorial_unresolved_widget_paths':missing_widget_paths,'classes':dict(collections.Counter(c for i in tables for c in i.get('classes',[])))}
(BASE/'summary.json').write_text(json.dumps(summary,ensure_ascii=True,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in summary.items() if k!='classes'},indent=2))
