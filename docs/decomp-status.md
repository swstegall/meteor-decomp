# Decompilation status — narrative recap

A running record of what's been recovered from `ffxivgame.exe`, what's
been validated against `garlemald-server` (the Rust port), and where
the open questions are. Last update: 2026-05-01.

For the strategic plan and exit criteria, see [PLAN.md](../PLAN.md).
For per-subsystem detail, see the auto-generated reports under
`build/wire/<binary>.*.md` (regenerable via `make`).

## Phase 0 — bootstrap (✅)

`make bootstrap` symlinks the workspace's installed retail binaries
into `orig/` and dumps PE structure as a sanity check. Captured by
`tools/extract_pe.py` → `build/pe/<bin>.json`.

The five binaries:

| Binary | Size | What it is |
|---|---|---|
| `ffxivgame.exe` | ~12 MB | The main game executable. Renderer + gameplay + lobby + zone + chat clients all in one PE. Built with MSVC linker 8.0 (VS 2005 SP1), statically links Miles MSSMIXER, OpenSSL 1.0.0, and the SE-internal Sqex / CDev / Sqwt frameworks. ImageBase `0x00400000`. **The primary target.** |
| `ffxivlogin.exe` | small | Patcher / login bootstrapper. Sanity-check target for the Phase 1 pipeline. |
| `ffxivboot.exe`, `ffxivupdater.exe`, `ffxivconfig.exe` | small | Auxiliary launcher / patcher / settings. Lower priority. |

## Phase 1 — Ghidra import + work pool (✅)

`make split BINARY=ffxivgame.exe` runs Ghidra 12 + JDK 21 (post-Jython
era; the analysis scripts are Java post-scripts under
`tools/ghidra_scripts/`) and produces:

- **9,729 vtable slots** across **576 net-relevant classes** in the
  RTTI dump (`config/<bin>.rtti.json`,
  `config/<bin>.vtable_slots.jsonl`, `build/wire/<bin>.net_handlers.md`).
- One `asm/<bin>/<rva>_<symbol>.s` file per function, full disassembly
  with annotations.
- A work-pool YAML (`config/<bin>.yaml`) listing every function with
  size, section, and any seed-hint flags (`__FILE__` / `__FUNCTION__`
  baked-in strings, Lua callback names, etc.).

Notable named classes recovered:
- The three `*ProtoChannel` IpcChannel families (Lobby / Zone / Chat)
  + their `ServiceConsumerConnectionManager` + `ConsumerConnection`
  + the per-channel `ClientPacketBuilder` (4 slots).
- `LobbyCryptEngine` (9 slots — the cipher API surface).
- `MyGameLoginCallback` (22 slots — the login state machine).
- `Sqex::Crypt::{Cert, Crc32, ShuffleString, SimpleString, CryptInterface}`
  — SE's higher-level crypto shims.
- `Sqex::Socket::RUDP2`, `RUDPSocket`, `PollerWinsock`, `PollerImpl`
  — RUDP2 transport stack.

## Phase 2 — matching toolchain (🟡 partial)

The goal is byte-identical recompilation: take the binary's matched
function, hand-write equivalent C++, compile with the original
toolchain, and confirm `objdiff` reports zero delta. This proves the
hand-written C++ is semantically and bit-exactly the source.

What works today:
- **Detection**: `tools/setup-msvc.sh` confirms the toolchain layout
  is VS 2005 SP1 (linker 8.0, `cl.exe` version banner matches).
- **CrossOver Wine 9 on Apple Silicon**: `tools/cl-wine.sh` runs
  `cl.exe` under wow64 mode (no `WINEARCH=win32`) with the
  `DYLD_FALLBACK_LIBRARY_PATH` shim that the workspace's install env
  needs. `make rosetta` produces real byte-level diffs.
- **Rosetta candidate staged**: `FUN_00b361b0` — 86 bytes, an
  unrolled 32-byte block-copy loop with no calls and no FP. Picked
  by `find_rosetta.py` as easiest-first.

