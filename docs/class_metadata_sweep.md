# Phase 9 ext2 — Class-metadata sweep (129 RTTI → vtables + ctors/dtors)

> Last updated: 2026-05-16. Companion to
> `docs/dynamic_cast_callsite_sweep.md`. Extends the same PE-parse
> infrastructure to walk RTTI → COL → vtable → ctor/dtor sites for
> all 129 classes, producing a comprehensive class metadata registry.

## TL;DR

**Coverage**: 118 of 129 classes (91 %) had at least one vtable and
one ctor/dtor candidate site recovered automatically.

| Metric | Count |
|---|---:|
| Total classes processed | 129 |
| Classes with ≥1 vtable found | 118 |
| Classes with multiple vtables (MI) | 45 |
| Classes with ≥1 ctor candidate | 118 |
| Total vtables identified | 196 |
| Vtable slot count: min / median / max | 1 / 31 / 188 |

**Validation**: Reproduced Phase 9 #8d's manual ActorBase /
DirectorBase / CharaBase / NpcBase findings exactly — both ctor
(larger write offset, vtable install after parent-ctor call) and
dtor (smaller write offset, vtable install at start before base-dtor
chain) functions located byte-identically.

## What's in the output

`build/class_metadata.json` (regenerable; gitignored): a dict
keyed by demangled class name, with for each:

```json
{
  "rtti_rva":        0x870964,
  "rtti_abs":        0x1270964,
  "col_count":       1,
  "cols": [{ "rva": 0xbd4fb0, "abs": 0xfd4fb0, ... }],
  "vtable_count":    1,
  "vtables": [
    { "rva": 0xbd4fe4, "abs": 0xfd4fe4,
      "slot0": 0x718690, "slot_count": 34, "col_rva": 0xbd4fb0 }
  ],
  "ctor_candidates": [
    { "fn_rva": 0x2dbb70, "fn_name": "002dbb70_FUN_006dbb70.s",
      "vtable_rva": 0xbd4fe4, "write_offsets": [0x3a] },
    { "fn_rva": 0x2dbbe0, "fn_name": "002dbbe0_FUN_006dbbe0.s",
      "vtable_rva": 0xbd4fe4, "write_offsets": [0x2a] }
  ],
  "dtor_candidates": [],
  "uncategorized_sites": []
}
```

## Disambiguating ctor vs dtor within "ctor_candidates"

The sweep classifies any function with a single vtable-write as
"ctor candidate" (since MSVC dtors usually only write the vtable
once, the same as ctors). To disambiguate per-vtable:

- **Larger write offset → CTOR.** MSVC ctor pattern:
  ```
  [SEH prolog] → CALL parent_ctor → MOV [this], own_vtable → init members → return
  ```
  The vtable install comes AFTER the parent ctor call, so offset is
  bigger (typically `+0x36`–`+0x4d`).

- **Smaller write offset → DTOR.** MSVC dtor pattern:
  ```
  [SEH prolog] → MOV [this], own_vtable → destroy members → CALL parent_dtor → return
  ```
  The vtable install is at the function start (right after SEH
  prolog), so offset is smaller (typically `+0x2a`–`+0x2c`).

For Phase 9 #8d's 8 Lua-class examples, the disambiguation worked
100 %:

| Class | Vtable | Bigger offset (CTOR) | Smaller offset (DTOR) |
|---|---|---|---|
| `ActorBase` | `0xbd4fe4` | `+0x3a` → `FUN_006dbb70` | `+0x2a` → `FUN_006dbbe0` |
| `DirectorBase` | `0xbd5d6c` | `+0x3a` → `FUN_006f1310` | `+0x2a` → `FUN_006ecf90` |
| `NpcBase` | `0xbd647c` | `+0x3e` → `FUN_006f3650` | `+0x2c` → `FUN_006f37a0` |
| `CharaBase` | `0xbd5cac` | `+0x36` → `FUN_006ecd80` | `+0x2b` → `FUN_006ece20` |
| `PlayerBase` | `0xbd5e04` | `+0x35` → `FUN_006ed720` | `+0x27` → `FUN_006ed7a0` |
| `AreaBase` | `0xbd63d4` | `+0x36` → `FUN_006f3210` | `+0x2a` → `FUN_006f32a0` |
| `PrivateAreaBase` | `0xbd653c` | `+0x3d` → `FUN_006f3d90` | `+0x2a` → `FUN_006f3e00` |
| `QuestBase` | `0xbdfdd0` | `+0x4d` → `FUN_00776f50` | `+0x35` → `FUN_00776fc0` |

