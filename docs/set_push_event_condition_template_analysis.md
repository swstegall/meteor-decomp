# `SetPushEventCondition*Receiver` template analysis

> Recovered 2026-05-17. Analysis of the 3 `SetPushEventCondition*` 2-slot
> Receivers (Circle / Fan / TriggerBox) — geometry packers from Phase 9
> #6's "Pattern A2.1 — pack-and-forward" group. Initial hope was to
> match all 3 via a single template (Phase 2.5 pipeline). Outcome:
> they're SIMILAR but NOT byte-identical templates; each needs its own
> source. Documenting the field layout + structural analysis as a
> head-start for future matching work.

## TL;DR

**Not template-derivable**: the 3 functions share a "pack stack args
then call downstream handler" shape but differ in:
- Field offsets read (different geometry per variant)
- Arg count (9 for Circle/TriggerBox, 11 for Fan)
- FPU usage pattern (Fan has 3 FLD/FSTP, others have 2)

So Phase 2.5's seed-template stamping won't yield 3 GREEN from 1
source. Each is a separate matching task; realistic estimate is
1-2 hours per function to reach GREEN due to FPU instruction
sequencing + PUSH-order register allocation sensitivity.

## The 3 receivers — at a glance

| Receiver | RVA | Size | Handler | Arg count | FLD count |
|---|---|---:|---|---:|---:|
| `SetPushEventConditionWithCircleReceiver` | `0x49db00` | 80 B | `FUN_006f2b70` | 9 | 2 |
| `SetPushEventConditionWithFanReceiver` | `0x49dc90` | 96 B | `FUN_006f2c30` | 11 | 3 |
| `SetPushEventConditionWithTriggerBoxReceiver` | `0x49de20` | 80 B | `FUN_006f2d00` | 9 | 1 |

## Circle receiver field layout (recovered)

```c
struct SetPushEventConditionWithCircleReceiver {
    void*  vtable;            // +0x00
    char   pad[0x54];         // +0x04..+0x57
    float  pos_x;             // +0x58 — passed as &pos_x (pointer)
    byte   flags1[3];         // +0x59 — passed as &flags1 (pointer to first byte)
                              //         (flags1[0/1/2] occupy +0x59/+0x5a/+0x5b)
    int    condition_id;      // +0x5c — passed by value
    float  pos_y;             // +0x60 — passed by value (after FSTP)
    byte   flags2_lo;         // +0x64
    byte   flags2_mid1;       // +0x65
    byte   flags2_mid2;       // +0x66
    byte   flags2_hi;         // +0x67
    float  radius;            // +0x68 — passed by value (after FSTP)
};

// Receive (slot 1, 80 B):
int Receive(Receiver *this, Caller *caller) {
    // The actual asm is hand-written and the C++ equivalent is roughly:
    return FUN_006f2b70(
        this + 4,             // arg: bumped this (after ADD ECX, 4)
        &this->pos_x,         // arg: pointer to pos_x
        &this->flags1,        // arg: pointer to flags1
        this->condition_id,   // arg: value
        this->pos_y,          // arg: float value
        this->flags2_lo,      // arg: byte
        this->flags2_mid1,    // arg: byte
        this->flags2_mid2,    // arg: byte
        this->flags2_hi,      // arg: byte
        this->radius,         // arg: float value
        caller                // arg from stack — the "Caller" passed to Receive
    );
}
```

## Fan receiver — adds 2 extra floats (angle / radius2)

`SetPushEventConditionWithFan` (96 B) has the same Circle layout PLUS:
- `+0x6c` float (probably `inner_radius` or `angle_start`)
- `+0x70` float (probably `angle_end` or `direction`)

The asm starts with `FLD [ECX+0x70]` and pushes via stack manipulation
(`SUB ESP, 0x0C; FSTP [ESP+0x08]; FLD [ECX+0x6c]; FSTP [ESP+0x04]`) —
that's the "push 2 floats" idiom MSVC uses for variadic-style call
sites with multiple float args.

## TriggerBox receiver — uses a word + a pointer

`SetPushEventConditionWithTriggerBox` (80 B) has DIFFERENT field types
at the equivalent offsets:
- `+0x6c` is a **u16 (word, 2 bytes)** — `MOVZX EDX, word [ECX+0x6c]`
- `+0x68` is a **4-byte pointer** — `MOV EAX, [ECX+0x68]`

So the trigger-box variant stores a bounding-box reference at +0x68
and a size/count word at +0x6c, rather than a center-point float
and radius like Circle/Fan.

## Recovered handler function pointers (downstream)

| Variant | Handler RVA | Size (TBD) |
|---|---|---|
| Circle | `FUN_006f2b70` | 9-arg signature |
| Fan | `FUN_006f2c30` | 11-arg signature |
| TriggerBox | `FUN_006f2d00` | 9-arg signature |

These 3 handlers are at consecutive addresses (`0x6f2bxx`) — they're
likely a related triplet inside some single module (maybe a
`PushConditionHandler` class).

## Why matching to GREEN is hard

Three sources of compiler-dependent byte choice:

1. **FPU register-stack ordering**. MSVC chooses the FLD/FSTP order
   based on which floats are read first. The orig sequence
   (`FLD +0x68; FSTP [ESP]; FLD +0x60; FSTP [ESP+N]`) requires
   matching the source-order of float pushes exactly.

2. **PUSH-order vs reg-load ordering**. The asm interleaves
   `MOVZX byte` reads with `PUSH` instructions in a specific pattern.
   The C source's argument order determines this — but MSVC may
   reorder for register efficiency.

3. **Frame setup variance**. Some receivers (Fan) use explicit
   `SUB ESP, 0x0C` to reserve space for the FPU push; others fold
   it into the natural `PUSH` sequence. This is compiler-driven and
   hard to coax from C source alone.

## Recommended next-step approach (when matching is attempted)

1. Start with Circle (smallest + simplest).
2. Write a C++ source with the struct layout above.
3. Write a `Receive` function with PUSH-order matching the orig.
4. Iterate with `tools/compare.py` until GREEN.
5. Adapt for Fan (add 2 float args) and TriggerBox (swap float/pointer
   types).

Realistic time: 1-2 hours per function (so 3-6 hours total for all 3).

## Alternative: byte-passthrough fallback

Phase 2.6's byte-passthrough fallback (`tools/emit_passthrough.py` —
producing `__declspec(naked)` `_emit`-only `.cpp` files) would let
these 3 functions enter the "complete .obj inventory" without
requiring source-level matching. The trade-off: passthrough .cpp files
aren't readable source code (just byte literals), but they're
byte-identical and compile cleanly.

For Phase 9 receiver work, passthrough is probably the right choice —
the receivers' BEHAVIOR is already documented in
`docs/receiver_gate_cheatsheet.md`; their byte-identical re-emission
isn't load-bearing for understanding.

## Cross-references

- `docs/receiver_gate_cheatsheet.md` — Phase 9 #6/#7 (the 3 receivers
  are listed in the "A2.1 — pack-and-forward" group)
- `docs/receiver_classes_inventory.md` — Phase 9 #1 (the 43-receiver
  inventory; these 3 share the `SetPushEventConditionWith*` naming)
- `docs/decomp-status.md` — Phase 2.5 template-derivation pipeline
  context (this analysis confirms NOT a template-stamp candidate)
