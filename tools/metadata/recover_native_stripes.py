"""Reconstruct libanort.so's native XOR and stripe permutation for analysis.

The first16KiB AES window of each section remains opaque until its key is
recovered. No package or target code is executed; only file bytes are transformed.
"""
from pathlib import Path
import argparse,json,struct,hashlib

SECTIONS=[('.text',0xaff250,0x3919d8),('il2cpp',0xe90c28,0x286f818),('.rodata',0x3700440,0x40cb7d)]
PERMUTATION=[0,4,5,3,1,7,6,2]

def decode_section(data,offset,size):
    cipher_start=((offset+0x1000)&~0xfff)+0x1000
    aes_range=(cipher_start,cipher_start+0x4000)
    stripes_start=cipher_start+0x10000
    remaining=offset+size-stripes_start
    full=remaining//0x10000
    for n in range(full):
        pos=stripes_start+n*0x10000
        for i in range(pos,pos+0x4000):data[i]^=0x66
        if n%8==7:
            base=stripes_start+(n-7)*0x10000
            stripes=[bytes(data[base+k*0x10000:base+k*0x10000+0x4000]) for k in range(8)]
            for source,dest in enumerate(PERMUTATION):
                pos=base+dest*0x10000
                data[pos:pos+0x4000]=stripes[source]
    tail=stripes_start+full*0x10000
    for i in range(tail,offset+size):data[i]^=0x66
    return {'section_offset':hex(offset),'section_size':hex(size),'opaque_AES_range':[hex(x) for x in aes_range],
            'stripes_start':hex(stripes_start),'full_64KiB_blocks':full,'final_partial_block':[hex(tail),hex(offset+size)]}

def main():
    p=argparse.ArgumentParser();p.add_argument('binary',type=Path);p.add_argument('output',type=Path);args=p.parse_args()
    raw=args.binary.read_bytes();data=bytearray(raw)
    sections={name:decode_section(data,offset,size) for name,offset,size in SECTIONS}
    args.output.write_bytes(data)
    report={'original_sha256':hashlib.sha256(raw).hexdigest(),'output_sha256':hashlib.sha256(data).hexdigest(),
            'permutation_source_to_destination':PERMUTATION,'sections':sections,
            'native_sources':{'config_offset':'libanort.so0x11edf4+0x60','config_value':'0x04531762',
                              'nibble_shifts':'libanort.so0x14dc80','decoder':'0x11239c ->0x111454 ->0x123358'},
            'limitations':['InitialAESwindows remain originalciphertext.','ELF relocation/layout protections remain unmodified.']}
    args.output.with_suffix('.json').write_text(json.dumps(report,indent=2),encoding='utf-8')
    print(json.dumps(report,indent=2))

if __name__=='__main__':main()
