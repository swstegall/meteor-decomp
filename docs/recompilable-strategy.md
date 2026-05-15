# Recompilable client — strategy

This is the plan for taking meteor-decomp from "lots of GREEN-matched
functions" to "we can actually re-link a working PE." Captured during
the 2026-05-15 push that landed the byte-passthrough fallback (Phase 2.6).

## Goal

`make link BINARY=ffxivlogin.exe` produces `build/link/ffxivlogin.exe`
that is byte-identical to `orig/ffxivlogin.exe` (modulo a few
PE-header fields the linker controls — timestamp, checksum). The same
flow extends to the four other binaries with the same recipe.

## Why this matters

Until the link works:
- We cannot run any of our own code through the original .exe path.
- We cannot validate that subsystem A's matched functions still
  cooperate correctly with subsystem B's (cross-function calls only
  exercised in vivo).
- We cannot cherry-pick which functions to substitute with our
  source-level decomp vs. leave as orig bytes — both lanes need to
  coexist in one PE.

Once the link works:
- Any matched function lands at its orig RVA via the `_rosetta/<sym>.cpp`.
- Every still-unmatched function lands at its orig RVA via the
  `_passthrough/FUN_<va>.cpp` byte-replay.
- We can flip individual functions from passthrough → rosetta as the
  decomp progresses, and the binary still boots.
- We can A/B test our re-derivation against a known-good baseline by
  comparing process behaviour.

## Architecture

The linker runs once per binary and pulls together five categories of
input:

1. **`.text` from .obj files** — one .obj per function, named
   `FUN_<va>.cpp` → `FUN_<va>.obj`. Each .obj has a single
   `.text$X<rva-hex>` subsection. `link.exe /MERGE:.text$*=.text`
   sorts subsections alphabetically and concatenates them into the
   merged `.text`. Because we keyed on RVA, the result lands at the
   right offsets.
2. **`.text` gap padding** — Ghidra-detected functions don't cover
   100% of `.text`. The 70k inter-function gaps are mostly 0xCC
   padding (~46k of them) plus some Ghidra-missed code/data
   (~24k). A `tools/emit_text_gaps.py` walks the YAML, finds gaps,
   and emits a single `_passthrough/_gaps.cpp` whose `.text$<rva>`
   subsections fill them.
3. **`.rdata` / `.data` / `.tls` / `.rsrc`** — non-code sections
   are byte-copied from orig into one `__declspec(allocate(...))`
   blob per section. `tools/emit_data_sections.py` reads the PE
   layout JSON, slices each section's raw bytes, and emits a
   `.cpp` that places the bytes into the correct section name.
4. **PE imports (.idata)** — orig's imports live in `.rdata`. To
   keep the IAT byte-identical, we either:
   - (a) let the byte-copy of `.rdata` carry the IAT bytes, AND
     emit a `.def` file telling the linker not to generate its own
     `.idata` directory entry but to point the PE header at the
     copied bytes (advanced `/DIRECTIVES:NO`+`/MERGE:.idata=.rdata`).
   - (b) write a `_imports.cpp` that declares each
     `__declspec(dllimport) extern "C"` symbol the binary uses,
     mapping to its `dll!ordinal_or_name` per
     `tools/extract_pe_imports.py`. This is the cleaner route but
     requires enumerating every import.
   - **Initial path: (a).** The byte-copy already includes the
     IAT; let the linker treat it as opaque. Set `/IGNORE:4108`
     (no IAT generated) and write the import directory's RVA + size
     into the PE header via `/MERGE` + a custom post-link patch.
5. **PE base relocations (.reloc)** — for binaries that have one.
   ffxivlogin doesn't. ffxivgame does. Same strategy: byte-copy
   from orig.

## Linker invocation (target)

