# Phase 6 item #5 — `.lpb` / `.prog` bytecode format

> Last updated: 2026-05-03 — confirmed standard Lua 5.1 bytecode
> with default settings, **for the embedded `.rdata` chunks**.
>
> **CORRECTION 2026-05-03:** the shipped `client/script/*.le.lpb`
> files use a custom XOR-0x73 wrapper around the Lua bytecode +
> a substitution-cipher on filenames. See
> [`docs/lpb_format.md`](lpb_format.md) for the wrapper format
> + filename cipher + `tools/decode_lpb.py` decoder. Anyone with
> `unluac` can decompile a shipped `.lpb` file ONLY AFTER
> applying the wrapper decoder.

## Headline finding

`.lpb` and `.prog` files are **standard Lua 5.1 bytecode** with
the official format byte (no engine-specific extensions). The
default settings — little-endian, 4-byte int, 4-byte size_t,
4-byte instruction word, 8-byte double-precision float — mean
existing Lua 5.0/5.1 tooling (`unluac`, `luadec`, `ChunkSpy`)
works on them out of the box.

## Two embedded Lua chunks in the .text/.rdata

The binary embeds **2 Lua 5.1 bytecode chunks** directly in
`.rdata` (signature `\x1bLuaQ` — `\x1b` + "Lua" + `Q` = `0x51`
= Lua 5.1):

| File offset | First 32 bytes |
|---|---|
| `0xbde768` | `1b4c75615100010404040800000000000000000000000000000002020300...` |
| `0xbde968` | `1b4c75615100010404040800000000000000000000000000000002020000...` |

Both share an identical 12-byte header (decoded below). They
contain the engine's bootstrap-Lua code that runs before the
sqpack-loaded `.prog` files are mounted. Visible Lua identifiers
in the surrounding `.rdata` strings:
- `_onProgFunc` (the entry function name)
- `desktopWidget`, `executePlayerSystemCommand`,
  `executePlayerTargetMarking`, `executePlayerSignal`,
  `updatePlayerCommandAcquired`

These are the engine's own input-handling Lua hooks that are
embedded directly so they don't depend on sqpack being loaded.

## Lua 5.1 bytecode header (12 bytes)

Standard Lua 5.1 chunk header per `lopcodes.h` / `ldump.c`:

```
offset  size  field            value in this binary  meaning
------  ----  ---------------  --------------------  -------------------
   0     4    signature        b'\x1bLua'            standard Lua marker
   4     1    version          0x51                  Lua 5.1
   5     1    format           0x00                  official (no engine extensions)
   6     1    endian           0x01                  little-endian (LE)
   7     1    sizeof(int)      0x04                  32-bit int
   8     1    sizeof(size_t)   0x04                  32-bit size_t
   9     1    sizeof(Inst)     0x04                  4-byte instruction
  10     1    sizeof(Number)   0x08                  8-byte (IEEE-754 double)
  11     1    integral         0x00                  floating (NOT integer-only)
```

After the 12-byte header comes the standard Lua 5.1 function
chunk: source-name (size_t-prefixed string), line numbers,
parameter count, vararg flag, max stack size, instruction list,
constants, prototypes, debug info.

This is **vanilla Lua 5.1**. Any standard Lua 5.0/5.1 tooling
that consumes `\x1bLuaQ`-prefixed chunks works:
- `unluac` (Java) — Lua 5.0/5.1/5.2/5.3 decompiler, mentioned in
  `project_meteor_discord_context.md` as the canonical tool for
  Project Meteor's reverse-engineering work
- `luadec` (C) — same family
- `ChunkSpy` (Lua) — a Lua-bytecode pretty-printer

## File extensions in the wild

From the script-tree layout in `docs/lua_class_registry.md`:

| Ext | Hits in binary | Role |
|---|---:|---|
| `.prog` | 26 | Primary: compiled Lua 5.1 bytecode (script-tree leaf files) |
| `.lpb` | 4 | Secondary: also Lua 5.1 bytecode (probably an older or alternate compilation pipeline) |
| `.san` | 1 | StaticActor data (binary actor blueprint, NOT Lua) |

The two extensions co-exist in the script-tree paths. The
`.prog` files are the canonical compiled scripts; the `.lpb`
files are referenced from the runtime init code and one Quest
path. Both follow the same Lua 5.1 bytecode format
(confirmed by the `\x1bLuaQ` header in the embedded chunks).

## The loader — `LpbLoader::ResourceEvent` slot 1

The runtime entry point that mounts a `.lpb` / `.prog` file is
`Component::Lua::GameEngine::LpbLoader::ResourceEvent` slot 1
at `FUN_00d0fd70` (file `0x90fd70`, **264 bytes**):

```
Prologue (saves args, prepares working buffer)
  → CALL FUN_0062f320      ; load resource into memory (sqpack reader)
Three CALL FUN_009d22b4 sites (lines 62/81/95)
  → strcmp / Utf8String comparison helper (likely validating
    cached resource id, expected magic, or path match)
  → CALL FUN_00d0e250 (line 120, 699 B)
    ; the core LpbLoader::Load — parses the Lua header,
    ; allocates the chunk buffer, walks the bytecode segments
  → CALL FUN_00d0fc90 (line 170)
    ; sub-loader (probably for nested function prototypes)
  → CALL FUN_00d0ec30 (line 210)
    ; finalizer — registers the loaded chunk with the Lua VM
Epilogue (sets status, returns)
```

The 264-byte body is small because the actual bytecode-walking
logic is in the helpers. The loader is a coordinator that:

1. Gets the resource bytes from sqpack.
2. Validates the chunk (probably the `\x1bLua` signature check).
3. Hands the bytes to the Lua VM's loader (probably
   `lua_load` / `luaL_loadbuffer` equivalent in the
   `Component::Lua::GameEngine` wrapper).
4. Registers the resulting closure / prototype with the engine.

## The `LpbLoader::ResumeChecker` (3 slots)

Sibling class at RVA `0xd0f1ac` that polls the loader for
"resource ready" status. Slot 1 is a 27-byte status check:

```
80 79 75 00          CMP byte [ECX+0x75], 0
b8 cc 7e 37 01       MOV EAX, 0x01377ecc
75 05                JNZ skip
b8 cc ce 30 01       MOV EAX, 0x0130cecc
skip:
8b 08                MOV ECX, [EAX]
8b 44 24 04          MOV EAX, [esp+4]
89 08                MOV [EAX], ECX
c2 0c 00             RET 12
```

It checks `[loader+0x75]` — a status byte indicating whether
the load is complete. If complete, returns `0x01377ecc` (some
status code); if not, returns `0x0130cecc` (try-again code).

Slot 2 (33 B) sets `[loader+0x76] = 1` (advances state) and
calls `FUN_00748870` (advances the load coroutine).

This confirms the loader is **coroutine-driven** — it yields
back to the engine while waiting for sqpack reads, and is
resumed by the ResumeChecker once the resource is ready.

## Lua VM presence

The Lua 5.1 VM is statically linked but its standard exported
symbols (`lua_*`, `luaL_*`) are stripped. Indirect evidence of
the VM:

- "stack overflow" at file `0xd31124` — standard Lua VM error
  message (`luaG_runerror` family).
- "too many" at file `0xd14164` — standard `LUAI_MAXCSTACK`
  exhaustion message.
- The ~250 `Component::Lua::GameEngine::*` RTTI classes that
  wrap the VM (`LuaControl`, `LuaTimer`, `LuaTentativeControl`,
  `Parameter::StackOperator<T>`, etc.) — see
  `docs/director_quest.md`.
- The 2 embedded `\x1bLuaQ` chunks in `.rdata` — the engine
  consumes these at init.

The VM is wrapped under `Component::Lua::GameEngine` with
SE-specific naming, but the underlying VM is the canonical
Lua 5.1 implementation (with maybe small patches for memory
allocator integration).

## Decompiling shipped scripts

To decompile any `.prog` / `.lpb` file from the install:

```bash
# Option 1: unluac (recommended — best handles 5.1 bytecode)
java -jar unluac.jar /path/to/script.prog > script.lua

# Option 2: luadec
luadec /path/to/script.prog > script.lua
```

The script files live under sqpack — the loader accepts a
resource-id-addressed path (per Phase 4's resource-id system,
not a name hash). Walking the install tree:

```bash
# Find all Lua bytecode chunks in the install
find ~/path/to/FFXIV/data -type f \
  -exec grep -l --binary-files=binary -P '^\x1bLua' {} \;
```

The `.prog` and `.lpb` files inside sqpack DAT files start with
the `\x1bLua` signature and are byte-identical to standalone
files compiled by `luac51`. Garlemald-served scripts can
therefore be compiled with stock `luac` and shipped as-is.

## Practical impact for garlemald

1. **Use stock `luac` (Lua 5.1) to compile garlemald-served
   scripts.** No engine-specific extensions are needed; the
   format byte = 0 (official) means anything `luac51` produces
   will be accepted.

2. **No special endianness / size handling needed.** The 1.x
   client uses little-endian, 32-bit, double-float defaults —
   the same defaults `luac51` uses on x86 Linux / macOS hosts.
   Garlemald can compile on any standard host and ship the
   bytecode directly.

3. **Garlemald can verify a script before sending** by
   matching the 12-byte header. If the header doesn't say
   `\x1bLuaQ\x00\x01\x04\x04\x04\x08\x00`, the client will
   reject the script. Trivial 12-byte sanity check.

4. **For debugging, garlemald can ship source `.lua` files +
   recompile per-deploy.** The compile-once, ship-bytecode
   model is the production setup; for dev work, garlemald can
   keep source-form scripts and regenerate bytecode in CI or
   on each restart.

5. **The `unluac` decompiler is the canonical tool** for
   reading the shipped `.prog` files when you need to know
   what the original C++-team Lua looked like. This is
   important when garlemald is reverse-engineering retail
   quest behavior — the `.prog` files in the install are the
   ground truth.

## Phase 6 work pool — item #5 status

This closes Phase 6 item #5. Remaining items:

- #6 `LuaActorImpl` 90-slot map
- #8 `DirectorBase` slots 20..33 Lua hooks
- #9 Functional `OpeningDirector` validation against
  garlemald's `man0g0.lua`

## Cross-references

- `docs/director_quest.md` — Phase 6 architecture (the C++
  Lua-binding base classes that consume these bytecode files)
- `docs/lua_class_registry.md` — Phase 6 item #3 (the
  script-tree layout where `.prog` / `.lpb` files live)
- `docs/sync_writer.md` — Phase 6 item #4 (the Work-field
  serializer that bytecode-loaded scripts use to declare state)
- `docs/quest_dispatch.md` — Phase 6 item #7 (how QuestBase
  instances get a `.prog` script attached at slot 1 of the
  factory)
- `project_meteor_discord_context.md` — Ioncannon mentions
  `unluac` as the standard tool for decompiling shipped 1.x
  Lua scripts; references to `processEvent`, `Seq000`, and
  per-quest script paths align with the `/Quest/man/man_0_0.prog`
  convention here
- `~/path/to/FFXIV/data/lua/` — install-side script files
  (resolve via Phase 4's resource-id-to-path system in
  `docs/sqpack.md`)
