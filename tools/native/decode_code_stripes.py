"""Analysis-only XOR66 of executable/rodata stripes; raw APK is not changed."""
import hashlib,json,pathlib,sys
sys.path.insert(0,str(pathlib.Path(__file__).parent/'python-libs'))
from elftools.elf.elffile import ELFFile
src=pathlib.Path(sys.argv[1]);dst=pathlib.Path(sys.argv[2]);data=bytearray(src.read_bytes());scope=[]
with src.open('rb') as f:
    elf=ELFFile(f)
    for name in ['.text','il2cpp','.rodata']:
        sec=elf.get_section_by_name(name);lo=sec['sh_offset'];hi=lo+sec['sh_size'];changed=0
        for i in range(lo,hi):
            if 0x2000<=i%0x10000<0x6000:data[i]^=0x66;changed+=1
        scope.append(dict(section=name,start=hex(lo),end=hex(hi),bytes_changed=changed))
dst.parent.mkdir(exist_ok=True,parents=True);dst.write_bytes(data)
print(json.dumps(dict(derived=str(dst),sha256=hashlib.sha256(data).hexdigest(),scope=scope,status='analysis only; stripe scope inferred from decoded instructions, not yet entire protection loader')))
