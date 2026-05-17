# Phase 8 #9 — Two-path packet dispatch in the FFXIV 1.x client

> Last updated: 2026-05-15. Resolves the long-standing question
> "where does the runtime-registered 0x0133 / 0x017A handler live?"
> Answer: it doesn't — those opcodes use a **different path** than
> Phase 7's Kick/RunEventFunction/EndEvent receivers.

## TL;DR

The 1.x client has **two parallel packet-handling paths**, used by
different opcode families:

1. **Receiver class system** (Phase 7-decoded) — opcodes that need
   actor-bound event lifecycle (`0x012F KickEvent`, `0x0130
   RunEventFunction`, `0x0131 EndEvent`, `0x0136 SetEventStatus`,
   `0x016B SetNoticeEventCondition`, etc.) dispatch through dedicated
   classes under `Application::Lua::Script::Client::Command::Network::*Receiver`,
   each with a 2- or 5-slot vtable. Slot 2 is `Receive(payload)`.

2. **Channel-bound stub-vtable dispatch** (this doc) — opcodes that
   read into work-table state (`0x017A SynchGroupWorkValues`, `0x0133
   GenericData / GROUP_CREATED` IN-side, etc.) flow through the
   `ZoneProtoChannel` → `ZoneProtoDownCallbackInterface` → per-opcode
   vtable slot path. The actual concrete callback at runtime is
   `ZoneProtoDownDummyCallback`, whose per-opcode slots are 3-byte
   `ret 0xc` stubs. The "real handler" for these opcodes is **not in
   any vtable** — the packet is consumed earlier in the chain by the
   dequeue / metadata-init code, which writes packet fields into a
   work-table that other code (Lua scripts, group state machines)
   reads on its own schedule.

The Group system docs (`docs/group_system_decomp.md`) cover the
work-table side. This doc covers the *dispatch* side: how packets
get from the network into the work tables.

## The dispatch wrapper — `FUN_00dae520`

Single caller of the dispatcher fn. Constructs a `DummyCallback`
*on the stack*, calls the dispatcher with it, then reads packet
fields the dispatch's prologue populated.

```c
// Pseudo-code reconstruction of FUN_00dae520 (181 bytes)
bool dispatch_one_packet(ChannelManager* mgr, Packet** out) {
    if (!mgr.dequeue_one_packet()) return false;          // FUN_00db1960

    char buf[0x14];                                        // local DummyCallback
    ZoneProtoDownCallbackInterface_ctor(&buf);             // FUN_00dbfd00 — sets vtable to Interface
    *(void**)&buf = (void*)0x1128b4c;                      // OVERWRITE vtable to DummyCallback

    /* the call below dispatches via the DummyCallback's vtable, so
       every per-opcode slot is `ret 0xc` — the "dispatch" is a
       no-op for game logic. The dispatcher's prologue, however,
       reads the opcode + payload offsets and stashes them into
       fields the caller reads back via [esp+0x2c]. */
    ZoneProtoDispatcher(&buf, packet, 0);                  // FUN_00dbfd10
    return true;
}
```

The dispatcher (`FUN_00dbfd10`, the slot 1 of `ZoneProtoDownCallbackInterface`):

```asm
mov edx, [esp+8]           ; load packet ptr from arg
mov eax, [edx+8]           ; load packet header struct
mov eax, [eax+0x24]        ; load some pointer (header at +0x24)
movzx esi, word [eax+2]    ; load opcode (u16)
add esi, -1                ; opcode - 1
cmp esi, 0x1f5             ; > 501?
ja default                 ; → default case (opcode out of range)
movzx esi, byte [esi + byte_table_va]   ; case_idx = byte_table[opcode-1]
jmp dword [esi*4 + jump_table_va]       ; → case
```

Each "case" loads `vtable[N]` from the callback object and calls it
with the payload — but for DummyCallback every slot is just
`ret 0xc`.

So the dispatch APPEARS to do per-opcode work but actually doesn't.
The opcode is read for routing; the call lands on a stub; the
caller assumes the slot did its work and reads packet fields out of
the local DummyCallback's storage area.

## Why DummyCallback?

The "DummyCallback" wasn't a debug placeholder — it's the
production callback. Two reasons the engine works this way:

1. **Most "real" handlers want side-effect semantics that don't fit
   the per-opcode vtable model.** Opcodes like 0x017A
   SynchGroupWorkValues update a *work table* (the per-group
   `SharedWork` instance with its `SyncWriter`-backed fields, see
   `docs/group_system_decomp.md`). The receiver doesn't need a
   per-opcode dispatch — it needs to `memcpy` the payload into the
   work table at the right offsets, and let scripts subscribed to
   field changes notice on their own polling cycle.

2. **The `Application::Lua::Script::Client::Command::Network::*Receiver`
   classes handle the opcodes that DO need per-opcode dispatch.**
   These have their own 5-slot vtables and live as Lua-engine-bound
   objects. The Lua engine wires them up at script-load time —
   that's what creates the *appearance* of "runtime registration"
   but it's actually compile-time RTTI'd, just under a different
   namespace than the network channel.

