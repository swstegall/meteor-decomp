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

## Decompiled bodies (2026-05-25, post-reimport) — mechanism mapped

The fresh Ghidra 12.1 project + the applied names made the cluster
decompilable (`tools/ghidra_scripts/DecompileToText.java`, `DECOMP_VAS=`).

**`FUN_006E32F0` = `MyPlayer::_fadeInNowLoadingForNoticeEventJustInArea`
(the clearer):**
```c
void __fastcall clearer(int this) {
  if (*(int*)(this+0x128) != SENTINEL || *(int*)(this+0x12c) != SENTINEL) {
    FUN_00cc7510();      // fade-in / hide "Now Loading"
    FUN_0075b510();
    *(int*)(this+0x128) = SENTINEL;   // reset notice-event state
    *(int*)(this+0x12c) = SENTINEL;   // (SENTINEL = DAT_0130c778)
  }
}
```
So `MyPlayer+0x128`/`+0x12c` ARE the notice-event state (the `npc+0x128`
priming field from #8c, on MyPlayer); this resets them AND clears the
"Now Loading" overlay. **The SEQ-005 hang is precisely this clearer never
running for the same-area path** — the overlay stays up. It's a Lua
binding, so the notice-event script is what must call it.

**`FUN_0089e200` (gate setter):** `*(byte*)(LuaParamsContainer+0x14) = 1`
then memsets + seeds the LuaParams buffer (first byte = 1). Arms the gate.

**`FUN_0089f180` (kick parser):** builds `KickClientOrderEventReceiver`,
stores event_type at `+0x68`; iff `event_type == tag` AND `FUN_0078f840`
returns true, calls the gate setter. Confirms the event_type-0x05 gate
(now with the extra `FUN_0078f840` secondary condition surfaced).

**Next concrete step (fix path):** the clearer is invoked from Lua, so the
question is which notice-event script call reaches it and why garlemald's
same-zone `DoZoneChangeContent` flow never does. Cross-reference against
the #8c finding (garlemald never makes the client fire the trigger event —
missing `SetPushEventCondition*` packets). Trace: who calls the clearer
binding (script side) → what server event/packet arms that script path for
the JustInArea (same-zone) case → emit it from garlemald's
`DoZoneChangeContent`. Also decompile `FUN_00cc7510`/`FUN_0075b510` (what
the clearer calls) + `FUN_0078f840` (the gate's secondary condition).

## Trace conclusion (2026-05-25) — the fix is server-side

`FindCallers` on the clearer `FUN_006E32F0`: **exactly one reference — a DATA
ref from the MyPlayer vftable at `0x00fd7964` (slot 66). No static call
sites.** So it is invoked only via virtual dispatch / the Lua binding
`_fadeInNowLoadingForNoticeEventJustInArea` — i.e. a **script** calls it after
zone-in completes. Callees: `FUN_0075b510 → FUN_004d70b0(1)` (render/scene
enable — the un-hide behind the fade-in); `FUN_00cc7510` (paired UI op).

Therefore the SEQ-005 "Now Loading" hang is NOT a missing client function —
it's that garlemald's same-zone `DoZoneChangeContent` never drives the client
to the zone-in-complete state whose script calls the clearer. This matches the
earlier garlemald diagnosis (client sends RX `0x0007` zone-in-complete after
working cross-zone warps but never after the same-zone one; same-zone path
skipped the `do_zone_change_with_private_area` helper — `is_updates_locked`
bracket + spatial-grid re-insert). **Fix lives in garlemald**, not the client:
make `DoZoneChangeContent` run the full cross-zone zone-in completion so the
client reaches the script path that fires the clearer.

## Convergence: the gap is the CLEAR side (EndEvent), not arming (2026-05-25)

- `FUN_0078f840` (gate secondary condition): reads the next kick-stream byte
  and sets the flag = `(byte == 3)`. So the gate arms iff `event_type == tag`
  AND that byte == 3. garlemald's kick body is byte-identical to pmeteor (#8a),
  so **arming is fine** — Now Loading does come up.
- The clearer `FUN_006E32F0` resets `MyPlayer+0x128` AND `+0x12c` and fades in.
  **`+0x12c` is exactly the `context_root[+0x12c]` that #8a said must be
  cleared** via "EndEvent slot 3's 102-case dispatcher." Two independent
  threads converge: the clearer IS the function that clears `+0x12c`, and it
  runs on the noticeEvent **ending**, not starting.

**Therefore:** garlemald (`apply_do_zone_change_content`) already arms the
notice event (synthetic noticeEvent EventStart, byte-identical kick, the
`do_zone_change_with_private_area` helper, zone-in replay) — but never drives
the **EndEvent** path whose noticeEvent case fires the clearer, so `+0x12c`
stays set and "Now Loading" never fades. The fix is on the END side.

**Next:** (a) decompile the EndEvent receiver's 102-case dispatcher to find the
noticeEvent-end case + the packet field that selects it (need its address —
EndEvent ≈ LuaActorImpl vtable slot 58, same 0x0089exxx cluster); OR (b) the
empirical experiment #8a proposed, now well-motivated: have garlemald emit the
EndEvent (and/or pmeteor's 5 extra pre-kick SetEventStatus) after the synthetic
noticeEvent and test whether Now Loading clears via fresh-start-gridania.sh.

## EndEvent dispatch chain (2026-05-25) — fully mapped client-side

`EndClientOrderEventReceiver` (vftable `0xc57348`, 5 slots) is the EndEvent
sibling of the kick's `KickClientOrderEventReceiver`. Slot 3 = `FUN_0089e2d0`
(thin wrapper; reads receiver `+0x8` payload, `+0xc/+0xd/+0x10`) → calls the
dispatcher `FUN_008a13a0`:
- 2-level switch: first on the event **category** (`*param_1`), cases 0–5 fall
  through to a second switch on the **sub-type** (`*puVar1`, from `param_2+0x18`);
  categories `0x32–0x37` go elsewhere (`FUN_008a10c0`); default returns.
- sub-type switch cases `0→FUN_006e1080`, `1→FUN_006e10a0`, `4→FUN_006e10c0`,
  `5→FUN_006e10e0` — all thin thunks into the `FUN_00893xxx` event-end
  subsystem (`8934a0/893520/8935b0/893800`), which run the event-end Lua.

So the **client fades in only when it receives the `EndClientOrderEvent`
packet** that drives `FUN_008a13a0` to the noticeEvent end-handler, which runs
the end-script that calls the Lua clearer `_fadeInNowLoadingForNoticeEventJustInArea`.
**Pivotal garlemald question:** does `apply_do_zone_change_content` actually
send `EndClientOrderEvent` for its synthetic noticeEvent (with a category/
sub-type that hits a real case, not `default`)? If not, that's the fix.

## Actionable conclusion (2026-05-25) — the testable garlemald hypothesis

garlemald already sends `0x0131 EndEvent` (`map-server/.../send/events.rs
build_end_event`); `0x0133` is its GenericData/GroupCreated. The client's
fade-in dispatcher `FUN_008a13a0` switches on the EndEvent **body** bytes —
category `*param_1` (cases 0–5 reach the sub-switch; 0x32–0x37 elsewhere;
**default = return, no fade-in**) then sub-type `*puVar1` (cases 0/1/4/5 →
the `FUN_00893xxx` end-handlers that run the end-script → the clearer).

**Hypothesis to test:** garlemald's `0x0131 EndEvent` for the synthetic
noticeEvent carries category/sub-type body bytes that hit `default` (or no
end-handler), so `FUN_008a13a0` no-ops, the end-script never runs, the Lua
clearer is never called, `MyPlayer+0x12c` stays set, and "Now Loading" hangs.

**Fix (task #24, live):** compare garlemald's `0x0131` EndEvent body for the
noticeEvent against pmeteor's in the gridania captures (the body offsets that
become `*param_1`/`*puVar1` — i.e. EndClientOrderEvent receiver fields
`+0xc/+0xd/+0x10`, set in `FUN_0089e2d0`), set garlemald's to the noticeEvent
values, and re-test via `fresh-start-gridania.sh` with packet logging. If the
body offsets are unclear, decompile `FUN_0089e2d0`'s param setup + `FUN_00cc7a50`
to map receiver field → switch selector.

## Static-decomp limit reached (2026-05-25) — pivot to empirical body read

Tried to pin the exact EndEvent body offsets statically and hit the floor:
- `FUN_00cc7a50` (category selector source) is a **generic container accessor**
  (`FUN_00cd80f0` discriminant → `FUN_00cd8160`/`FUN_00cd81d0` getter → deref),
  so the category = "first element of the receiver `+0x8` payload container,"
  not a fixed wire offset.
- No static refs to the `EndClientOrderEventReceiver` vftable `0xc57348` — the
  receiver is built by a template/factory, so there is no single parser to read
  the packet→field mapping from.

Conclusion: the **mechanism + selectors are fully mapped** (category = payload
container head; sub-type = receiver `+0xc`; dispatcher `FUN_008a13a0`
category×sub-type → end-handler → end-script → clearer → fade-in). The exact
noticeEvent category/sub-type **values** are now best read empirically from the
captures — extract pmeteor's `0x0131 EndEvent` body for its noticeEvent and
compare to garlemald's, then set garlemald's body to match. That is task #24
prep, not more decomp.

## PINNED (2026-05-25) — garlemald never ENDS the synthetic noticeEvent

Targeted body-comparison result (code + decomp; pmeteor capture inconclusive
on exact bytes — it sends 0 `0x12f` kicks, a different start mechanism, and its
`0x131` bodies don't expose the noticeEvent end at garlemald's offsets):

garlemald `apply_do_zone_change_content` (map-server/src/processor.rs ~3185)
opens the noticeEvent — sets `event_session.current_event_{name="noticeEvent",
type=5}` and calls `dispatch_event_start_to_content_director(...,"noticeEvent",
type 5,...)` — but there is **NO `build_end_event` call anywhere in the content
path**. `build_end_event` is only reached via the Lua `EndEvent` command,
journal qtdata, and post-zone-in — none of which the synthetic noticeEvent
triggers (it has no real client script to call `player:EndEvent()`).

Decomp says the client fades in (clears `MyPlayer+0x12c`, hides "Now Loading")
ONLY on receiving the `0x0131 EndClientOrderEvent` whose category routes to the
noticeEvent case. So: started, never ended → hang.

**FIX (task #24):** after the synthetic noticeEvent EventStart + content
warp/zone-in, garlemald must send `build_end_event(player, director,
"noticeEvent", event_type=5)` to close the event so the client's
`FUN_008a13a0` dispatcher (category=5) runs the end-handler → end-script →
`_fadeInNowLoadingForNoticeEventJustInArea` → fade-in. event_type=5 matches the
open session the kick created (the client takes owner/sub-type from that
session, per build_end_event's own doc-comment). Confirm timing + body via the
live `fresh-start-gridania.sh` test.


## ffxivDecomp 2026-05-28 session — new leads

_Cross-referenced from ffxivDecomp's 2026-05-27/28 session, not byte-verified. The six EndEvent/kick gate bodies this doc decompiled (`FUN_008a13a0`/`0089e2d0`/`006e32f0`/`0089f180`/`0089e200`/`0078f840`) are confirmed ABSENT from the ffxivDecomp 993-symbol import — they remain meteor-decomp-only decomp work. Clarifier: ffxivDecomp numbers the clearer as PlayerBase registration slot 25 (registrar `0x003324f0`, thunk `LAB_0071e3f0`); this doc's vtable slot 66 is the same binding via a different table. **0x0131 disambiguation:** ffxivDecomp's client-OUT `0x131` is a 24B `_sendByteToggle`, NOT the server→client EndClientOrderEvent — the `0x0131` in this doc is garlemald's own server-side constant. Confirm via Ghidra which server→client wire opcode `EndClientOrderEventReceiver` (vftable `0xc57348`, slot 3 = `FUN_0089e2d0`, dispatcher `FUN_008a13a0`) actually consumes before the task #24 body-offset experiment._

1. InstanceRaidBaseClass.startEvent ENTRY-path clearer (finding_instance_raid_system.md + finding_questbaseclass_quest_engine_model.md): the clearer _fadeInNowLoadingForNoticeEventJustInArea is called BOTH at quest-reward/content-exit (showQuestRewardAsClientCall) AND at content-instance ENTRY in InstanceRaidBaseClass.startEvent (after processLogin(true), driven by an inbound directive). EXPERIMENT: decompile InstanceRaidBaseClass.lpb (already in lpb_corpus_survey.md line 37, 12 fns, NOT yet decompiled) via tools/decode_lpb.py+unluac to recover the exact inbound directive (opcode + A1/field selector) that drives startEvent, then test driving the client through that ENTER directive as an alternative to build_end_event. Most promising because it gives a second, possibly-more-faithful fix path for a synthetic noticeEvent that has no real reward/exit script.

2. ARM-side notice authorization spec (finding_server_notify_family_and_notice_authorization.md §9): callServerOnX -> 0x12d SIMPLE 128B -> ServerNotify_createResumeChecker_atCmdSubsystem_0xf8 -> script SUSPENDS on ClientOrderEventWaitingResumeChecker (legacy_structs.json:22051, vftable 0xc56dc8); server ACCEPT wakes it, REJECT -> _onNoticeRejected -> _onEventCancel(closeAllOwnedContentWidget + resetFade). EXPERIMENT: dump the pending-resume-checker queue after the hang to confirm a ClientOrderEventWaitingResumeChecker is stuck; decompile its 3-slot isReady() and confirm it polls the same MyPlayer+0x12c that the clearer FUN_006E32F0 resets. Confirms garlemald must drive BOTH accept/resume AND EndEvent for the synthetic event.

3. 0x0131 EndEvent opcode disambiguation (finding_outbound_complete_lobby_zone_chat.md): ffxivDecomp's client-OUT 0x131 is a 24B _sendByteToggle, NOT the server->client EndClientOrderEvent. The doc's '0x0131' is garlemald's own server-side constant. EXPERIMENT: before the task #24 fresh-start-gridania body-offset experiment, confirm via Ghidra which server->client wire opcode the EndClientOrderEventReceiver (vftable 0xc57348, slot 3 = FUN_0089e2d0, dispatcher FUN_008a13a0) actually consumes (these bodies are meteor-decomp-only, confirmed absent from the ffxivDecomp import). Prevents chasing the wrong opcode.

4. DesktopWidget +0x7b pre/post-warp gate (finding_inbound_routers_named_opcodes_round2.md): _onPostWarp (sub-idx 21) silently no-ops unless _onPreWarp (sub-idx 20) set DesktopWidget+0x7b=1. EXPERIMENT: verify garlemald's same-zone DoZoneChangeContent emits the pre-warp sub-event before the warp; if not, the client never arms +0x7b and the post-warp state machine stalls — a hang cause distinct from and possibly upstream of the kick-gate/EndEvent theory.

5. Client-side inbound 0x07 'resync loop' (finding_spawn_wire_side_CLOSED_opcode_0x17c_zone_main_inbound_dispatcher.md): FUN_004dc690 case 0x07 is a state-resync handshake, implying zone-in completion is a handshake not a one-shot. EXPERIMENT: decompile the case-0x07 branch (VA 0x004dc690) to map what server state it walks and the client-state precondition that makes the client EMIT vs only CONSUME 0x07 — the same-zone DoZoneChangeContent path likely fails that precondition.

6. Stalled async-CSV-load coroutine (ffxivtool_export_overview.md): the engine's standard yield is FunctionEndCallback (40B) + LoadDataResumeChecker (148B). If a content-script CSV load is in-flight when the same-zone path runs (and that path skips do_zone_change_with_private_area's re-bootstrap), the coroutine never resumes -> also manifests as 'never sends RX 0x0007'. EXPERIMENT: HWBP CoroutineContext_pushResumeChecker (FUN_00cd2860) and LoadDataResumeChecker ctor (FUN_00725a50) during a broken warp; compare against a working cross-zone warp to see if the working path drains these checkers.

7. 0x143 despawn-on-zone-change scan (finding_opcode_0x143_DESPAWN_packet_breakupBuilder_path.md): FUN_006c5020 (DespawnPipeline_scanPendingList) emits despawns on zone change; the despawn-clearer FUN_00703970 (context_root_128_priming_hunt.md) clears MyPlayer+0x128/+0x12c IFF a despawn id matches the kick prev-target. EXPERIMENT: check whether a working cross-zone warp's despawn-scan clears +0x12c while the same-zone path skips it; if so, drive a matching despawn as an alternative clear path.

8. PLAYER_MODE_TICKER bindings (finding_perFrameTick_subsystems_COMPLETE_15_slots_characterized.md): PerFrameTick slot 1+0x110 (FUN_0075d120) polls bindings 0x7a121/0x7a122/0x7a123 + mode root 0xc0000024 and keeps the client in event/cutscene mode. EXPERIMENT: decompile FUN_0075d120, add 0x7a121-0x7a123+0xc0000024 to the bindWork catalog, and check whether the noticeEvent leaves MyPlayer in a locked mode the never-sent EndEvent would clear.

9. System cutscene-cancel path (finding_system_invokeLua_and_userdatareceiver_dispatch.md): sub-idx 17/18 (_onPre/PostCutSceneCancel) resolve System singleton 0xC0000024 and run an asymmetric 4-subsystem broadcast. EXPERIMENT: evaluate whether driving the System-cancel path could supply the missing 'close the synthetic noticeEvent' trigger as an alternate teardown to build_end_event.

10. CutScene finalize pump (finding_cutscene_block_complete_opcodes_4_to_18.md + finding_director_family_and_cutscene_closure.md): opcode 14 = CutScene_invokeLua_onFinalizeClip (FUN_006fb9c0, dual-pass), and onPostWarp (FUN_006fef10) — both in the imported 993-symbol set. EXPERIMENT: decompile these (they decompile cleanly with names) to capture the client-side completion state each asserts, then confirm whether onFinalizeClip/onPostWarp ever fire for the same-zone DoZoneChangeContent vs the working cross-zone warp.
