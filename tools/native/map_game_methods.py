"""Map recovered metadata method tokens to native pointers without generic table parsing."""
import hashlib,json,pathlib,struct,sys
sys.path.insert(0,str(pathlib.Path(__file__).parent/'python-libs'))
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
source=pathlib.Path(sys.argv[1]);data=source.read_bytes()
with source.open('rb') as f:
    elf=ELFFile(f);segs=[dict(s.header) for s in elf.iter_segments() if s['p_type']=='PT_LOAD']
    rel={int(r['r_offset']):int(r['r_addend']) for s in elf.iter_sections() if isinstance(s,RelocationSection) for r in s.iter_relocations() if r['r_info_type']==1027}
def off(v):
    for s in segs:
        if s['p_vaddr']<=v<s['p_vaddr']+s['p_filesz']:return v-s['p_vaddr']+s['p_offset']
    raise ValueError(hex(v))
def ptr(v):
    if v in rel:return rel[v]
    try:return struct.unpack_from('<Q',data,off(v))[0]
    except ValueError:return 0
def cstr(v):
    q=off(v);e=data.find(b'\0',q,q+300);return data[q:e if e>=q else q+100].decode('utf8',errors='replace')
modules={}; summary=[]
for i in range(119):
    m=ptr(0x492abf0+i*8); name=cstr(ptr(m));n=ptr(m+8);p=ptr(m+16)
    summary.append(dict(index=i,addr=hex(m),name=name,count=n,method_pointers=hex(p)))
    if n<200000 and p:modules[name]=[ptr(p+j*8) for j in range(n)]
types=json.loads(pathlib.Path('analysis/metadata/game-types.json').read_text(encoding='utf8'))
mapped=[]
for t in types:
    arr=modules.get(t['image'],[])
    for meth in t['methods']:
        rid=int(meth['token'],16)&0xffffff
        addr=arr[rid-1] if 0<rid<=len(arr) else 0
        mapped.append(dict(image=t['image'],type_index=t['type_index'],class_name=(t['namespace']+'.' if t['namespace'] else '')+t['name'],**meth,native_address=hex(addr),file_offset=hex(off(addr)) if addr else None))
out=pathlib.Path('analysis/native');out.mkdir(exist_ok=True,parents=True)
(out/'module-map.json').write_text(json.dumps(summary,indent=2),encoding='utf8')
(out/'game-method-map.json').write_text(json.dumps(mapped,indent=2,ensure_ascii=False),encoding='utf8')
print(json.dumps(dict(source_sha256=hashlib.sha256(data).hexdigest(),modules=len(modules),methods=len(mapped),nonzero=sum(x['native_address']!='0x0' for x in mapped))))
for m in mapped:
    if m['class_name'] in ['Init','PlayerDataManager','ET.LoginHelper']:
        print(m['class_name'],m['name'],m['token'],m['native_address'])
