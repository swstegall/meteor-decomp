# Decompilation status — narrative recap

A running record of what's been recovered from `ffxivgame.exe`, what's
been validated against `garlemald-server` (the Rust port), and where
the open questions are. Last update: 2026-05-02.

For the strategic plan and exit criteria, see [PLAN.md](../PLAN.md).
For per-subsystem detail, see the auto-generated reports under
`build/wire/<binary>.*.md` (regenerable via `make`).

## Headline numbers (2026-05-02)

`make progress` summary across all five binaries:

| Binary | YAML matched (count / bytes) | _rosetta/*.cpp files | Notes |
|---|---|---|---|
| `ffxivgame.exe` | 23,106 / 210,648 B | 38,593 | Primary target |
| `ffxivboot.exe` | 14,330 / 125,304 B | 26,103 | Cross-binary template multiplier |
| `ffxivlogin.exe` | 357 / 8,326 B | 281 | |
| `ffxivupdater.exe` | 431 / 5,975 B | 433 | ZiPatch home |
| `ffxivconfig.exe` | 176 / 1,715 B | 185 | |
| **Overall** | **38,400 / 351,968 B (1.86 %)** | **65,595 files / 683,986 B (3.61 %)** | |

The jump from "single-digit functions matched" to "tens of thousands"
came from the **template-derivation pipeline** (§ Phase 2.5 below)
landed across late April and early May: rather than match one function
at a time, cluster shape-equivalent functions, derive a
relocation-aware template per cluster, and stamp every cluster member
GREEN in one pass.

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

## Phase 2 — matching toolchain (✅ working)

The goal is byte-identical recompilation: take the binary's matched
function, hand-write equivalent C++, compile with the original
toolchain, and confirm `compare.py`/`objdiff` reports zero delta. This
proves the hand-written C++ is semantically and bit-exactly the source.

What works today:
- **VS 2005 Express RTM operational** under CrossOver Wine 9 on Apple
  Silicon. `vstudio2005-workspace/install.sh` extracts cl.exe / link.exe
  / c1.dll / c1xx.dll / c2.dll / mspdb80.dll + headers + libs from the
  official VS2005EE ISO via msitools (bypassing Wine's broken msiexec).
  `cl.exe Version 14.00.50727.42 for 80x86`. `make setup-msvc` passes.
- **Platform SDK 2003 R2** also installed via
  `vstudio2005-workspace/install-psdk.sh` — `PSDK-x86.msi` extracted
  via msiextract (same Wine-bypass technique), giving us the full PSDK
  tree (`sdk/PSDK/Include/` + `sdk/PSDK/Lib/`). This unblocked
  Win32-touching matches; previously they failed at link time.
- **`tools/cl-wine.sh`**: runs cl.exe under wow64 mode (no
  `WINEARCH=win32`) with the `DYLD_FALLBACK_LIBRARY_PATH` shim and
  reads `MSVC_TOOLCHAIN_DIR` from `~/.config/meteor-decomp.env`.
- **`tools/compare.py`**: relocation-aware byte-level diff. Reads the
  `.text` section from the staged `.obj`, slices the corresponding RVA
  range from `orig/<bin>.exe`, and prints GREEN/PARTIAL/MISMATCH +
  per-byte diff with first-mismatch offset. Exit codes 0/1/2 gate Make
  / CI on match status.
- **First GREEN match**: `FUN_004165b0` (28-byte int setter) landed
  2026-05-01 — see [`reference_meteor_decomp_rosetta_match.md`](../../../.claude/projects/-Users-swstegall-Documents-Programming-server-workspace/memory/reference_meteor_decomp_rosetta_match.md)
  for the recipe (Ghidra-decompiler-assist + 3 MSVC-2005 source-pattern
  tricks: element-wide pointers, two-pointer w/ both deref, count > 0
  vs != 0).

The original blocker ("waiting on Platform SDK") is now resolved — both
toolchains are installed and matches are landing. **Phase 2's exit
criterion is met**; matching is now ongoing decomp work, not a
toolchain blocker.

## Phase 2.5 — template-derivation pipeline (✅ scaling matching)

The single-function matching loop (write C++, compile, diff, iterate)
takes ~10–60 minutes per function. At 75k+ functions in `ffxivgame.exe`
alone, that's not the right rhythm. The **template-derivation
pipeline** scales matching by an order of magnitude.

The insight: most functions in a Win32 game binary are not unique. They
are dozens of copies of the same compile-time pattern — getter/setter
trampolines, scalar deleting destructors, vtable trampolines, SEH catch
handlers, bool-nonzero predicates, etc. — instantiated once per type by
MSVC. If we can recover *one* C++ source for the cluster, we can stamp
every member GREEN simultaneously.

Pipeline stages:

1. **`tools/cluster_shapes.py`** — group functions by *byte-shape modulo
   relocations*: replace each rel32/rel8 with a placeholder, then bucket
   by the resulting fingerprint. Output: clusters of "structurally
   identical" functions across all five binaries.
2. **`tools/cluster_relocs.py`** — within each cluster, decode the
   ModR/M / SIB at every relocation site so the template knows what
   kind of operand each placeholder represents (a function pointer, a
   global address, a stack offset, etc.). Handles the full ALU
   `0x80/0x81/0x83`, `0x88/0x89/0x8a/0x8b/0x8d` (MOV/LEA),
   `0xc0/0xc1/0xc6/0xc7/0xd0..0xd3/0xfe/0xff` (rotates / immediate
   stores / arithmetic), and `0x69/0x6b` (IMUL imm) opcode families
   with proper length decoding.
3. **`tools/recompute_sizes.py`** — Ghidra sometimes drops mid-function
   bytes (epilogue mis-detection, single-byte INT3 padding); this pass
   walks the binary and re-derives true function ends, accepting
   "next-function-starts" as the boundary signal.
4. **`tools/seed_templates.py --reloc`** — for each cluster, pick the
   smallest member as the seed, generate a `.cpp` that compiles to the
   same shape, and verify the seed matches. If GREEN, stamp every
   cluster sibling.
5. **`tools/derive_templates.py`** — when the seed approach can't
   generate a working `.cpp` (some MSVC idioms don't have a clean
   C-source equivalent), drop down to a **naked-asm template**: emit
   `_emit` byte sequences with `__asm` blocks and patch the relocation
   slots from a per-instance manifest. ~75 patterns hand-written so
   far covering scalar deleting destructors (D2), array deleting
   destructors (D3), SEH `Catch_All` handlers, push-call wrappers,
   chained-pointer getters, vtable trampolines, MOV/LEA disp32,
   constant-byte clusters, etc.
6. **`tools/stamp_clusters.py`** — runs the matching template against
   every member of a cluster, validating each individually with
   `compare.py`. Members that match are stamped GREEN in
   `_rosetta/<rva>.cpp`.
7. **`tools/validate_clusters.py`** — a separate pass that re-validates
   already-stamped templates against the binary; catches regressions
   when the toolchain or pipeline changes.
8. **`tools/update_yaml_status.py`** — folds per-file validate results
   back into the YAML work pool (`status: matched`).
9. **`tools/find_easy_wins.py`** — scans the work pool for high-value
   single-function matching candidates not yet covered by a template
   (smallest unmatched function with the most cross-binary copies,
   fewest relocations, etc.).
10. **`tools/verify_asm_vs_orig.py`** + **`verify_by_symbol.py`** —
    universal ASM-vs-orig sanity check; catches mid-function Ghidra
    drops that would otherwise let a bogus template "match" against
    truncated bytes.

Cumulative effect (commit history through 2026-05-02): going from ~10
hand-matched functions to **38,400 GREEN-status functions in YAML
across 5 binaries** + **65,595 durable `_rosetta/*.cpp` files**. The
single largest individual landings were the 1,552-sibling stamped
cluster (`780c628c3`) and the auto-template pass that emitted 10,577
GREEN templates in one go (`d9f64cf19`).

## Phase 2 / 2.5 — open work

- **Sweep more cluster patterns** — every new `derive_templates.py`
  pattern unlocks a new family of trivial functions (the per-pattern
  yields range from 13 to 406 GREEN templates each). Look at unmatched
  clusters of size ≥ 10 that share a structural fingerprint and write
  the matching template.
- **Cross-binary multipliers** — when a template matches in
  `ffxivgame.exe` it usually multiplies into the small binaries too
  (`seed_templates.py --reloc` has been delivering ~700 ffxivboot +
  3 ffxivconfig per pass). Re-run after every fresh template.
- **Tighten epilogue detection** — `recompute_sizes.py` still has edge
  cases where a function's true end gets misidentified; auditing
  remaining mismatches in stamped clusters is the way in.

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

## Phase 4 — Pack / ChunkRead / InstallUnpacker (▶ active matching)

Phase 4 targets the file-system + installer subsystems. Detailed
architecture in [`sqpack.md`](sqpack.md) and
[`install-unpacker.md`](install-unpacker.md). Headline finding from
reconnaissance: 1.x is **resource-id-addressed**, not string-path-hashed
— the `Sqpack::Hash` family that ships with ARR/DQX does not exist in
1.x. Files live at `<game>/data/<b3>/<b2>/<b1>/<b0>.DAT` keyed by a
32-bit `resource_id`.

### 4.1 — Sqex::Data class hierarchy (✅ recovered)

```
Sqex::Data::ChunkRead<unsigned int, unsigned int>      (vtable RVA 0xb931c8)
└── Sqex::Data::PackRead                                (vtable RVA 0xd0dd40)

Sqex::Data::ChunkWrite<unsigned int, unsigned int>     (vtable, 1 slot)
└── Sqex::Data::PackWrite                               (vtable RVA 0xd1311c)

(parallel byte-sized chunk variants)
Sqex::Data::ChunkRead<unsigned char, unsigned short>   (1 slot)
Sqex::Data::ChunkWrite<unsigned char, unsigned short>  (1 slot)
```

Vtables expose only the destructor; every other method is non-virtual
and must be enumerated by xref-walking the constructor / destructor
sites. The `<u8,u16>` instantiations are parallel — likely texture
streams or audio with smaller chunk-id and chunk-size widths.

### 4.2 — Sqex::Data matches (4 GREEN, 2 PARTIAL)

Source under [`src/ffxivgame/sqpack/`](../src/ffxivgame/sqpack/) and
[`src/ffxivgame/_partial/`](../src/ffxivgame/_partial/).

| Function | RVA | Size | Status | Notes |
|---|---|---:|---|---|
| `PackRead::~PackRead` | `0x008c6670` | 110 B | ✅ GREEN | First Phase-4 GREEN — sets vtable, frees `[this+0x74]`, hands to `ChunkRead<u32,u32>::~ChunkRead` |
| `PackRead::PackRead` (ctor) | `0x00942800` | 132 B | 🟡 130/132 PARTIAL | Two iterations; sets vtable + initialises heap buffer. Same shape as dtor; off-by-2 bytes in cookie/SEH frame setup |
| `PackRead::ReadNext` | (tiny stub) | 27 B | ✅ GREEN | Trivial loop driver |
| `PackRead::Rewind` | (tiny stub) | 18 B | ✅ GREEN | |
| `PackRead::ProcessChunk` | (mid) | 177 B | 🟡 180/177 PARTIAL | Buffer-guard cookie blocker — the function uses `/GS` cookie + `__security_check_cookie` whose epilogue ordering is sensitive to exact local layout |
| `ChunkReadUInt::ReadNextChunkHeader` | (mid) | 81 B | 🟡 74/81 PARTIAL | Header-parsing inner loop |

### 4.3 — Sqex::Misc::Utf8String (2 GREEN, 3 PARTIAL)

Recovered the layout (vtable + size + capacity + heap pointer). Source
under [`src/ffxivgame/sqex/Utf8String.cpp`](../src/ffxivgame/sqex/Utf8String.cpp).

| Function | Size | Status | Notes |
|---|---:|---|---|
| `Utf8String::Utf8String` (default ctor) | 39 B | ✅ GREEN | |
| `Utf8String::~Utf8String` | 24 B | ✅ GREEN | |
| `Sqex::Misc::Utf8String::Utf8String` (alt ctor) | 116 B | 🟡 109/116 PARTIAL | Layout recovered |
| `Utf8String::Reserve` | 153 B | 🟡 144/153 PARTIAL | 94 % match; pending Ghidra-GUI globals identification |

### 4.4 — Sqex slab allocator pair (2 PARTIAL)

`Utf8String` delegates allocation to two cdecl helpers
(`Utf8StringAlloc` / `Utf8StringFree`) that index global slab tables
at `0x01266dc0` (slab descriptors), `0x0132cec8` (free-list buckets),
`0x0132cf1c` (mutex array). Source under
[`src/ffxivgame/sqex/Allocator.cpp`](../src/ffxivgame/sqex/Allocator.cpp).
See [`ghidra-tasks.md`](ghidra-tasks.md) for the open Ghidra-GUI tasks
to recover the missing slab-descriptor / mutex struct names.

| Function | RVA | Size | Status |
|---|---|---:|---|
| `Utf8StringAlloc` | `0x0004d500` | 225 B | 🟡 222/225 PARTIAL |
| `Utf8StringFree`  | `0x0004d350` | 105 B | 🟡 104/105 PARTIAL |

### 4.5 — Component::Install::InstallUnpacker (3 GREEN, 2 PARTIAL, 1 deferred)

`InstallUnpacker` is a `Sqex::Thread::Thread` subclass with a
secondary `InstallWriter` base at `+0x38`. Slot 2 of its primary
vtable (RVA `0x00d0d53c`) is the `Run` override — a 490-byte
producer-consumer chunk-extraction loop. The class is the only
direct consumer of `PackRead` in `ffxivgame.exe`. Detailed structural
decode in [`install-unpacker.md`](install-unpacker.md).

Source under [`src/ffxivgame/install/`](../src/ffxivgame/install/).

| Function | RVA | Size | Status | Notes |
|---|---|---:|---|---|
| `InstallUnpacker::WaitForReady` | (tiny) | 71 B | ✅ GREEN | Spin loop using `InterlockedExchangeAdd` |
| `ResourceQueue::TryEnqueue` | — | 122 B | ✅ GREEN | |
| `ChunkSource::ReleaseChunk` | — | 124 B | ✅ GREEN | |
| `ChunkSource::AcquireChunk` | — | 144 B | 🟡 144/144 PARTIAL | 21 byte mismatches, structurally aligned — a few iterations away from GREEN |
| `InstallUnpacker::Unpack` (slot 2) | `0x008c6700` | 490 B | 🟡 428/490 (Iteration #1) | 249 mismatches; deferred pending parent-class layout recovery + helper signatures (see "Next blocker" below) |

All six kernel32 IAT entries the unpacker uses have been resolved
via Ghidra GUI: `InterlockedExchange`, `InterlockedCompareExchange`,
`InterlockedExchangeAdd`, `Sleep`, `InterlockedIncrement`,
`SwitchToThread`. See [`ghidra-tasks.md § Status snapshot`](ghidra-tasks.md).

### 4.6 — CRT helper sweep (32+ GREEN with cross-binary multipliers)

Source under [`src/ffxivgame/crt/`](../src/ffxivgame/crt/) — covers the
small functions MSVC's CRT statically links into every binary. Each
match cross-multiplies into the four other binaries (same MSVC build,
same library). Pattern: write the C source for one CRT helper, stamp
every cross-binary copy GREEN.

Files matched: `Strncmp`, `Strcmp`, `Strlen`, `Memset`, `Fopen`,
`Atol`, `Alloca`, `EHProlog` (`__EH_prolog3_catch_GS`), `Exit`,
`InitTerm`, `InvalidParameter` (`_invalid_parameter_noinfo`), `Unwind`.
Cumulative landings (per commit history):
- `e7181509` — 12 GREEN initial sweep (357 B)
- `5a7121a9` — `fopen` + 25 cross-binary multipliers
- `6a642ebb` — `__EH_prolog3_catch_GS` + `memset` (4 GREEN, 7 with multipliers)
- `5b55ef56` — `strcmp` + `strlen` (10 GREEN with multipliers)
- `ede50a9` — `strncmp` (4 GREEN)

### 4.7 — Next blocker — `InstallUnpacker::Unpack` (FUN_00cc6700)

The 490-byte slot-2 method is the highest-value remaining Phase 4
target. To match it we need (in priority order):

1. **Parent class layout** beyond the inferred fields — especially
   what's at `m_field_40 + 0x60` and `m_field_40 + 0x2140` (atomic-
   counter accesses suggest a nested counter struct).
2. **Helper function signatures** for `FUN_00cc5db0` (268 B chunk-
   source acquire), `FUN_00cc5e40` (124 B release), `FUN_00cc6510`
   (343 B), and `FUN_00cc6620` (71 B wait-for-ready spin).
3. **The "alt" Utf8String ctor at `0x00445cf0`** — distinct from
   `Sqex::Misc::Utf8String::Utf8String @ 0x00047260`, likely a
   different overload or a Sqwt-namespace string class.
4. **`FUN_008edbf0`** (122 B `WaitablePredicate::TryReady`) — now
   writable since `InterlockedIncrement` IAT entry is resolved.

Each of these is a separate Ghidra GUI task. See
[`ghidra-tasks.md`](ghidra-tasks.md) for the list.

## Open work (backlog)

In rough priority order:

1. **Push `InstallUnpacker::Unpack` GREEN** — biggest remaining
   Phase-4 win. Needs the four Ghidra-GUI deliverables above.
2. **Push `ChunkSource::AcquireChunk` GREEN** — 144/144 with 21 byte
   mismatches; structurally aligned, just needs cookie-frame /
   register-allocation iteration.
3. **Push `Utf8String::Reserve` + `Utf8StringAlloc/Free` GREEN** —
   pending Ghidra GUI on the slab-allocator globals
   (see [`ghidra-tasks.md`](ghidra-tasks.md)).
4. **Sweep more cluster patterns** in `derive_templates.py` — every
   pattern unlocks 13–406 more GREEN templates.
5. Resolve `CharacterListPacket::Deserialize` (open question above) —
   closes the chara-list bugs.
6. Apply the 4 surfaced `chara_make_validation` patches to
   `garlemald-server::parse_new_char_request`.
7. Full Up-opcode enumeration (per-callsite arg propagation through
   CPB ctor's arg0).
8. `LobbyCryptEngine::vtable[6/7]` callsite trace — would
   definitively show what `len` arg is passed in retail traffic
   (currently inferred as benign via the worked example argument).
9. Decompile `*ProtoChannel::Recv`/`Send` paths into C++ headers
   under `include/net/` for the remaining fields not yet captured
   in `lobby_proto_channel.h`.

## Toolbox

### Phase 0/1 — bootstrap + static analysis
| Tool | Role |
|---|---|
| `tools/extract_pe.py` | PE structure dump (Phase 0) |
| `tools/symlink_orig.sh` | Populate `orig/` from the workspace install |
| `tools/import_to_ghidra.py` | Ghidra import + analysis (Phase 1) |
| `tools/build_split_yaml.py` | Work-pool emission (Phase 1) |
| `tools/regenerate_overridden_asm.py` | Re-dump asm for size-overridden functions |

### Phase 2 — single-function matching
| Tool | Role |
|---|---|
| `tools/cl-wine.sh` | Wraps cl.exe / link.exe under CrossOver Wine |
| `tools/setup-msvc.sh` | Toolchain detection (cl.exe + PSDK) |
| `tools/compare.py` | Relocation-aware byte-level diff (orig slice vs `.obj`) |
| `tools/find_rosetta.py` | Picks the best small Rosetta candidate |
| `tools/find_easy_wins.py` | Auto-rank single-function matching candidates |
| `tools/verify_asm_vs_orig.py` | Catches mid-function Ghidra drops |
| `tools/verify_by_symbol.py` | Per-symbol asm-vs-orig sanity check |

### Phase 2.5 — template-derivation pipeline
| Tool | Role |
|---|---|
| `tools/cluster_shapes.py` | Group functions by byte-shape modulo relocations |
| `tools/cluster_relocs.py` | Decode ModR/M / SIB at every relocation site (full ALU + MOV + LEA + IMUL families) |
| `tools/recompute_sizes.py` | Re-derive true function ends; catches Ghidra drops |
| `tools/seed_templates.py` | Per-cluster seed-and-stamp pass (`--reloc` for cross-binary) |
| `tools/derive_templates.py` | Naked-asm `_emit` templates for clusters that resist source matching |
| `tools/stamp_clusters.py` | Run a template against every cluster member; stamp matches |
| `tools/validate_clusters.py` | Re-validate stamped templates against the binary |
| `tools/update_yaml_status.py` | Fold validate results into the YAML work pool |

### Phase 3 — wire-protocol extraction
| Tool | Role |
|---|---|
| `tools/extract_net_vtables.py` | Net-class slot map |
| `tools/extract_gam_params.py` | GAM property registry |
| `tools/extract_paramnames_dispatch.py` | PARAMNAME dispatcher walker |
| `tools/extract_gam_types_rtti.py` | GAM types from RTTI |
| `tools/emit_gam_header.py` | C++ header emission |
| `tools/extract_opcode_dispatch.py` | Down opcode → slot map |
| `tools/extract_up_opcodes.py` | Up opcode reconnaissance |
| `tools/extract_crypt_engine.py` | LobbyCryptEngine 9-slot decode + Blowfish validation |
| `tools/validate_murmur2.py` | MurmurHash2 vectors |
| `tools/validate_chara_make.py` | chara_info.rs ↔ GAM CharaMakeData |
| `tools/validate_chara_list.py` | build_for_chara_list ↔ GAM ClientSelectData |

### Reporting
| Tool | Role |
|---|---|
| `tools/progress.py` | Per-binary headline numbers (matched / total / `_rosetta/*.cpp`) |
