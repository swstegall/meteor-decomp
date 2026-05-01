# MSVC 2005 SP1 toolchain setup (Phase 2)

Matching decomp lives or dies on using the *exact* `cl.exe` and
`link.exe` the original developers used. Wrong service-pack → wrong
codegen → no amount of source tweaking will match.

For meteor-decomp, the target is **MSVC 14.00.50727.762** — Visual
Studio 2005 SP1 — based on the linker version 8.0 baked into all
five FFXIV 1.x PE binaries. See [`compiler-detection.md`](compiler-detection.md)
for the full evidence.

This document covers:

1. What to obtain
2. Where it can legitimately come from
3. How to lay it out for `tools/cl-wine.sh`
4. How to verify on the Rosetta Stone function

## 1. What to obtain

The minimum viable VS 2005 SP1 setup needs:

| Path                        | What                                  |
|-----------------------------|---------------------------------------|
| `bin/cl.exe`                | The C/C++ compiler driver             |
| `bin/c1.dll`                | C front-end                           |
| `bin/c1xx.dll`              | C++ front-end                         |
| `bin/c2.dll`                | Code generator                        |
| `bin/link.exe`              | Linker                                |
| `bin/cvtres.exe`            | Resource compiler (optional)          |
| `bin/ml.exe`                | MASM (optional, for any inline asm)   |
| `bin/mspdb80.dll`           | Symbol-server runtime                 |
| `include/`                  | C/C++ runtime + STL headers           |
| `lib/`                      | Static libraries (`libcmt.lib`,       |
|                             | `libcpmt.lib`, `kernel32.lib`,        |
|                             | `user32.lib`, `gdi32.lib`, etc.)      |
| Platform SDK 2003 R2 SP1    | Windows headers (`windows.h`,         |
|                             | `winsock2.h`, etc.) + import libs    |

You don't need the IDE (`devenv.exe`), the help docs, the F# / VB
compilers, or any of the redistributables. Cherry-pick the C++
compiler, linker, headers, and libs.

## 2. Legitimate sources

In strict order of preference:

### a) MSDN Subscriber Downloads (the canonical path)

If you have an MSDN / Visual Studio Subscription that covers VS 2005
(most legacy enterprise subscriptions do), the SP1 ISO is at:

> Visual Studio 2005 → Visual Studio Professional 2005 → Visual
> Studio 2005 Professional Edition (English) (SP1 included for
> later builds) — `en_vs_2005_pro_sp1.iso`

Plus the Platform SDK 2003 R2 SP1 ISO. Both have valid license keys
attached to your subscription.

### b) `archive.org` Microsoft archive

Microsoft's older shipping ISOs were archived to the Internet
Archive's Software Library:

> <https://archive.org/details/microsoft>
> <https://archive.org/details/Microsoft-Visual-Studio-Express-2005>

(The Express SKU has the same `cl.exe` as Pro; SP1 builds match
byte-for-byte across SKUs because only the IDE differs.)

These items typically carry a "Microsoft Permitted Items" license
note in the `details` page — Microsoft has historically allowed
archive.org to redistribute end-of-life Visual Studio shipped
products. Confirm the license note is present before downloading.

### c) Microsoft's own legacy download URLs (rare survivors)

Microsoft has periodically purged old download endpoints, but a few
SP1 patch installers still exist on Microsoft Update Catalog:

> <https://www.catalog.update.microsoft.com/Search.aspx?q=Visual+Studio+2005+SP1>

These are **patches**, not full installers — useful only if you
already have the RTM release.

### d) Community redistribution (LEGO Island decomp's setup)

The LEGO Island decomp has a documented `setup-msvc.sh` recipe that
points at known-good, Microsoft-licensed redistributable bundles.
We cross-reference their setup notes:

> <https://github.com/isledecomp/isle/blob/master/docs/dev_setup.md>

Match their procurement steps; their MSVC version is similar
(MSVC 4.20 / VC ~6 era for the original 1997 binary, but they have
sister projects on VS 2005). When in doubt, copy LEGO Island's
recipe verbatim.

## 3. Layout for `tools/cl-wine.sh`

