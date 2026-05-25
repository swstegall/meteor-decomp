# On-demand struct analysis via the FFXIVLegacyClientStructs CLI

`tools/analyze_legacy_struct.sh` runs the FFXIVLegacyClientStructs CLI
(github.com/Yokimitsuro/FFXIVLegacyClientStructs, MIT) against **our own**
`orig/ffxivgame.exe`. The CLI is third-party tooling; pointing it at our
own legitimately-installed binary recovers facts from *our* binary — the
provenance-clean way to get a struct layout that the committed
FFXIVLegacyClientStructs `*.cs` left as a one-field shell.

It builds the CLI on first use (net8.0 target, rolled forward to the
installed net10 runtime).

## Usage

```sh
tools/analyze_legacy_struct.sh --analyze CharaActor                 # ctor-walk field offsets
tools/analyze_legacy_struct.sh --vtfuncs Client::Control::PlayerBase # vtable fn signatures
tools/analyze_legacy_struct.sh --hierarchy GameManagerActor          # base/derived chain
tools/analyze_legacy_struct.sh --vtable  Sqex::Socket::SocketBase    # raw vtable dump
tools/analyze_legacy_struct.sh --search  Director                    # class-name grep
```

`--analyze` output annotates **each field with the exact ctor instruction
address** that writes it, e.g. `+0x0154 [float] = xmm0 (@ 0x0065F209)` —
so when you're matching a ctor you can line offsets up against the asm
directly. Bulk machine catalog of the committed layouts lives in
`config/ffxivgame.legacy_structs.json` (see `tools/import_legacy_structs.py`).

## Yield — what `--analyze` can and cannot reach

`--analyze` walks a **standard `__thiscall` constructor**. That covers the
scene/system actor family and similar concrete classes:

- Works: `CharaActor` (55 fields), `GameManagerActor` (44 fields + 18
  sub-objects — *more* than the committed `.cs`'s 17, since live analysis
  is unabridged), `TargetActor`, `BootupActor`, `Client::Control::PlayerBase`
  (3 fields, ctor 0x00720F70), etc.
- **Cannot reach** (reports "Constructor not found"):
  - The GAM `Network::...::GameAttributeManager::Data::*` classes
    (`CharaMakeData`, `ClientSelectData`, `ZoneInitData`, `Player`,
    `PlayerPlayer`) — these are `CompileTimeParameterCollection` template
    instantiations with no walkable ctor. Use the GAM PARAMNAME dispatcher
    recovery instead (see the meteor-decomp PARAMNAME memory / chara-make
    validation work).
  - Most of the Lua `Control` hierarchy (`NpcBase`, `CharaBase`,
    `MyPlayer`, `DirectorBase`) — non-standard init; use `--vtable` /
    `--vtfuncs` / `--hierarchy` and the vtable→Lua-name recipe instead.

So this CLI is best used **on demand** while matching a concrete class's
ctor, not as a bulk catalog generator — the high-want server/decomp
classes (GAM data, Lua hierarchy) are exactly the ones it can't ctor-walk.
The ffxivDecomp import (`tools/import_ffxivdecomp_symbols.py`) is the
better lever for the Lua hierarchy (it names the registrar functions).
