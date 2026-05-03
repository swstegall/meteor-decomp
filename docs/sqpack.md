# Phase 4 — Pack / ChunkRead / ZiPatch architecture

> Last updated: 2026-05-02 — active matching, multiple GREEN.

This document captures what's been recovered about the FFXIV 1.x file
system from `ffxivgame.exe` and `ffxivupdater.exe`, and steers the
Phase 4 work pool. It supersedes the speculative "Sqpack::Hash" entry
in `PLAN.md`: the 1.x file system is **not** the string-hashed Sqpack
format that ARR / DQX shipped — it uses 32-bit resource IDs.

## Key finding: 1.x is resource-id-addressed, not path-hashed

In ARR-era Sqpack, file paths are hashed (folder hash + file hash, both
CRC32) and the hash pair indexes the `.index` file. The 1.x format
predates that: there is **no string-path hash**. Instead every asset
has a 32-bit `resource_id` and the file lives at:

```
<game-root>/data/<b3>/<b2>/<b1>/<b0>.DAT
```

where `b3..b0` are the four bytes of `resource_id` written as 2-digit
uppercase hex. Confirmed by:

- The path-format string `"%cdata%c%02X%c%02X%c%02X%c%02X.DAT"` at
  RVA `0x00b672bc` (abs `0x00f672bc`).
- The SeventhUmbral port's `CFileManager::GetResourcePath` (port
  cited at `SeventhUmbral/dataobjects/FileManager.cpp`):

  ```cpp
  auto resourceIdName = string_format("%0.2X/%0.2X/%0.2X/%0.2X.DAT",
      (resourceId >> 24) & 0xFF, (resourceId >> 16) & 0xFF,
      (resourceId >>  8) & 0xFF, (resourceId >>  0) & 0xFF);
  return dataPath / resourceIdName;
  ```

So the actual matching target list looks like:

| Original PLAN entry            | Reality in 1.x                                |
|--------------------------------|-----------------------------------------------|
| `Sqpack::Hash` (string→u32)    | **Does not exist.** Resource IDs are literal. |
| `Sqpack::PathBuilder` / lookup | `FUN_0044b3a0` builds DAT path from u32.     |
| Pack file readers              | `Sqex::Data::PackRead` / `PackWrite`         |
| Chunk I/O                      | `Sqex::Data::ChunkRead<u32,u32>` / `ChunkWrite` |
| Decompression                  | TBD — likely zlib wrapped in a chunk          |
| ZiPatch unpacker               | `ffxivupdater.exe` only                       |

## Class hierarchy (recovered from RTTI)

```
Sqex::Data::ChunkRead<unsigned int, unsigned int>      (vtable RVA 0xb931c8, 1 slot)
└── Sqex::Data::PackRead                                (vtable RVA 0xd0dd40, 1 slot)

Sqex::Data::ChunkWrite<unsigned int, unsigned int>     (vtable, 1 slot)
└── Sqex::Data::PackWrite                               (vtable RVA 0xd1311c, 1 slot)

(parallel byte-sized chunk variants)
Sqex::Data::ChunkRead<unsigned char, unsigned short>   (1 slot)
Sqex::Data::ChunkWrite<unsigned char, unsigned short>  (1 slot)
```

Vtables have only 1 slot each — the destructor. Every other interface
method on these classes is **non-virtual**, so the work pool isn't
discoverable through vtable analysis alone. We have to walk xrefs to
the vtable VAs (constructor / destructor sites set the vtable) and
fan out from there.

The ChunkRead<u8,u16> + ChunkWrite<u8,u16> instantiations are
parallel — for a different chunk family (likely texture streams or
audio with smaller chunk-id and chunk-size widths).

## Anchor functions found so far

| RVA           | Size  | Role                                                       |
|---------------|------:|------------------------------------------------------------|
| `0x008c6670`  | 107 B | `PackRead::~PackRead` — sets vtable, frees heap @ this+0x74, calls into ChunkRead destructor at this+0x1c, hands over to `ChunkRead<u32,u32>::~ChunkRead` (vtable swap to 0xb931c8) |
| `0x00942230`  |  84 B | Allocates / constructs a `PackWrite` instance (writes vtable 0x111311c) |
| `0x00942800`  | 132 B | Constructs / re-initialises a `PackRead` instance (writes vtable 0x110dd40) |
| `0x0004b3a0`  | 615 B | Builds the `data\<b3>\<b2>\<b1>\<b0>.DAT` path from a 32-bit `resource_id` (PUSH 0x5c separators × 5, calls into a sprintf-like helper at FUN_00447620) |

The `PackRead::~PackRead` destructor is the smallest concrete unit and
the natural first matching target.

## PackRead struct layout (inferred from destructor)