## The Receiver class system (cross-ref Phase 7)

49 RTTI'd Receiver classes under
`Application::Lua::Script::Client::Command::*::Receiver`. The
significant ones for SEQ_005:

| Opcode | Receiver class | Vtable RVA | Slots |
|---|---|---|---|
| `0x012F` KickEvent | `KickClientOrderEventReceiver` | `0xc574b0` | 5 |
| `0x0130` RunEventFunction | `StartServerOrderEventFunctionReceiver` | `0xc574c8` | 5 |
| `0x0131` EndEvent | `EndClientOrderEventReceiver` | `0xc57348` | 5 |
| `0x0136` SetEventStatus | `SetEventStatusReceiver` | `0xc573a0` | 2 |
| `0x016B` SetNoticeEventCondition | `SetNoticeEventConditionReceiver` | `0xc573ac` | 2 |
| `0x0136`+`type=2` Push | `SetPushEventConditionWith{Circle,Fan,TriggerBox}Receiver` | `0xc573b8..c4` | 2 |

Phase 7 already decomposed slots 2 (Receive) of the 5-slot variants.
The 2-slot variants are simpler (slot 0 = dtor, slot 1 = Receive).

The decoded slot 2 of `KickClientOrderEventReceiver` (Phase 7,
`docs/event_kick_receiver_decomp.md`) shows the now-known
`actor[+0x5c]` gate that drops kicks targeting actors not yet
spawned on the client.

## Opcodes WITHOUT a dedicated Receiver class

Browsing the 49 Receiver classes — several common opcodes have no
dedicated class:

- `0x0133` (IN: GROUP_CREATED, OUT: GenericData) — no Receiver
- `0x017A` (OUT: SynchGroupWorkValues) — no Receiver
- `0x017C/D/E/F` (OUT: GroupHeader/Begin/X/End) — no Receiver
- `0x0183` (OUT: ContentMembersX08) — no Receiver

All of these are *group-related*. They flow through the
`ZoneProtoDownCallbackInterface` → `DummyCallback` path AND get
consumed by the `Group::PacketProcessor` (3 slots, RVA `0xbd42f4`,
documented in `docs/group_system_decomp.md`). The PacketProcessor
walks the channel's packet queue independently — it doesn't go
through the per-opcode dispatch.

So the "two parallel paths" really are:

```
       ┌──── network arrives ────┐
       │                         │
       v                         v
  Receiver class             Group::PacketProcessor
  (per-opcode vtable         (queue walk + 2-subdec
   dispatch via              dispatch on each packet,
   Lua engine)               with state-mutation side
                             effects on SharedWork)
```

Both consume the same packet stream. Different opcodes belong to
different paths.

## Practical impact for garlemald

1. **The Phase 8 work that decoded `Group::PacketProcessor` +
   `SyncWriter` is the right place to look for `0x017A` / `0x0133`
   semantics.** There's no per-opcode dispatch entry to find — the
   packet-handling side IS the queue-walk + work-table-mutate
   pattern documented in `docs/group_system_decomp.md`.

2. **For SEQ_005-style kicks, the KickClientOrderEventReceiver's
   `actor[+0x5c]` gate is the dispositive check.** Garlemald's job
   on the wire is to ensure the actor-spawn sequence completes
   before the kick fires. The pre-warp ordering work in
   `apply_do_zone_change_content` (commit `7e895dd`) is the right
   shape; remaining work is making sure the spawn packet sequence
   actually sets the `+0x5c` flag client-side.

3. **The dispatch path is essentially vestigial for game logic.**
   When debugging "is this opcode handled?", checking whether the
   opcode has a dedicated Receiver class is enough. If yes, look at
   slot 2 of the receiver's vtable. If no, the opcode flows into a
   work-table via the channel-bound queue, and the relevant entry
   point is `Group::PacketProcessor::OnPacket` (or one of its
   sibling Updaters).

## Cross-references

- `docs/receiver_dispatch_via_actorimpl.md` — **Phase 9 #5 (2026-05-16):
  closes the "Lua engine wires them up" hypothesis above with a
  concrete mapping — 35 of 42 Receivers dispatch via specific vtable
  slots on `Component::Lua::GameEngine::{LuaActorImpl, NullActorImpl}`,
  including slot 56 = Kick, slot 57 = RunEventFunction, slot 58 =
  EndEvent (the SEQ_005 event lifecycle).**
- `docs/group_system_decomp.md` — Group system + PacketProcessor +
  SharedWork + SyncWriter + per-opcode wire formats for the
  no-Receiver opcodes
- `docs/event_kick_receiver_decomp.md` — Phase 7 decomp of
  KickClientOrderEventReceiver (slot 2 + actor[+0x5c] gate)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7
  decomp of StartServerOrderEventFunctionReceiver
- `docs/event_end_receiver_decomp.md` — Phase 7 decomp of
  EndClientOrderEventReceiver
- `garlemald-server/docs/post_warp_respawn_fix_analysis.md` — the
  garlemald-side application of the actor[+0x5c] finding
