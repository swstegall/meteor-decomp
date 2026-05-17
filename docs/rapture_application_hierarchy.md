# Rapture::Application + Main — top-level engine hierarchy

> Recovered 2026-05-17 while tracing Phase 9 #5 Half A (the
> per-opcode → Lua-closure binder lives in `channel[+8]`'s tree;
> needed to identify the channel-manager class as a precursor to
> finding the tree-writer).

## TL;DR — class hierarchy from WinMain down to packet-dispatch

```
class Main {                                       ; outer app, RTTI ?AVMain@@
    void* vtable;                                  ; +0x00 = 0xf54a24 (10+ slots)
    char field_4[0x2c];
    Rapture::Application rapture;                  ; +0x30 — embedded sub-object
    char field_3a0[…];                             ; +0x3a0 — another sub-object (ctor 0x4b8640)
    char field_880[…];                             ; +0x880 — another (ctor 0x445cf0)
    char field_8d8[…];                             ; +0x8d8 — another (ctor 0x44c890)
    char field_960[…];                             ; +0x960
};

class Rapture::Application {                       ; RTTI ?AVRapture@Application@@
    void* primary_vt;                              ; +0x00 = 0xf8cc30 (parent vt write)
    void* secondary_vt;                            ; +0x04 = 0xf8cc1c (our identified vt, 7 slots)
    char field_8[0x2c];
    char field_34[…];                              ; zero-init members from +0x34 to +0x6c
    ChannelMgr* channel_mgr;                       ; +0x60 — INITIAL VALUE NULL (set later by Init code)
    char field_70[…];                              ; loop init of 2x 0x2c-byte structs
    char field_c8[…];                              ; init via FUN_00d35280
    char field_e8[…];                              ; init via FUN_00d351e0
    char field_340[…];                             ; init via FUN_00cc6cd0 (Phase 4 known helper)
    char field_368_byte;
    char field_369_byte_true;
    char field_3a0_…;
};
```

## Confirmed call chain — Rapture::Tick to packet dispatch

```
WinMain
  ↓
FUN_00401750 (Main ctor, 182 B)
  - sets Main vtable @ 0xf54a24
  - constructs Rapture at this+0x30 → CALL Rapture::Application::Application (0x4b3b50, 245 B)
  - constructs sub-objects at this+0x3a0, +0x880, +0x8d8, +0x960
  ↓
[runtime] Main::Run() (TBD — likely Main vtable slot 4 = FUN_004e4a20)
  ↓
Rapture::Application::Tick (FUN_004b3c50, 523 B, vtable slot 1 of Rapture)
  - iterates per-tick subsystems via virtual calls on [this+0x40..+0x48]
  - calls non-virtual member fns on [this+0x50, +0x54, +0x58, +0x60]
  - MOV ECX, [ESI+0x60]
  - CALL FUN_004e30a0                              ; this = channel_mgr
  ↓
FUN_004e30a0 (per-frame network tick, ~?? B, non-virtual on ChannelMgr)
  - reads [this+0x234] (the actual ChannelManager state)
  - reads [this+0x390..+0x3a8] (timing fields)
  - calls FUN_004e20a0 (the outer packet router) for each dequeued packet
  ↓
FUN_004e20a0 (channel-control router — see packet_dispatch_router.md)
  - opcodes 1/2/0xe/0x11 inline
  - else → FUN_004e5ff0 with ECX = channel = [packet+8]
  ↓
FUN_004e5ff0 (channel-bound dispatch)
  - calls channel->vtable[1]() setup
  - calls FUN_004e5ca0(&channel[+8], &result, packet)   ; the per-opcode lookup
  - calls channel->vtable[2]() commit
  ↓
FUN_004e5ca0 (RB-tree walk keyed on packet header field)
  - reads channel[+8]'s tree, returns handler ptr
```

## The two classes involved

There are TWO distinct classes in the network path:

