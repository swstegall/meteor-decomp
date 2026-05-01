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

1. **Validate every `OP_*` in `opcodes.rs`** against the binary's
   recovered union members. Run `tools/extract_net_vtables.py`
   (TODO) to dump every member of `LobbyProtoUp`, `LobbyProtoDown`,
   `ZoneProtoUp`, `ZoneProtoDown`, `ChatProtoUp`, `ChatProtoDown`.
   Names that exist in the binary but not in garlemald are gaps.
2. **Decompile `LobbyCryptEngine::Init`** (or the equivalent zone
   variant). Currently `garlemald-server/common/src/blowfish.rs`
   has a hand-derived key schedule; we want to confirm it matches
   the binary's, especially the SqexId-token-derived key.
3. **Decompile the `PacketBufferTmpl::Encode` / `::Decode` slot
   pair** for one channel and document the exact endian / padding
   semantics. The `BasePacketHeader` field order is known but the
   subpacket layout has more unknowns (`unknown_06`, `unknown_0a`,
   etc.).
4. **Map the GAM `CompileTimeParameter<id, &PARAMNAME_id, ...>`
   template instantiations** (343 found in `.rdata`) to Project
   Meteor's `playerWork.*` / `actor.*` property names. Each
   `PARAMNAME_N` is a `const char*` symbol — its target string
   (e.g. `playerWork.activeQuest`) tells us what wire ID `N` means.

Deliverables 1 and 4 don't need a matching toolchain — they're pure
documentation work and unblock garlemald-server immediately.
Deliverables 2 and 3 land cleaner with matching, but functional
re-derivation is still useful.

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
