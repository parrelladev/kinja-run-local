"""Map an edit in recovered native bytes to the supplied protected ELF bytes.

Prints a reviewable byte patch plan only; never edits the input binary.
AES windows are deliberately unsupported until the native section key is known.
"""
from pathlib import Path
import argparse,json,struct,zlib
from recover_native_stripes import SECTIONS,PERMUTATION

def map_plain_offset(offset):
    for _,section,size in SECTIONS:
        if not section<=offset<section+size:continue
        cipher=((section+0x1000)&~0xfff)+0x1000
        if cipher<=offset<cipher+0x4000:
            raise ValueError(f'Cannot patch opaque AES window at0x{offset:x}')
        start=cipher+0x10000
        full=(section+size-start)//0x10000
        if offset<start:return offset,0
        tail=start+full*0x10000
        if offset>=tail:return offset,0x66
        stripe,within=divmod(offset-start,0x10000)
        if within>=0x4000:return offset,0
        group=(stripe//8)*8
        if group+7<full:
            source=PERMUTATION.index(stripe%8)
            return start+(group+source)*0x10000+within,0x66
        return offset,0x66
    return offset,0

def plan(raw,offset,replacement):
    edits=[]
    for i,byte in enumerate(replacement):
        original,xor=map_plain_offset(offset+i)
        encoded=byte^xor
        edits.append({'recovered_offset':hex(offset+i),'original_offset':hex(original),
                      'original_byte':f'{raw[original]:02x}','recovered_before':f'{raw[original]^xor:02x}',
                      'recovered_after':f'{byte:02x}','replacement_encoded':f'{encoded:02x}'})
    return edits

def integrity_deltas(raw,edits):
    """Update declared plaintext section CRCs from byte changes only.

    CRC32 is affine over fixed-length messages, so the XOR delta's contribution
    is independent of still-opaque AES bytes. Returns a plan; writes nothing.
    The original descriptors begin at file0x4a83014 with44-byte record stride.
    """
    result=[]
    for n,(name,section,size) in enumerate(SECTIONS):
        changes=[e for e in edits if section<=int(e['recovered_offset'],0)<section+size]
        if not changes:continue
        delta=bytearray(size)
        for edit in changes:
            pos=int(edit['recovered_offset'],0)-section
            delta[pos]^=int(edit['recovered_before'],16)^int(edit['recovered_after'],16)
        contribution=zlib.crc32(delta)^zlib.crc32(bytes(size))
        slot=0x4a83014+n*44+0x1c
        old=struct.unpack_from('<I',raw,slot)[0];new=old^contribution
        result.append({'section':name,'original_descriptor_crc_file_offset':hex(slot),
                       'declared_crc_before':hex(old),'declared_crc_after':hex(new),
                       'original_crc_bytes':struct.pack('<I',old).hex(),
                       'replacement_crc_bytes':struct.pack('<I',new).hex(),
                       'changed_plaintext_bytes':len(changes)})
    return result

def main():
    p=argparse.ArgumentParser();p.add_argument('binary',type=Path);p.add_argument('offset',type=lambda s:int(s,0));p.add_argument('hex_bytes');args=p.parse_args()
    raw=args.binary.read_bytes();edits=plan(raw,args.offset,bytes.fromhex(args.hex_bytes))
    print(json.dumps({'byte_edits':edits,'section_integrity_edits':integrity_deltas(raw,edits)},indent=2))

if __name__=='__main__':main()
