"""Disassemble selected original game methods and label resolved static calls."""
import argparse,json,pathlib,sys
sys.path.insert(0,str(pathlib.Path(__file__).parent/'python-libs'))
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
from elftools.elf.elffile import ELFFile
p=argparse.ArgumentParser();p.add_argument('binary');p.add_argument('--class-name',required=True);p.add_argument('--method');p.add_argument('--out');a=p.parse_args()
mapping=json.loads(pathlib.Path('analysis/native/game-method-map.json').read_text(encoding='utf8'))
byaddr={};starts=set()
for m in mapping:
    v=int(m['native_address'],16)
    if v:starts.add(v);byaddr.setdefault(v,[]).append(m['class_name']+'.'+m['name'])
starts=sorted(starts);d=pathlib.Path(a.binary).read_bytes()
with open(a.binary,'rb') as f:segs=[dict(s.header) for s in ELFFile(f).iter_segments() if s['p_type']=='PT_LOAD']
def off(v):
    for s in segs:
        if s['p_vaddr']<=v<s['p_vaddr']+s['p_filesz']:return v-s['p_vaddr']+s['p_offset']
    raise ValueError(hex(v))
md=Cs(CS_ARCH_ARM64,CS_MODE_ARM);lines=[]
for m in mapping:
    if m['class_name']!=a.class_name or (a.method and m['name']!=a.method):continue
    v=int(m['native_address'],16)
    if not v:continue
    end=next((s for s in starts if s>v),v+4096);n=min(end-v,10000)
    lines.append('\n### '+m['class_name']+'.'+m['name']+' token '+m['token']+' VA '+hex(v)+' file '+hex(off(v)))
    for i in md.disasm(d[off(v):off(v)+n],v):
        note=''
        if i.mnemonic in ['bl','b'] and i.op_str.startswith('#'):
            target=int(i.op_str[1:],0);names=byaddr.get(target)
            if names:note=' ; '+', '.join(names[:5])
        lines.append(f'{i.address:08x}  {i.bytes.hex()}  {i.mnemonic:<8} {i.op_str}{note}')
text='\n'.join(lines)
if a.out:pathlib.Path(a.out).parent.mkdir(parents=True,exist_ok=True);pathlib.Path(a.out).write_text(text,encoding='utf8')
else:print(text)
