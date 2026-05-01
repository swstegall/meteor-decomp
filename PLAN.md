# meteor-decomp — Decompilation plan for FINAL FANTASY XIV 1.23b

This is the master plan for a decompilation of the FINAL FANTASY XIV 1.x
(specifically patch 1.23b) Windows client. The end goal is a readable,
buildable, and — for the parts that matter — byte-matching C/C++
re-derivation of the five shipped PE binaries, scoped to whatever depth
is useful for the rest of the workspace (`garlemald-server`, `garlemald-client`,
`project-meteor-server`, `SeventhUmbral`).

## 1. Goals and non-goals

### Goals
- Recover **enough** of the client's C/C++ source to:
  1. Document every wire-protocol packet (opcodes, struct layouts,
     bitfield meanings) — this directly unblocks `garlemald-server`.
  2. Document the in-memory game-state layout (Actor, Inventory, Map,
     Director, Quest, Battle) so `garlemald-server` and the LSB
     cross-reference effort have a ground-truth reference instead of
     spreadsheet folklore.
  3. Document the file-format layer (sqpack hashing, `.dat`/`.idx`,
     ZiPatch, BattleCommand.csv decoders, BGM/cutscene format) so
     tooling miners (`ffxiv-mozk-tabetai-miner`, `mirke-menagerie-miner`,
     `wiki-scraper`) can be replaced with first-party readers.
  4. Recover combat / damage / hit-rate / status-effect formulas as
     compilable C — calibration ground truth for the
     `battle-command-parser`-derived tables and the `youtube-watcher`
     atlas damage samples.
  5. Provide a reverse-engineered open-source client capable enough
     to drive automated client-side replay against `garlemald-server`
     without depending on Wine + the original `.exe`.

### Non-goals (for now)
- **Shipping a playable open-source client.** Renderer and audio
  paths are huge time-sinks and are already covered well enough by
  the original .exe + WineD3D for our automation purposes
  (`ffxiv-actor-cli`).
- **A 100 % matching decomp of every binary.** Fully matching 11.8 MB
  of MSVC 2005 `.text` is many person-years. We prioritise *function
  matching* on the high-value subsystems above and accept *functional
  (non-matching)* re-derivation for the long tail (UI, settings,
  installers, telemetry).
- **Decompiling `MSSMIXER` / Miles Sound System / DirectX wrapper
  glue / VC++ runtime / STL.** These are off-the-shelf middleware. We
  identify them, exclude them from the work-pool, and link against
  prebuilt equivalents.

## 2. Binary inventory

All five shipped PEs (from `ffxiv-install-environment/target/prefix/.../FINAL FANTASY XIV/`):

| Binary             | Size (bytes) | Linker | Build timestamp     | Image base | Entry RVA   | `.text` size | Sections |
|--------------------|-------------:|--------|---------------------|-----------:|-------------|-------------:|---------:|
| `ffxivgame.exe`    | 15,996,808   | 8.0    | 2012-09-11 16:30:23 | 0x400000   | 0x5d4baa    | 11,784,192   | 6        |
| `ffxivboot.exe`    | 12,961,112   | 8.0    | 2010-09-16 11:46:54 | 0x400000   | 0x507a6a    | 9,527,296    | 6        |
| `ffxivconfig.exe`  | 3,471,240    | 8.0    | 2012-09-11 16:37:31 | 0x400000   | 0x1dec0     | 303,104      | 5        |
| `ffxivupdater.exe` | 640,344      | 8.0    | 2010-09-16 11:42:12 | 0x400000   | 0x3fa4b     | 434,176      | 5        |
| `ffxivlogin.exe`   | 403,296      | 8.0    | 2011-01-28 09:25:41 | 0x400000   | 0x26838     | 258,048      | 4        |

All are PE32, `IMAGE_FILE_MACHINE_I386` (0x14c), GUI subsystem,
`Characteristics=0x103`. Linker version 8.0 means the **toolchain is
Visual Studio 2005** (MSVC `_MSC_VER=1400`, `cl.exe` 14.00.x). That
identification is load-bearing for matching decomp; see §4.

`ffxivgame.exe` sections (the prize):

```
.text       vaddr=0x00001000  vsize=0xb3b56d  rsize=0xb3c000  RX
MSSMIXER    vaddr=0x00b3d000  vsize=0x00006d  rsize=0x001000  RX  -- Miles Sound System mixer (off-the-shelf)
.rdata      vaddr=0x00b3e000  vsize=0x326032  rsize=0x327000  R
.data       vaddr=0x00e65000  vsize=0x117940  rsize=0x0bf000  RW
.tls        vaddr=0x00f7d000  vsize=0x0000a9  rsize=0x001000  RW
.rsrc       vaddr=0x00f7e000  vsize=0x01a54c  rsize=0x01b000  R
```

Priority order for decomp work: `ffxivgame.exe` first (everything
gameplay), `ffxivboot.exe` second (its bytes-9.5 MB `.text` is mostly
the launcher GUI but it embeds the early-network code path that the
real game inherits a lot of from), then the small binaries
opportunistically.

## 3. Strategy: hybrid matching + functional decomp