## Top-15 most-complex classes (by vtable count + ctor sites)

The most complex classes are dominated by Sqwt UI controls
(3 vtables = WPF-style MI from `DependencyObject` + `IInputElement` +
intermediate bases). Multi-vtable counts reveal multiple-inheritance:

| Class | # Vtables | Top vtable slot count | # Ctor/dtor candidates |
|---|---:|---:|---:|
| `Application::Main::RaptureElementContainer` | **6** | 81 | 12 |
| `Sqwt::Controls::Border` | 3 | 65 | 9 |
| `Sqwt::Controls::Button` | 3 | 73 | 9 |
| `Sqwt::Controls::Label` | 3 | 71 | 9 |
| `Sqwt::Controls::ListBoxItem` | 3 | 71 | 9 |
| `Sqwt::Controls::Primitives::ButtonBase` | 3 | 73 | 9 |
| `Sqwt::Controls::Primitives::ToggleButton` | 3 | 76 | 9 |
| `Sqwt::Controls::ProgressBar` | 3 | 73 | 9 |
| `Application::Lua::Script::Client::Control::Global` | 3 | 34 | 6 |
| `SQEX::CDev::Engine::Cut::Plugins::CameraClip` | 3 | 42 | 6 |
| `SQEX::CDev::Engine::Cut::Plugins::LocalCameraClip` | 3 | 43 | 6 |
| `SQEX::CDev::Engine::Lay::Default::External::Cut::Scheduler::LaySchedulerManipulator` | 3 | 31 | 6 |
| `Sqwt::Controls::CheckBox` | 3 | 76 | 6 |
| `Sqwt::Controls::ContentControl` | 3 | 71 | 6 |
| `Sqwt::Controls::ContentPresenter` | 3 | 66 | 6 |

**`RaptureElementContainer` has 6 vtables** — the heaviest MI class
in the binary. Per the dynamic_cast sweep it extends
`Sqwt::DesktopWindow` (the top-level window class), and the 6
vtables likely correspond to inherited interfaces:
`{DesktopWindow, IInputElement, UIElement-base, AppMain-base,
some-event-sink, some-IDispatch-equivalent}`.

**Most Sqwt::Controls have 3 vtables** — corresponding to the WPF
MI hierarchy of `{Control-base, IInputElement, IDispose-or-similar}`.
The vtable with ~70 slots is the main control vtable; the 4-slot
vtable is likely a small interface adapter.

## Classes with NO ctor candidate (11)

These are all expected gaps — either pure-abstract interfaces or
template instantiations:

| Class | Reason |
|---|---|
| `SQEX::CDev::Engine::Common::ISceneObject::IActor` | Pure-abstract interface — no concrete ctor |
| `SQEX::CDev::Engine::Cut::Scheduler::IClip` | Pure-abstract interface |
| `Application::Main::Element::Window::Debug::DebugLabelSpinBinder` | Template instantiation — defined in headers, inlined at use sites |
| `Application::Main::Element::Window::Debug::DebugLabelSpinRefBinder` | Template instantiation |
| `Application::Main::Element::Window::Debug::{H/I/M}::?$DebugBinderT` | Template instantiations (6 entries — H/I/M for byte/short/int + Ref variants) |
| `Sqex::Misc::VUtf8String::?$DebugBinderT` | Template instantiation |

The 8 template instantiations are normal MSVC behavior — templates
get inlined at every use site rather than emitted as discrete
functions, so they have a vtable (for RTTI) but no callable ctor in
the code section.

## Application: closing Phase 9 #8e (+0x5c writer hunt)

The +0x5c kick gate is on the Lua-side wrapper class hierarchy
rooted at `LuaControl`. Phase 9 #8e narrowed the writer candidates
to 6 functions but couldn't identify the real writer without
manual disambiguation.

Cross-checking the 6 candidates against the class-metadata sweep:

