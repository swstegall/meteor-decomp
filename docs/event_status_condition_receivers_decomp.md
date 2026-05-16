# Phase 9 #8b — `SetEventStatusReceiver` + `SetNoticeEventConditionReceiver` decomp

> Last updated: 2026-05-15. Slot 1 (`Receive`) of both 2-slot receivers
> in the SEQ_005 cinematic-body opcode set. Together with Phase 7's
> KickReceiver decomp and Phase 9 #8a's KickReceiver instance-layout
> map, this closes the gate audit for every receiver garlemald
> currently emits in the post-warp cinematic path.

## TL;DR

Both receivers' `Receive` body is the same MSVC-RTTI two-step:

```c
TargetType* derived = dynamic_cast<TargetType*>(dispatch_ctx);
derived->doIt(...);    // (SetEventStatus — unguarded, always runs)
// OR (SetNoticeEventCondition):
if (derived) derived->doRich(...);   // (path A: rich registration on DirectorBase)
else         dispatch_ctx->doBasic(...); // (path B: fallback on the original ActorBase)
```

`dispatch_ctx` is the receiver's first arg — the per-packet target
**actor object** (looked up by the IpcChannel dispatcher before invoking
Receive). The cast source type is shared across **all 32+ Network
namespace receivers**:

- **Source type** (`G1 = 0x1270964`, RTTI Type Descriptor in `.data`):
  `Application::Lua::Script::Client::Control::ActorBase` —
  the **Lua-side actor base class**. Same namespace as Phase 6's
  `DirectorBase` (cf. `docs/director_base_hooks.md`).

- **`SetEventStatusReceiver` target type** (`G2 = 0x12709e4`):
  `Application::Lua::Script::Client::Control::NpcBase`. Cast result is
  the NPC actor whose event-status table will be updated.

- **`SetNoticeEventConditionReceiver` target type** (`G2 = 0x12bf9c8`):
  `Application::Lua::Script::Client::Control::DirectorBase`. Cast
  result is the director whose notice-event registry will get a new
  entry.

**Neither receiver has a `+0x5c`-style actor flag gate** (unlike
KickReceiver — Phase 7). The gate is **implicit in the dynamic_cast**:
the target actor must already be of the right derived type when the
packet arrives. For SetNoticeEventCondition this is *soft* — the
fallback path runs unconditionally — so a "wrong type" target produces
**silent misplacement of the registration** (it lands in
`ActorBase[+0x118]` instead of `DirectorBase[+0x60]`), not a silent
drop.

### Implications for the SEQ_005 hang

These receivers are **NOT** the silent-drop suspect in the KickReceiver
sense (gated on a `+0x5c` flag that toggles via the spawn sequence).
They always do *something*. But:

- **SetNoticeEventConditionReceiver's fallback path is the only
  remaining "silent divergence" risk**: if the post-warp director
  hasn't been "promoted" to `DirectorBase` (i.e. its Lua-side class is
  still raw `ActorBase`) at the moment the packet lands, the notice
  conditions go into `ActorBase[+0x118]`, where the cinematic's
  notice-event evaluator may not find them. Garlemald's spawn sequence
  has `SetNoticeEventCondition` at step 2 — BEFORE `ScriptBind`
  (`ActorInstantiate`) at step 8. If `ScriptBind` is what allocates
  the `DirectorBase`-derived Lua object on the C++ actor, then for the
  three full ticks between step 2 and step 8, the cast would fail.
  This is the next thing to verify with a Ghidra GUI pass on
  `StartServerOrderEventFunctionReceiver` (which handles `ScriptBind`).

- **SetEventStatusReceiver is unguarded by null-check** — but garlemald
  emits its three `SetEventStatus` calls at step 10, AFTER
  `ScriptBind`. So by then the cast should succeed (assuming
  `ScriptBind` promotes the actor's Lua class). If the cast still
  fails, the unguarded `CALL EAX->vtable[9]` would either crash or
  no-op (depending on the discriminator byte at `receiver[+0x58]`
  — see below).

The audit's prior "third suspect mechanism" (some receiver state init
missing) is **partially eliminated**: SetEventStatus and
SetNoticeEventCondition don't need state priming. But the dispatch
context's *derived type* matters and is the new vector for silent
failure. KickReceiver's `+0x80` flag remains the prime suspect.

## SetEventStatusReceiver — `FUN_0089d860` (RVA `0x0049d860`, 58 bytes)

### Vtable map (2 slots)

