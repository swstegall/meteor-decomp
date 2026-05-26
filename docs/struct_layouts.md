# Struct layouts — ffxivgame (FFXIVLegacyClientStructs × RTTI)

> Generated 2026-05-25 by `tools/build_struct_layouts.py`. Re-run via `make struct-layouts`.

FFXIVLECS addresses are from a different client build and do NOT align with this binary; join is by RTTI vtable name. Field layouts validated independently (CharaActor.PositionX @ 0x154).

## Summary

- FFXIVLECS classes imported: **2572**
- Confidently joined to a binary RTTI vtable (by demangled name): **1058**
  - of which appear in >1 vtable (multi-inheritance / dupes): 367
- Field-rich classes (≥ 2 named fields, header generated): **7**
- Total named field definitions: **1265**

The join gives every one of those classes a concrete vtable RVA in this binary, so their virtual methods can be named by class; the field-rich classes additionally get usable struct layouts.

## Field-rich classes (struct headers under `include/structs/ffxivgame/`)

| Class | Fields | Size | vtable RVA(s) |
|---|---:|---:|---|
| `Sqex::Socket::SocketBase` | 59 | 420 | 0xd132dc |
| `Application::Scene::Actor::Chara::CharaActor` | 58 | 11184 | 0xbc0d34 |
| `Application::Scene::Actor::System::TargetActor` | 40 | 2496 | 0xbb83cc |
| `Application::Scene::Actor::System::BootupActor` | 31 | 2496 | 0xbb86c4 |
| `Application::Scene::Actor::System::GameManagerActor` | 17 | 2496 | 0xbb8134 |
| `Application::Scene::Actor::Chara::CharaCutVisualCtrl` | 7 | 80 | 0xc4447c |
| `SQEX::CDev::Engine::Fw::SceneObject::Actor` | 2 | 256 | 0xc9ca94 |

Generated 7 header(s). The full join (all 1058 classes, with vtable RVAs + sizes) is in `config/ffxivgame.struct_layouts.json` — the durable artifact a future Ghidra session can import to apply these layouts wholesale.
