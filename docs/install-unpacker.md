# `Component::Install::InstallUnpacker`

The FFXIV installer's chunk-extraction class. Documented here as part
of the Phase 4 reconnaissance trail — `FUN_00cc6700` is its slot-2
virtual method (the main unpack-loop) and the only direct consumer of
`Sqex::Data::PackRead` in `ffxivgame.exe`.

## Class hierarchy (recovered 2026-05-02)

`Component::Install::InstallUnpacker` is a **thread class** — extends
`Sqex::Thread::Thread` as primary base, with a secondary base in the
`InstallWriter` family at member offset `+0x38`. Confirmed via:
  - The D1 destructor at `0x008be9f0` calls
    `Sqex::Thread::Thread::~Thread` at `0x00935560` (which writes
    `[ESI] = 0x01110688` = the Sqex::Thread::Thread vtable, RTTI-named).
  - The destructor swaps `[ESI+0x38]` between `0x0110d524` (an
    InstallUnpacker secondary vtable) and `0x0110d4f0`
    (InstallWriter::WriteEntry vtable) — typical MSVC virtual-base
    destruction sequence.

So `Unpack` (slot 2) is a Thread::Run-style override that runs on
worker threads dispatched from a chunk-source. The whole architecture
is producer-consumer:
  - Main thread fills a `ChunkSource` with chunk descriptors.
  - Worker InstallUnpackers spin on the source via `WaitForReady`,
    extract chunks via `PackRead` + a Utf8String per chunk, then
    `ChunkSource::ReleaseChunk` to signal completion.
  - Source state machine: state=3 = "all dispatched, waiting", state=4
    = "all released → done".

## Primary vtable

```
Component::Install::InstallUnpacker  primary vtable @ RVA 0x00d0d53c (4 slots)
  slot 0: FUN_00cbea90 @ 0x008bea90    ; ~InstallUnpacker (D2 wrapper, 30 B)
  slot 1: FUN_00d35590 @ 0x00935590    ; tiny `MOV AL, 1; RET` — likely
                                       ; Thread::IsAlive() override returning true
  slot 2: FUN_00cc6700 @ 0x008c6700    ; ★ Thread::Run override — unpack
                                       ;   loop (490 B); see below
  slot 3: FUN_00d355a0 @ 0x009355a0    ; single `RET` — empty/nop slot

typeinfo ptr at vtable-4: 0x0119d4e8
```

## `FUN_00cc6700` — slot-2 unpack loop (490 B)

### High-level structure

The method follows a "wait for resource → construct PackRead →
iterate chunks → tear down" pattern:

1. **SEH frame setup** — large frame (~0xe0 bytes locals) with
   security cookie. The frame holds a stack-allocated `Utf8String`
   (at `[ESP+0x9c]`) and a stack-allocated `PackRead` (at
   `[ESP+0x1c]` via `LEA ECX, [ESP+0x24]` minus 2-arg push offset).
2. **Wait for resource ready** — calls `EDI` (= `InterlockedExchangeAdd`,
   the same atomic primitive `Utf8StringFree` uses) on
   `&[ESI+0x40]+0x60` and `&[ESI+0xa8]`. The first returns 4 → bail
   path; the second returns 0 → bail path. Both probe a refcount
   or "is-ready" state.
3. **Acquire chunk source** — calls `FUN_00cc5db0(this->m_field40,
   &local)` which returns a non-null pointer (saved in `EBX`); if
   null, jump to teardown.
4. **Construct local Utf8String** — `Utf8String::Utf8String` at
   `[ESP+0x9c]` via `CALL 0x00445cf0` (note: this Utf8String ctor is
   at a DIFFERENT RVA than the one we matched at 0x00047260 — likely
   a different overload or a Sqwt-namespace string class).
5. **Construct PackRead** — `PackRead::PackRead([ESP+0x24], data, size)`
   via `CALL 0x00d42800`.
6. **Process chunks in a loop** — body at offset 0xc0..0x190:
   - `FUN_00447450(&[ESP+0x38])` — same target as
     `SubObjAt1c::Process` in `PackRead::ProcessChunk`, takes a
     pointer to a stack local
   - `FUN_00cc6510(&[ESI+0x48], &[ESI+0xa4])` — InstallUnpacker
     internal helper
   - Various atomic operations on `[ESI+0x40+0x2140]` (a counter
     ~0x2140 bytes into a child object)
   - `FUN_00d22b4` — looks like a CRT routine (in 0x9d2xxx
     range = MSVC 2005 crt section)
   - `FUN_00cc6620(&[ESI+0x38])` — small (71 B) wait-for-ready
     helper using `InterlockedExchangeAdd`
   - `EDI` calls (atomic add) on the resource state
   - `PackRead::ReadNext` at `[ESP+0x1c]`
   - Optional secondary refcount probe (loop back if not ready)
