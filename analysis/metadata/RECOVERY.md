# Kinja Run 1.9.1: recovered IL2CPP metadata and native stripe layout

All steps transform bytes from the supplied APK for static analysis. The APK and
its libraries were not launched, and the original APK was not modified.

## Target identities

- APK SHA-256: `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`
- Original metadata SHA-256: `d765c0946244c0525dfa0626659caf0e5bd7e6eb45c8247ef44180e8186fb0bf`
- Original libil2cpp.so SHA-256: `18fa330ed22bb35db0bb9966a5ed8086ec8fc7125bd865ac73e92921fc4639d7`
- Original libanort.so SHA-256: `dc5f4c6c87bf8e139fc220c2fce3064a05d20c4952b22ed81aab86117117a66d`

## Metadata recovery

Artifact: `global-metadata.recovered.dat`, 13,953,100 bytes.
SHA-256: `9f4d19008b4004f09352fa576206e5464d34de8c822236528393d7a27c380f97`.
Reproducible script: `tools/metadata/recover_metadata.py`.

The Android protection library `libanort.so` supplies the decoding algorithm.
Its mmap hook at virtual address `0x11fe10` checks the custom first four bytes
`94 43 72 12` at `0x11feb8..0x11fee4`. It then calls decoder `0x123174` with
key `0xd96603c0` at `0x11ff98..0x11ffa8`.

The decoder performs these transformations:

1. XOR `0x66` on file bytes `[8,0x1000)`.
2. AES-128-CBC decrypt file range `[0x1000,0x5000)` in independent 2,048-byte
   chunks. The key is the 16-byte ASCII string `d96603c0d96603c0`, generated with
   `%08x%08x` at `0x1231e8..0x1231fc`; the format string is at `0x14b220`.
   The IV resets for every chunk to hex `02030405060708090a0b0c0d0e0f1011`, read
   from `0x14b1e0` by helper `0x1111d8`. The chunk loop is at
   `0x1113f8..0x11143c`, and CBC decryption is at `0x11211c`.
3. Beginning at offset `0x11000`, XOR `0x66` on the first 16 KiB of every
   complete 64 KiB block. XOR the entire final partial block. In this file the
   final partial block is `[0xd41000,0xd4e84c)`.

The first eight custom bytes are not ordinary metadata magic/version bytes.
The runtime hook clears bytes `[0,4)` and `[5,8)` at `0x11ffbc..0x11ffc4`,
preserving byte four, `0x18`. For analysis tools the recovered artifact replaces
these eight bytes with standard sanity `0xfab11baf` and inferred version 27.
Il2CppDumper and Cpp2IL identify the resulting structure as 27.1. This tooling
header substitution must not be confused with an original recompilable project
or with a drop-in replacement for the protected runtime metadata.

Validation covers every row of the following tables:

- 31 table ranges are contiguous, fit in the file, and end exactly at EOF.
- All 119,518 methods have bounded name indexes, bounded declaring type indexes,
  and metadata token prefix `0x06`.
- All 16,980 types have bounded name/namespace indexes and token prefix `0x02`.
- All 119 image name indexes fit the string table.
- All 28,076 string literal records fit the string literal data table.
- All 122,721 parameters have bounded name indexes and token prefix `0x08`.
- All 979 exported type indexes fit the type table.

The earlier `global-metadata.partial.dat` and `global-metadata.analysis.dat`
artifacts are superseded. They lacked the AES recovery and the exact final
partial-block rule. Their names do not denote final recovery.

## Native stripe recovery

Artifact: `libil2cpp.stripes.so`.
SHA-256: `a2a4510a6650a6bfc9c23949e466392a098f9a1c992e21194b531c1e45babdbe`.
Script: `tools/metadata/recover_native_stripes.py`.

The first decoder called by protected `DT_INIT=0x4cc104c` is
`g_acf_array[1]`, libanort routine `0x6de54`. Its descriptors at target virtual
address `0x4cd3014` explicitly identify the `.text`, `il2cpp`, and `.rodata`
sections. The section byte transformations are reached through
`0x11239c -> 0x111454 -> 0x123358`.

Each section's cipher start is the next 4 KiB boundary after its start, plus
another 4 KiB. The first 16 KiB at this point are AES encrypted and remain opaque
in the native analysis copy because the native section key is not yet recovered.
Beginning 64 KiB after cipher start, the first 16 KiB of each complete 64 KiB
block are XORed with `0x66`; the entire final partial block is XORed.

In addition, every group of eight complete XOR stripes is permuted. The global
configuration pointer at libanort GOT `0x16cf98` relocates to configuration data
at `0x11edf4`. Its uint32 at `+0x60` is `0x04531762`. Routine `0x123358`
extracts its eight nibbles using shifts `(28,24,20,16,12,8,4,0)` from table
`0x14dc80`. This yields source-to-destination stripe indexes
`(0,4,5,3,1,7,6,2)`. The recovered copy applies this exact permutation.

The initial AES ranges still opaque are:

- `.text`: `[0xb01000,0xb05000)`
- `il2cpp`: `[0xe92000,0xe96000)`
- `.rodata`: `[0x3702000,0x3706000)`

The generic method table at target address `0x38d5464` now begins with coherent
records `(0,1,8729,0)`, `(1,2,7268,1)`, `(2,3,8729,2)`. Every one of its 78,866
generic spec indexes is below the 84,097-entry method spec bound. This validates
the stripe permutation against a table that the earlier XOR-only hypothesis
could not recover.

The root investigation ran Il2CppDumper successfully to completion with this
native copy and `global-metadata.recovered.dat`. A successful metadata/native
symbol dump provides names, signatures, native addresses, and field layouts.
Its generated C# declarations do not recover the original C# method bodies.

The native AES key is obtained separately through
`0x6bee0 -> 0x6c54c(config,1)` inside `0x6de54`; it is not the metadata file's
hardcoded key. This call eventually combines protected key state and integrity
state in `0x6c9cc`/`0x6cc40`. The metadata AES key does not decrypt the first
native windows. The native key recovery remains open; the remaining opaque
coverage is three 16 KiB ranges (48 KiB total).

The helper `tools/metadata/map_native_patch.py` prints a reversible byte patch
plan from recovered offsets to original protected offsets. It does not modify
the input and refuses opaque AES ranges. The inverse mapping was compared with
the recovered file at 294 points across section boundaries, shuffled groups,
stripe edges, plaintext ranges, and final partial blocks; all comparisons agree.

## Game metadata index

`game-types.json` includes fields, methods, tokens, and metadata indexes for
4,520 types in Assembly-CSharp and selected game service assemblies.
`startup-types.json` and `startup-types.md` focus on startup, network, account,
player, and battle names. These relationships are observed metadata; names
alone do not establish method control flow.

Assembly-CSharp is image 118, spanning types 12678 through 16979 (4,302 types).
Useful entry points include Init (type 13001), AssetChecker`1 (12703),
PlayerDataManager (13363), and ET.LoginHelper (16014).
