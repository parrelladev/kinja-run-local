"""Repack the selected original APK with explicit member replacements.

Preserves unmodified members' uncompressed bytes. Does not install, sign, change
the Android manifest, or claim that any replacement implements offline mode.
"""
from __future__ import annotations

import argparse
import copy
import hashlib
import json
import re
import struct
import zipfile
from pathlib import Path, PurePosixPath

ORIGINAL_SHA256 = '7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e'


def digest(path):
    with path.open('rb') as f:
        return hashlib.file_digest(f, 'sha256').hexdigest()


def old_signature(name):
    upper = name.upper()
    return (upper == 'STAMP-CERT-SHA256' or upper == 'META-INF/MANIFEST.MF'
            or bool(re.fullmatch(r'META-INF/[^/]+\.(SF|RSA|DSA|EC)', upper))
            or bool(re.fullmatch(r'META-INF/SIG-[^/]+', upper)))


def member_sources(values):
    result = {}
    for value in values:
        member, sep, file = value.partition('=')
        relative = PurePosixPath(member)
        if (not sep or not member or not file or member in result or relative.is_absolute()
                or '..' in relative.parts or '\\' in member):
            raise ValueError(f'Invalid or duplicate member source: {value}')
        path = Path(file).absolute()
        if not path.is_file() or old_signature(member):
            raise ValueError(f'Invalid member source: {member}')
        result[member] = path
    return result


def alignment_info(info, file_pointer, alignment):
    target = copy.copy(info)
    if alignment and info.compress_type == zipfile.ZIP_STORED:
        try: encoded_name = info.filename.encode('ascii')
        except UnicodeEncodeError: encoded_name = info.filename.encode('utf8')
        data_start = file_pointer + 30 + len(encoded_name) + len(target.extra)
        padding = -data_start % alignment
        if padding:
            target.extra += struct.pack('<HH', 0xd935, padding) + b'\0' * padding
    return target


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--input', required=True, type=Path)
    parser.add_argument('--output', required=True, type=Path)
    parser.add_argument('--replace', action='append', default=[], metavar='MEMBER=FILE')
    parser.add_argument('--add', action='append', default=[], metavar='MEMBER=FILE', help='Append a new DEX/library/asset member')
    parser.add_argument('--expected-sha256', default=ORIGINAL_SHA256)
    parser.add_argument('--align', type=int, choices=[4], help='Align uncompressed ZIP data with a valid padding extra field')
    args = parser.parse_args()
    source, output = args.input.resolve(), args.output.resolve()
    if source == output or output.exists():
        raise ValueError('Output must be a new file, distinct from the original APK')
    source_hash = digest(source)
    if source_hash.lower() != args.expected_sha256.lower():
        raise ValueError(f'Original APK SHA-256 mismatch: {source_hash}')
    replacements, additions = member_sources(args.replace), member_sources(args.add)
    if not replacements and not additions:
        raise ValueError('At least one explicit replacement or addition is required')
    if set(replacements) & set(additions): raise ValueError('A member cannot be replaced and added simultaneously')
    record = dict(input=str(source), input_sha256=source_hash, output=str(output), alignment=args.align,
                  removed_signatures=[], members=[])
    with zipfile.ZipFile(source) as original:
        infos = original.infolist()
        names = [x.filename for x in infos]
        if len(names) != len(set(names)):
            raise ValueError('Duplicate ZIP members require manual inspection')
        missing = set(replacements) - set(names)
        if missing:
            raise ValueError(f'Replacement names absent from original APK: {sorted(missing)}')
        conflicts = set(additions) & set(names)
        if conflicts: raise ValueError(f'New member names already present in original APK: {sorted(conflicts)}')
        output.parent.mkdir(parents=True, exist_ok=True)
        with zipfile.ZipFile(output, 'x') as rebuilt:
            rebuilt.comment = original.comment
            for info in infos:
                name = info.filename
                if old_signature(name):
                    record['removed_signatures'].append(name)
                    continue
                original_bytes = original.read(info)
                replacement = replacements.get(name)
                content = original_bytes if replacement is None else replacement.read_bytes()
                target_info = alignment_info(info,rebuilt.fp.tell(),args.align)
                rebuilt.writestr(target_info, content)
                record['members'].append(dict(name=name, replaced=replacement is not None,added=False,
                                              replacement_path=None if replacement is None else str(replacement),
                                              original_sha256=hashlib.sha256(original_bytes).hexdigest(),
                                              output_sha256=hashlib.sha256(content).hexdigest(),
                                              original_size=len(original_bytes), output_size=len(content)))
            timestamp = original.getinfo('AndroidManifest.xml').date_time
            for name, path in sorted(additions.items()):
                content = path.read_bytes()
                info = zipfile.ZipInfo(name,date_time=timestamp)
                info.compress_type = zipfile.ZIP_DEFLATED
                info.create_system = 3
                info.external_attr = 0o100644 << 16
                rebuilt.writestr(alignment_info(info,rebuilt.fp.tell(),args.align),content)
                record['members'].append(dict(name=name,replaced=False,added=True,addition_path=str(path),
                                              original_sha256=None,output_sha256=hashlib.sha256(content).hexdigest(),
                                              original_size=None,output_size=len(content)))
    # Confirm every resulting ZIP member against the expected bytes/hash, using
    # the produced APK rather than an assumption about successful compression.
    with zipfile.ZipFile(output) as rebuilt:
        if len(rebuilt.infolist()) != len(record['members']):
            raise ValueError('Repacked member count mismatch')
        for member in record['members']:
            actual = hashlib.sha256(rebuilt.read(member['name'])).hexdigest()
            if actual != member['output_sha256']:
                raise ValueError(f'Content mismatch in repacked APK: {member["name"]}')
        if args.align:
            with output.open('rb') as raw:
                for info in rebuilt.infolist():
                    if info.compress_type != zipfile.ZIP_STORED: continue
                    raw.seek(info.header_offset)
                    header = raw.read(30)
                    name_size, extra_size = struct.unpack_from('<HH', header, 26)
                    data_start = info.header_offset + 30 + name_size + extra_size
                    if data_start % args.align:
                        raise ValueError(f'Unaligned uncompressed member: {info.filename}')
    record['output_sha256'] = digest(output)
    report = Path(str(output) + '.repack.json')
    report.write_text(json.dumps(record, indent=2), encoding='utf8')
    print(json.dumps(dict(output=str(output), output_sha256=record['output_sha256'],
                          report=str(report), preserved_members=sum(not m['replaced'] and not m['added'] for m in record['members']),
                          replaced_members=[m['name'] for m in record['members'] if m['replaced']],
                          added_members=[m['name'] for m in record['members'] if m['added']],
                          removed_signatures=record['removed_signatures']), indent=2))


if __name__ == '__main__':
    main()
