# Phase 7 — `EndClientOrderEventReceiver` decomp

> First written 2026-05-04 — third receiver in the cinematic
> dispatch chain, completing the trio with KickEvent and
> RunEventFunction. Compact doc because the receiver is mostly
> structural rather than complex; the bulk of the interest is the
> per-receiver pattern confirmation it provides.

## TL;DR

`EndClientOrderEventReceiver` is the **client-side handler for the
end-of-cinematic packet**. Unlike its siblings (`KickClient` and
`StartServerOrder`), it does NO actor-readiness gating — it
unconditionally dispatches via slot 3 even if the actor lookup
returns NULL. This is by design: end-event has to work even when
the actor has been removed in the meantime (otherwise event
state could never be cleaned up for despawned actors).

Slot 2 (the network entry) is a **15-byte no-op success-writer**
— writes `0x01` to the out-result byte and returns. The actual
end-event work happens via slot 3 (`FUN_0089e2d0`, 73 B), which
calls `ActorRegistry_lookup_actor` then dispatches to a
**102-case jump-table dispatcher** at `FUN_008a13a0` based on a
single event-type byte.

## Vtable map (5 slots)

The 5-slot pattern matches `KickClient` and `StartServerOrder`
exactly — confirms this is a **shared template-style class
shape** across the entire `*EventReceiver` family.

| Slot | rva (meteor-decomp) | absolute (Ghidra) | Size | Role |
|---|---|---|---|---|
| 0 | `0x004a1100` | `0x008a1100` | (small) | Scalar deleting destructor |
| 1 | `0x0049d180` | `0x0089d180` | (small) | `New()` factory |
| 2 | `0x0049d200` | `0x0089d200` | **15 B** | **`Receive()` — no-op success writer** |
| 3 | `0x0049e2d0` | `0x0089e2d0` | 73 B | **Auxiliary dispatch — actor lookup + 102-case event-type dispatcher** |
| 4 | `0x0049d210` | `0x0089d210` | 15 B | Predicate — `[ECX+8] != NO_ACTOR` (identical to KickEvent slot 4) |

## Slot 2 (Receive) — full disassembly

```asm
0x0089d200:
    MOV EAX, [ESP+4]                 ; arg0 = out result byte ptr
    MOV CL, [0x012c41af]              ; load SUCCESS byte (0x01) — same
                                      ;  CommandUpdaterBase RTTI string
                                      ;  trick used by KickReceiver
    MOV [EAX], CL                     ; *out_result = SUCCESS
    RET 8
```

That's the entire Receive method. **No actor lookup, no flag
check, no state mutation.** The packet is acknowledged
unconditionally as success.

## Slot 3 (Auxiliary dispatch) — annotated

```asm
0x0089e2d0:
    SUB ESP, 0x1c                     ; allocate stack staging
    PUSH ESI
    MOV ESI, ECX                      ; this = receiver
    MOV ECX, [ESP+0x24]               ; arg = registry / context
    LEA EAX, [ESI+8]                  ; arg = &receiver[+8] (actor id ptr)
    PUSH EAX
    CALL ActorRegistry_lookup_actor   ; resolve actor (NO null-check after!)
                                      ; EAX = actor pointer (or NULL)

    LEA EDX, [ESI+0x10]               ; receiver field at +0x10
    LEA ECX, [ESI+0xd]                ; receiver field at +0xd
    MOV [ESP+0x8],  EDX               ; stage struct: many fields...
    MOV [ESP+0x14], EDX
    LEA EDX, [ESP+0x4]                ; pointer to staging area
    ADD ESI, 0xc                      ; receiver += 0xc (advance)
    PUSH EDX                          ; push staging area
    MOV [ESP+0x8],  EAX               ; ...actor ptr (may be NULL!)
    MOV [ESP+0x10], ECX               ; ...receiver[+0xd] address
    MOV [ESP+0x14], EAX               ; ...actor ptr again
    MOV [ESP+0x1c], ECX               ; ...receiver[+0xd] address again
    MOV [ESP+0x20], ESI               ; ...receiver+0xc

    CALL FUN_008a13a0                 ; the actual end-event-type dispatcher
    POP ESI
    ADD ESP, 0x1c
    RET 4
```