```c++
class ChunkRead_uint_uint {
    void *vtable;          // +0x00 — 0xb931c8 (ChunkRead<u32,u32>)
    char  base_state[0x1c]; // +0x04..0x1f — base ChunkRead members (TBD)
    // (the destructor calls a method on `this+0x1c`, hinting that the
    //  base class portion ends around offset 0x1c)
};

class PackRead : public ChunkRead_uint_uint {
    // +0x00 vtable (0x110dd40 = PackRead vtable)
    // +0x04..+0x1c base ChunkRead state
    // +0x1c..+0x73 PackRead-specific fields (unknown, but a method call
    //              at LEA ECX,[ESI+0x1c]; CALL ... destructs whatever
    //              object lives at offset 0x1c — likely a sub-object
    //              like a file handle or buffer descriptor)
    void *heap_buffer;     // +0x74 — heap-allocated, freed in dtor if non-null
    void *unknown_1;       // +0x78 — cleared with EDI=0 in dtor
    void *unknown_2;       // +0x7c — cleared with EDI=0 in dtor
    // total size ≥ 0x80
};
```

## PackRead's complete external API surface

Mapped by an xref-scan + per-caller analysis pass on 2026-05-02. The
class has a surprisingly small consumer footprint for an FFXIV 1.x
file-system reader:

| Consumer | Calls into | Role |
|---|---|---|
| `FUN_00cc66e0` (30 B) | PackRead::~PackRead | Vtable slot 0 — the canonical MSVC scalar deleting destructor (D2). Auto-matched GREEN by the deriver's `try_scalar_deleting_dtor_30b` pattern. |
| `FUN_00cc6700` (490 B) | PackRead::PackRead, ReadNext, ~PackRead | The only direct consumer in `ffxivgame.exe`. Stack-allocates a PackRead at `[ESP+0x1c]`, constructs it from a buffer slice, drives `ReadNext` in a chunk-iteration loop, destructs at end of scope. Not yet matched (490 B with multi-chunk SEH frame). |

Earlier xref-scan hits at `0x00d31xxx..0x00d33xxx` turned out to be
`Sqex::Input::RepeatCounter` users (a different class whose code
lives interleaved in the same `.text` range due to MSVC COMDAT
ordering). The reliable filter for "PackRead consumer" is
**xref-to-PackRead-vtable** (only 2 sites — the ctor + dtor) plus
**xref-to-PackRead's-known-methods** (Rewind / ReadNext / ProcessChunk
/ destructor).

The conclusion: PackRead's API surface is now fully accounted for in
this binary — anyone needing to read 1.x pack data goes through
either the D2 vtable slot or the FUN_00cc6700 wrapper.

## Phase 4 work pool — current state

Status as of 2026-05-02. See
[`decomp-status.md § Phase 4`](decomp-status.md) for the consolidated
status snapshot and [`install-unpacker.md`](install-unpacker.md) for
the consumer-side write-up.

### Sqex::Data layer

| Function | RVA | Size | Status | File |
|---|---|---:|---|---|
| `PackRead::~PackRead` | `0x008c6670` | 110 B | ✅ GREEN | [`src/ffxivgame/sqpack/PackRead.cpp`](../src/ffxivgame/sqpack/PackRead.cpp) |
| `PackRead::PackRead` (ctor) | `0x00942800` | 132 B | 🟡 130/132 PARTIAL | same |
| `PackRead::ReadNext` | (tiny) | 27 B | ✅ GREEN | same |
| `PackRead::Rewind` | (tiny) | 18 B | ✅ GREEN | same |
| `PackRead::ProcessChunk` | (mid) | 177 B | 🟡 180/177 PARTIAL | [`src/ffxivgame/_partial/`](../src/ffxivgame/_partial/) — buffer-guard cookie blocker |
| `ChunkReadUInt::ReadNextChunkHeader` | (mid) | 81 B | 🟡 74/81 PARTIAL | [`src/ffxivgame/sqpack/ChunkRead.cpp`](../src/ffxivgame/sqpack/ChunkRead.cpp) |

PackRead's vtable D2 dtor (slot 0, RVA `0x00cbea90`, 30 B) is matched
GREEN by the deriver's `try_scalar_deleting_dtor_30b` template (auto-
stamped via Phase 2.5 cluster pipeline).

### Remaining work

- 🟡 **Push the PackRead ctor (130/132) to GREEN** — two-byte off in
  cookie/SEH frame setup; same shape as the destructor. Likely needs
  Ghidra GUI to confirm exact `__security_cookie` ordering against the
  2-byte trailing INT3 padding the linker inserted between functions.
- 🟡 **Push `ProcessChunk` (180/177) to GREEN** — the
  `__security_check_cookie` epilogue is sensitive to local layout;
  needs a one-byte buffer reordering or a `/GS`-trigger local rearrange.
- 🟡 **Push `ReadNextChunkHeader` (74/81) to GREEN** — 91 % match;
  inner-loop register allocation differs by one MOV.