What's blocked:
- Iterating `MSVC_FLAGS` to drive the Rosetta delta to zero needs
  the **Platform SDK 2003 R2** (libcmt MUMSI variant headers and
  the SE-style preprocessor flags). VS 2005 SP1 alone produces
  diffs that are close but not identical. Procurement of a legal
  copy is pending.

When Phase 2 unblocks, the strategy in `PLAN.md` is to use the
matched Rosetta as a calibrated `MSVC_FLAGS` reference, then expand
to functional `.cpp` files in `src/ffxivgame/net/` (incremental
porting of named classes).

## Phase 3 — functional decomp (🟢 substantial)

Phase 3 is producing wire-level ground truth via static analysis +
cross-validation against `garlemald-server` and (where it agrees)
`project-meteor-server`. Each subsystem below has a tool that
emits a regenerable Markdown report under `build/wire/`.

### 3.1 — GAM property registry (✅ 192/192 names)

The GAM (Game Attribute Manager) `Component::GAM::CompileTimeParameter
<id, &PARAMNAME_id, T, Decorator>` template instantiations form a
compile-time, type-safe property registry. Mangled into `.rdata`
strings. `tools/extract_gam_params.py` parses 192 unique
`(id, namespace, type, decorator)` tuples across **6 Data classes**:

| Data class | Count | ID range | Purpose |
|---|---|---|---|
| `CharaMakeData` | 26 | 100..125 | New-character-creation request body |
| `Player` | 92 | 135..233 | Per-character persistent state |
| `PlayerPlayer` | 37 | 203, 321..345, 579..595 | Inner persistent state (uses dual-bound dispatcher) |
| `ClientSelectData` | 17 | 100..119 | Character-list-display schema |
| `ClientSelectDataN` | 17 | 100..116 | Renumbered alternate of ClientSelectData |
| `ZoneInitData` | 3 | 100..102 | Zone-load payload |

Output:
- `config/<bin>.gam_params.{json,csv}` — machine-readable.
- `build/wire/<bin>.gam_params.md` — human-readable.
- `include/net/gam_registry.h` — C++ header (auto-generated by
  `emit_gam_header.py`) for direct `#include` from
  `garlemald-server` / future garlemald-client.

### 3.2 — PARAMNAME dispatcher walkers (✅ 192/192 string names)

Ghidra's auto-analysis doesn't create symbols for the per-class
`PARAMNAME_<id>` strings — they're inlined into the
`MetadataProvider::vtable[2]` dispatcher's jump table.
`tools/extract_paramnames_dispatch.py` walks the dispatcher's
prologue (`ADD EAX, -<base>; CMP EAX, <count-1>; JA default;
JMP [EAX*4 + JT]`), then for each case body, extracts the `PUSH
<imm32>` immediate that lands in `.data` and dereferences the
C string there.

Two dispatcher kinds discovered:
- `global_id` (CharaMakeData, Player, ClientSelectData,
  ClientSelectDataN, ZoneInitData, PlayerPlayer slot 2):
  prologue normalizes the global GAM id; K-th `.data` PUSH = K-th
  sorted GAM id. Pattern works directly.
- `local_index` (PlayerPlayer slot 4 — earlier misdirection):
  prologue is just `CMP EAX, <count-1>; JMP [EAX*4 + JT]` with no
  `ADD`. Names extractable by local index but NOT pairable to GAM
  ids without decompiling the global→local translator. The
  CANONICAL global-id dispatcher for PlayerPlayer is slot 2 (RVA
  `0x001aee30`) with a dual-bound prologue.

Pattern documented in
`memory/reference_meteor_decomp_paramname_dispatcher.md` for
reapplication.

### 3.3 — Down opcode → handler map (✅ 211 opcodes, 3 channels)

`tools/extract_opcode_dispatch.py` walks each Down channel's
dispatcher (slot 1 of `*ProtoDownDummyCallback::vtable`, the
`MOVZX/CMP/byte_table/dword_table` jump). Output:
`build/wire/<bin>.opcodes.md`.

