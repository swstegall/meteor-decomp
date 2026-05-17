# Phase 9 #4 — UserDataReceiver decomp

> Recovered 2026-05-16. Closes the second of the two 6-slot Receivers
> in the Phase 9 inventory work pool (the first, JobQuestCompleteTriple,
> closed in `docs/event_job_quest_complete_triple_decomp.md`).
> Together these close all 4 Pattern-B-or-larger Receivers; only the
> standard 2-slot Pattern-A's remain to optionally walk.

## TL;DR — UserDataReceiver is Pattern A (NOT Pattern C)

Despite both being 6-slot, **UserDataReceiver and JobQuestCompleteTriple
use different dispatch patterns**:

- **JobQuestCompleteTriple** = Pattern C: real work happens in slot 5
  via the 2-step success-gated `FUN_00785bf0` dispatcher; the standard
  Receive slot (slot 2) is a constant success-byte stub.

- **UserDataReceiver** = Pattern A: the **6-slot vtable is MI-thunk
  scaffolding** for the SECONDARY base; the real Receive lives at
  slot 1 of a **second 2-slot vtable** (the PRIMARY base, at `this+0`).
  LuaActorImpl::slot59 stack-builds the receiver and directly calls
  the primary's slot 1 — standard Pattern A.

This refines the Pattern-A/B/C taxonomy from `docs/receiver_dispatch_via_actorimpl.md`:
**vtable size alone doesn't determine pattern** — the dispatch path
through LuaActorImpl matters more.

## Two vtables via multiple inheritance

UserDataReceiver has **two RTTI-named vtables**, both COL→TD-confirmed
as `Application::Lua::Script::Client::Command::Network::UserDataReceiver`:

| Vtable | RVA | Slots | Role |
|---|---|---:|---|
| Primary | `0xc574a4` | 2 | At `this+0`. Contains the actual functions. |
| Secondary | `0xc57488` | 6 | At `this+8`. Contains MI thunks back to primary + a few sub-interface methods. |

The dtor confirms the layout (`FUN_0089e760` at offset +0x2a..+0x36):

```asm
LEA EDI, [ESI+8]                  ; EDI = this+8
MOV [ESI], 0x010574a4             ; write primary vtable at this+0
MOV [EDI], 0x01057488             ; write secondary vtable at this+8
```

So `this+0` holds the primary subobject (2-slot iface), `this+8` holds
the secondary subobject (6-slot iface).

### Primary vtable (`0xc574a4`, 2 slots) — the real interface

| Slot | RVA | Size | Role |
|---:|:---|---:|---|
| 0 | `0x4a1660` | ~32 B | **Scalar deleting dtor** — calls inner dtor `FUN_0089e760`, optionally `delete this` per heap flag |
| 1 | `0x4a0190` | **592 B** | **Receive** — the actual UserData handler (big body) |

### Secondary vtable (`0xc57488`, 6 slots) — MI thunks

| Slot | RVA | Size | Role |
|---:|:---|---:|---|
| 0 | `0x4a15e0` | 8 B | **MI thunk dtor** — `SUB ECX, 8; JMP <primary dtor>` (back to this+0 view) |
| 1 | `0x4a2a20` | ~16 B | **MI thunk for slot 5** — `CALL slot 5; if non-null ADD EAX, 8` (return secondary subobject ptr) |
| 2 | `0x49e080` | 13 B | **Sync "success" byte** — `*out = [0x012c41af]`. Same global as KickReceiver's "default success byte" + JobQuestComplete's gate 1. |
| 3 | `0x49e090` | ~48 B | **Actor lookup + virtual call** — calls `FUN_00cc7a50` (`ActorRegistry::lookup_actor`, Phase 7 known); if found calls `[ECX-8]->vtable[1]` (back to primary's slot 1 — the big Receive!) |
| 4 | `0x49e0c0` | 14 B | **`m_field_18 != NO_ACTOR` predicate** — same 14-byte shape as Phase 7 EndEvent slot 4 |
| 5 | `0x49f070` | ~192 B | **36-byte sub-object factory** — `operator new(0x24)`, computes `base = this[+0x10] - this[+0xc]`, calls `FUN_0089efa0`. Distinct from the receiver's own ctor. |

The secondary's slot 3 is interesting — it routes back to the primary
via `[ECX-8]->vtable[1]`, which IS the primary's slot 1 (the 592-byte
Receive). So even external callers using the secondary interface
ultimately reach the same handler.

