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

## Sibling-method decomp results — 2026-05-04

Closed the 3-method follow-up. Each turned out to be a small
wrapper around an inner body, surfacing **two more architectural
findings** beyond just labelling the methods.

### FUN_00cc7180 (7-byte navigation thunk)

```asm
MOV ECX, [ECX + 0x1c8]         ; navigate: this = (*this)[+0x1c8]
JMP FUN_00cd80e0               ; tail-call (whose body is itself
                               ;  a thunk to FUN_00d132b0)
```

The inner FUN_00cd80e0 (11 bytes) IS the body — but it's another
thunk: `MOV ECX, [ECX+0x1c8]; JMP FUN_00d132b0`. So the chain is
two thunks deep before reaching the real predicate at
`FUN_00d132b0`.

**Architectural finding #1 — sibling classifier sub-objects.**
This thunk is **structurally parallel** to the
`id_partition_predicate_thunk` (FUN_00cd80f0) we already found:

| Thunk | Nav offset | Tail-call target | Role (inferred) |
|---|---|---|---|
| `FUN_00cd80f0` | `[+0x1c4]` | `FUN_00d035d0` | **Type-tag predicate** (collection A vs B; tag == `0x0F`) |
| `FUN_00cd80e0` | `[+0x1c8]` | `FUN_00d132b0` | **Sibling predicate** (used by FUN_00cc7180; semantic TBD) |

The registry has **multiple classifier sub-objects at adjacent
offsets** — staged classifier pattern. Each classifier (+0x1c4,
+0x1c8, possibly more) is its own object with its own predicate +
lookup table. The parent registry navigates to the right one
based on which lookup method was called.

### FUN_00cc78c0 (26-byte lookup wrapper)

```asm
MOV EAX, [ESP+0x4]             ; arg0 = lookup key
MOV ECX, [ECX]                 ; this = *this
PUSH 0                         ; push 0 (probably "create if missing"?)
PUSH EAX                       ; push key
CALL FUN_00cdde20              ; call the heavyweight lookup body
TEST EAX, EAX
JNZ +3
RET 4                          ; null → return 0
MOV EAX, [EAX]                 ; deref entry → actor pointer
RET 4
```

The inner FUN_00cdde20 is **1187 bytes** and operates on the
`[+0x1c8]` classifier (the SAME one FUN_00cc7180's thunk
navigates to). The leading `PUSH 0` is the second arg, probably a
"create if missing" or "lazy resolution" boolean flag.

**Why this exists:** RunEventFunction's Phase 1 calls this AFTER
both `lookup_actor` and `FUN_00cc7180`'s predicate fail — meaning
this is the **fallback "find or register placeholder"** path that
keeps the queue making progress when the actor isn't yet known.

### FUN_00cc72a0 (18-byte flag-read wrapper)

```asm
MOV EAX, [ESP+0x4]             ; arg0 = actor pointer
MOV ECX, [ECX]                 ; this = *this
PUSH EAX
CALL FUN_00cd7a30              ; alias resolver
MOV AL, [EAX + 0x7d]           ; READ byte at +0x7d of result
RET 4
```

The inner FUN_00cd7a30 (29 bytes) is an **alias resolver**:

```c
void *FUN_00cd7a30(this, EDX /* actor_ptr */) {
  if (EDX == NULL) return NULL;
  EAX = this[+0x1bc];                       // registry's "focus" obj
  if (EDX == *EAX) return EAX;              // if actor matches focus
                                            //  → return focus obj
  return EDX[+0x4];                         // else return actor[+0x4]
                                            //  (an aliased pointer
                                            //  stored inside the actor)
}
```

So FUN_00cc72a0 = "look up the canonical / aliased object for
this actor, then return the byte flag at offset `+0x7d`".

**Architectural finding #2 — second actor flag at `+0x7d`.**
This is a NEW actor flag distinct from the `+0x5c` we found via
the kick receiver. Both must be set for a full kick → run-event
chain to land:

| Actor flag | Set by | Read by | Inferred semantic |
|---|---|---|---|
| `+0x5c` | spawn-side opcode (TBD) | `KickReceiver` Branch A & B2 | **"Ready for event reception"** (kick gate) |
| `+0x7d` | TBD | `RunEventFunction` Phase 3 dispatcher (via FUN_00cc72a0) | **"Ready for event dispatch"** |

The fact that they're at different offsets — and read at
different stages of the dispatch chain — strongly suggests they
represent different lifecycle states:

- `+0x5c` is set early (probably when `AddActor` is processed)
  and gates whether the actor can RECEIVE event packets at all.