**Critical observation**: there is NO null-check on the
`ActorRegistry_lookup_actor` return value before the staging
struct is built and `FUN_008a13a0` is called. If the actor
lookup returns NULL, the dispatcher receives a NULL actor
pointer and proceeds anyway. This means **EndEvent can clean
up state for an actor that no longer exists in the registry** —
a deliberate design choice for the cleanup-side packet.

The receiver fields accessed:
- `+0x8` — actor id (passed to lookup)
- `+0xc` — unknown (passed via staging at +0x20; possibly the event-type byte the dispatcher reads as `[ECX]`)
- `+0xd` — small field passed twice
- `+0x10` — pointer/struct passed twice

## Slot 4 (Predicate) — full disassembly

```asm
0x0089d210:
    MOV ECX, [ECX+8]                  ; load receiver[+8] (the actor id)
    XOR EAX, EAX
    CMP ECX, [0x0130c778]             ; compare to NO_ACTOR sentinel
    SETNZ AL                          ; return (actor_id != NO_ACTOR)
    RET
```

**Identical structure to KickEvent's slot 4** — same `[ECX+8] !=
NO_ACTOR` predicate using the same sentinel global. This confirms
slot 4 is a **shared template-instantiated predicate** across all
`*EventReceiver` classes (it's just "is this kick/end/etc.
targeting a real actor id?").

## FUN_008a13a0 — the 102-case event-type dispatcher

The actual end-event work happens here. The function starts with
a jump-table dispatch:

```asm
MOVSX EAX, byte [ECX]                 ; event_type = staging[0] (byte)
CMP EAX, 0x65                          ; if > 0x65 (= 101)
JA <default_case>                      ; → default
MOVZX EAX, byte [EAX + 0x8a149c]       ; case_index = byte_table[event_type]
JMP [EAX*4 + 0x8a1464]                 ; handler = case_table[case_index]
```

So end-events are a **discriminated union** with up to 102 event
types (0..101). Each type has a small case body (mostly tail-jump
to a shared processor at `0x008a09e0`, a few CALL into
`0x008a10c0`). The case bodies generally:

```asm
MOV EAX, [ESP+4]                      ; load staging
MOV ECX, [EAX+0x18]                   ; load per-type method ptr from receiver
MOV [ESP+4], EAX
JMP 0x008a09e0                        ; tail-jump to shared processor
```

So each event type maps to a per-type method pointer at
`receiver[+0x18 + offset]` (different cases load different fields)
and a generic processor consumes (event-type, method-ptr,
staging) tuples.

## Implications for garlemald

**EndEvent is NOT a hang risk** for the man0g0 cinematic. Even
if the post-warp actor list is empty when the EndEvent packet
arrives, the receiver:
1. Dispatches via slot 3 → calls `ActorRegistry_lookup_actor`
   (returns NULL, but no gate)
2. Stages the NULL actor pointer + receiver fields
3. Calls `FUN_008a13a0` which dispatches by event-type byte
4. The per-type handler may or may not crash on NULL actor —
   needs case-by-case inspection — but the receiver itself doesn't
   silently drop

This rules out "EndEvent silent drop" as a tonight's-hang root
cause. The hang is at the **KICK** stage (`+0x5c` gate) and
possibly cascades to **RunEventFunction** (`+0x7d` gate). EndEvent
will eventually land regardless.

**Garlemald porting note for the EndEvent TX builder:** if
garlemald emits EndEvent packets, the wire format includes the
event-type byte at `staging[0]` (= `receiver[+0xc]`). This is a
discriminated-union opcode; up to 102 type variants. The exact
mapping (what each byte value means) needs the per-case decomp
of the 102 handlers in `FUN_008a13a0`'s jump table — substantial
work, deferred.

## Architectural insight — the consistent receiver shape

All three `*EventReceiver` classes share an identical 5-slot vtable
template, with predictable slot semantics:

| Slot | Role | KickEvent | RunEventFunction | EndEvent |
|---|---|---|---|---|
| 0 | Destructor | trivial | trivial | trivial |
| 1 | `New()` factory | 125 B | small | small |
| 2 | `Receive()` (network entry) | **207 B (3-way state machine + gates)** | **28 B trampoline → 344 B inner handler** | **15 B no-op success-writer** |
| 3 | Auxiliary dispatch | 48 B | 48 B (small) | 73 B (actor lookup + 102-case dispatcher) |
| 4 | `[ECX+8] != NO_ACTOR` predicate | 15 B (identical) | 15 B (likely identical) | 15 B (identical) |

The complexity gradient at slot 2 is informative:
- **KickEvent** has the most logic at slot 2 (managing kick state, both gates)
- **RunEventFunction** delegates to a separate inner handler
- **EndEvent** does almost nothing at slot 2; the work is in slot 3

This suggests slot 2 is the **synchronous network-receive entry**
that returns a result byte, while slot 3 is the **deferred /
async dispatch entry** that does the heavy lifting. KickEvent's
slot 2 is heavy because the kick gate itself needs to be checked
synchronously to return an accurate result byte. EndEvent's slot
2 returns success unconditionally because the cleanup work in
slot 3 doesn't affect the response.

## Open follow-ups

1. **~~Decompile `FUN_008a09e0`~~ — ✅ DONE 2026-05-04** (with a
   workaround — see "tooling note" below). It's a **6-case
   sub-dispatcher** on a method-descriptor's type byte, NOT a
   monolithic processor. ~44 bytes including the trailing
   jump table:

   ```c
   void FUN_008a09e0(MethodDescriptor *desc) {
       uint8_t type = desc[0];          // type byte at offset 0
       if (type > 5) return;            // default: bail
       switch (type) {
         case 0: /* JMP 0x006e1080 — 1-arg signature variant */
         case 1: /* JMP 0x006e10a0 — 1-arg, different layout */
         case 2: /* JMP 0x006e10c0 — 1-arg, third layout */
         case 3: /* PUSH staging[4] + staging[8]; CALL 0x006e10e0 — multi-arg */
         case 4: /* (not in visible bytes) */
         case 5: /* (not in visible bytes) */
       }
   }
   ```

   **Architectural insight — compile-time RPC dispatch.** This is
   a polymorphic method-binding pattern, not a simple switch. The
   receiver stores pointers to **method-descriptor objects** at
   `receiver[+0x18 + N*offset]`. Each descriptor has a leading
   type byte (0..5) that encodes its calling convention / field
   layout, and FUN_008a09e0 is the generic dispatcher that decodes
   the layout and forwards to the right per-signature thunk in the
   `0x006e10xx` range.

   This is classic MSVC `std::function` / `member-pointer-bind`
   machinery — the parent end-event dispatcher (FUN_008a13a0, 102
   cases) selects WHICH method to call based on the event-type
   byte; this sub-dispatcher then selects HOW to call it based on
   the method's signature class (6 variants).

   So the "102 end-event types" are really:
   - 102 distinct event-type slots in the receiver
   - Each slot points to a method descriptor (one of 6 signature shapes)
   - Two-level dispatch: (event type → method ptr) → (method type → calling thunk)

   Much smaller and more efficient than the 102-case appearance
   suggests — the actual calling code paths are bounded by 6
   signature thunks, not 102 unique handlers.

   **Tooling note — meteor-decomp asm-tree gap.** FUN_008a09e0
   is a real function in the binary but isn't enumerated in
   `asm/ffxivgame/`. Recovered via direct PE-header parse of
   `orig/ffxivgame.exe`. This is an analysis-pass gap —
   indirect-jump-only targets (reached only via case-table JMPs
   without any direct CALL) aren't auto-promoted to function
   heads by the static analyzer. There are almost certainly more
   such functions (the case table at 0x008a149c has 100+ entries
   reached only via indirect jump from FUN_008a13a0). Worth
   flagging as a tooling improvement opportunity.
2. **~~Decompile `FUN_008a10c0`~~ — ✅ DONE 2026-05-04.** Even
   simpler than FUN_008a09e0; ~33 bytes of real code + 6-entry
   jump table:

   ```c
   void FUN_008a10c0(StagingStruct *staging /* in ECX */) {
       void *adjusted_this = staging + 0xc;
       MethodDescriptor *desc = staging[+0x18];
       uint8_t type = desc[0];
       if (type > 5) return;
       switch (type) {
         case 0: return;                  // EXPLICIT no-op
         case 1: return;                  // EXPLICIT no-op
         case 2: JMP 0x004a0640;          // tail-call cleanup
         case 3: JMP 0x004a0660;          // tail-call cleanup
         case 4: return;                  // EXPLICIT no-op
         case 5: return;                  // EXPLICIT no-op
       }
   }
   ```

   Jump table at `0x008a10e4` confirms — 4 of 6 entries point to
   the same `RET` instruction at `0x008a10e1`; only cases 2 and 3
   reach actual handlers.

   **Architectural insight — two-axis end-event taxonomy.**
   Combined with FUN_008a09e0, the end-event dispatcher has TWO
   orthogonal axes:

   | Sub-dispatcher | Active | No-op | Phase |
   |---|---|---|---|
   | `FUN_008a09e0` | 6 of 6 | 0 | **Invoke** — handles all 6 method-signature classes |
   | `FUN_008a10c0` | 2 of 6 | 4 | **Post-invoke cleanup** — only for stateful method-classes (2 + 3) |

   So:
   - `FUN_008a09e0` = "invoke" sub-dispatcher (per-signature calling thunks)
   - `FUN_008a10c0` = "cleanup" sub-dispatcher (only types 2 + 3 are stateful)

   This is consistent with classic generic-dispatch patterns
   where some method classes are stateful (need cleanup) and
   others are stateless (don't). The compiler/runtime emits both
   dispatchers from a template; each specializes in a different
   lifecycle phase.

   For garlemald porting, this confirms the wire format is
   structurally simple: per-event-type, the receiver carries a
   method descriptor whose first byte is one of 6 well-defined
   values, and the cleanup behavior is determined entirely by
   that byte.

   **Updated end-event dispatch architecture summary:**

   ```
   FUN_008a13a0 (102-case event-type dispatcher)
     ↓ key: event_type byte at staging[0]
     ↓ each case loads receiver[+0x18+offset] (= MethodDescriptor*)

   FUN_008a09e0 (6-case INVOKE sub-dispatcher)
     ↓ for stateless event types: tail-jump → call thunk → done
     ↓ for stateful event types: tail-jump → call thunk → fall through

   FUN_008a10c0 (6-case CLEANUP sub-dispatcher)
     ↓ for stateful event types (case 2 + 3): tail-jump to
       cleanup thunk at 0x004a0640 / 0x004a0660
     ↓ for stateless event types: explicit RET (no-op)
   ```

   So the 102 "end-event types" really resolve to:
   - 102 distinct event-type slots in the receiver
   - Each slot points to a method descriptor (one of 6 signature
     shapes; only shapes 2 & 3 need post-invoke cleanup)
   - Three-level dispatch: (event type → method ptr) → (method
     type → invoke thunk) → optional (method type → cleanup thunk)
3. **~~Map out the 102 event-type cases~~ — ✅ DONE 2026-05-15.**
   Decoded `FUN_008a13a0`'s byte table at VA `0x008a149c` (102
   entries) and dword table at VA `0x008a1464` (14 entries).

   The "102 cases" really only ACT on 12 event types — the other
   90 fall through to the `RET 0x4` no-op at slot 13. Layout:

   ```
   byte_table[0x65+1] at VA 0x008a149c — event_type byte → slot index:
     event 0..5     → slots 0..5 (6 distinct invoke trampolines, identical body)
     event 6..49    → slot 13   (no-op default, 44 entries)
     event 50..55   → slots 6..11 (6 distinct cleanup trampolines, identical body)
     event 56..74   → slot 13   (no-op default, 19 entries)
     event 75..80   → slot 12   (no-op via 0x008a145f)
     event 81..99   → slot 13   (no-op default, 19 entries)
     event 100..101 → slot 12   (no-op via 0x008a145f)

   dword_table[14] at VA 0x008a1464 — slot index → case body VA:
     slot 0..5  → `MOV ECX, [EAX+0x18]; JMP FUN_008a09e0`  (invoke chain)
     slot 6..11 → `MOV ECX, [ESP+4]; CALL FUN_008a10c0; RET 0x4`  (cleanup chain)
     slot 12, 13 → `RET 0x4`  (no-op)
   ```

   So the dispatcher really has TWO active event-type bands:

   | Event types | Action | Path |
   |---|---|---|
   | 0..5 | invoke | tail-jump → FUN_008a09e0 → per-method-class thunk |
   | 50..55 | cleanup | call → FUN_008a10c0 → tail-jump (only types 2 + 3 are stateful) |
   | 6..49, 56..99 | no-op | RET |
   | 75..80, 100..101 | no-op | RET (different `RET 0x4` instance) |

   **Cross-reference with garlemald + Project Meteor scripts** —
   the 6 "invoke" event types map directly to the FFXIV 1.x quest
   event hooks. From
   `project-meteor-server/Map Server/bin/Debug/scripts/base/chara/npc/populace/PopulaceStandard.lua`'s
   `eventType ==` switch:

   | event_type | quest hook | garlemald trigger |
   |---|---|---|
   | 0 | `OnCommand` (default fallthrough) | `"commandRequest"` (journal/menu opens) |
   | 1 | `quest:OnTalk(...)` | `"talkDefault"` (NPC click → dialogue) |
   | 2 | `quest:OnPush(...)` | `"pushDefault"` (proximity push trigger) |
   | 3 | `quest:OnEmote(...)` | `"emoteDefault"` (player emote at NPC) |
   | 4 | (no PopulaceStandard handler) | unmapped — likely a Director-only signal |
   | 5 | `quest:OnNotice(...)` | `"noticeDefault"` (notice trigger) |

   The garlemald "trigger" string sent in `KickEventPacket` (0x012F)
   is what the client uses to FIND the script function to call,
   while `event_type` (the byte at offset +0x08 of `EndEventPacket`
   0x0131) tells the END-event receiver which lifecycle phase
   (invoke vs cleanup) to run. They're orthogonal: the trigger
   string says "what kind of event" and the type byte says "what
   phase".

   The 50..55 cleanup band is the post-invoke phase for events
   that need it. Per `FUN_008a10c0`'s sub-dispatcher, only
   method-classes 2 and 3 actually do cleanup work (the others
   no-op at the cleanup phase). Plausibly: events that allocate
   server-tracked resources during invoke (queued packets,
   waitable predicates) need a cleanup hook to release them.
   Stateless events (most) skip cleanup entirely.

   For garlemald the headline finding: the `event_type` byte is
   load-bearing for cleanup correctness on stateful events
   (types 2 + 3 in the 50..55 cleanup band). Garlemald
   currently always sends `event_type=0` (per
   `event/dispatcher.rs`), which is the OnCommand invoke path.
   For correct cleanup on push / emote / notice events, garlemald
   needs to send `event_type=2 or 3` in the corresponding
   `EndEventPacket` so the cleanup dispatcher fires. Surfaced
   by this analysis as a likely gap.

   Decoder script (re-runnable to refresh):

   ```python
   import json, struct
   pe = json.loads(open('build/pe-layout/ffxivgame.json').read())
   text = next(s for s in pe['sections'] if s['name'] == '.text')
   orig = open('orig/ffxivgame.exe', 'rb').read()
   image_base = int(pe['image_base'], 16)
   def at_va(va, n):
       file_off = text['raw_pointer'] + ((va - image_base) - text['virtual_address'])
       return orig[file_off:file_off + n]
   byte_table = at_va(0x008a149c, 102)
   n_entries  = max(byte_table) + 1
   dword_table = at_va(0x008a1464, n_entries * 4)
   for slot in range(n_entries):
       addr = struct.unpack_from('<I', dword_table, slot*4)[0]
       print(f'slot[{slot:2d}] → VA 0x{addr:08x}')
   ```

## Cross-references

- `docs/event_kick_receiver_decomp.md` — KickEvent receiver
  (slot-2 heavy) — the 5-slot pattern was first identified here
- `docs/event_run_event_function_receiver_decomp.md` —
  RunEventFunction receiver (slot-2 trampoline) — the inner
  handler was the most complex single piece of cinematic
  dispatch we've decoded
