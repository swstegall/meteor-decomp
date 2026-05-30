# NOTICE

`meteor-decomp` is a clean-room decompilation of the FINAL FANTASY XIV
v1.23b Windows client binaries (`ffxivgame.exe`, `ffxivboot.exe`,
`ffxivlogin.exe`, `ffxivupdater.exe`, `ffxivconfig.exe`). Source code
recovered here is written by reading disassembled output of binaries
that the contributor obtained from a legitimate retail install. It is
**clean-room** in the sense that matters — no original Square Enix
source code is used; everything is reconstructed from the binary.

The work is **collaborative**, not the sole authorship of one
contributor: it incorporates reverse-engineering findings from the
`FFXIVLegacyClientStructs` and `ffxivDecomp` projects
(github.com/Yokimitsuro), used with their author's express permission.
Those projects are themselves clean-room (RE'd from the same retail
binary, not from SE source), so incorporating them does not introduce
any Square Enix source. See the per-project sections below.

The original `.exe` binaries themselves are copyright Square Enix
Holdings Co., Ltd. and are NOT distributed with this repository. They
must be supplied by the user from a legitimate
`ffxiv-install-environment` install.

The recovery effort builds on prior reverse-engineering work by:

## Project Meteor Server

- Source: <https://bitbucket.org/Ioncannon/project-meteor-server>
- License: GNU Affero General Public License v3.0 (AGPL-3.0)

Project Meteor Server is the C# FFXIV 1.23b server emulator
(Lobby / World / Map) maintained by Ioncannon and contributors. Its
packet-header reverse engineering, opcode tables, Blowfish session
handshake, actor-system field names, ZiPatch format notes, and Lua
director scaffolding are the principal symbol-naming and structural
seed for this decomp.

## Seventh Umbral

- Source: <https://github.com/Meteor-Project/SeventhUmbral>
- License: 2-clause BSD-style (see upstream `License.txt`)

Seventh Umbral is the original Windows-only C++ launcher and
client-side research suite used to drive the 1.23b client against
Project Meteor Server. Its packet structs, Blowfish helpers, and
PE-patching scaffolding inform corresponding modules in
`meteor-decomp`.

## FFXIVLegacyClientStructs

- Source: <https://github.com/Yokimitsuro/FFXIVLegacyClientStructs>
- License: MIT (see upstream `LICENSE`) — also used with the author's
  express permission as a collaborator

FFXIVLegacyClientStructs is a separate RTTI-driven reverse
engineering of the same 1.23b `ffxivgame.exe`, modeled on the retail
`FFXIVClientStructs` project. It ships ~2,572 `[StructLayout]`-explicit
structs (RTTI mangled name, struct size, per-field offsets/names) plus
a 4,358-row RTTI database (vtable VA / vfunc count / TypeDescriptor VA /
mangled name). Its struct field layouts, constructor RVAs, and struct
sizes are imported into our type catalog via
`tools/import_legacy_structs.py`, which emits
`config/ffxivgame.legacy_structs.json`,
`config/ffxivgame.legacy_symbols.json`, and per-class notes under
`decomp-notes/types/ffxivgame/`. Imported layouts are flagged
**cross-referenced, not byte-verified** and are confirmed against our
own asm before a match is accepted. Cross-validation against our own
Ghidra RTTI walk agrees on 2,270 shared vtables (the few differences
are cosmetic demangling normalization). Its bundled
`FFXIVClientStructs.Tools.CLI` is also wrapped by
`tools/analyze_legacy_struct.sh` for on-demand layout recovery — run
against our own `orig/ffxivgame.exe`, so the recovered offsets are facts
from our binary.

## ffxivDecomp

- Source: <https://github.com/Yokimitsuro/ffxivDecomp>
- Usage: incorporated with the author's express permission

ffxivDecomp is a docs-only reverse-engineering analysis of the same
1.23b `ffxivgame.exe` (finding write-ups; no decompiled function
bodies). Its findings — function names paired with virtual addresses,
opcode rosters, Lua-binding names, dispatch structure, and the
accompanying analysis — are incorporated into meteor-decomp with the
author's (Yokimitsuro's) express permission. The structured import
(`tools/import_ffxivdecomp_symbols.py`) emits
`config/ffxivgame.ffxivdecomp_symbols.json` and
`docs/ffxivdecomp_opcode_binding_map.md`; imported (name, VA) pairs are
cross-checked against our own `config/ffxivgame.symbols.json` and
confirmed against our own disassembly as a **correctness** practice (so
a match doesn't inherit an upstream error), and the opcode set is
cross-validated against our own `up_opcodes` extraction. As of the
2026-05-28 refresh, the importer's bulk name source is ffxivDecomp's
full USER_DEFINED Ghidra symbol export
(`docs/re/ghidra_symbols_userdefined.tsv`, ~993 VA→name pairs); the
curated prose findings still supply the opcode / Lua-binding metadata
and win by VA where they overlap.

## LandSandBoat (referenced, not copied)

- Source: <https://github.com/LandSandBoat/server>
- License: GNU General Public License v3.0 (GPL-3.0)

LandSandBoat is the most actively-maintained open-source Final
Fantasy XI server emulator. FFXIV 1.x inherited XI's combat /
status / aggro / damage-formula grammar through its shared design
team (Hiromichi Tanaka, Nobuaki Komoto), and LandSandBoat's annotated
formulas are referenced as a structural cross-check when
re-deriving the corresponding 1.x routines. **No code is copied
from LandSandBoat.** Where its citations are useful, they appear
as breadcrumb comments (`// LSB: physical_utilities.lua::wRatioCapPC`)
that point at the file being mined.

LandSandBoat is GPL-3.0 and `meteor-decomp` is AGPL-3.0-or-later;
verbatim translation would trigger the combined-work clause. We
therefore "read, re-derive, cite" as documented in this workspace's
top-level `CLAUDE.md`.

## LEGO Island Decompilation (methodology reference)

- Source: <https://github.com/isledecomp/isle>
- License: BSD-3-Clause

The LEGO Island decomp is the leading reference for matching decomps
of MSVC-2005-era PE32 i386 binaries. Its toolchain (Wine + VS 2005
SP1 `cl.exe` + `objdiff` + per-function `.cpp` files keyed by RVA)
and its function-by-function workflow are adopted here. **No code is
copied from LEGO Island.**

## License of this repository

This repository is licensed under **AGPL-3.0-or-later**. See
[`LICENSE.md`](LICENSE.md) for the full GNU AGPL v3 text. New source
files in this tree open with the AGPL boilerplate header and the
project tagline:

```
meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
Copyright (C) 2026  Samuel Stegall

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License ...

SPDX-License-Identifier: AGPL-3.0-or-later
```

The comment syntax (`//` for C/C++, `#` for Python and shell) varies
per file but the wording is identical across the workspace's AGPL
projects.
