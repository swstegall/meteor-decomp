# Library signature matching (Ghidra Function ID / FidDb)

Goal: name the thousands of MSVC CRT/STL functions ffxivgame.exe links
statically — currently sitting in the work pool as `FUN_xxxxxxxx` — so they
drop out of the matching pool instead of being hand-decompiled one by one.
This is the Ghidra equivalent of IDA FLIRT.

## Why it's worth it (and where the ceiling is)

`config/ffxivgame.middleware.json` shows the heuristic classifier flags only
**~149 of ~94,700** functions as library. A 33 MB statically-linked MSVC
binary has *thousands* of CRT/STL functions — they're all in the `matching`
pool right now, indistinguishable from game logic. A FidDb pass names them;
then `build_split_yaml.py`'s **existing** `std::` / CRT / MFC / zlib name
patterns reclassify them `matching` → `middleware-*` on the next `make split`
**automatically** — no new classifier code needed, just the names.

Realistic scope:
- **In scope:** MSVC-2005 (VC8) static CRT (`libcmt.lib`) + STL
  (`libcpmt.lib`) — the win is concentrated here (probably a few thousand fns).
- **Out of scope — the bulk:** Square Enix's own engine
  (`engine_cdev` ~5,089, `net` ~2,461, `render` ~938 functions = the
  CDev/Rapture engine). It's proprietary, not a library; FidDb/FLIRT can't
  touch it, and that's the part parity work actually cares about.
- **DX9 is moot here:** the binary imports `d3dx9_41.dll` *dynamically*
  (March-2009 DX SDK), so D3DX functions aren't in the exe — they're already
  named via the import table. No static D3DX to match.

So FidDb clears CRT/STL *noise out of the way* of the real work; it doesn't
do the real work.

## Toolchain (important correction vs. generic advice)

This binary is **MSVC 2005 (VC8)**, not VS2008/2010. meteor-decomp already
ships the exact toolchain (`vstudio2005-workspace`, byte-identical re-link),
so the FidDb is built from **the same VC8 static libs the binary links** —
giving a far higher CRT/STL hit rate than the generic "stock VS install,
20–40%" caveat. The x86 libs:

```
vstudio2005-workspace/.../VC/lib/libcmt.lib    (static CRT, /MT)   -> 792 .obj
vstudio2005-workspace/.../VC/lib/libcpmt.lib   (static STL)        ->  90 .obj
```

LanguageID for population/matching: `x86:LE:32:default`.

## The key gotcha (validated)

`analyzeHeadless -import foo.lib` **fails** with *"No load spec found"* —
Ghidra cannot load a COFF *archive* directly. The fix: extract the `.obj`
members and import *those* (Ghidra loads single `.obj` natively). macOS BSD
`ar` mangles MS COFF member names (`/`, `//` linker members); **`llvm-ar`**
(`brew install llvm`) extracts them correctly.

## Build + apply (automated)

`tools/build_fid.py` orchestrates everything, using the same `launch.sh`
`-Xmx8G` JDK-21 contract as `import_to_ghidra.py`:

```sh
# one-time: build the FidDb (extract .obj -> import w/ FID&LID off -> populate)
python3 tools/build_fid.py gen          # long: imports ~882 .obj into Ghidra

# apply it to ffxivgame + re-dump symbols
python3 tools/build_fid.py apply

# then the names propagate + reclassify:
make split BINARY=ffxivgame.exe         # std::/CRT patterns -> middleware-*
```

Sub-steps (`extract` / `import` / `populate` / `apply`) can be run
individually; `gen` chains the first three.

## GUI fallback for the populate step

FidDb *population* via the stock `CreateMultipleLibraries.java` is driven
headlessly through a generated `.properties` file. Headless `ask*` is
finicky; if `populate` balks, do this one step in the Ghidra GUI instead
(the import + apply remain scripted):

1. Open the `build/fid/proj` project (it holds the imported `.obj` under
   `/MSVC/8.0/x86`).
2. `Tools → Function ID → Create new empty FidDb…` → `build/fid/ffxiv_vc8.fidb`.
3. `Tools → Function ID → Populate FidDb from programs…` → root folder `/`,
   library `MSVC` / `8.0` / `x86`, LanguageID `x86:LE:32:default`.
4. Then `python3 tools/build_fid.py apply`.

FidDb generation is a notoriously fiddly one-time headless op but trivial in
the GUI — and it only has to be done once.

## How the hand-off works (no extra code)

`build_split_yaml.py` already classifies by function name:
`std::` → `middleware-stl`; `memcpy`/`strlen`/`_security_init_cookie`/… →
`middleware-crt`; `C{Wnd,String,…}::` → `middleware-mfc`; `inflate`/`crc32` →
`middleware-zlib`; `lua_*` → `middleware-lua`. Those patterns match on
*names*. Today the names are `FUN_xxx` so nothing matches and everything
stays in `matching`. After `apply` lands real names in `symbols.json`, the
next `make split` fires those patterns and the CRT/STL functions move to the
`middleware-*` tiers (i.e. out of the contributor pool).
