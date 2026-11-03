"""Static ARM64 method trace with Il2CppDumper symbols and metadata handles."""
from pathlib import Path
import json,sys,re,bisect,struct,collections,hashlib
sys.path.insert(0,str(Path(__file__).resolve().parents[1]/'native/python-libs'))
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
ROOT=Path(__file__).resolve().parents[2]
binary=ROOT/'analysis/metadata/libil2cpp.stripes.so';data=binary.read_bytes()
script=json.loads((ROOT/'analysis/il2cpp/script.json').read_text('utf-8'))
with binary.open('rb') as f:
    elf=ELFFile(f);segments=[dict(s.header) for s in elf.iter_segments() if s['p_type']=='PT_LOAD']
    reloc={r['r_offset']:r['r_addend'] for s in elf.iter_sections() if isinstance(s,RelocationSection) for r in s.iter_relocations() if r['r_info_type']==1027}
def off(v):return next(v-s['p_vaddr']+s['p_offset'] for s in segments if s['p_vaddr']<=v<s['p_vaddr']+s['p_filesz'])
def ptr(v):
    try:return reloc.get(v,struct.unpack_from('<Q',data,off(v))[0])
    except Exception:return None
aliases=collections.defaultdict(list)
for m in script['ScriptMethod']:aliases[m['Address']].append(m['Name'])
notes={m['Address']:m['Name'] for key in ['ScriptMetadata','ScriptMetadataMethod'] for m in script[key]}
notes.update({m['Address']:'STRING '+repr(m['Value']) for m in script['ScriptString']})
starts=sorted(aliases);md=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
def decode(v,end):
    regs={};lines=[];tramp=[]
    for i in md.disasm(data[off(v):off(v)+(end-v)],v):
        note='';op=i.op_str.split(', ')
        if i.mnemonic=='adrp':regs[op[0]]=int(op[1][1:],0)
        elif i.mnemonic=='add' and len(op)==3 and op[1] in regs and op[2].startswith('#'):
            regs[op[0]]=regs[op[1]]+int(op[2][1:],0)
        elif i.mnemonic=='ldr':
            m=re.match(r'(\w+), \[(\w+)(?:, #(0x[0-9a-f]+|[0-9]+))?\]',i.op_str)
            if m:
                dst,src,delta=m.groups();base=regs.get(src)
                if base is not None:
                    address=base+(int(delta,0) if delta else 0);value=ptr(address)
                    if value is not None:
                        if address in notes:note=' ; '+notes[address]
                        elif value in notes:note=' ; '+notes[value]+' [handle '+hex(value)+']'
                        regs[dst]=value
                    else:regs.pop(dst,None)
                else:regs.pop(dst,None)
        elif i.mnemonic in ('bl','b') and i.op_str.startswith('#'):
            target=int(i.op_str[1:],0);names=aliases.get(target)
            if names:note=' ; '+' | '.join(names[:3])+(' [shared generic address; check Method handle]' if len(names)>3 else '')
            if i.mnemonic=='b' and not v<=target<end and not names:
                tramp.append(target)
            if i.mnemonic=='bl':
                for rn in [f'x{n}' for n in range(19)]:regs.pop(rn,None)
        elif op and re.match('^[wx][0-9]+$',op[0]):regs.pop(op[0],None)
        lines.append(f'{i.address:08x} {i.bytes.hex():8} {i.mnemonic:8} {i.op_str}{note}')
    return lines,tramp
for name in sys.argv[1:]:
    selected=[m for m in script['ScriptMethod'] if m['Name']==name]
    for m in selected:
        v=m['Address'];end=starts[bisect.bisect_right(starts,v)]
        lines,tramp=decode(v,end)
        lines.insert(0,f'SOURCE {binary} SHA256 {hashlib.sha256(data).hexdigest()}\nMETHOD {name} VA {v:#x} END {end:#x}')
        for t in sorted(set(tramp)):
            extra,_=decode(t,t+64);lines+=['\nEXTERNAL BRANCH '+hex(t)]+extra
        safe=name.replace('$$','.').replace('<','_').replace('>','_').replace('/','_')
        if len(selected)>1:safe+=f'.{v:x}'
        (ROOT/'analysis/metadata'/f'{safe}.txt').write_text('\n'.join(lines),encoding='utf-8')
        print(name,hex(v),end-v)
