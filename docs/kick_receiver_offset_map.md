# Phase 9 #8a — KickReceiver instance offset map

> Last updated: 2026-05-15. Maps the KickClientOrderEventReceiver
> instance fields to packet body bytes. Resolves "what does
> `receiver[+0x80]` correspond to in the packet?" as far as static
> analysis can take it without runtime tracing.

## Summary

The receiver instance is **132 bytes** (allocated via `operator new(0x84)`
in slot 1). The instance layout, recovered from the copy-constructor
`FUN_0089f2b0` and the sub-object copy-constructor `FUN_0089ecf0`:

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
  /* +0x04 */ uint8_t  body[16];                // some 16 bytes of state — probably array
                                                //   ptr+count+capacity for the LuaParam list
  /* +0x14 */ uint8_t  unknown_byte;            // ← THIS IS receiver[+0x80]
  /* +0x15 */ uint8_t  pad[3];                  // (or more state)
};
```

So **`receiver[+0x80]` = `(LuaParamsContainer at +0x6c)[+0x14]`** =
the LAST byte of the Lua-params container's 16-byte state region
+ 1.

## Where does `+0x80` come from in the packet bytes?

Static analysis can't pinpoint this exactly without finding the
packet→receiver parse routine (which slot 1 doesn't expose —
slot 1 is just the heap-clone copy ctor). The original parse
happens in IpcChannel / packet-decoder code we haven't traced.

Three possibilities:

1. **It's a discriminator byte the parser sets based on the LuaParam
   stream's leading bytes.** E.g., if the Lua params are
   non-empty, set `params.unknown_byte = 1`. Garlemald's
   KickEvent body sends `args = [LuaParam::True, end-marker]`
   for `noticeEvent` (matches pmeteor exactly per byte-diff), so
   this byte SHOULD be set if it's discriminator-driven.

2. **It's a packet-body byte at offset 0x80.** The KickEvent body
   is 0x80 bytes (= 128) — so byte 0x80 would be just past the
   end. That's unlikely.

3. **It's set by an EARLIER packet** (e.g. a preceding "prepare
   for kick" packet that primes the receiver state). This would
   explain why pmeteor's flow works (some prior packet sets it)
   while garlemald's fails (we don't send the prior packet).

## Why the byte-by-byte KickEvent body diff didn't help

The pmeteor pcap byte-diff (commit `093a1b27c`-era smoke-test) showed
garlemald's KickEvent body is **byte-identical** to pmeteor's at
offsets 0..0x60. The packet's own bytes can't explain the
divergence.

The only remaining explanation is that the gate's value comes
from RECEIVER STATE, not from the packet — and that state is
established by something OTHER than the kick packet itself.

## Cross-reference: Branch B1's full check

```c
// docs/event_kick_receiver_decomp.md slot 2 logic
if (context_root[+0x128] == NO_ACTOR) {
    // BRANCH B1: completely fresh, no previous target
    if (receiver[+0x80] != 0) {                  // ← THIS CHECK
        context_root[+0x12c] = receiver[+0xc];   // store target id
        return FAILURE;                           // (queues for later retry)
    }
    // (else fall through → return SUCCESS, no-op kick)
}
```

For garlemald's post-warp kick:
- `context_root[+0x128]` is `NO_ACTOR` — fresh post-warp (warp wiped it)
- Branch B1 fires
- If `receiver[+0x80] == 0`, the kick silently no-ops
- If `receiver[+0x80] != 0`, the target is QUEUED for a later retry
  attempt (Branch A would pick it up next tick if the actor is
  spawned by then)

So if we can figure out what makes `receiver[+0x80] != 0`, we
unblock the kick.

## Next-step options

| # | Approach | Cost |
|---|---|---|
| #8a-i | Trace IpcChannel's KickEvent packet parser to find `params.unknown_byte` writes | High — requires finding the parser routine across the codebase |
| #8a-ii | Compare the receiver state side-by-side at the moment of the kick (runtime debug) | Highest — needs Wine + breakpoint or Cheat Engine |
| #8a-iii | Hypothesize "we need to send a prior packet to prime the receiver state" → look at pmeteor's pre-warp packets for any opcode garlemald doesn't send (esp. ones targeting the kick receiver's owning context) | Mediumi — depends on identifying the right packet from pmeteor's bundle |
| #8a-iv | Patch garlemald to send `receiver[+0x80]=1` indirectly by sending an extra LuaParam in the kick body, and re-test smoke | Medium — empirical, but may surface what the byte controls |

## Cross-references

- `docs/seq005_receiver_gate_audit.md` — Phase 9 #8 (the audit
  that surfaced this gate as the prime suspect)
- `docs/event_kick_receiver_decomp.md` — Phase 7 KickReceiver
  decomp (slot 2 = Receive; the source of the Branch B1 logic)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 (full
  receiver inventory; KickReceiver vtable @ 0xc574b0, 5 slots)
