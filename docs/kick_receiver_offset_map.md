# Phase 9 #8a — KickReceiver instance offset map ✅ RESOLVED

> Resolved 2026-05-17. Maps the KickClientOrderEventReceiver
> instance fields to packet body bytes. Phase 9 #8a's core question
> — "what does `receiver[+0x80]` correspond to in the packet?" — is
> now answered via static decomp of the parser `FUN_0089f180`.

## TL;DR (2026-05-17 update)

**`receiver[+0x80]` is set to 1 IFF the KickEvent's `event_type`
byte == `0x05` ("noticeEvent" type).** Specifically:

1. The kick packet parser `FUN_0089f180` reads the event_type
   byte from an arg pointer (caller pre-extracted it from packet
   bytes), stores it at `this[+0x68]`.
2. Just before returning, the parser compares `this[+0x68]`
   against the byte at global `[0x012c3f7e] = 0x05`.
3. **If equal**: parser calls `FUN_0089e200(LuaParamsContainer*)`
   which sets `LuaParamsContainer[+0x14] = 1` (≡ `receiver[+0x80] = 1`).
4. **If not equal**: `receiver[+0x80]` stays `0` → KickReceiver's
   Branch B1 silently falls through (no-op).

The byte table at `0x012c3f7a..7e` is a small enum: `{0x01, 0x02,
0x03, 0x04, 0x05}` indexed by event type. `0x05` is the
"noticeEvent" tag — the cinematic-dispatch event kind.

## Summary

The receiver instance is **132 bytes** (allocated via `operator new(0x84)`
in slot 1). The instance layout, recovered from the copy-constructor
`FUN_0089f2b0` and the parser `FUN_0089f180`:

```
struct KickClientOrderEventReceiver {  // 0x84 bytes
  /* +0x00 */ void**   vtable;                  // = 0x10574b0 (RVA 0xc574b0)
  /* +0x04 */ uint32_t parent_field;            // 4 bytes from base ctor (FUN_007942b0)
  /* +0x08 */ uint32_t src_actor_id;            // == packet body [0..3] (trigger_actor_id)
  /* +0x0c */ uint32_t owner_actor_id;          // == packet body [4..7]
  /* +0x10 */ uint32_t event_type_word;         // == packet body [8..11] (event_type+magic+u16)
  /* +0x14 */ Sqex::Misc::Utf8String event_name; // 0x54 bytes (str body from packet [16..47])
  /* +0x68 */ uint8_t  event_type_byte;         // == packet body [8] (event_type, byte form)
  /* +0x6c */ LuaParamsContainer params;        // 0x18 bytes (sub-object)
  /* +0x84 */                                   // end
};

struct LuaParamsContainer {  // 0x18 bytes (at receiver +0x6c)
  /* +0x00 */ void**   vtable;
  /* +0x04 */ void*    param_buffer_begin;       // pointer to first LuaParam byte
  /* +0x08 */ void*    param_buffer_end;          // (= begin + length)
  /* +0x0c */ void*    param_buffer_cap;          // (capacity)
  /* +0x10 */ uint32_t spare;                     // (some 4-byte spare/flags)
  /* +0x14 */ uint8_t  is_notice_flag;            // ← receiver[+0x80]; set to 1 iff event_type==0x05
  /* +0x15 */ uint8_t  pad[3];                    // (alignment)
};
```

So **`receiver[+0x80]` = `(LuaParamsContainer at +0x6c)[+0x14]`** =
the `is_notice_flag` byte, set by **`FUN_0089e200`** when the parser
detects `event_type == 0x05`.

## The decisive code path (FUN_0089f180 — kick packet parser)

```asm
; (after copying packet bytes into receiver fields +0x08..+0x10)
0049f207: MOV CL, [EDI]                  ; CL = event_type byte (from caller arg3)
0049f209: MOV [ESI+0x68], CL              ; receiver.event_type_byte = CL

; (LuaParamsContainer ctor at +0x6c)
0049f225-2d: CALL 0x0089ec30              ; new LuaParamsContainer(...)

; The decisive check:
0049f237: MOV AL, [ESI+0x68]              ; reload event_type_byte
0049f23a: CMP AL, byte ptr [0x012c3f7e]   ; compare against tag table[4] = 0x05
0049f240: JNZ skip                        ; ← BRANCH: if not 5, skip the +0x80 set

; (intermediate setup — LuaParams handling)
0049f242-64: setup local for FUN_0078f810/40 sub-calls

0049f265: CMP byte ptr [ESP+0x30], 0      ; check intermediate result
0049f26a: JZ skip                         ; skip if zero
0049f26c: MOV ECX, EDI                    ; EDI = &receiver[+0x6c] = LuaParamsContainer
0049f26e: CALL 0x0089e200                 ; ⭐ THE +0x80 SETTER

skip:
0049f281: ... cleanup, return ...
```

## FUN_0089e200 — the +0x80 setter (92 bytes)

```c
// ECX = this = LuaParamsContainer*  (= &receiver[+0x6c])
void LuaParamsContainer::__set_is_notice_and_init_first_param() {
    this->is_notice_flag = 0x01;                  // ⭐ receiver[+0x80] = 1
    
    // Validate the byte buffer is non-empty + non-degenerate:
    if (this->param_buffer_begin == NULL ||
        (this->param_buffer_end - this->param_buffer_begin) == 0)
        runtime_assert();                          // FUN_009d22b4 = std::_Xinvalid_argument
    
    // memzero the byte buffer:
    memset(this->param_buffer_begin, 0, end - begin);   // FUN_009d2110
    
    // Validate again, then mark the FIRST byte of the buffer:
    if (this->param_buffer_begin == NULL || end - begin == 0)
        runtime_assert();
    *((uint8_t*)this->param_buffer_begin) = 0x01;       // = LuaParam::Type::True (or similar)
}
```