| Channel | Real opcodes | Total possible |
|---|---|---|
| zone | 197 | 502 |
| lobby | 10 | 23 |
| chat | 4 | not yet enumerated |
| **Total** | **211** | — |

Cross-referenced with `garlemald-server/map-server/src/packets/opcodes.rs`:
60 opcodes the binary handles are NOT in garlemald (server-side
"holes" — features the server can't yet send). The garlemald
opcodes file has no opcodes the binary doesn't handle (= no
invented opcodes).

### 3.4 — Up opcode reconnaissance (✅ validation pass; full enumeration deferred)

The Up direction (client → server) uses `ClientPacketBuilder`
constructors — each CPB ctor takes the opcode as an arg and stores
it at `[builder+0x1C]`. Full enumeration requires per-callsite
constant propagation through the CPB ctor's arg0, which is a
Ghidra-driven analysis still TBD.

`tools/extract_up_opcodes.py` runs a NECESSARY-but-not-sufficient
check: every garlemald `OP_RX_*` value appears as a `PUSH imm32`
somewhere in `.text`. Confirms no garlemald RX opcode is invented
(but says nothing about which opcodes garlemald is missing).

### 3.5 — MurmurHash2 validation (✅ bit-for-bit)

The `SetActorPropertyPacket` (zone protocol gameplay state) wire
ids are 32-bit Murmur2 hashes of property-name strings (e.g.
`"charaWork.parameterSave.hp[0]"` → `0xE14B0CA8`). `FUN_00d31490`
in the binary is a Murmur2 variant that walks the buffer
**backward** from `data + len - 4` in 4-byte chunks (canonical
Murmur2 walks forward). `tools/validate_murmur2.py` runs a Python
port; matches against `garlemald-server/common/src/utils.rs::
murmur_hash2` over 6 known test vectors. See `docs/murmur2.md`.

### 3.6 — CharaMakeData parse-side validation (✅ 4 surfaced bugs)

`tools/validate_chara_make.py` cross-references
`garlemald-server/lobby-server/src/data/chara_info.rs::
parse_new_char_request` against the binary's GAM CharaMakeData
schema. Output: `build/wire/<bin>.chara_make_validation.md`.

Surfaced field-level mismatches (suggested patch in the report):
- `appearance.face_features` → should be `face_cheek` (id 112)
- `appearance.ears` → should be `face_jaw` (id 114) — 1.x doesn't
  expose ears as a separate slot
- `info.current_class: u16` → conflates GAM id 122
  `initialMainSkill` + id 123 `initialEquipSet` (loses the
  equipment-set value)
- Three trailing `u32 skip` reads → ARE GAM id 124
  `initialBonusItem: int[3]` (starter items the parser silently
  drops)

These are real bugs. Applying them to garlemald-server is on the
backlog (see "Open work" below).

### 3.7 — CharacterListPacket build-side validation (🟡 schema-level only; byte-layout TBD)

`tools/validate_chara_list.py` cross-references
`garlemald-server::build_for_chara_list` against GAM
ClientSelectData. Output:
`build/wire/<bin>.chara_list_validation.md`.

**Important caveat**: this is *schema-level*, not byte-layout
validation. The `build_for_chara_list` output is a **hand-rolled
flat blob** (Project Meteor reverse-engineered from network
captures), NOT a GAM-encoded `(id, value)` self-describing
structure. The validator pairs each Rust write with its
nearest-named GAM field and flags type mismatches that "look
like" bugs — but the chara-list packet may legitimately use
different wire types than the GAM schema declares for the
"same" semantic field.

Five flags surfaced (`current_level: u16` vs `mainSkillLevel:
signed char`, `tribe: u8` vs `tribe: Utf8String`,
`location1/2_bytes` vs `zoneName/territoryName: signed char`,
`initial_town: u32 (twice)` vs `initialTown: short`). Definitive
resolution needs the binary's `CharacterListPacket::Deserialize`
— see "Open question" below.

### 3.8 — LobbyCryptEngine 9-slot decode (✅)

