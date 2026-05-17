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

## 2026-05-17 Ghidra GUI session — corrections + Lua-binding identified

A fresh Ghidra walk corrected and extended the static-analysis findings:

**Corrections to earlier static work:**
- Vtable address was a typo: `0xbd785c` → **`0x00fd785c`** (`b` → `f`, single digit off). The xref to `FUN_006e32f0` is at `0x00fd7964`, not `0xbd7964`. Confirmed in Ghidra GUI: 0xfd785c carries the RTTI label `const Application::Lua::Script::Client::Control::MyPlayer` (slot 0 = `FUN_007493e0`, the scalar-deleting destructor). The doc's class identification was right; just the address was wrong.
- `FUN_006e32f0` body confirmed byte-for-byte as the clearer: loads `DAT_0130c778` (= `0xE0000000` = NO_ACTOR), guards `if [+0x128] != NO_ACTOR || [+0x12c] != NO_ACTOR`, calls `FUN_00cc7510` + `FUN_0075b510`, writes NO_ACTOR to both fields. ~76 bytes ✓.

**Lua-binding name identified:** slot 66 of MyPlayer's vtable corresponds to the Lua method **`_fadeInNowLoadingForNoticeEventJustInArea`**.

Recovery procedure (reusable for any vtable[N] → Lua-name lookup):
1. Search for the MSVC virtual-call thunk pattern: `8B 01 8B 80 NN NN 00 00 FF E0` where `NN NN` is the slot offset (N*4) in little-endian. For slot 66 (offset 0x108): `8B 01 8B 80 08 01 00 00 FF E0`.
2. Two hits found. First (real thunk): `0x0071e3f0` (labeled `LAB_0071e3f0`), xrefs from `FUN_007324f0:00732513` and `FUN_007324f0:0073252a`. Second (related): `0x00575140` — `MOV ECX,[ECX]; MOV EAX,[ECX]; MOV EAX,[EAX+0x108]; JMP EAX` — a sub-object wrapper called from `FUN_004d8860:004d8b86`.
3. The xref helper `FUN_007324f0` follows the same shape as `FUN_00731290` (the `_cancelNotice` helper): builds a method descriptor in `local_60`, names it via `FUN_00447260("_fadeInNowLoadingForNoticeEventJustInArea", 0xffffffff)`, registers it via `FUN_00cccad0(local_60, ...)`. The thunk `LAB_0071e3f0` is the function pointer that gets registered, so the Lua binding `_fadeInNowLoadingForNoticeEventJustInArea` dispatches to `MyPlayer::vtable[66]`.

