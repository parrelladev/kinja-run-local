"""Recover the observed XOR regions of the supplied Kinja Run IL2CPP metadata.

This produces analysis artifacts only. It does not patch or execute the APK.
Native libanort.so routine 0x123174 supplies the XOR/AES transformations.
The metadata version 27 is inferred from header layout and Unity 2020.3;
the original custom sanity/version words are retained in the report.
"""
from __future__ import annotations
import argparse
import hashlib
import json
import struct
import zipfile
import subprocess
from pathlib import Path

MEMBER = "assets/bin/Data/Managed/Metadata/global-metadata.dat"
TABLES = [
    "stringLiteral", "stringLiteralData", "string", "events", "properties",
    "methods", "parameterDefaultValues", "fieldDefaultValues",
    "fieldAndParameterDefaultValueData", "fieldMarshaledSizes", "parameters",
    "fields", "genericParameters", "genericParameterConstraints", "genericContainers",
    "nestedTypes", "interfaces", "vtableMethods", "interfaceOffsets", "typeDefinitions",
    "images", "assemblies", "fieldRefs", "referencedAssemblies", "attributesInfo",
    "attributeTypes", "unresolvedVirtualCallParameterTypes", "unresolvedVirtualCallParameterRanges",
    "windowsRuntimeTypeNames", "windowsRuntimeStrings", "exportedTypeDefinitions",
]

def sha(data: bytes | bytearray) -> str:
    return hashlib.sha256(data).hexdigest()

def recover(raw: bytes) -> bytearray:
    data = bytearray(raw)
    # 0x1231cc: first page, excluding the custom eight-byte header.
    ranges = [(8, 0x1000)]
    # 0x12321c..0x123294: XOR first 16 KiB of each complete 64 KiB
    # block after offset 0x11000; XOR the ENTIRE final partial block.
    full_blocks = (len(raw) - 0x11000) // 0x10000
    ranges += [(0x11000 + n * 0x10000, 0x15000 + n * 0x10000) for n in range(full_blocks)]
    ranges += [(0x11000 + full_blocks * 0x10000, len(raw))]
    for start, end in ranges:
        for pos in range(start, end):
            data[pos] ^= 0x66
    # 0x1231e8 key "%08x%08x", 0x123218 decrypt function 0x11132c,
    # 0x1113f8 independent 0x800-byte AES-CBC blocks. Node's built-in
    # crypto performs only the reconstructed transformation on input bytes.
    js = """
const crypto=require('crypto');
const chunks=[];process.stdin.on('data',b=>chunks.push(b));
process.stdin.on('end',()=>{const input=Buffer.concat(chunks);const output=[];
for(let p=0;p<input.length;p+=0x800){
const d=crypto.createDecipheriv('aes-128-cbc',Buffer.from('d96603c0d96603c0'),Buffer.from(Array.from({length:16},(_,i)=>i+2)));
d.setAutoPadding(false);output.push(d.update(input.subarray(p,p+0x800)),d.final());}
process.stdout.write(Buffer.concat(output));});
"""
    cipher = subprocess.run(["node", "-e", js], input=raw[0x1000:0x5000], capture_output=True, check=True)
    assert len(cipher.stdout) == 0x4000
    data[0x1000:0x5000] = cipher.stdout
    struct.pack_into("<II", data, 0, 0xFAB11BAF, 27)
    return data

