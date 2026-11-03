"""Read-only ELF/ARM64 probe. Does not execute the APK or ELF."""
import argparse, hashlib, json, pathlib, struct, sys
sys.path.insert(0, str(pathlib.Path(__file__).parent / 'python-libs'))
from elftools.elf.elffile import ELFFile
from elftools.elf.relocation import RelocationSection
from capstone import Cs, CS_ARCH_ARM64, CS_MODE_ARM

def main():
    p=argparse.ArgumentParser()
    p.add_argument('binary'); p.add_argument('--disasm'); p.add_argument('--bytes',type=lambda x:int(x,0),default=512)
    p.add_argument('--read'); p.add_argument('--out')
    a=p.parse_args(); data=pathlib.Path(a.binary).read_bytes()
    with open(a.binary,'rb') as f:
        elf=ELFFile(f)
        sections=[dict(name=s.name,addr=int(s['sh_addr']),offset=int(s['sh_offset']),size=int(s['sh_size']),flags=int(s['sh_flags'])) for s in elf.iter_sections()]
        segments=[dict(type=s['p_type'],addr=int(s['p_vaddr']),offset=int(s['p_offset']),filesz=int(s['p_filesz']),memsz=int(s['p_memsz']),flags=int(s['p_flags'])) for s in elf.iter_segments()]
        symbols={}
        for sn in ['.dynsym','.symtab']:
            s=elf.get_section_by_name(sn)
            if s:
                for sym in s.iter_symbols():
                    if sym['st_value']: symbols[int(sym['st_value'])]=sym.name
        relocations={}
        for sec in elf.iter_sections():
            if isinstance(sec,RelocationSection):
                for r in sec.iter_relocations():
                    relocations[int(r['r_offset'])]=dict(type=int(r['r_info_type']),addend=int(r.entry.get('r_addend',0)),symbol=int(r['r_info_sym']))
        def offset(v):
            for s in segments:
                if s['type']=='PT_LOAD' and s['addr']<=v<s['addr']+s['filesz']:return v-s['addr']+s['offset']
            raise ValueError(hex(v)+' is not file-backed')
        def ptr(v):
            r=relocations.get(v)
            return r['addend'] if r and r['type']==1027 else struct.unpack_from('<Q',data,offset(v))[0]
        def diss(addr,n):
            md=Cs(CS_ARCH_ARM64,CS_MODE_ARM); md.detail=True
            lines=[]
            for i in md.disasm(data[offset(addr):offset(addr)+n],addr):
                target=None
                if i.mnemonic in ['bl','b'] and i.op_str.startswith('#'):
                    target=int(i.op_str[1:],0)
                lines.append(dict(addr=hex(i.address),bytes=i.bytes.hex(),mnemonic=i.mnemonic,op=i.op_str,target_name=symbols.get(target)))
            return lines
        out=dict(binary=a.binary,sha256=hashlib.sha256(data).hexdigest(),machine=elf['e_machine'],sections=sections,segments=segments,
                 exports=[dict(addr=hex(v),name=n) for v,n in symbols.items() if 'metadata' in n.lower() or 'init' in n.lower() or 'register' in n.lower()],relocation_count=len(relocations))
        init=elf.get_section_by_name('.init_array')
        if init:out['init_array']=[dict(slot=hex(v),target=hex(ptr(v)),name=symbols.get(ptr(v))) for v in range(init['sh_addr'],init['sh_addr']+init['sh_size'],8)]
        if a.disasm:out=dict(sha256=out['sha256'],disasm=diss(int(a.disasm,0),a.bytes))
        if a.read:
            v=int(a.read,0);out=dict(sha256=out['sha256'],read=[dict(addr=hex(q),raw=data[offset(q):offset(q)+8].hex(),value=hex(ptr(q))) for q in range(v,v+a.bytes,8)])
        text=json.dumps(out,indent=2)
        if a.out:pathlib.Path(a.out).parent.mkdir(parents=True,exist_ok=True);pathlib.Path(a.out).write_text(text,encoding='utf-8')
        else:print(text)
if __name__=='__main__':main()