- `+0x7d` is set later (probably when the actor's event-handling
  subsystem is initialized — e.g. after the actor's vtable
  bindings are registered) and gates whether the event body can
  actually RUN against it.

For garlemald's post-warp re-spawn fix: re-broadcasting `AddActor`
will set `+0x5c`. Whether that's enough to also restore `+0x7d`
depends on whether `+0x7d` is set in the same opcode pipeline OR
in a follow-up packet (e.g. the actor's per-instance Lua-binding
setup). If garlemald's re-broadcast only re-fires `AddActor` and
not the binding-setup packets, RunEventFunction may STILL drop
silently even after the kick gate is cleared.

**Updated registry-method roster (the `0x00cc7` cluster):**

| RVA | Confirmed role | Body shape |
|---|---|---|
| `0x00cc70b0` | (xref-only — likely add/remove sibling) | TBD |
| `0x00cc7180` | Sibling predicate using `[+0x1c8]` classifier | Thunk → FUN_00cd80e0 → FUN_00d132b0 |
| `0x00cc7190` | (xref-only — likely add/remove sibling) | TBD |
| `0x00cc72a0` | Read actor's `+0x7d` "event dispatch ready" flag | Wrapper around FUN_00cd7a30 alias resolver |
| `0x00cc78c0` | Heavyweight "find or placeholder" lookup on `[+0x1c8]` classifier | Wrapper around 1187-byte FUN_00cdde20 |
| `0x00cc7a50` | `ActorRegistry::lookup_actor` (Actor* or NULL) | Wrapper that partitions by `[+0x1c4]` then dispatches to A or B |

## Open follow-ups (still deferrable)

1. **~~Decompile FUN_00d132b0~~ — ✅ DONE 2026-05-04.** It's a
   **circular-linked-list set-membership test** (not a tag check
   like FUN_00d035d0):

   ```c
   bool FUN_00d132b0(this, ulong *key_ptr) {
       void *root = this[+0x100];                // container root
       Node *cur = *root;                        // first node
       while (cur != root) {                     // until cycle back
           if (cur->key /* +0x8 */ == *key_ptr) return true;
           cur = cur->next /* +0x0 */;
       }
       return false;
   }
   ```

   **The two classifiers use DIFFERENT backing data structures** —
   not just different predicates on the same shape:

   | Classifier | Sub-obj offset | Predicate body | Backing structure | Returns |
   |---|---|---|---|---|
   | **Type-tag** | `[+0x1c4]` | `FUN_00d035d0` | hashmap w/ metadata bytes | `meta[0] == 0x0F` |
   | **Set membership** | `[+0x1c8]` | `FUN_00d132b0` | circular linked-list set, key at +0x8 | `key in set?` |

   Strong inference: `[+0x1c4]` is the **main actor registry**
   (hashmap-backed for O(1) lookup), and `[+0x1c8]` is a
   **pending / placeholder set** (linked-list for cheap
   insert/remove of transient queue entries).

   This re-interprets RunEventFunction Phase 1's three lookups:
     1. `lookup_actor` (uses `[+0x1c4]`) — find in main registry
     2. `FUN_00cc7180` predicate (uses `[+0x1c8]`) — check pending set
     3. `FUN_00cc78c0` (uses `[+0x1c8]` with create-flag) — register
        a placeholder in the pending set

   So an actor queued for spawn but not yet fully spawned can
   still receive RunEventFunction packets — the engine queues
   them against a placeholder until the real actor arrives.

   **Strengthened garlemald implication:** the post-warp re-spawn
   fix may need to ALSO re-create pending-set entries for
   "expected" actors, not just the main-registry entries via
   AddActor. Otherwise pending RunEventFunction items for actors
   not-yet-arrived will never resolve. (Mirroring pmeteor's
   `playerSession.UpdateInstance(aroundMe, true)` semantics
   should cover this — it re-broadcasts the FULL state of the
   area, not just the actor list.)

2. **Identify FUN_00cc70b0 + FUN_00cc7190** — these are referenced
   by the `id_partition_predicate_thunk`'s xref list but we
   haven't decompiled them. Likely add/remove siblings.
3. **Decompile FUN_008a1370** — the "post-loop completion check"
   called twice in the inner handler.
4. **Decompile FUN_0077a210** — the "shift/move" call in Phase 3's
   dequeue. Probably a vector::erase-style helper.
5. **Find what sets actor `+0x7d`** — same difficulty as the
   `+0x5c` writer hunt (Task C of the kick-receiver doc); deferred
   to the same class-hierarchy decomp pass.