## The LuaActorImpl::slot59 wrapper — Pattern A

Per `docs/receiver_dispatch_via_actorimpl.md`, UserDataReceiver
dispatches from **LuaActorImpl::slot59 (FUN_00759e50, 118 B)**:

```c
void LuaActorImpl::slot59(void *arg) {  // ECX=this, [ESP+3c]=arg
    char buf[0xc0];     // 192-byte stack receiver
    
    /* Construct receiver on stack */
    PUSH 0xc0                              ; size = 192
    PUSH arg                               ; arg0
    LEA ECX, [ESP+0x10]                    ; ECX = &stack_receiver
    CALL FUN_0089eed0                      ; UserDataReceiver::ctor(this, arg, size)
    
    /* Directly call primary's slot 1 (Receive) */
    MOV ECX, [ESI+8]                       ; ECX = this->m_field_8 (some context)
    PUSH ECX
    ADD ESI, 4                             ; ESI = this+4
    PUSH ESI
    LEA ECX, [ESP+0x10]                    ; ECX = &stack_receiver (primary at +0)
    CALL FUN_008a0190                      ; ⭐ PRIMARY VT SLOT 1 — the 592-byte Receive
    
    /* Tear down */
    LEA ECX, [ESP+8]
    CALL FUN_0089e760                      ; inner dtor (not scalar-deleting)
}
```

So the dispatch is **classic Pattern A** — ctor → direct Receive call
→ dtor, with no intermediate gating helper. The "primary slot 1" call
target (`FUN_008a0190` at `0x4a0190`) is exactly what the primary
vtable's slot 1 entry points at.

The 6-slot main vtable's existence has NO effect on the dispatch path
from LuaActorImpl::slot59 — those 6 slots exist for external callers
who acquired a UserDataReceiver pointer via some MI-aware interface
(probably the "receiver" abstract base that `Command::Network::*`
classes participate in via secondary inheritance).

## FUN_008a0190 — the 592-byte Receive

Too large to fully decode in this pass, but the call-target footprint
gives a clear semantic picture:

| Helper | Phase 4/7/8 known role |
|---|---|
| `FUN_009d22b4` (×3) | String compare helper (Phase 4 sqpack) |
| `FUN_0044726?` / `FUN_0004726?` | Sqex Utf8String ctor (Phase 4) |
| `FUN_00046f5?` | Sqex Utf8String dtor (Phase 4) |
| `FUN_0078f810` / `0078fa60` / `0078fab0` / `0078fac0` | Lua-engine helpers (same neighborhood as `FUN_00785bf0` from JobQuestComplete Pattern C) |
| `FUN_00cc7ea0` / `FUN_00cc7a90` | Engine root / ActorRegistry navigation (Phase 7 cluster) |
| `FUN_00584e10` / `FUN_00584540` | Earlier Lua helpers |
| `FUN_0004d350` | `operator delete` (Phase 4) |
| `FUN_00790a30` | Lua engine helper |

So Receive: navigates the engine context, builds/looks-up Utf8String
keys, talks to ActorRegistry, dispatches per-key into Lua-engine
helpers. Consistent with **"user data" = persistent player KV blob
sync**, probably how the server pushes a player's saved preferences,
options, or per-character state at login.

**Notably absent**: no call to `FUN_00785bf0` (the 2-step gated
dispatcher). UserDataReceiver does NOT share Pattern C with
JobQuestCompleteTriple — they happen to both be 6-slot but use
different dispatch architectures.

## FUN_0089eed0 — the ctor (201 B)

Standard MSVC SEH-protected ctor:
1. Write parent class vtable at `[this+0]` = `0x00fdf980` (a parent — likely `CommandUpdaterBase` or `CommandReceiverBase`)
2. ESI = `this+8` (secondary subobject ptr)
3. Call parent ctor at `0x007942b0` with `ECX = this+8`
4. (Later writes the correct primary + secondary vtables — done by the dtor visibly; ctor probably does it too further in)
5. Initialize Utf8String members via `FUN_009d22b4` + `FUN_005d4600`
6. Initialize a 36-byte child object (size matches the slot-5 factory)

The size arg `0xc0` (192 bytes) from LuaActorImpl::slot59 is plausibly
the total UserDataReceiver instance size including its embedded
Utf8String + child sub-object.