`tools/extract_crypt_engine.py` reads the 9 vtable slots of
`Application::Network::LobbyProtoChannel::ServiceConsumerConnection
Manager::LobbyCryptEngine` and validates the embedded Blowfish P/S
init tables. Output:
`build/wire/<bin>.crypt_engine.md`.

| Slot | RVA | Semantic |
|---:|:---|:---|
| 0 | `0x009a1e40` | `~LobbyCryptEngine` (frees `[this+0x30]` = BF_KEY*) |
| 1 | `0x009a1590` | `PrepareHandshake` — copies 32-byte ASCII seed `"Test Ticket Data\0\0\0\0clientNumber"` from `.data 0x011274F0` to `this+0x10`; `_time64(NULL)` low 32 bits → `this+0x8` + `req+0x74` |
| 2 | `0x009a1640` | 3-arg stub returning 0 |
| 3 | `0x009a0f10` | 2-arg stub returning false |
| 4 | `0x009a1670` | `SetSessionKey` — frees old `BF_KEY`, mallocs 4168 bytes (= sizeof(BF_KEY)), constructs 16-byte key, calls `BF_set_key(key, len=16)` |
| 5 | `0x009a0f20` | 2-arg stub returning false |
| 6 | `0x009a18d0` | `Encrypt(_, buf, len)` — rounds len DOWN to multiple of 32, in-place ECB Blowfish encrypt via OpenSSL `BF_encrypt` per block |
| 7 | `0x009a0f30` | `Decrypt(_, buf, len)` — same shape, OpenSSL `BF_decrypt` per block |
| 8 | `0x009a1920` | 1-arg stub returning true |

Per-block primitives (statically-linked OpenSSL):
- `FUN_0045aac0` = `BF_encrypt(BF_LONG[2], BF_KEY*)`
- `FUN_0045aa30` = `BF_decrypt(BF_LONG[2], BF_KEY*)`
- `FUN_0045abf0` = `BF_set_key(BF_KEY*, int keylen, const unsigned char*)`

P/S init constants live at fixed VA `0x01267278` (P[18], 72 bytes)
and `0x012672C0` (S[4][256], 4096 bytes). **Canonical pi-derived**
(Schneier 1993 / OpenSSL `bf_pi.h`):

```
P[0..3]    = { 0x243F6A88, 0x85A308D3, 0x13198A2E, 0x03707344 }
S[0][0..3] = { 0xD1310BA6, 0x98DFB5AC, 0x2FFD72DB, 0xD01ADFB7 }
```

**Garlemald cross-validation** (✅ all bit-for-bit):
- `common/src/blowfish_tables.rs::P_VALUES` matches binary
  `0x01267278..0x012672BF` byte-for-byte.
- `common/src/blowfish_tables.rs::S_VALUES` matches binary
  `0x012672C0..0x012682BF` byte-for-byte.
- The non-canonical `MOVSX byte` (sign-extend) in the binary's
  key-schedule byte-cycling step is reproduced in
  `common/src/blowfish.rs:74-78` via `key[j] as i8 as i32 as u32`.
- 16 rounds + final swap + P[16]/P[17] XOR matches OpenSSL canonical.

**Zone/chat encryption — confirmed absent**:
- RTTI sweep finds only ONE concrete CryptEngineInterface
  subclass (`LobbyCryptEngine`).
- Garlemald's world-server and map-server have zero
  `blowfish` / `encipher` / `encrypt` call sites.
- Lobby is the only encrypted channel; zone and chat are
  plaintext.

**32-byte alignment quirk — resolved as benign**:
- Lobby slots 6/7 round buffer length DOWN to multiples of 32 via
  `AND EAX, 0xFFFFFFE0`. Trailing 0..31 bytes pass through
  unencrypted by the client.
- Garlemald's `encipher`/`decipher` require 8-aligned and encrypt
  ALL of `len`. The two policies diverge — but both Project Meteor
  and garlemald produce the SAME output (Meteor uses `Encipher
  (data, offset+0x10, subpacketSize-0x10)` with no 32-aligned
  check), and Project Meteor has shipped against the real client
  for years.
