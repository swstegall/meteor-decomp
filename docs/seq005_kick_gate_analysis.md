# SEQ-005 kick-gate — ffxivDecomp cross-reference + next anchors

Cross-references meteor-decomp's long-running SEQ-005 cutscene/kick-hang
analysis against ffxivDecomp's independent RE of the same inbound-receiver
region. **This does not fix the hang** — ffxivDecomp explicitly never
covers the gate mechanism — but it names neighbouring functions and shows
that two separate efforts converged on the **same 0x0089exxx code
cluster**, which sharpens the next experiment.

## What meteor-decomp already had (the kick gate)

From the garlemald SEQ-005 work (server side) + prior Ghidra passes:

- `FUN_0089f180` — parses the inbound kick packet; **conditionally**
  calls the setter below iff the event_type byte (at `receiver+0x68`,
  packet body offset 8) == `0x05` (the "noticeEvent" tag).
- `FUN_0089e200` — setter; writes `receiver+0x80` (= `LuaParamsContainer+0x14`)
  to 1. If event_type != 5, the gate stays 0 and the kick silently
  falls through (Branch B1).
- `FUN_006e3440` = `MyPlayer::vtable[3]` — swaps the dispatcher `[+0xf8]`
  subscriber (the mechanism that *clears* the kick-gate); single absolute
  ref in the binary; a C++ virtual, not a Lua binding.
- `MyPlayer` vtable slot 66 = `_fadeInNowLoadingForNoticeEventJustInArea`
  (the kick-dispatcher clearer), slot 52 = `_cancelNotice`.

All five of these are still `FUN_xxxxxxxx` in `config/ffxivgame.symbols.json`.

## What ffxivDecomp adds (same cluster, named from the packet side)

From `ffxivDecomp/docs/re/exe/finding_polymorphic_block_userdataReceiver.md`:

- `FUN_0089eed0` = **`Network::UserDataReceiver` ctor** — the inbound
  data-packet receiver. Two vtables: primary `0x010574a4`, secondary
  `0x01057488`; object size `0x24`.
- `FUN_0089fbf0` = secondary-vtable slot 23 = **multi-mode dispatcher**,
  a 4-way switch on the mode byte at `this+0x10`:
  `0`=CharaBase actor, `1`=numeric id, `2`=name, `0xff`=broadcast. Also
  the router for non-polymorphic inbound dispatch entry 42.
- UserDataReceiver object fields (inferred): `+0x10` mode byte,
  `+0x14` target ref, `+0x18` resolved target pointer, `+0x04` payload
  container, `+0x20` container head.

Both are now seeded into `config/ffxivgame.ffxivdecomp_symbols.json`
(`Network_UserDataReceiver_ctor`, `Network_UserDataReceiver_multiModeDispatcher`).

## The synthesis

The kick receiver and `UserDataReceiver` are the **same object family**
in the same `0x0089exxx` cluster, analysed from two sides:

- meteor-decomp came at it from the **gate**: event_type 0x05 →
  `receiver+0x80`.
- ffxivDecomp came at it from the **packet class**: `UserDataReceiver`
  with a mode byte at `+0x10` driving a 4-way dispatch.

`receiver+0x80` (= `LuaParamsContainer+0x14`) and the UserDataReceiver
`+0x10/+0x14/+0x18` fields are very likely the same struct viewed at
different offsets. The cutscene-clip opcodes 4–18 (now all named — see
`docs/ffxivdecomp_opcode_binding_map.md` and the harvested
`ZoneIn_handler_opcode_*` symbols) are the **post-kick rendering flow**;
the kick itself is what arms that flow, via this receiver.

## Concrete next anchors (highest-value first)

1. **Read `FUN_0089f180` (gate parser) and `FUN_0089fbf0` (mode
   dispatcher) together.** They operate on the same object at adjacent
   offsets (`+0x68` event_type vs `+0x10` mode byte vs `+0x80` gate).
   Mapping event_type → mode → gate in one pass should reveal whether the
   SEQ-005 "noticeEvent" path (type 5) takes a dispatch branch that the
   same-zone `DoZoneChangeContent` path never arms.
2. **DONE — the clearer body is `FUN_006E32F0`.** Dumping MyPlayer's
   vtable (`tools/analyze_legacy_struct.sh --vtable Client::Control::MyPlayer`)
   gives `vt[52] = 0x006E8F50` and `vt[66] = 0x006E32F0`. The 14-slot span
   matches the PlayerBase registrar order exactly (`_cancelNotice` → 14
   bindings → `_fadeInNowLoadingForNoticeEventJustInArea`), cross-
   validating the slot IDs. So:
   - `FUN_006E32F0` = `MyPlayer::_fadeInNowLoadingForNoticeEventJustInArea`
     — **the kick-dispatcher clearer body; decompile this** for the hang.
   - `FUN_006E8F50` = `MyPlayer::_cancelNotice`.
   The **full 39-binding → MyPlayer-vtable-slot map is now done**:
   `config/ffxivgame.myplayer_bindings.json` (via
   `tools/map_playerbase_vtable_slots.py`, which asserts both anchors so it
   can't silently mislabel). Bodies named `MyPlayer::_<binding>` — 2
   validated, 13 high (bracketed by the anchors, slots 52–66), 24 inferred
   (same thunk-order mechanism, anchored one side — verify before relying).
   The validated + high subset is folded into the name-override layer.
3. **`FUN_006fb9c0` = `CutScene_invokeLua_onFinalizeClip`** (opcode 14,
   dual-pass Preview+Personage). The finalize is what the working warp
   path reaches and the hung path may not. Cross-check against the
   garlemald "Now Loading" hang (same-zone warp never sends the zone-in
   complete that would let finalize run).

## Caveat

ffxivDecomp's own conclusion stands: it does **not** decode the
event_type-0x05 gate, `npc+0x128` priming, or why the same-zone path
hangs. The most recent garlemald diagnosis points at the warp not
completing client-side (the "Now Loading" hang), which may be upstream of
the gate entirely. Treat the above as sharpened anchors, not a solution.
