# Phase 8 — Group / SharedWork system decomp

> Last updated: 2026-05-15 — kickoff inventory + Group::PacketProcessor
> dispatch pattern + receiver-side dispatch tracing for opcode 0x0133
> SynchGroupWorkValues.

## Why this phase

Garlemald's recent work on the Group/SynchGroupWorkValues path (commits
`adc3244` "0x0133: wire SynchGroupWorkValues /_init reply", `175f53d`
"0x0133: restrict SynchGroupWorkValues reply to content-director group
inits only", `eb7c573` "director: emit content-director spawn packets at
content-area creation + post-warp respawn") was empirically driven by
packet captures and pmeteor parity, not by principled decomp. The
GroupHeader/Begin/X08/End trio + 0x0133 init-reply was discovered by
"what makes the cinematic post-warp work" rather than "what does the
engine actually expect."

This doc captures what the engine actually does on the receive path so
future garlemald work has a non-empirical reference.

## Group class hierarchy (18 vtables)

All under `Application::Lua::Script::Client::Group::*`. Sizes from
`config/ffxivgame.rtti.json`:

| Class | Vtable RVA | Slots | Role |
|---|---|---:|---|
| `PropertyUpdater::Listener` | `0xbd40f0` | 2 | Listener-side stub for property-change notifications |
| `MemberInfoUpdater::Listener` | `0xbd40fc` | 2 | Listener-side stub for member-info changes |
| `SyncWriterOwnerInterface` | `0xbd4108` | 2 | Owner-side stub for SyncWriter callbacks |
| `Entry::EntryDisplayNameListener` | `0xbd4114` | 2 | Listener for entry display-name changes |
| **`PacketRequestBase`** | `0xbd4120` | 13 | **Send-side packet builder base (request engine)** |
| **`EntryBuilderBase`** | `0xbd415c` | 19 | **Group-entry creation (incl. EntryBuilder + EntryLinkShellBuilder)** |
| `MemberInfoUpdater` | `0xbd41ac` | 13 | Member-info change pipeline |
| `PropertyUpdater` | `0xbd41e4` | 13 | Property change pipeline |
| `WorkSyncUpdater` | `0xbd421c` | 13 | Work-table sync change pipeline |
| `OnlineStatusUpdater` | `0xbd4254` | 19 | Online-status change pipeline |
| `BreakupBuilder` | `0xbd42a4` | 19 | Group-breakup pipeline |
| **`PacketProcessor`** | `0xbd42f4` | 3 | **Receive-side dispatch** |
| `WorkSync` | `0xbd4304` | 2 | Work-sync data wrapper variant A |
| `WorkSync` | `0xbd4310` | 4 | Work-sync data wrapper variant B (richer) |
| `WorkSync` | `0xbd4324` | 3 | Work-sync data wrapper variant C |
| **`SharedWork`** | `0xbd4334` | 28 | **The work-table that gets synced (heart of the system)** |
| `EntryBuilder` | `0xbd442c` | 19 | Concrete EntryBuilder (subclass of EntryBuilderBase) |
| `EntryLinkShellBuilder` | `0xbd447c` | 19 | Linkshell-specific EntryBuilder |

Plus the Lua-script binding base:

| Class | Vtable RVA | Slots | Role |
|---|---|---:|---|
| `Application::Lua::Script::Client::Control::GroupBase` | `0xbd53cc` | 34 | Lua-script-bindable group base (15 client-side methods exposed via `_x_cpp` / `_x_inl`; see `cpp_bindings_index.md`) |

## Group::PacketProcessor dispatch pattern (slot 1 = `OnPacket`)

The engine's per-packet receive entry is `PacketProcessor::OnPacket(buf)`
at `FUN_006cde30` (RVA `0x2cde30`, 141 B). Decoded structure:

```c
struct PacketProcessor {
  /* +0x00 */ void**  vtable;
  /* +0x3c */ void*   on_both_complete_arg;   // passed into combined cb
  /* +0x40 */ Subdecoder1 sub1;               // ~84 B, e.g. MemberInfo
  /* +0x94 */ Subdecoder2 sub2;               // ~84 B, e.g. Property
  /* +0xe8 */ uint8_t sub1_complete;
  /* +0xe9 */ uint8_t sub2_complete;
  /* +0xea */ uint8_t both_callback_pending;
};

void PacketProcessor::OnPacket(this, buf) {
    // Try subdecoder 1
    if (sub1.TryParse(buf)) {                  // FUN_00445d20
        sub1.Process(this);                    // FUN_00445530
        sub1_complete = 1;
        // If sub1's combined-callback target is alive, fire it
        if (sub1_combined_target != 0)
            sub1.AfterParse(this, &sub1.work); // FUN_00cc76f0
    } else if (sub2.TryParse(buf)) {           // FUN_00445d20 again (same fn, different `this`)
        sub2.Process(this);                    // FUN_00445530
        sub2_complete = 1;
    }
    // If both subdecoders have completed, fire the on-both-done callback
    if (sub1_complete && sub2_complete) {
        OnBothComplete(this, &this->on_both_complete_arg);  // FUN_006cda80
        both_callback_pending = 0;
    }
}
```

The two subdecoders are co-resident on the same PacketProcessor object
(at `+0x40` and `+0x94`). The engine **routes a single inbound packet
to whichever subdecoder accepts it**, tracks both completion flags
independently, and fires a combined callback when both have completed.

This is the structural reason garlemald sends a **trio**
(GroupHeader/Begin/X08/End) for the SynchGroupWorkValues path: the
engine's PacketProcessor expects two subdecoder rounds to complete
before it fires the "group is now fully synced" callback. The X08
mid-marker is the second subdecoder's signal.

## Group::PacketProcessor slot 0 (dtor) and slot 2

- **Slot 0** (`FUN_006d7d90`, 27 B) — standard MSVC virtual dtor. Calls
  base-class dtor at `FUN_006c4a20` (the parent destructor), then
  conditionally calls `operator delete` (offset `0x9d1b17`) if the
  delete-flag bit is set on the stack.
- **Slot 2** (`FUN_006bfe70`, 19 B) — a guard-tail dispatcher. If
  `[ecx+0xeb] != 0` (a re-entrant guard), no-op return. Else,
  tail-jumps to `vtable[1]` of `*ecx` — i.e. tail-calls a parent
  class's slot 1 (the OnPacket entry). This is the safe-reentrant entry
  point that callers use when they don't know if the processor is
  currently executing.

### Wire vs runtime: 0x30 wire-slot vs 16-byte engine-internal storage

A potential audit concern from item #1's findings was that
`SharedWork::GetMemberAt` (slot 19) computes element offsets via
`shl esi, 4` (i.e. `idx * 16` — 16-byte member stride). Garlemald's
wire packets (`build_group_members_x08` / x16 / x32 / x64) use **0x30
(48) bytes per member slot** — the on-the-wire layout includes a
32-byte ASCII name field, two flag bytes, IDs, and 2 padding bytes.

These two strides are **not in conflict**:

- **Wire format** (`encode_group_member_at` at
  `garlemald-server/map-server/src/packets/send/groups.rs:55`): 0x30
  bytes per member slot, includes the full ASCII name.
- **Engine post-parse storage** (`SharedWork::members[+0x14..+0x18]`):
  16 bytes per entry — likely just `(actor_id, name_id_or_ptr,
  flag_byte, padding)` after the engine condenses the wire data.

The engine parses incoming X08/X16/X32/X64 packets, extracts the
short fields it needs for fast lookup, and stores them in the 16-byte
runtime member array. The wire-side and the runtime-side serve
different purposes: the wire carries names + flags (for the UI),
the runtime cares about ID-keyed lookup by index.

Garlemald's wire emission is correct as-is. No gap to fix.

## Group::SharedWork — the work-table API

`SharedWork` (28 slots, `0xbd4334`) is the heart of the per-group
state. Decoded slot layout:

| Slot | RVA | Role |
|---:|---|---|
| 0 | `FUN_006dab60` (27 B) | dtor |
| 1 | `FUN_006da290` (~30 B) | (likely Reset / Init) |
| 2 | `FUN_006da300` | (likely BeginUpdate) |
| 3 | `FUN_006da370` | (likely EndUpdate) |
| 4..7 | `FUN_006da3e0`/`450`/`4c0`/`530` | per-field accessors (variants by type) |
| 8 | `FUN_006da5a0` | (per-type accessor) |
| 9 | `FUN_006ce2e0` | shared LuaControl helper (used across all `*Updater` classes) |
| 10..12 | `FUN_006bffc0`..`FUN_006bffe0` | tiny no-op stubs (`ret 0x8`) — placeholder slots |
| 13..15 | `FUN_006cbda0`..`FUN_006cbdc0` | (member iterator helpers) |
| 16..18 | `FUN_006c5500`..`FUN_006c55c0` | (member lookup helpers) |
| 19 | `FUN_006c2d80` (73 B) | **`GetMemberAt(u16 idx)`** — bounds-checked 16-byte-stride lookup against `[+0x14..+0x18]` array |
| 20 | `FUN_006c2dd0` | (sibling of 19) |
| 21 | `FUN_006c2e20` | (sibling of 19) |
| 22 | `FUN_006c9930` (121 B) | **`CopyMemberByLookup(key, dst, len)`** — looks up by short key, validates extent, copies len bytes via shared `memcpy` at `0x9d4600` |
| 23..27 | `FUN_006c99b0`..`FUN_006c9bb0` | (sibling copy variants) |

**Member-array layout** (deduced from slot 19 / 22):
- `[this+0x14]` — pointer to first member entry
- `[this+0x18]` — pointer to one-past-last member entry
- Member entries are 16 bytes each (`shl esi, 4` = `idx * 16`)
- Out-of-range index → calls `0x9d22b4` (likely `__report_rangecheckfailure` or assert)

So `SharedWork` exposes a **bounded array of fixed-size member entries**
(16 B each) with typed slot accessors. The 28-slot vtable is the
per-field reader/writer surface that the SyncWriter pipeline drives
when a property changes.

## 0x0133 dispatch — runtime-registered callback

The Zone-channel inbound dispatcher
(`Application::Network::ZoneProtoChannel::Dispatcher` at RVA
`0x9bfd10`) routes opcode 0x0133 to **vtable slot 52** of the registered
callback interface (`ZoneProtoDownCallbackInterface`, 199 slots).

Default-installed handler is
`Application::Network::ZoneClient::ZoneProtoDownDummyCallback` whose
slot 52 is a 3-byte stub `ret 0xc` (`FUN_00db8810`). The real handler
is plugged in at runtime via callback registration — RTTI doesn't
expose it because the engine constructs the handler instance + sets
the vtable pointer dynamically (no compile-time inheritance edge to
trace via `??_R*` records).

Practical implication for garlemald: the on-the-wire format the engine
expects for 0x0133 has to be reverse-engineered from the
PacketProcessor / SharedWork **state-mutation observed when a 0x0133
arrives**, NOT from a static handler. The current best-known wire
format is the `(GroupHeader, Begin, X08, End)` trio that pmeteor /
garlemald empirically validated.

The dispatcher emits per-case 17-byte blocks of the shape:

```asm
mov esi, [ecx]           ; ecx = registered callback obj, load vtable
add eax, 0x10            ; skip the 16-byte packet header
push eax                 ; push payload ptr
mov eax, [esi + N*4]     ; load slot N from vtable
push edx                 ; push the size/context arg
mov edx, [esp+0x10]
push edx                 ; push self-context
call eax                 ; → vtable slot N
pop esi
ret 8
```

For 0x0133 → case 50 → vtable slot 52. The dispatch table lives at
`byte_table_va = 0xdc1274` (502-entry case map) and the per-case
entry pointers at `0xdc0f5c` (jump table).

## What this means for garlemald

1. **The engine decodes packets via two parallel subdecoders.** The
   GroupHeader/Begin/X08/End trio garlemald sends maps to: GroupHeader
   = packet header, Begin = subdecoder-1 frame, X08 = subdecoder-2 frame,
   End = "trigger combined callback." Get any one of these wrong (wrong
   opcode, wrong size, wrong field order) and the corresponding
   `sub*_complete` flag never sets, so the combined callback never
   fires, and the group state stays half-initialized.

2. **The `+0xea` `both_callback_pending` flag is the engine's "ready to
   commit" gate.** If garlemald's send timing is wrong (e.g. sending
   the X08 before the engine has consumed the Begin frame), the engine
   silently discards the X08 because the dispatcher state expected
   subdecoder-1 to be feeding bytes. Rate-limiting the trio to a
   single tick on garlemald's side is critical.

3. **`SharedWork` exposes a 16-byte-per-member array.** Garlemald's
   `transient_party_members` / `transient_director_members` should
   serialize each member as a 16-byte struct on the wire. The current
   per-member layout in
   `garlemald-server/map-server/src/runtime/broadcast.rs` should be
   audited against this 16-byte stride.

