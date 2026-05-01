# Prior art — projects we learn from

We are far from the first to decompile an MSVC-era PE32 game binary.
The decomp community has converged on a handful of conventions that
this project adopts wholesale. Read at least one of these before
working on `meteor-decomp` so you recognise the idioms.

## Direct methodological references (matching MSVC PE)

### LEGO Island decompilation
- Repo: <https://github.com/isledecomp/isle>
- License: BSD-3-Clause
- Target: `ISLE.EXE` + `LEGO1.DLL` (Mindscape, 1997, MSVC ~6.0)
- Why it matters: closest analogue to our project. Same toolchain
  family (MSVC C++ with RTTI), same per-function `.cpp` layout
  keyed by RVA, same Wine + objdiff workflow. Their `tools/`
  directory is a ready-made template for ours.
- Specific patterns we adopt:
  - `// FUNCTION: LEGO1 0x10001234` magic comments above each
    function in the .cpp source so a Ghidra-style RVA→source
    cross-reference works.
  - The split YAML schema (we extend it).
  - The matching-vs-functional dual-track.

### LEGO Racers decomp
- Repo: <https://github.com/legoracers/LEGORacers>
- Same toolchain family, slightly newer (MSVC 6.0 with extensions).

### Tony Hawk's Pro Skater 1+2 decomp (THPS)
- Repo: <https://github.com/jasperdejong0/THPS>
- MSVC 5.0/6.0 PSX + N64 + PC. PC port is closest to us.

### Pizza Tower decomp (in progress)
- Active community decomp of the GameMaker-compiled binary.
- Mostly uses Ghidra-driven splits + manual `.cpp` writing.

## Console / non-MSVC decomps (conceptual references only)

These do not share toolchain with us, but their *workflow patterns*
(splat, YAML manifests, `objdiff`, decomp.me) are the lingua franca.

| Project               | Target                              | Why we care                                    |
|-----------------------|-------------------------------------|------------------------------------------------|
| OoT decomp            | Zelda OoT (N64, IDO 5.3, MIPS)      | Defines the per-function .c + asm/.s pattern.  |
| Mario 64 decomp       | SM64 (N64, IDO, MIPS)               | First "matching decomp" project; conventions.  |
| Paper Mario decomp    | Paper Mario (N64, GCC, MIPS)        | Best-organised splat-driven repo to study.     |
| FF7-PSX decomp        | FF7 (PSX, GCC, MIPS)                | Square Enix-adjacent; lore-relevant.           |
| Sonic Mania           | Sonic Mania (PC/MSVC)               | Modern MSVC matching.                          |

## Tools we use that came from these projects

- **`splat`** — splitter. Best for MIPS; we roll our own x86 splitter
  in `tools/build_split_yaml.py` because no x86-PE port exists.
- **`objdiff`** — function-level diff. Cross-toolchain, cross-arch.
  We use it as-is.
- **`decomp.me`** — function-level collaboration site. Web UI; you
  paste asm + a C draft, it compiles + diffs server-side. Has MSVC
  presets (including a 2005 one). We post hard functions there.
- **Splat/funny conventions**: every commit "claims" or "frees" a
  function via the YAML; PRs touch one row. We adopt this.

## FFXI / FFXIV-adjacent prior art

These are not decomps but they are reverse-engineering of related
games and inform our symbol naming.

### Project Meteor Server (in this workspace)
- C# RE of the FFXIV 1.23b server; primary symbol-naming source.
- Discord notes archived in `project_meteor_discord_context.md`.

### LandSandBoat / DarkStar (FFXI)
- See `land-sand-boat-server/xi-private-server.md` for our mining
  guide. *Structural* cousin to FFXIV 1.x — same combat-formula
  grammar, same Director/AI shape — but different binary entirely.

### Project Topaz (FFXI, retired)
- Dead. Predecessor to LandSandBoat. Mentioned for completeness.

### Sapphire (FFXIV ARR)
- Source: <https://github.com/SapphireServer/Sapphire>
- C++ ARR (2.0+) server. NOT 1.x. ARR rewrote the wire protocol from
  scratch, so Sapphire's packets and IDs are wrong for 1.x — but its
  general server-architecture vocabulary (Zone / Quest / Action /
  StatusEffect classes, OO patterns) influenced Project Meteor's
  later iterations.

### CemuRE / Dolphin / RPCS3
- Console emulators that have to RE the games they run. We mention
  them only because their public reverse-engineering forums
  occasionally contain reusable insight on Square Enix's wire
  formats (FF13 series shared engine code with FFXIV).