| Slot | rva | absolute | Size | Role |
|---|---|---|---|---|
| 0 | (dtor) | (dtor) | — | Scalar deleting destructor |
| 1 | `0x0049d860` | `0x0089d860` | **58 B** | **`Receive()` — entry from the IpcChannel dispatcher** |

### Decomp

```c
char *SetEventStatusReceiver::Receive(int dispatch_ctx, char *out_result) {
  NpcBase *npc = (NpcBase *)__RTDynamicCast(
      (ActorBase *)dispatch_ctx,    // [EBP+8]   inptr
      0,                             // [EBP+0xC] vfDelta
      &TypeDesc_ActorBase,           // [EBP+0x10] SrcType   = 0x1270964
      &TypeDesc_NpcBase,             // [EBP+0x14] TargetType = 0x12709e4
      0                              // [EBP+0x18] isReference (no throw)
  );

  // No null check — caller guarantees the actor is an NpcBase.
  // If npc is NULL here, the next call dereferences a junk this (0x00),
  // which only crashes if the discriminator byte at receiver[+0x58]
  // matches one of CONST_A/B/C (see FUN_006e67c0 below).
  NpcBase::setEventStatus(
      npc,                           // ECX = NpcBase *
      dispatch_ctx,                  // arg1 (lookup context, passthrough)
      &this->event_name,             // arg2 = &this[+0x04] — Utf8String (event name)
      &this->event_kind,             // arg3 = &this[+0x58] — Utf8String (status kind: 1st byte selects sub-vector)
      &this->new_value               // arg4 = &this[+0x59] — byte (new status value: enable/disable)
  );

  return out_result;
}
```

### Downstream — `NpcBase::setEventStatus` (`FUN_006e67c0`, 113 bytes)

```c
void NpcBase::setEventStatus(NpcBase *this, void *ctx,
                             Utf8String *event_name,
                             Utf8String *kind_key,
                             byte *new_value)
{
  byte kind = ((byte *)kind_key)[0];   // first byte of the kind Utf8String
  EventTable *table;
  if (kind == CONST_A) table = &this->table_a; // this[+0xe8]
  else if (kind == CONST_B) table = &this->table_b; // this[+0xf8]
  else if (kind == CONST_C) table = &this->table_c; // this[+0x108]
  else return;                          // ← silent no-op for unknown kind

  EventEntry *entry = table->lookup_by_name(kind_key);  // FUN_0071ca50
  if (entry == NULL) return;            // ← silent no-op if name not registered

  entry->vtable[9](entry, new_value, this, event_name);
}
```

- `FUN_0071ca50` is a **linear-scan over `std::vector<EventEntry*>`**
  with 4-byte element stride; per-element comparison is
  `Utf8String::operator==` (`FUN_00445d20`) against `key->[+0x4]`
  (where the entry stores its name).
- Three sub-tables: `this[+0xe8]`, `this[+0xf8]`, `this[+0x108]` —
  best guess "notice / talk / push" status tables given the broader
  context.
- The unknown bytes at `[0x012c3f7a]` / `[0x012c3f7b]` / `[0x012c3f7c]`
  are three contiguous 1-byte discriminator constants in `.rdata`
  (TBD — likely ASCII chars).

### Gates summary

1. `dynamic_cast<NpcBase>(dispatch_ctx)` — must succeed or downstream
   may crash. Implicit: target actor must be NpcBase-derived.
2. `kind_key->[0]` must match `CONST_A/B/C` — else silent no-op.
3. Sub-table lookup must find an entry by name — else silent no-op.

Gates 2 and 3 are **per-packet semantic** — both depend on the
packet's own contents being well-formed against a script-registered
event-name. Not a "spawn ordering" issue like KickReceiver.

## SetNoticeEventConditionReceiver — `FUN_0089d980` (RVA `0x0049d980`, 83 bytes)

### Vtable map (2 slots)

| Slot | rva | absolute | Size | Role |
|---|---|---|---|---|
| 0 | (dtor) | (dtor) | — | Scalar deleting destructor |
| 1 | `0x0049d980` | `0x0089d980` | **83 B** | **`Receive()` — entry from the IpcChannel dispatcher** |

### Decomp

