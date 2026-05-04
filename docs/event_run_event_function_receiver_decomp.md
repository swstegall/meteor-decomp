# Phase 7 — `StartServerOrderEventFunctionReceiver` decomp

> First written 2026-05-04 — companion to
> `event_kick_receiver_decomp.md`. Pulled into Phase 7 because
> `RunEventFunction` is the OTHER half of the cinematic-dispatch
> pair: KickEvent kicks off a director, and RunEventFunction runs
> the body of the noticeEvent / talk script INSIDE the director
> session. Tonight's man0g0 hang likely involves both packets.

## TL;DR

`StartServerOrderEventFunctionReceiver::Receive` (slot 2) is a
**28-byte trampoline** that advances `this` by `+0xCC` (delegates
to a sub-object inside the receiver) and tail-calls the inner
handler `FUN_0089e8e0` (344 bytes).

The inner handler iterates through a **vector of 8-byte items**
inside the receiver's sub-object (`this[+0x4]..this[+0x8]`,
stride 8). For each item it tries up to 3 different registry
lookups — `ActorRegistry_lookup_actor` plus two siblings we
haven't seen before — to find a target actor. If all three miss,
the function enters a phase-3 dequeue path that processes items
backwards and shrinks the array.

**The same kick-gate principle applies:** RunEventFunction packets
that target an actor not yet on the client (e.g. post-warp before
spawn re-broadcast) will fall through all 3 lookups and silently
no-op, just like KickEvent does.

## Vtable map (5 slots)

| Slot | rva (meteor-decomp) | absolute (Ghidra) | Size | Role |
|---|---|---|---|---|
| 0 | `0x004a1bb0` | `0x008a1bb0` | (small) | Scalar deleting destructor |
| 1 | `0x0049f430` | `0x0089f430` | (small) | `New()` factory |
| 2 | `0x0049eb20` | `0x0089eb20` | **28 B** | **`Receive()` — trampoline; advances `this+0xCC`, tail-calls `FUN_0089e8e0`** |
| 3 | `0x0049e260` | `0x0089e260` | (small) | Auxiliary |
| 4 | `0x0049e060` | `0x0089e060` | (small) | Auxiliary |

## Slot 2 (`Receive`) — annotated

```asm
0x0089eb20:
    MOV EAX, [ESP+0x8]              ; arg0 (out-result byte ptr?)
    PUSH ESI
    MOV ESI, [ESP+0x8]              ; arg1 (registry / context)
    PUSH EAX                         ; push arg0
    PUSH ESI                         ; push arg1
    ADD ECX, 0xCC                    ; this += 0xCC (advance to sub-object)
    CALL 0x0089e8e0                 ; the actual handler
    MOV EAX, ESI                     ; return arg1
    POP ESI
    RET 8
```

That's it for slot 2. The receiver's outer object holds some
header fields at `[0..0xCC]`; the actual processable state lives
at `+0xCC` and is what gets passed into the inner handler. This
is the canonical "delegate to inner" composition pattern — the
receiver is a thin wrapper around an inner state machine.

## Inner handler `FUN_0089e8e0` (rva `0x0049e8e0`, 344 B)

Processes a vector of pending event-actor items. Decomp shape:

```c
char *RunEventFunction_inner(SubObject *this,    // ECX = receiver+0xCC
                              ResultByte *out,   // arg0
                              ActorRegistry *reg /* or game ctx */) {
  // this[+0x4] = vector::_First (start)
  // this[+0x8] = vector::_Last  (end)
  // each item is 8 bytes
  
  uint8_t *first = this->_First;
  if (first == NULL) goto early_success;
  uint8_t *last = this->_Last;
  size_t count = (last - first) / 8;
  if (count == 0) {
early_success:
    *out = 0x01;                            // success default (from
                                            //  CommandUpdaterBase RTTI
                                            //  string + 0x3f, same as
                                            //  KickReceiver's pattern)
    return out;
  }
  
  // PHASE 1: forward iteration — try to satisfy each item via
  //          three different registry-lookup paths
  EBP = reg;
  for (uint8_t *cur = first; cur != last; cur += 8) {
    if (cur >= this->_Last) crash();        // bounds check
    
    // Try lookup A: ActorRegistry_lookup_actor (FUN_00cc7a50)
    Actor *a = ActorRegistry_lookup_actor(reg, cur);
    if (a != NULL) continue;                // satisfied → next item
    
    // Try lookup B: FUN_00cc7180 (some sibling lookup — likely
    //               "lookup by name" or "lookup in alternate
    //               namespace")
    if (FUN_00cc7180(reg, cur) != 0) continue;
    
    // Try lookup C: FUN_00cc78c0 (third sibling — possibly the
    //               "register placeholder" / "queue for later"
    //               path)
    FUN_00cc78c0(reg, cur);
    // (no early-continue; falls through)
  }
  
  // PHASE 2: post-loop completion check
  if (FUN_008a1370(this) != 0) goto done_success;
  
  // PHASE 3: backwards iteration / dequeue
  for (uint8_t *cur = this->_Last; this->_First <= cur; ) {
    cur -= 8;
    Actor *a = ActorRegistry_lookup_actor(reg, cur);
    if (a == NULL) goto done_success;       // can't find — give up
    
    // Try lookup D: FUN_00cc72a0 (fourth sibling — likely the
    //               actual "dispatch" call that runs the event
    //               function on the actor)
    if (FUN_00cc72a0(reg, a) == 0) goto done_success;
    
    // Dequeue: pop the item from the back of the vector
    if (this->_First != NULL) {
      uint8_t *end = this->_Last;
      size_t left = (end - this->_First) / 8;
      if (left != 0) {
        FUN_0077a210(end - 8, end, this, ebx);  // shift / move
        this->_Last -= 8;
      }
    }
    if (FUN_008a1370(this) != 0) break;
  }
  
done_success:
  if (this->_First == NULL || (this->_Last - this->_First)/8 == 0)
    goto early_success;                     // queue empty → success
  
  // queue still has items but we couldn't process them all → failure
  *out = 0x00;                               // FAILURE_BYTE (DAT_0134c560)
  return out;
}
```

