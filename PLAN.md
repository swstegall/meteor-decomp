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

### Phase 2 — Toolchain pinning ⏸ scaffolding done; awaiting MSVC procurement
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
- ⏸ **Manual blocker**: VS 2005 SP1 `cl.exe 14.00.50727.762` +
  Platform SDK 2003 R2 SP1 must be obtained from a legitimate
  source (Microsoft no longer redistributes them). When the bits
  land at `$MSVC_TOOLCHAIN_DIR`:
    1. `make setup-msvc`           → all checks GREEN
    2. `make rosetta`              → first attempt; diff will be
       non-zero
    3. Iterate `MSVC_FLAGS` in `Makefile` — the
       `docs/matching-workflow.md §7` cheat-sheet covers the bag
       of tricks (RTM vs SP1 vs SP1-ATL, /Oy vs /Oy-, /GS, etc.)
       until objdiff reports zero delta.
    4. Lock the matching `MSVC_FLAGS` for the rest of the project.
- **Exit criterion (unchanged)**: at least one function in
  `src/ffxivgame/_rosetta/*.cpp` matches byte-for-byte under
  `objdiff`. The rest of the project compiles even though no
  other C source exists yet.
- **Fallback if procurement stalls indefinitely**: drop matching
  decomp on the contested modules and proceed with functional-only
  decomp (Phase 3 onward — see `docs/msvc-setup.md §6`). The bulk
  of the workspace's deliverables (wire opcodes, packet structs,
  battle math, director state machines) live in the functional
  tier and don't need a matching toolchain.

### Phase 3 — Net layer (matching target)
- Locate the packet-base vtable via RTTI (Project Meteor's C#
  `BasePacket` and the FFXIV 1.0 Opcodes spreadsheet from
  `ffxiv_linkchannel_context.md` are our seeds).
- Decompile Blowfish init / encrypt / decrypt — these are
  reference-implementation cribs, easy to match.
- Decompile packet header CRC + sequence handling.
- Decompile each known opcode encoder/decoder (~200 packets).
  Cross-check by re-encoding a saved capture from
  `ffxiv-actor-cli/logs/*.log` or `captures/` and byte-comparing.
- Each function lands as a PR with `objdiff: GREEN` evidence.
- **Exit criterion**: `garlemald-server` can replace its
  hand-written packet structs with `#include`-able C++ from
  `meteor-decomp/include/net/`, and round-trips a capture session.

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