**Production callers in the decoded Lua corpus:**
- `tp5rq/r75w9s1v/x9w/x9wj3j.lua` = man0g0 (Gridania) — 3 calls
- `tp5rq/r75w9s1v/x9w/x9wjyj.lua` = man0l0 (Limsa) — 3 calls
- `tp5rq/r75w9s1v/x9w/x9wjpj.lua` = man0u0 (Ul'dah) — 3 calls
- `tp5rq/tp5rq89r57y9rr_7vxxvw.lua` = QuestBaseClass common
- Various Director scripts (InstanceRaid, InstanceContent)

In man0g0 specifically, the 3 calls are inside:
- `processTtrNomal001withHQ` (line 385) — HQ variant of opening cinematic
- `processTtrNomal001` (line 616) — standard opening cinematic
- `processTtrAfterBtl001` (line 1097) — post-battle handler (after SEQ_005 wolf fight)

The call pattern is consistent: `setTutorialMask(...)` → `player:_fadeInNowLoadingForNoticeEventJustInArea()` → `startFadeOut(player, 0)`. The clearer fires INSIDE cinematic-prep code that the server triggers via `RunEventFunction("delegateEvent processTtrX...")`.

**Implication for SEQ_005 unblock:** the opening cinematic's `processTtrNomal001` calls the clearer, so state SHOULD be clean entering SEQ_005. That redirects suspicion AWAY from the "stale `[+0x12c]`" hypothesis and BACK to the **Branch B1 `receiver[+0x80] == 0` silent no-op** hypothesis (the original Phase 7 KickReceiver finding). If state is clean, Branch B1 fires — and if `receiver[+0x80]` is zero, the kick is dropped silently without storing for retry. Resolving that requires either runtime tracing of what populates `receiver[+0x80]` or a definitive packet-byte → instance-offset mapping for KickEventPacket.

Alternative paths if Branch B1 turns out to be a dead-end too:
- **Send a packet that causes the client to invoke `_fadeInNowLoadingForNoticeEventJustInArea` on its main player** — would force-clear the kick state from the server side. Need to find what server packet causes Lua dispatch of an arbitrary PlayerBaseClass method (probably a variant of `RunEventFunction` or a generic "call client method" opcode).
- Search for `_cancelNotice`-style direct sibling methods that might also clear `[+0x12c]` as a side effect.

## 2026-05-17 Ghidra GUI session — full Receive + gate-setter decoded

Deep walk through the KickReceiver state machine and `+0x80` provenance. Definitive findings:

### `FUN_0089e200` IS the gate-setter — and does more

```asm
0089e200 PUSH ECX; PUSH ESI; MOV ESI, ECX        ; this = ECX = LuaParamsContainer
0089e204 MOV byte ptr [ESI + 0x14], 0x1          ; ← SETS THE GATE BYTE TO 1
0089e208 MOV EAX, [ESI + 0x8]                    ; EAX = params data buffer ptr
0089e20b TEST EAX, EAX
0089e20e JNZ LAB_0089e214
0089e210 XOR EDI, EDI                             ; if null, size = 0
0089e212 JMP LAB_0089e219
LAB_0089e214:
0089e214 MOV EDI, [ESI + 0xc]                    ; EDI = buffer end ptr
0089e217 SUB EDI, EAX                             ; EDI = (end - start) = size
LAB_0089e219:
... bounds check ...
0089e229 MOV EDX, [ESI + 0x8]
0089e22c PUSH EDI (size)
0089e22d PUSH 0
0089e22f PUSH EDX (start)
0089e230 CALL _memset                             ; zero the buffer
0089e235 MOV EAX, [ESI + 0x8]
... bounds check ...
0089e255 POP EDI; MOV byte ptr [EAX], 0x1; POP ESI; POP ECX; RET
                                                  ; write 1 to first byte of buffer
```

So FUN_0089e200 does THREE things atomically:
1. `LuaParamsContainer[+0x14] = 1` (the gate byte = receiver[+0x80])
2. `memset(data_buffer, 0, size)` (clear the params buffer)
3. `*data_buffer = 1` (write 1 as a single-byte param)

Effectively: "Replace the params with `[byte(1)]` and set the gate flag."

### The constructor's noticeEvent-only call to the gate-setter

`FUN_0089f180` (parameterized KickReceiver constructor) has special-case code that runs ONLY when `event_type_byte == 0x05` (the noticeEvent type — SEQ_005's kick type):

```c
FUN_0078f810(local_14, p7, p8);  // init iterator with {buffer_ptr, count}
result = FUN_0078f840(local_14); // read next byte, return (byte == 0x03)
if (result != 0) {
    FUN_0089e200(this+0x6c);     // sets the gate
}
```

`FUN_0078f810` is a trivial 2-store function — initializes a `{ptr, count}` iterator pair. `FUN_0078f840` is a "consume one byte, return (byte == 0x03), advance pointer".

So **the gate is set iff (event_type == 5) AND (first byte of p7-buffer == 0x03)**.

### The actual Receive state machine (was partially wrong in older docs)

`FUN_0089e450` (slot 2 = Receive) decoded fully — THREE branches, not two:

```c
char * KickReceiver::Receive(this, *out_result) {
  *out_result = SUCCESS;
  context = engine_root->dispatcher;  // via FUN_00cc7510 chain
  
  if (context[+0x12c] != NO_ACTOR) {
    // ----- Branch A: previous target stored, retry path -----
    actor = ActorRegistry_lookup(this+0xc);   // lookup THIS kick's owner
    if (actor == NULL || actor[+0x5c] == 0 || FUN_006e11d0() != 0)
        *out_result = FAILURE;
    return;
  }
  
  if (context[+0x128] == NO_ACTOR) {
    // ----- Branch B1: BOTH clean, fresh dispatch -----
    if (this[+0x80] != 0) {
        context[+0x12c] = this[+0xc];    // store for retry
        *out_result = FAILURE;
    }
    // else: silent no-op
  }
  else {
    // ----- Branch B2: [+0x128] set, [+0x12c] clean — process queued -----
    FUN_0089e200();                      // this->LuaParamsContainer[+0x14] = 1 (re-arm)
    actor = ActorRegistry_lookup(context+0x128);
    if (actor == NULL || actor[+0x5c] == 0)
        *out_result = FAILURE;
  }
}
```

### Reframing the SEQ_005 hang root cause

The doc previously believed `+0x80 == 0` caused the silent no-op in Branch B1. With the byte-diff showing garlemald and pmeteor send identical bytes (including `03 0F 00 00` at packet body offset 0x30), **both servers' kicks SHOULD satisfy the constructor's gate-set condition** (event_type=5 AND first byte=0x03) → both should have `+0x80 = 1` → both Branch B1 calls should store `[+0x12c]` and queue for retry.

If that's true, the hang must be on the next-tick retry (Branch A), not Branch B1. Branch A fails on any of:
- `actor == NULL` (director not spawned client-side — but garlemald's spawn sequence is documented to be complete)
- `actor[+0x5c] == 0` (kick-enabled flag not set on director — possible if spawn sequence is subtly off)
- `FUN_006e11d0() != 0` (unknown global predicate — likely "is some transition / loading state in progress?")

Next investigation step: **decode `FUN_006e11d0`** — it's the new prime suspect. If it returns "is a zone-load in progress" or "is a fade transition in progress", then the loading screen itself is what's gating the kick retry, creating the deadlock.

### 2026-05-17 (continued) — the +0x1e byte's writer found, but no explicit clearer

Walked the chain `Branch A → FUN_006e11d0 → dispatcher->[+0xf8]->[+0x1e]`. Search results for writers to `[reg+0x1e]`:

- `0x00d7ce49`/`ceef`/`cf9b` (3 hits) — **red herring**: inside `FUN_00d7cae0`, a 600-line trail/particle ringbuffer renderer (SIMD math, 0x20-stride structs of "trail points" each with active flag at +0x1e).
- `0x00d90c9c` (1 hit) — **red herring**: inside `FUN_00d90bc0`, a particle emitter that initializes random particle params via FUN_00e3f2x0 RNGs; the +0x1e write is when no particle template exists.
- `0x0089313c` (1 hit) — **the relevant one**: inside `FUN_008930e0`, in the same 0x89xxxx code region as KickReceiver. **Sets `[+0x1e] = 1`.**

`FUN_008930e0` is called from `FUN_008955c0` at two call sites. `FUN_008955c0` is **the event dispatcher**:
- Walks a linked list of subscribers at `this->[+0x14]`
- For each subscriber, calls `FUN_008930e0` (sets `[+0x1e] = 1`), then dispatches via vtable
- Has explicit clears for `[+0x1c]` (line 00895671) and `[+0x1d]` (line 00895675) — **but NEVER clears `[+0x1e]`**

`FUN_008955c0`'s sole caller is `FUN_006e11b0` (just 0x20 bytes before `FUN_006e11d0` — they're sibling methods of the same dispatcher class). So we now have:

```
DispatcherClass (engine-side, ~0x6e1000 method region)
  ::FUN_006e11b0(...)      — dispatches event → calls FUN_008955c0
  ::FUN_006e11d0()         — returns this->[+0xf8]->[+0x1e]
                              (Branch A's "inhibitor" predicate)

FUN_008955c0(this) — the dispatcher's event-fan-out helper
  iterates this->[+0x14] subscriber list
  for each subscriber:
    FUN_008930e0(subscriber) — sets subscriber->[+0x1e] = 1
    invoke subscriber callback
```

So the byte at `dispatcher->[+0xf8]->[+0x1e]` represents **"the subscriber at +0xf8 is/has been in notification"**. Set to 1 by FUN_008930e0; **never explicitly cleared by a `MOV imm` we found**.

### Open question — what clears subscriber[+0x1e]?

Three hypotheses (in decreasing likelihood):

1. **Subscriber destructor zeroes the whole struct** (memset on dtor) — the byte is implicitly cleared when subscribers are freed/recreated. So the question reduces to: does garlemald keep subscribers alive longer than pmeteor does? Worth checking subscriber lifecycle around state transitions.

2. **`dispatcher->[+0xf8]` gets REPLACED** with a different subscriber pointer — the byte at the old location stays 1, but `[+0xf8]` now points to a different (potentially fresh) subscriber. So we'd need to find writers to `[reg+0xf8]` where reg is the dispatcher.

3. **A non-`MOV imm` write we missed** — could be `XOR reg, reg; MOV [...], reg` or a struct-copy via REP MOVSB. To find these, broaden the search beyond `C6 4? 1E` patterns.

### Bottom line for SEQ_005

We've narrowed the inhibitor to ONE byte (`dispatcher->[+0xf8]->[+0x1e]`) and its setter (`FUN_008930e0`) and the broader context (engine event-dispatcher class). We haven't conclusively found the clearer. The next step needs either:

- More Ghidra: find writers to `[reg+0xf8]` (replacement of the pointer) or subscriber destruction patterns
- OR runtime tracing: set a breakpoint on the byte's value in Wine's ffxivgame.exe during pmeteor's successful SEQ_005 cinematic transition, observe what the dispatcher state looks like at the critical moment

### Reusable findings + key addresses

| Address | Role |
|---|---|
| `0x010574b0` | KickClientOrderEventReceiver vftable (RTTI-labeled, full namespace) |
| `FUN_008a1b90` | slot 0 — scalar deleting destructor |
| `FUN_0089f530` | slot 1 — heap-clone wrapper |
| `FUN_0089e450` | slot 2 — Receive (Branch A/B1/B2 state machine — see above) |
| `FUN_0089d230` | slot 3 — auxiliary dispatch → FUN_006ee680 (DirectorBase::dispatchEvent) |
| `FUN_0089d260` | slot 4 — predicate `[ECX+8] != NO_ACTOR` |
| `FUN_0089e800` | regular destructor (writes vtable, destructs sub-objects in reverse) |
| `FUN_0089f180` | parameterized constructor (8 args, special-case noticeEvent path) |
| `FUN_0089f2b0` | heap-clone copy-ctor (called by slot 1 wrapper) |
| `FUN_0089e660` | LuaParamsContainer destructor (called by KickReceiver destructor on `[ESI+0x6c]`) |
| `FUN_0089ec30` | LuaParamsContainer parameterized constructor (called by KickReceiver ctor) |
| `FUN_0089e200` | **The gate-setter**: writes `[ECX+0x14] = 1`, clears + reinits params buffer |
| `FUN_0078f810` | `{ptr, count}` iterator initializer (8 call sites) |
| `FUN_0078f840` | iterator advance + return (byte == 0x03) predicate |
| `FUN_0076c0d0` | **The packet parser** — constructs stack KickReceiver via FUN_0089f180, dispatches, destructs via FUN_0089e800 |
| `FUN_006e11d0` | Branch A predicate — returns `dispatcher->[+0xf8]->[+0x1e]` byte |
| `FUN_006e11b0` | Sibling of FUN_006e11d0 (engine event-dispatcher class method); single caller of FUN_008955c0 |
| `FUN_008955c0` | Event-fan-out helper — walks subscriber list at `this->[+0x14]`, calls FUN_008930e0 + dispatches |
| `FUN_008930e0` | The subscriber `[+0x1e] = 1` setter (with sub-object install at `[+0x4]`) |
| `dispatcher->[+0xf8]->[+0x1e]` | **The inhibitor byte** Branch A checks; set by FUN_008930e0; no explicit clearer found |

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
