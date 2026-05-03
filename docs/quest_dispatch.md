# Phase 6 item #7 — Quest dispatch path

> Last updated: 2026-05-03 — QuestBase MI layout + ctor + factory
> + master Lua-class dispatcher all identified.

## QuestBase has multiple inheritance (two vtables)

`Application::Lua::Script::Client::Control::QuestBase` is a
**multi-inheritance** class with two distinct vtables:

| Vtable RVA | Slots | Role | Object offset |
|---|---:|---|---|
| `0xbdfddc` | 34 | Primary vtable (full slot map) | `+0x00` |
| `0xbdfdd0` | 1 | Secondary RTTI marker (dtor-only) | `+0x60` |

The 1-slot leaf at `+0x60` is for `dynamic_cast` lookups against
the secondary base class. When `delete` is invoked through the
secondary pointer, slot 0 of the secondary vtable thunks back:

```
FUN_00777d70 (8 B):
  83 e9 60       SUB ECX, 0x60      ; adjust this back to primary
  e9 18 f3 ff ff JMP FUN_00777090   ; jump to primary dtor
```

This is the standard MSVC adjustor-thunk pattern for MI dtors.

## QuestBase object layout

From decoding the constructor:

```
+0x00   primary vtable (= 0xfdfddc, 34 slots)
+0x04   parent fields (LuaControl + ActorBase + intermediate base)
...
+0x60   secondary vtable (= 0xfdfdd0, 1-slot RTTI marker)
+0x64   QuestBase-specific tail
```

Total `sizeof(QuestBase) = 0x64 (100 bytes)` — confirmed by the
factory's `operator new(0x64)` call.

The primary vtable at `+0x00` provides the 34 inherited Lua-
binding slots; the secondary vtable at `+0x60` is a thin RTTI
marker the engine uses to identify "this is a QuestBase" via
`dynamic_cast` in template-bound code (e.g. the
`StackOperator<QuestBase*>` Lua-bridge).

## Constructor — `FUN_00776f50` (109 B at file `0x376f50`)

```
6a ff             PUSH -1                ; SEH frame setup
68 43 47 ea 00    PUSH 0xea4743          ; SEH handler
64 a1 00 00 00 00 MOV EAX, FS:[0]
50 51 56 57       PUSH FS chain + EBP/ESI/EDI save
a1 b0 a8 2e 01    MOV EAX, [GS_COOKIE]
33 c4             XOR EAX, ESP
50                PUSH EAX               ; /GS cookie
8d 44 24 10       LEA EAX, [esp+0x10]
64 a3 00 00 00 00 MOV FS:[0], EAX        ; install SEH frame
8b f1             MOV ESI, ECX           ; ESI = this
89 74 24 0c       MOV [esp+0xc], ESI

e8 f2 4b f6 ff    CALL 0x2dbb70          ; → parent ctor (intermediate base)
c7 44 24 18 00 00 00 00  MOV [esp+0x18], 0   ; SEH state = 0

8d 7e 60          LEA EDI, [ESI+0x60]    ; embedded sub-object at +0x60
8b cf             MOV ECX, EDI
e8 60 5c 55 00    CALL 0x8ccbf0          ; sub-object init (secondary base ctor)

c6 44 24 18 01    MOV byte [esp+0x18], 1 ; SEH state = 1

c7 06 dc fd fd 00 MOV [ESI+0], 0xfdfddc  ; PRIMARY vtable
c7 07 d0 fd fd 00 MOV [EDI+0], 0xfdfdd0  ; SECONDARY vtable (at +0x60)

c7 44 24 18 ff ff ff ff  MOV [esp+0x18], -1
8b c6             MOV EAX, ESI           ; return this
8b 4c 24 10       MOV ECX, [esp+0x10]
64 89 0d 00 00 00 00     MOV FS:[0], ECX  ; restore SEH chain
59 5f 5e          POP / restore
83 c4 10          ADD ESP, 0x10
c3                RET
```

Standard MSVC ctor pattern: SEH-protected, calls parent ctor first
(at offset 41), then constructs the embedded `+0x60` sub-object
(at offset 59), then writes both vtables, then unwinds the SEH
state and returns `this`.

The parent ctor at `0x2dbb70` is the next-most-derived class
(probably an intermediate `ActorBase`-style base). The embedded
sub-object init at `0x8ccbf0` is likely an `LuaControl::Init`-style
hook for the secondary base.

## Factory — `FUN_00777030` (94 B, **slot 1** of QuestBase)