```sh
"$WINE" link.exe \
    /NOLOGO \
    /BASE:0x00400000 \
    /SUBSYSTEM:WINDOWS \
    /MACHINE:X86 \
    /ENTRY:_entry \                   # name of the orig entry-RVA function
    /MERGE:.text$X*=.text \           # collapse our per-RVA subsections
    /MERGE:.rdata$X*=.rdata \
    /MERGE:.data$X*=.data \
    /SECTION:.text,ER \
    /SECTION:.rdata,R \
    /SECTION:.data,RW \
    /OPT:NOREF /OPT:NOICF \           # keep every fn live, no fold
    /INCREMENTAL:NO \
    /FIXED \                          # no relocations relative to BASE
    /NODEFAULTLIB \                   # no CRT — passthroughs cover it
    /OUT:build/link/ffxivlogin.exe \
    @build/link/ffxivlogin.objlist     # @-file with all .obj paths
```

We override `/ENTRY` because cl.exe-emitted naked functions don't
auto-discover their entry. `_entry` is whichever passthrough .cpp
covers the orig entry RVA; we add an alias to it via `#pragma comment(linker, "/ALIAS:_entry=FUN_<va>")` in that .cpp.

`/FIXED` is essential — orig binaries are not relocatable (no
.reloc directory in PE header for some), and embedding the .reloc
bytes raw would conflict with the linker's default behaviour of
generating its own.

## Section ordering

`link.exe` sorts grouped sections alphabetically and concatenates them
inside the merged section. Names are case-sensitive ASCII:

```
.text$X00001020   <  .text$X00001030   <  .text$X000a0000
```

This works because we pad the RVA hex to 8 chars; lexicographic
ordering matches numeric ordering. The `X` prefix exists to keep our
subsections clearly distinct from any MSVC-internal `.text$mn`,
`.text$x` reserved names.

## Open questions / known unknowns

- **Import directory in byte-copied .rdata.** If link.exe insists on
  rebuilding `.idata`, we may need a post-link binary patch that
  rewrites the `IMAGE_DIRECTORY_ENTRY_IMPORT` entry in the optional
  header to point back at the orig RVA. `tools/postlink_patch.py`.
- **`__security_cookie` and `_imp__*` IAT thunks.** Passthrough
  .objs don't reference these symbols (they emit raw bytes). So the
  linker shouldn't even ask for them. But one `_rosetta/<sym>.cpp`
  that *does* call `_imp__memcpy` will. Workaround: link with the
  CRT static lib and pass `/NODEFAULTLIB:libcmt.lib` to skip its
  startup; or emit our own `crt_stubs.cpp`.
- **Resource section (.rsrc).** Either embed orig bytes verbatim
  OR re-emit via `rc.exe` from a `.rc` source we generate from the
  orig resource directory. Embed-orig is the lower-risk MVP.
- **PE timestamp + checksum.** Linker writes a fresh timestamp;
  fix via `/TSAWARE:NO` + post-link patch (`postlink_patch.py`
  handles checksum recompute via `imagehlp.CheckSumMappedFile`).
- **Optional header / NT header invariants.** `SizeOfImage`,
  `SizeOfHeaders`, `BaseOfData`, `Subsystem`, `DllCharacteristics` —
  we want all of these to come out matching orig. Linker flags
  control most; the rest is post-link patching.

## Phased delivery

Each phase ends in a `make link BINARY=ffxivlogin.exe` invocation
that produces *something* and `tools/diff_pe.py` quantifies the gap.

### Stage A — `.text` only, no link (✅ landed 2026-05-15)
- `emit_passthrough_cpp.py` produces naked `_emit` `.cpp` per fn
- Output ".obj" `.text` is byte-identical to orig (validated on
  sizes 8 B / 83 B / 209 B / 1825 B / 5517 B, all GREEN)
- `make compile-passthrough BINARY=…` bulk-builds .obj inventory
- `tools/mark_passthrough_yaml.py` flips YAML rows to
  `passthrough` once .obj's are byte-verified

### Stage B — text-coverage analysis (✅ landed 2026-05-15)
- `tools/emit_text_gaps.py` — single .cpp filling the inter-fn gaps
- Every byte of `.text` is contributed by some .obj's
  `.text$X<rva>` section.
- Validated on ffxivlogin (557 gap subsections, 33,000 bytes)

### Stage C — non-code sections (✅ landed 2026-05-15)
- `tools/emit_data_sections.py` — `.rdata.cpp`, `.data.cpp`,
  `.rsrc.cpp` as one `__declspec(allocate(".<sec>$X<rva>"))` blob
  per section.
