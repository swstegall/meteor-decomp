# Shipped `.le.lpb` script-file format + filename cipher

> Last updated: 2026-05-03 — wrapper format reverse-engineered, all
> 2671 shipped script files successfully decoded to standard Lua 5.1
> bytecode. `tools/decode_lpb.py` is the canonical decoder.

## Correction to `docs/lua_bytecode_format.md` (Phase 6 item #5)

That doc claimed "anyone with `unluac` can decompile any shipped
`.lpb` file the game ships." That's **only true after applying the
wrapper format documented below.**

The original Phase 6 item #5 finding ("vanilla Lua 5.1 bytecode") was
correct for the **2 chunks embedded in the binary's `.rdata`**
(file `0xbde768` and `0xbde968` — the engine's bootstrap
`_onProgFunc` script). It was an over-generalization for the **2671
shipped script files** under `client/script/`, which use one of two
custom wrappers documented here.

## The two wrapper variants

Across the 2671 `.le.lpb` files in a typical 1.x install:

| Variant | Magic | Header | Files | Notes |
|---|---|---|---:|---|
| `rlu\x0b` | `72 6c 75 0b` | 8 bytes | **1** | Uncompressed: header + raw Lua 5.1 bytecode |
| `rle\x0c` | `72 6c 65 0c` | 16 bytes | **2670** | XOR-0x73 obfuscated payload |

The single `rlu\x0b` file is `9s59/kvw5/kvw5xvo5usv3q5rq.le.lpb` =
**`ZoneMoveProgTest`** — likely an unencoded build leftover or
intentional test fixture. All other shipped scripts use the obfuscated
variant.

## `rle\x0c` — XOR-0x73 obfuscation

Despite the "rle" name (suggesting Run-Length Encoding), there's no
RLE step. The obfuscation is **byte-wise XOR with the constant 0x73**.

### Header (16 bytes)

| Offset | Bytes | Meaning |
|---:|---|---|
| 0..3 | `72 6c 65 0c` | Magic `rle\x0c` |
| 4..7 | `1f c5 00 00` | Constant 0x0000c51f (purpose unknown — appears in 2513/2670 files; ~158 files have a different value here, likely a per-file checksum or version tag) |
| 8..11 | variable | 4-byte size field (little-endian; matches body length within ±3 bytes) |
| 12 | `ff` | XOR'd `0x8c` (purpose unknown) |
| 13..15 | `68 3f 06` | XOR'd `\x1bLu` — the **first 3 bytes of the Lua 5.1 signature** |

### Payload (offset 16+)

The payload is the **rest of the Lua 5.1 bytecode starting at byte 3
of the standard Lua header** (`a` = byte 3 of `\x1bLua`), each byte
XOR'd with `0x73`.

To reconstruct the full Lua bytecode:

```python
def decode_rle_lpb(data: bytes) -> bytes:
    assert data[:4] == b"rle\x0c"
    prefix = bytes(b ^ 0x73 for b in data[13:16])  # = b"\x1bLu"
    body   = bytes(b ^ 0x73 for b in data[16:])    # = b"aQ\x00\x01\x04..."
    return prefix + body                            # = b"\x1bLuaQ\x00\x01\x04..."
```

The first 5 bytes of the result are `\x1bLuaQ` — the standard Lua
5.1 chunk signature confirmed in `docs/lua_bytecode_format.md`. After
that, vanilla Lua 5.1 bytecode follows; `unluac` decompiles it
directly.

## Filename cipher

The shipped script-tree filenames are **substitution-ciphered**:

```python
def encode_filename(name: str) -> str:
    out = []
    for c in name.lower():
        if c.isalpha():
            pos = ord(c) - ord("a") + 1   # 1..26
            if 1 <= pos <= 10:
                out.append(str(10 - pos))             # a→9, b→8, ..., j→0
            else:
                out.append(chr(ord("a") + (37 - pos) - 1))  # k↔z, l↔y, m↔x, ..., r↔s
        elif c.isdigit():
            d = int(c)
            out.append(chr(ord("a") + (10 - d) - 1))  # 0↔j, 1↔i, ..., 9↔a
        else:
            out.append(c)
    return "".join(out)
```

Properties:
- **Case-folded** — uppercase input collapses to lowercase before
  encoding (e.g., both `Z` and `z` map to `k`).
- **Involution** — applying the cipher twice yields the original
  (lowercase) input.
- **Letter-pair fingerprint**: positions 1-10 ↔ digits 9-0; positions
  11-26 pair such that `pos_a + pos_b = 37` (k=11↔z=26, l=12↔y=25,
  m=13↔x=24, n=14↔w=23, o=15↔v=22, p=16↔u=21, q=17↔t=20, r=18↔s=19).

