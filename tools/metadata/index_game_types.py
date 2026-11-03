"""Export all game metadata names and structural method/field relationships."""
from pathlib import Path
import json,struct,re
root=Path(__file__).resolve().parents[2]
b=(root/'analysis/metadata/global-metadata.recovered.dat').read_bytes()
q=lambda pos:struct.unpack_from('<II',b,pos)
table=lambda header:b[q(header)[0]:sum(q(header))]
names=table(24)
def name(i):
    return names[i:names.find(b'\0',i)].decode('utf-8','replace')
types=list(struct.iter_unpack('<16I8H2I',table(160)))
methods=list(struct.iter_unpack('<IIIIIIHHHH',table(48)))
fields=list(struct.iter_unpack('<III',table(96)))
images=list(struct.iter_unpack('<10I',table(168)))
result=[]
focus=[]
pat=re.compile(r'init|startup|login|account|player|user|network|http|socket|hotupdate|version|fight|battle|game$',re.I)
selected=('Assembly-CSharp.dll','AccountInterface.dll','HabbyAccountLib.dll','MetaGameLib.dll','SQLiteLib.dll','HabbyToolLib.dll','HabbyMetaGameInterface.dll')
for image_index,image in enumerate(images):
    image_name=name(image[0])
    if image_name not in selected:continue
    for type_index in range(image[2],image[2]+image[3]):
        t=types[type_index]
        record={'image':image_name,'image_index':image_index,'type_index':type_index,
                'name':name(t[0]),'namespace':name(t[1]),'token':hex(t[-1]),
                'byval_type_index':t[2],'method_start':t[9],'field_start':t[8],
                'parent_type_index':t[4],'declaring_type_index':t[3],
                'methods':[],'fields':[]}
        for i in range(t[9],t[9]+t[16]):
            m=methods[i]
            record['methods'].append({'index':i,'name':name(m[0]),'token':hex(m[5]),
                                      'return_type_index':m[2],'parameter_start':m[3],
                                      'parameter_count':m[9],'flags':hex(m[6])})
        for i in range(t[8],t[8]+t[18]):
            f=fields[i]
            record['fields'].append({'index':i,'name':name(f[0]),'type_index':f[1],'token':hex(f[2])})
        result.append(record)
        if pat.search(record['name']) and not record['name'].startswith('<'):focus.append(record)
out=root/'analysis/metadata'
(out/'game-types.json').write_text(json.dumps(result,ensure_ascii=False,indent=2),encoding='utf-8')
(out/'startup-types.json').write_text(json.dumps(focus,ensure_ascii=False,indent=2),encoding='utf-8')
lines=['# Startup and service metadata candidates','',
       'Names and relationships recovered from APK metadata. Method bodies and native addresses are not established by this index.','']
for r in focus:
    lines.append(f"## {r['namespace']}.{r['name']} (type {r['type_index']}, {r['image']})")
    lines.append('')
    lines.append('Fields: '+', '.join(f["name"] for f in r['fields']))
    lines.append('')
    lines.extend(f"- {m['name']} (method {m['index']}, token {m['token']}, {m['parameter_count']} parameters)" for m in r['methods'])
    lines.append('')
(out/'startup-types.md').write_text('\n'.join(lines),encoding='utf-8')
print(json.dumps({'selected_types':len(result),'focused_types':len(focus),'output':str(out)}))