```c
char *SetNoticeEventConditionReceiver::Receive(int dispatch_ctx, char *out_result) {
  DirectorBase *director = (DirectorBase *)__RTDynamicCast(
      (ActorBase *)dispatch_ctx,    // [EBP+0x10] inptr (note +0x10 here, not +0x8 — fn uses different prolog)
      0,                             // vfDelta
      &TypeDesc_ActorBase,           // SrcType = 0x1270964
      &TypeDesc_DirectorBase,        // TargetType = 0x12bf9c8
      0                              // isReference (no throw)
  );

  if (director != NULL) {
    // Path A: target IS a director — store in DirectorBase[+0x60]
    DirectorBase::addNoticeCondition(
        director,                    // ECX = DirectorBase *
        &this->event_name,           // arg1 = &this[+0x04]
        &this->cond_data,            // arg2 = &this[+0x58]
        &this->cond_flag             // arg3 = &this[+0x59]
    );
  } else {
    // Path B: target is NOT a director — store in ActorBase[+0x118]
    ActorBase::addNoticeConditionFallback(
        (ActorBase *)dispatch_ctx,   // ECX = the ORIGINAL ActorBase
        &this->event_name,           // arg1 = &this[+0x04]
        &this->cond_data,            // arg2 = &this[+0x58]
        &this->cond_flag             // arg3 = &this[+0x59]
    );
  }

  return out_result;
}
```

### Downstream — both add-condition methods (208/211 bytes each)

`FUN_006f1380` (path A → DirectorBase) and `FUN_006f2e80` (path B →
ActorBase fallback) are **near-identical**. Difference is the
container they push the new entry into:

- Path A stores into `this[+0x60]` (DirectorBase's notice-condition
  vector).
- Path B stores into `this[+0x118]` (ActorBase's generic-condition
  vector).

Both bodies:

```c
void addNoticeCondition(T *this, Utf8String *event_name,
                        Utf8String *cond_data, byte *cond_flag)
{
  ConditionEntry *entry;
  byte kind = *((byte *)cond_data);          // first byte = condition kind
  if (kind == CONST_X /* [0x0134c3fe] */) {
    // "rich" condition (0x60 bytes, 2 ctor args)
    entry = (ConditionEntry *)operator new(0x60);   // FUN_009d1b35
    if (entry != NULL) {
      ConditionEntry::ctor_rich(entry, event_name, cond_data);  // FUN_00892770
    }
  } else {
    // "basic" condition (0x5c bytes, 1 ctor arg)
    entry = (ConditionEntry *)operator new(0x5c);
    if (entry != NULL) {
      ConditionEntry::ctor_basic(entry, event_name);            // FUN_008927f0
    }
  }
  if (entry != NULL) {
    // push_back into the receiver's condition container
    ConditionVector::push_back(&this->conditions, entry);       // FUN_00725ed0
  }
}
```

The 0x60/0x5c sizes correspond to two ConditionEntry subclasses (one
with 2 storage slots, one with 1). The `[0x0134c3fe]` constant
discriminates between them.

### Gates summary

1. `dynamic_cast<DirectorBase>(dispatch_ctx)` — **soft gate**. If it
   fails, the fallback path runs on the original ActorBase. **Silent
   divergence**: registration lands in `ActorBase[+0x118]` instead of
   `DirectorBase[+0x60]`. Whether anything still works depends on
   whether the cinematic's notice-event evaluator looks in both fields
   or only one.
2. `operator new` — silent no-op on OOM (extremely rare).
3. Per-packet kind discriminator (`cond_data[0]`) only selects ctor
   size; never causes a no-op.

The fallback divergence is the **interesting one for SEQ_005 debugging**.

## Why this matters for SEQ_005

The receiver-class system is **statically typed against the Lua actor
class hierarchy**:

```
Application::Lua::Script::Client::Control::ActorBase   (base, ~32 receivers)
├── NpcBase                                              (used by SetEventStatus, mounts, etc.)
├── DirectorBase                                         (used by SetNoticeEventCondition)
├── (other *Base classes — TBD)
```

When a packet like `SetEventStatus` arrives, the IpcChannel dispatcher
looks up the target actor by id, **fetches its Lua actor object**, and
passes that to the receiver as `dispatch_ctx`. The receiver casts to
its own expected type and runs.

For SEQ_005's flow:

1. **Step 1 — `AddActor`** creates the actor in the registry. **What
   Lua-side class** is allocated at this point? If it's just bare
   `ActorBase`, then steps 2+ that need a derived type will fall back
   or crash.
2. **Step 2 — `SetNoticeEventCondition x3`** (targets the director).
   If the actor isn't yet `DirectorBase`, fallback path B runs.
   Registrations land in `ActorBase[+0x118]`.
3. **Step 8 — `ActorInstantiate` (ScriptBind)** is when the actor's
   Lua-side derived class is presumably instantiated (per Phase 7
   `+0x7d` finding — `StartServerOrderEventFunctionReceiver` is the
   handler).
