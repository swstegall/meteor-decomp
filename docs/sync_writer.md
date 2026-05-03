# Phase 6 item #4 — SyncWriter wire format

> Last updated: 2026-05-03 — vtable shape decoded; per-type Set
> + Serialize bodies decoded for Boolean/Int8/Int16/Int32/Float/
> String. The Lua `playerWork` / `groupWork` sync layer that
> garlemald has been using is now mapped end-to-end.

## What `SyncWriter*` is

`Component::Lua::GameEngine::Work::Memory::SyncWriter*` is the
**typed Lua-Work-field-to-wire serializer**. Every Lua-declared
field in a `*Work` table (e.g. `playerWork.activeQuest`,
`groupWork.memberStatus`) is backed by exactly one
SyncWriter instance on the C++ side. When game code mutates the
field via Lua (`self.playerWork.activeQuest = 0xa0f05e93`), the
SyncWriter:

1. Writes the new value into the SyncWriter's pending-value slot.
2. Increments a 16-bit "dirty counter" at `[this+0xc]`.
3. On the next sync flush, serializes both the previous (live)
   value and the new value onto the outbound packet stream, then
   commits the new value as live and decrements the counter.

This is the "double-buffered diff" pattern — the wire layer
sees both old and new, allowing per-field-change packets without
locking the producer.

## SyncWriter object layout (recovered from typed-write bodies)

```
+0x00  vtable                        (8-slot vtable below)
+0x04  output_handler                (pointer to wire-write callback)
+0x08  output_handler_vtable         (vtable for the handler)
+0x0c  dirty_counter (u16)           (++ on Set, -- on Flush)
+0x0e  is_diff_mode (u8)             (drives extra writes in Set)
+0x0f  reserved
+0x10  live_value                    (1, 2, or 4 bytes by type)
+0x11  pending_value (Boolean variant only — Boolean stores
       both at +0x10 and +0x11)
```

For `SyncWriterString`, the value is a `Sqex::Misc::Utf8String`
inline at `+0x10` (8 bytes for the small-string-optimised
Utf8String inline form, with overflow heap-allocated). For
`SyncWriterArray*`, the value at `+0x10` is a pointer to the
heap-allocated array body.

## The 8-slot SyncWriter vtable

All `SyncWriter*` classes share an 8-slot vtable shape. Slots
1..5 are inherited from the `SyncWriterBase` in identical bodies
across every typed subclass; slots 6..7 are class-specific
(typed) overrides.

| Slot | Purpose | Notes |
|---|---|---|
| 0 | Destructor | Class-specific (calls type-specific cleanup, e.g. Utf8String destruction for SyncWriterString) |
| 1 | **`Set(value)` entry point** | `++[this+0xc]; jmp slot[6]` — increments the dirty counter then tail-calls slot 6 |
| 2 | (shared no-op) | `FUN_00a72a20` — generic no-op stub |
| 3 | **`Reset()`** | Class-specific (Boolean/Int variants share `FUN_00d30c80`; Actor/Array variants override) |
| 4 | **`Flush()`** | If `[this+0xc] != 0`, calls slot 7 then decrements counter |
| 5 | (shared) `FUN_006ce2e0` | Common helper (probably `GetTypeId()` returning a small enum tag) |
| 6 | **Typed Set** (called via tail-call from slot 1) | Stores the new value into `[this+0x10..]` |
| 7 | **Typed Serialize** (called from Flush) | Reads live + pending values from `[this+0x10..]` and emits them via the output handler at `[this+0x4]` |

## Typed Set (slot 6) — bodies decoded

### `SyncWriterBoolean::Set` (16 B at file `0x92f8b0`)

```
8b 44 24 04          MOV EAX, [esp+4]      ; load value-pointer arg
80 38 00             CMP byte [EAX], 0
0f 97 c2             SETA DL               ; DL = (*p != 0) ? 1 : 0
88 51 11             MOV [ECX+0x11], DL    ; store at +0x11 (pending)
c2 0c 00             RET 12
```

Wire shape: 1 byte. Note Boolean stores at `+0x11`, not `+0x10`
(the `+0x10` byte is the live value, written by Flush).

