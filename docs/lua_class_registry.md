# Phase 6 item #3 — Lua class registry + script-tree layout

> Last updated: 2026-05-03 — registration table extracted; script
> source-tree layout recovered.

## The Lua class registry function

`FUN_0078e3a0` (file `0x38e3a0`, **1986 bytes**) is the
**single Lua-class registration function**. Every reference to
the Lua class names + script-tree paths in the binary clusters
inside this one function — confirmed by cross-referencing all
known class names (`Global`, `OtherArea`, `AreaBaseClass`,
`JudgeBaseClass`, `CommandBaseClass`, `QuestBaseClass`,
`DirectorBaseClass`, etc.) against function ranges; every site
sits inside `FUN_0078e3a0`.

This function runs at engine startup and registers every Lua-
bindable C++ class with the VM, pairing each with its `.prog`
(compiled Lua bytecode) source file path.

## Registered Lua classes (full enumeration)

The class-name strings live contiguously in `.rdata` starting at
file `0xbe05a8`. Two distinct groups:

### Engine / utility classes (no `*BaseClass` suffix — globals)

| Lua name | Source path | Notes |
|---|---|---|
| `Global` | `/Global.prog` | Global module (also aliased as `global`) |
| `Debug` | `/System/Debug.prog` | Debug module (also `_debug` → `debug`) |
| `Math` | `/System/Math.prog` | Math module (also `_math` → `math`) |
| `String` | `/System/String.prog` | String module (also `_string` → `string`) |
| `Table` | `/System/Table.prog` | Table module (also `_table` → `table`) |
| `WorldMaster` | `/World/WorldMaster.prog` | World-master script |
| `OtherArea` | `/World/OtherArea.prog` | Sibling-area-info script |
| `SpreadSheet` | `/GameData/SpreadSheet.prog` | Generic CSV/spreadsheet binding |
| `CutScene` | `/GameData/CutScene` (no `.prog`) | Cut-scene script binding |
| `CommandDebuggerGM` | `/CommandDebugger/CommandDebuggerGM` | GM debug commands |
| `CommandDebuggerDEV` | `/CommandDebugger/CommandDebuggerDEV` | Dev debug commands |
| `CommandDebuggerTEST` | `/CommandDebugger/CommandDebuggerTEST` | Test debug commands |

### Script-binding base classes (the `*BaseClass` family)

These are the C++ Lua-binding bases documented in
`docs/director_quest.md`. The `.prog` file at the listed path is
the **Lua-side base class** that quest / director / actor / etc.
scripts subclass.

| Lua class name | Source path | C++ vtable | Slots |
|---|---|---|---:|
| `SystemBaseClass` | `/System/SystemBaseClass` | (parent of all) | — |
| `ProgDebugBaseClass` | `/prog/ProgDebugBaseClass` | (debug build only) | — |
| `WorldBaseClass` | `/World/WorldBaseClass.prog` | — | — |
| `AreaBaseClass` | `/Area/AreaBaseClass.prog` | `0xbd63d4` | 35 |
| `ZoneBaseClass` | `/Area/Zone/ZoneBaseClass.prog` | — | — |
| `PrivateAreaBaseClass` | `/Area/PrivateArea/PrivateAreaBaseClass.prog` | `0xbd653c` | 35 |
| `CharaBaseClass` | `/Chara/CharaBaseClass.prog` | `0xbd5cac` | 41 |
| `PlayerBaseClass` | `/Chara/Player/PlayerBaseClass.prog` | `0xbd5e04` | 133 |
| `NpcBaseClass` | `/Chara/Npc/NpcBaseClass.prog` | `0xbd647c` | 41 |
| `ActorBaseClass` | `/ActorBaseClass.prog` | `0xbd4fe4` | 34 |
| `DirectorBaseClass` | `/Director/DirectorBaseClass.prog` | `0xbd5d6c` | 34 |
| `QuestBaseClass` | `/Quest/QuestBaseClass` | `0xbdfdd0` | 35 |
| `DebugBaseClass` | `/Debug/DebugBaseClass.prog` | `0xbd5274` | 34 |
| `JudgeBaseClass` | `/Judge/JudgeBaseClass` | `0xbdfd38` | 1 |
| `CommandBaseClass` | `/Command/CommandBaseClass` | `0xbdf834` | 1 |
| `StatusBaseClass` | `/Status/StatusBaseClass` | `0xbdf8d4` | 1 |
| `CommandDebuggerBaseClass` | `/CommandDebugger/CommandDebuggerBaseClass` | `0xbd510c` | 34 |
| `CommandDebuggerFUNCBaseClass` | `/CommandDebugger/CommandDebuggerFUNC/CommandDebuggerFUNCBaseClass` | — | — |
| `ItemBaseClass` | `/Item/ItemBaseClass.prog` | `0xbd5464` | 7 |
| `ImportantItemBaseClass` | `/Item/Important/ImportantItemBaseClass` | — | — |
| `MoneyItemBaseClass` | `/Item/Money/MoneyItemBaseClass` | — | — |
| `NormalItemBaseClass` | `/Item/Normal/NormalItemBaseClass.prog` | — | — |
| `GameDataBaseClass` | `/GameData/GameDataBaseClass` | — | — |
| `GroupBaseClass` | `/Group/GroupBaseClass.prog` | `0xbd53ac` | 7 |
| `Player` (instance) | `/Chara/Player/Player` | (player instance, not a base) | — |

### Embedded Lua bootstrap

A small Lua script literal is embedded directly in `.rdata` at
file `0xbe08b0`:

```lua
function ProgDebugBaseClass:_onInit()
    self:_callSuperClassFunc("_onInit")
    self.progDebugWork._temp = {
        {"_assignForChild",256},
        ...
    }
end
```