There are three established models; we adopt a **hybrid**.

| Model                  | Output                          | Difficulty | Verifiability                                | Examples                        |
|------------------------|---------------------------------|------------|----------------------------------------------|---------------------------------|
| Matching decomp        | Byte-identical `.exe`           | Highest    | `objdiff` zero-diff per function             | LEGO Island, OoT, FF7-decomp    |
| Functional decomp      | Equivalent C, any compiler      | Medium     | Behavioural test (round-trip a packet, etc.) | most game-engine REs            |
| Fully-RE / clean-room  | New code, original spec only    | Low–Medium | None per-function — only end-to-end          | OpenRA, OpenMW, ScummVM         |

Our hybrid:

- **Matching for the wire/protocol/file-format layer.** Packet
  encoders/decoders, sqpack readers, ZiPatch, BattleCommand parser,
  Blowfish/cipher routines. These are small (≤ 1 KB per function),
  numerically dense, and *correctness is checkable byte-for-byte* by
  re-encoding a known input. Exactly the cases where matching is
  cheap and pays off: the moment `objdiff` is green, the function is
  unambiguously correct.
- **Functional for game-state and gameplay logic.** Battle math,
  status effects, mob AI, quest/event scripting host, director
  framework. Re-derive into clean C++ with whatever helpers we want;
  match against behavioural fixtures (saved packet captures, save
  states from `data-backups/`, OCR damage samples in
  `ffxiv_youtube_atlas_context.md`).
- **Excluded** middleware: Miles Sound System (`MSSMIXER` + linked
  `mss32.dll`), DirectX 9 wrappers, MSVC C/C++ runtime, ATL/MFC
  fragments, RSA/CryptoAPI shims, CRT zlib. Identified and skipped.
- **Renderer**: deferred. We document the call-graph + buffer
  layouts so `garlemald-client` could, if it ever wants, re-implement
  on Vulkan/Metal.

Function matching uses a pinned **MSVC 14.00.50727.42** (VS 2005 RTM)
or **14.00.50727.762** (VS 2005 SP1) `cl.exe` running under Wine on
Apple Silicon, mirroring the LEGO Island decomp setup. SP1 is the
working hypothesis; we confirm by matching a hand-picked "Rosetta
Stone" function (a small cdecl strchr-like helper) against both and
seeing which matches with default `/O2` flags.

## 4. Toolchain plan

### Static analysis
- **Ghidra 11.x** (free, scriptable, headless mode, JDK 17, runs
  natively on Apple Silicon). Primary disassembler + decompiler.
  Project: `meteor-decomp/build/ghidra/ffxivgame.gpr`.
- **rizin / cutter** as a secondary opinion — its decompiler often
  recovers different control-flow shapes than Ghidra and the diff
  is informative.
- **objdump (LLVM)** — sanity check section layouts, run after every
  rebuild.
- **decomp.me** — function-level collaboration; we register a "msvc
  2005 x86" preset and post bite-sized functions there for matching.

### Diff + verification
- **objdiff** (`https://github.com/encounter/objdiff`) — the
  matching-decomp standard. Cross-platform, reads PE/PDB/ELF, gives
  per-function delta highlights. Configured with the same MSVC
  toolchain.
- **`tools/compare.py`** — wraps `objdiff` for batch runs and dumps a
  CSV of per-function status (matched / partial / unmatched / TODO).

### Compiler under Wine
- VS 2005 SP1 `cl.exe` + `link.exe` from Microsoft's archive, plus
  the matching SDK (`Windows Server 2003 SDK / Platform SDK 2003 R2`).
- Wrapped by `tools/cl-wine.sh` so Make/Ninja can invoke it on macOS.
- The MSVC runtime headers we ship are the *VS 2005 SP1 headers*.
  Modern STL won't match.

### Disassembly + splitting
- There is **no `splat`-equivalent for x86 PE** at the maturity of
  the MIPS ecosystem. We roll our own:
  - `tools/extract_pe.py` — parse `IMAGE_NT_HEADERS`, list sections,
    dump per-section binaries.
  - `tools/ghidra_scripts/DumpFunctions.java` — Ghidra headless
    script that walks `currentProgram.getFunctionManager()` and
    writes one `asm/<rva>_<symbolname>.s` per function plus a JSON
    symbol map. (Java, not Jython — Ghidra 12 dropped Jython 2.7.)
  - `tools/build_split_yaml.py` — generate `config/ffxivgame.yaml`
    listing every `(start_rva, end_rva, name, status)` tuple — the
    work-pool for the project.

### Symbol seed sources
We do NOT have a PDB. But we have several side-channels:
- **Strings + RTTI**: Ghidra's `RTTI Analyzer` recovers C++ class
  names + vtable layouts from PE32 MSVC binaries; this gives us
  free names for hundreds of classes. (1.x predates `/GR-` being
  default-off, so RTTI is fully present in the binary.)
- **Function-name leaks via `__FILE__` / `__FUNCTION__` macros** —
  any `assert`/`Verify`/`MES_LOG` call on a hot path embeds the
  function name as a literal string in `.rdata`. Run
  `strings -td .rdata | grep -E '\\.(cpp|h)'` to grep them.
