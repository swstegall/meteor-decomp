# FFXIV 1.x wire protocol — architectural overview

This is the canonical "what we know" from reading `ffxivgame.exe`'s
RTTI + strings + vtable contents. Cross-references with
`project-meteor-server` (C# server) and `garlemald-server` (Rust
server) are the naming convention.

## TL;DR

The 1.x client opens **three concurrent IpcChannels** to backend
servers:

| Channel               | Purpose                          | Up payload union                     | Down payload union                       |
|-----------------------|----------------------------------|--------------------------------------|------------------------------------------|
| `LobbyProtoChannel`   | Login + character select + gate  | `LobbyProtoUp`                       | `LobbyProtoDown`                         |
| `ZoneProtoChannel`    | World/map gameplay               | `ZoneProtoUp`                        | `ZoneProtoDown`                          |
| `ChatProtoChannel`    | Chat (independent of zone)       | `ChatProtoUp`                        | `ChatProtoDown`                          |

Each channel has a `ServiceConsumerConnectionManager` that owns:
- one or more `ConsumerConnection` instances,
- a per-connection `LobbyCryptEngine` (or zone/chat equivalent —
  same code, different keys), implementing `CryptEngineInterface`.

Underneath, the **transport is RUDP2** (Reliable UDP version 2 — SE's
in-house protocol, NOT raw TCP). Segment types observed:

| RTTI class                       | Purpose                                          |
|----------------------------------|--------------------------------------------------|
| `Sqex::Socket::RUDP2::SYNSegment` | Connection establishment                        |
| `Sqex::Socket::RUDP2::ACKSegment` | Acknowledgement                                 |
| `Sqex::Socket::RUDP2::EAKSegment` | Extended ack (selective)                        |
| `Sqex::Socket::RUDP2::DATSegment` | Data carrier (the IPC packets)                  |
| `Sqex::Socket::RUDP2::NULSegment` | Keepalive / null                                |
| `Sqex::Socket::RUDP2::RSTSegment` | Reset / disconnect                              |

The lower socket abstraction is `Sqex::Socket::Socket` →
`SocketBase` → `SocketImpl` → `RUDPSocket` → `RUDP2::RUDPImpl`. There
are also two `PollerImpl` variants — `PollerWinsock` (the live one
on Windows) and `PollerBase::PeerSocket` (interface). On real
servers, the listening side runs `Application::Network::ZoneProtoChannel::SocketThread`
+ its `ChatProtoChannel` cousin in dedicated threads.

> **Practical implication for `garlemald-server`**: the existing
> Rust transport reads **TCP**, not RUDP2. Project Meteor reverse-
> engineered the wire as TCP because the retail-era client always
> connected to TCP-tunnelled forwards via the launcher's `ws2_32.dll`
> shim. The native client transport is RUDP2 over UDP. If we ever
> want to drive the unmodified client (no `ws2_32` shim), we need a
> RUDP2 server. Until then, TCP-via-shim is fine.

## Crypto: OpenSSL 1.0.0 + Blowfish

The binary statically links **OpenSSL 1.0.0 (29 Mar 2010)** —
confirmed via the embedded version string `Blowfish part of OpenSSL
1.0.0 29 Mar 2010` at `.rdata` RVA 0x4048. The full crypto suite is
present (RSA, AES, RC4, SHA1/256/512, X.509 ASN.1) — most of it is
used for the SqexId login flow's TLS-like authentication
(`Sqex::Login::SqexIdAuthentication`).

For the per-channel cipher (`LobbyCryptEngine` /
`CryptEngineInterface`), the algorithm is **Blowfish**. The OpenSSL
Blowfish P-array initial state (`0x243F6A88, 0x85A308D3, 0x13198A2E,
0x03707344`) appears at two locations:

- file offset 0xb84078 (`.rdata`) — the static `bf_pi[]` array used
  for key-schedule init,
- file offset 0xe67278 (`.data`) — a runtime `BF_KEY` instance
  pre-zeroed at process startup.