## FUN_0089e760 — the inner dtor (159 B)

```c
void UserDataReceiver::~UserDataReceiver(this) {  // not scalar-deleting
    /* Restore vtables (post-dtor convention) */
    [this+0] = 0x010574a4;        // primary vt
    [this+8] = 0x01057488;        // secondary vt
    
    /* Tear down sub-objects */
    FUN_008c9330(...);             // some helper teardown
    operator delete(...);          // FUN_0004d350
    
    /* Parent dtor */
    FUN_007942c0(this);            // parent dtor (CommandUpdaterBase::~CommandUpdaterBase)
}
```

The vtable-restore-then-parent-dtor sequence is classic MSVC, ensuring
parent-dtor sees the parent's vtable.

## Refined Pattern taxonomy

This decode refines the Pattern A/B/C taxonomy from
`docs/receiver_dispatch_via_actorimpl.md`:

| Pattern | Examples | Recognition |
|---|---|---|
| **A** (stack temporary) | 24+ 2-slot receivers (SetEventStatus, ChangeSystemStat, …) + **UserDataReceiver** (despite 6-slot main) | LuaActorImpl::slotN calls `Receive` directly after ctor; no intermediate dispatcher |
| **B** (heap allocated, event lifecycle) | 5-slot Kick/Start/End events | LuaActorImpl::slotN allocates via `New` factory; `Receive` is invoked LATER via the receiver's own vtable as part of the event lifecycle |
| **C** (stack temporary + 2-step success-gated dispatch) | JobQuestCompleteTriple | LuaActorImpl::slotN calls `FUN_00785bf0` between ctor and dtor; the receiver's "official" Receive slot is a constant-success stub; real work is reached through slot 3→slot 5 |

**vtable size does NOT determine pattern.** Both 6-slot Receivers use
DIFFERENT patterns. The recognition signal is **what LuaActorImpl::
slotN calls between the ctor and dtor**: direct Receive (A), New
factory (B), or `FUN_00785bf0` (C).

## What "UserData" probably is

Naming + dispatch shape + helpers suggest:

- **Server-side**: persistent per-character KV state — saved menu
  preferences, hotbar layouts, custom UI configurations, tutorial
  progress flags, "don't show again" dialogs, last-viewed-zone
  bookmarks, etc.
- **Wire**: probably a single opcode that emits `(key: u16, value:
  blob)` pairs — multiple pairs per packet (the 592-byte Receive
  needs to handle a non-trivial payload).
- **Client**: writes into a per-character KV table accessed by Lua UI
  scripts via something like `mainPlayer:getUserData("hotbar.slot1")`.

This would be the canonical mechanism for client-side state that the
server needs to persist across sessions but doesn't otherwise care
about (game logic doesn't depend on hotbar layouts). The 36-byte
sub-object factory (secondary slot 5) probably allocates the
per-entry storage struct.

## Cross-references

- `docs/receiver_classes_inventory.md` — Phase 9 #1 (the 43 Receivers
  + UserDataReceiver listed with BOTH vtables `0xc57488` (6-slot) and
  `0xc574a4` (2-slot) — the inventory noted this MI but didn't
  decode it)
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 (LuaActorImpl
  slot 59 = UserDataReceiver dispatch, Pattern recognition heuristic)
- `docs/event_job_quest_complete_triple_decomp.md` — Phase 9 #3 (the
  other 6-slot Receiver, introduces Pattern C — distinct from
  UserDataReceiver's Pattern A despite same slot count)
- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (source of the
  `[0x012c41af]` "default success byte" referenced in slot 2 stub +
  the `[0x0130c778]` NO_ACTOR sentinel referenced in slot 4 predicate)
- `docs/event_end_receiver_decomp.md` — Phase 7 #3 (source of the
  14-byte `m_field_18 != NO_ACTOR` predicate shape — slot 4 here)
- `asm/ffxivgame/0049eed0_FUN_0089eed0.s` — ctor (201 B)
- `asm/ffxivgame/0049e760_FUN_0089e760.s` — inner dtor (159 B)
- `asm/ffxivgame/004a0190_FUN_008a0190.s` — primary slot 1, the
  592-byte Receive (main handler — partial decode here; full decomp
  would yield the wire format for the UserData blob)