4. **The 0x0133 handler is not statically discoverable in the binary.**
   Wire-format work for 0x0133 (and any other runtime-callback opcode)
   has to come from packet captures + state-mutation observation, not
   from decompiling a fixed handler vtable. This is consistent with
   the empirical approach garlemald has been taking; the doc just
   confirms there isn't a simpler path.

## Phase 8 work pool

| Item | Description | Status |
|---|---|---|
| #1 | Group class hierarchy inventory | ✅ done (this doc) |
| #2 | PacketProcessor dispatch pattern | ✅ done (this doc) |
| #3 | SharedWork slot map | 🟡 partial (slots 0..27 listed; semantic role of slots 13..18 needs deeper trace) |
| #4 | EntryBuilderBase 19-slot map | ✅ done (this doc, "EntryBuilderBase + EntryBuilder slot maps" section below) |
| #5 | PacketRequestBase 13-slot map | ✅ done (this doc, "PacketRequestBase slot map" section below) |
| #6 | OnlineStatusUpdater + BreakupBuilder slot maps | ✅ done (this doc, "OnlineStatusUpdater + BreakupBuilder" section below) |
| #7 | 0x0133 / 0x017A wire-format derivation from packet captures | ✅ done (this doc, "Retail wire format" section below) |
| #8 | Audit garlemald's per-member SharedWork serialization vs. the 16-byte stride | ✅ no-op (the 16-byte stride is the engine's INTERNAL post-parse storage; the wire format is separate at 0x30 bytes/member — verified against `garlemald-server/map-server/src/packets/send/groups.rs:55 encode_group_member_at`) |
| #9 | Find the runtime registration site for ZoneProtoDownCallbackInterface — gives us the real 0x0133 handler RVA | 🔲 pending — search for code that writes a vtable ptr into the dispatcher's `ecx` arg storage |

## Retail wire format — 0x017A SynchGroupWorkValues vs 0x0133 GenericDataPacket

**Discovery surprise:** opcode 0x0133 OUT (server→client) is **NOT** the
SynchGroupWorkValues path. Two different opcodes carry related but
distinct payloads:

| Opcode | OUT body fmt | Use |
|---|---|---|
| `0x017A` | runningByteTotal + typed property entries + target | **SynchGroupWorkValues** — work-table sync (the actual 0x0133 semantic the wiki names "Group Created", but the OUT wire opcode is `0x017A`) |
| `0x0133` | LuaParam-encoded variadic args | **GenericDataPacket** — `player:SendDataPacket(class, target, name, ...)` from Lua. Carries `attentionMessage` calls + similar Lua-driven RPCs |

The IN side is opcode `0x0133` for `GROUP_CREATED` (client→server "I
created group X via /_init"); garlemald's IN handler responds by
sending an OUT `0x017A` SynchGroupWorkValues. Direction disambiguates
the two semantic uses of 0x0133.

### 0x017A SynchGroupWorkValues — exact wire layout

Confirmed against retail captures `combat_autoattack #1..5` in
`ffxiv_traces/`:

```
SubPacket size: 0xB0 (176 bytes total)
SubPacket header (16 B): standard
GameMessage header (16 B): unknown4=0x14, opcode=0x017A, unknown5=0,
                           timestamp=u32, unknown6=0
Body (144 bytes, padded to 0xB0 with zero):

  body[0..8]   = u64 group_id (little-endian)
                 retail uses 0x2680XXXX_XXXXXXXX for monster groups,
                 0x80000000_XXXXXXXX for player-work groups
  body[8]      = u8 runningByteTotal = total bytes of property entries
                 + target trailer (written last by sender)
  body[9..]    = property entries, packed in declared order:

    type=1 (byte):    u8(1) + u32 LE id + u8 value          → 6 bytes
    type=2 (short):   u8(2) + u32 LE id + u16 LE value      → 7 bytes
    type=4 (int):     u8(4) + u32 LE id + u32 LE value      → 9 bytes
    type=8 (long):    u8(8) + u32 LE id + u64 LE value      → 13 bytes
    type=N (buffer):  u8(N) + u32 LE id + N bytes           → 5+N bytes
                       (N is 5..0x80; type-tag IS the buffer size)

  target trailer:
    u8(0x82+len) + ASCII bytes                              → 1+len bytes
    (the 0x82 base flips to 0x62 when isMore=true, signalling that
     this is the second-or-later packet of a multi-packet sync)

  remainder: 0x00 padding to 0xB0 total body
```

The `id` field is the **MurmurHash2** of the dotted property path
(e.g. `MurmurHash2("contentGroupWork._globalTemp.director", 0)`). The
`target` is the property-path leaf the client should drive (commonly
`/_init` for group bring-up, or specific path strings for targeted
field updates).

### Cross-check against garlemald

Garlemald's `build_synch_group_work_values_content_init` (in
`map-server/src/packets/send/groups.rs:442`) matches the format
**exactly** — confirms the existing builder is wire-correct.

