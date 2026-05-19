# MyPlayer::vtable[3] — caller hunt (Phase 9 #8e continuation)

> Recovered 2026-05-18. Continuation of
> `dispatcher_subscriber_replacer.md`.

## TL;DR

**MyPlayer::vtable[3] = `FUN_006e3440`** is NOT a Lua-bound binding.
It's a pure C++ virtual method, invoked exclusively through bridge
wrappers embedded as vtable slots of **cutscene/layout system
classes**. This means the trigger that clears the dispatcher
inhibitor is a C++ cutscene-state-change side-effect, not a
script-callable hook.

## How we know it's not Lua-bound

Walked the PlayerBaseClass binding-setup function (FUN_0072deb0
≈ FUN_00753f90 per recipe) by scanning for all `MOV reg, imm32`
loads of .text addresses, then filtering to addresses pointing to
valid 10-byte vtable-N thunks. Result: 75 slot thunks registered,
covering slots **34 through 132** in scattered fashion. **Slot 3
is NOT among them.**

Also searched the binary for any 10-byte slot-3 thunk pattern
(`8B 01 8B 80 0C 00 00 00 FF E0`) — **zero hits**. And searched
for any direct virtual call to vtable slot 3 (patterns
`8B 01 FF 50 0C`, `8B 06 FF 50 0C`, `8B 07 FF 50 0C`,
`FF 51 0C`) — **zero hits**.

So slot 3 is reached only via vtable dispatch and never via Lua.

## How it IS called

The 7-byte short-form slot-3 thunk pattern `8B 01 8B 40 0C FF E0`
has 9 hits, all inside larger functions that look like:

```asm
MOV ECX, [ECX + offset]   ; load sub-object pointer
TEST ECX, ECX
JNZ +5
XOR EAX, EAX
RET 4
8B 01 8B 40 0C FF E0      ; vtable[3] call via inner object
```

These are **bridge wrappers** — functions that take "this" (some
container class), reach into a member sub-object at a specific
offset, and forward the call to that sub-object's vtable[3]. Five
bare wrappers identified:

| File | VA | Subobject offset | Containing-class RTTI |
|---|---|---|---|
| 0xd6780 | 0x4d6780 | `+0x84` | (no .rdata xrefs — unused) |
| 0x257730 | 0x657730 | `+0xc8` | **RaptureLayoutManager** (Layout/Map/Actor/Scene) |
| 0x3dfc10 | 0x7dfc10 | `+0x04` | **CutReferenceResource** (CutScenePlayer) |
| 0x6191b0 | 0xa191b0 | `+0x78` | **MccScheduler** (Plugins/Cut/Engine) |
| 0x69d250 | 0xa9d250 | `+0x84` | (bogus RTTI — unable to decode) |

All four decoded classes are **cutscene / layout system classes**.
None of them is MyPlayer or a player-control class.

## Implication for SEQ_005

The dispatcher inhibitor `dispatcher->[+0xf8]->[+0x1e]` gets
cleared by a **cutscene/layout state-change side-effect** —
specifically, when one of those wrapper classes invokes its
vtable slot that contains the bridge to MyPlayer::vtable[3].

For the SEQ_005 hang:

1. **If the gate is supposed to clear BEFORE the kick lands**:
   garlemald must be missing a cutscene/layout state transition
   that pmeteor produces. Worth checking the layout-manager and
   cutscene-related packets in the pre-kick window.

2. **If the gate is supposed to clear AFTER the kick lands**:
   the kick is a chicken-and-egg problem — the kick won't fire
   until the cinematic plays, and the cinematic won't play until
   the kick fires. This is consistent with the observed hang
   pattern. The fix in this case would be on a different gate
   (Branch B1's `receiver[+0x80]`, or context_root state).

The most likely scenario is **(1)** — a layout-manager state
transition during the warp should fire one of the bridge wrappers,
which calls MyPlayer::vtable[3], which swaps the dispatcher
subscriber, which clears `[+0x1e]`, which then lets the kick
through.

## Next-session leads

1. **Layout-manager state diff**: walk garlemald + pmeteor packet
   logs for any Layout or RaptureLayoutManager-related opcodes
   in the pre-kick window. The wiki / cpp_bindings.md should
   surface their names.

2. **Decompile FUN_006e03b0** and the other helpers called by
   FUN_006e3440 — they might reveal what state the dispatcher
   ends up in after the swap, which clarifies what the swap is
   "for". Specifically, what gets stored in the new 0x20-byte
   subscriber.

3. **Walk the bridge-wrapper vtable slots**: in
   RaptureLayoutManager / CutReferenceResource / MccScheduler,
   find which slot N hosts the bridge to MyPlayer::vtable[3].
   That slot's purpose (visible from sibling slots / class
   documentation) names the higher-level event.

## Cross-references

- `docs/dispatcher_subscriber_replacer.md` — the slot 3 = FUN_006e3440
  finding (2026-05-18, prior in this session)
- `docs/kick_dispatcher_clearer.md` — the slot 66 clearer (Phase 9 #8e)
- `memory/reference_meteor_decomp_vtable_lookup_recipe.md` — recipe
  applied here that conclusively rules out slot 3 being Lua-bound
- `memory/project_garlemald_seq005_packet_replay_invalidated.md` —
  established that wire is identical pre-kick, narrowing the search
  to C++-side state difference