- Validated: `.rdata`, `.data`, `.rsrc` for ffxivlogin all
  byte-identical to orig.

### Stage D — first link (✅ landed 2026-05-15)
- `make link BINARY=ffxivlogin.exe` invokes link.exe with all the
  bits.
- Key gotcha #1: cl.exe's `code_seg` pragma creates COMDAT
  subsections with default 16-byte alignment. link.exe pads each
  obj-file's contribution UP to its alignment boundary when
  concatenating into the merged `.text`. With 557 gap subsections
  spread across many .objs, this introduced ~10-byte gaps after
  every function, breaking RVA fidelity. Fixed by `tools/patch_obj_alignment.py`
  (rewrites COFF section characteristics field bits 20-23 to set
  align=1 byte) — but this still left obj-file-boundary alignment
  gaps inside `.text` that link.exe wouldn't drop.
- Final design: emit ONE giant naked-asm function (`_text_blob`)
  containing every byte of orig `.text`, in `tools/emit_text_blob.py`.
  Single .obj → single section → no obj-boundary alignment artifacts.
  CNT_CODE/EXECUTE/READ characteristics match orig exactly.
  Trade-off: gives up function-level granularity (no cherry-picking
  rosetta sources for individual functions). Reclaim later by
  upgrading the rosetta cluster pipeline to emit `.text$X<rva>`
  pragmas + dedicated link recipe.
- Entry symbol: link.exe requires `/ENTRY:<symbol>`. We use
  `_text_blob` (lives at `.text+0`, the start of orig `.text`).
  The actual entry RVA lives elsewhere (e.g. ffxivlogin entry RVA
  0x26838 = inside the blob at byte 0x25838). `tools/postlink_patch.py`
  fixes `AddressOfEntryPoint` post-link.

### Stage E — post-link patcher (✅ landed 2026-05-15)
- `tools/postlink_patch.py` does five things:
  1. Splice orig DOS header + DOS stub (incl. Rich header) into ours;
     relocate our NT headers to orig's pe_off if needed.
  2. Copy COFF timestamp from orig.
  3. Copy ~20 optional-header fields (entry_rva, size_init_data,
     size_uninit_data, size_image, size_headers, version fields,
     stack/heap reserve/commit, dll_characteristics, loader_flags…).
  4. Copy section table fields per section (VirtualSize,
     VirtualAddress, SizeOfRawData, Characteristics).
  5. Copy all 16 data directory entries (Imports, Exports, IAT,
     Resources, etc.) — orig values, since we don't define our own.
  6. Splice orig's Authenticode certificate (data directory 4) at
     its file offset (this is a POST-section blob).
  7. Recompute PE checksum via the imagehlp algorithm.

### Stage F — bytewise PE diff = 0 (✅ ffxivlogin landed 2026-05-15)
- Exit criterion: `cmp orig/<bin>.exe build/link/<bin>.exe` is empty.
All five binaries land byte-identical as of 2026-05-15:

| Binary             | Size        | Status              |
|--------------------|-------------|---------------------|
| `ffxivlogin.exe`   |     403,296 | ✅ BYTE-IDENTICAL   |
| `ffxivupdater.exe` |     640,344 | ✅ BYTE-IDENTICAL   |
| `ffxivconfig.exe`  |   3,471,240 | ✅ BYTE-IDENTICAL   |
| `ffxivboot.exe`    |  12,961,112 | ✅ BYTE-IDENTICAL   |
| `ffxivgame.exe`    |  15,996,808 | ✅ BYTE-IDENTICAL   |

`make relink BINARY=<bin>.exe` rebuilds, relinks, patches, and produces
output that `cmp` confirms is byte-identical to orig.

### Stage G — per-function rosetta swap (✅ landed 2026-05-15)
- `tools/swap_rosetta.py <bin> FUN_<va>` (or `make swap-rosetta
  BINARY=<bin>.exe FUNC=FUN_<va>`) splices a hand-written
  `_rosetta/<sym>.cpp` into the relink in place of the byte-blob's
  coverage at the function's RVA.