4. **Step 10 — `SetEventStatus x3`** runs AFTER ScriptBind, so the
   cast to NpcBase should succeed.

**Open question**: does the post-`ScriptBind` `DirectorBase` instance
inherit the conditions from `ActorBase[+0x118]` that were registered
pre-`ScriptBind`? Or are they orphaned in `ActorBase[+0x118]` while
`DirectorBase[+0x60]` is empty?

If orphaned, that's a real bug — the cinematic's notice-condition
evaluator wouldn't find them, and the kick that would normally fire
based on a satisfied notice condition would never trigger.

**Next investigation**: trace `StartServerOrderEventFunctionReceiver`'s
slot 2 to see if it migrates `ActorBase[+0x118]` into the newly
constructed `DirectorBase[+0x60]`. (Phase 9 #8c if not already
identified.)

## The shared `__RTDynamicCast` helper (`FUN_009da6cc`)

Confirmed via the surrounding RTTI helpers in the asm tree:
`FindSITargetTypeInstance` (`FUN_009da3e2`), `FindMITargetTypeInstance`
(`FUN_009da476`), `FindVITargetTypeInstance` (`FUN_009da578`),
`PMDtoOffset` (`FUN_009da457`), `__RTtypeid` (`FUN_009da34f`). The
function signature is the standard MSVC pattern:

```c
PVOID __RTDynamicCast(
    PVOID inptr,        // [EBP+0x08]
    LONG  vfDelta,      // [EBP+0x0C]
    PVOID SrcType,      // [EBP+0x10] — &TypeDescriptor
    PVOID TargetType,   // [EBP+0x14] — &TypeDescriptor
    BOOL  isReference   // [EBP+0x18] — non-zero throws bad_cast
);
```

Early null-return guard: if `inptr == NULL`, returns NULL immediately
without throwing (regardless of `isReference`). All callers in the
receiver inventory pass `isReference = 0`, so failed casts return
NULL.

## RTTI Type Descriptors discovered

| RTTI addr | TypeInfo vtable | Mangled name | Demangled |
|---|---|---|---|
| `0x01270964` | `0x01085d0c` | `.?AVActorBase@Control@Client@Script@Lua@Application@@` | `Application::Lua::Script::Client::Control::ActorBase` |
| `0x012709e4` | `0x01085d0c` | `.?AVNpcBase@Control@Client@Script@Lua@Application@@` | `Application::Lua::Script::Client::Control::NpcBase` |
| `0x012bf9c8` | `0x01085d0c` | `.?AVDirectorBase@Control@Client@Script@Lua@Application@@` | `Application::Lua::Script::Client::Control::DirectorBase` |

The vtable pointer is shared (`0x01085d0c`) — that's the global
`type_info` vftable used by all MSVC RTTI Type Descriptors in this
binary.

The `.data` section spans `[0xe65000, 0xf7c940]` raw and these
descriptors all sit within it. There are **45 other receiver Receive
bodies** that all push `0x1270964` (G1=`ActorBase`) — direct evidence
this is the common source-type for the entire Network namespace
receiver fan-out. The per-receiver target type G2 varies — the next
Phase 9 sub-task (#8b-extension) could enumerate all 45 G2 globals
and resolve them to subclass names in a single sweep, producing a
"receiver → target subclass" map that complements the existing
inventory.

## Cross-references

- `docs/event_kick_receiver_decomp.md` — Phase 7 KickReceiver decomp
  (the `+0x5c` actor gate finding; addressing convention)
- `docs/event_run_event_function_receiver_decomp.md` — Phase 7
  StartServerOrderEventFunctionReceiver (the `+0x7d` gate; presumed
  `ScriptBind` handler)
- `docs/event_end_receiver_decomp.md` — Phase 7 EndClientOrderEvent
- `docs/kick_receiver_offset_map.md` — Phase 9 #8a KickReceiver
  instance layout (the `receiver[+0x80]` flag mystery)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 inventory of all
  43 receivers
- `docs/seq005_receiver_gate_audit.md` — Phase 9 #8 SEQ_005-specific
  cross-reference (the audit that prioritized this decomp)
- `docs/director_base_hooks.md` — Phase 6 #8 DirectorBase 34-slot
  vtable (the engine-side `DirectorBase`, where the Lua-side
  `DirectorBase` decomp'd here ultimately delegates)
- `garlemald-server/docs/post_warp_respawn_fix_analysis.md` — the
  garlemald-side application of these findings
