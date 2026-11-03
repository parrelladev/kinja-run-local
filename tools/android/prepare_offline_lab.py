"""Prepare a diagnostic variant of the original APK, keeping native code intact.

Appends strings to the AXML string pool without changing any existing string
index or resource map. Writes only replacement files and a public patch report.
The resulting build is diagnostic: disabling an update check does not establish
that server-dependent login/progression already works offline.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import struct
import zipfile
from pathlib import Path
import xml.etree.ElementTree as ET

from inspect_android_static import axml, u16, u32
from repack_apk import ORIGINAL_SHA256


def sha(b): return hashlib.sha256(b).hexdigest()


def utf8_length(n):
    if n >= 0x8000: raise ValueError('String exceeds AXML UTF-8 length range')
    return bytes((0x80 | (n >> 8), n & 255)) if n >= 128 else bytes((n,))


def encoded_string(value, utf8):
    units = len(value.encode('utf-16-le')) // 2
    if utf8:
        encoded = value.encode('utf8')
        return utf8_length(units) + utf8_length(len(encoded)) + encoded + b'\0'
    length = struct.pack('<H', units) if units < 0x8000 else struct.pack('<HH', 0x8000 | (units >> 16), units & 0xffff)
    return length + value.encode('utf-16-le') + b'\0\0'


def patch_manifest(original, package, label, launcher_activity=None):
    root, events = axml(original)
    old_package = root.get('package')
    if old_package != 'com.habby.kinjarun': raise ValueError('Unexpected original package')
    pool_pos = u16(original, 2)
    kind, header, size = struct.unpack_from('<HHI', original, pool_pos)
    if kind != 1: raise ValueError('AXML first chunk must be string pool')
    count, styles, flags, strings_start, styles_start = struct.unpack_from('<IIIII', original, pool_pos + 8)
    if styles != 0 or styles_start != 0: raise ValueError('Styled string pool requires an additional preservation path')
    if strings_start != header + count * 4: raise ValueError('Unexpected string pool offset table layout')
    additions, assigned, changes = [], {}, []
    body = bytearray(original[pool_pos + size:])
    declared = {}
    android_ns = 'http://schemas.android.com/apk/res/android'
    for event in events:
        if event['name'] == 'permission':
            for attr in event['attributes']:
                if attr['name'] == 'name' and attr['namespace'] == android_ns:
                    value = attr['value']
                    declared[value] = (package + value[len(old_package):]) if value.startswith(old_package + '.') else (value + '.offline.lab')
    for event in events:
        for attr in event['attributes']:
            tag, name, namespace, value = event['name'], attr['name'], attr['namespace'], attr['value']
            new = None
            if tag == 'manifest' and name == 'package' and namespace == '': new = package
            elif tag == 'application' and name == 'label' and namespace == android_ns: new = label
            elif (launcher_activity and tag == 'activity' and name == 'name' and namespace == android_ns
                  and value == 'com.unity3d.player.UnityPlayerActivity'):
                new = launcher_activity
            elif tag == 'provider' and name == 'authorities' and namespace == android_ns:
                # Every declared authority must be unique relative to the
                # installed original, including Facebook's app-ID authority.
                package_suffix = package.rsplit('.', 1)[-1]
                new = ';'.join(package + item[len(old_package):] if item.startswith(old_package + '.')
                               else item + '.offline.' + package_suffix for item in value.split(';'))
            elif name in ('name', 'permission', 'readPermission', 'writePermission') and value in declared:
                new = declared[value]
            elif tag == 'data' and name == 'host' and value == 'cct.' + old_package:
                new = 'cct.' + package
            if new is None or new == value: continue
            if new not in assigned:
                assigned[new] = count + len(additions)
                additions.append(new)
            index = assigned[new]
            old_offset = int(attr['file_offset'], 16)
            relative = old_offset - pool_pos - size
            # ResXMLTree_attribute: rawValue@8, typedValue.dataType@15,
            # typedValue.data@16. Keep ns/name/type.size/resource map unchanged.
            struct.pack_into('<I', body, relative + 8, index)
            body[relative + 15] = 3
            struct.pack_into('<I', body, relative + 16, index)
            changes.append(dict(element=tag,attribute=name,namespace=namespace,old=value,new=new,
                                original_attribute_offset=attr['file_offset'],new_string_index=index))
    pool = original[pool_pos:pool_pos + size]
    offsets = pool[header:header + count * 4]
    string_bytes = pool[strings_start:]
    new_offsets, added_bytes = bytearray(), bytearray()
    for value in additions:
        new_offsets.extend(struct.pack('<I', len(string_bytes) + len(added_bytes)))
        added_bytes.extend(encoded_string(value, bool(flags & 0x100)))
    new_header = bytearray(pool[:header])
    new_count = count + len(additions)
    new_start = header + new_count * 4
    new_pool = new_header + offsets + new_offsets + string_bytes + added_bytes
    new_pool.extend(b'\0' * (-len(new_pool) % 4))
    struct.pack_into('<I', new_pool, 4, len(new_pool))
    struct.pack_into('<I', new_pool, 8, new_count)
    # Appending breaks sorted ordering; original already has flags=0.
    struct.pack_into('<I', new_pool, 16, flags & ~1)
    struct.pack_into('<I', new_pool, 20, new_start)
    output = bytearray(original[:pool_pos]) + new_pool + body
    struct.pack_into('<I', output, 4, len(output))
    parsed, parsed_events = axml(output)
    if parsed.get('package') != package: raise ValueError('Reparsed package mismatch')
    if parsed.find('application').get('{'+android_ns+'}label') != label: raise ValueError('Reparsed label mismatch')
    if launcher_activity:
        activities=[node for node in parsed.find('application').findall('activity')
                    if node.get('{'+android_ns+'}name') == launcher_activity]
        if len(activities) != 1: raise ValueError('Expected exactly one wrapper launcher declaration')
    for old, new in zip(events, parsed_events, strict=True):
        if old['name'] != new['name'] or len(old['attributes']) != len(new['attributes']):
            raise ValueError('AXML element layout changed unexpectedly')
        for old_attr, new_attr in zip(old['attributes'], new['attributes'], strict=True):
            expected = next((c['new'] for c in changes if c['original_attribute_offset'] == old_attr['file_offset']), old_attr['value'])
            if new_attr['value'] != expected or old_attr['name'] != new_attr['name'] or old_attr['namespace'] != new_attr['namespace']:
                raise ValueError('Unexpected AXML attribute change')
    # Validate resource IDs chunk bytes and all original string pool references.
    old_res_end = u32(original, pool_pos + size + 4)
    if original[pool_pos + size:pool_pos + size + old_res_end] != output[pool_pos + len(new_pool):pool_pos + len(new_pool) + old_res_end]:
        raise ValueError('AXML resource map was modified')
    ET.indent(parsed)
    return bytes(output), ET.tostring(parsed, encoding='unicode'), dict(changes=changes,original_string_count=count,
        new_string_count=new_count,preserved_existing_indices=True,preserved_resource_map=True)


def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--input', type=Path, required=True)
    p.add_argument('--output-dir', type=Path, required=True)
    p.add_argument('--package', default='com.habby.kinjarun.offline.lab')
    p.add_argument('--label', default='Kinja Run Offline Lab')
    p.add_argument('--launcher-activity', help='Replace the original Unity Activity declaration with its wrapper subclass')
    args=p.parse_args()
    with args.input.open('rb') as f: original_sha=hashlib.file_digest(f,'sha256').hexdigest()
    if original_sha != ORIGINAL_SHA256: raise ValueError('Original APK SHA-256 guard failed')
    with zipfile.ZipFile(args.input) as z:
        original_manifest=z.read('AndroidManifest.xml')
        original_level=z.read('assets/bin/Data/level0')
    manifest, xml, manifest_report=patch_manifest(original_manifest,args.package,args.label,args.launcher_activity)
    if len(original_level) != 5036 or original_level[4980] != 1:
        raise ValueError('Original level0 size/IsCheckUpdate byte mismatch')
    level=bytearray(original_level); level[4980]=0
    differences=[i for i,(old,new) in enumerate(zip(original_level,level,strict=True)) if old != new]
    if differences != [4980]: raise ValueError('Unexpected level0 differences')
    args.output_dir.mkdir(parents=True,exist_ok=True)
    for file,data in [('AndroidManifest.xml',manifest),('level0',bytes(level))]:
        target=args.output_dir/file
        if target.exists(): raise ValueError(f'Output already exists: {target}')
        target.write_bytes(data)
    (args.output_dir/'AndroidManifest.decoded.xml').write_text(xml,encoding='utf8')
    report=dict(original_apk_sha256=original_sha,package=args.package,label=args.label,launcher_activity=args.launcher_activity,
        manifest=dict(original_sha256=sha(original_manifest),patched_sha256=sha(manifest),original_size=len(original_manifest),
                      patched_size=len(manifest),**manifest_report),
        level0=dict(original_sha256=sha(original_level),patched_sha256=sha(level),original_size=len(original_level),
                    patched_size=len(level),modified_offsets=differences,field='IsCheckUpdate',before=1,after=0),
        native_code_modified=False,dex_modified=False,resources_arsc_modified=False,
        diagnostic_only=True)
    (args.output_dir/'diagnostic-patch.json').write_text(json.dumps(report,indent=2),encoding='utf8')
    print(json.dumps(report,indent=2))


if __name__=='__main__': main()
