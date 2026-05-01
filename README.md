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

This is a hybrid effort: pure matching decomp (Phase 2, byte-identical
recompilation) is gated on a complete VS 2005 SP1 toolchain procurement;
in the meantime, the **functional/static-analysis track** (Phase 3) has
already produced enough wire-level ground truth to validate
`garlemald-server` (the Rust port) byte-for-byte against the binary in
several subsystems.

| Area | Status | Output |
|---|---|---|
| **Phase 0 — bootstrap** | ✅ done | `make bootstrap`: PE structure dump |
| **Phase 1 — Ghidra import + work pool** | ✅ done | 9,729 vtable slots / 576 net-relevant classes (`config/<bin>.*.json`, `asm/<bin>/*.s`) |
| **Phase 2 — matching toolchain** | 🟡 partial | `cl.exe` runs under CrossOver Wine 9 on Apple Silicon (`make rosetta` produces real diffs); waiting on Platform SDK 2003 R2 + libcmt MUMSI variant for byte-identical matches. Rosetta candidate `FUN_00b361b0` staged. |
| **Phase 3 — functional decomp** | 🟢 substantial — see below | `tools/extract_*.py`, `build/wire/*.md`, `include/net/*.h` |

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

### Remaining Phase 3 work (open)

- **CharacterListPacket byte-layout decompilation** — the abstract `LobbyProtoDownCallbackInterface` has only ONE concrete subclass in the binary: `LobbyProtoDownDummyCallback@LobbyClient` (RTTI confirmed), whose slot 5 (where opcode 0x0D would dispatch) is a `RET 0xc` no-op stub. The real chara-list deserializer is reachable but not anchorable through purely-static Python xref scans — likely lives behind indirect calls in the lobby state machine (`MyGameLoginCallback`-adjacent code). Two paths forward: GUI-Ghidra interactive xref walk, or capture-and-decrypt empirical observation. See [docs/decomp-status.md](docs/decomp-status.md).
- **Apply 4 surfaced chara-make bugs** to garlemald-server (`build/wire/<bin>.chara_make_validation.md § Suggested patch`).
- **Full Up-opcode enumeration** — current pass validates that all garlemald `OP_RX_*` constants appear as PUSH immediates in `.text`, but per-callsite arg propagation (the canonical mapping) is deferred pending Ghidra-driven analysis.
- **`LobbyCryptEngine::vtable[6/7]` callsite trace** — would close out the alignment quirk by definitively showing what `len` arg is passed in retail.
- **Phase 2 closure** — VS 2005 SP1 + Platform SDK 2003 R2 procurement (legal copy required); once installed, iterate `MSVC_FLAGS` until `objdiff` reports zero delta on `FUN_00b361b0`.

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

## Phase 2 — matching toolchain

Phase 2 needs VS 2005 SP1 `cl.exe` (linker version 8.0 — see
[`docs/compiler-detection.md`](docs/compiler-detection.md)). Microsoft
no longer redistributes it; obtain from MSDN subscription, archive.org,
or LEGO-Island-decomp's recipe (see
[`docs/msvc-setup.md`](docs/msvc-setup.md)). Once installed:

```sh
# 1. Configure (one-time):
echo 'export MSVC_TOOLCHAIN_DIR="$HOME/sdk/msvc-2005-sp1"' \
    > ~/.config/meteor-decomp.env

# 2. Verify:
make setup-msvc                # all checks should pass

# 3. First match attempt:
make rosetta                   # compiles src/ffxivgame/_rosetta/*.cpp,
                               # diffs against the binary slice
```

Iterate `MSVC_FLAGS` in `Makefile` until `objdiff` reports zero delta
on the staged Rosetta function (currently `FUN_00b361b0` — 86 bytes,
unrolled 32-byte block-copy loop, no calls or FP).

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
