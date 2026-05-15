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
| #4 | EntryBuilderBase 19-slot map | 🔲 pending — walk slots 0..18 to identify per-event hooks |
| #5 | PacketRequestBase 13-slot map | 🔲 pending — walk slots 0..12 (send-side mirror of PacketProcessor) |
| #6 | OnlineStatusUpdater + BreakupBuilder slot maps | 🔲 pending |
| #7 | 0x0133 wire-format derivation from packet captures | 🔲 pending — diff garlemald 0x0133 emissions against retail captures (`captures/retail_pcap_*`) using `packet-diff/` |
| #8 | Audit garlemald's per-member SharedWork serialization vs. the 16-byte stride | 🔲 pending — `map-server/src/runtime/broadcast.rs` |
| #9 | Find the runtime registration site for ZoneProtoDownCallbackInterface — gives us the real 0x0133 handler RVA | 🔲 pending — search for code that writes a vtable ptr into the dispatcher's `ecx` arg storage |

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
