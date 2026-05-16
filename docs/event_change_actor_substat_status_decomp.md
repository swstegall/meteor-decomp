# Phase 9 #2 — `ChangeActorSubStatStatusReceiver` decomp

> Last updated: 2026-05-16. Decomp of the last 5-slot receiver — the
> only one in the System namespace, completing the 5-slot receiver
> coverage (Phase 7 closed the other three: KickClientOrderEvent,
> StartServerOrderEventFunction, EndClientOrderEvent).

## TL;DR

`ChangeActorSubStatStatusReceiver` is the **most thoroughly-gated
receiver in the entire inventory**. Unlike the other 5-slot receivers
which check ONE actor flag, this one checks **two flags on two
different actors**:

1. **Primary actor**'s `+0x7d` (event-dispatch-ready) — same gate as
   `StartServerOrderEventFunctionReceiver`
2. **Secondary actor**'s `+0x5c` (kick-gate) — same gate as
   `KickClientOrderEventReceiver`

Plus a **per-instance done-flag** at `receiver[+0x15]` that
short-circuits to success if already processed (so subsequent
re-dispatches don't re-run the gates).

It uses the **3-path registry lookup chain** (`lookup_actor` →
`FUN_00cc7180` predicate → `FUN_00cc78c0` find-or-queue) — same as
`StartServerOrderEventFunctionReceiver` Phase 1.

**Two `__RTDynamicCast` calls** reveal the receiver's semantic
purpose:
- Primary cast: `LuaControl → StatusBase` (the status-effect instance)
- Secondary cast: `LuaControl → CharaBase` (the character bearing it)

So the receiver applies / updates a **status-effect sub-state on a
character**: "tell character C that status S has sub-state X".

### New RTTI types recovered

This decomp **discovered two new RTTI Type Descriptor addresses**
beyond the Network-namespace ones from Phase 9 #8b:

| RTTI addr | Class | Notes |
|---|---|---|
| `0x01270b4c` | `Component::Lua::GameEngine::LuaControl` | **System-namespace SrcType** — distinct from Network's `0x01270964 ActorBase` |
| `0x012c31f8` | `Application::Lua::Script::Client::Control::StatusBase` | **NEW subclass** — extends LuaControl; represents an active status effect |
| `0x012709a4` | `Application::Lua::Script::Client::Control::CharaBase` | Already known from #8d hierarchy; here its RTTI Type Descriptor address is concretely recovered |

**Key new finding**: System-namespace receivers cast from
`Component::Lua::GameEngine::LuaControl`, NOT from
`Application::Lua::Script::Client::Control::ActorBase`. This is a
**deeper engine-level base class** that BOTH ActorBase AND StatusBase
extend.

Updated inheritance picture:

```
Component::Lua::GameEngine::LuaControl                  (System-ns SrcType)
└── Application::Lua::Script::Client::Control::ActorBase  (Network-ns SrcType; per Phase 9 #8b)
    ├── CharaBase, DirectorBase, ...                      (Phase 9 #8d/8b hierarchy)
    │
└── Application::Lua::Script::Client::Control::StatusBase (per this decomp — sibling of ActorBase under LuaControl)
```

So `StatusBase` is **NOT** a subclass of `ActorBase` — it's a SIBLING
under the deeper `LuaControl` base. Both can be reached via
`__RTDynamicCast` from a `LuaControl*`.

## Vtable map (5 slots) — RTTI `0xc575c8`

| Slot | rva | abs | Size | Role |
|---|---|---|---|---|
| 0 | `0x004a4f40` | `0x008a4f40` | 30 B | Scalar deleting destructor (MSVC pattern: dtor body + optional `operator delete`) |
| 1 | `0x004a34d0` | `0x008a34d0` | 120 B | **`New()` factory** — allocates 0x18 bytes via `operator new`, calls ctor at `0x008a4cf0` with 5 args reading from `[ESI+0x8/0xc/0x10/0x14/0x15]` |
| 2 | `0x004a3550` | `0x008a3550` | **219 B** | **`Receive()` — the most thoroughly gated entry in the inventory. See below.** |
| 3 | `0x004a3630` | `0x008a3630` | 97 B | **Dispatch entry** — runs after slot 2's gate passes; calls a method on the secondary CharaBase with the cached StatusBase as an arg |
| 4 | `0x004a36a0` | `0x008a36a0` | 13 B | Predicate — `bool isSecondaryActorSet() { return this->[+0x8] != NO_ACTOR; }` |

## Slot 2 (`Receive`) — full decomp

```c
char *Receive(this, char *out_result_byte, void *lookup_ctx) {
  // CHECK 1: per-instance done-flag — already-processed receivers short-circuit
  if (this->done_flag /* [+0x15] */ != 0) {
    *out_result_byte = SUCCESS_BYTE;   // from [0x012c41af] (same SUCCESS_BYTE constant
                                       //  as KickReceiver — clever offset into the
                                       //  CommandUpdaterBase RTTI string)
    return out_result_byte;
  }

  // CHECK 2: resolve primary actor (id at [+0xc]); may be cached at [+0x10]
  if (this->cached_primary /* [+0x10] */ == NULL) {
    // Same 3-path lookup chain as StartServerOrderEventFunctionReceiver Phase 1
    Actor *primary = ActorRegistry::lookup_actor(lookup_ctx, &this->[+0xc]);
    if (primary == NULL) {
      // path A miss — try path B (predicate)
      if (FUN_00cc7180(lookup_ctx, &this->[+0xc]) != 0) {
        // path B hit → SUCCESS_SET_FLAG
        goto success_set_flag;
      }
      // path A + B miss — try path C (find-or-queue placeholder)
      primary = FUN_00cc78c0(lookup_ctx, &this->[+0xc]);
      if (primary == NULL) goto success_set_flag;
    }
    // Cast primary to StatusBase
    StatusBase *status = __RTDynamicCast(
        primary, 0,
        &TypeDesc_LuaControl,           // 0x1270b4c — SrcType
        &TypeDesc_StatusBase,           // 0x12c31f8 — TargetType
        0                                // isReference (no throw)
    );
    this->[+0x10] = status;              // cache for next call
  }

  // CHECK 3: primary actor's +0x7d (event-dispatch-ready) — via alias resolver
  // FUN_00cc72a0 = wrapper that calls FUN_00cd7a30 (alias resolver) and returns byte [alias+0x7d]
  byte ready = FUN_00cc72a0(lookup_ctx, this->cached_primary /* [+0x10] */);
  if (ready == 0) {
    *out_result_byte = FAILURE_BYTE;     // from [0x0134c560]
    return out_result_byte;
  }

  // CHECK 4: lookup secondary actor (id at [+0x8])
  Actor *secondary = ActorRegistry::lookup_actor(lookup_ctx, &this->[+0x8]);
  if (secondary == NULL) goto success_set_flag;  // secondary not in registry → skip the gate

  // Cast secondary to CharaBase
  CharaBase *chara = __RTDynamicCast(
      secondary, 0,
      &TypeDesc_LuaControl,             // 0x1270b4c — SrcType
      &TypeDesc_CharaBase,              // 0x12709a4 — TargetType
      0
  );

  // CHECK 5: secondary actor's +0x5c (kick-gate)
  if (chara->[+0x5c] == 0) {
    *out_result_byte = FAILURE_BYTE;
    return out_result_byte;
  }

success_set_flag:
  this->done_flag /* [+0x15] */ = 1;     // mark this instance processed
  *out_result_byte = SUCCESS_BYTE;
  return out_result_byte;
}
```

## Slot 3 (dispatch) — runs after slot 2 gate passes

```c
void slot3_dispatch(this, void *lookup_ctx) {
  // Re-lookup the secondary actor (slot 2 didn't cache it)
  Actor *secondary = ActorRegistry::lookup_actor(lookup_ctx, &this->[+0x8]);
  if (secondary != NULL) {
    CharaBase *chara = __RTDynamicCast(secondary, 0, &TypeDesc_LuaControl, &TypeDesc_CharaBase, 0);
    // Call chara's status-update method with the cached StatusBase
    chara->updateSubStatus(
        lookup_ctx,                       // arg1
        (byte)this->[+0x14],              // arg2 = sub-stat status enum
        this->cached_primary /* [+0x10] */, // arg3 = StatusBase ptr (cached by slot 2)
        1                                  // arg4 = some "apply" flag
    );  // FUN_007084b0 — slot 2 of CharaBase? or non-virtual setter
  } else {
    // No secondary — check if there's a "global" target at [+0xc]
    if (this->[+0xc] == NO_ACTOR /* 0x0130c778 */) {
      return;  // no target → no-op
    }
    // Use [+0xc] as the lookup key via FUN_00cc7170 (yet ANOTHER registry method —
    // sibling of FUN_00cc7180; not in the Phase 7 cluster roster)
    FUN_00cc7170(lookup_ctx, &this->[+0xc]);
  }
}
```

**FUN_00cc7170 is a NEW registry method** not in Phase 7's roster.
The `0x00cc7` cluster now has **7 known methods**:

| RVA | Phase | Role |
|---|---|---|
| `0x00cc70b0` | xref-only | Likely add/remove sibling |
| `0x00cc7170` | **#2 (this doc)** | **NEW — some "fallback dispatch" call** |
| `0x00cc7180` | Phase 7 | Sibling predicate using `[+0x1c8]` classifier |
| `0x00cc7190` | xref-only | Likely add/remove sibling |
| `0x00cc72a0` | Phase 7 | Read actor's `+0x7d` flag (alias resolver wrapper) |
| `0x00cc78c0` | Phase 7 | Heavyweight "find or queue placeholder" |
| `0x00cc7a50` | Phase 7 | `ActorRegistry::lookup_actor` |

## Slot 1 (New() factory) — instance layout

The factory allocates a **24-byte instance** (`operator new(0x18)`)
and calls the ctor at `FUN_008a4cf0` with 5 args. The args are read
from the receiver template's stack memory at offsets `[+0x8]`,
`[+0xc]`, `[+0x10]`, `[+0x14]`, `[+0x15]` — matching the slot 2
Receive body's field accesses. So the instance layout is:

```c
struct ChangeActorSubStatStatusReceiver {  // 0x18 bytes
  /* +0x00 */ void**  vtable;              // = 0x10575c8 (RVA 0xc575c8)
  /* +0x04 */ uint32_t parent_field;        // base-class field (likely the LuaControl owner ptr)
  /* +0x08 */ uint32_t secondary_actor_id;  // (from packet)
  /* +0x0c */ uint32_t primary_actor_id;    // (from packet — the StatusBase id)
  /* +0x10 */ StatusBase *cached_primary;   // initially NULL; populated by slot 2
  /* +0x14 */ uint8_t  sub_stat_enum;        // (from packet) — which sub-stat to update
  /* +0x15 */ uint8_t  done_flag;            // 0 initially; set to 1 after first successful Receive
  /* +0x16 */ uint8_t  pad[2];               // alignment
};
```

The done_flag pattern is a **performance optimization** — once a
status update has been processed, the receiver instance can be
re-dispatched (e.g. by a retry loop) without re-running the gates.
The instance persists across calls.

## Implications for the receiver gate model

This receiver **combines** the gate types we've seen in isolation:

| Gate | Where checked elsewhere | In this receiver |
|---|---|---|
| `[+0x7d]` (event-dispatch-ready) | `StartServerOrderEventFunctionReceiver` Phase 3 | CHECK 3 — on the primary StatusBase |
| `[+0x5c]` (kick-gate) | `KickClientOrderEventReceiver` Branch A/B2 | CHECK 5 — on the secondary CharaBase |
| Per-instance done-flag `[+0x15]` | (none — new pattern) | CHECK 1 — short-circuit success |
| 3-path registry lookup | `StartServerOrderEventFunctionReceiver` Phase 1 | CHECK 2 — for the primary id |

**For garlemald porting / debugging**: if any status-effect update
packet seems to "silently no-op" client-side, the most likely cause
is one of these gates not being satisfied. Specifically, since this
receiver requires BOTH actors' flags set, it's the most failure-prone
receiver in the inventory.

But: in the SEQ_005 cinematic flow, this receiver isn't on the hot
path. Status effects aren't part of the man0g0 tutorial fight setup.
So this finding doesn't directly unblock SEQ_005 — it's groundwork
for status-effect packet debugging.

## The `0x012c41af` "SUCCESS_BYTE" constant — third receiver to use it

ChangeActorSubStatStatusReceiver reads its SUCCESS byte from `[0x012c41af]`,
matching:
- `KickClientOrderEventReceiver` (Phase 7)
- `StartServerOrderEventFunctionReceiver` (Phase 7)

Phase 7 noted this was "loaded from a clever offset into the
CommandUpdaterBase RTTI string at `0x012c41af` = string + 0x3f". So
ALL three Network/System 5-slot receivers reuse this trick — load
the result byte from a fixed RTTI-string-offset rather than emitting
a `MOV [out], 0x01` literal. Curious optimization choice.

## Cross-references

- `docs/event_kick_receiver_decomp.md` — Phase 7 KickReceiver (the
  `+0x5c` gate; source of the 3-path lookup pattern)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7
  StartServerOrderEventFunctionReceiver (the `+0x7d` gate; the
  3-path lookup; FUN_00cc72a0 alias resolver)
- `docs/event_end_receiver_decomp.md` — Phase 7 EndClientOrderEvent
- `docs/event_status_condition_receivers_decomp.md` — Phase 9 #8b
  (SetEventStatus + SetNoticeEventCondition — also use
  __RTDynamicCast but with different SrcType `ActorBase` vs this
  receiver's `LuaControl`)
- `docs/lua_actor_class_construction.md` — Phase 9 #8d
  (ActorBase/CharaBase ctor walk; CharaBase RTTI inferred — here
  concretely recovered as `0x12709a4`)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 inventory
- `docs/lua_class_registry.md` — Phase 6 #3 (Lua class registry —
  may have entries for `StatusBase` and `LuaControl` that this
  decomp now ties to their C++ vtable RTTIs)