### `SyncWriterInteger8::Set` (12 B at file `0x92f8e0`)

```
8b 44 24 04          MOV EAX, [esp+4]
8a 10                MOV DL, [EAX]
88 51 10             MOV [ECX+0x10], DL    ; store at +0x10 (1-byte)
c2 0c 00             RET 12
```

Note Int8 stores directly to `+0x10` (no separate pending slot
because the dirty counter handles the diff).

### `SyncWriterInteger16::Set` (59 B at file `0x92f910`)

Loads the 2-byte argument, calls `memcpy(this+0x10, &value, 2)`,
then if `[this+0xe]` (`is_diff_mode`) is set, copies the OLD
value to a backup slot at `[this+0x12]`. This is the "snapshot
the previous value before overwriting" path that lets Flush
emit a true (old, new) pair on the wire.

```
0f b7 44 24 08       MOVZX EAX, word [esp+8]
56 57                PUSH ESI; PUSH EDI
8b f9                MOV EDI, ECX
8b 4c 24 0c          MOV ECX, [esp+0xc]
50 51                PUSH EAX; PUSH ECX
8d 77 10             LEA ESI, [EDI+0x10]
56                   PUSH ESI
e8 d8 4c ca ff       CALL memcpy           ; → 0x5d4600 (the shared memcpy)
83 c4 0c             ADD ESP, 12
80 7f 0e 00          CMP byte [EDI+0xe], 0
74 15                JZ skip_diff
8a 56 01             MOV DL, [ESI+1]       ; old MSB
8a 06                MOV AL, [ESI]         ; old LSB
88 54 24 10          MOV [esp+0x10], DL    ; ...
... (snapshot the old value)
skip_diff:
5f 5e c2 0c 00       POP EDI; POP ESI; RET 12
```

### `SyncWriterInteger32::Set` (73 B at `0x92f9b0`) and
### `SyncWriterFloat::Set` (73 B at `0x92fa20`)

Same shape as Int16 but with 4-byte memcpy. Float uses x87
loads/stores (`d9 44 24 10` `FLD dword [esp+0x10]` /
`d9 1e` `FSTP dword [ESI]`) to copy the float without going
through an integer register (avoids signaling-NaN coercion).