7. **Teardown** — `FUN_00cc5e40` (release chunk source),
   `PackRead::~PackRead`, `Utf8String::~Utf8String`, SEH frame
   teardown, `__security_check_cookie`, RET.

### Parent class field layout (inferred from access pattern)

```c
class Component::Install::InstallUnpacker {
    void *vtable;                  // +0x00
    /* +0x04..+0x37: unknown */
    char  m_resource_state[4];     // +0x38 (passed to FUN_00cc6620)
    /* +0x3c: another field accessed as ptr */
    int   m_field_40;              // +0x40 (used in atomic ops at
                                   //         offset +0x60 and +0x2140)
    int   m_field_44;              // +0x44
    /* +0x48: a sub-object accessed via LEA, passed to FUN_00cc6510 */
    /* +0x4c..+0x9b: unknown */
    int   m_field_9c;              // +0x9c (gets a value stored)
    int   m_field_a0;              // +0xa0
    void *m_field_a4;              // +0xa4 (passed to FUN_00cc6510)
    char  m_field_a8[N];           // +0xa8 (atomic-counter probed at
                                   //         start; bail-out trigger)
};
```

### Direct call graph

| Offset | Target | Notes |
|---|---|---|
| 0x4c | `EDI` (= [`0x00f3e1a4`]) | `InterlockedExchangeAdd` (atomic) — probe field+0x60 |
| 0x60 | `EDI` | Same — probe field+0xa8 |
| 0x72 | `FUN_00cc5db0` (268 B) | Acquire chunk source |
| 0x8c | `FUN_00445cf0` | Utf8String alt-ctor (different from 0x47260) |
| 0xa6 | `PackRead::PackRead` (✅ matched 98%) | Construct local PackRead |
| 0xc8 | `FUN_00447450` | Same target as `SubObjAt1c::Process` in ProcessChunk |
| 0xd2 | `FUN_00cc6510` (343 B) | InstallUnpacker helper |
| 0xe5 | `EDI` | Atomic add |
| 0x113 | `FUN_00d22b4` | CRT-like helper (signed div?) |
| 0x143 | `FUN_00cc6620` (71 B) | Wait-for-ready spin |
| 0x14b | `EDI` | Atomic add |
| 0x154 | `[0x00f3e1c8]` = `KERNEL32.DLL::Sleep` | Yield/sleep in wait loop (confirmed via Ghidra 2026-05-02) |
| 0x15d | `EDI` | Atomic add |
| 0x168 | `PackRead::ReadNext` (✅ matched GREEN) | Loop step |
| 0x17a | `EDI` | Atomic add |
| 0x18c | `FUN_00cc5e40` (124 B) | Release chunk source |
| 0x19d | `PackRead::~PackRead` (✅ matched GREEN) | Local PackRead teardown |
| 0x1b4 | `Utf8String::~Utf8String` (✅ matched GREEN) | Local string teardown |
| 0x1de | `__security_check_cookie` | Standard MSVC `/GS` check |

## Iteration history

### Iteration #1 (2026-05-02 16:40, commit ea0bf0aaf) — 41 % match