- **Project Meteor's C# server** — the `project-meteor-server` and
  variant trees are reverse-engineered from the same client; their
  symbol naming (`SetActorPropertyPacket`, `WeaponSkill`, etc.)
  is our naming convention.
- **`SeventhUmbral` workspace** — the upstream C++ launcher already
  has reversed packet structs we can import.
- **`ffxiv-actor-cli/logs/*.log`** + per-region capture dirs —
  packet-level traces with annotated opcodes (already in the
  workspace from earlier sessions).
- **`battle-command-parser` decoded enums** — every BattleCommand
  field maps to a struct member in the binary; the column legends
  in `BattleCommand.csv` *are* the field-name source.
- **Gamer Escape / Fandom / Console Games wikis** — see CLAUDE.md;
  zone IDs, weather IDs, NPC IDs, item IDs. These appear in `.rdata`
  as numeric constants and let us pin functions ("the function that
  references zone ID 166 must touch Gridania").

## 5. Repository layout

```
meteor-decomp/
├── PLAN.md                       <- this file
├── README.md                     <- quickstart for contributors / agents
├── LICENSE.md                    <- license for OUR original work
├── NOTICE.md                     <- crediting upstreams + stating clean-room status
├── .gitignore                    <- excludes orig/, build/, ghidra projects
├── Makefile                      <- top-level: split / build / diff
├── orig/
│   ├── README.md                 <- "do not commit binaries; symlink from ffxiv-install-environment"
│   └── (symlinks to the five .exe files, populated by tools/symlink_orig.sh)
├── asm/
│   ├── ffxivgame/                <- one .s per function, named <rva>_<symbol>.s
│   ├── ffxivboot/
│   └── ...
├── src/
│   ├── ffxivgame/
│   │   ├── net/                  <- packet encoders/decoders (matching target)
│   │   ├── sqpack/               <- file-format readers (matching target)
│   │   ├── battle/               <- combat math (functional target)
│   │   ├── director/             <- event/quest framework (functional target)
│   │   ├── actor/                <- Actor hierarchy (functional target)
│   │   ├── ui/                   <- HUD/menus (deferred)
│   │   └── render/               <- DX9 binding (deferred)
│   └── ...
├── include/                      <- headers shared across decomp targets
├── config/
│   ├── ffxivgame.yaml            <- function work-pool (rva ranges + status)
│   ├── symbols.txt               <- known symbols (manual + extracted)
│   ├── strings.json              <- extracted .rdata strings keyed by RVA
│   └── rtti.json                 <- recovered RTTI class names + vtables
├── tools/
│   ├── setup.sh                  <- one-shot: ghidra + JDK + wine + msvc + objdiff
│   ├── symlink_orig.sh           <- populate orig/ from ffxiv-install-environment
│   ├── extract_pe.py             <- dump PE structure + per-section binaries
│   ├── import_to_ghidra.py       <- headless Ghidra import + analysis (Java scripts)
│   ├── ghidra_scripts/
│   │   ├── DumpFunctions.java    <- export every function as asm/symbol map
│   │   ├── DumpStrings.java      <- .rdata strings → config/<bin>.strings.json
│   │   └── DumpRtti.java         <- RTTI → config/<bin>.rtti.json
│   ├── build_split_yaml.py       <- ghidra dump → config/ffxivgame.yaml
│   ├── cl-wine.sh                <- wraps VS2005 cl.exe under Wine
│   ├── compare.py                <- objdiff batch runner → CSV report
│   └── progress.py               <- count matched / partial / unmatched
├── docs/
│   ├── pe-layout.md              <- the 6-section breakdown above
│   ├── compiler-detection.md     <- how we pinned MSVC 8.0 → VS 2005 SP1
│   ├── matching-workflow.md      <- per-function workflow
│   ├── known-libraries.md        <- Miles, DX9, CRT — what to ignore
│   ├── seed-symbols.md           <- where each name in symbols.txt came from
│   └── prior-art.md              <- LEGO Island, OoT, FF7-decomp pointers
└── build/                        <- gitignored
    ├── ghidra/
    ├── obj/
    └── reports/
```

Module priorities (highest to lowest), with a sketch of the public
surface we expect to recover for each:

1. **`net/`** — opcode constants, packet base class hierarchy,
   Blowfish cipher, packet header (CRC/sequence/etc.). Unblocks
   garlemald-server's wire layer immediately.
2. **`sqpack/`** — `Sqpack::Hash`, `.dat`/`.idx` index lookup,
   ZiPatch unpacking. Replaces the workspace's hand-coded sqpack
   readers.
3. **`actor/`** — Actor base class, ActorParam tables, motion-pack
   IDs. Cross-references `ffxiv_1x_battle_commands_context.md` and
   `ffxiv_mozk_tabetai_context.md`.
4. **`battle/`** — damage formula, hit/crit roll, status-effect
   ticks, Battle Regimens. Calibrates against
   `ffxiv_youtube_atlas_context.md` damage samples.
5. **`director/`** — OpeningDirector, QuestDirector, ZoneDirector,
   WeatherDirector, ContentArea, PrivateArea. Cross-references
   garlemald's existing director scaffolding.
6. **`ui/`**, **`render/`** — deferred.

## 6. Phased roadmap

### Phase 0 — Bootstrap (this PR)
- Scaffold directory tree (this commit).
- Write PLAN.md (this file), README, .gitignore, NOTICE.
- Provide `tools/extract_pe.py` working today (no Ghidra/Wine yet).
- Provide `tools/symlink_orig.sh` so binaries don't have to be
  copied or committed.
- Document the PE-layout findings (`docs/pe-layout.md`,
  `docs/compiler-detection.md`).
- **Exit criterion**: a fresh clone + `make bootstrap` populates
  `orig/` and runs `tools/extract_pe.py` cleanly.

### Phase 1 — Static-analysis pipeline ✅ COMPLETE 2026-04-30
- Install Ghidra 12.0.4 + JDK 21 via `brew install ghidra` (pulls
  `openjdk@21` as a dep). JDK 25 happens to also work but the brew
  formula targets 21.
- Ghidra headless wrapper at `tools/import_to_ghidra.py` calls
  `support/launch.sh` directly (so we can override the brew default
  `MAXMEM=2G` — the 16 MB `ffxivgame.exe` needs ~6 GB to analyse;
  the wrapper defaults to 8 GB).
- Three Java post-scripts in `tools/ghidra_scripts/`:
  `DumpFunctions.java`, `DumpStrings.java`, `DumpRtti.java`.
  (Ghidra 12 dropped Jython 2.7; PyGhidra is opt-in / venv-only.
  Java is the path of least resistance for headless.)
- `tools/build_split_yaml.py` folds the three JSON dumps into
  `config/<binary>.yaml` — the work-pool.
- **Exit criterion** ✅: `make split` produces `asm/<binary>/` with
  one .s per function and `config/<binary>.yaml` listing every
  function with status=`unmatched` (or `matched` for auto-classified
  middleware).

### Phase 2 — Toolchain pinning ▶ working toolchain; matching iteration ongoing
- ✅ `tools/find_rosetta.py` scans the binary for the best Rosetta
  Stone candidate. For ffxivgame.exe the top pick is
  `FUN_00b361b0` at RVA 0x007361b0 (86 bytes, 31 integer ops, no
  calls / no FP / no SEH — score 80 of 1,789 valid candidates).
  Disassembly cached at `build/rosetta/ffxivgame.top.txt`; full
  ranked list at `build/rosetta/ffxivgame.candidates.json`.
- ✅ `src/ffxivgame/_rosetta/FUN_00b361b0.cpp` is the contributor's
  starting C draft (Ghidra-derived, annotated).
- ✅ `tools/cl-wine.sh` wraps `cl.exe` / `link.exe` under Wine —
  reads `MSVC_TOOLCHAIN_DIR` from `~/.config/meteor-decomp.env`,
  sets `INCLUDE` / `LIB`, dispatches via Wine's `Z:` drive.
- ✅ `tools/setup-msvc.sh` verifies Wine + cl.exe + objdiff are
  reachable and the cl.exe version is "Microsoft … 14.00.x".
- ✅ `make rosetta` compiles every staged `_rosetta/*.cpp` and
  invokes `tools/compare.py` for the diff.
- ✅ Procurement guide at [`docs/msvc-setup.md`](docs/msvc-setup.md)
  — MSDN subscriber downloads / archive.org / Microsoft Update
  Catalog / LEGO Island recipe.
- ✅ **VS 2005 Express RTM installed and operational** —
  `vstudio2005-workspace/install.sh` extracts `cl.exe` /
  `link.exe` / `c1.dll` / `c1xx.dll` / `c2.dll` / `mspdb80.dll`
  + headers + libs from the official VS2005EE ISO via msitools
  (bypasses Wine's broken msiexec). Reports
  `cl.exe Version 14.00.50727.42 for 80x86` running under
  CrossOver Wine 9 on macOS Apple Silicon. `make setup-msvc`
  passes (with two PSDK warnings — Express doesn't bundle
  Platform SDK; Rosetta Stone doesn't need it).
- ✅ **`tools/compare.py` byte-level diff implemented** — reads
  the `.text` section from `build/obj/_rosetta/<name>.obj`,
  reads the corresponding bytes from `orig/ffxivgame.exe` at
  the function's RVA, and prints a side-by-side hex diff with
  GREEN/PARTIAL/MISMATCH verdict + first-mismatch offset.
  Exit codes 0/1/2 let Make and CI gate on match status.
- ▶ **Matching iteration on `FUN_00b361b0` ongoing** —
  three iterations done so far. Toolchain is confirmed
  working (every iteration compiles cleanly to a valid 32-bit
  COFF i386 object); the matching gap is in the C source's
  ability to coax MSVC into the original's two-pointer
  addressing pattern (`EAX = dest, EDX = dest + 0x10` both
  advancing by 0x20 each iteration). Iteration history
  preserved in the .cpp file's leading comment.
- **Note on RTM vs SP1**: cl.exe `.42` is RTM, not SP1
  (`.762`). The FFXIV binary's linker version 8.0 is
  consistent with both. If the iteration loop doesn't reach
  GREEN under RTM, install SP1 separately and switch
  `MSVC_TOOLCHAIN_DIR` to its `sdk/`.
- **Exit criterion**: revised — Phase 2's exit is "the
  matching iteration loop is fully wired up and produces
  actionable diffs," not "the first Rosetta candidate is
  green on first try." Achieving a byte-matched function
  is now ongoing decomp work, not a Phase-2 blocker. Future
  matching wins land as individual commits to the relevant
  module's `.cpp` files.

### Phase 3 — Net layer ▶ in progress (functional track; matching deferred)
- ✅ Wire architecture documented at
  [`docs/wire-protocol.md`](docs/wire-protocol.md):
  - Three IpcChannels: `LobbyProtoChannel`, `ZoneProtoChannel`,
    `ChatProtoChannel` (with Up/Down union types each).
  - Transport is **RUDP2** (Sqex::Socket::RUDP2 — SE in-house
    protocol, NOT raw TCP). Project Meteor's TCP impl works because
    of the launcher's `ws2_32.dll` shim.
  - Crypto is **OpenSSL 1.0.0 (29 Mar 2010)** statically linked
    (`Blowfish part of OpenSSL 1.0.0` string at .rdata RVA 0x4048).
    Blowfish is the per-channel cipher; OpenSSL's full crypto suite
    (RSA / AES / SHA1/256/512 / X.509) is also present for the
    SqexId auth flow.
  - 343 `Component::GAM::CompileTimeParameter<id, &PARAMNAME_id>`
    template instantiations recovered — that's the actor-property
    serialization registry, IDs 100-345 + 579-595.
- ✅ `tools/extract_net_vtables.py` walks the RTTI dump and emits
  a per-class slot map at `build/wire/<binary>.net_handlers.md`.
  For ffxivgame.exe: **576 net-relevant classes / 9,729 vtable
  slots**, each linked to the per-function `asm/<rva>_*.s` file —
  this is the Phase 3 work pool. Notable entries:
  - `LobbyCryptEngine` — 9 slots (the cipher API surface)
  - `MyGameLoginCallback` — 22 slots (login state machine)
  - `SqexIdAuthentication` — 1 slot
  - Three `Application::Network::*ProtoChannel` classes
  - `Sqex::Socket::RUDP2`, `RUDPSocket`, `PollerWinsock`,
    `PollerImpl`
  - Three `*ProtoChannel::ClientPacketBuilder` instances
  - `Sqex::Crypt::{Cert, Crc32, ShuffleString, SimpleString,
    CryptInterface}` — SE's higher-level crypto shims
- ✅ Cross-referenced with `garlemald-server`'s existing wire layer
  (`common/src/{packet,subpacket,blowfish}.rs`,
  `map-server/src/packets/opcodes.rs`) — the Rust impl already
  models the BasePacketHeader correctly, the opcodes registry is
  comprehensive, and Blowfish key schedule matches OpenSSL bf_init.
- ✅ **GAM property registry extracted** —
  `tools/extract_gam_params.py` parses 192 unique `(id, namespace,
  type, decorator)` tuples from the mangled CompileTimeParameter
  types in `.rdata`. Output:
  - `config/<binary>.gam_params.{json,csv}` (machine-readable)
  - `build/wire/<binary>.gam_params.md` (human-readable, grouped
    by namespace)
  Six Data classes recovered: Player (92), PlayerPlayer (37),
  CharaMakeData (26), ClientSelectData / ClientSelectDataN (17
  each), ZoneInitData (3).
  Important finding: the 343 `?PARAMNAME_<id>@...` symbols
  referenced in mangled CompileTimeParameter types do NOT carry
  user-meaningful property names — the actual strings in `.rdata`
  are generic placeholders (`IntData.Value0`, etc.). The
  (namespace, id) tuple IS the property identifier; semantic names
  (`playerWork.activeQuest`, etc.) are Project Meteor's invention.
- ✅ **Two-systems finding documented** — turns out garlemald's
  `SetActorPropertyPacket` and the binary's GAM
  `CompileTimeParameter` are *parallel* wire systems, not the same
  one (see [`docs/wire-protocol.md`](docs/wire-protocol.md) → "Two
  parallel actor-property systems"). The first uses 32-bit
  Murmur2 hashes of `/`-path strings; the second uses small
  ordinal ids per Data-class namespace. The original Phase-3
  task #3 (type-check `SetActorPropertyPacket` against GAM) was
  a category error.
- ✅ **`include/net/gam_registry.h`** — auto-generated C++
  `constexpr` schema declaring all 192 GAM parameters across 6
  Data classes. Each row carries `(id, TypeKind, element_size,
  total_bytes, raw_type)`. Generated by `tools/emit_gam_header.py`
  from `config/<binary>.gam_params.json`. Future Rust code can
  consume via FFI or build.rs codegen as the ground-truth schema
  for lobby-side type checking.
- ✅ **MurmurHash2 validated** — `FUN_00d31490` (RVA 0x00931490,
  170 bytes) is the binary's *backward-walking* MurmurHash2 used to
  derive 32-bit wire ids for `SetActorPropertyPacket` from
  `/`-path string keys (`"charaWork.parameterSave.hp[0]"` →
  `0x4232BCAA`). Bit-for-bit step-by-step trace against
  `garlemald-server/common/src/utils.rs::murmur_hash2` in
  [`docs/murmur2.md`](docs/murmur2.md), plus 6/6 test vectors
  cross-verified by a standalone Rust binary on 2026-05-01. The
  `SetActorProperty` wire-id derivation in garlemald is correct.
- ✅ **Down-channel opcode dispatchers extracted** —
  `tools/extract_opcode_dispatch.py` walks each `*ProtoDownDummyCallback`
  vtable slot 1 (the dispatcher; slots 0=destructor, 2..N=handlers),
  parses its prologue + two-level dispatch tables (byte_table maps
  opcode → case_index, dword_table → 21-byte case body, each loading
  `MOV EAX, [ESI + slot*4]`), and produces an opcode → vtable_slot
  map cross-referenced with garlemald-server's `opcodes.rs`. Output
  at `build/wire/<binary>.opcodes.md`. Headline numbers:
  - **211 total real Down opcodes** across 3 channels (Zone 197 of
    502 possible; Lobby 10 of 23; Chat 4 of 200).
  - **60 opcodes the binary handles that garlemald has no entry for**
    — potentially missing handlers in the Rust impl.
  - **5 opcodes garlemald has labelled RX-only that the binary
    handles in Down** — likely miscategorized in garlemald (need
    both directions or just TX).
  - **19 garlemald TX opcodes that don't appear in any Down channel**
    — server-invented opcodes the client doesn't actually handle, or
    out-of-scope ranges (e.g., the ≥ 0x1000 world-server↔map-server
    coordination opcodes).
  This is the canonical cross-reference for garlemald's
  `map-server/src/packets/opcodes.rs` going forward.
- 🟡 **Up direction reconnaissance** —
  `tools/extract_up_opcodes.py` documented the architectural finding
  that Up packets don't have a flat dispatcher analogous to Down.
  The opcode is stored at `[ClientPacketBuilder + 0x1C]` and set by
  the constructor (`MOV [this+0x1C], <stack-arg>`); each per-opcode
  send is a separate "send Xxx" function that builds a CPB and
  passes the opcode dynamically (not as a literal immediate the
  static scanner can recover). Validated that all 30 garlemald
  `OP_RX_*` values appear as PUSH immediates in `.text` (necessary
  but not sufficient — confirms no garlemald RX opcode is invented).
  Full Up-opcode enumeration requires Ghidra-driven cross-reference
  analysis (per-callsite constant propagation through the CPB
  constructor's arg0); deferred.
- ⏸ **Functional decomp — pending PRs**:
  1. ~~Map every Project Meteor `OP_*` constant to its handler
     vtable slot~~ — **done above** for the Down direction. Up
     direction reconnaissance done; full enumeration deferred.
  2. Decompile `LobbyCryptEngine`'s 9 slots and the
     `*ProtoChannel::Recv`/`Send` paths into C++ headers under
     `include/net/`.
  3. ✅ Cross-validate `garlemald-server/lobby-server/src/data/chara_info.rs`
     against the GAM CharaMakeData registry. **Definitive answer**
     in `build/wire/<binary>.chara_make_validation.md`:
     - The dispatcher fn in `CharaMakeData::MetadataProvider`
       (vtable slot 2 at RVA 0x001ad010) is a 26-way jump table
       that maps GAM id → property name string in `.data`.
       `tools/extract_paramnames_dispatch.py` walks it to
       recover the names: `tribe`, `size`, `hair`, `hairOption1`,
       …, `initialBonusItem`, `initialTown`. All 26 resolved.
     - Player's dispatcher (slot 2 at RVA 0x001add90) similarly
       resolves all 92 GAM ids to names like `guildleveId`,
       `craft_assist_buff_type`, `anima`, `companyId`,
       `latest_aetheryte`, etc. Property categories: ~20
       `guildleve*` fields, ~14 `company*` (Free Company state),
       7 `snpc*` (1.x Soul-Sync NPCs), 5 `harvest_*` (gathering
       history), 5 `profile_*` (item/monster encounter logs),
       Anima resource, mailbox, festival counters.
     - ClientSelectData's slot 2 (RVA 0x001ad580) is global-id
       keyed and resolves all 17 names — `displayName`,
       `graphics`, `loginFlag`, `mainSkill`, `mainSkillLevel`,
       `physicalLevel` (Utf8String — pre-formatted display
       string), `tribe` (also Utf8String — race display name),
       `zoneName`/`territoryName` (1-byte ID codes with string
       lookup elsewhere), `guardian`, `birthdayMonth/Day`,
       `initial*`, `currentJob`. Garlemald-server's
       `lobby-server/src/data/chara_info.rs::build_for_chara_list`
       produces this payload as a hand-rolled flat blob; the
       schema cross-reference is in
       `build/wire/<binary>.chara_list_validation.md` (regenerable
       via `make validate-chara-list`). Five likely bugs surfaced:
       * `current_level: u16` writes 2 bytes where GAM says 1
         (`mainSkillLevel: signed char`).
       * `tribe: u8` written where GAM declares `Utf8String`.
       * `location1/2`: full strings written where GAM declares
         1-byte IDs (this one's less certain — wire form may
         legitimately differ from in-memory).
       * `initial_town: u32 (twice)`: 4-byte write × 2 where GAM
         declares one 2-byte field. The duplicate is likely a
         port bug masking a separate field (favourite_aetheryte?).
       Definitive resolution requires decompiling the binary's
       `CharacterListPacket::Deserialize` — TBD.
     - ClientSelectDataN's slot 2 (RVA 0x001ad990) is global-id
       keyed (contiguous ids 100..116, no gaps) and resolves 17
       names. Same fields as CSD but renumbered — likely a later
       wire-format revision; both versions kept in the binary.
     - ZoneInitData's slot 2 (RVA 0x001af640) uses a chained-
       compare cascade rather than a JT (more compact for 3 ids).
       Resolves `startCity` (int), `zoneName` (Utf8String),
       `zoneId` (i8).
     - PlayerPlayer's slot 4 (RVA 0x001af270) extracts 37 names
       (`tribe`, `size`, `hair`, …, `loginCount`,
       `questScenario`, `questGuildleve*`, `npcLinkshellChat*`,
       `actionSaveWork`) but its dispatcher is **local-index-keyed**
       (no `ADD EAX, -<base>` in prologue), so pairing names with
       GAM ids requires decompiling slot 2 (RVA 0x001aee30) — TBD.
       Names are reported under "Local-index-keyed dispatchers" in
       `build/wire/<binary>.paramnames.md` and excluded from the
       gam_params enrichment until the local→global translator is
       resolved.
     - Pattern (dispatcher walk + the global-id vs local-index
       distinction) documented in memory at
       `reference_meteor_decomp_paramname_dispatcher.md` for
       reapplication to ClientSelectData / ClientSelectDataN /
       ZoneInitData.
     - The wire format IS GAM-id-ordered, with two non-GAM
       `u32 skip` sub-record headers (between ids 107 and 108,
       and between ids 115 and 116) and a 16-byte `seek 0x10`
       trailer.
     - garlemald's parser aligns field-for-field, surfacing
       three concrete bugs:
       * `appearance.face_features` should be `face_cheek` (id 112)
       * `appearance.ears` should be `face_jaw` (id 114) — 1.x
         doesn't expose ears as a separate slot
       * `info.current_class: u16` lumps GAM id 122 `initialMainSkill`
         + id 123 `initialEquipSet` (loses the equipment-set value)
       * Three trailing `u32 skip` reads ARE GAM id 124
         `initialBonusItem: int[3]` (starter items the parser
         silently drops)
     - Suggested patch in `build/wire/<binary>.chara_make_validation.md §
       Suggested patch`. Apply to garlemald-server when ready.
- **Exit criterion (unchanged)**: `garlemald-server` can replace
  its hand-written packet structs with `#include`-able C++ from
  `meteor-decomp/include/net/`, and round-trips a capture session.
- **Matching upgrade path**: when Phase 2 unblocks (VS 2005 SP1
  procurement), revisit each functional `.cpp` in `src/ffxivgame/net/`
  and re-derive matching codegen via `make rosetta`-style iteration.
  The functional source we ship now is the starting C, not the final.

### Phase 4 — Sqpack / ZiPatch (matching target)
- Decompile `Sqpack::Hash` — single-function landmark, easy to
  verify against known input/output pairs.
- Decompile index lookup, decompression (zlib wrapped).
- Decompile ZiPatch unpacker.
- **Exit criterion**: `tools/sqpack-cat <path>` extracts a known
  file from the install's `.dat` archives, byte-identical to what
  the original game produces.

### Phase 5 — Actor + Battle (functional target)
- Decompile Actor base class (vtable from RTTI), ActorParam tables.
- Decompile battle math: `ComputeDamage`, `ComputeHit`,
  `ComputeCrit`, `ApplyStatus`. Cross-check against LSB's XI
  cousins (see CLAUDE.md "Land Sand Boat" cross-reference) and the
  YouTube atlas damage samples.
- Status-effect tick loop.
- Battle Regimen (combo) resolver.
- **Exit criterion**: a self-contained `damage_simulator`
  executable in `src/ffxivgame/battle/` that reads a damage roll
  setup from JSON and produces a number that matches a recorded
  damage sample within ±1 (rounding tolerance).

### Phase 6 — Director / Quest framework (functional target)
- Decompile Director base class, ContentArea, PrivateArea, the
  `XML/CSV` content loader.
- Decompile the Lua VM glue (the client embeds Lua 5.x — already
  hinted at by `project_meteor_discord_context.md` — function
  names like `processEvent`, `onTalk`, `Seq000` will jump out).
- Decompile OpeningDirector, ZoneDirector, WeatherDirector.
- **Exit criterion**: garlemald can drive the original .exe
  through an opening-cinematic capture cycle using
  meteor-decomp-derived director sequencing instead of garlemald's
  current Lua scaffolding.

### Phase 7+ — Long tail
UI, render, audio, settings, telemetry. Fully optional.

## 7. Per-function workflow

Each function the project tackles follows the same loop. This is
designed so an automated agent (or a human) can pick up a row from
`config/ffxivgame.yaml` with status=`unmatched` and complete it.

1. **Claim**: change status to `wip` in `config/ffxivgame.yaml` so
   no one else picks the same function.
2. **Read assembly**: `asm/ffxivgame/<rva>_<sym>.s`. If the symbol
   is `FUN_xxxx` (Ghidra-generated), guess a better name from
   nearby strings, RTTI, or seed-symbol cross-references.
3. **Read decompiler output**: open in Ghidra, copy the
   pseudo-C, paste into `src/ffxivgame/<module>/<sym>.cpp`.
4. **Clean up**: replace `local_4`, `iVar1`, etc. with meaningful
   names. Replace integer constants with named enum values where
   possible. Add `#include`s.
5. **Match (matching modules)** or **smoke-test (functional)**:
   - Matching: `make diff FUNC=<sym>`. Iterate until `objdiff`
     reports zero delta. Common knobs: argument order, struct
     padding, `__cdecl` vs `__fastcall`, inline vs not, `/Oy`
     frame-pointer omission, `_alloca` vs `__chkstk`.
   - Functional: write a small `tests/<module>/<sym>_test.cpp`
     that exercises the function on a known input/output pair
     drawn from a packet capture, save state, or wiki dump.
6. **Update status**: change `unmatched`/`wip` →
   `matched`/`functional` in `config/ffxivgame.yaml`. Add a row to
   `docs/seed-symbols.md` if a new naming source was used.
7. **Commit**: one function per commit in the early days; group
   later when the work-pool is well-understood. Commit message:
   `decomp: match Sqpack::Hash @0x004a1230` or
   `decomp: functional ComputeDamage @0x008c5a40`.

## 8. Legal & copyright

The original PE binaries are copyright Square Enix. They MUST NOT
be committed to this repository. Contributors fetch their own copy
from a legitimate `ffxiv-install-environment` install (the
workspace's installer pipeline) and `tools/symlink_orig.sh` makes
them visible to the build without copying.

The decompiled C/C++ in `src/`, `include/`, and the YAML/JSON
config in `config/` are *original work derived through clean-room
reverse engineering of the binary's behaviour*. Everything we
publish here is licensed under **AGPL-3.0-or-later**, matching
`garlemald-server` and `garlemald-client`. New source files get the
standard AGPL header; see `CLAUDE.md` § "Source-file license
headers" — copy the header verbatim from a `garlemald-client`
sibling, with the project tagline:

```
meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
```

`NOTICE.md` credits Project Meteor Server (AGPL-3.0), Seventh
Umbral (BSD-2-clause-style), LandSandBoat (GPL-3.0 — referenced
only, no code copied), and the LEGO Island decomp project for
prior-art on MSVC 2005 matching.

## 9. Cross-references in this workspace

Lean on these at every step instead of re-deriving knowledge:

- `ffxiv_classic_wiki_context.md` — opcodes, region IDs, motion IDs.
- `project_meteor_discord_context.md` — first-hand notes from
  Ioncannon / Tiam / Decimus on packet field layouts.
- `ffxiv_linkchannel_context.md` — FFXIV 1.0 Opcodes spreadsheet,
  Actor Param Names spreadsheet, Motion IDs spreadsheet (mined).
- `ffxiv_1x_battle_commands_context.md` — every BattleCommand row
  is a struct member in the binary.
- `ffxiv_mozk_tabetai_context.md` — every item / shop / recipe ID
  appears as a numeric constant in `.rdata` and helps pin
  functions.
- `ffxiv_youtube_atlas_context.md` — damage samples for battle
  formula calibration.
- `mirke-menagerie-context.md` — quest dialogue text we'll see as
  literals in `.rdata`.
- `land-sand-boat-server/xi-private-server.md` — XI structural
  cousin for damage / aggro / status-effect grammar.
- `porting-progress-context.md` — the workspace's master roadmap;
  meteor-decomp slots in as a Tier 1 unblocker (the source-of-truth
  for net/sqpack), and a Tier 3 calibrator (battle formulas).

## 10. Definition of "done" for the workspace

We don't need to finish meteor-decomp. The workspace declares
victory when:

- garlemald-server's wire layer is generated from
  `meteor-decomp/include/net/`, not hand-rolled — every opcode
  matches.
- garlemald-server's Sqpack reader, ZiPatch reader, and
  BattleCommand reader are calls into `meteor-decomp` libs, not
  duplicates.
- garlemald-server's damage / hit / crit / status-tick formulas
  are line-for-line ports of `src/ffxivgame/battle/*.cpp`.
- The opening-cinematic and quest-framework events run through
  meteor-decomp-derived director sequencing rather than
  garlemald's current "best effort" Lua.

After that, anything else is bonus. If the long tail never gets
done, fine.