The shared `memcpy` helper sits at file `0x5d4600` — a
standard MSVC inlined-memcpy with size dispatch
(`81 f9 00 01 00 00; 72 1f` = "if size < 256 use the small
loop"). Three-way call site: Int16/Int32/Float Set all hit it.

### `SyncWriterString::Set` (70 B at `0x930550`)

Calls a Utf8String constructor (`FUN_00cae0ce` — tail of body),
then a 5-arg dispatch into the output handler. String fields
are non-trivial because they require allocation; the
SyncWriterString takes ownership of the Utf8String and emits
it via the standard string-on-wire protocol (length-prefixed
or null-terminated, decided by the output handler).

## Typed Serialize (slot 7)

### `SyncWriterBoolean::Serialize` (29 B at `0x92f8c0`)

```
53 8b c1             PUSH EBX; MOV EAX, ECX
0f b6 58 11          MOVZX EBX, byte [EAX+0x11]   ; pending value
8b 48 04             MOV ECX, [EAX+0x4]            ; output_handler ptr
8b 11 8b 52 08       MOV EDX, [ECX]; MOV EDX, [EDX+8]  ; handler vtable slot 2
53                   PUSH EBX                       ; push pending byte
0f b6 58 10          MOVZX EBX, byte [EAX+0x10]   ; live value
8b 40 08             MOV EAX, [EAX+0x8]            ; (?)
53 50                PUSH EBX; PUSH EAX
ff d2                CALL EDX
5b c3                POP EBX; RET
```

This emits BOTH old (`+0x10`) and new (`+0x11`) values to the
output handler — confirming the double-buffered diff design.
The output handler's vtable slot 2 is a **pair-write** function:
`emit(handler, live_value, pending_value)`.

### Other types' Serialize (slot 7) — same shape

Each typed Serialize has the same structure as Boolean's:
load live + pending values from `+0x10..`, push them to the
output handler with the type-appropriate width, call the
handler's pair-write entry. Int8 / Int16 / Int32 / Float /
Actor / Array all conform.

## Endian-adjusting variants

`SyncWriterArrayEndianAdjust<short>` (`0xd1037c`),
`SyncWriterArrayEndianAdjust<int>` (`0xd103a0`), and
`SyncWriterArrayEndianAdjust<float>` (`0xd103c4`) are
specialized array-writer wrappers that **byte-swap each
element** before serialization.

This **confirms FFXIV 1.x's wire format is big-endian** at the
field level — x86 is little-endian, so the EndianAdjust wrapper
exists to swap on the way out. (The C++-side game state stays
little-endian; only the wire is swapped.) This is consistent
with the `garlemald-server/common/src/utils.rs` Murmur2
big-endian assumption already validated in
`docs/murmur2.md`.

## Practical impact for garlemald

Garlemald's `SetActorPropertyPacket` builder already drives the
wire side of this system; the decomp confirms several
correctness points:

1. **Per-field SyncWriters** — every `playerWork` /
   `groupWork` Lua field corresponds to ONE SyncWriter instance
   on the C++ side. Garlemald's per-property wire entries map
   1:1 to these. The 343 GAM CompileTimeParameter slots
   (Phase 3) are the registered wire IDs for these SyncWriters.

2. **Double-buffered diff is the wire model** — when garlemald
   sends a property update, it should send (old_value,
   new_value) for diff purposes (or omit old_value if the
   wire opcode is "absolute set", which is the simpler form
   garlemald already uses). The typed Serialize bodies show
   the client expects the old+new pair when the server
   delivers a "diff update" opcode; for "absolute" updates
   the live value is just overwritten.

3. **Big-endian on the wire** — the EndianAdjust variants
   confirm this. Garlemald already byte-swaps field values
   in its packet builders; this is the right policy.

4. **Field widths are tied to the SyncWriter type** — the
   typed slot-6 bodies tell us exactly how many bytes each
   field type writes:
   - Boolean → 1 byte (at `+0x11` for pending)
   - Int8 → 1 byte
   - Int16 → 2 bytes (BE on wire)
   - Int32 → 4 bytes (BE on wire)
   - Float → 4 bytes (BE on wire)
   - String → Utf8String (Utf8String wire form — handler-decided)
   - Actor → actor-reference type (8 bytes typical: actor_id u32 + slot_id u32 or similar)
   - Array → handler-decided count-prefixed body

5. **Dirty-counter at `+0xc`** — the engine tracks ANY change
   via the dirty counter, even if the value reverts to its
   original. So garlemald sending a "no-op" update (same value)
   still triggers a wire emit on the next flush. To avoid
   spurious emits, garlemald's higher layer should diff *before*
   the SyncWriter Set call (which is the standard pattern).

## Phase 6 work pool — item #4 status

This closes Phase 6 item #4. Remaining items:

- #5 `.lpb` / `.prog` bytecode format
- #6 `LuaActorImpl` 90-slot map
- #7 Quest dispatch path
- #8 `DirectorBase` slots 20..33 Lua hooks
- #9 Functional `OpeningDirector` validation against
  garlemald's `man0g0.lua`

## Cross-references

- `docs/director_quest.md` — Phase 6 architecture (the C++
  base classes that own SyncWriter Work fields)
- `docs/lua_class_registry.md` — Phase 6 item #3 (the Lua
  classes whose Work fields use these SyncWriters)
- `docs/wire-protocol.md` — Phase 3 (the GAM CompileTimeParameter
  registry that names each SyncWriter's wire ID)
- `docs/murmur2.md` — Phase 3 (the wire-id derivation that
  identifies which SyncWriter receives a property packet)
- `garlemald-server/common/src/packet_writers/*.rs` — the
  Rust-side wire builders that mirror the SyncWriter Serialize
  paths
- `project_meteor_discord_context.md` — Ioncannon notes on
  `SetActorProperty`, the pre-Sapphire wire layer