Both sites match `openssl-1.0.0/crypto/bf/bf_init.c` byte-for-byte;
this is canonical OpenSSL Blowfish, not a custom variant. The
`BF_set_key`, `BF_encrypt`, `BF_decrypt` functions are present in
`.text` (look up by string proximity to "blowfish part of OpenSSL").

For matching decomp, we link against the same OpenSSL 1.0.0 build
rather than re-deriving the cipher. The interesting code is the
*key derivation* (how the per-session key is generated from the
SqexId token), which is `LobbyCryptEngine::Init` or equivalent — to
be located via vtable walk.

## Packet framing (TCP-shim view)

This is what `garlemald-server` and Project Meteor implement; it's
the wire layout *after* the RUDP2 layer has been stripped (or via
the `ws2_32.dll` TCP-tunnel that the workspace's `ffxiv-actor-cli`
uses).

### `BasePacketHeader` — 16 bytes

```c
struct BasePacketHeader {
    uint8_t   is_authenticated;   // 0 = no, 1 = encrypted
    uint8_t   is_compressed;      // 0 = no, 1 = zlib (deflate)
    uint16_t  connection_type;    // 1 = ZONE, 2 = CHAT (0 = LOBBY)
    uint16_t  packet_size;        // total bytes including this header
    uint16_t  num_subpackets;
    uint64_t  timestamp;          // ms since epoch (sender's clock)
};
```

(Source: `garlemald-server/common/src/packet.rs` — confirmed against
the binary's frame parser that reads exactly these fields in this
order off the recv buffer.)

### `SubPacket` (game-message envelope)

Each `BasePacket` carries `num_subpackets` of these:

```c
struct SubPacketHeader {
    uint16_t  size;            // including this header
    uint16_t  source_id;       // routing tag — usually the actor id
    uint16_t  target_id;       // routing tag — usually the player id
    uint16_t  unknown_06;      // observed always 0; reserved
    uint16_t  type;            // 0x03 = gamemessage, others = control
    uint16_t  unknown_0a;      // observed always 0
    uint16_t  unknown_0c;
    uint16_t  unknown_0e;
};
struct GameMessageHeader {     // type = 0x03 only
    uint16_t  opcode;          // the thing garlemald::opcodes calls OP_*
    uint16_t  unknown_02;
    uint32_t  source_id;
    uint32_t  unknown_08;
};
```

