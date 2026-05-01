# Seed symbol sources

This document lists where every name in `config/symbols.txt` comes
from. As the project's symbol-naming convention solidifies, new names
get back-cited here so the next contributor can replay the
derivation.

Convention: each entry follows a fixed shape

```
<symbol> @ <rva> — source: <one-line citation> ; confidence: <high | medium | low>
```

## Initial seeds (Phase 0)

Seeded from the `CLAUDE.md` cross-references. None of these are
RVA-pinned yet — they'll get pinned once Ghidra has run and a
contributor associates the name with a specific function.

### From Project Meteor Server (C# class names)

- `Actor`, `BattleNpc`, `Npc`, `Aetheryte`, `Player` — class hierarchy.
- `Director`, `OpeningDirector`, `ZoneDirector`, `WeatherDirector`,
  `QuestDirector`, `GuildleveDirector`, `EventDirector`,
  `CraftDirector`, `HarvestDirector`, `ContentArea`, `PrivateArea`.
- `BasePacket`, `SubPacket`, `SetActorPropertyPacket`,
  `EventStartPacket`, `KickEventPacket`, `RunEventFunctionPacket`,
  `EndEventPacket`, `SetEventStatusPacket`, `SetActorQuestGraphicPacket`.
- `Quest`, `QuestState`, `QuestData`, `QuestSequence`.
- `Inventory`, `InventoryItem`.
- `World`, `Lobby`, `Map`, `Zone`.
- `Lua`, `LuaPlayer`, `LuaQuest`, `LuaActor` — Lua binding wrappers.

Source: `project-meteor-server/`, `ffxiv-project-meteor-server-develop/`,
`ffxiv-project-meteor-server-vooplv-ver/` — all three C# variants
agree on these names.

### From Seventh Umbral (C++ ports)

- Packet helper structs (`PacketHeader`, `BlowfishCipher`,
  `SqpackHash`, `SqpackIndex`, `ZiPatchEntry`).
- PE-patch helpers (will appear in `ffxivboot.exe` more than the
  game proper).

Source: `SeventhUmbral/`.

### From the wikis (numeric constants)

These appear as `mov eax, <id>` or `cmp eax, <id>` literals; they
let us pin functions by their constant references.

| Domain  | Constant range | Source                                              |
|---------|----------------|-----------------------------------------------------|
| Zone IDs | 100-300       | `ffxiv_classic_wiki_context.md` (Region/Weather/Music IDs section) |
| Weather | 0-30           | same                                                |
| Music   | varies         | same                                                |
| Opcodes | server→client / client→server, separate ranges | FFXIV 1.0 Opcodes spreadsheet, mined into `ffxiv_linkchannel_context.md` |
| Item IDs | 1-7311        | `ffxiv_mozk_tabetai_context.md`                     |
| Battle command IDs | 23000-29200 | `ffxiv_1x_battle_commands_context.md`           |
| Motion pack IDs | 1000-1109 | `project_meteor_discord_context.md` (Populace Motion Pack discussion) |
| Class job IDs | 1-44     | `ffxiv_mozk_tabetai_context.md`                     |
| Stat param IDs | 1-260   | `ffxiv_mozk_tabetai_context.md`                     |

### From `.rdata` string literals (Phase 1 seeds)

Once Ghidra is run, `tools/ghidra_scripts/dump_strings.py` writes
every string in `.rdata` to `config/strings.json` with its RVA. We
expect these patterns to surface function names:

- `__FILE__` macros: `c:\\dev\\ffxiv\\src\\<module>\\<file>.cpp`. The
  path components ARE module names.
- `__FUNCTION__` macros: `<ClassName>::<MethodName>`. Direct hits.
- `assert` / `Verify` strings: `"actor != nullptr"` near a function
  using `actor` as parameter; pin function by the closest `string
  reference site → caller`.
- Lua function names: `"onTalk"`, `"onPush"`, `"onStateChange"`,
  `"Seq000"`, `"populaceStandard"`, `"ElevatorStandard"`. These name
  the Lua *binding glue*, not the script side.
- Director state names: `"DIRECTOR_STATE_INIT"`, `"DIRECTOR_STATE_RUN"`,
  etc. Pin Director functions.

### From RTTI (Phase 1)

`tools/ghidra_scripts/dump_rtti.py` writes vtable + class hierarchy
data to `config/rtti.json`. MSVC RTTI exposes:

- `??_R0` — `type_info` records → fully-qualified class names.
- `??_R1` — Hierarchy descriptor → base classes (parent vtables).
- `??_R2` — Base class array.
- `??_R3` — Class hierarchy descriptor.
- `??_R4` — Complete object locator → vtable address → class name.

Walking `??_R4` records gives us *every* polymorphic class name in
the binary. For a 3+ MB `.rdata` PE this is typically thousands of
classes.

### From the binary's own `__FILE__` macros

Source: TBD, populated when Phase 1 runs. Pin the Ghidra-discovered
file paths as `notes:` on the corresponding `config/ffxivgame.yaml`
rows.
