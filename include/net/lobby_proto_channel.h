// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// LobbyProtoChannel wire framing — recovered from ffxivgame.exe.
//
// Decoded by reading the four `LobbyProtoChannel::ClientPacketBuilder`
// vtable slots (RVAs 0x009a2cb0 / 0x009a2ac0 / 0x009a2ad0 / 0x009a2b00)
// + the `LobbyProtoDownCallbackInterface` slot 1 dispatcher
// (RVA 0x009a4160) + the `LobbyCryptEngine` 9 slots already documented
// in `build/wire/<binary>.crypt_engine.md`.
//
// Three siblings exist in the binary, all sharing the 4-slot CPB shape:
// `LobbyProtoChannel::ClientPacketBuilder`,
// `ZoneProtoChannel::ClientPacketBuilder`,
// `ChatProtoChannel::ClientPacketBuilder`. The slot-2 `BuildHeader`
// implementations are byte-for-byte identical EXCEPT for the
// `timestamp` field at byte 8 (Lobby/Zone call `_time64(NULL)`; Chat
// hardcodes `0x0A`). Slot-3 `Send` is also identical in shape, calling
// per-channel dispatch helpers that — in this build — are
// `MOV EAX, [ESP+N]; RET` no-op stubs returning one of their args.
// The actual send path therefore lives outside this 4-slot interface
// (likely the surrounding RUDP2 / IpcChannel framework).
//
// IMPORTANT: this header is HAND-WRITTEN, not auto-generated. Treat
// the named offsets/sizes as the wire spec; the field NAMES below
// reflect what the binary actually encodes (bytes 0..3 are constant
// `0x14, 0x00, conn_type_lo, conn_type_hi`, etc.). See the comments
// for the reconciliation with `garlemald-server/common/src/{packet,
// subpacket}.rs`.

#pragma once

#include <cstddef>
#include <cstdint>

