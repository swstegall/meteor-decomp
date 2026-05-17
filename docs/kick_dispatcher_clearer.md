# `context_root[+0x128]` / `[+0x12c]` clearer — `FUN_006e32f0` = MyPlayer::vtable[66]

> Recovered 2026-05-16. Supplements `docs/event_kick_receiver_decomp.md`
> (Phase 7 #1 / KickReceiver slot 2 decomp) — answers the open question
> "what clears `[+0x128]` and `[+0x12c]` so the next kick can use
> Branch B1 (init) instead of stale Branch A?"

## The clearer

`FUN_006e32f0`, **76 bytes**, RVA `0x002e32f0`, VA `0x006e32f0`. It's
the **only** function in the binary that writes NO_ACTOR (`0xE0000000`)
to BOTH `[+0x128]` and `[+0x12c]` of the kick dispatcher:

```c
void KickDispatcher::ResetTarget(this, arg1) {  // ECX=this, [ESP+8]=arg1
    EAX = *NO_ACTOR;                  // load 0xE0000000 sentinel
    if (this[+0x128] == NO_ACTOR && this[+0x12c] == NO_ACTOR)
        return;                       // already cleared — no-op guard
    
    arg = arg1[4];
    arg2 = trampoline(arg);           // CALL 0x00cc7510 — navigate to engine root
    something = arg2[0][+4][+8];
    FUN_0075b510(something);          // pre-clear notify / cleanup helper
    
    this[+0x128] = NO_ACTOR;          // clear previous-target
    this[+0x12c] = NO_ACTOR;          // clear current-target
    return;
}
```

The body is symmetric: it touches both fields, and only does any work
if at least one was non-NO_ACTOR. So calling this is **idempotent and
safe** — when both fields are already cleared it just returns.

## Discovery method

```sh
# 1. Find all .text writers to [reg + 0x12c]
# 2. Find all .text readers of the NO_ACTOR constant at 0x0130c778
# 3. Intersection = strongest candidates for "clearer"
```

Out of 40 writers and 146 NO_ACTOR readers, 4 functions intersect:

| Function | Role |
|---|---|
| `FUN_0089e450` | `KickReceiver::Receive` (already known — establishes the target) |
| `FUN_006e32f0` | **The clearer** (this doc) |
| `FUN_00703970` | Unrelated — 414-byte fn with 5 internal call sites |
| `FUN_00773270` | Unrelated — 283-byte fn with 1 caller (FUN_0057a3c0) |

Only `FUN_006e32f0` writes NO_ACTOR (the load-bearing semantic). The
other two write register values that happen to be NO_ACTOR sometimes,
or set up the target rather than clear it.

## The clearer is a virtual method — `MyPlayer::vtable[66]`

`FUN_006e32f0` has **zero direct CALL rel32 callers**. Its only address
reference in the binary is at `.rdata 0xbd7964` — a slot in the
vtable of `Application::Lua::Script::Client::Control::MyPlayer`
(RTTI confirmed via COL→TD walk; vtable starts at `0xbd785c`).

```
MyPlayer vtable @ 0xbd785c
  slot 0..65: ...
  slot 66:    FUN_006e32f0   ← THE CLEARER
  slot 67..:  ...
```

So the clearer is **invoked as a virtual method on the local-player
MyPlayer instance**. Whatever class holds the `[+0x128]/[+0x12c]`
target-state IS the MyPlayer instance (or a parent class that shares
the layout — `MyPlayer : PlayerBase : CharaBase : ActorBase`, per the
hierarchy in `docs/receiver_classes_inventory.md`).

Direct virtual-call searches (`CALL [reg + 0x108]` for slot 66 *
4 = 0x108) return **zero hits** in `.text`. Same outcome as Phase 9 #5
([[receiver_dispatch_via_actorimpl]]) — the dispatch chain isn't
through a static C++ virtual call. Most likely path:

1. **Lua VM closure**: MyPlayer's 90+ vtable slots are Lua-bindable
   methods. Slot 66 is exposed under a Lua name (probably
   `mainPlayer:resetKickTarget()` or `mainPlayer:clearEvent()`) and
   invoked from a Lua script in response to some lifecycle event.
2. **Computed-index dispatch**: a sibling function loads the slot
   index from a runtime variable, then `CALL [EAX + ECX*4]`. Hard to
   trace statically; would surface via runtime tracing.

## Implications for SEQ_005 unblock

The garlemald-side hang (kick silently fails because
`context_root[+0x12c]` is stale from the prior cinematic — see
[[project_garlemald_seq005_8a_findings]]) has TWO possible root causes
now, sharpened:

1. **The clearer never fires on garlemald** — some packet pmeteor
   sends in the EndEvent → next-Kick window triggers
   `MyPlayer::vtable[66]`, and garlemald isn't sending it.
2. **The clearer fires on both, but on garlemald the timing is off**
   — clearer runs but the state has already been re-set by an in-flight
   kick.

Hypothesis 1 is the higher-probability path because:
- pmeteor's pre-kick salvo includes 5 extra `SetEventStatus` + extra
  `SetActorProperty` packets that garlemald doesn't send.
- The clearer's guard ("if both already NO_ACTOR, return") means it's
  perfectly safe to over-call. So whatever pmeteor does to trigger it
  is over-triggered in normal play, not a precise event.

**Concrete next-step experiments** (in order of cost):

| Cost | Experiment |
|---|---|
| Low | Walk MyPlayer slot 66's "natural callers" via runtime trace — set a hardware breakpoint on `FUN_006e32f0` in the Wine'd `ffxivgame.exe` and see what triggers it during the prior cinematic-end. |
| Low | Empirically: garlemald sends an extra `OUT 0x0131 EndEvent` for the OpeningDirector before the SEQ_005 kick (Experiment A in [[project_garlemald_seq005_8a_findings]]). If that ALONE doesn't reset `[+0x12c]`, try the 5 missing SetEventStatus packets. |
| Medium | Ghidra GUI decomp of all MyPlayer slot 66's parent classes (`PlayerBase`, `CharaBase`, `ActorBase`) to find any slot N that also points to `FUN_006e32f0` — would tell us if multiple inheritance paths reach the clearer. |
| High | Walk Lua scripts under `client/script/` for the binding name (`resetKickTarget` / `clearEvent` / similar) — would tell us the Lua-side trigger. Requires decoding the `.le.lpb` corpus (already started — [[reference_meteor_decomp_lpb_format]]). |

## The `[+0x128]/[+0x12c]` state machine — full picture

Combining Phase 7's KickReceiver decomp with this finding:

| Mutator | Where | Effect |
|---|---|---|
| `KickReceiver::Receive` Branch B1 | `FUN_0089e450` @ `0x49e4ff` | Sets `[+0x12c] = receiver[+0xc]` (the incoming kick's target id) |
| **`MyPlayer::vtable[66]`** | `FUN_006e32f0` (this doc) | Resets BOTH `[+0x128]` and `[+0x12c]` to NO_ACTOR |
| (no other clearer in the binary) | — | — |

So the dispatcher target-state lifecycle is:
- Initial: both NO_ACTOR
- Kick fires Branch B1 (primary kick, `receiver[+0x80] != 0`) → `[+0x12c]` set to kick-target
- Subsequent kicks hit Branch A (target is already set), gate on `[+0x12c]`-target's `+0x5c` flag
- At end-of-event, the clearer must run to reset state → next kick can re-enter Branch B1

If the clearer doesn't run between two events, the second event's
kick sees Branch A with the stale `[+0x12c]` value → ActorRegistry
lookup of the stale id returns NULL → kick silently fails. Exactly
the garlemald SEQ_005 hang shape.

## Cross-references

- `docs/event_kick_receiver_decomp.md` — Phase 7 #1 (the slot 2
  state machine — where `[+0x128]`/`[+0x12c]` are READ)
- `docs/event_end_receiver_decomp.md` — Phase 7 #3 (EndEvent slot 3
  102-case dispatcher; the clearer might be reached via one of the
  active cases but no static evidence yet — the 6 invoke thunks
  (FUN_006e1080..0e0) and 2 cleanup thunks (0x4a0640/0660) don't
  call FUN_006e32f0)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 (Lua-actor class
  hierarchy — MyPlayer's RTTI at `0x012c19a4`, inherits PlayerBase →
  CharaBase → ActorBase)
- `docs/receiver_dispatch_via_actorimpl.md` — Phase 9 #5 (the parallel
  problem: how opcode-bound Receivers dispatch via LuaActorImpl slots;
  same "no static virtual call" pattern)
- `memory/project_garlemald_seq005_8a_findings.md` — the active
  SEQ_005 hang context this resolves part of
- `memory/reference_meteor_decomp_lpb_format.md` — the `.le.lpb` Lua
  bytecode format that the Lua-side caller of slot 66 (if any) lives in
