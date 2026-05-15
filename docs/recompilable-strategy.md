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

### Stage A — `.text` only, no link (✅ infrastructure ready)
- Done as of 2026-05-15:
  - `emit_passthrough_cpp.py` produces naked `_emit` `.cpp` per fn
  - Output ".obj" `.text` is byte-identical to orig (validated on
    sizes 8 B / 83 B / 209 B / 1825 B / 5517 B, all GREEN)
  - `make compile-passthrough BINARY=…` bulk-builds .obj inventory
  - `tools/mark_passthrough_yaml.py` flips YAML rows to
    `passthrough` once .obj's are byte-verified

### Stage B — text-coverage analysis (🔲 next)
- `tools/emit_text_gaps.py` — single .cpp filling the inter-fn gaps.
- Goal: every byte of `.text` is contributed by some .obj's
  `.text$X<rva>` section.
- Validation: walk every byte of orig's `.text`, confirm there's
  exactly one .obj that owns it.

### Stage C — non-code sections (🔲)
- `tools/emit_data_sections.py` — `.rdata.cpp`, `.data.cpp`,
  `.rsrc.cpp` (or `.res`).
- Validation: for each section, the bytes between `raw_pointer`
  and `raw_pointer+raw_size` are emitted into a `.cpp` whose
  `__declspec(allocate(...))` blob matches.

### Stage D — first link (🔲)
- `make link BINARY=ffxivlogin.exe` invokes link.exe with all the
  bits. Expected first failure: missing entry, missing imports, or
  PE-header mismatch.
- Iterate until link succeeds, then run `tools/diff_pe.py
  build/link/ffxivlogin.exe orig/ffxivlogin.exe` and walk the
  diffs.

### Stage E — post-link patcher (🔲)
- `tools/postlink_patch.py` — fix-up timestamp, checksum, any
  PE-header fields the linker controls but we want to match.

### Stage F — bytewise PE diff = 0 (🔲)
- The "recompilable" exit criterion: `cmp` of our re-link vs
  orig is empty.

## Why ffxivlogin first

It's the smallest (403 KB, 4 sections, 2,239 functions) and has the
fewest external surface (no MSVCR/D3D9/dinput/ws2/lua imports per
the PE fingerprint). If we get it byte-matching first, the same
recipe extends to the other binaries with mostly more inputs, not
new categories of input.

## Status as of 2026-05-15

- Stage A: ✅ landed (passthrough emitter + bulk compile + YAML flip)
- Stage B–F: 🔲 not yet started

Bumping the matched-byte percentage in YAML now follows from running
the compile + mark_passthrough_yaml pipeline against each binary; the
per-binary report is in `make progress`.
