"""Rename the fixed UTF-16 package field in resources.arsc, preserving IDs.

Does not rebuild resources, edit their values, or change any chunk size/offset.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path

from repack_apk import ORIGINAL_SHA256


def sha(b): return hashlib.sha256(b).hexdigest()


def packages(b):
    typ, header, size = struct.unpack_from('<HHI', b)
    if typ != 2 or header < 12 or size != len(b): raise ValueError('Invalid RES_TABLE_HEADER')
    count = struct.unpack_from('<I', b, 8)[0]
    found, offset = [], header
    while offset < len(b):
        typ, chunk_header, chunk_size = struct.unpack_from('<HHI', b, offset)
        if chunk_size < chunk_header or chunk_header < 8 or offset + chunk_size > len(b):
            raise ValueError(f'Invalid resource chunk at {offset:#x}')
        if typ == 0x200:
            if chunk_header < 284: raise ValueError('Short RES_TABLE_PACKAGE header')
            raw = b[offset + 12:offset + 268]
            name = raw.decode('utf-16-le').split('\0', 1)[0]
            found.append(dict(offset=offset, header_size=chunk_header, chunk_size=chunk_size,
                              id=struct.unpack_from('<I', b, offset + 8)[0], name=name,
                              name_offset=offset + 12, name_field_bytes=256))
        offset += chunk_size
    if offset != len(b) or len(found) != count: raise ValueError('Resource package count/layout mismatch')
    return found


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--input', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--old-package', default='com.habby.kinjarun')
    p.add_argument('--new-package', default='com.habby.kinjarun.offline.lab')
    args=p.parse_args()
    with args.input.open('rb') as f: apk_sha=hashlib.file_digest(f,'sha256').hexdigest()
    if apk_sha != ORIGINAL_SHA256: raise ValueError('Original APK SHA-256 guard failed')
    with zipfile.ZipFile(args.input) as z: original=z.read('resources.arsc')
    original_packages=packages(original)
    encoded=args.new_package.encode('utf-16-le')
    if len(encoded) > 254 or '\0' in args.new_package: raise ValueError('Resource package name exceeds the fixed field')
    matches=[x for x in original_packages if x['name']==args.old_package]
    if len(matches)!=1: raise ValueError('Expected exactly one matching resource package')
    package=matches[0]; start=package['name_offset']; end=start+256
    patched=bytearray(original)
    patched[start:end]=encoded+b'\0'*(256-len(encoded))
    parsed=packages(patched)
    for old,new in zip(original_packages,parsed,strict=True):
        if {k:v for k,v in old.items() if k!='name'} != {k:v for k,v in new.items() if k!='name'}:
            raise ValueError('Resource IDs/layout changed')
    if patched[:start]!=original[:start] or patched[end:]!=original[end:]:
        raise ValueError('Resource bytes outside the package name changed')
    changed=[i for i,(old,new) in enumerate(zip(original,patched,strict=True)) if old!=new]
    if args.output.exists(): raise ValueError('Resource output already exists')
    args.output.parent.mkdir(parents=True,exist_ok=True)
    args.output.write_bytes(patched)
    report=dict(original_apk_sha256=apk_sha,original_resources_sha256=sha(original),patched_resources_sha256=sha(patched),
                size_before=len(original),size_after=len(patched),package_id=package['id'],
                package_chunk_offset=hex(package['offset']),name_field_offset=hex(start),name_field_bytes=256,
                old_package=args.old_package,new_package=args.new_package,modified_byte_offsets=changed,
                existing_resource_ids_preserved=True,chunk_sizes_and_offsets_preserved=True,
                resource_values_and_strings_preserved=True)
    Path(str(args.output)+'.patch.json').write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps(report,indent=2))


if __name__=='__main__': main()
