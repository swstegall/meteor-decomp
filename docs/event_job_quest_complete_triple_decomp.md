# Phase 9 #3 — JobQuestCompleteTripleReceiver decomp

> Recovered 2026-05-16. Closes one of the two remaining 6-slot
> Receivers from the Phase 9 inventory work pool (the other is
> UserDataReceiver, also 6-slot; Phase 9 #4 still pending).

## TL;DR

**`JobQuestCompleteTripleReceiver`** is a 6-slot Receiver wired into
`Application::Lua::Script::Client::Command::Network::*` (vtable RVA
`0xc57360`). It introduces a **third dispatch pattern** distinct from
Pattern A (stack-temporary 2-slot) and Pattern B (heap-allocated
5-slot) documented in `docs/receiver_dispatch_via_actorimpl.md`:

**Pattern C — stack-built receiver dispatched through a 2-step
success-gated helper.** LuaActorImpl::slot78 stack-builds the
receiver, then calls `FUN_00785bf0` (a 238-byte central dispatcher)
that gates twice on different "success byte" globals before invoking
the receiver's slot-5 real handler. Slot 5 (`FUN_0089e520`) navigates
into `MyPlayer[+0x110]` (a 36-byte "current job-quest completion
triple" struct), allocates a new triple, swaps it in, and frees the
old one.

## The 6-slot vtable

Vtable @ RVA `0xc57360`:

| Slot | RVA | Size | Role | Pattern |
|---:|:---|---:|---|---|
| 0 | `0x4a1120` | 96 B | **Scalar deleting dtor** — writes vtable, calls parent dtor at `0x7942c0`, optionally `delete this` per heap flag | std MSVC dtor |
| 1 | `0x49d350` | 112 B | **`New()` factory** — `operator new(0x20)` → `ctor(this, args)` at `0x49d270` | std MSVC New |
| 2 | `0x49d3c0` | 13 B | **Sync receive — return success byte** — writes `*out = [0x012c41af]` (the same global success byte used by KickReceiver), `RET 8` | tiny stub |
| 3 | `0x49d3d0` | 17 B | **Trampoline to slot 5** — `MOV EAX, [ECX]; CALL [EAX+0x14]` (vtable[5]) | virtual chain |
| 4 | `0x49d3f0` | 3 B  | **`return true` predicate** — `MOV AL, 1; RET` | const predicate |
| 5 | `0x49e520` | 48 B | **Real handler** — calls `FUN_002f9520` (152 B), the JobQuestObject swapper | dispatch leaf |

Plus 3 **non-virtual member functions** sharing the vtable's RTTI
neighborhood (not in any vtable slot, called by name from
`FUN_00785bf0` and the LuaActorImpl wrapper):

| RVA | Size | Role |
|:---|---:|---|
| `0x49d270` | ~0xe0 B | **Ctor** — SEH prolog, calls parent ctor at `0x7942b0` (CommandUpdaterBase), writes vtable `0x1057360`, copies arg into `this+8` |
| `0x49d2f0` | 72 B | **Stack-temp dtor** — writes vtable defensively, chains to parent dtor at `0x7942c0`. NOT the scalar-deleting variant (no `delete this`). Used by the LuaActorImpl wrapper after the dispatch returns. |
| `0x49d340` | 15 B | **3-arg "Receive" variant** — same body as slot 2 (`*out = [0x012c41af]; RET`) but `RET 0xc` (cleans 3 args instead of 2). Used by `FUN_00785bf0` as the first success-gate check. |

The presence of both a 2-arg (slot 2) AND a 3-arg (FUN_0089d340)
"return success byte" function — both bodies identical except for
arg-cleanup count — suggests JobQuestCompleteTriple's interface
exposes the result-byte query in two calling conventions. Slot 2 is
for the standard Receiver interface; FUN_0089d340 is for
`FUN_00785bf0`'s dispatcher convention.

## The LuaActorImpl::slot78 wrapper

Per `docs/receiver_dispatch_via_actorimpl.md` (Phase 9 #5 partial),
JobQuestCompleteTripleReceiver dispatches from **LuaActorImpl::slot78
(FUN_0076c690, 143 B)**:

```c
void LuaActorImpl::slot78(SomeArg *arg) {  // ECX=this, [EBP+8]=arg
    char buf[?];
    
    /* Build receiver on stack */
    PUSH arg
    LEA ECX, [ESP+0x10]                       ; ECX = &stack_receiver
    CALL 0x0089d270                           ; JobQuestCompleteTripleReceiver::ctor
    
    /* Dispatch through the 2-step gated dispatcher */
    LEA EAX, [ESP+0xc]                        ; out buffer
    PUSH EAX
    PUSH arg
    ADD EDI, 4                                ; EDI = this+4 (advance into actor state)
    PUSH EDI
    CALL 0x00785bf0                           ; ⭐ 2-step success-gated dispatcher
    
    /* Tear down */
    LEA ECX, [ESP+0xc]
    CALL 0x0089d2f0                           ; stack-temp dtor (not scalar-deleting)
}
```

Notably, this wrapper **does NOT call the standard Receive method
(slot 1 of 2-slot Receivers; slot 2 of 5/6-slot)** in the normal way
— it routes through `FUN_00785bf0` instead. That makes JobQuestComplete
the first decoded Receiver where the dispatch path doesn't pass
through the "official" Receive slot.

## FUN_00785bf0 — the 2-step success-gated dispatcher

`FUN_00785bf0` (238 B, RVA `0x00385bf0`) is a generic dispatcher that
gates twice before doing real work:

```c
void FUN_00785bf0(LuaSomething *arg0, ReceiverInstance *receiver, BytePtr flag) {
    /* ECX=arg0, [ESP+34]=arg0, [ESP+38]=flag, [ESP+3c]=receiver */
    
    char gate1_byte, gate2_byte;
    bool gate1_passed = false;
    
    /* Navigate from arg0 to engine context root, then to per-LuaEngine registry */
    engine_root = navigate_engine_root(arg0);  // CALL 0x00cc7510 (same trampoline as KickReceiver Phase 7)
    registry = engine_root[0][+4][+0xf4];      // load registry pointer
    
    /* Skip dispatch if registry is empty */
    if (registry[+0x10] != NULL) goto skip;
    
    /* GATE 1: call receiver's 3-arg "Receive" via FUN_0089d340 */
    receiver->vtable[?](&gate1_byte, arg0, flag);  // CALL 0x0089d340
    if (gate1_byte != *GLOBAL_SUCCESS_BYTE_1)      // CMP AL, [0x012c41af]
        goto skip;
    gate1_passed = true;
    
skip:
    if (!gate1_passed) goto cleanup;
    
    /* GATE 2: call receiver->vtable[?] */
    receiver->vtable[?](&gate2_byte, arg0);        // CALL 0x00794250
    if (gate2_byte != *GLOBAL_SUCCESS_BYTE_2)      // CMP DL, [0x012c3120]
        goto cleanup;
    
    /* Real work — both gates passed */
    EAX = receiver->vtable[?](arg0);               // CALL 0x00794240
    registry->vtable[?](EAX, ...);                  // CALL 0x00785570
    
cleanup:
    ...
}
```

Two global "success byte" sentinels are involved:

| Byte | Where | Role |
|---|---|---|
| `[0x012c41af]` | Phase 7 KickReceiver doc identified this as the **default kick result byte** | Gate 1 — "did the first-pass check succeed?" |
| `[0x012c3120]` | New — not yet documented elsewhere | Gate 2 — "did the second-pass check succeed?" |

The 2-step gate is consistent with a JobQuestComplete update needing
to: (1) verify the player is in a valid state to receive a quest-
complete update (gate 1 — e.g. "is the player even loaded?"), (2)
verify the update applies to this player's current job (gate 2 — e.g.
"is the player currently the job whose quest just completed?"), then
do the actual triple swap.

## The real handler — slot 5 → FUN_002f9520

Slot 5 (`FUN_0089e520`, 48 B) is a thin wrapper:

```c
void slot5(this, arg0) {
    /* Navigate the LuaEngine context root */
    EAX = navigate_engine_root(arg0);     // FUN_00cc7510
    ECX = EAX[0][+4][+0xc];               // load the JobQuest manager
    
    /* Hand off to the actual processor */
    FUN_002f9520(this+8, arg0);           // ⭐ real work
}
```

**`FUN_002f9520` (152 B, RVA `0x002f9520`)** is the actual processor.
Body summary:

```c
void FUN_002f9520(receiver_state *state, void *arg0) {
    /* Allocate a 36-byte JobQuestObject */
    JobQuestObject *new_obj = operator new(0x24);      // PUSH 0x24; CALL 0x5d1b35
    
    if (new_obj != NULL) {
        /* Initialize the new triple */
        new_obj_init(new_obj, state[+0x24], state[+0x28]);   // CALL 0x2dda90
    } else {
        new_obj = NULL;
    }
    
    /* Read the existing triple (stored at the LuaEngine's m_field_110) */
    JobQuestObject *old_obj = engine[+0x110];  // (offset relative to bound base)
    
    if (new_obj == old_obj) return;
    
    if (old_obj != NULL) {
        /* Deinit + free the old one */
        old_obj->deinit();                  // CALL 0x2f7cd0
        operator delete(old_obj);           // CALL 0x5d1b17
    }
    
    /* Install the new triple */
    engine[+0x110] = new_obj;
    
    return;
}
```

So the per-update lifecycle:
1. Server emits a packet that resolves to `JobQuestCompleteTripleReceiver`
2. LuaActorImpl::slot78 stack-builds the receiver
3. `FUN_00785bf0` gates twice, then invokes slot 5 indirectly through
   slot 3's vtable trampoline → slot 5 → `FUN_002f9520`
4. `FUN_002f9520` allocates a fresh 36-byte JobQuestObject, swaps it
   into `engine[+0x110]` (replacing any existing one), and deletes
   the old object's memory
5. LuaActorImpl::slot78 destructs the stack receiver and returns

## What is the 36-byte JobQuestObject?

The name "JobQuestCompleteTriple" + 36 bytes = strongly suggests
**3 × 12-byte entries**, each entry being one completed job quest
(e.g. (jobId: u32, questId: u32, completionFlags: u32) — 12 bytes
each, 36 total).

Cross-reference: in FFXIV 1.x, each Discipline of War/Magic job has
a "Job Quest" line — the Triple naming probably refers to the
"current + previous + 2-back" or "3 most recent" completed quests
displayed somewhere in the UI (e.g. on the character page or
nameplate). The "3 most recent" pattern is common in FFXIV's UI.

The deeper structural confirmation comes from the binding being on
`MyPlayer` (local-player only) — JobQuest history is per-player, not
broadcast to other actors.

## Architectural insight — Receiver dispatch pattern C

This decomp introduces a third dispatch pattern alongside A and B
from `docs/receiver_dispatch_via_actorimpl.md`:

| Pattern | Examples | Receiver lifetime | Dispatch path |
|---|---|---|---|
| A (2-slot) | SetEventStatus, ChangeSystemStat, … (24+) | Stack temporary | `LuaActorImpl::slotN` → ctor → `Receive` (slot 1) → dtor |
| B (5-slot) | Kick, Start, End event lifecycle | Heap allocated | `LuaActorImpl::slot56-58` → `new` → slot 1 (factory) keeps it; slot 2 (`Receive`) invoked later by event lifecycle |
| **C (6-slot) JobQuestCompleteTriple** | This receiver | **Stack temporary** | `LuaActorImpl::slot78` → ctor → **`FUN_00785bf0` (2-step gate)** → conditional slot 5 (via slot 3 trampoline) → dtor |

Pattern C is structurally a hybrid: it allocates like Pattern A but
dispatches through an additional success-gated helper that knows how
to talk to the LuaEngine's per-receiver registry. The presence of two
distinct global success-byte sentinels (`0x012c41af` + `0x012c3120`)
plus the gated invocation suggests JobQuestComplete (and likely
UserDataReceiver, the other 6-slot) are **player-state-update
receivers that need to verify the update applies before processing**
— hence the 2-step verification.

UserDataReceiver (Phase 9 #4) is the natural next target — likely the
same Pattern C since it's also 6-slot. The decoded LuaActorImpl::
slot 59 dispatch would let us confirm.

## Cross-references

- `docs/receiver_classes_inventory.md` — Phase 9 #1 (the 43 Receivers
  + their vtable RVAs; JobQuestCompleteTriple is listed as
  `0xc57360`, 6 slots)
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 (the
  LuaActorImpl/NullActorImpl 90-slot dispatch model; this Receiver
  occupies slot 78)
- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (KickReceiver
  decomp — source of the `[0x012c41af]` "default success byte"
  identification; same global is used as Gate 1 in `FUN_00785bf0`)
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (the structural
  pattern this Receiver fits into — dispatch through a sibling class,
  not through a per-opcode vtable)
- `asm/ffxivgame/0049d270_FUN_0089d270.s` — ctor
- `asm/ffxivgame/0049d2f0_FUN_0089d2f0.s` — stack-temp dtor
- `asm/ffxivgame/0049d340_FUN_0089d340.s` — 3-arg success-byte variant
- `asm/ffxivgame/00385bf0_FUN_00785bf0.s` — the 2-step success-gated
  dispatcher (probably shared by other receivers — investigate
  next when decoding UserDataReceiver)
- `asm/ffxivgame/002f9520_FUN_006f9520.s` — `FUN_002f9520`, the
  JobQuestObject swapper at `MyPlayer[+0x110]`