namespace meteor_decomp::net::lobby_proto_channel {

// =====================================================================
// BasePacketHeader — 16 bytes, written by CPB::BuildHeader (slot 2).
// =====================================================================
//
// Slot 2 (`FUN_00da2ad0`, 46 bytes) executes:
//
//   AND  [out],     0xFFFFFF14   ; clear bits 0,1,3,5,6,7 of byte 0
//   OR   [out],     0x14         ; set bits 2,4 of byte 0  → byte 0 = 0x14
//   MOV  [out+1],   0            ; byte 1 = 0
//   MOV  CX,        [this+0x1c]
//   MOV  [out+2],   CX           ; bytes 2..3 = (u16) this->captured_conn_type
//   PUSH 0
//   CALL _time64                  ; EAX = (u32) current Unix time low half
//   MOV  [out+8],   EAX          ; bytes 8..11 = timestamp low 32 bits
//
// Bytes 4..7 and 12..15 are NOT written by BuildHeader. The caller
// must populate them (or they remain whatever was in the buffer).
// In garlemald-server's view those four-byte regions are
// `packet_size: u16 + num_subpackets: u16` (bytes 4..7) and the high
// half of `timestamp: u64` (bytes 12..15). The high half of the
// timestamp is therefore typically zero on the wire from this client.
//
// The Chat channel's slot-2 (`FUN_00e409e0`, 39 bytes) writes byte 8
// as the literal constant `0x0A` instead of calling _time64. So
// chat-channel headers carry `timestamp = 0x0A` always.
struct BasePacketHeader {
    // The binary unconditionally writes `0x14`. Garlemald-server reads
    // this as `is_authenticated: u8`, treating any non-zero as
    // "authenticated". Effectively a constant marker (`0x14` =
    // 0b00010100 — bits 2 and 4) that may carry version/flags
    // semantics in older or newer protocol revisions.
    std::uint8_t magic_or_flags;        // = 0x14
    // The binary unconditionally writes `0x00`. Garlemald reads this
    // as `is_compressed: u8`. No traffic observed with this set.
    std::uint8_t reserved;               // = 0x00
    // Captured at CPB construction time at `this+0x1c`. Identifies
    // which channel's CPB built this packet; values are channel-
    // specific (Lobby / Zone / Chat). garlemald-server names this
    // `connection_type: u16`.
    std::uint16_t connection_type;
    // NOT written by BuildHeader. Caller-populated. garlemald names
    // this `packet_size: u16` (total bytes of this BasePacket
    // including the 16-byte header).
    std::uint16_t packet_size;
    // NOT written by BuildHeader. Caller-populated. garlemald names
    // this `num_subpackets: u16`.
    std::uint16_t num_subpackets;
    // The Lobby/Zone CPBs write the LOW 32 bits of `_time64(NULL)`
    // here (4-byte u32 store, NOT 8-byte). The Chat CPB writes the
    // literal `0x0A`. The high 32 bits of garlemald's u64 timestamp
    // (bytes 12..15) are therefore not written by this client and
    // are typically zero.
    std::uint32_t timestamp_lo;
    std::uint32_t timestamp_hi_unused;   // typically 0
};
static_assert(sizeof(BasePacketHeader) == 16,
              "BasePacketHeader must be 16 bytes");

// =====================================================================
// SubPacketHeader — 16 bytes (cross-validated against garlemald-
// server's `common/src/subpacket.rs`, no divergences observed).
// =====================================================================
//
// The CPB framework writes one BasePacketHeader followed by one or
// more SubPackets. Each SubPacket starts with a 16-byte
// SubPacketHeader followed by a body. Specific SubPacket types embed
// further nested structures (e.g. type 0x03 GameMessage prepends a
// 16-byte GameMessageHeader to its body before the actual payload).
struct SubPacketHeader {
    std::uint16_t subpacket_size;        // total bytes of this subpacket (header + body)
    std::uint16_t type;                  // 0x03 = GameMessage; others observed but undocumented
    std::uint32_t source_id;             // sender entity id
    std::uint32_t target_id;             // recipient entity id
    std::uint32_t unknown_0c;            // garlemald: `unknown1`
};
static_assert(sizeof(SubPacketHeader) == 16,
              "SubPacketHeader must be 16 bytes");

// =====================================================================
// GameMessageHeader — 16 bytes, present only in type-0x03 SubPackets.
// =====================================================================
struct GameMessageHeader {
    std::uint16_t unknown_00;            // garlemald: `unknown4`
    std::uint16_t opcode;                // identifies the inner message
    std::uint32_t unknown_04;            // garlemald: `unknown5`
    std::uint32_t timestamp;
    std::uint32_t unknown_0c;            // garlemald: `unknown6`
};
static_assert(sizeof(GameMessageHeader) == 16,
              "GameMessageHeader must be 16 bytes");

// =====================================================================
// ClientPacketBuilder — 4-slot per-channel encoder vtable.
// =====================================================================
//
// All three channels (Lobby / Zone / Chat) share this 4-slot shape.
// The CPB owns a packet buffer at `this[0x10..]` (the body) and
// captures a `connection_type` u16 at `this[0x1c]` at construction.
//
// vtable[0] = ~ClientPacketBuilder() — scalar deleting destructor.
// vtable[1] = uint8_t* Begin() — returns &this[0x10] (write pointer).
// vtable[2] = void BuildHeader(BasePacketHeader* out) — see semantics
//             documented above.
// vtable[3] = void Send(uint8_t* buffer, size_t total_len) — splits
//             into header (first 16 bytes) + payload (rest). In this
//             build, the per-channel dispatch helpers it invokes are
//             no-op stubs returning one of their args; the actual
//             outbound path lives elsewhere (likely the RUDP2 layer
//             owned by the surrounding ProtoChannel object).
//
// Inheritance: extends an abstract `*ProtoChannel::ClientPacketBuilder`
// base whose vtable address is at .rdata 0x011274cc-ish per the
// reconstruction; the dtor at slot 0 sets `[this] = base_vtable`
// then calls a sub-dtor on `[this+8]` before freeing.
struct ClientPacketBuilder;  // opaque; concrete layout TBD

// =====================================================================
// LobbyProtoDownCallbackInterface slot 1 — incoming dispatch entry.
// =====================================================================
//
// Slot 1 (`FUN_00da4160`, 319 bytes) is the lobby's incoming packet
// dispatcher. Prologue:
//
//   MOV EDX, [ESP+8]         ; arg1 = recv-context*
//   MOV EAX, [EDX+8]         ; ctx->[+8] = a "current packet" descriptor
//   MOV EAX, [EAX+0x24]      ; descriptor->[+0x24] = pointer into the byte buffer
//   MOVZX ESI, word [EAX+2]  ; dispatch key = u16 at offset 2
//   CMP ESI, 0x1F4
//   JG handle_above_500
//   JZ handle_500
//   SUB ESI, 1
//   CMP ESI, 0x16
//   JA invalid
//   MOVZX ESI, byte [ESI + 0xDA42CC]    ; byte_table[key-1] → case index
//   JMP dword [ESI*4 + 0xDA42A0]         ; dword_table[case] → handler
//
// This is the same shape as the well-known Down opcode dispatcher
// already extracted by `tools/extract_opcode_dispatch.py` for the
// Zone/Chat channels (see `build/wire/<binary>.opcodes.md`).
// Coverage on the lobby-key range:
//   - keys 1..23 routed via the byte_table at .data 0x012a42cc
//   - key 0x1F4 (= 500) special-cased
//   - keys > 500 dispatched in a separate range past `handle_above_500`
//
// The exact semantic of the dispatch key (subpacket type vs game-
// message opcode vs something else) needs cross-reference at the
// caller; the +0x24 offset points part-way through the chain
// `BasePacket → SubPacket → GameMessage`, so it's likely the inner
// GameMessage opcode (matching `SubPacket.type == 0x03` payloads).
extern const std::uintptr_t LOBBY_DOWN_DISPATCH_RVA;       // = 0x009a4160
extern const std::uintptr_t LOBBY_DOWN_DISPATCH_BYTE_TABLE; // = 0x012a42cc (.data RVA)
extern const std::uintptr_t LOBBY_DOWN_DISPATCH_DWORD_TABLE;// = 0x012a42a0 (.data RVA)

// =====================================================================
// LobbyCryptEngine — 9-slot Blowfish wrapper.
// =====================================================================
//
// Documented in detail in `build/wire/<binary>.crypt_engine.md`. The
// concrete-class slot map is fixed (only one subclass exists in this
// build); zone and chat traffic is NOT encrypted (no concrete
// CryptEngine subclass for those channels in the binary; garlemald-
// server's world-server and map-server contain no Blowfish call sites
// either, confirming the absence is intentional).
//
// Slot layout:
//   0  ~LobbyCryptEngine        (frees [this+0x30] = BF_KEY*)
//   1  PrepareHandshake         (32-byte "Test Ticket Data..." seed
//                                + (u32) time(NULL) into req+0x34/0x74)
//   2  GetExtendedFlag          (3-arg stub, returns 0)
//   3  Verify-A                 (2-arg stub, returns false)
//   4  SetSessionKey            (BF_set_key with 16-byte key)
//   5  Verify-B                 (2-arg stub, returns false)
//   6  Encrypt(_, buf, len)     (in-place ECB on len-rounded-down-to-32)
//   7  Decrypt(_, buf, len)     (in-place ECB on len-rounded-down-to-32)
//   8  GetCompatibility         (1-arg stub, returns true)
//
// =====================================================================
// 32-byte alignment quirk — RESOLVED (benign in practice).
// =====================================================================
//
// Slots 6 and 7 round the input length DOWN to a multiple of 32:
//
//   MOVZX EAX, word [ESP+0xc]
//   AND   EAX, 0xFFFFFFE0   ; = round_down(len, 32)
//
// 32 = 4 Blowfish blocks. The trailing 0..31 bytes of every encrypt /
// decrypt call are silently passed through as plaintext.
//
// Garlemald-server's `Blowfish::encipher`/`decipher` (in
// `common/src/blowfish.rs`) require 8-aligned lengths and encrypt
// ALL of `len`. The two policies are NOT equivalent — but the gap
// is BENIGN in this protocol because of how the lobby builds packets:
//
//   1. Project Meteor's reference C# server (`Lobby Server/Packets/
//      Send/AccountListPacket.cs`, `SelectCharacterConfirmPacket.cs`,
//      etc.) and garlemald-server BOTH compute the encryption body
//      as `subpacketSize - 16` and encrypt it with an 8-aligned
//      Blowfish. Neither rounds to 32. Project Meteor has been
//      working against the actual 1.x client for years.
//
//   2. Both servers allocate FIXED-size lobby buffers (`MemoryStream
//      (0x98)`, `MemoryStream(0x210)`, `MemoryStream(0x280)`, etc. in
//      C#; identical `CAPACITY` constants in garlemald). The actual
//      meaningful content typically fills only the first N bytes of
//      these buffers; the trailing region is **zero padding** that
//      `MemoryStream.GetBuffer()` returns wholesale.
//
//   3. The mismatch therefore plays out as: server encrypts the full
//      `subpacketSize - 16` bytes; client decrypts only
//      `floor((subpacketSize - 16) / 32) * 32` of them. The trailing
//      0..31 bytes the client fails to decrypt fall inside the
//      buffer's trailing zero-padding region, where no meaningful
//      data lives. The client's parser doesn't read past the field
//      boundaries it cares about, so the garbled trailing bytes are
//      invisible.
//
// Worked example — `SelectCharacterConfirm` (opcode 0x0F):
//
//   - Buffer `data` is exactly 0x98 = 152 bytes (filled completely
//     with: 8B sequence, 4B characterId×2, 4B zero, 64B sessionToken
//     padded, 2B reserved, 2B worldPort, 32B worldIp padded, 32B
//     unknownIp padded). The trailing region of `unknownIp` is
//     zero-padding ASCII nulls.
//   - subpacketSize = 32 + 152 = 184 bytes (0xB8).
//   - Encryption body = 184 - 16 = 168 bytes. 168 mod 32 = 8.
//   - Server encrypts all 168 bytes; client decrypts the floor →
//     160 bytes; last 8 bytes left encrypted.
//   - Those 8 bytes correspond to data[144..152], i.e. the last
//     8 bytes of the 32-byte `unknownIp` field — entirely zero
//     padding. The client never displays / parses past the
//     meaningful "192.168.0.44" prefix.
//
// Worked example — `AccountList` (opcode 0x0C, `MAX=8` per packet):
//
//   - Buffer `data` is 0x280 = 640 bytes; meaningful content =
//     16-byte list header + 8 × 72-byte entries = 592 bytes.
//     Trailing 48 bytes are zero.
//   - subpacketSize = 32 + 640 = 672 bytes. Encryption body = 656.
//     656 mod 32 = 16. Last 16 of the 656 are not decrypted by
//     client. Those 16 fall within the 48-byte trailing zero pad —
//     never read by the client's account list parser.
//
// Conclusion: garlemald's `encipher`/`decipher` are CORRECT as
// written. The 8-aligned check is appropriate (matches Project
// Meteor; would break interop only if a packet had non-padding
// content in its trailing 0..31 bytes — which the protocol's
// over-provisioned buffers preclude). No code change needed; this
// is a documentation-only finding.
//
// If a future packet builder packs meaningful data into the LAST
// 32 bytes of a non-32-aligned encryption body, the client would
// silently see garbage there. Tag any such builder for review with
// a comment referencing this section.

}  // namespace meteor_decomp::net::lobby_proto_channel