- Pipeline:
  1. Wrap the rosetta source in `#pragma code_seg(".text$X<rva>")`
     so cl.exe places its `.text` contribution at the right
     subsection key — the merged `.text` then sorts the swap into
     the hole instead of appending it after the last chunk.
  2. Compile via cl-wine.sh.
  3. Patch the .obj's `.text$X*` align bits to 1 byte.
  4. Verify the .obj's .text matches orig bytes at the RVA — if not,
     ABORT (don't ship a regression).
  5. Append the swap to `_swap_manifest.json`.
  6. `emit_text_blob.py` reads the manifest and splits the blob at
     each swap boundary, leaving holes that the swap's
     `.text$X<rva>` subsection fills via lexicographic sort.
- **Critical bug fix that unblocked this**: text-blob chunks now
  use the chunk's RVA as the `.text$X<key>` subsection key (not the
  body offset). Otherwise a swap's `.text$X<rva>` keys against a
  different namespace than the chunks and lands in the wrong
  position. Both must use the same key space.
- Validated end-to-end on ffxivlogin: `FUN_00401350` (3-byte
  `__thiscall` empty stub) swapped from a hand-written
  `void C::empty1(int) {}` → `make relink` produces a binary that is
  STILL byte-identical to orig, with the link map confirming
  `?empty1@C@@QAEXH@Z` at offset 0x350 of the merged `.text`.
- Idempotent: re-running `swap-rosetta` on the same fn no-ops; the
  manifest deduplicates.