- The trailing 0..31 garbled bytes always land in the over-
  provisioned trailing zero region of fixed-capacity lobby
  buffers (`MemoryStream(0x98)`, `vec![0u8; 0x280]`, etc.). The
  client never reads past the meaningful prefix.
- **Conclusion**: garlemald is correct as-written; documentation-
  only finding. Worked examples in
  `include/net/lobby_proto_channel.h`.

### 3.9 — Lobby Recv/Send paths (✅ field-level decoded)

`include/net/lobby_proto_channel.h` (hand-written) captures the
4-slot `ClientPacketBuilder` shape shared by all three channels
(Lobby / Zone / Chat):

```
vtable[0] ~ClientPacketBuilder    : scalar deleting destructor
vtable[1] uint8_t* Begin()        : returns &this[0x10] (write ptr)
vtable[2] void BuildHeader(out*)  : writes header[0..3] + header[8..11]
vtable[3] void Send(buf, len)     : split into header(16) + payload,
                                     call dispatch helpers (no-ops in
                                     this build — real send is via
                                     RUDP2 layer)
```

`BuildHeader` writes (Lobby/Zone):

```
header[0]   = 0x14                    (constant magic / flags byte)
header[1]   = 0x00                    (reserved)
header[2..4]= (u16) this->[0x1c]      (connection_type, captured at ctor)
header[4..8]= NOT WRITTEN              (caller-populated:
                                        packet_size + num_subpackets)
header[8..12]= (u32) _time64(NULL)    (timestamp low 32 bits;
                                        Chat hardcodes 0x0A here)
header[12..16]= NOT WRITTEN           (typically 0 — high 32 of u64
                                        timestamp in garlemald's view)
```

Field offsets cross-validated against `common/src/{packet,
subpacket}.rs` — no divergences.

**Receive dispatcher**: `LobbyProtoDownCallbackInterface::vtable[1]`
at RVA `0x009a4160` (319 bytes). Same `byte_table` + `dword_table`
two-stage jump pattern as the Down opcode dispatchers documented
in 3.3.

## Open question — `CharacterListPacket::Deserialize`

The user-flagged 5 chara-list "bugs" from 3.7 need byte-layout
confirmation against the binary's actual deserializer (Project
Meteor's encoder is **observational evidence**, not authoritative —
Meteor has its own bugs that have been masked by client tolerance).

A static-analysis cross-reference walk produced these findings:

1. **Lobby Down dispatch goes through `LobbyProtoDownCallback
   Interface::vtable[1]` (= `FUN_00da4160`)** — confirmed dispatcher
   shape. For opcode `0x0D` (CharacterList), `byte_table[12] = 3`
   → `dword_table[3] = handler stub at 0x009a41da` → calls
   `[this->vtable + 0x14]` = slot 5 of the LobbyProtoDownCallback
   subclass.

2. **The abstract base has only ONE concrete subclass** —
   `LobbyProtoDownDummyCallback@LobbyClient@Network@Application`,
   confirmed via RTTI extraction (`config/<bin>.rtti.json`). And
   this subclass's slot 5 is `FUN_00da2d10` = `RET 0xc` — a no-op
   stub. So the "obvious" dispatch path **does not handle opcode
   0x0D in this build**.

3. **No third subclass exists** — RTTI sweep for
   `.?AV*LobbyProtoDown*` finds only the abstract base + the
   Dummy subclass.

4. **Magic constants from Project Meteor's `BuildForCharaList` are
   dead code in the binary** — `0x232327EA` and `0xE22222AA`
   each appear in tiny 6-byte "return constant" getter functions
   with **zero callers**. The client doesn't validate them.

5. **String literals from Meteor's format are not in the binary** —
   `"prv0Inn01"`, `"defaultTerritory"`, `"CharacterListPacket"`
   are all absent. The client doesn't string-match them.

