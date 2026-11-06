"""Inspect the exact original UI prefabs; output only to a caller-selected local report."""
import argparse, json, pathlib, sys, zipfile

p = argparse.ArgumentParser()
p.add_argument('apk', type=pathlib.Path)
p.add_argument('--unitypy', type=pathlib.Path, required=True)
p.add_argument('--output', type=pathlib.Path, required=True)
a = p.parse_args()
sys.path.insert(0,str(a.unitypy))
import UnityPy

reports = {}
with zipfile.ZipFile(a.apk) as apk:
    for bundle in ['clientgameobjectsui78.ht','clientgameobjectsui5f.ht']:
        env = UnityPy.load(apk.read('assets/AssetBundles/Android/'+bundle))
        objects = {o.path_id:o for o in env.objects}
        trees = {pid:o.read_typetree() for pid,o in objects.items() if o.type.name in ['GameObject','Transform','MonoBehaviour']}
        transforms = {pid:t for pid,t in trees.items() if objects[pid].type.name=='Transform'}
        def path(pid):
            t = transforms[pid]
            parent = t['m_Father']['m_PathID']
            name = trees[t['m_GameObject']['m_PathID']]['m_Name']
            return (path(parent)+'/' if parent in transforms else '')+name
        rows = []
        for pid,t in transforms.items():
            go = trees[t['m_GameObject']['m_PathID']]
            rows.append(dict(path=path(pid),active=go['m_IsActive'],transform=t,
                components=[dict(id=c['component']['m_PathID'],data=trees.get(c['component']['m_PathID'])) for c in go['m_Component']]))
        reports[bundle] = rows
a.output.parent.mkdir(parents=True,exist_ok=True)
a.output.write_text(json.dumps(reports,indent=2),encoding='utf-8')
for bundle,rows in reports.items():
    print(bundle)
    for row in rows:
        if bundle.endswith('5f.ht') or any(k in row['path'].lower() for k in ['setting','shrink']):
            print(row['path'], 'active='+str(row['active']), 'components='+str([c['id'] for c in row['components']]))