- ✅ **Functional re-derive of the path builder @ `0x0004b3a0`** —
  done 2026-05-02. Source at
  [`src/ffxivgame/sqpack/PathBuilder.cpp`](../src/ffxivgame/sqpack/PathBuilder.cpp);
  Python ref + scan tool at [`tools/sqpack_path.py`](../tools/sqpack_path.py).
  Format string recovered: `\data\<b3:02X>\<b2:02X>\<b1:02X>\<b0:02X>.DAT`
  (verbatim from `.rdata 0x00b672bc`). Cross-validated against
  `SeventhUmbral/dataobjects/FileManager.cpp::CFileManager::GetResourcePath`
  (which uses `/` separators but otherwise identical). Self-test in
  the .cpp covers 8 canonical resource_ids; Python `--scan` mode
  successfully roundtrips path → resource_id against 140,180 real
  DAT files in a retail install.
  The "modded resource lookup" path (taken when `[0x01266b64] == 0`)
  is unimplemented — garlemald doesn't have replaceable resources so
  the standard path suffices.
- 🔲 **Walk PackRead's full non-virtual interface** — confirmed
  consumer surface is the FUN_00cc6700 (InstallUnpacker::Unpack) +
  FUN_00cc66e0 (D2 trampoline) pair. ChunkRead<u32,u32> base methods
  may have additional callers — xref-walk vtable RVA `0xb931c8`.
- 🔲 **Decompression layer** — locate by xref to standard zlib magic
  (`78 9c` / `78 da`) or zlib symbol names if any survive in `.rdata`.
- 🔲 **ZiPatch unpacker** — lives in `ffxivupdater.exe`. Block types
  from the wiki: FHDR, APLY, APFS, ETRY, ADIR, DELD. Tackle separately
  under `src/ffxivupdater/sqpack/` after the game-side reader works.

## Exit criterion (revised)

A working `tools/sqpack-cat <resource_id>` that:
1. ✅ **Resolves a resource_id to a DAT file path** — done via
   [`tools/sqpack_path.py`](../tools/sqpack_path.py) +
   [`src/ffxivgame/sqpack/PathBuilder.cpp`](../src/ffxivgame/sqpack/PathBuilder.cpp).
   Verified against 140,180 real DAT files in a retail install.
2. ✅ **Opens the DAT file** — done via
   [`tools/sqpack_cat.py`](../tools/sqpack_cat.py).
3. 🟡 **Streams the contained chunks** — best-effort chunk walker
   in `sqpack_cat.py` correctly walks PackRead-format files; flags
   false positives (offset-table files like `03/A2/0D/00.DAT` that
   *look* chunked but aren't) with `OVERFLOW` status. Most DAT
   files use file-type-specific magics (GTEX texture, SEDB sound DB,
   MapL map layout, PWIB unknown, `#fil` CSV text) and aren't
   PackRead-chunked at all — those are recognised and skipped.
4. ✅ **Decompression layer** — done 2026-05-02. The binary
   statically links zlib 1.2.3 (`"inflate 1.2.3 Copyright 1995-2005
   Mark Adler"` at `.rdata 0xd16e71`). Found the inflate chain by
   xref-walking the `"incorrect header check"` error string at
   `.rdata 0xd14208`:
   - `FUN_00d4f640` (5,451 B) — zlib's `inflate()` itself
   - `FUN_00d4f510` (25 B) — `inflateInit_` thunk
   - `FUN_00d42590` (427 B) — `PackRead::ProcessNextChunk`, the
     bridge that wraps a chunk payload in a `z_stream` and drives
     inflate

   Chain: `PackRead::ProcessChunk → FUN_00d42590 →
   FUN_00d4f510 → FUN_00d4f640`.

   `tools/sqpack_cat.py` now exposes `--inflate` which inflates each
   chunk's payload (when the zlib heuristic hits — first byte's low
   nibble is 0x8 for deflate, header word `% 31 == 0`). End-to-end
   verified against a synthetic chunked DAT with a known
   zlib-compressed payload — round-trips perfectly.

   For matching-decomp purposes, `inflate()` is already byte-identical
   in the binary (it IS the upstream zlib 1.2.3 binary linked verbatim;
   no SE customization). No need to re-derive it. For tools, Python's
   `zlib.decompress()` and Rust's `flate2` crate are byte-compatible.

Plus byte-matching `PackRead::~PackRead` (✅ GREEN), `Rewind`
(✅ GREEN), `ReadNext` (✅ GREEN), and ideally the constructor
(🟡 130/132) + `ProcessChunk` (🟡 180/177).

**Note on PackRead's actual scope**: the only direct caller of
PackRead in `ffxivgame.exe` is `Component::Install::InstallUnpacker
::Unpack` (FUN_00cc6700). So PackRead is for **installer / patcher
manifests**, NOT runtime game data. That explains why most DAT files
in a retail install aren't PackRead-chunked — they're per-file-type
binary blobs read by other code paths.