6. **Base64 decode is reachable** — URL-safe base64 alphabet at
   `.data 0x0126726c`, encode/decode at `FUN_0045a1d0` /
   `FUN_0045a590`, wrappers at `FUN_0045a920` / `FUN_0045a970`,
   with 6 external direct callers (in functions of size 415B,
   534B, 640B, 973B, 606B, 1928B). One of those is the chara-list
   payload reader, but without GUI-Ghidra type propagation, the
   pure-Python xref scan can't disambiguate which one.

**Architectural conclusion**: the chara-list deserializer is
reachable but lives behind indirect calls in a parallel codepath
(likely `MyGameLoginCallback`-adjacent in the lobby state machine),
not the abstract `LobbyProtoDownCallbackInterface` dispatch we
identified. The "Dummy" subclass naming suggests the entire
Down-callback interface scaffolding is inactive in retail builds
and the lobby uses a different dispatch mechanism.

**Two paths to close this question**:

1. **Interactive Ghidra GUI session** — open `ffxivgame.exe`,
   use auto-analysis + "Find References To..." on the abstract
   `LobbyProtoDownCallbackInterface` typeinfo, the lobby opcode
   `0x0D` constant in dispatch sites, and `MyGameLoginCallback`'s
   non-stub slots (7, 10, 11, 12). The GUI can follow indirect
   calls and propagate types in ways the Python xref scans can't.
2. **Capture-and-decrypt empirical observation** — boot a working
   `fresh-start-*.sh` session, log encrypted chara-list bytes from
   the wire, decrypt them using garlemald's session BF key (which
   the server already knows), and inspect the actual byte layout
   the **client accepted**. Direct ground truth from observation.

Either approach resolves the 5 schema flags definitively.

## Open work (backlog)

In rough priority order:

1. Resolve `CharacterListPacket::Deserialize` (above) — closes the
   chara-list bugs.
2. Apply the 4 surfaced `chara_make_validation` patches to
   `garlemald-server::parse_new_char_request`.
3. **Phase 2 closure** — procure VS 2005 SP1 + Platform SDK 2003 R2
   (legal copy required); iterate `MSVC_FLAGS` until `objdiff`
   reports zero delta on `FUN_00b361b0`.
4. Full Up-opcode enumeration (per-callsite arg propagation through
   CPB ctor's arg0).
5. `LobbyCryptEngine::vtable[6/7]` callsite trace — would
   definitively show what `len` arg is passed in retail traffic
   (currently inferred as benign via the worked example argument).
6. Decompile `*ProtoChannel::Recv`/`Send` paths into C++ headers
   under `include/net/` for the remaining fields not yet captured
   in `lobby_proto_channel.h`.

## Toolbox

| Tool | Role |
|---|---|
| `tools/extract_pe.py` | PE structure dump (Phase 0) |
| `tools/import_to_ghidra.py` | Ghidra import + analysis (Phase 1) |
| `tools/build_split_yaml.py` | Work-pool emission (Phase 1) |
| `tools/extract_net_vtables.py` | Net-class slot map |
| `tools/extract_gam_params.py` | GAM property registry |
| `tools/extract_paramnames_dispatch.py` | PARAMNAME dispatcher walker |
| `tools/emit_gam_header.py` | C++ header emission |
| `tools/extract_opcode_dispatch.py` | Down opcode → slot map |
| `tools/extract_up_opcodes.py` | Up opcode reconnaissance |
| `tools/extract_crypt_engine.py` | LobbyCryptEngine 9-slot decode + Blowfish validation |
| `tools/validate_murmur2.py` | MurmurHash2 vectors |
| `tools/validate_chara_make.py` | chara_info.rs ↔ GAM CharaMakeData |
| `tools/validate_chara_list.py` | build_for_chara_list ↔ GAM ClientSelectData |
| `tools/find_rosetta.py` | Phase 2: pick best Rosetta candidate |
| `tools/cl-wine.sh` | Phase 2: cl.exe under CrossOver Wine |
| `tools/setup-msvc.sh` | Phase 2: toolchain detection |
| `tools/compare.py` | Phase 2: objdiff on a single function |
| `tools/progress.py` | Phase 2/3: progress dashboard |