This is the **Lua-callable `New()` entry point** that `QuestBase:new()`
in Lua dispatches to:

```
6a ff [SEH setup]
6a 64                    PUSH 0x64        ; sizeof(QuestBase)
e8 dd aa 25 00           CALL 0x5d1b35    ; operator new
83 c4 04
89 44 24 04              MOV [esp+4], EAX ; save allocated ptr
c7 44 24 10 00 00 00 00  MOV [esp+0x10], 0
85 c0                    TEST EAX, EAX
74 09                    JZ skip          ; null check
8b c8                    MOV ECX, EAX
e8 de fe ff ff           CALL 0x376f50    ; → QuestBase ctor
eb 02
33 c0
c7 44 24 10 ff ff ff ff
[SEH unwind + RET]
```

Pattern: `operator new(0x64) → null-check → call ctor with this in
ECX → return new object`. The `0x64` confirms the QuestBase object
size; the `e8 de fe ff ff` resolves to `FUN_00776f50` (the ctor
above).

This function has **zero direct call sites** — it's invoked
exclusively through vtable dispatch (slot 1 of the primary
vtable). The Lua VM's class-instantiation glue is what calls it.

## Master Lua-class dispatcher — `FUN_0075f9b0` (7,481 B)

The single entry point that handles instantiation for **every**
`*BaseClass` Lua type is `FUN_0075f9b0` at file `0x35f9b0`. Its
body contains 79 PUSH-immediate string references covering all
known Lua class names:

```
ActorBaseClass, CharaBaseClass, PlayerBaseClass, NpcBaseClass,
WidgetBaseClass, DesktopWidgetBaseClass, DesktopWidget,
DesktopUtil, DirectorBaseClass, WorldBaseClass, AreaBaseClass,
PrivateAreaBaseClass, ZoneBaseClass, SystemBaseClass,
ProgDebugBaseClass, [+ ~49 more]
```

The strings appear in pairs: parent-class name, then child-class
name. This is the **inheritance-resolution dispatcher** — given
a Lua class request, it walks the class hierarchy by name and
dispatches to the right factory (slot 1 of the matched vtable)
to instantiate the object.

This function is the runtime counterpart to the **registration**
function `FUN_0078e3a0` documented in
`docs/lua_class_registry.md`. The registration function builds
the (name → vtable) registry at startup; the dispatcher uses it
to instantiate at runtime.

## Quest-specific Lua-callable hooks

Compared against the `AreaBase` sibling (which has the same 35-
slot footprint), QuestBase overrides 8 slots:

| Slot | QuestBase | AreaBase | Role |
|---:|---|---|---|
| 0 | `FUN_00777d70` | `FUN_0073e720` | Destructor (MI thunk) |
| 1 | `FUN_00777030` | `FUN_0073e6b0` | **Factory `New()`** |
| 2 | `FUN_00776340` (no-op) | `FUN_00753cf0` | (Quest does not need slot 2's per-area init) |
| 3 | `FUN_00712b40` (no-op) | `FUN_006e1670` | (Quest does not need slot 3's hook) |
| 6 | `FUN_006dbe80` | `FUN_006dc000` | Lifecycle hook (probably `_onInit` or `_onActivate`) |
| 26 | `FUN_006dbe90` | `FUN_006dbff0` | Lua-callable hook |
| 28 | `FUN_006dcfd0` | `FUN_00a72a20` (no-op) | **Quest-specific Lua hook** (likely `getQuestData` or similar) |
| 29 | `FUN_006dbea0` | `FUN_006e1640` | Lua-callable hook |

The fact that QuestBase has a non-no-op at slot 28 while AreaBase
doesn't suggests slot 28 is a **Quest-only Lua method** — most
likely `getQuestData()` or `getQuestState()` based on Discord
context (Ioncannon's `quest:GetData()` references in
`project_meteor_discord_context.md`).

Slot 28's body is a tiny thunk:

```
FUN_006dcfd0 (17 B):
  8b 44 24 08    MOV EAX, [esp+8]    ; arg 2
  8b 4c 24 04    MOV ECX, [esp+4]    ; arg 1
  50             PUSH EAX
  e8 92 a1 5e 00 CALL 0x8c7170       ; → impl
  c2 0c 00       RET 12
```

The impl at `0x8c7170` is itself a virtual-dispatch trampoline:

```
FUN_00cc7170 (7 B):
  8b 09          MOV ECX, [ECX]      ; load vtable
  e9 69 3f 01 00 JMP +0x13f69        ; tail-jump
```

So slot 28 is "load the embedded handler's vtable, dispatch via
its method N." The actual quest-data getter logic lives in some
other class that QuestBase references — a follow-up Ghidra walk
through the JMP target would identify it.

## How a quest gets loaded (end-to-end flow)

```
1. Server tells client "load quest man0g0"
2. Client invokes the master Lua dispatcher (FUN_0075f9b0)
   with the class name "QuestBaseClass" (or a Lua-defined
   subclass name like "Man0g0Quest").
3. Dispatcher matches the class name in the registry built by
   FUN_0078e3a0, finds the QuestBase vtable.
4. Dispatcher calls slot 1 (FUN_00777030) — the factory.
5. Factory does operator new(0x64), then CALL ctor (FUN_00776f50).
6. Ctor calls parent ctor → embedded sub-object init → writes
   primary + secondary vtables.
7. The Lua engine attaches the .prog script file (e.g.
   /Quest/man/man_0_0.prog) to the new QuestBase instance,
   binding Lua methods to the C++ vtable slots.
8. Lua-side _onInit fires.
9. Quest is now active; engine routes incoming events
   (onTalk, onPush, onUpdate, onNotice) to the matching slot.
```

## Practical impact for garlemald

Garlemald already drives this entire flow:

- The server sends a quest-add packet that names the quest
  (e.g. `quest_id = 0xa0f05e93` for man0g0).
- The client's master Lua dispatcher resolves the class
  hierarchy and instantiates the QuestBase.
- The Lua script attached to the quest (man0g0.lua, served
  by garlemald) is loaded via `LpbLoader` and bound to the
  instance.
- Subsequent events (talk, push, update) hit the right vtable
  slots which dispatch into the Lua script's methods.

The decomp confirms several invariants garlemald already
respects:

1. **Quest object size is fixed at 100 bytes (0x64)** — garlemald
   doesn't directly allocate this (it's client-side), but any
   wire payload that names a quest must use the (id → name)
   mapping consistent with the registered class names.
2. **Multiple inheritance via two vtables** — irrelevant on the
   wire layer (server doesn't see the C++ vtables) but explains
   why the client's `dynamic_cast<QuestBase*>` checks succeed
   from either branch of the hierarchy.
3. **Slot 1 is the universal `New()` entry point** — any
   `*BaseClass` Lua type can be instantiated via slot 1 of
   its registered vtable. Garlemald's quest-spawn packets work
   because the engine routes them through this entry.
4. **Slot 28 is the Quest-specific Lua hook** — likely
   `getQuestData()`. If garlemald's quest-script Lua uses
   `quest:GetData()` (the canonical 1.x spelling per Discord),
   it dispatches through slot 28 → `FUN_006dcfd0` → embedded
   handler. The data the Lua sees comes from a server-side
   table garlemald already maintains.
5. **The 0xa0f05e93 quest-id (and similar)** is a wire ID that
   garlemald already sends correctly per the man0g0 work in
   `project_garlemald_man0g0_seq000_complete.md`.

## Phase 6 work pool — item #7 status

This closes Phase 6 item #7. Remaining items:

- #5 `.lpb` / `.prog` bytecode format
- #6 `LuaActorImpl` 90-slot map
- #8 `DirectorBase` slots 20..33 Lua hooks (apply the same
  diff-vs-sibling approach used here)
- #9 Functional `OpeningDirector` validation against
  garlemald's `man0g0.lua`

## Cross-references

- `docs/director_quest.md` — Phase 6 architecture (the C++
  Lua-binding base classes that QuestBase inherits)
- `docs/lua_class_registry.md` — Phase 6 item #3 (the
  registration function FUN_0078e3a0 that builds the registry
  this dispatcher consumes)
- `docs/sync_writer.md` — Phase 6 item #4 (the typed Work-field
  serializer that quest scripts use to push state to the wire)
- `project_garlemald_man0g0_seq000_complete.md` (memory) —
  garlemald's man0g0 implementation that this dispatcher path
  drives
- `project_garlemald_journal_qtdata_fix.md` (memory) —
  garlemald's `commandRequest` / `qtdata` reply that quest
  scripts use to expose data via slot 28 (the `getQuestData`
  hook inferred here)
- `project_meteor_discord_context.md` — Ioncannon notes on
  `quest:GetData()`, `processEvent`, `Seq000`