This is the seed snippet for the debug-build `progDebug` class
hierarchy. It tells us:

- The standard Lua-side super-class call pattern is
  `self:_callSuperClassFunc("methodName")`.
- The work-table assignment pattern declares each field as
  `{name, default}` pairs (the `256` is the default value for
  `_assignForChild`).
- `_onInit` is the standard "constructor" method name (also
  visible as a string at file `0xd0e7e8` in the
  `Component::Lua::GameEngine` runtime).

Other lifecycle method names visible in the binary:
`_onInit`, `_onFinalize`, `_onTimer` — the standard Lua-side
lifecycle hooks for any LuaControl subclass.

## File extensions

| Ext | Role | Hits |
|---|---|---:|
| `.prog` | Compiled Lua bytecode (script-tree leaf files) | 26 |
| `.lpb` | Compiled Lua bytecode (probably a different compilation pipeline / version) | 4 |
| `.san` | StaticActor data (binary actor blueprints) | 1 |

`.prog` is the dominant extension. The 4 `.lpb` references are
in narrower contexts (one for Quest paths, one in the runtime
init code) — likely a legacy / second-pipeline format that
co-exists.

## Script-tree directory layout

The script tree is rooted somewhere under the game's Lua
sandbox path (probably `data/lua/` in the install). The
top-level subdirectories visible in this registration table:

- `/Global.prog`
- `/ActorBaseClass.prog`
- `/Area/AreaBaseClass.prog`
- `/Area/Zone/ZoneBaseClass.prog`
- `/Area/PrivateArea/PrivateAreaBaseClass.prog`
- `/Chara/CharaBaseClass.prog`
- `/Chara/Player/PlayerBaseClass.prog`
- `/Chara/Player/Player`
- `/Chara/Npc/NpcBaseClass.prog`
- `/Command/CommandBaseClass`
- `/CommandDebugger/CommandDebuggerBaseClass`
- `/CommandDebugger/CommandDebugger{GM,DEV,TEST}`
- `/CommandDebugger/CommandDebuggerFUNC/CommandDebuggerFUNCBaseClass`
- `/Debug/DebugBaseClass.prog`
- `/Director/DirectorBaseClass.prog`
- `/GameData/GameDataBaseClass`
- `/GameData/CutScene`
- `/GameData/SpreadSheet.prog`
- `/Group/GroupBaseClass.prog`
- `/Item/ItemBaseClass.prog`
- `/Item/Important/ImportantItemBaseClass`
- `/Item/Money/MoneyItemBaseClass`
- `/Item/Normal/NormalItemBaseClass.prog`
- `/Judge/JudgeBaseClass`
- `/prog/ProgDebugBaseClass` (debug build)
- `/Quest/QuestBaseClass`
- `/Status/StatusBaseClass`
- `/StaticActor` + `.san` extension
- `/System/SystemBaseClass`
- `/System/Debug.prog`
- `/System/Math.prog`
- `/System/String.prog`
- `/System/Table.prog`
- `/World/WorldBaseClass.prog`
- `/World/WorldMaster.prog`
- `/World/OtherArea.prog`

The pattern: each directory holds the base-class script for that
domain, with sub-domains nested. Concrete instances (like
`man0g0` for the Limsa intro quest) would live at
`/Quest/<tribe>/<instance_name>` — extending `/Quest/QuestBaseClass`.

## Practical impact for garlemald

This is huge for garlemald's Lua-script loader:

1. **Exact script path conventions confirmed.** Garlemald already
   serves Lua scripts from a `scripts/lua/` tree; the per-domain
   subdirectory layout (`/Quest/`, `/Director/`, `/Area/`,
   `/Chara/{Player,Npc}/`, `/Item/{Important,Money,Normal}/`) is
   the EXACT convention the client expects. Garlemald should
   mirror this layout when serving scripts to the client.
2. **Class-name registration is fixed.** The `*BaseClass`
   identifiers (e.g. `QuestBaseClass`, `DirectorBaseClass`,
   `PrivateAreaBaseClass`) are the Lua-side names that scripts
   must subclass. Project Meteor and garlemald already use these
   names; the binary confirms them.
3. **Lua lifecycle methods are `_onInit`, `_onFinalize`, `_onTimer`.**
   Any garlemald-served script that wants to hook lifecycle
   needs to use exactly these names.
4. **Super-class invocation idiom: `self:_callSuperClassFunc("method")`.**
   This is the engine's standard pattern for chaining to the
   parent class's implementation of a method. Garlemald-served
   scripts must use this exact form (not Lua's standard
   `Parent.method(self, ...)` form, because the engine's
   class system manages the inheritance chain).
5. **The `.prog` file extension is the standard.** Garlemald's
   on-disk script files should be `.prog` (or whatever
   extension is configured to be served as compiled Lua bytecode);
   the `.lpb` extension is a secondary format.

## Cross-references

- `docs/director_quest.md` — Phase 6 item #1+#2 (the C++ side
  Lua-binding base classes that pair with the Lua names here)
- `docs/wire-protocol.md` — Phase 3 (the wire layer that
  transports script-state-sync via SyncWriter)
- `garlemald-server/scripts/lua/` — the Lua script tree garlemald
  serves; should be cross-checked against the directory layout
  here
- `project_meteor_discord_context.md` — Ioncannon notes on Lua
  scripting (`processEvent`, `Seq000`, `MotionPack ID
  (1000-1109)`, `talkDefault`, `quest:GetData()`,
  `populaceStandard`, `ElevatorStandard.lua`, `Shop.lua`)
- `project-meteor-server/scripts/quests/` — C# reference impl's
  Lua script tree; should be a close match to the layout here
