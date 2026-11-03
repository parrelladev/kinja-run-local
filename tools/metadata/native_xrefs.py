from pathlib import Path
import sys,struct
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / "native/python-libs"))
from elftools.elf.elffile import ELFFile
from capstone import Cs,CS_ARCH_ARM64,CS_MODE_ARM
p=Path(sys.argv[1]);target=int(sys.argv[2],0)
with p.open('rb') as f:
    e=ELFFile(f)
    md=Cs(CS_ARCH_ARM64,CS_MODE_ARM)
    for s in e.iter_sections():
        if not s['sh_flags']&4:continue
        b=s.data();base=s['sh_addr']
        for i in range(0,len(b)-4,4):
            ins=struct.unpack_from('<I',b,i)[0]
            if ins&0x9f000000 in (0x90000000,0x10000000):
                imm=((ins>>29)&3)|(((ins>>5)&0x7ffff)<<2)
                if imm &0x100000:imm-=0x200000
                adrp = ins&0x80000000 != 0
                address=((base+i)&~0xfff)+(imm<<12) if adrp else base+i+imm
                if address==(target&~0xfff if adrp else target):
                    print(f'ADR page xref {s.name} 0x{base+i:x} register {ins&31}')
                    for inst in md.disasm(b[max(0,i-16):i+64],base+max(0,i-16)):
                        print(f'  {inst.address:x}: {inst.mnemonic} {inst.op_str}')