(Source: `garlemald-server/common/src/subpacket.rs`. Re-deriving
from the binary requires walking the `Component::Network::IpcChannel::PacketBufferTmpl`
vtable's read-callbacks — TBD via `tools/extract_net_vtables.py`.)

## Opcode space — three views

**Garlemald-server's authoritative list**:
`garlemald-server/map-server/src/packets/opcodes.rs` — ~280 named
constants grouped by direction (`OP_*` outbound, `OP_RX_*` inbound)
and major subsystem (handshake, world↔map session, actor lifecycle,
chat, social, recruiting, achievements, etc.).

**Binary's view**: each direction has a `union` type
(`LobbyProtoUp` / `LobbyProtoDown` / `ZoneProtoUp` / etc.) whose
members are the per-opcode payload structs. The
`Component::Network::IpcChannel::PacketBufferTmpl<union ...>`
template instantiates a buffer keyed by the union, and the
opcode-dispatch logic switches on the union's tag. This is the
fastest route to the opcode → struct map *if* we can recover the
union member layout — which we can, because each union member has
its own RTTI vtable and `__FILE__` strings.

**Project Meteor's view**: the C# `Packets/Send/*.cs` and
`Packets/Receive/*.cs` files have one class per opcode with the
struct fields named. Garlemald's Rust ports those names; we use the
same names in `meteor-decomp`'s `include/net/`.

## What's actionable for `garlemald-server` today

1. ✅ **`tools/extract_net_vtables.py`** — done. 576 net-relevant
   classes / 9,729 vtable slots dumped to
   `build/wire/<binary>.net_handlers.md`. Each row links to the
   per-function asm/ file.
2. ✅ **`tools/extract_gam_params.py`** — done. Parses the mangled
   `Component::GAM::CompileTimeParameter<id, &PARAMNAME_id, T,
   Decorator>` types from `.rdata` and emits the structured
   `(id, namespace, type, decorator)` registry to
   `build/wire/<binary>.gam_params.md` +
   `config/<binary>.gam_params.{json,csv}`. **192 unique params
   recovered** across six Data classes:
   - `Player` (92 params, ids 135-233): bool flag arrays up to
     `bool[16384]`, int/short arrays up to `[300]`, signed char
     gear-slot arrays `[16]`, plus single-value scalars.
   - `PlayerPlayer` (37 params): includes `Blob<2500>` and
     `Blob<128>[16]`.
   - `CharaMakeData` (26): chiefly `signed char` (face/body
     attributes) and `short` (hairstyle, etc.).
   - `ClientSelectData` / `ClientSelectDataN` (17 each): char
     select metadata; both include `Sqex::Misc::Utf8String`
     (player name / etc.).
   - `ZoneInitData` (3): zone-load payload.
3. ⏸ **Validate every `OP_*` in garlemald-server's `opcodes.rs`**
   against the binary's `LobbyProtoUp` / `ZoneProtoUp` /
   `ChatProtoUp` union members. The union member layout is
   recoverable via Ghidra's decompile of one of the
   `Component::Network::IpcChannel::PacketBufferTmpl<...>::dispatch`
   functions, but TBD as a tool. Names in the binary not in
   garlemald = gaps; names in garlemald not in the binary =
   server-side invented opcodes.
4. ✅ **Decompiled `LobbyCryptEngine`'s 9 vtable slots** —
   `tools/extract_crypt_engine.py` + `build/wire/<binary>.crypt_engine.md`.
   The cipher is statically-linked OpenSSL Blowfish (`BF_set_key` /
   `BF_encrypt` / `BF_decrypt` at RVAs `0x0005abf0`, `0x0005aac0`,
   `0x0005aa30`). The P/S init constants at VA `0x01267278`
   (P[18]) and `0x012672C0` (S[4][256]) are canonical pi-derived,
   confirmed bit-for-bit; garlemald-server's
   `common/src/blowfish_tables.rs` matches both byte-for-byte. The
   key schedule has one non-canonical quirk (`MOVSX` not `MOVZX`
   on each cycled key byte → keys with bytes ≥ 0x80 produce a
   different schedule than stock OpenSSL); garlemald reproduces
   this via `key[j] as i8 as i32 as u32`. Slot map: 0=dtor,
   1=PrepareHandshake (32-byte "Test Ticket Data..." seed +
   timestamp), 2/3/5=interface stubs, 4=SetSessionKey (16-byte
   key, allocates fresh BF_KEY at `[this+0x30]`), 6/7=Encrypt /
   Decrypt buffer in 32-byte chunks, 8=capability probe.
   **Buffer-length quirk**: slots 6/7 round length DOWN to a
   multiple of 32 (= 4 Blowfish blocks); garlemald's
   `encipher`/`decipher` require 8-aligned length, which is
   *stricter* — divergent payloads will silently leave trailing
   plaintext on the client side.
5. ⏸ **Decompile the `*ProtoChannel::ClientPacketBuilder` Encode /
   `RecvCallbackInterface` Decode slots** for one channel and
   document the exact endian / padding semantics. The
   `BasePacketHeader` field order is known; subpacket layout has
   more unknowns (`unknown_06`, `unknown_0a` in
   `garlemald-server/common/src/subpacket.rs`).

The GAM-params extraction (#2) is the biggest immediate win —
garlemald-server's actor-property system can now be type-checked
against the ground truth: every `SetActorPropertyPacket` carrying
`(id, value)` should have a `value` whose Rust type matches the
binary's recovered C++ type.

### Two parallel actor-property systems — discovered Phase 3

The binary has **two distinct actor-property wire systems** that are
easy to conflate. They serve different protocol layers and use
different wire-id schemes.

| System | Wire id | Where used |
|---|---|---|
| **GAM `CompileTimeParameter`** | small ordinal id (100, 116, 137, ...) per-`Data`-class namespace | Lobby protocol — `CharaMakeData`, `ClientSelectData`, `Player`, `PlayerPlayer`, `ZoneInitData`. Sent during char creation, char select, and zone init. |
| **`SetActorPropertyPacket` (0x0137)** | 32-bit Murmur2 hash of the property's `/`-path string (e.g. `"charaWork.parameterSave.hp[0]"` → `0xE14B0CA8`) | Zone protocol gameplay — every in-game state mutation (HP/MP, current quest sequence, equip slots, command bindings). |

Garlemald-server (`map-server/src/packets/send/actor.rs`) and
Project Meteor (`Map Server/Packets/Send/Actor/SetActorPropetyPacket.cs`)
both implement the second system — string-keyed builders
(`add_byte / add_short / add_int / add_float`) that hash the path
with Murmur2 to produce the wire id. The string keys themselves
(`"charaWork.parameterSave.hp[0]"`, `"playerWork.questScenario[0]"`)
are server-side conventions, not symbols stored in the binary.

The first system (GAM) is what the `tools/extract_gam_params.py`
extractor recovers. Its 192 parameters are the lobby-protocol
schema for the five Data classes — useful as a type-check for
garlemald-server's `lobby-server/src/data/chara_info.rs` parser,
but **not** for `SetActorPropertyPacket`. The two systems are
parallel and independent.

The auto-generated `include/net/gam_registry.h` declares the GAM
schema as `constexpr` C++; future Rust code can use this header
(via FFI or build.rs codegen) as the source-of-truth for
lobby-side type checking.

### About the `PARAMNAME_*` symbols (resolved 2026-04-30)

Initial Phase-3 inspection found only generic
`IntData.Value0`/`StringData.Value0`/etc. placeholders in `.rdata`,
which led to an incorrect conclusion that the GAM ids were anonymous.

The real property names are recovered by walking each Data class's
**MetadataProvider dispatcher** — a vtable slot containing a 26/92/N-way
unrolled jump table that maps `id → const char*` lookups in `.data`.
`tools/extract_paramnames_dispatch.py` walks the dispatcher's asm,
extracts the `PUSH <imm32>` immediates that land in `.data`, and
dereferences each. Currently resolved:

| Data class | dispatcher RVA | ids resolved |
|---|---:|---:|
| `CharaMakeData` | 0x001ad010 | 26 / 26 |
| `Player` | 0x001add90 | 92 / 92 |
| `ClientSelectData` | TBD | 0 / 17 |
| `ClientSelectDataN` | TBD | 0 / 17 |
| `PlayerPlayer` | TBD | 0 / 37 |
| `ZoneInitData` | TBD | 0 / 3 |

Results land in `config/<binary>.gam_params.json` (the existing
GAM registry, enriched in-place with a `paramname` field per entry)
and in the auto-generated `include/net/gam_registry.h`. Sample names
recovered for the `Player` class:

```
135 craft_assist_buff_type     159 guildleveSeed (bool[4096])
136 craft_assist_buff_level    160 guildleveFaction
144 guildleveId                166 event_achieve_aetheryte
148 guildleveBoostPoint        191 latest_aetheryte
149 guildleveMark              202 anima
150 guildleveRewardItem        211 companyId
153 guildleveRewardSubItem     212 companyMemberRank
155 guildleveRewardSubNumber   228 craftMakingRecipeHistory
156 guildleveBonusRewardStock  230 favoriteAetheryte
```

**Important caveat**: these are LOBBY-protocol property names, not
the same naming convention as Project Meteor's
`SetActorPropertyPacket`-side `playerWork.*` / `charaWork.*`
strings. The two systems are still parallel (see "Two parallel
actor-property systems" above), but each one's properties are now
named — GAM via dispatcher walk, SetActorProperty via
project-meteor-server's C# class names hashed through Murmur2.

The `IntData.Value0` placeholder strings ARE in `.rdata` — they're
referenced by *other* GAM-related code paths (probably debug
formatting), separate from the dispatcher's per-id name table.

## Cross-references in the workspace

- `garlemald-server/common/src/packet.rs` — BasePacketHeader struct.
- `garlemald-server/common/src/subpacket.rs` — SubPacket layout.
- `garlemald-server/common/src/blowfish.rs` — current Blowfish impl.
- `garlemald-server/common/src/blowfish_tables.rs` — P-array init.
- `garlemald-server/map-server/src/packets/opcodes.rs` — opcode list.
- `project-meteor-server/FFXIVClassic.Common/Packets/` — C# packet
  base classes.
- `project-meteor-server/FFXIVClassic Map Server/packets/` — C#
  per-opcode classes (map-server side).
- `ffxiv_classic_wiki_context.md` — "Game Opcodes" section for the
  community-known opcode list.
- `ffxiv_linkchannel_context.md` — FFXIV 1.0 Opcodes spreadsheet
  (more complete than the wiki, mined from Project Meteor Discord).
- `project_meteor_discord_context.md` — first-hand notes on packet
  field layouts.
- `meteor-decomp/config/ffxivgame.rtti.json` — every recovered
  vtable, including the 79 `Component::Network` + 57
  `Application::Network` ones.
- `meteor-decomp/config/ffxivgame.vtable_slots.jsonl` — function
  pointers per vtable; this is the bridge from RTTI class →
  function RVA → asm.
- `meteor-decomp/config/ffxivgame.strings.json` — 343 `PARAMNAME_*`
  mangled names (filter by `kind: "string"` and `value.contains
  "PARAMNAME"`), 491 lower-level strings hinting at field names
  (`loginCount`, `loginFlag`, `name.fullname`, etc.).

## Transport / session layer — call-site grounding (ffxivDecomp, 2026-05-28)

> Sourced from **ffxivDecomp** (github.com/Yokimitsuro/ffxivDecomp), an independent
> docs-only RE of FFXIV 1.23b `ffxivgame.exe`, used with permission (see `NOTICE.md`).
> **Cross-referenced, not byte-verified** by meteor-decomp's own decompilation —
> confirm against the asm before relying on offsets for a code change.
> Captured 2026-05-30 from the ffxivDecomp 2026-05-27/28 session.

The "Transport is RUDP2" line in the TL;DR above is the **RTTI-and-vtable**
view — the `Sqex::Socket::RUDP2::*Segment` classes are real and present. The
ffxivDecomp session adds the **call-site-grounded** picture, which refines
(does *not* contradict) it: at the bottom of the stack the client drives a
**plain Winsock `select()` loop over a generic `Socket` wrapper class**, and
the RUDP2 *segments* are framed by code sitting **above** that generic
socket. So the live socket is just a TCP/UDP descriptor with stats counters;
"RUDP2" is a framing/reliability layer layered on top, not a property of the
socket object itself.

### The generic Socket layer (Winsock select-driven)

(Source: `finding_net_event_loop.md`.) The Socket class has a **vtable at
`0x01113340`** (slot `+0x0c` = `on_error` / `on_close`). The thin Winsock
shims are tiny — they pull a `SOCKET` from `Socket+0x04`, call the import,
and normalise `WSAEWOULDBLOCK` (`0x2733` = 10035) to "no data":

| Function (RVA) | Role | Winsock import |
|---|---|---|
| `FUN_00d43140` | `Socket_recv_thin` — recv into buffer, `WSAEWOULDBLOCK → -2` | `recv` |
| `FUN_00d430d0` | `Socket_send_thin` — send, `WSAEWOULDBLOCK → 0` | `send` |
| `FUN_00d57530` | `NetIo_SelectWait` — the I/O loop's wait | `select` |
| `FUN_00d42d90` / `FUN_00d43060` / `FUN_00d432a0` | socket / connect / WSAStartup shims | `socket` / `connect` / `WSAStartup` |

The event loop is `FUN_00d514f0` (one tick): `select()` → on ready,
`FUN_00d511c0` walks a vector of 12-byte ready-descriptor entries and calls
`FUN_00d44ae0(sock, r, w, x)`, the per-socket state machine. **Socket state
field is at `Socket+0x98`** (atomic), with these values:

| State | Meaning | recv path |
|---|---|---|
| 2 | `LISTENING_OR_ACCEPTING` | accept-path (`FUN_00d44370`) |
| 3 | `CONNECTING` | connect-complete (`FUN_00d438e0`) |
| 4 | `CONNECTED_TCP` | `FUN_00d447e0` → `recv()` |
| 5 | `CONNECTED_UDP` | `FUN_00d44950` → `recvfrom()` |

The same Socket class therefore has **both a TCP recv path (state 4) and a
UDP recvfrom path (state 5)** — consistent with RUDP2 running over UDP but
the launcher's `ws2_32` shim tunnelling over TCP (state 4). Raw bytes reach
user code via a per-socket callback installed at **`Socket+0x38`**
(`on_recv_tcp(ctx, buf, len)`; the UDP twin at `+0x3c` also gets a
`sockaddr*`), with the channel object installed as the context at
`Socket+0x90`. **Framing happens in that callback, not in the recv worker**
— the client hands its parser arbitrary-length `recv()` chunks, so frames
may arrive split or coalesced (the per-channel `PacketBufferBase` reassembles
them; see "IPC channel framing" below).

This grounds the existing TL;DR caveat: a TCP-via-shim server (what
`garlemald-server` and Project Meteor implement) hits the **state-4** path
cleanly; a native RUDP2-over-UDP server would drive **state 5**.

### IPC channel framing sits above the socket

(Source: `finding_ipc_channel_framing.md`.) The three channels
(`Lobby` / `Zone` / `Chat`) are **independent TCP connections**, each owning
its own `PacketBufferTmpl<TXxxProtoDown>` (recv) and `…Up` (send). They are
**not multiplexed onto one socket** — so a compatible server must serve three
separate connections per session, and **opcode spaces are per-channel** (the
same opcode value means different things on Lobby vs Zone vs Chat). The
boundary between byte-stream and typed packet is `tryGetNextPacket`
(`FUN_00db6140` base; `FUN_00db6d20` typed); per-packet dispatch is virtual
through the primary processor at `PacketBufferBase+0x08` (vtable `+0x14`),
with an optional secondary processor at `+0x78` (vtable `+0x20`) that matches
the `Lua::Script::Client::Group::PacketProcessor` RTTI — i.e. the Lua bridge,
consulted only after the primary processor runs.

### Segment-level (transport) opcode roster

These are the **RUDP2 segment / session opcodes**, distinct from the
game-message opcodes in the IPC payload. The zone session sub-tick
(`FUN_004e20a0` / `ZoneClient_mainLoopTick`, driven from the master tick
below) emits the outbound ones; `FUN_004dc690`
(`Zone_MAIN_inbound_opcode_dispatcher`) handles the inbound ones in its
**low-opcode session range** (cf. `finding_outbound_complete_lobby_zone_chat.md`
and `finding_spawn_wire_side_…0x17c…md`).

Outbound (client → server, session/transport level):

| Op | Size | Role |
|---|---|---|
| `0x01` | — | latency ping (≈ every 1 s) |
| `0x02` | 56B | handshake (carries the version constant; see below) |
| `0x03` | 560B | large-state push |
| `0x04` | — | disconnect-ack |
| `0x06` | 24B | heartbeat |

Inbound (server → client, session/transport level) recognised by the
dispatcher's low-opcode arm: `0x01`, `0x02`, `0x0E`, `0x11` (plus the
`0x08`/`0x09`/`0x0A`/`0x0B` bulk-state-push 1/16/32/64 size variants).

> **SEQ-005 disambiguation — read this before touching the zone-in handshake.**
> ffxivDecomp's `FUN_004dc690` decodes a **DOWN-side** (server→client) case
> `0x07` as a *resync* handshake ("resync loop"). This is **NOT** the same
> thing as the **UP-side** (client→server) `0x0007` zone-in-complete that
> garlemald fails to send in the SEQ-005 "Now Loading" hang (see
> `MEMORY.md` → *garlemald SEQ_005 Now Loading hang*). One is a server-driven
> down-side resync; the other is a client-driven up-side zone-in-complete
> acknowledgement. They share the byte value `0x07` but live on opposite
> directions of opposite-layer dispatchers — do not conflate them when
> diagnosing the warp-completion gap.

### Master tick that owns Lobby + Zone

(Source: `finding_network_client_module.md`.) `NetworkClientModule::tick`
(`FUN_004e30a0`) is the singleton 6-state master state machine (state field
at `+0x250`) that owns both the `LobbyClient` (`+0x240`) and the `ZoneClient`
(`+0x234`) and drives the lobby→zone handoff:

| State | Meaning |
|---|---|
| 0 | IDLE / connect-requested — allocates `LobbyClient` (0x4A8 bytes), wires + starts TCP |
| 1 | LOBBY ACTIVE — runs the 4-phase login sub-tick (`FUN_004e2d00`) |
| 2 / 3 | transient → state 4 (lobby torn down, switch to zone) |
| 4 | ZONE STEADY — `ZoneClient_mainLoopTick` (`FUN_004e20a0`); emits the session opcodes above |
| 5 | LOBBY CLEANUP COMPLETE — finishes any leftover teardown; keeps ticking the zone |

The lobby login sub-tick advances through `0x1f5` (lobby login) → `0x05`
(service login) → `0x06` (game login) → world-server info, then the
`RaptureLobbyCallback::switchToWorld` slot instantiates the
`ZoneProtoChannel::ServiceConsumerConnectionManager` and raises the
"switch to zone" flag at `+0x24c`.

### Version constants a server must accept

(Source: `finding_outbound_complete_lobby_zone_chat.md`; corroborated by
`finding_client_version_identification.md` for build provenance.)

| Constant | Decimal | Where | Field |
|---|---:|---|---|
| `0x3C6B` | 15467 | Zone + Chat handshake (`0x02`) | version word at `+0x00` |
| `0x6E` | 110 | Lobby `0x05` ServiceLogin / `0x06` GameLogin | `clientVer1` (1 byte @ `+0x0a`) |
| `0x1347` | 4935 | Lobby `0x05` / `0x06` | `clientVer2` (2 bytes @ `+0x0c`) |
| `0x14C` | 332 | — | sub-proto threshold |

A version-tolerant test server must accept these exact values **or** relax
the version check. (Provenance note: the binary self-identifies as the FFXIV
1.x Crystal Tools-era unified client — `CDev.Engine.Dw.RenderInterface`
banner `build at Sep 5 2012`, between 1.23a and 1.23b — consistent with the
1.23b target but not proven from a literal in-binary `1.23b` string; see
`finding_client_version_identification.md`.)

### Spawn / actor replication is layered above all of this

(Source: `finding_spawn_wire_side_CLOSED_opcode_0x17c_zone_main_inbound_dispatcher.md`.)
For completeness on where game-message opcodes sit relative to the transport:
the Zone inbound dispatcher `FUN_004dc690` also covers a **high-opcode
game-protocol range** (`0x143`–`0x1a8`, 40+ cases) in addition to the
session arm above. Wire opcode **`0x17c`** (380) is the SPAWN packet — it
routes through `FUN_00576250` → `FUN_006cc620` → the
`SpawnPipeline_FACTORY` (`FUN_006cc070`), keyed by a TYPE_TAG at payload
`+0x10` (`0` = `EntryBuilder`/spawn, `0xe` = `OnlineStatusUpdater`) and a
null-terminated **class-name string at payload `+0x44`**, with the `0x2711`
(10001) list-object signature gating the notification chain. The client
ACKs a spawn with **2× `0x130` + 1× `0x133`**. These are the *Group::* typed
replication packets, well above the segment/session layer documented in this
section.