First-pass translation. Treated the loop body as if it filled a separate
stack-allocated subobj (`char subobj_buf[0x58]; char *str_begin, *str_end`)
and called a stub Process() on it. Frame allocated at `SUB ESP, 0x138`
(0x58 over orig's 0xe0).

Result: 218/428 reloc-masked matches (50.9 % of our 428 B vs orig's 490 B).

### Iteration #2 (2026-05-02, commit 2ecb15be1) — STRUCTURAL FIX

**Key discovery** from cross-referencing PackRead.cpp + FUN_00447450's
60-byte body:

1. **The "stack subobj" at `[ESP+0x38]` is not a separate local** — it's
   `pack_reader.m_subobj`. PackRead embeds a Utf8String at +0x1c, and
   pack_reader lives at `[ESP+0x1c]`, so its m_subobj naturally lands at
   `[ESP+0x38]`.
2. **FUN_00447450 is `Utf8String::operator=`** — verified by walking its
   body (copies m_data via memcpy, calls Reserve, copies m_field_c +
   m_flag_10). So the call sequence
   `LEA ECX,[ESI+0x48]; LEA EDX,[ESP+0x38]; PUSH EDX; CALL 0x00447450`
   is `m_field_48 = pack_reader.m_subobj` — Utf8String copy assignment
   into the InstallUnpacker's m_field_48 (also a Utf8String).
3. **The `[ESP+0x90]` and `[ESP+0x94]` reads** in the loop body are
   `pack_reader.m_buffer` and `pack_reader.m_field78` (PackRead +0x74 /
   +0x78 — the heap-buffer begin/end pointers). Not separate locals.

Source changes:
- Removed bogus `subobj_buf[0x58]; str_begin; str_end;` locals.
- Added `Utf8String m_field_48` to the class layout.
- Replaced `((SubObjAt1cStub *)&m_field_48)->Process(subobj_buf)` with
  `m_field_48 = pack_reader.m_subobj`.
- Read `pack_reader.m_buffer` / `m_field78` directly for begin/end ptrs.
- Dropped `extern "C"` from helpers — needed so MSVC presumes throwing
  and emits the C++ EH frame (with `/EHsc`, `extern "C"` is nothrow,
  which would suppress the EH frame setup orig has).

Result: 244/490 reloc-masked matches (49.8 %), function size 493 B vs
orig's 490 B. **Frame size now correct (`SUB ESP, 0xe0`).** Prologue
matches modulo reloc slots; SEH state-byte writes match exactly (just
shifted by ±0x10 bytes due to body length differences).

### Iteration #3 territory — register-allocator divergence

The remaining ~50 % byte mismatches are all **register-allocator
choices**. Side-by-side:

| Variable | orig | iter #2 |
|---|---|---|
| `this` | ESI | EBP |
| `InterlockedExchangeAdd` (long-lived) | EDI | EBX |
| `chunk_handle` (post-AcquireChunk) | EBX (then spilled to `[ESP+0x14]`) | (other) |
| `counter_ptr` (loop-hoisted `&m_field_a4`) | EBP | (other) |
| `pending_ptr` (loop-hoisted `this+0x3c`) | EBX (after chunk_handle dies) | (other) |

This cascades into hundreds of single-byte differences (every
`CALL EDI` vs `CALL EBX`, every `[ESI+N]` vs `[EBP+N]`, etc.).

To push iter #3 toward GREEN: nudge MSVC's regalloc to match orig's
ESI/EDI/EBX/EBP assignment. Likely needs experimentation with:
- **Local declaration order** — affects live-range start times.
- **Intermediate variable placement** — taking address of a local can
  force a stack spill.
- **Volatile spill of `chunk_handle`** — orig spills it to `[ESP+0x14]`
  right after AcquireChunk, freeing EBX for pending_ptr later. Adding
  `volatile` to chunk_handle (or otherwise forcing its address-take)
  may trigger the same spill in our code.
- **Function-pointer hoist hints** — declaring an explicit
  `static const auto add_fn = InterlockedExchangeAdd;` at function top
  might give the loader a more obvious live range.

Each experiment is one or two compile-and-diff cycles; expect 5-10
iterations to match.

## What's still needed in Ghidra GUI (for matching beyond regalloc)

To fully match `FUN_00cc6700`, the remaining open items are:

1. **Helper function signatures** for `FUN_00cc5db0` (268 B chunk-source
   acquire), `FUN_00cc5e40` (124 B release), `FUN_00cc6510` (343 B). The
   smallest one — `FUN_00cc6620` (71 B wait-for-ready spin) — is
   already matched GREEN in `InstallUnpackerHelpers.cpp`.
2. **Parent class layout details** beyond the inferred fields — what's
   at `m_field_40 + 0x60` and `m_field_40 + 0x2140`? Likely a nested
   counter struct in ChunkSource.
3. **The "alt" Utf8String at 0x00445cf0** — confirmed (2026-05-02) to be
   a COMDAT duplicate of `Sqex::Misc::Utf8String::Utf8String @ 0x00047260`
   (same 39-byte body). Not a structural blocker — both ctors map to the
   same source `Utf8String()`. The link-time address differs but
   compare.py wildcards reloc slots, so any `e8 rel32` matches.

All 6 kernel32 IAT entries used by Unpack are already resolved
(2026-05-02):
```
[0x00f3e148]  InterlockedExchange
[0x00f3e1a0]  InterlockedCompareExchange
[0x00f3e1a4]  InterlockedExchangeAdd
[0x00f3e1c8]  Sleep                  ← used in Unpack wait loop @ 0x154
[0x00f3e2cc]  InterlockedIncrement   ← used in FUN_008edbf0 @ 0xc52
[0x00f3e2d4]  SwitchToThread
```

The **structural decode in this document is the deliverable** — anyone
iterating on `FUN_00cc6700` can start from iteration #2 and focus
purely on the regalloc nudges above.
