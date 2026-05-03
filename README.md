# meteor-decomp

Decompilation of the FINAL FANTASY XIV 1.23b Windows client binaries
(`ffxivgame.exe`, `ffxivboot.exe`, `ffxivlogin.exe`,
`ffxivupdater.exe`, `ffxivconfig.exe`).

See **[PLAN.md](PLAN.md)** for the full strategy, scope, and roadmap;
**[docs/decomp-status.md](docs/decomp-status.md)** for a deeper
narrative of what's been recovered so far and where the open questions
sit; and **[docs/wire-protocol.md](docs/wire-protocol.md)** for the
architectural model of the network layer.

## Status — at a glance

This is a hybrid effort. The pure matching track (Phase 2 — byte-
identical recompilation) is **operational** as of 2026-05-01 (first
GREEN landed) and **scaling** via the template-derivation pipeline
(Phase 2.5). The functional/static-analysis track (Phase 3) has
already produced enough wire-level ground truth to validate
`garlemald-server` (the Rust port) byte-for-byte in several subsystems.

| Area | Status | Output |
|---|---|---|
| **Phase 0 — bootstrap** | ✅ done | `make bootstrap`: PE structure dump |
| **Phase 1 — Ghidra import + work pool** | ✅ done | 9,729 vtable slots / 576 net-relevant classes |
| **Phase 2 — matching toolchain** | ✅ working | VS 2005 RTM + PSDK 2003 R2 under CrossOver Wine 9; first GREEN match landed 2026-05-01 |
| **Phase 2.5 — template-derivation pipeline** | ✅ live | `cluster_shapes.py` + `cluster_relocs.py` + `derive_templates.py` + `seed_templates.py` + `stamp_clusters.py` |
| **Phase 3 — functional / wire decomp** | 🟢 substantial — see below | `tools/extract_*.py`, `build/wire/*.md`, `include/net/*.h` |
| **Phase 4 — Pack / ChunkRead / InstallUnpacker** | ▶ active matching | 11+ GREEN, 8 PARTIAL across `src/ffxivgame/{sqpack,sqex,install,crt}/` |

### Headline numbers (2026-05-02)

`make progress` summary:

| Binary | YAML matched | `_rosetta/*.cpp` files |
|---|---:|---:|
| `ffxivgame.exe` | 23,106 / 210,648 B | 38,593 |
| `ffxivboot.exe` | 14,330 / 125,304 B | 26,103 |
| `ffxivlogin.exe` | 357 / 8,326 B | 281 |
| `ffxivupdater.exe` | 431 / 5,975 B | 433 |
| `ffxivconfig.exe` | 176 / 1,715 B | 185 |
| **Total** | **38,400 / 351,968 B (1.86 %)** | **65,595 / 683,986 B (3.61 %)** |

### Phase 3 — what's been recovered