The retail capture shows ONE long property + target = 20 bytes
`runningByteTotal`. Garlemald's content_init emits TWO properties
(`contentGroupWork._globalTemp.director` int + `contentGroupWork.property[0]`
byte) + target = 22 bytes runningByteTotal. The garlemald formulation
follows pmeteor `ContentGroup.SendInitWorkValues`
(`Map Server/Actors/Group/ContentGroup.cs:105`) and is structurally
correct for a `/_init` reply; the retail capture happens to show a
combat-time party-sync emission (different scenario, same wire shape).

### `0x0133` GenericDataPacket — wire layout

Confirmed against retail captures `accept_quest #1`,
`local_leve_complete #1..7`:

```
SubPacket size: 0xE0 (224 bytes total)
SubPacket header (16 B): standard
GameMessage header (16 B): opcode=0x0133, ...
Body (192 bytes = 0xC0):

  body[0..]    = LuaUtils.WriteLuaParams(luaParams)
                 — variadic Lua-typed values, e.g. for
                   attentionMessage(p, textId, ...):
                   ["attention" (string), worldMaster (actor),
                    "" (empty string), textId (int), ...]
  remainder: 0x00 padding
```

Per pmeteor `GenericDataPacket.cs`. The Lua-param encoding follows
`LuaUtils.WriteLuaParams` (string = type-marker + ASCII + null-term;
int = type-marker + LE u32; etc.). Decoded by the receiving Lua VM
as variadic args to a script handler keyed by the leading class-name
(e.g. `"attention"` → `attentionMessage` Lua handler).

## Group::PacketRequestBase slot map (item #5)

`PacketRequestBase` (13 slots, RVA `0xbd4120`) is the **abstract base
of every send-side packet-emitter** in the Group subsystem. The 5
known subclasses (`EntryBuilderBase`, `MemberInfoUpdater`,
`PropertyUpdater`, `WorkSyncUpdater`, `BreakupBuilder`) all derive from
it and add their per-event payload state on top.

### PacketRequestBase slot map

| Slot | Body RVA | Bytes | Role |
|---:|---|---:|---|
| 0 | `0x2d0c20` | 27 | Destructor — calls parent dtor at `0x6d0b90` (the *same* parent dtor that `EntryBuilderBase` slot 0 calls, confirming the inheritance edge `EntryBuilderBase → PacketRequestBase`) |
| 1 | `0x6b7340` | 3 | `xor eax,eax; ret` — returns 0/false (inherited stub) |
| 2 | `0x2d0c10` | 5 | `xor eax,eax; xor edx,edx; ret` — returns u64 (0, 0) (default sequence pair) |
| 3 | `0x2ce2e0` | 1 | `ret` — empty no-op |
| 4 | `0x773290` | 3 | `mov al,1; ret` — returns 1 (true) — default `IsActive` |
| 5 | `0x1c5c80` | — | Inherited LuaControl helper |
| 6, 7 | `0x672a20` | 3 | `ret 0xc` — accept 12-byte arg, do nothing (subclasses override for member add/remove) |
| 8 | `__purecall` | — | **Subclasses MUST override — the `Send` / `Build` hook** |
| 9 | `0x1b8d90` | — | Inherited LuaControl no-op |
| 10 | `0x40fa00` | — | Inherited |
| 11 | `0x1c5c80` | — | Inherited |
| 12 | `0x837620` | 5 | `xor al,al; ret 8` — returns 0 (false), accepts 8-byte arg — likely `IsCompleted` / `IsBuilt` default |

So `PacketRequestBase` is a 5-method-real, 8-method-stub abstract:
1. dtor
2. `IsActive()` (default true)
3. `IsCompleted()` (default false)
4. `OnAddMember()` (default no-op)
5. `OnRemoveMember()` (default no-op)
6. `Send/Build()` (`__purecall`)

