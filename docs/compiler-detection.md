# Compiler detection — pinning the toolchain

Matching decomp lives or dies on using the same `cl.exe` and `link.exe`
the original developers used. Getting this wrong by even a service
pack can leave fundamental codegen mismatches (different
`__chkstk` thresholds, different stack-protector cookie placement,
different jump-table emission) that no amount of source tweaking will
fix.

## What the binary tells us

From `IMAGE_OPTIONAL_HEADER`:

```
MajorLinkerVersion = 8
MinorLinkerVersion = 0
```

Linker 8.0 ships with **Visual Studio 2005**. Microsoft's own
mapping:

| MSVC | Visual Studio | `_MSC_VER` | `cl.exe` |
|------|---------------|------------|----------|
| 7.0  | VS 2002       | 1300       | 13.00.x  |
| 7.1  | VS 2003       | 1310       | 13.10.x  |
| **8.0** | **VS 2005**   | **1400**   | **14.00.x** |
| 9.0  | VS 2008       | 1500       | 15.00.x  |
| 10.0 | VS 2010       | 1600       | 16.00.x  |
| 11.0 | VS 2012       | 1700       | 17.00.x  |

So the toolchain is **VS 2005**, but VS 2005 had two service packs:

- **VS 2005 RTM** — `cl.exe 14.00.50727.42`
- **VS 2005 SP1** — `cl.exe 14.00.50727.762`
- **VS 2005 SP1 + ATL Security Update** — `cl.exe 14.00.50727.4035`

The release date of FFXIV ARR's predecessor, the original FFXIV 1.0
launch, is 2010-09-30 — the boot/updater builds are from 2010-09-16
which is two weeks before launch and well after VS 2005 SP1
(November 2006). The game was almost certainly built with **VS 2005
SP1** (or SP1 + ATL update). RTM is unlikely — by 2010, no shipping
title would still be on RTM.

We pick **VS 2005 SP1** as the working hypothesis and verify on the
Rosetta Stone function (see [`matching-workflow.md`](matching-workflow.md))
by re-targeting SP1 vs RTM vs SP1+ATL and seeing which one matches
default `/O2` codegen for a 30-line landmark function. If SP1 fails
and SP1+ATL matches, we switch.

## Compiler flags hypothesis

Educated guesses for the original build flags, to seed flag-search:

| Flag         | Why                                                               |
|--------------|-------------------------------------------------------------------|
| `/O2`        | Standard "favour speed" — every shipping game does this.          |
| `/Oy-` or `/Oy` | Frame-pointer omission. Ship games typically `/Oy` for speed.  |
| `/GR`        | RTTI on (we observe RTTI in the binary).                          |
| `/EHsc`      | Standard SEH + C++ exceptions.                                    |
| `/Gy`        | Function-level linking — necessary for `/OPT:REF` to be useful.   |
| `/GS-` or `/GS` | Buffer security check. Typically ON for retail; the binary's `__security_cookie` symbol will tell us which. |
| `/Zc:wchar_t` | `wchar_t` as built-in. MSVC 2005 default is OFF; games often flip it on. |
| `/Zc:forScope` | Standards-compliant `for` scoping.                              |
| `/MT`        | Static CRT linkage. Ship games avoid `/MD` (no DLL CRT dep).      |
| `/W3` or `/W4` | Compiler warnings — unobservable in binary.                     |

We confirm each by inspection:

- **`/MT`** vs `/MD` → look for `MSVCR80.dll` import. If absent, `/MT`.
- **`/GS`** vs `/GS-` → grep `.rdata` for `__security_cookie` literal.
  If present and referenced from prologues, `/GS` is on.
- **`/GR`** confirmed by RTTI presence (`type_info`, `??_R0` mangled
  symbols in `.rdata`).

`tools/extract_pe.py` (Phase 1) reports each of these.

## Linker flags hypothesis

| Flag                | Why                                                    |
|---------------------|--------------------------------------------------------|
| `/SUBSYSTEM:WINDOWS` | Confirmed by header (subsystem=2).                    |
| `/FIXED:NO`         | Defaults; `relocs_stripped` characteristic is present so this is actually `/FIXED` — but games are typically loaded at `0x400000` regardless. |
| `/STACK:reserve,commit` | Probe via `IMAGE_OPTIONAL_HEADER.SizeOfStackReserve`. |
| `/HEAP:reserve,commit`  | Same — read from header.                            |
| `/SECTION:MSSMIXER,ER`  | Custom section directive — Miles SDK shipped this.  |
| `/MERGE:.tls=.data` (no) | Their `.tls` is its own section.                  |

The `MSSMIXER` section name is itself a fingerprint of the **Miles
Sound System SDK** version that was shipped — RAD Game Tools' Miles
9.x of the era used `MSSMIXER` as the section name for the
statically-linked mixer DSP. Knowing this avoids wasted effort
"decompiling" what's actually third-party middleware; see
[`known-libraries.md`](known-libraries.md).

## Confirmation procedure (Phase 2 deliverable)

```sh
# Procure VS 2005 SP1 and Platform SDK 2003 R2
./tools/setup-msvc.sh

# Pick a Rosetta Stone function. tools/find_rosetta.py scans the
# decompiler output for the smallest non-trivial function whose
# pseudo-C is unambiguous (e.g. a hand-rolled strncmp, an integer
# clamp helper). It writes src/ffxivgame/_rosetta/rosetta.cpp.

# Try compiling with each candidate toolchain.
for tc in vs2005-rtm vs2005-sp1 vs2005-sp1-atl; do
    make rosetta MSVC_TOOLCHAIN=$tc
    objdiff orig/ffxivgame.exe build/obj/_rosetta.obj
done

# Lock the winner into the Makefile.
```