So **`FUN_0089e200` sets `receiver[+0x80] = 1` AND seeds the first
LuaParam byte to `0x01`** (probably the `LuaParam::True` type tag).

## The event-type enum table

At `0x012c3f7a..7e` (in `.data` section), 5 sequential bytes:

| Address | Value | Semantic (inferred) |
|---|---|---|
| `0x012c3f7a` | `0x01` | (event type 1 — talk?) |
| `0x012c3f7b` | `0x02` | (event type 2 — emote?) |
| `0x012c3f7c` | `0x03` | (event type 3 — push?) |
| `0x012c3f7d` | `0x04` | (event type 4 — walk?) |
| **`0x012c3f7e`** | **`0x05`** | **"noticeEvent" — cinematic dispatch** ⭐ |

These bytes are referenced by various receivers (e.g.
SetEventStatusReceiver at `FUN_006e67c0` uses `[0x012c3f7a/b/c]`
for its 3-way dispatch on event type — see Phase 9 #8c work).

The 5-tag scheme matches the FFXIV 1.x event_type enum
documented in pmeteor server code.

## Kick state-machine implication (what garlemald is hitting)

Re-examining Branch B1's full check (from
`docs/event_kick_receiver_decomp.md`):

```c
if (context_root[+0x128] == NO_ACTOR) {
    // BRANCH B1: completely fresh, no previous target
    if (receiver[+0x80] != 0) {                  // ← THE GATE
        context_root[+0x12c] = receiver[+0xc];   // store target id
        return FAILURE;                           // queue for later retry
    }
    // (else fall through → return SUCCESS, no-op kick)
}
```

So:
- If garlemald sends kick with `event_type=5`: **receiver[+0x80]=1**
  → Branch B1 stores target id, returns FAILURE → kick is queued
  for re-attempt next tick (Branch A picks it up when actor's
  `+0x5c` flag is set)
- If garlemald sends kick with `event_type≠5`: **receiver[+0x80]=0**
  → Branch B1 silent fall-through → kick is dropped

## Implications for garlemald's SEQ_005 silent-drop

If garlemald's KickEvent has `event_type == 0x05`:
- `receiver[+0x80]` IS set to 1
- Branch B1 should queue for retry
- The kick should eventually fire when actor `+0x5c` is set

If garlemald's KickEvent has `event_type != 0x05` (e.g. 1 for talk,
3 for push):
- `receiver[+0x80]` stays 0
- Branch B1 silent fall-through
- Kick never fires (this matches the observed silent drop)

**Garlemald needs to verify**: what is the `event_type` byte in
the KickEvent body it sends for `noticeEvent` (SEQ_005's cinematic
trigger)? The byte at packet body offset 8 (== `receiver[+0x68]`)
must be `0x05` for the kick to register.

The byte-by-byte pcap diff (commit `093a1b27c`-era) reportedly
showed garlemald's kick body is byte-identical to pmeteor's at
offsets 0..0x60, so `event_type` byte (offset 8) should match if
the diff was accurate. But re-verify — this is the single most
load-bearing byte for the kick gate.

## Cross-reference: Branch B2 also uses FUN_0089e200

Notably, the kick Branch B2 calls `FUN_0089e200(receiver + 0x6c)`:

```c
} else {
    // ─── BRANCH B2: previous target stored at [+0x128] ───
    FUN_0089e200(receiver + 0x6c);                  // ⭐ ALSO sets is_notice_flag = 1
    actor = ActorRegistry_lookup_actor(context_root + 0x128);
    if (actor == NULL || actor[+0x5c] == 0) {
        *out_result = FAILURE_CODE;
        return out_result;
    }
}
```

So Branch B2 ALSO sets `receiver[+0x80] = 1` (defensively) when
it activates. This is consistent: B2 is the "previous target in
flight, gate-check the actor" path — and the is_notice_flag is
re-asserted before the +0x5c gate check.

## Phase 9 #8a status: ✅ RESOLVED

| Step | Status |
|---|---|
| Map receiver[+0x80] to instance offset (LuaParamsContainer[+0x14]) | ✅ (pre-existing) |
| Find packet byte source | ✅ resolved 2026-05-17 — `event_type == 0x05` triggers `FUN_0089e200` which sets it |
| Identify the trigger function | ✅ `FUN_0089e200` (92 B) |
| Identify the parser | ✅ `FUN_0089f180` (289 B) |

## Cross-references

- `docs/seq005_receiver_gate_audit.md` — Phase 9 #8 (the audit
  that surfaced this gate as the prime suspect)
- `docs/event_kick_receiver_decomp.md` — Phase 7 KickReceiver
  decomp (slot 2 = Receive; the source of the Branch B1 logic)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 (full
  receiver inventory; KickReceiver vtable @ 0xc574b0, 5 slots)
- `docs/context_root_128_priming_hunt.md` — Phase 9 #8c (the
  related question of how `[+0x128]` gets primed; uses the same
  event-tag table at `0x012c3f7a/b/c` for SetEventStatusReceiver)