1. **`Rapture::Application[+0x60]` — the "ChannelMgr"** (RTTI not yet
   recovered). Its non-virtual tick method is `FUN_004e30a0`. Manages
   the per-frame packet-dequeue loop. Has fields up to at least +0x3b0
   (extensively used by FUN_004e30a0). NOT directly virtual-dispatched
   in the dispatch chain.

2. **The "Channel"** (also RTTI not yet recovered). One instance per
   RUDP2 stream (3 of them per Phase 6 architecture: Lobby/Zone/Chat).
   Each packet carries `[packet+8] = owning_channel`. Has at least 3
   vtable slots used: slot 0 (unknown), slot 1 (setup at dispatch
   start), slot 2 (commit at dispatch end). Its field at offset `+8`
   is the **per-opcode handler tree** — an STL `std::map`-like
   structure walked by `FUN_004e5ca0`.

The Channel's tree (`channel[+8]`) is what Phase 9 #5 Half A needs to
recover. Each tree entry should map (opcode_key → handler_fn_ptr).

## Why I'm stopping here

Three angles attempted to find the tree-writer this session:

1. **Walking up FUN_004e30a0's callers** — surfaced
   `Rapture::Application::Tick` (slot 1 of Rapture) and `Main` as the
   outer class. Useful structural recovery, but doesn't directly reveal
   the tree writer.

2. **Searching for `MOV [reg+0x60], EAX` near operator-new** — found
   3 candidates (FUN_008ed010, FUN_00d4e9a0, FUN_0099f560) but none
   are in the Rapture-init code path. The channel-manager might be
   assigned via a non-allocating method (e.g. via a getter that
   returns a pre-existing object).

3. **Searching for FUN_004e30a0's own refs** — zero refs besides the
   single CALL site in FUN_004b3c50. Confirms it's not in any vtable
   (no dispatch-table hit) — it's just a directly-called member fn.

What's needed to close Half A: identify the **Channel class's vtable**
(the one with slot 1 / 2 used in FUN_004e5ff0). The Channel class is
the parent of the tree at `channel[+8]`. Its ctor is the natural site
of tree-init; its setup code (called from Main::Init or
Rapture::Init) would be where opcodes are bound.

Concrete next-session leads:

- **Trace `[packet+8]` upstream** in `FUN_00dae520` (the dummy-callback
  wrapper) — packet[+8] is populated by whoever creates the packet
  from the RUDP2 layer. Finding the packet ctor → likely reveals
  Channel's ctor.
- **Walk Main::Init / Rapture::Init for vtable writes near `RUDP2`
  setup** — RUDP2 (`Sqex::Socket::RUDP2`) is a known class from Phase
  1; finding its construction in Main::Init would reveal where each
  channel (Lobby/Zone/Chat) is created and bound.
- **Search for known RUDP2 ctor calls** — RUDP2's vtable is identified
  (per `docs/decomp-status.md` Phase 1); finding callers of RUDP2's
  ctor would surface the channel-construction code.

## Independent value

Even without closing Half A, this session recovered:

- **Main + Rapture::Application class hierarchy** — useful for any
  future investigation into the engine's top-level lifecycle.
- **Channel-manager location** — `Rapture::Application[+0x60]`, with
  non-virtual tick at `FUN_004e30a0`.
- **Confirmation of two-class network architecture** — ChannelMgr
  (singleton on Rapture) + Channel (per-stream, one of 3 RUDP2 streams).

## Cross-references

- `docs/packet_dispatch_router.md` — Phase 9 #5 (decomp of
  `FUN_004e20a0` and the dispatch chain from packet to
  `channel->vtable[2]`)
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 (the 35
  receivers mapped to LuaActorImpl slots; this doc continues the
  hunt for the per-opcode binder)
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (the dummy-
  callback dispatch chain `FUN_00dae520` upstream of
  `FUN_004e20a0`)
- `docs/decomp-status.md` — Phase 1 (RUDP2 / RUDPSocket / PollerImpl
  recovered classes — candidates for the Channel base class)
- `memory/reference_meteor_decomp_actor_rtti.md` — earlier RTTI walk
  recipe (applicable to finding the Channel class's vtable)
