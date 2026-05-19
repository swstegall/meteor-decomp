# Phase 9 #8e follow-up — Dispatcher [+0xf8] subscriber replacer

> Recovered 2026-05-18. Continuation of the [+0xf8] clearer hunt
> noted in `kick_dispatcher_clearer.md` ("hypothesis 2: dispatcher
> ->[+0xf8] gets REPLACED with a different subscriber pointer").

## TL;DR

Found `FUN_006e3440` — the function that REPLACES the dispatcher
subscriber at `[+0xf8]`. It allocates a fresh 0x20-byte subscriber
via `FUN_00895f50`, destroys the old one through its vtable[0]
destructor, and writes the new pointer into `[ESI+0xf8]`. The
new subscriber starts with `[+0x1e] = 0`, which clears the kick
gate (Branch A's `FUN_006e11d0` predicate returns 0 again).

**FUN_006e3440 is reached only via vtable dispatch — exactly one
absolute reference exists in the binary (at file offset 0xbd7868
= VA 0xfd7868 = vtable slot 3 at vtable base 0xfd785c)**, which is
**`MyPlayer::vtable[3]`** (same vtable as the Phase 9 #8a
`_fadeInNowLoadingForNoticeEventJustInArea` clearer at slot 66).

So the kick-receiver gate has TWO clearer mechanisms, both on
MyPlayer:

| Slot | Function | Mechanism |
|---|---|---|
| **3** | **FUN_006e3440** | Replaces dispatcher->[+0xf8] subscriber (this doc) |
| 66 | FUN_006e32f0 | Writes NO_ACTOR to context_root [+0x128] / [+0x12c] |

## Decompiled body of FUN_006e3440 (178 bytes)

```c
// Pseudocode synthesized from asm/ffxivgame/002e3440_FUN_006e3440.s
void MyPlayer__vtable_slot_3(this, p1, p2) {   // __thiscall
    // p1 in [ESP+0x20], p2 in [ESP+0x24] after PUSH-prologue
    
    // 1. Helper call — installs p1/p2 into some intermediate state
    FUN_006e03b0(p1, p2);
    
    // 2. Get the engine "context root" — same FUN_00cc7510 the
    //    other dispatcher methods use
    void *ctx = FUN_00cc7510(p1);
    void *something = ctx->vtable[1];
    FUN_00758e40(this, p2);
    FUN_00cc73e0(this, 1, p1);          // some "register" call
    
    // 3. Allocate a fresh 0x20-byte subscriber
    void *new_sub = operator_new(0x20);
    if (new_sub) {
        // Initialize the new subscriber using p1 as input
        new_sub = FUN_00895f50(new_sub, p1);
    } else {
        new_sub = NULL;
    }
    
    // 4. Read the OLD subscriber pointer
    void *old_sub = this->[0xf8];
    
    // 5. If different from new AND non-null, destroy the old
    if (old_sub != new_sub && old_sub != NULL) {
        // vtable[0] = scalar deleting destructor
        old_sub->vtable[0](old_sub, /* delete_flag */ 1);
    }
    
    // 6. Replace the pointer
    this->[0xf8] = new_sub;
}
```

The key insight: **calling this method swaps the subscriber for a
fresh one**. The new subscriber's `[+0x1e]` byte starts at 0 (it's
a fresh allocation — `operator new` returns zeroed memory in
debug builds, and FUN_00895f50's initialization presumably doesn't
set `[+0x1e]=1`). So immediately after this call, FUN_006e11d0
returns 0, and the kick gate predicate clears.

## Why this matters for SEQ_005

The Phase 9 #8e walk identified that `dispatcher->[+0xf8]->[+0x1e]`
stuck at 1 would inhibit Branch A of `KickClientOrderEventReceiver`,
causing the kick to silently no-op. We now know how the byte gets
"cleared": not via a direct `MOV imm 0` but via a SUBSCRIBER SWAP.

If the SEQ_005 hang is caused by `[+0x1e]` stuck at 1, the natural
fix is to find the trigger that calls `MyPlayer::vtable[3]` in
pmeteor's working flow and ensure garlemald replicates whatever
provokes that call client-side.

## What MyPlayer::vtable[3] is named in Lua

**TBD — needs a Ghidra session.** Per the
`reference_meteor_decomp_vtable_lookup_recipe.md` recipe, slot 3
maps to byte offset 0xC. The MSVC thunk pattern
`8B 01 8B 40 0C FF E0` (7-byte short-form for slots 0-31) was
searched in the binary and produces 9 hits — but most are
sub-object wrapper thunks rather than the bare Lua-bound thunk.
The 10-byte `8B 01 8B 80 0C 00 00 00 FF E0` form (32-bit disp
variant the recipe expects) produces 0 hits, which suggests MSVC
chose the short form for this slot (legal since 0xC < 0x80).

Candidate thunk locations (5 of the 9 hits with clean CC padding):

| File offset | VA | Pre-context |
|---|---|---|
| 0xd6786 | 0x4d6786 | `8b 89 84 00 00 00` (sub-object wrapper at +0x84) |
| 0x25773f | 0x65773f | `c9 75 05 33 c0 c2 04 00` (clean function-boundary) |
| 0x3dfc13 | 0x7dfc13 | `cc cc cc cc cc 8b 49 04` (clean — sub-object wrapper at +0x4) |
| 0x6191bc | 0xa191bc | `c9 75 05 33 c0 c2 04 00` (clean function-boundary) |
| 0x69d25f | 0xa9d25f | `74 0d 8b 89 84 00 00 00` (sub-object wrapper at +0x84) |

The two "clean function-boundary" hits at VA 0x65773f and 0xa191bc
are the strongest candidates for the bare Lua-bound thunk that
appears as a small `LAB_xxxxxxxx` in Ghidra. Their xrefs would
lead to the binding-helper registration call (the `FUN_00447260("...", 0xffffffff)`
recipe step). The Lua name there is `MyPlayer::vtable[3]`'s callable
identifier.

**Next-session action**: open one of those two VAs in Ghidra,
walk xrefs to the binding helper, read the `_xxx_cpp` string. The
name will tell us what player-visible action / state-change
triggers the subscriber swap.

## Cross-references

- `docs/kick_dispatcher_clearer.md` — the hypothesis tree this
  resolves part of; the "subscriber pointer replacement" candidate
  was hypothesis 2 (now confirmed)
- `docs/event_kick_receiver_decomp.md` — Branch A's
  `FUN_006e11d0` predicate gates on `this->[+0xf8]->[+0x1e]`
- `memory/reference_meteor_decomp_vtable_lookup_recipe.md` — the
  thunk → Lua-name recipe to apply for slot 3
- `memory/project_garlemald_seq005_packet_replay_invalidated.md` —
  prior session's wire-level finding that no packet diff exists;
  this RTTI-side finding supplies the missing mechanism