The shared inheritance edge `EntryBuilderBase → PacketRequestBase`
explains why their slot 0 dtors share the same parent (`0x6d0b90`).
PacketRequestBase is the actual abstract send-side base; the
`*Updater` and `*Builder` classes specialize the abstract Build hook.

## Group::OnlineStatusUpdater + BreakupBuilder slot maps (item #6)

Both classes derive from `PacketRequestBase` (via the same parent
dtor `0x6d0b90` / `0x6cb760`). They share most of the inherited slot
shape but diverge in their override count — **BreakupBuilder is the
minimal subclass** (only 4 overrides), while **OnlineStatusUpdater is
richer** (9 overrides) because it iterates a status array.

### BreakupBuilder slot map (4 overrides)

`BreakupBuilder` (19 slots, RVA `0xbd42a4`) is the one-shot "this
group is being torn down" emitter.

| Slot | Override RVA | Bytes | Role |
|---:|---|---:|---|
| 0 | `0x2d6ff0` | (dtor) | Concrete destructor |
| 8 | `0x2d6f80` | 5 | `Send()` — `mov al,1; ret 4` (returns success, no per-member work) |
| 13 | `0x6b7340` | 3 | `xor eax,eax; ret` (override the abstract `__purecall` to return null/0) |
| 14 | `0x2da8a0` | 132 | `Detach(out_subpacket)` — SEH-protected single-use builder finalize + self-destruct |

All other slots (1, 2..7, 9..12, 15..18) are inherited from
`PacketRequestBase` unchanged — confirms BreakupBuilder is the
minimal "fire-and-forget" packet emitter. No member iteration,
no work-table state — just "I'm sending the breakup packet."

### OnlineStatusUpdater slot map (9 overrides)

`OnlineStatusUpdater` (19 slots, RVA `0xbd4254`) tracks online/offline
state changes for each member of a group. Object layout deduced from
slot 18 (`IsComplete`):

```c
struct OnlineStatusUpdater : PacketRequestBase {
  /* +0x3c */ StatusEntry* status_array_begin;   // null if not started
  /* +0x40 */ StatusEntry* status_array_end;     // size = (end-begin)/8
  /* +0x48 */ uint32_t     expected_count;
};
```

