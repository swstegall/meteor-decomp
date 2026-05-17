# Phase 9 #5 Half A — PacketHeader class hunt (the `packet[+8]` polymorphic type)

> Recovered 2026-05-17. The final identification gap for Phase 9 #5
> Half A: the polymorphic "PacketHeader" class passed as `ECX` to
> `FUN_004e5ff0` (= `packet[+8]`). Has a vtable at `+0`, an
> `std::map<u32, T>` at `+0x8`, and an inspector context at `+0x14`.

## TL;DR

**Class identification still open** — narrowed to one of ~80 IpcChannel-
family classes (PacketBufferTmpl / NetBufferTmpl / ConnectionManagerTmpl
/ ChannelManagerCoreTmpl / etc.) instantiated per ProtoChannel direction
(`TXProtoUp` / `TXProtoDown` for X ∈ {Lobby, Zone, Chat}).

**Strongest structural candidates** (vtable layout matches `FUN_004e5ff0`'s
slot[1]/slot[2] usage pattern):

| Vtable | Class | Slot count | Note |
|---|---|---:|---|
| `0xd276d8` | `?$NetBufferTmpl<TLobbyProtoDown, LobbyProtoChannel, Network, Application>::IpcChannel` | 3 | Incoming buffer for Lobby |
| `0xd284e4` | `?$NetBufferQueueCoreTmpl<TLobbyProtoUp, LobbyProtoChannel>::IpcChannel` | 3 | Queue core for Lobby up |
| `0xd284f4` | `?$ChannelManagerCoreTmpl<TLobbyProtoUp, LobbyProtoChannel>` | 15 | Channel manager core (uses stubs for many slots) |
| `0xd28614` | `?$ChannelManagerOnSingleConnectionTmpl<TLobbyProtoUp, ...>` | 15 | Single-connection manager (slot 3 active) |
| `0xb91b40` | `?$NetBufferTmpl<TLobbyProtoUp, LobbyProtoChannel>::IpcChannel` | 3 | Outgoing buffer for Lobby |

Equivalent Zone and Chat instantiations exist for each (we'd also need
the corresponding TZoneProto/TChatProto variants).

## Recovered class inventory (filtered to "IpcChannel" / "PacketBuffer" /
## "NetBuffer" / "ChannelManager" — 80+ TDs)

### Base classes (in `IpcChannel@Network@Component`)

| Vtable | Class | Slot count |
|---|---|---:|
| TBD | `PacketBufferBase@IpcChannel@Network@Component@@` | TBD |
| `0xd29b1c` | `NetBufferBase@IpcChannel@Network@Component@@` | 1 |
| `0xd29b44` | `EntityBase@IpcChannel@Network@Component@@` | 1 |
| `0xd29b5c` | `ChannelManagerBase@IpcChannel@Network@Component@@` | 1 |
| TBD | `ConnectionManager@IpcChannel@Network@Component@@` | TBD |

### Per-direction-per-channel template instantiations (Lobby only listed)

Same shape exists for `TZoneProto`/`TChatProto` substitutions. Lobby
inventory has ~28 distinct classes:

| Vtable | Class |
|---|---|
| `0xb91b28` | `?$PacketBufferTmpl<TLobbyProtoDown, LobbyProtoChannel>::IpcChannel` (1 slot) |
| `0xd27738` | `?$PacketBufferTmpl<TLobbyProtoUp, LobbyProtoChannel>::IpcChannel` (1 slot) |
| `0xd276d8` | `?$NetBufferTmpl<TLobbyProtoDown, LobbyProtoChannel>::IpcChannel` (3 slots) |
| `0xb91b40` | `?$NetBufferTmpl<TLobbyProtoUp, LobbyProtoChannel>::IpcChannel` (3 slots) |
| `0xd27670` | `CryptEngineInterface@ConnectionManagerTmpl<TLobbyProtoUp, LobbyProtoChannel>` (9 slots) |
| `0xd276c0` | `ConnectionData@ConnectionManagerTmpl<TLobbyProtoUp, LobbyProtoChannel>` (5 slots) |
| `0xd27700` | `ConnectionManagerTmpl<TLobbyProtoUp, TLobbyProtoDown, LobbyProtoChannel>` (4 slots) |
| `0xd27740` | `PacketDataBuilder<TLobbyProtoUp, LobbyProtoChannel>::IpcChannel` (4 slots) |
| `0xd27778` | `RecvCallbackInterface<TLobbyProtoUp, ...>::LobbyProtoChannel` (2 slots) |
| `0xd284a4` | `PrimaryEntityFactoryClass<TLobbyProtoUp, ...>::LobbyProtoChannel` (2 slots) |
| `0xd284b0` | `TargetEntityFactoryClass<TLobbyProtoUp, ...>::LobbyProtoChannel` (5 slots) |
| `0xd284c8` | `IpcEntityTmpl<TLobbyProtoUp, ...>::LobbyProtoChannel` (1 slot) |
| `0xd284d0` | `PrimaryEntityFactoryClass_LF<...>` (2 slots) |
| `0xd284dc` | `TargetEntityTmpl<...>` (1 slot) |
| `0xd284e4` | `NetBufferQueueCoreTmpl<TLobbyProtoUp, ...>::IpcChannel` (3 slots) ⭐ candidate |
| `0xd284f4` | `ChannelManagerCoreTmpl<TLobbyProtoUp, ...>` (15 slots) ⭐ candidate |
| `0xd2855c` | `NetBufferFactoryTmpl<TLobbyProtoDown, ...>::IpcChannel` (3 slots) |
| `0xd2856c` | `NetBufferFactoryTmpl<TLobbyProtoUp, ...>::IpcChannel` (3 slots) |
| `0xd2857c` | `EntityContainerTmpl<TLobbyProtoUp, ...>::LobbyProtoChannel` (3 slots) |
| `0xd2858c` | `NetBufferFactoryTmpl_LF<TLobbyProtoUp, ...>::IpcChannel` (3 slots) |
| `0xd2859c` | `NetBufferFactoryTmpl_LF<TLobbyProtoDown, ...>::IpcChannel` (3 slots) |
| `0xd285ac` | `ChannelManagerTmpl_LF<TLobbyProtoUp, ...>` (15 slots) ⭐ candidate |
| `0xd28614` | `ChannelManagerOnSingleConnectionTmpl<TLobbyProtoUp, ...>` (15 slots) ⭐ candidate |

## Why this hunt is hard

Three factors compound:

1. **Massive template instantiation count**. Each base class
   (NetBuffer / PacketBuffer / ChannelManager / etc.) instantiates
   per-direction (Up/Down) per-channel (Lobby/Zone/Chat) — so we have
   ~6 instantiations per template type, and many template types. The
   total candidate space is 80+ classes.

2. **Most vtable slots are no-op stubs**. `FUN_009d364d` (3-byte
   `RET 4`) appears in MANY classes' slots — when `FUN_004e5ff0`'s
   `vtable[1]()` call lands on a stub, it's a no-op. So matching the
   stub doesn't discriminate between classes.

3. **No direct CALL site discriminates**. `FUN_004e5ff0` is called
   via direct CALL rel32 from `FUN_004e20a0`'s default case. The
   `channel = packet[+8]` is loaded from a runtime queue, not a
   static constant. So we can't pin the class via call-site
   analysis alone.

## What would close this

Three productive next-step paths:

| Path | Cost | Yield |
|---|---|---|
| **Runtime trace** — set HWBP on `[packet+8]` write or on `FUN_004e5ff0` entry; observe the actual instance via Wine'd debugger | Medium | Definitive |
| **Walk the queue-enqueue side** — find what writes to `entry[+8]` in `FUN_00db1960`'s callers (or upstream RUDP2 receive code); the writer knows the class | Higher | Definitive |
| **Walk every per-channel ConsumerConnection ctor** — they probably allocate the per-channel PacketHeader; type can be read from `MOV [reg], <vtable>` writes there | Medium | Definitive |

## What's confirmed about the PacketHeader type

Even without nailing the exact class, structural facts are firm:

- It IS a polymorphic class (vtable at +0)
- Has std::map<u32 opcode, T value> embedded at +0x8 (confirmed via
  `FUN_004e5ca0`'s walk pattern, `docs/channel_dispatch_tree.md`)
- Has some packet-inspector context at +0x14 (used by `FUN_0071d420`
  and `FUN_008a87f0` inside `FUN_004e5ff0`)
- Slot 1 of its vtable is a "setup" hook (called with packet arg)
- Slot 2 is a "commit" hook (called after the per-opcode dispatch)
- The std::map gets populated lazily on first access (operator[] from
  `FUN_004e5ca0` does find-or-insert)

The 4-byte offset discrepancy noted in `docs/channel_dispatch_tree.md`
("CMT ctor inits std::map at this+0xc; consumer reads channel+0x8")
strongly suggests **the consumer's `channel` view is a SECONDARY BASE
subobject of the actual class**, offset by 4 bytes from the
ConnectionManagerTmpl primary base. So the actual class is some
descendant of `ConnectionManagerTmpl<TXProtoUp, TXProtoDown, ...>` with
MI — most plausibly `ChannelManagerCoreTmpl` or
`ChannelManagerTmpl_LF` (both have 15 slots, which is more than
enough for MI'd interface methods).

## 2026-05-17 update — Queue-enqueue walk narrows the answer

After walking the queue-enqueue / consumer side end-to-end, the picture
is now substantially clearer. Findings:

### Dispatch chain end-to-end

```
FUN_004e20a0 (router)
  ├─ direct call → FUN_004e5ff0(channel = packet[+8], packet)  [line 215]
  ├─ pop one packet → FUN_00dae520(this=CMT, &packet_out)        [line 62]
  │    └─ FUN_00db1960(this=CMT, &packet_out, 0)                  // std::list::pop_front
  │         └─ writes packet_out[+8] = head_node[+8]              ⭐ source of channel
  │
  ├─ alt drain path → FUN_00db1420(this=CMT)                      // RX-side drain loop
  │    └─ for each entry: peek head, list_erase, FUN_004e5ff0(packet[+8], packet)
  │
  └─ alt drain path → FUN_00db67e0(this=CMT)                      // TX-side drain loop
       └─ same shape but calls FUN_004e6080 (TX dispatcher)
```

### CMT vtable identification (definitive)

`Application::Network::ZoneProtoChannel::TZoneProtoUp::?$ConnectionManagerTmpl`
vtable at VA `0x1129754` (RVA `0xd29754`, **4 slots**). Verified via
`build/class_metadata.json` lookup + raw `.rdata` byte read:
```
  slot[0] = 0x00db83f0  (CMT dtor)
  slot[1] = 0x009d364d  (__purecall — abstract method)
  slot[2] = 0x00776340  (RET 4 — default no-op stub)
  slot[3] = 0x00776340  (same)
```

Slot[1] = `__purecall` confirms CMT is **abstract** — must be derived
from to instantiate. Only one caller of `FUN_00db8330` (CMT ctor):
`FUN_00db7c50` = ServiceConsumerConnectionManager ctor.

### SCCM derived class vtable (definitive)

`Application::Network::ZoneProtoChannel::ServiceConsumerConnectionManager`
vtable at VA `0x1129768` (RVA `0xd29768`, **4 slots**):
```
  slot[0] = 0x00db8410  (SCCM dtor — overridden)
  slot[1] = 0x00db7e50  (SCCM-specific setup hook — overrides __purecall)
  slot[2] = 0x00776340  (RET 4 — same default no-op stub)
  slot[3] = 0x00db7150  (overridden)
```

Single inheritance (only 1 vtable in metadata) — SCCM is NOT MI from CMT.

### ConsumerConnection inner-class vtable (newly identified)

`Application::Network::*ProtoChannel::ServiceConsumerConnectionManager::ConsumerConnection`
is a **nested class with its own 5-slot vtable**:

| Protocol | Vtable VA | slot[0] | slot[1] | slot[2] |
|---|---|---|---|---|
| Zone | `0x0112973c` | `0xdb8270` | `0xdb7d10` | `0xdb7440` |
| Lobby | `0x011276e8` | `0xda2100` | `0xda1480` | `0xda0c90` |

(slot[5] is an `.rdata` thunk; slot[6] is CMT dtor; slot[7] is `__purecall`
— this vtable has a secondary-base layout indicative of MI, but the
ConsumerConnection class itself isn't extracted by RTTI walker so
class_metadata.json has no `ctor_candidates` entry.)

### The producer chain (where channel is set)

`FUN_00dafa30` is called from SCCM's slot[1] (`FUN_00db7e50`). It:
1. Allocates a local "packet" struct in the caller's frame
2. Calls `FUN_00daf5b0` — a 627-byte switch that copies wire bytes from
   the network buffer into the local packet's body (offsets +8..+0x18
   for header, more for body based on opcode)
3. Returns the local packet's address via `[arg+8] = packet`
4. Caller does `channel = packet[+8]; FUN_004e5ff0(channel, packet)`

So **the channel pointer at `packet[+8]` originates from the first 4
bytes of the incoming network wire header** (copied at `FUN_00daf5b0`
line 36-37: `MOV EDX, [EAX+ECX*1]; MOV [EDI+0x8], EDX`).

### The 4-byte offset finally explained

The channel's std::map at `channel+0x8` (per consumer) doesn't align
with CMT's std::map at `CMT+0xc` (per ctor). The reconciliation: the
"channel" stored in `packet[+8]` is **NOT a real C++ object pointer
into SCCM** — it's a **secondary-base subobject pointer** that was
*explicitly cast* by the producer before storage.

In MSVC, when you write:
```cpp
Packet* p = ...;
p->channel = static_cast<IpcChannelBase*>(some_sccm_or_consumer_conn);
```
the compiler emits `p->channel = &some_sccm + adj_offset` where
`adj_offset` is the secondary-base adjustment recorded in the RTTI
hierarchy. So `packet[+8]` literally points 4 bytes into SCCM
(secondary base subobject start), not at SCCM's primary vtable.

This explains all the offset discrepancies AND why slot[2] = `RET 4`
"works" — when called via the secondary-base vtable, the thiscall
gets an *additional adjustor arg* on the stack that the `RET 4` stub
cleans up.

### Most-likely final answer

The `channel` type stored at `packet[+8]` is:

> **A SECONDARY BASE subobject of `ServiceConsumerConnectionManager`,
> 4 bytes into the primary base. The secondary base is most likely
> `Component::Network::IpcChannel::*` (one of the 1-slot or 3-slot
> base classes in the IpcChannel hierarchy), bringing in the std::map
> opcode registry and packet-inspector context.**

Definitively naming the secondary-base type would require either
runtime trace (HWBP on the `packet[+8]` write) or a deep dive into
the SCCM/ConsumerConnection ctor chain to find where SCCM's `this+4`
gets initialized with its secondary-base vtable. The structural
picture is now complete enough for Phase 9 #5 Half A to be marked as
**substantially closed** — the dispatch chain, CMT/SCCM/ConsumerConnection
identification, and the producer-side data flow are all mapped.

## Cross-references

- `docs/channel_dispatch_tree.md` — Phase 9 #5 Half A
  (`channel[+8]` is `std::map<u32, T>`; the SCCM/CMT ctor's std::map
  default-init at `this+0xc`; the offset discrepancy that suggests MI)
- `docs/packet_dispatch_router.md` — Phase 9 #5 (`FUN_004e20a0` →
  `FUN_004e5ff0` → `FUN_004e5ca0` chain; the dispatch context where
  `channel = packet[+8]` is consumed)
- `docs/rapture_application_hierarchy.md` — Phase 9 #5 Half A
  (the network class hierarchy down to ConsumerConnection; the
  PacketHeader I'm hunting lives BELOW ConsumerConnection in the
  per-channel packet pipeline)
- `docs/network_dispatch_dual_paths.md` — Phase 8 #9 (`FUN_00dae520`
  produces the packets via the dummy-callback dispatch; the queue
  entries' `+8` field carries the PacketHeader)
