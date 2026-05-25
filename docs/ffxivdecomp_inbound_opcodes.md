# ffxivDecomp inbound (server→client) Zone opcode roster

The second-level Zone inbound dispatch table at **`0x00fdfb80`** (RVA
`0xbdfb80`; ~224 declared entries, ~70 in the used range). Consolidated
from ffxivDecomp's inbound-dispatch findings (used with permission — see
`NOTICE.md`); cross-referenced, not byte-verified. Handler / Lua-invoker
function names are in `config/ffxivgame.ffxivdecomp_symbols.json`
(grep `ZoneIn_handler_` / `_invokeLua_`).

This is the **server→client** event stream — the opcodes a server pushes
to drive client behaviour (the outbound/client→server family 0x12d–0x135
is in `docs/ffxivdecomp_opcode_binding_map.md`). Opcode numbers here are
the **dispatch-table index**, not the wire opcode word.

## Lua-hook opcodes (~30) — server push → client `_on*` hook

| Op | Lua hook | Actor class | Notes |
|----|----------|-------------|-------|
| 0  | `_onTouch` (proxBegin) | Player | proximity / gathering / sit / raid enter |
| 1  | `_onTouch` (proxEnd)   | Player | proximity leave |
| 2  | `_onMoveAtSit` | Player | |
| 4  | `_onTargetChanged` | DesktopWidget | soft target (hover/lock preview) |
| 5  | `_onTargetDecided` | DesktopWidget | hard target commit |
| 7  | `_onInitializationClip` (PreviewSetupClip) | CutScene | char-create/mount/gear preview |
| 8  | `_onInitializationClip` (Personage) | CutScene | story/quest cutscene |
| 9  | `_onShowUIClip` | CutScene | (pairs 10) |
| 10 | `_onHideUIClip` | CutScene | |
| 11 | `_onShowWidgetClip` | CutScene | (pairs 12) |
| 12 | `_onHideWidgetClip` | CutScene | |
| 13 | `_onOpenUIClip` | CutScene | |
| 14 | `_onFinalizeClip` | CutScene | dual-pass Preview+Personage; cutscene end |
| 16 | Debug script exec | Debug | dev/QA arbitrary Lua; disable in prod |
| 17 | `_onPreCutSceneCancel` | System (0xc0000024) | cutscene skip/cancel start |
| 18 | `_onPostCutSceneCancel` | System | cancel end |
| 20 | `_onPreWarp` | DesktopWidget | (pairs 21) |
| 21 | `_onPostWarp` | DesktopWidget | warp finish — relevant to the SEQ-005 warp path |
| 43 | `_onChangeSystemFlag` | CharaBase | flag toggle |
| 44 | `_onReceiveLimitAddicted` | — | anti-fatigue / play-time notice |
| 60 | `_onFinalize` | — | actor destroyed |

## Non-Lua state opcodes (~6) — direct C++ state, no Lua hook

| Op | Role | Notes |
|----|------|-------|
| 3  | 4-arg payload (family A) | placeholder; no specific Lua |
| 6  | `GET_CURRENT_TARGET` query | reads DesktopWidget+0x70 |
| 19 | tree-node flag set | sets byte at node+4 in container at this+0x130 |
| 38 | `_onReceiveDataPacket` | generic 192-byte data packet |
| 39 | internal map insert | |
| 40 | timed-execution scheduler | |
| 41 | request-response result push | |

## Polymorphic UserDataReceiver (entries 22–26, 42)

One packet class (`Network::UserDataReceiver`, primary vtable
`0x010574a4`) with 2 active slots + 3 inherited no-ops; entry 42 is its
multi-mode dispatcher (`FUN_0089fbf0`, mode byte at +0x10: 0=actor /
1=id / 2=name / 0xff=broadcast). See
`docs/seq005_kick_gate_analysis.md` — same `0x0089exxx` cluster as the
kick gate.

| Op | Slot | Role |
|----|------|------|
| 22 | primary slot 21 | NO-OP (inherited) |
| 23 | primary slot 22 | append payload entry → container at this+4 |
| 24 | primary slot 23 | resolve target actor → this+0x18 |
| 25, 26 | slots 24, 25 | NO-OP (inherited) |
| 42 | secondary slot 23 | UserDataReceiver multi-mode dispatch |

## Network-receiver opcodes (~14) — per-subsystem receiver classes

These map to specific `Network::*Receiver` classes (cross-ref
meteor-decomp's Receiver→LuaActorImpl-slot work; the 0x0089exxx /
0x008axxxx receiver region).

| Op | Receiver | Owner | Subsystem |
|----|----------|-------|-----------|
| 27 | SetEventStatusReceiver | NpcBase | event-status (talk/push/emote enable) |
| 45 | HateStatusReceiver | NpcBase | enmity/hate nameplate |
| 46 | ChocoboReceiver | MyPlayer | mount |
| 47 | ChocoboGradeReceiver | MyPlayer | |
| 48 | GoobbueReceiver | MyPlayer | |
| 49 | VehicleGradeReceiver | MyPlayer | |
| 50 | GrandCompanyReceiver | PlayerBase | GC rank/affiliation (polymorphic) |
| 53 | AchievementPointReceiver | — | |
| 54 | AchievementTitleReceiver | — | |
| 55 | AchievementIdReceiver | — | |
| 56 | AchievementCountReceiver | — | |
| 58 | JobChangeReceiver | — | |
| 59 | EntrustItemReceiver | — | |

## Chat opcodes (4) — Command-update notifications

| Op | Variant | Notes |
|----|---------|-------|
| 35 | chat A | single-name Command update |
| 36 | chat B | 40-char name |
| 37 | chat C | recipient-targeted (/tell-style) |
| 57 | chat D | variant D |

## No-op / reserved (10)

`15` (in cutscene band), `28–34` (7-slot cut-feature cluster — possibly
retainer/materia/housing), `51–52` (between mount and achievement bands).
Wrappers are empty `void f(void){return;}`. A server never needs to send
these.

---

**Totals:** ~60 active + 10 no-op ≈ 70 used entries. For a server bring-up,
the load-bearing inbound pushes are: the cutscene block (7–14) + cancel
(17/18) for events, warp (20/21), the SetEventStatus (27) / Hate (45)
receivers for NPC interaction, and the chat block (35–37/57).