- `swap_rosetta.py --bulk` walks every `_rosetta/FUN_*.cpp` and
  attempts the swap; per-fn outcome is JSON-logged to
  `build/wire/<bin>.bulk_swap.json`. First ffxivlogin bulk run
  (2026-05-15): 281 candidates → 266 prefilter_extern (cluster
  placeholders with unresolvable `the_global` / `target` refs) +
  11 byte_mismatch (import thunks with IAT relocs we can't resolve)
  + **4 accepted** (1-3 byte trivial stubs). All 4 baked into a
  byte-identical ffxivlogin.exe.
- Why such a low yield on ffxivlogin: most `_rosetta/*.cpp` here
  are CLUSTER-DERIVED stubs whose source references fictional
  symbols (`int the_global`, `void target()`, `void operator_delete`)
  to make cl.exe emit the right shape — they're validation-only,
  not link-ready. The high-yield rosettas are in ffxivgame
  (38,593 _rosetta files including hand-written matches like
  `Utf8StringFree`, `PackRead` methods, CRT helpers).
- Each swap is wrapped in a per-RVA C++ namespace (`namespace
  swap_<rva> { ... }`) so cluster-derived sources reusing names
  like `class C` / `void target()` don't collide at link time
  (LNK2005).

## Why ffxivlogin first

It's the smallest (403 KB, 4 sections, 2,239 functions) and has the
fewest external surface (no MSVCR/D3D9/dinput/ws2/lua imports per
the PE fingerprint). If we get it byte-matching first, the same
recipe extends to the other binaries with mostly more inputs, not
new categories of input.

## Status as of 2026-05-15

- Stage A: ✅ landed (passthrough emitter + bulk compile + YAML flip)
- Stage B: ✅ landed (text-gap manifest)
- Stage C: ✅ landed (data-section emitter)
- Stage D: ✅ landed (link.exe driver)
- Stage E: ✅ landed (post-link patcher with cert + checksum + DOS stub)
- Stage F: ✅ landed for all 5 binaries (byte-identical)
- Stage G: ✅ landed (per-function rosetta swap on ffxivlogin)
- Stage H: ✅ landed (multi-function rosetta swap, source-file granularity)
- Stage I: ✅ landed (functional validation — `tools/validate_relink.sh`)
- Stage J: ✅ landed (cross-binary swap fan-out — bulk swap_rosetta on all 5)

### Stage H — multi-function rosetta swap (✅ landed 2026-05-15)
- `tools/swap_source_file.py` handles hand-written multi-fn source
  files in `crt/` / `sqex/` / `sqpack/` / `install/`. Each function
  in the source compiles to a `/Gy` COMDAT `.text` section; we
  auto-discover its RVA via reloc-aware byte-pattern search against
  orig `.text`, then patch the .obj to rename the section to
  `.text$X<rva>` + freeze the bytes (replace with orig + zero relocs).
- Strict mode rejects files where any function is skipped OR the
  .obj has surviving undefined external symbols (e.g. `_memcpy`,
  `_imp_*` IAT thunks). Avoids LNK1120 link errors without needing
  a fake import library.
- ffxivgame yield: 7 fn(s) across 5 obj files (CRT helpers — Atol,
  Strncmp, Strlen, Strcmp, Alloca, InitTerm, Exit). Multi-fn yield
  is small because most hand-written sources have at least one
  not-yet-byte-perfect function.
- Future: rebuild .obj keeping only accepted sections instead of
  whole-file rejection. Would unlock per-function granularity inside
  partial multi-fn sources.

### Stage I — functional validation (✅ landed 2026-05-15)
- `tools/validate_relink.sh`: SHA-256s `build/link/<bin>.exe` vs
  install `<bin>.exe` for all 5 binaries. Substitutes the install
  with our re-link to prove the launcher / patcher / Wine pipeline
  runs against our output exactly the same as orig.
- Byte-identity ⇒ functional identity: the OS loader sees identical
  bytes; `ffxivgame.patched.exe` produced by garlemald-client's
  runtime PE patcher applies to our re-link the same way (same patch
  byte ranges, same outcome). No observable difference at any layer.
- Validation result: 5/5 binaries match install at SHA-256 level
  (33,472,800 bytes).

### Stage J — cross-binary swap fan-out (✅ landed 2026-05-15)
- `python3 tools/swap_rosetta.py <bin> --bulk --jobs N` ran against
  every binary using the comment-hint matcher + extern neutraliser
  improvements from Stage H.
- Per-binary swap totals after fan-out (each relink remains
  byte-identical to orig):

  | Binary             | Candidates | Accepted | byte_mismatch | prefilter_extern |
  |--------------------|-----------:|---------:|--------------:|-----------------:|
  | `ffxivgame.exe`    |     38,593 |    2,762 |             — |                — |
  | `ffxivboot.exe`    |     26,103 |    2,556 |         1,911 |           21,636 |
  | `ffxivupdater.exe` |        433 |       78 |             6 |              349 |
  | `ffxivlogin.exe`   |        281 |        4 |            11 |              266 |
  | `ffxivconfig.exe`  |        185 |        2 |             6 |              177 |

  (ffxivgame totals are from the Stage G-era bulk run, not re-run in
  this fan-out.) The bulk JSON for each lives at
  `build/wire/<bin>.bulk_swap.json`.
- Stage J confirms: the swap pipeline scales without per-binary tuning.
  ffxivupdater's 78 accepted (vs the 4 baseline of ffxivlogin) is the
  representative win — it hosts more straight-forward pre-deduped
  rosetta sources and the comment-hint matcher catches them.
- Why the byte_mismatch count varies so widely: most rosetta candidates
  are cluster-derived placeholders that compile to *similar*-looking
  bytes (right shape, wrong constants/relocs). Stage J accepts only
  exact matches against orig; rejected ones still hold their place via
  the text-blob passthrough.

The recompilable-client effort hit its ffxivlogin milestone in one
session. The other four binaries are next; for each, the recipe is:

```sh
make relink BINARY=<bin>.exe
make diff-pe BINARY=<bin>.exe   # should report "100.00%"
cmp orig/<bin>.exe build/link/<bin>.exe && echo BYTE-IDENTICAL
```

Expected per-binary friction:
- `ffxivconfig` / `ffxivupdater`: similar shape to ffxivlogin
  (small, few sections, no special imports). Should land first try.
- `ffxivboot`: ~9.5 MB `.text`, more sections. May expose new PE
  header fields the patcher doesn't yet copy.
- `ffxivgame`: 12 MB binary with `.tls`, `MSSMIXER`, `.reloc`. Largest
  and most likely to surface new edge cases. The single-naked-fn
  approach for `_text_blob` will compile (`/bigobj` already in flight
  for the gap manifest); link.exe needs to handle 5 sections + reloc
  + tls + a 12 MB blob.

Bumping the matched-byte percentage in YAML now follows from running
the compile + mark_passthrough_yaml pipeline against each binary; the
per-binary report is in `make progress`.
