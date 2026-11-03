"""Disassemble restored client bootstrap with metadata-handle notes, without execution."""
import pathlib,json,sys,re,bisect,struct,collections,hashlib
sys.path.insert(0,str(pathlib.Path(__file__).parents[1]/'native'/'python-libs'))
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
BASE=pathlib.Path(__file__).parents[2]
binary=BASE/'analysis'/'metadata'/'libil2cpp.stripes.so'
script=json.loads((BASE/'analysis'/'il2cpp'/'script.json').read_text(encoding='utf-8'))
data=binary.read_bytes()
with binary.open('rb') as f:
    elf=ELFFile(f); segs=[dict(s.header) for s in elf.iter_segments() if s['p_type']=='PT_LOAD']
    reloc={int(r['r_offset']):int(r['r_addend']) for s in elf.iter_sections() if isinstance(s,RelocationSection) for r in s.iter_relocations() if r['r_info_type']==1027}
def off(v):
    return next(v-s['p_vaddr']+s['p_offset'] for s in segs if s['p_vaddr']<=v<s['p_vaddr']+s['p_filesz'])
def ptr(v):
    try:return reloc.get(v,struct.unpack_from('<Q',data,off(v))[0])
    except Exception:return None
aliases=collections.defaultdict(list)
for m in script['ScriptMethod']:aliases[m['Address']].append(m['Name'])
notes={m['Address']:m['Name'] for key in ['ScriptMetadata','ScriptMetadataMethod'] for m in script[key]}
notes.update({m['Address']:'STRING '+repr(m['Value']) for m in script['ScriptString']})
starts=sorted(aliases)
md=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
desired=sys.argv[1:]
for name in desired:
    selected=[m for m in script['ScriptMethod'] if m['Name']==name]
    for meth in selected:
        v=meth['Address']; end=starts[bisect.bisect_right(starts,v)]
        lines=[f"SOURCE {binary} SHA256 {hashlib.sha256(data).hexdigest()}",f"### {name} address {hex(v)} length {end-v}"]
        regs={}
        for i in md.disasm(data[off(v):off(end)],v):
            note=''; operands=i.op_str.split(', ')
            if i.mnemonic=='adrp':regs[operands[0]]=int(operands[1].lstrip('#'),0)
            elif i.mnemonic=='add' and len(operands)==3 and operands[1] in regs and operands[2].startswith('#'):
                regs[operands[0]]=regs[operands[1]]+int(operands[2][1:],0)
            elif i.mnemonic=='ldr':
                match=re.match(r'(\w+), \[(\w+)(?:, #(0x[0-9a-f]+|[0-9]+))?\]',i.op_str)
                if match:
                    dst,src,delta=match.groups(); addr=regs.get(src)
                    if addr is not None:
                        eff=addr+(int(delta,0) if delta else 0); value=ptr(eff)
                        if value is not None:
                            if eff in notes:note=' ; '+notes[eff]
                            elif value in notes:note=' ; '+notes[value]+' [handle '+hex(value)+']'
                            regs[dst]=value
                        else:regs.pop(dst,None)
                    else:regs.pop(dst,None)
            elif i.mnemonic in ('bl','b') and i.op_str.startswith('#'):
                target=int(i.op_str[1:],0); names=aliases.get(target)
                if names:note=' ; '+' | '.join(names[:3])+(' [shared generic address; use preceding Method handle]' if len(names)>3 else '')
                if i.mnemonic=='bl':
                    for rn in [f'x{n}' for n in range(19)]:regs.pop(rn,None)
            elif operands and re.match('^[wx][0-9]+$',operands[0]):regs.pop(operands[0],None)
            lines.append(f'{i.address:08x} {i.mnemonic:8} {i.op_str}{note}')
        path=BASE/'analysis'/'bundles'/(name.replace('$$','.').replace('<','_').replace('>','_')+'.annotated.txt')
        path.write_text('\n'.join(lines),encoding='utf-8')
        print(path.name,end-v)
