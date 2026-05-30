# WorkSync inbound chain (wire → Lua `_onUpdateWork`)

> Sourced from **ffxivDecomp** (github.com/Yokimitsuro/ffxivDecomp), an independent
> docs-only RE of FFXIV 1.23b `ffxivgame.exe`, used with permission (see `NOTICE.md`).
> **Cross-referenced, not byte-verified** by meteor-decomp's own decompilation —
> confirm against the asm before relying on offsets for a code change.
> Captured 2026-05-30 from the ffxivDecomp 2026-05-27/28 session.

## Scope

`docs/sync_writer.md` covers only the **OUTBOUND** half — the
double-buffered-diff `SyncWriter*` serializer that turns a mutated
Lua `*Work` field into wire bytes. This doc covers the **INBOUND**
half: how a received WorkSync packet is walked, parsed into transient
records, and surfaced to the Lua script as the `_onUpdateWork`
callback (and, on the engine apply path, written into the actor's
bit-packed binding storage at `actor+0x214`).

Read both together — they are the two directions of the same
subsystem. The asymmetric wire model (verbose C→S string `WorkPath`
vs. compact S→C binding-id) is the load-bearing design fact that ties
them, documented under [§4](#4-the-asymmetric-wire-model) below.

## 1. The 7-level inbound chain

A WorkSync update arriving on the Zone channel descends through seven
single-responsibility functions, each transforming the data one level
closer to script-visible form:

| Lvl | Function (VA) | Role |
|-----|---------------|------|
| 0 | wire receive | Zone-channel packet (opcode `0x12f`/`0x132`/`0x133`, or via the generic data-packet path) |
| 1 | entry **38** of inbound dispatch table `0x00fdfb80` → `_onReceiveDataPacket` | generic 192 B data packet; WorkSync payload is one multiplexed variant (inner type tag) |
| 2 | `FUN_006e17e0` / `FUN_006e1f70` (Lua-bound dispatch) | looks up the per-class dispatcher via `vtable[0xec]`, routes the packet through it; the `_17e0` variant also fires 2 optional notify callbacks gated on flags `+0xe0`/`+0xe1` |
| 3 | `FUN_00775890` / `FUN_00775a30` → `FUN_00775180` | per-class WorkSync dispatcher entry (the slot at `class+0xec`, slot index `0x3b`) |
| 4 | `FUN_00775180` (byte parser) | walks the buffer byte-by-byte; 4-mode varint encoding via the type-tag thresholds `DAT_00fe059b`/`05a0`/`05a1`; per record, conditionally ECHOes (re-serializes) then calls level 5 |
| 5 | `FUN_00774220` (per-record processor) | looks up / allocates a 200 B inbound `CommandUpdate` record (CTOR `FUN_00768260`), walks payload ints via `FUN_00768310`, fires level 6, frees the record |
| 6 | `CommandUpdater_invokeLua_onUpdateWork_complex` | runs the per-update filter chain, applies the 0-based → 1-based arg conversion, fires the Lua callback |
| 7 | Lua `MyClass:_onUpdateWork(structName, slotName, idx0, idx1)` | script-facing reaction (UI refresh, reactive logic) |

The deep stack is intentional: level 1 = general packet routing,
level 2 = lets Lua register custom per-class handlers, level 3 = each
class owns its own `WorkPath` tree so updates route to the correct
field set, level 4 = the 4-mode decode, level 5 = record alloc +
`WorkPath`-map lookup, level 6 = filter + arg normalize, level 7 =
game logic. Cost is 7 frames per update, acceptable at the typical
< 100 updates/tick/client.

The 5 DATA xrefs to `FUN_006e17e0` (binding-target table at
`0x00fd5cd8`, `0x00fd5e30`, `0x00fd5d98`, `0x00fd64a8`, `0x00fd7888`)
are believed to be per-class registrations; the simpler
`FUN_006e1f70` is for classes without notify callbacks. Not
byte-verified.

## 2. Inbound vs. outbound `CommandUpdate` records (two distinct types)

The inbound parser allocates a **different, smaller** record type than
the outbound serializer — they share only the callback interface, not
the C++ class.

| Direction | Size | CTOR (VA) | Use |
|-----------|------|-----------|-----|
| OUTBOUND | 280 B (`0x118`) | `CommandUpdater_allocAndEnqueueRecord` @ `0x0076b3d0` | send to wire (carries serializer state + queue links) |
| INBOUND | 200 B (`0xc8`) | `FUN_00768260` @ `0x00768260` | transient receive container + Lua callback fire |

The inbound CTOR (`FUN_00768260`) copies the first uint from its arg,
zeroes `+0x08..0x10`, inits a substruct via `FUN_006bf100(this+0x14)`,
sets a sync flag at `+0xc5` and clears the pending / filter-applied
flags at `+0xc4`/`+0xc6`/`+0xc7`, then allocates an inner buffer at
`this+0x4` via `FUN_007222d0`. The buffer is **two-mode**:

- **Compact:** `0xa0` (160 B) inner + 200 B header ≈ 360 B total
- **Extended:** `0x300` (768 B) inner + 200 B header ≈ 968 B total
  (selected when the CTOR flag is non-zero — accommodates broadcasts
  with many fields). The size selector is the expression
  `(-(uint)(flag != 0) & 0x260) + 0xa0`.

## 3. The 4-mode varint packet format (level-4 byte walker)

`FUN_00775180` treats the packet as a **stream of variable-length
records**, each `[type_tag_byte] [payload]`. The tag is classified
against three `DAT` thresholds:

| Tag range | Mode | Payload |
|-----------|------|---------|
| `tag < DAT_00fe059b` | **binding-id reference** | 5 bytes: `tag` + 4 B binding id → look up in the `WorkPath` tree, copy the data (compact S→C addressing) |
| `DAT_00fe059b ≤ tag < DAT_00fe05a0` | **short literal** | `(tag − DAT_00fe059b)` bytes follow (small inline value) |
| `tag == DAT_00fe05a0` | **string-keyed `WorkPath`** | variable-length string + value (verbose C→S addressing) |
| `tag ≥ DAT_00fe05a1` | **large literal** | `(tag − DAT_00fe05a1)` bytes follow (oversized value) |

This is a domain-specific varint: the four ranges encode four
addressing modes (binding-id / short literal / string-keyed / large
literal). The binding-id mode is ~5 bytes/field vs. ~30 bytes for a
full string path — the source estimates ~95% bandwidth savings vs. an
always-string format, which is exactly why the wire is asymmetric
(§4). The three `DAT` constants are not yet given symbolic names in
ffxivDecomp.

## 4. The asymmetric wire model

| Direction | Opcode | Payload | Size | Purpose |
|-----------|--------|---------|------|---------|
| C → S | `0x12f` | string `WorkPath` + value | 56 B | "I changed this field" (player UI action) |
| C → S | `0x135` | binding id (u32) | 24 B | "subscribe to field X" (request push) |
| S → C | **UNPINNED** | binding id + value | ~6–8 B | "field X is now Y" (broadcast tick) |

**The S→C broadcast opcode is NOT pinned in ffxivDecomp.** The
candidates flagged in the source are `0x130`/`0x131`/`0x132` (adjacent
to the `0x12f` sender) — `0x130`/`0x131` are the leading guesses. This
is an explicit **meteor-decomp matching target**: the inbound
binding-update handler is reachable via the vtable at `0x0110fcf8`
(see §6), but the exact wire opcode that extracts `(binding id, value)`
and routes to the BitPacked writers (§5) has not been confirmed
against the dispatch table. Pinning it from the asm closes the last
WorkSync gap on the S→C direction.

Why asymmetric: C→S is **rare** (player UI events) so verbose string
paths cost little; S→C is **frequent** (every visible actor's HP ticks
~3×/s) so only compact binding-ids are affordable.

### Subscribe-based, not broadcast-all

`opcode 0x135` (24 B) carries a binding id, classified by
`FUN_006e7370` into three subscription types:

| Type | Meaning |
|------|---------|
| 0 | unsubscribe / cleanup |
| 1 | one-shot query — check the local cache only |
| 2 | persistent subscribe — send to server (triggers the `0x135` out) |

Bindings **`0x3f2`/`0x3f3`/`0x3f4`** (= decimal `1010`/`1011`/`1012` =
`hp[1]` / `hpMax[1]` / `state_mainSkillLevel`) are **special-cased to
ALWAYS take the server-query path**, even on a type-1 lookup, because
they are the fastest-ticking fields and must stay fresh. These three
literals are the third independent EXE confirmation that
`binding id == runtime field id`. The implication for a server: track
a per-(client, binding-id) subscription set and push only subscribed
bindings, rather than broadcasting all fields to all clients.

## 5. The apply path — 4 BitPacked leaf writers → `actor+0x214`

When the engine applies an inbound field update internally (rather
than the Lua handler doing it), the value flows to one of four
type-specialized **BitPacked writers**, the inverses of the BitPacked
readers documented in
[`docs/sync_writer.md`](sync_writer.md)'s sibling RE:

| typeTag | bytes | Writer (VA) |
|---------|-------|-------------|
| 1 | 1 B | `BitPacked_writeByte_type1` @ `0x00d11d30` |
| 2 | 2 B | `BitPacked_writeShort_type2` @ `0x00d11e90` |
| 3 | 3 B | `BitPacked_writeUint24_type3` @ `0x00d11fd0` |
| 4 | 4 B | `BitPacked_writeUint32_type4` @ `0x00d12080` |

All four delegate to **`BindingStorage_writeField_lowLevel_byBindingId`**
@ `0x00ce44d0`, the universal "apply field update" leaf. It indexes a
binding-metadata table (`storage+0x2c` start, `+0x30` end), validates
bounds, derives the bit-packed write offset, and writes into the raw
buffer at `storage+0x04` via `FUN_00ccb560`. Bit-to-byte alignment is
`(uint)((*bitOffsetPtr & 7) != 0) + (*bitOffsetPtr >> 3)`, so fields
can start mid-byte — the packing is dense, no inter-field padding.

The intermediate dispatchers (`FUN_00d294b0` for type 2, with
`FUN_00d29e00`/`FUN_00d2a2d0`/`FUN_00d2b400` predicted for types
1/3/4) do a **read-then-write** on the binding: read the current local
value via the BitPacked reader, write the new server value via the
matching writer. This is the **reconciliation / change-detection**
pattern (dirty-check before overwrite, then notify watchers via the
suspected `FUN_00d25900`). They are vtable methods (data ref from
`0x0110fcf8`) — the owning packet-processor class is identifiable from
the vtable RTTI and is the lead toward pinning the §4 S→C opcode.

Every actor that uses `bindWork` carries this storage at `this+0x214`;
the same four writer leaves serve all classes (the difference is only
which bindings register at which ids per class — `PlayerBase` ~94,
`CharaBase` ~76, `NpcBase` ~23, `ItemBase` ~19, `GroupBase` ~16,
`Director` ~5).

## 6. ECHO-on-inbound reconciliation

`FUN_00775180` (level 4) does something unexpected: after parsing a
record it **conditionally re-serializes** via the OUTBOUND function
`WorkSync_serializePayloadAndSend(actor, packetCtx, recordData)`. The
echo is **gated** on specific path strings — `DAT_0134c4b0` and
`DAT_0134c504` (the latter associated with a clip-object slot) — so it
is **not** universal.

The source's leading hypotheses (Medium confidence, not byte-verified):

1. **Validation** — client confirms the update by re-sending it.
2. **Reflection cancel** — for a self-originated update that arrives
   back via broadcast, the echo cancels out.
3. **Predictive reconciliation** — when the server's authoritative
   update lands, the client echoes its own (corrected) value to
   confirm sync.

For a server implementation this matters: an inbound `0x12f` write may
provoke an echoed `0x12f` back from the client for the gated slots —
the server should treat it idempotently, not as a second distinct
write.

## Confidence / caveats

ffxivDecomp marks the 7-level chain, the 200 B inbound record size,
the 4-mode encoding, the Lua-bound dispatch points, the four BitPacked
writers, and the ECHO behavior as **Confirmed** at the EXE level. The
**S→C broadcast opcode remains UNPINNED** (`0x130`/`0x131` candidates)
— this and the symbolic naming of the three `DAT` tag thresholds are
the open items. All VAs above are from the ffxivDecomp session and are
cross-referenced, not byte-verified here; confirm against the asm
before a code change.

## Cross-references

- `docs/sync_writer.md` — the OUTBOUND `SyncWriter` double-buffered
  diff (this doc's counterpart; field widths, vtable, EndianAdjust /
  big-endian wire)
- `docs/ffxivdecomp_inbound_opcodes.md` — the `0x00fdfb80` Zone inbound
  dispatch table (level-1 entry 38 `_onReceiveDataPacket` multiplex
  lives here; the two-table architecture note)
- `docs/ffxivdecomp_opcode_binding_map.md` — the client→server
  `0x12d`–`0x135` family (including `0x12f` and the `0x135` subscribe)
- `docs/ffxivdecomp_2026-05-28_session_integration.md` — the broader
  2026-05-28 session integration overview