Pick a stable path outside the repo (don't commit MSVC binaries) and
export `MSVC_TOOLCHAIN_DIR`:

```sh
export MSVC_TOOLCHAIN_DIR="$HOME/sdk/msvc-2005-sp1"
```

Lay it out as:

```
$MSVC_TOOLCHAIN_DIR/
├── VC/
│   ├── bin/
│   │   ├── cl.exe
│   │   ├── link.exe
│   │   ├── c1.dll
│   │   ├── c1xx.dll
│   │   ├── c2.dll
│   │   └── mspdb80.dll
│   ├── include/        # C/C++ + STL headers
│   └── lib/            # libcmt.lib etc.
└── PSDK/               # Platform SDK 2003 R2 SP1
    ├── Include/
    └── Lib/
```

`cl-wine.sh` will set `INCLUDE` to `VC/include;PSDK/Include` and
`LIB` to `VC/lib;PSDK/Lib` automatically. Adjust the paths in
`cl-wine.sh` if your layout differs.

## 4. Wine on Apple Silicon

Wine runs x86 PE binaries on Apple Silicon via Rosetta 2. Two
practical Wine builds:

- **wine-stable** (`brew install --cask wine-stable`) — vanilla
  upstream Wine. Slowest but most predictable.
- **CrossOver Wine** — what `xiv1point0-apple-silicon-installer`
  uses for the FFXIV game itself. Already configured in the
  workspace; can reuse the same prefix for compilation if desired.

For meteor-decomp's compilation needs, **vanilla Wine is fine and
simpler.** No DX9, no audio, no graphics — just process invocation
on a small set of `.exe` and `.dll` files.

```sh
brew install --cask wine-stable
wine --version  # expect wine-9.x or wine-10.x

# One-time prefix init (32-bit, since cl.exe is i386)
WINEPREFIX="$HOME/.wine-msvc2005" WINEARCH=win32 wine cmd /c exit
```

Then `cl-wine.sh` sets `WINEPREFIX=$HOME/.wine-msvc2005` so the
compilation prefix is isolated from any other Wine usage.

## 5. Verifying on the Rosetta Stone function

Phase 2's exit criterion is a single, hand-translated function from
the binary that compiles to byte-identical bytes when run through
`cl-wine.sh` with the chosen flags.

```sh
# 1. Pick the candidate function (tools/find_rosetta.py picks one for you).
python3 tools/find_rosetta.py ffxivgame
# → build/rosetta/ffxivgame.top.txt

# 2. Hand-translate it to C in src/ffxivgame/_rosetta/<sym>.cpp.
#    (One has been pre-staged based on Ghidra's first-pass decompile;
#    you'll need to revise it.)

# 3. Compile + diff.
make rosetta

# Iterate compiler flags in Makefile until objdiff reports zero delta.
```

Common flag-search axes (in rough order of likelihood to be the
culprit):

| Knob              | Try         |
|-------------------|-------------|
| Optimization      | `/O2` then `/Ox`, `/O1` |
| Frame pointer     | `/Oy` vs `/Oy-` |
| String pooling    | `/GF` |
| Function-level linking | `/Gy` |
| Stack security cookies | `/GS` vs `/GS-` |
| RTTI              | `/GR` (almost certainly on) |
| Exception handling | `/EHsc` |
| `wchar_t` builtin | `/Zc:wchar_t` vs `/Zc:wchar_t-` |
| Standard `for`    | `/Zc:forScope` |
| CRT linkage       | `/MT` vs `/MD` (`/MT` for ship games) |
| Float model       | `/fp:precise` (default) |
| Inline expansion  | `/Ob2` (default at /O2) |

Lock in the matching set as `MSVC_FLAGS=` in the Makefile once the
Rosetta Stone matches.

## 6. What if VS 2005 SP1 can't be procured?

Drop to **functional decomp** (PLAN.md §3 — the hybrid model). The
matching tier is only a strict win for net/sqpack/file-format
modules where byte-equivalence is unambiguous correctness. The bulk
of the decomp value (game-state structures, battle math, director
framework) lives in the functional tier and doesn't need a matching
toolchain.

This is the fallback if Phase 2 stalls indefinitely on procurement.
Phase 3 (net layer) can run as functional decomp first, with
matching upgraded later when the toolchain is in place.