| Slot | Override RVA | Bytes | Role |
|---:|---|---:|---|
| 0 | `0x2d4130` | (dtor) | Concrete destructor |
| 8 | `0x2bfc60` | 10 | `Send()` — tail-calls `(*ecx)->vtable[18]` (the inner status-list's IsComplete-or-similar) |
| 12 | `0x2cb070` | — | (override of inherited slot 12) |
| 13 | `0x6b7340` | 3 | Returns 0 (override `__purecall`) |
| 14 | `0x2da930` | 121 | `Detach(out_subpacket)` — standard detach + self-destruct |
| 15 | `0x2c3e00` | — | (override of inherited slot 15) |
| 16 | `0x2c44d0` | — | (override of inherited slot 16) |
| 17 | `0x2bfc70` | 1 | `MarkComplete()` — empty `ret` (no state change; the array length tracks completeness) |
| 18 | `0x2c01f0` | 37 | **`IsComplete()`** — returns true when `((array_end - array_begin) / 8) == expected_count`. If `array_begin` is null, returns true only if `expected_count == 0` |

### Practical impact for garlemald

1. **The `EntryBuilder::Detach` self-destruct pattern means the engine
   creates these on the heap.** When garlemald sends member-add /
   member-remove broadcasts, the receiving client builds a fresh
   `EntryBuilder` per event (via the `EntryBuilderCreate` factory),
   feeds it the packet bytes, then calls `Detach` to extract the
   resulting structure and destroy the builder. There's no expectation
   on garlemald's end that the builder is reused across events.

2. **OnlineStatusUpdater needs an explicit `expected_count` field on
   the wire.** The Send + IsComplete logic checks `array_size ==
   expected_count`, so the broadcast must declare up-front how many
   member status entries follow. Garlemald's group broadcasts should
   set this count correctly when emitting an OnlineStatus update — if
   it sends fewer entries than declared, the client never marks the
   updater complete and the UI hangs.

3. **BreakupBuilder is trivial — garlemald should also keep its
   breakup packet trivial.** A single fire-and-forget message; no
   need for member-list payload.

## Group::EntryBuilderBase + EntryBuilder slot maps (item #4)

`EntryBuilderBase` (19 slots, RVA `0xbd415c`) is the **single-use,
self-destructing builder** that produces an outbound packet for one
group event. `EntryBuilder` (RVA `0xbd442c`) is the concrete subclass
used for party/content groups; `EntryLinkShellBuilder` (RVA `0xbd447c`)
is the linkshell variant. They share the abstract slot shape.

### EntryBuilderBase object layout (deduced from slot bodies)

```c
struct EntryBuilderBase {
  /* +0x00 */ void**   vtable;
  /* +0x10 */ uint8_t  inline_data[24];   // payload area passed to slot 6/7
  /* +0x28 */ uint64_t sequence_pair;     // slot 2 = get, slot 3 = reset
  /* +0x30 */ uint8_t  state_flag;        // slot 4: == 1, slot 5: == 0
};

// EntryBuilder extends with a pimpl-pointer pattern:
struct EntryBuilder : EntryBuilderBase {
  /* +0x38 */ BuilderImpl* impl;          // most overrides forward here
  /* +0x3c */ uint16_t    member_count;   // slot 9 returns this
  /* +0x3e */ uint8_t     is_complete;    // slot 17 sets, slot 18 reads
};
```

### EntryBuilderBase slot map

| Slot | Body RVA | Bytes | Role |
|---:|---|---:|---|
| 0 | `0x2d0cd0` | 93 | Destructor (SEH-protected, calls parent dtor at `0x6d0b90`, optional `operator delete`) |
| 1 | `0x6b7340` | 3 | `xor eax,eax; ret` — trivial returns 0/false (inherited stub) |
| 2 | `0x2d0c90` | 7 | `GetSequencePair()` — returns `[+0x28]` in EAX, `[+0x2c]` in EDX (a u64 pair) |
| 3 | `0x2d0ca0` | 9 | `ResetSequencePair()` — zeros `[+0x28..+0x2c]` |
| 4 | `0x2d0cb0` | 10 | `IsState1()` — returns `[+0x30] == 1` |
| 5 | `0x2d0cc0` | 9 | `IsState0()` — returns `[+0x30] == 0` |
| 6 | `0x672a20` | 3 | `ret 0xc` — base no-op (subclasses override) |
| 7 | `0x672a20` | 3 | (same as 6) |
| 8 | `0x5d364d` | — | `__purecall` — subclasses MUST override (the build hook) |
| 9 | `0x1b8d90` | — | Inherited `LuaControl` no-op |
| 10 | `0x40fa00` | — | Inherited |
| 11 | `0x1c5c80` | — | Inherited |
| 12 | `0x837620` | — | Inherited (shared across many classes — likely `GetClassId`) |
| 13 | `0x5d364d` | — | `__purecall` — subclasses MUST override |
| 14 | `0x5d364d` | — | `__purecall` — subclasses MUST override (the detach/finalize hook) |
| 15 | `0x376340` | — | Inherited |
| 16 | `0x130890` | — | Inherited |
| 17 | `0x2ce2e0` | 1 | `ret` — empty no-op |
| 18 | `0x773290` | 3 | `mov al,1; ret` — returns true (default `IsActive`?) |

### EntryBuilder concrete overrides

13 of the 19 slots are overridden by `EntryBuilder` — almost everything
delegates through the pimpl at `[+0x38]`:

| Slot | Override RVA | Bytes | Role |
|---:|---|---:|---|
| 0 | `0x2dac90` | 27 | Concrete dtor (calls `0x6cb760` parent dtor) |
| 1 | `0x2c0550` | 7 | `GetInnerWork()` — returns `[[+0x38] + 4]` (the SharedWork ptr from the impl) |
| 6 | `0x2cac90` | 32 | `OnAddMember(member, sub_idx)` — calls `0x6ca270` with `(impl, payload, &inline_data, sub_idx, 1)` |
| 7 | `0x2cacb0` | 32 | `OnRemoveMember` — same shape, calls `0x6ca590` |
| 8 | `0x2cb7e0` | 8 | `Build()` — tail-jumps `[+0x38]->build_method` (`0x6cb5f0`) |
| 9 | `0x2da9b0` | 5 | `GetMemberCount()` — returns u16 at `[+0x3c]` |
| 10 | `0x2c3550` | 8 | Forward to `[+0x38]` impl (slot variant) |
| 11 | `0x2c0ce0` | 8 | Forward to `[+0x38]` impl |
| 13 | `0x2c01e0` | 4 | `GetImpl()` — returns `[+0x38]` (the pimpl pointer) |
| 14 | `0x2cb7f0` | 101 | **`Detach(out_subpacket)`** — see below |
| 15 | `0x2c0560` | 3 | `ret 4` — accepts 4-byte arg, ignores |
| 16 | `0x2cd680` | 8 | Forward to `[+0x38]` impl |
| 17 | `0x2c0570` | 5 | `MarkComplete()` — sets `[+0x3e] = 1` |
| 18 | `0x2da9c0` | 4 | `IsComplete()` — returns `[+0x3e]` |

### Slot 14 — `EntryBuilder::Detach(out_subpacket)`

The most important override. SEH-protected, takes one out-pointer arg.
Decoded body:

```cpp
SubPacket EntryBuilder::Detach(SubPacket** out_subpacket) {
    SubPacket* impl_handoff = this->impl_;     // +0x38
    this->impl_ = nullptr;                     // detach ownership
    *out_subpacket = impl_handoff;             // hand to caller
    if (this != nullptr) {
        // Tail-call vtable[0] (dtor) with delete-flag = 1
        this->vtable[0](this, 1);              // self-destruct
    }
    return *out_subpacket;
}
```

This is the **single-use builder pattern**: `EntryBuilder` is created
on the heap, populated by repeated `OnAddMember` / `OnRemoveMember`
calls, then `Detach` hands off the constructed packet to the caller
and immediately deletes the builder. Mirrors C# `using
(EntryBuilder b = …) { … }` but C++-side via explicit ownership
transfer.

The 19-slot vtable is therefore the **complete event-emission API
for one group transition** — Add/Remove members, Build, Mark complete,
finally Detach. The pimpl at `+0x38` holds the actual SubPacket
under construction; the EntryBuilder is just the typed lifecycle
wrapper.

### Practical impact for garlemald

1. **No wire-format gap** — garlemald's `build_synch_group_work_values_content_init`
   matches retail bytes exactly. The SEQ_005 hang is not a malformed
   0x017A packet.

2. **The hang is upstream of the 0x017A reply.** If garlemald's IN
   handler isn't receiving the 0x0133 GROUP_CREATED message in the
   right shape (e.g. the synthetic group_id prefix `0x2680XXXX` or
   `0x80000000` isn't being matched), the reply never fires. The
   garlemald handler at `map-server/src/processor.rs:6325` filters by
   `event_name == "/_init" && high == 0`; the `high == 0` check is the
   right filter for content-director groups but excludes player-work
   groups (`high & 0x80000000`). For SEQ_005 specifically, this is
   the right filter (content-director group is what the cinematic
   needs).

3. **0x0133 OUT (GenericDataPacket) might be needed for SEQ_005 too.**
   The C# project-meteor `attentionMessage(player, textId, ...)`
   helper sends BOTH a SendGameMessage AND a SendDataPacket("attention",
   …). If garlemald's tutorial cinematic currently only emits the
   SetActorProperty path and skips the attentionMessage, popups won't
   fire. Search `scripts/lua/quests/man/man0g0.lua` for
   `attentionMessage` calls and audit garlemald's Lua binding side.

## Cross-references

- `docs/sync_writer.md` — Phase 6 #4 (the per-field SyncWriter that
  drives state-change emits into the SharedWork pipeline)
- `docs/director_quest.md` — Phase 6 architecture (the *Base classes
  that own SyncWriter Work fields)
- `docs/wire-protocol.md` — Phase 3 (the GAM CompileTimeParameter
  registry that names each SyncWriter's wire ID)
- `docs/cpp_bindings_index.md` — `groupbaseclass` 15 client-side methods
- `garlemald-server/map-server/src/runtime/broadcast.rs` — the
  GroupHeader/Begin/X08/End trio emitter
- `garlemald-server/map-server/src/processor.rs` — the 0x0133 inbound
  dispatch + reply (`SynchGroupWorkValues`)
- Memory: `project_garlemald_seq005_b2.md`, `project_garlemald_seq005_b4.md`
  — the LuaParty:AddMember / LuaDirectorHandle:AddMember work that
  surfaces the GroupHeader/Begin/X08/End trio
- Memory: `project_garlemald_seq005_now_loading_hang.md` — the SEQ_005
  same-zone DoZoneChangeContent hang (current open blocker; Phase 8
  item #7+#8 directly address its root cause)