def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("apk", type=Path)
    parser.add_argument("out", type=Path)
    args = parser.parse_args()
    with zipfile.ZipFile(args.apk) as z:
        raw = z.read(MEMBER)
    data = recover(raw)
    args.out.mkdir(parents=True, exist_ok=True)
    recovered = args.out / "global-metadata.recovered.dat"
    recovered.write_bytes(data)
    table_info = {}
    for i, name in enumerate(TABLES):
        offset, size = struct.unpack_from("<II", data, 8 + i * 8)
        table_info[name] = {"offset": offset, "size": size, "end": offset + size,
                            "in_file": offset + size <= len(data)}
    string_table = table_info["string"]
    names = data[string_table["offset"]:string_table["end"]]
    names_file = args.out / "names.utf8.txt"
    names_file.write_text("\n".join(s.decode("utf-8", "backslashreplace") for s in names.split(b"\0")), encoding="utf-8")
    # Validate independently of the unrecovered literals: record token prefixes,
    # string-name bounds, declaring-type bounds, and table contiguity.
    methods = table_info["methods"]
    method_rows = list(struct.iter_unpack("<IIIIIIHHHH", data[methods["offset"]:methods["end"]]))
    types = table_info["typeDefinitions"]
    type_rows = list(struct.iter_unpack("<16I8H2I", data[types["offset"]:types["end"]]))
    images = table_info["images"]
    image_rows = list(struct.iter_unpack("<10I", data[images["offset"]:images["end"]]))
    literals=table_info['stringLiteral']
    literal_rows=list(struct.iter_unpack('<II',data[literals['offset']:literals['end']]))
    parameters=table_info['parameters']
    parameter_rows=list(struct.iter_unpack('<III',data[parameters['offset']:parameters['end']]))
    exports=table_info['exportedTypeDefinitions']
    export_rows=list(struct.iter_unpack('<I',data[exports['offset']:exports['end']]))
    report = {
        "apk": str(args.apk.resolve()), "member": MEMBER,
        "apk_sha256": sha(args.apk.read_bytes()),
        "original_metadata_sha256": sha(raw), "length": len(raw),
        "custom_header_words": list(struct.unpack_from("<II", raw, 0)),
        "recovered_header_words": [0xFAB11BAF, 27],
        "version_status": "27 inferred from the 256-byte header and Unity 2020.3; not yet traced in native loader",
        "transformation": "XOR0x66 at [8,0x1000), first16KiB of each complete64KiB block after0x11000, entire finalpartial block; AES128CBC independently for each0x800-byte block in[0x1000,0x5000), key ASCII d96603c0d96603c0, IV hex02030405060708090a0b0c0d0e0f1011",
        "source_native_evidence": {"library":"lib/arm64-v8a/libanort.so",
                                   "mmap_hook": "0x11fe10", "metadata_magic_check":"0x11feb8..0x11fee4",
                                   "key":"0x11ff98..0x11ffa8", "decoder":"0x123174",
                                   "key_format":"0x14b220", "AES_chunks":"0x1113f8..0x11143c",
                                   "IV":"0x14b1e0", "AES_CBC":"0x11211c"},
        "unrecovered_file_ranges": [],
        "recovered_byte_count_excluding_header_substitution": len(raw) - 8,
        "artifacts": {"recovered": {"path": str(recovered.resolve()), "sha256": sha(data),
                                   "substitution": "first8bytes replaced with standard sanity and inferred version27"}},
        "tables": table_info,
        "validation": {
            "all_table_ranges_in_file": all(t["in_file"] for t in table_info.values()),
            "contiguous_tables": all(a["end"] == b["offset"] for a, b in zip(list(table_info.values()), list(table_info.values())[1:])),
            "last_table_ends_at_file_end": list(table_info.values())[-1]["end"] == len(data),
            "method_count": len(method_rows), "type_count": len(type_rows), "image_count": len(image_rows),
            "method_name_in_bounds": sum(r[0] < string_table["size"] for r in method_rows),
            "method_declaring_type_in_bounds": sum(r[1] < len(type_rows) for r in method_rows),
            "method_token_prefix_0x06": sum(r[5] >> 24 == 6 for r in method_rows),
            "type_name_in_bounds": sum(r[0] < string_table["size"] for r in type_rows),
            "type_namespace_in_bounds": sum(r[1] < string_table["size"] for r in type_rows),
            "type_token_prefix_0x02": sum(r[-1] >> 24 == 2 for r in type_rows),
            "image_name_in_bounds": sum(r[0] < string_table["size"] for r in image_rows),
            "string_literal_count":len(literal_rows),
            "string_literal_ranges_valid":sum(r[0]+r[1]<=table_info['stringLiteralData']['size'] for r in literal_rows),
            "parameter_count":len(parameter_rows),
            "parameter_name_in_bounds":sum(r[0]<string_table['size'] for r in parameter_rows),
            "parameter_token_prefix_0x08":sum(r[1]>>24==8 for r in parameter_rows),
            "exported_type_count":len(export_rows),
            "exported_type_indices_in_bounds":sum(r[0]<len(type_rows) for r in export_rows),
        },
        "limitations": ["Static analysis only; no APK or native code executed.",
                        "The first8bytes are custom format markers, substituted with standard sanity and inferred version27 for tooling.",
                        "This artifact contains metadata and names, not recovered method bodies or a recompilable Unity project."]
    }
    (args.out / "recovery-report.json").write_text(json.dumps(report, indent=2), encoding="utf-8")
    print(json.dumps({"artifacts": report["artifacts"], "validation": report["validation"]}, indent=2))

if __name__ == "__main__":
    main()
