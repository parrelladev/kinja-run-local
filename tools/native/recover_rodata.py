"""Derive an analysis-only ELF copy by undoing observed XOR66 rodata stripes.

Do not package this copy into the game: the original loader decodes these bytes
at runtime. Original code and APK remain untouched.
"""
import hashlib,json,pathlib,sys
sys.path.insert(0,str(pathlib.Path(__file__).parent/'python-libs'))
from elftools.elf.elffile import ELFFile
source=pathlib.Path(sys.argv[1]);dest=pathlib.Path(sys.argv[2])
data=bytearray(source.read_bytes())
with source.open('rb') as f:
    elf=ELFFile(f);s=elf.get_section_by_name('.rodata');lo=s['sh_offset'];hi=lo+s['sh_size']
ranges=[];run=None
for i in range(lo,hi):
    selected=0x2000 <= i % 0x10000 < 0x6000
    if selected:
        data[i]^=0x66
        if run is None:run=i
    elif run is not None:ranges.append([run,i]);run=None
if run is not None:ranges.append([run,hi])
dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(data)
report={'source':str(source),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'derived':str(dest),'derived_sha256':hashlib.sha256(data).hexdigest(),'xor':0x66,'scope':'rodata file offsets modulo65536 in[8192,24576)','ranges':ranges,'bytes_changed':sum(b-a for a,b in ranges),'status':'hypothesis for analysis; never a playable patched binary'}
dest.with_suffix('.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
print(json.dumps({k:v for k,v in report.items() if k!='ranges'}))
