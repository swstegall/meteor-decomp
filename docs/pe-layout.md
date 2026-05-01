# PE layout — `ffxivgame.exe` and siblings

All five binaries that ship with FFXIV 1.23b are PE32 Intel 80386
(`IMAGE_FILE_MACHINE_I386` = 0x14c), GUI subsystem, base 0x400000,
characteristics 0x103 (`IMAGE_FILE_RELOCS_STRIPPED |
IMAGE_FILE_EXECUTABLE_IMAGE | IMAGE_FILE_32BIT_MACHINE`).

Linker version 8.0 in every header → MSVC 2005 toolchain. See
[`compiler-detection.md`](compiler-detection.md).

## Per-binary summary

| Binary             | Size       | Build timestamp     | Entry RVA  | `.text` size | Sections |
|--------------------|-----------:|---------------------|-----------:|-------------:|---------:|
| `ffxivgame.exe`    | 15,996,808 | 2012-09-11 16:30:23 | 0x5d4baa   | 11,784,192   | 6        |
| `ffxivboot.exe`    | 12,961,112 | 2010-09-16 11:46:54 | 0x507a6a   | 9,527,296    | 6        |
| `ffxivconfig.exe`  | 3,471,240  | 2012-09-11 16:37:31 | 0x1dec0    | 303,104      | 5        |
| `ffxivupdater.exe` | 640,344    | 2010-09-16 11:42:12 | 0x3fa4b    | 434,176      | 5        |
| `ffxivlogin.exe`   | 403,296    | 2011-01-28 09:25:41 | 0x26838    | 258,048      | 4        |

All timestamps are UTC. `ffxivgame.exe` and `ffxivconfig.exe` share
their build minute (2012-09-11) which corresponds to the patch 1.23
build cycle. `ffxivboot.exe` and `ffxivupdater.exe` were built on
2010-09-16 (~two weeks before launch) and were never rebuilt. The
launcher binaries baked in the original network code path; the game
proper kept evolving for two more years.

## `ffxivgame.exe` sections

```
.text       vaddr=0x00001000  vsize=0xb3b56d  rsize=0xb3c000  RX
MSSMIXER    vaddr=0x00b3d000  vsize=0x00006d  rsize=0x001000  RX
.rdata      vaddr=0x00b3e000  vsize=0x326032  rsize=0x327000  R
.data       vaddr=0x00e65000  vsize=0x117940  rsize=0x0bf000  RW
.tls        vaddr=0x00f7d000  vsize=0x0000a9  rsize=0x001000  RW
.rsrc       vaddr=0x00f7e000  vsize=0x01a54c  rsize=0x01b000  R
```

Notes:

- **`MSSMIXER`** is the Miles Sound System mixer engine, statically
  linked. It's a third-party section name, not a Microsoft-tools
  default. We exclude it from decomp work; see
  [`known-libraries.md`](known-libraries.md).
- **`.tls`** at 0xa9 bytes is a single TLS slot, almost certainly
  the per-thread CRT state plus one or two game-state pointers.
- **`.rsrc`** is small (~107 KB) — version info + maybe the icon
  group. No embedded HTML / PNG / etc.
- **`.rdata` is huge** (3.1 MB). At MSVC 2005, `.rdata` holds:
  - vtables (RTTI present),
  - exception-handling tables (`/EH` SEH unwind),
  - string literals,
  - import directory + IAT,
  - constants,
  - C++ `type_info` records.

  The 3+ MB scale tells us we have *thousands* of vtables, i.e. the
  client is fully OO C++ (not C with a thin C++ veneer like older
  Square titles). Project Meteor's class names are good seeds.

## `ffxivboot.exe` sections

(To be confirmed — `tools/extract_pe.py` will dump these.)

The boot binary is 12.9 MB and shares its build date with the
updater. It probably embeds its own copy of the renderer +
WindowsForms-style boot-screen UI — `ffxivboot` is the in-game patch
notes reader and therefore needs its own DX9 surface; the game proper
re-initialises DX9 once boot has handed off to it.

## Where the binaries come from

The workspace pulls them out of a retail install via
`xiv1point0-apple-silicon-installer/install.sh`, which lays them down
at:

```
ffxiv-install-environment/target/prefix/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV/
  ffxivgame.exe
  ffxivgame.patched.exe   <- 16-byte cinematic-crash patch (see project_garlemald_cinematic memory)
  ffxivboot.exe
  ffxivconfig.exe
  ffxivlogin.exe
  ffxivupdater.exe
  ws2_32.dll              <- Seventh Umbral / garlemald-client network shim
  ws2_32-trace.log
  game.ver / boot.ver / patch.ver
```

`tools/symlink_orig.sh` mirrors the five .exe files into
`meteor-decomp/orig/` as symlinks.