| Candidate fn | In any vtable? | Same containing function as any known ctor/dtor? |
|---|---|---|
| `FUN_00766f00` | NO | NO |
| `FUN_007b43e0` | NO | NO |
| `FUN_009018f0` | NO | NO |
| `FUN_00a42c90` | NO | NO (confirmed: Phase 7 sync primitive false positive) |
| `FUN_00acc050` | NO | NO |
| `FUN_00c54710` | NO | NO |

**None of the 6 candidates is in any class's vtable**, and none of
them appears in any class's ctor/dtor function list. So either:

1. The +0x5c writer is in a non-virtual member function of one of
   the 24 LuaControl-derived classes (callable directly, not via
   vtable — wouldn't show up in this sweep)
2. The +0x5c writer is in a class we haven't yet recovered via the
   dynamic_cast sweep (because nothing in the engine downcasts
   TO that class — only FROM it or never)
3. The writer is engine-internal scaffolding (not a Lua-bound
   class) — e.g. a packet handler that takes an actor pointer and
   sets the byte

For runtime-trace verification (when available), the most
productive starting point is now: set a breakpoint on
`[Lua-side ActorBase + 0x5c]` and see what packet handler triggers
it during the spawn sequence.

## Application: ctor/dtor reference for any future class investigation

For ANY of the 118 classes with metadata, the path "what code
constructs/destroys this?" is now a 1-step lookup:

```python
import json
md = json.load(open('build/class_metadata.json'))
info = md['Application::Lua::Script::Client::Control::StatusBase']
print(info['ctor_candidates'])  # → list of (fn_name, write_offset)
```

This unblocks several previously-deferred investigations:
- **`StatusBase` ctor** (RTTI `0x012c31f8`) — to confirm what fields
  it initializes; needed to make sense of slot 3 of
  ChangeActorSubStatStatusReceiver's dispatch
- **`MyPlayer` ctor** — to confirm what's in the local-player
  specialization vs PlayerBase
- **`ItemBase` / `WidgetBase` / `GroupBase` ctors** — never
  documented, now in 1-step reach
- **`LuaTentativeControl` family ctors** — engine-internal classes
  not in `lua_class_registry.md`; worth understanding their role

## Method

`tools/sweep_class_metadata.py`:

```
For each RTTI Type Descriptor (from dynamic_cast_callsites.json):
  1. Search binary for 4-byte LE of TD's abs address
     → for each hit at offset N, candidate COL starts at N - 0xc
  2. Validate COL by checking signature dword (must be 0 at COL+0x0)
     and section (must be .rdata / .data)
  3. For each valid COL, search binary for 4-byte LE of COL's abs addr
     → for each hit at offset M, vtable starts at M + 4
  4. For each vtable, walk forward 4-byte slots while values point
     into .text — that's the slot count
  5. For each vtable, search binary for 4-byte LE of vtable's abs addr
     → filter to .text → these are ctor/dtor write sites
  6. Map each write site RVA to its containing function via
     asm/ffxivgame/<rva>_FUN_<...>.s filename pattern
  7. Group writes by containing function:
     - 1 write per fn  → "ctor_candidate" (further disambiguable by offset)
     - 2 writes per fn → "dtor_candidate" (complete-object dtor pattern)
     - 3+ writes      → "uncategorized" (rare; multi-vtable MI confusion)
```

Run time: ~3 seconds on a 12 MB binary with 94 000 asm files.

## Cross-references

- `docs/dynamic_cast_callsite_sweep.md` — Phase 9 ext (the source
  of the 129 RTTI addresses)
- `docs/lua_actor_class_construction.md` — Phase 9 #8d (manually
  walked 8 Lua-class ctors; results validated by this sweep)
- `docs/lua_class_registry.md` — Phase 6 #3 (every BaseClass entry's
  C++ vtable RVA can be cross-checked against this sweep's output)
- `docs/actor_5c_writer_decomp.md` — Phase 9 #8e (the +0x5c writer
  hunt; this sweep eliminates "the writer is in a Lua-class
  vtable" as a hypothesis)
- `memory/reference_meteor_decomp_actor_rtti.md` — the engine-side
  actor RTTI walk recipe (this sweep automates it)
- `build/class_metadata.json` — raw structured output (regenerable
  via `python3 tools/sweep_class_metadata.py`)
- `tools/sweep_class_metadata.py` — the extractor