## New findings — sibling registry methods

The `0x00cc7` cluster on the actor registry now has confirmed
**6 methods** (4 of them surfaced via this decomp):

| RVA | Confirmed role | Source |
|---|---|---|
| `0x00cc70b0` | (xref-only — likely add/remove sibling) | Inferred from `id_partition_predicate_thunk` xref list |
| `0x00cc7180` | Some lookup variant — returns bool | RunEventFunction Phase 1 attempt B |
| `0x00cc7190` | (xref-only — likely add/remove sibling) | Inferred from `id_partition_predicate_thunk` xref list |
| `0x00cc72a0` | Some dispatch call — returns bool | RunEventFunction Phase 3 attempt D |
| `0x00cc78c0` | Some "register / enqueue" call — returns void | RunEventFunction Phase 1 attempt C |
| `0x00cc7a50` | `ActorRegistry::lookup_actor` (Actor* or NULL) | KickReceiver decomp + this one |

The 3 lookup-style attempts in Phase 1 of RunEventFunction's inner
handler suggest the registry has **multiple lookup namespaces**
that are tried in sequence — likely:
1. Direct actor lookup (by id)
2. Alternate identifier lookup (by name? by stable id?)
3. Lazy-creation lookup ("if not found, queue for later
   resolution")

If lookup A returns the actor → use it (early continue).
If A misses and B succeeds → also continue.
If both miss → call C (probably enqueue / placeholder) and fall
through.

This is consistent with the "type-tag-based partition" finding
from the kick receiver decomp — the registry's API is rich enough
to serve different actor flavours (regular actors / directors /
synthetic / lazy) via different access paths.

## Implications for garlemald

Same diagnosis as for KickEvent: **RunEventFunction packets that
target an actor not yet spawned on the client will fall through
all 3 lookups and silently no-op**. The `*out = FAILURE_BYTE`
result isn't surfaced to the user — the caller can't distinguish
"event function ran successfully" from "actor wasn't found".

The garlemald porting fix is **the same**: in
`apply_do_zone_change_content`, after the zone-in bundle's
`DeleteAllActors` wipe, re-broadcast spawn packets for every
actor in the destination content area BEFORE any
RunEventFunction or KickEvent packets.

Beyond that, the **8-byte item stride** in the receiver's vector
is informative for the wire format: each pending event item is
8 bytes (probably `[u32 actor_id, u32 function_id]` or similar).
Worth confirming when garlemald implements the
RunEventFunction-side TX builder — if the wire format is a
list-of-pairs, garlemald needs to encode it consistently.

## Cross-references

- `docs/event_kick_receiver_decomp.md` — the companion KickEvent
  receiver decomp; `+0x5c` actor flag finding is shared.
- `garlemald-server/map-server/src/runtime/quest/run_event_function.rs`
  (or wherever the TX builder lives) — porting target for the
  wire-format encoding side.

## Open follow-ups

1. **Identify FUN_00cc7180, FUN_00cc78c0, FUN_00cc72a0** — quick
   3-function decomp pass would close out the registry API
   surface. Each is probably small (<50 bytes) since they're
   sibling lookups.
2. **Decompile FUN_008a1370** — the "post-loop completion check"
   called twice in the inner handler. If it's a "queue exhausted?"
   predicate, our Phase 1/2/3 decomp interpretation is solid.
3. **Decompile FUN_0077a210** — the "shift/move" call in Phase 3's
   dequeue. Probably a vector::erase-style helper. Useful for
   understanding the receiver's memory model but not blocking.