| # | Subsystem | Tool | Output |
|---|---|---|---|
| 1 | **GAM property registry** (192 properties, 6 Data classes) | `extract_gam_params.py` | `config/<bin>.gam_params.{json,csv}`, `build/wire/<bin>.gam_params.md`, `include/net/gam_registry.h` |
| 2 | **GAM PARAMNAME dispatchers** (192/192 names recovered) | `extract_paramnames_dispatch.py` | `build/wire/<bin>.paramnames.md`, `config/<bin>.paramnames_resolved.json` |
| 3 | **Down opcode → handler map** (211 opcodes, 3 channels) | `extract_opcode_dispatch.py` | `build/wire/<bin>.opcodes.md` |
| 4 | **Up opcode reconnaissance** (CPB constructor inventory, RX-opcode validation) | `extract_up_opcodes.py` | `build/wire/<bin>.up_opcodes.md` |
| 5 | **MurmurHash2 validation** (FUN_00d31490 ↔ garlemald's `murmur_hash2`) | `validate_murmur2.py` | `docs/murmur2.md`, integration tests |
| 6 | **CharaMakeData parse-side validation** | `validate_chara_make.py` | `build/wire/<bin>.chara_make_validation.md` (4 surfaced bugs) |
| 7 | **CharacterListPacket build-side validation (schema-level)** | `validate_chara_list.py` | `build/wire/<bin>.chara_list_validation.md` (5 schema flags pending byte-layout confirmation) |
| 8 | **LobbyCryptEngine 9-slot decode + Blowfish validation** | `extract_crypt_engine.py` | `build/wire/<bin>.crypt_engine.md` |
| 9 | **Lobby Recv/Send paths** (CPB 4-slot vtable, BasePacketHeader, Down dispatcher) | hand-written | `include/net/lobby_proto_channel.h` |

**Resolved key questions:**

- **Cipher**: lobby uses statically-linked OpenSSL Blowfish (`BF_set_key` / `BF_encrypt` / `BF_decrypt` at known RVAs). P/S init constants are canonical pi-derived (Schneier 1993), confirmed bit-for-bit. Garlemald's `common/src/blowfish_tables.rs` matches byte-for-byte; the `MOVSX` byte-cycling quirk in the key schedule is reproduced in garlemald's `i8 as i32 as u32` cast.
- **Zone/chat encryption**: NONE. Only lobby uses Blowfish — confirmed by RTTI sweep (no concrete CryptEngine subclass for zone/chat) AND by absence of blowfish call sites in garlemald's world-/map-server.
- **32-byte alignment quirk** (lobby slots 6/7 round length DOWN to multiples of 32): benign in practice. Trailing 0–31 bytes the client fails to decrypt always fall inside the over-provisioned trailing zero padding of fixed-capacity buffers (`MemoryStream(0x98)`, `vec![0u8; 0x280]`, etc.). Garlemald's 8-aligned `encipher` is correct as-written.
- **BasePacketHeader layout**: 16 bytes, byte-for-byte aligned with garlemald's `common/src/packet.rs::BasePacketHeader`. The CPB::BuildHeader writes `[0]=0x14, [1]=0x00, [2..4]=connection_type, [8..12]=u32 timestamp` (Lobby/Zone) or `[8]=0x0A` (Chat hardcodes). Bytes 4..7 (packet_size + num_subpackets) and 12..15 are caller-populated.

### Remaining Phase 3 / 4 work (open, in priority order)

- **Push `InstallUnpacker::Unpack` (FUN_00cc6700) GREEN** — biggest remaining Phase-4 target (490 B, 49.8 % match at iter #2). Iter #2 fixed the frame size to `0xe0` (matches orig); remaining gap is MSVC register-allocator divergence. See [`docs/install-unpacker.md`](docs/install-unpacker.md).
- **Push `ChunkSource::AcquireChunk` GREEN** — 144/144 with 21 byte mismatches; cookie / register-allocation iteration.
- **Push `Utf8String::Reserve` + `Utf8StringAlloc` GREEN** — `Utf8StringAlloc` at 222 vs orig 225 (3 B short due to MSVC's "shared ADD" optimization), `Utf8String::Reserve` at 144/153. (`Utf8StringFree` ✅ GREEN as of 2026-05-02 — see commit `06ef7dd24`.)
- **Sweep more cluster patterns** in `derive_templates.py` — every new pattern unlocks 13–406 GREEN templates.
- **`FUN_00891f00` decompile** to close the 5 chara-list field-type flags (`current_level: u16` vs `mainSkillLevel: i8`, etc.) — chara-list packet structure was confirmed correct on 2026-05-02, only the field types inside each 464-byte entry remain unverified.
- **Full Up-opcode enumeration** — current pass validates that all garlemald `OP_RX_*` constants appear as PUSH immediates in `.text`, but per-callsite arg propagation (the canonical mapping) is deferred pending Ghidra-driven analysis.
- **`LobbyCryptEngine::vtable[6/7]` callsite trace** — would close out the alignment quirk by definitively showing what `len` arg is passed in retail.

**Closed (don't re-suggest):**
- ✅ ~~Apply chara-make patches~~ — landed 2026-05-01; see `build/wire/ffxivgame.chara_make_validation.md § Patch history`.
- ✅ ~~Find chara-list deserializer~~ — `FUN_00da76b0` confirmed 2026-05-02; garlemald's packet structure is architecturally correct.

## Quickstart

```sh
# 0. Symlink original binaries from the workspace install (does NOT copy)
#    + dump PE structure (sanity check):
make bootstrap

# 1. Static-analysis pipeline. Requires Ghidra 12 + JDK 21
#    (`brew install ghidra` pulls openjdk@21).
make split BINARY=ffxivlogin.exe   # ~30s — sanity check
make split BINARY=ffxivgame.exe    # ~30-60 min on Apple Silicon

# 2. Inspect the work pool:
make progress

# 3. Run the Phase 3 extraction / validation pipeline:
make extract-net                   # net-class vtable → fn_rva map
make extract-gam                   # GAM property registry
make extract-paramnames            # PARAMNAME dispatchers (192/192)
make emit-gam-header               # include/net/gam_registry.h
make extract-opcodes               # Down opcode → vtable-slot
make extract-up-opcodes            # Up CPB ctor inventory
make extract-crypt-engine          # LobbyCryptEngine 9-slot decode
make validate-murmur2              # MurmurHash2 vectors
make validate-chara-make           # chara_info.rs ↔ GAM CharaMakeData
make validate-chara-list           # build_for_chara_list ↔ GAM ClientSelectData
```

After `make split`:
- `asm/<binary>/<rva>_<symbol>.s` — one file per function
- `config/<binary>.symbols.json` — function list with sizes / sections
- `config/<binary>.strings.json` — strings + seed-hint flags (`__FILE__`,
  `__FUNCTION__`, Lua callbacks)
- `config/<binary>.rtti.json` — recovered class names + vtable RVAs
- `config/<binary>.vtable_slots.jsonl` — per-vtable function-pointer map
- `config/<binary>.yaml` — work pool, one row per function

After Phase 3 extractions:
- `build/wire/<binary>.*.md` — per-subsystem ground-truth reports (these are
  generated, not committed; rerun `make` to refresh)
- `include/net/*.h` — C++ headers that capture validated wire layouts; safe
  to `#include` from garlemald-server / garlemald-client to pin field
  offsets against the binary

## Phase 2 — matching toolchain (operational)

Toolchain installation handled by `vstudio2005-workspace/install.sh`
(VS 2005 Express RTM via msitools, bypassing Wine's broken msiexec)
+ `install-psdk.sh` (Platform SDK 2003 R2). Detail in
[`docs/msvc-setup.md`](docs/msvc-setup.md). Once installed:

```sh
# 1. Configure (one-time):
echo 'export MSVC_TOOLCHAIN_DIR="$HOME/sdk/msvc-2005"' \
    > ~/.config/meteor-decomp.env

# 2. Verify:
make setup-msvc                # cl.exe + PSDK + objdiff reachable

# 3. Match attempts:
make rosetta                   # compiles src/ffxivgame/_rosetta/*.cpp,
                               # diffs against the binary slice
make rosetta-bulk              # never-bails variant for stamped sweeps
```

The match-or-iterate loop is documented in
[`docs/matching-workflow.md`](docs/matching-workflow.md). The recipe
that landed the first GREEN match (`FUN_004165b0`) — Ghidra-decompiler-
assist + 3 MSVC-2005 source-pattern tricks — is in
[`reference_meteor_decomp_rosetta_match.md`](../../../.claude/projects/-Users-swstegall-Documents-Programming-server-workspace/memory/reference_meteor_decomp_rosetta_match.md).

For batch matching, the **template-derivation pipeline** (Phase 2.5):

```sh
make cluster-shapes            # group functions by byte-shape mod relocs
make stamp-clusters            # apply matching templates to all members
make validate-clusters         # re-validate against the binary
```

See [`docs/decomp-status.md § Phase 2.5`](docs/decomp-status.md) for the
full pipeline.

## Why "meteor-decomp"?

Project Meteor is the long-running effort to revive FFXIV 1.x — the
C# server (`project-meteor-server`), the launchers, the dataminers.
This subproject is the missing piece: a first-party reading of the
client itself, so the rest of the workspace stops reverse-engineering
through capture-and-guess and starts working from the source.

Project Meteor's C# server has been working against the real 1.x
client for years and its byte layouts are valuable observational
evidence — but they are **not authoritative**. Where this project
flags a divergence, the binary's actual deserializer is the tie
breaker, not Meteor's encoder.

## Original binaries are NOT in this repo

Square-Enix-copyright `.exe` files belong only in
`ffxiv-install-environment/target/prefix/.../FINAL FANTASY XIV/`.
`tools/symlink_orig.sh` makes them visible under `orig/` for the
build pipeline. Never `git add` them.

## License

AGPL-3.0-or-later (matches `garlemald-server` / `garlemald-client`).
See [LICENSE.md](LICENSE.md) and [NOTICE.md](NOTICE.md).
