# Phase 9 #5 — Receiver dispatch via LuaActorImpl / NullActorImpl slots

> Last updated: 2026-05-16. Partial close on Phase 9 work-pool item #5
> ("cross-reference each receiver to its opcode — the engine wires
> opcode → receiver at script load; need to find that registration").

## TL;DR

**35 of 42 Receiver classes** dispatch via specific vtable slots on
`Component::Lua::GameEngine::{LuaActorImpl, NullActorImpl}` — two
parallel 90-slot vtables. The receiver itself has zero direct
CALL-rel32 callers; its `Receive` method is invoked from inside a
LuaActorImpl slot.

| Vtable | RVA | Slots | Class |
|---|---|---:|---|
| `LuaActorImpl` | `0xbdfb2c` | 90 | Production actor-impl (Lua-bound, used by all spawned actors) |
| `NullActorImpl` | `0xbe02ac` | 90 | Sentinel / no-op impl (used for unloaded actor slots) |

Both classes implement the same interface — slot N in LuaActorImpl
and slot N in NullActorImpl are paired (same Receiver dispatched in
both, but NullActorImpl's body usually no-ops).

## Why this matters

Phase 8 #9 (`docs/network_dispatch_dual_paths.md`) established that
"event-bound" opcodes (Kick, RunEventFunction, EndEvent,
SetEventStatus, etc.) flow through `Application::Lua::Script::
Client::Command::{Network,System}::*Receiver` classes — but did NOT
identify how the network layer reaches those Receivers. The bit
about "Lua engine wires them up at script-load time — that's what
creates the *appearance* of 'runtime registration' but it's actually
compile-time RTTI'd, just under a different namespace than the
network channel" was a hypothesis without a recovered binding.

This phase recovers the binding:

```
[wire opcode 0x012F arrives]
         ↓
[opcode dispatcher reads packet header, picks actor]
         ↓
[actor.lua_impl→vtable[56](payload)]    ← LuaActorImpl::slot56
         ↓
[stack-build / heap-allocate a KickClientOrderEventReceiver]
[invoke Receiver::Receive(payload) → vtable slot 2]
[destruct Receiver]
```

The opcode → slot mapping is **fixed at compile time** (encoded
into the LuaActorImpl vtable layout); only the actor target is
runtime-resolved.

## Two dispatch patterns

Receivers split into two physical-allocation patterns based on
their vtable size:

### Pattern A — stack temporary (2-slot Receivers, ~24 of 43)

LuaActorImpl::slotN body stack-builds the Receiver, invokes
`Receive`, then destructs it in-place:

```c++
void LuaActorImpl::slot48(SetEventStatusPacket* pkt) {
    char buf[?];
    /* unpack pkt fields */
    SetEventStatusReceiver::ctor(&buf, args);   // FUN_0089d770
    SetEventStatusReceiver::Receive(&buf);      // FUN_0089d860
    SetEventStatusReceiver::dtor(&buf);         // FUN_0089d800
}
```

This is visible by direct CALL rel32 from the LuaActorImpl slot to
the Receiver's `Receive` method.

### Pattern B — heap-allocated, long-lived (5/6-slot Receivers, ~6 of 43)

For Receivers participating in the event lifecycle (Kick /
StartServerOrderEvent / EndClientOrderEvent / JobQuestCompleteTriple
/ UserData / ChangeActorSubStatStatus), the LuaActorImpl slot
heap-allocates the Receiver and keeps it alive across multiple
packets. `Receive` itself is invoked through the Receiver's own
vtable (slot 2), so the slot has no direct CALL to `Receive` — only
to the Receiver's ctor. The slot mapping is recovered by tracing
**ctor write callsites** instead.

## Recovered slot map (LuaActorImpl)

`build/wire/ffxivgame.receiver_actorimpl_map.md` is the regenerable
authoritative version. Snapshot at 2026-05-16:

| Slot | Slot fn | Receiver | Pattern |
|---:|:---|:---|:---|
| 7   | `FUN_00759630` | ExecuteDebugCommandReceiver         | A |
| 19  | `FUN_00759720` | ExecutePushOnEnterTriggerBoxReceiver  | A |
| 20  | `FUN_007597a0` | ExecutePushOnLeaveTriggerBoxReceiver  | A |
| 21  | `FUN_00759820` | AttributeTypeEventEnterReceiver     | A |
| 22  | `FUN_007598a0` | AttributeTypeEventLeaveReceiver     | A |
| 48  | `FUN_00759d20` | SetEventStatusReceiver              | A |
| 56  | `FUN_0076c0d0` | **KickClientOrderEventReceiver**    | B |
| 57  | `FUN_0076c220` | **StartServerOrderEventFunctionReceiver** | B |
| 58  | `FUN_0076c3b0` | **EndClientOrderEventReceiver**     | B |
| 59  | `FUN_00759e50` | UserDataReceiver                    | B |
| 60  | `FUN_00759ed0` | SyncMemoryReceiver                  | A |
| 61  | `FUN_00759f50` | SetTargetTimeReceiver               | A |
| 62  | `FUN_0076c4d0` | SetDisplayNameReceiver              | A |
| 63  | `FUN_00759fd0` | SendLogReceiver                     | A |
| 64  | `FUN_0075a060` | ChangeSystemStatReceiver            | A |
| 65  | `FUN_0075a0e0` | AddictLoginTimeKindReceiver         | A |
| 66  | `FUN_0075a160` | HateStatusReceiver                  | A |
| 67  | `FUN_0075a200` | ChocoboReceiver                     | A |
| 68  | `FUN_0075a280` | ChocoboGradeReceiver                | A |
| 69  | `FUN_0075a300` | GoobbueReceiver                     | A |
| 70  | `FUN_0075a380` | VehicleGradeReceiver                | A |
| 71  | `FUN_0075a400` | GrandCompanyReceiver                | A |
| 74  | `FUN_0075a4b0` | AchievementPointReceiver            | A |
| 75  | `FUN_0075a530` | AchievementTitleReceiver            | A |
| 76  | `FUN_0075a5b0` | AchievementIdReceiver               | A |
| 77  | `FUN_0075a630` | AchievementAchievedCountReceiver    | A |
| 78  | `FUN_0076c690` | JobQuestCompleteTripleReceiver      | B |
| 79  | `FUN_0075a6b0` | JobChangeReceiver                   | A |
| 80  | `FUN_0075a730` | EntrustItemReceiver                 | A |
| 82  | `FUN_0075a7c0` | HamletSupplyRankingReceiver         | A |
| 83  | `FUN_0075a870` | HamletDefenseScoreReceiver          | A |
| 85  | `FUN_0075a920` | ChangeActorExtraStatReceiver        | A |
| 86  | `FUN_0075a9a0` | ChangeActorSubStatModeBorderReceiver | A |
| 88  | `FUN_0076c720` | ChangeActorSubStatStatusReceiver    | B |
| 89  | `FUN_0075aaa0` | ChangeShadowActorFlagReceiver       | A |

**Slot 56/57/58 — the SEQ_005 event lifecycle** — are particularly
load-bearing: every active script event passes through these three
slots, in order. KickReceiver (slot 56) kicks the event off,
StartServerOrderEventFunctionReceiver (slot 57) advances it,
EndClientOrderEventReceiver (slot 58) closes it.

## Slot validation walkthrough — SetEventStatus

The simplest case to read top-to-bottom is slot 48
(`FUN_00759d20`, 180 bytes — `asm/ffxivgame/00359d20_FUN_00759d20.s`):

```
;; bytes 00359d50..00359d62: unpack packet header from arg
MOV ESI, [ESP+0x80]              ; pkt = arg
MOVZX EAX, byte [ESI+0]          ; first byte
LEA ECX, [ESP+0x13]
PUSH EAX; PUSH ECX
CALL 0x0078ddc0                  ; small string ctor

;; bytes 00359d67..00359d75: more arg packing
MOVZX EDX, byte [ESI+4]
PUSH EAX; PUSH EDX; PUSH addr
CALL 0x0078de00                  ; another string ctor

;; bytes 00359d7d..00359d86: ─── CTOR ─────────────────────
PUSH EAX
ADD ESI, 0x5                     ; bump past packed header
PUSH ESI
LEA ECX, [ESP+0x1c]              ; ECX = &stack_receiver
CALL 0x0089d770                  ; SetEventStatusReceiver::ctor

;; bytes 00359d93..00359d9f: ─── RECEIVE ─────────────────
MOV ECX, [EDI+0x8]               ; EDI=actor; ECX = actor field
PUSH ECX
ADD EDI, 0x4
PUSH EDI
LEA ECX, [ESP+0x18]              ; ECX = stack_receiver
CALL 0x0089d860                  ; SetEventStatusReceiver::Receive

;; bytes 00359dac..00359db0: ─── DTOR ────────────────────
LEA ECX, [ESP+0x10]
CALL 0x0089d800                  ; SetEventStatusReceiver::dtor
```

Three sequential calls — ctor / Receive / dtor — confirm Pattern A.

## Why NullActorImpl maps the same slots

For the 5/6-slot Pattern-B receivers (Kick, JobQuestCompleteTriple,
UserData, ChangeActorSubStatStatus, etc.) the same slot index in
NullActorImpl ALSO writes the Receiver's vtable — i.e., NullActorImpl
allocates the receiver too. This makes sense if Pattern B's
"long-lived receiver" must be allocated even when the actor is the
Null impl (so the event lifecycle can complete cleanly).

For Pattern-A 2-slot receivers, NullActorImpl's slot N usually has no
receiver involvement — it just no-ops the packet.

## Unmapped 7 receivers — `Set*EventCondition` family

| Receiver | Pattern |
|---|---|
| `SetCommandEventConditionReceiver` | Unknown |
| `SetEmoteEventConditionReceiver` | Unknown |
| `SetNoticeEventConditionReceiver` | Unknown |
| `SetPushEventConditionWithCircleReceiver` | Unknown |
| `SetPushEventConditionWithFanReceiver` | Unknown |
| `SetPushEventConditionWithTriggerBoxReceiver` | Unknown |
| `SetTalkEventConditionReceiver` | Unknown |

All 7 are **event-condition configurators** — they set "when this
condition fires, kick this event". The receiver itself doesn't
deliver gameplay data; it stores a condition rule into some
per-event registry. The receiver is likely owned by an event-handler
instance (one per active event), not by the actor's LuaActorImpl —
which would explain why they don't appear in either 90-slot vtable.

This is consistent with Phase 9 #8b's finding that
`SetNoticeEventConditionReceiver` has a fallback that writes into
`ActorBase[+0x118]` — the receiver is "owned" by the actor's
condition list, not by LuaActorImpl.

To close this gap: find the owner class (probably under
`Application::Lua::Script::Client::Command::Network::Event::`),
walk its vtable for slots that ctor these 7 receivers.

## What this unlocks for SEQ_005 debugging

Per the existing `docs/seq005_receiver_gate_audit.md`, the SEQ_005
opening cinematic depends on a sequence of Kick / RunEventFunction /
EndEvent packets reaching their Receivers. Now we know the exact
dispatch path:

1. Network arrives with opcode 0x012F (Kick)
2. Opcode dispatcher routes to `actor.lua_impl→vtable[56]` (LuaActorImpl::slot56 = `FUN_0076c0d0`)
3. Slot 56 calls `KickClientOrderEventReceiver::ctor` (`FUN_0089f180`) — building the receiver in heap-allocated memory
4. The receiver is registered into the event registry
5. The receiver's vtable slot 2 (`KickClientOrderEventReceiver::Receive` = `FUN_0089e450`) is invoked when the kick payload is processed
6. The Receive body gates on `actor[+0x5c]` (Phase 7 finding — the kick-gate)

So the silent-kick-drop scenario unfolds as:
- garlemald emits 0x012F correctly
- LuaActorImpl::slot56 runs (no problem)
- The Receiver is constructed (no problem)
- BUT: the `actor[+0x5c]` gate inside KickClientOrderEventReceiver::Receive eats the kick

This re-confirms that the **gate fault is in the Receiver**, not in
the wire path. The dispatch layer works. The actor-state flag layer
is what needs fixing.

## What this doesn't unlock yet

**Open: the opcode → LuaActorImpl-slot mapping itself.** We now know
slot 56 = Kick, but the per-opcode dispatcher (the function that
loads the actor and CALLs the right slot index) isn't yet recovered.
Direct vtable-disp searches (`FF 90 c0 00 00 00` for slot 48 = disp
0xc0, etc.) all return zero hits, suggesting the dispatch goes
through a Lua-VM closure or a computed-index call (`CALL [EAX +
ECX*4]`) where the slot index is loaded at runtime from a packet
header field or a per-opcode constant.

**Update 2026-05-16 — `docs/packet_dispatch_router.md`:**
`FUN_004e20a0` is now decoded. It's a 4-case channel-control router
(opcodes 1/2/0xe/0x11 inline; everything else forwards to
`FUN_004e5ff0`). `FUN_004e5ff0` does channel-bound dispatch via
`channel->vtable[2]` after a tree-walk lookup (`FUN_004e5ca0`) keyed
on a packet header field. The dispatch genuinely is NOT a static
C++ vtable call — the tree-lookup result is most likely a Lua-VM
closure, and the per-opcode binding lives in script-load
registration code (sibling to `FUN_0078e3a0` per
`docs/lua_class_registry.md`).

**Update 2026-05-16 (later) — `docs/opcode_translation_table.md`:**
Hunted `FUN_0078fc90`'s siblings (the parent of `FUN_0078e3a0`) and
followed an opcode-immediate-pattern grep that surfaced
`FUN_0070ab40` as a 740-case jump table containing the SEQ_005
opcodes as `MOV imm32` operands. **Ruled out** as the per-opcode →
receiver dispatcher: `FUN_0070ab40` is an opcode-translation table
(input opcode → translated/canonicalized opcode), and the 5 SEQ_005
event opcodes (`0x012F..0x016B`) are NOT in its input set — they fall
through to its default `XOR EAX,EAX; RET 0`. The MOVs of `0x12F` etc.
that I saw are case-body OUTPUTS (input `0x25B..0x25D` → output Kick/
Start/End), suggesting a protocol-version alias map rather than a
dispatcher. Also ruled out: `FUN_0078fad0` (Lua-module alias init for
`global/math/string/table`).

The remaining work to close #5: walk channel-construction code (find
who writes to `channel[+8]`, the tree root walked by `FUN_004e5ca0`)
or decode `.le.lpb` scripts looking for `bindOpcode(0x012F, …)`-shaped
binders (~higher cost; other-session territory). Once recovered,
Phase 9 #7 ("cheat-sheet of what gate does each opcode's receiver
check") falls out for free.

## Regenerating

```sh
python3 tools/extract_receiver_actorimpl_map.py
```

Output:
- `build/wire/ffxivgame.receiver_actorimpl_map.json`
- `build/wire/ffxivgame.receiver_actorimpl_map.md`

Both gitignored under `build/`; the snapshot above is checked in via
this doc.

## Cross-references

- `docs/receiver_classes_inventory.md` — Phase 9 #1 (the 43 Receivers
  + their vtable RVAs + Phase 7 decomp summary)
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (the two-path
  dispatch model — Receiver class system vs DummyCallback /
  PacketProcessor)
- `docs/lua_actor_impl.md` — Phase 6 #6 (90-slot LuaActorImpl
  classification — small/medium/large bindings; this doc *names*
  what those slots do)
- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (KickReceiver
  Receive body + `actor[+0x5c]` gate)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7 #2
- `docs/event_end_receiver_decomp.md` — Phase 7 #3
- `docs/event_status_condition_receivers_decomp.md` — Phase 9 #8b
  (SetEventStatusReceiver + SetNoticeEventConditionReceiver decomp)
- `docs/event_change_actor_substat_status_decomp.md` — Phase 9 #2
  (ChangeActorSubStatStatusReceiver — the 5-slot System ns receiver)
- `docs/dynamic_cast_callsite_sweep.md` — Phase 9 ext (RTTI
  recovery; the 43 receiver RTTI addresses come from this sweep)
- `tools/extract_receiver_actorimpl_map.py` — the extractor
- `build/wire/ffxivgame.receiver_actorimpl_map.{json,md}` — raw
  regenerable output