Validated against:
- `ZoneMoveProgTest` → `kvw5xvo5usv3q5rq` (16/16 chars match the
  outlier `rlu\x0b` file's actual filename)
- `Man0g0` → `x9wj3j` (6/6 — found at expected path
  `tp5rq/r75w9s1v/x9w/x9wj3j.le.lpb` = `Quest/Scenario/Man/Man0g0.lpb`)

## Tool: `tools/decode_lpb.py`

Two modes:

```bash
# Single-file: find + decode by source name
tools/decode_lpb.py <install_root> Man0g0
# → build/lpb/Man0g0.luac

# Bulk: decode all 2671 shipped .lpb to build/lpb/ mirroring tree
tools/decode_lpb.py <install_root>
# → build/lpb/<ciphered_path>/<ciphered_stem>.luac

# Show cipher table (for manual lookups)
tools/decode_lpb.py <install_root> --show-cipher
```

The `--show-cipher` mode prints the full 36-character mapping table
for reference. The bulk mode mirrors the script-tree directory layout
under `build/lpb/`, preserving the ciphered filenames (since most
files don't have an obvious source name to invert).

After decoding, run `unluac` (Java) on any `.luac` to get readable
Lua source:

```bash
java -jar /path/to/unluac.jar build/lpb/Man0g0.luac > Man0g0.lua
```

## Validation: Man0g0.lpb extraction

Decoded `Man0g0.lpb` is **15868 bytes** of standard Lua 5.1 bytecode
that decompiles to **1951 lines** of Lua source. Recovered the
complete method inventory (29 methods):

```
initText
processEvent000_0, processEvent000_1, processEvent000_2,
  processEvent000_3, processEvent000_4
processEvent010_1
processEvent020_1, processEvent020_2, processEvent020_3,
  processEvent020_4, processEvent020_5, processEvent020_6
processTtrNomal001withHQ, processTtrNomal001,
  processTtrNomal002, processTtrNomal003
processTtrMini001, processTtrMini002, processTtrMini003
processTtrAfterBtl001
processTtrBtl001, processTtrBtlMagic001, processTtrBtl002,
  processTtrBtl003, processTtrBtl004
processTtrBlkNml001, processTtrBlkNml002
processInformDialogAsQuest
```

**This matches garlemald's `man0g0.lua` comment block (lines 64-93)
EXACTLY.** Every method named in garlemald's `delegateEvent` calls is
present in the binary. Garlemald's understanding of the
client-side method names is 100% accurate.

Additional findings from the decompilation:
- **Inheritance:** `Man0g0` extends `ScenarioBaseClass` (loaded via
  `require "/Quest/Scenario/ScenarioBaseClass"`). `ScenarioBaseClass`
  is an intermediate base between `QuestBase` and per-quest scripts
  for main-scenario quests. Garlemald's quest-script class hierarchy
  should mirror this for parity.
- **`initText(self)`** loads text-data resource ID 391 with tag
  `"man0g0"` via `_loadTextDataPermanently(391, "man0g0")`. The text
  IDs (`52`, `53`, `54`, `55`, etc.) used in `processEvent*` calls
  index into this table.
- **Cutscene calls** use `startFadeOutCutSceneDefault` /
  `startHQCutScene("MAN0G000", 1)` / `startFadeInCutSceneDefault`
  pattern. The cutscene asset IDs are `MAN0G000` through `MAN0G030`
  matching the client-side `client/cut/man0g0NN/` directories.
- **Speaker dispatch** uses `worldMaster:say(player, line_id)` and
  `<actor>:say(player, line_id, flag)` — confirms garlemald's
  `quest:say(npc, line_id)` pattern shape.

## Closes Phase 6 item #9 follow-up gap #2

This finding closes one of the four minor non-blocking gaps from
`docs/garlemald_validation.md` — specifically:

> **Gap 2 — Quest-side `.prog` scripts not decompiled**

Garlemald's `man0g0.lua` comment block listing the `processEvent*` /
`processTtr*` function names is now confirmed against the binary
ground-truth. Garlemald's wire dispatch is correct.

## Cross-references

- `docs/lua_bytecode_format.md` — Phase 6 item #5 (the embedded
  `.rdata` Lua chunks; correction noted at the top of that doc)
- `docs/garlemald_validation.md` — Phase 6 item #9 (the gap-list
  this closes)
- `docs/lua_class_registry.md` — Phase 6 item #3 (the script-tree
  layout; the directory paths under `client/script/` are similarly
  ciphered — `tp5rq` = `Quest`, `r75w9s1v` = `Scenario`, `x9w` =
  `Man`, etc.)
- `docs/quest_dispatch.md` — Phase 6 item #7 (the quest dispatch
  flow that loads these `.lpb` files via `LpbLoader::ResourceEvent`)
- `tools/decode_lpb.py` — the canonical decoder
- `garlemald-server/scripts/lua/quests/man/man0g0.lua` — garlemald's
  server-side `delegateEvent` consumer of these client-side methods
- `project_meteor_discord_context.md` — Ioncannon mentions `unluac`;
  the wrapper-format reverse-engineering documented here was
  apparently not previously published
