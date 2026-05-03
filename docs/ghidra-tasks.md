# Ghidra GUI work items — Phase 4 allocator helpers

This document captures specific Ghidra GUI tasks that would unblock
further matching in `meteor-decomp`. Each item lists the function /
data location, what to look at, and what we need back to write a
matching template.

When the GUI session is done, drop the recovered names + struct shapes
into this file and a future session can write the matchers.

---

## Status snapshot — 2026-05-02

All Phase 4 IAT entries identified. Worker-pipeline allocator + chunk-
queue subsystem mapped. Recent identifications:

- `[0x00f3e148]`  `InterlockedExchange`           ✓
- `[0x00f3e1a0]`  `InterlockedCompareExchange`    ✓ (2026-05-01)
- `[0x00f3e1a4]`  `InterlockedExchangeAdd`        ✓
- `[0x00f3e1c8]`  `Sleep`                         ✓ (2026-05-02)
- `[0x00f3e2cc]`  `InterlockedIncrement`          ✓ (2026-05-02)
- `[0x00f3e2d4]`  `SwitchToThread`                ✓

Phase 4 matches landed: 6 GREEN (PackRead ctor/ReadNext/Rewind/dtor,
Utf8String dtor, WaitForReady) + 4 PARTIAL @ 99% (ProcessChunk,
SlabFree, SlabAlloc, ReleaseChunk, AcquireChunk) + Reserve PARTIAL
@ 94%. Next blocker: `FUN_008edbf0` (122 B `WaitablePredicate::TryReady`)
— now writable since all IAT entries known.

---

## Phase 4 — Sqpack / Utf8String allocator pair

The Sqex::Misc::Utf8String class delegates allocation to two cdecl
helper functions that index a global slab-allocator table. Both
functions use bit-shifting and global-array indexing that's much
clearer in Ghidra's decompiler view than in raw asm.

### `Utf8StringFree` @ RVA `0x0004d350` (105 B)

**Signature** (recovered from PackRead.cpp's call sites):
```
extern "C" void Utf8StringFree(void *data, int capacity, int alloc_class);
```

The `alloc_class` arg (always `0xb` in the only-observed call sites) is
not actually read by this function — it reads the size-class byte from
the **memory header at `data - 4`** instead.

**Hand-decoded structural shape:**

```c
void Utf8StringFree(void *data, int capacity, int alloc_class) {
    if (!data) return;
    char *header = (char *)data - 4;
    unsigned size_class = *(unsigned *)header & 0xff;

    if (size_class == 0) {
        // Fast path: CRT free()
        free_crt(header);  // RVA 0x005d1be9
        return;
    }

    // Slow path: pooled slab free with mutex.
    void *mutex = &g_mutex_table[size_class];          // 0x0132cf1c
    void (*acquire)(...) = g_mutex_fn_ptr;              // [0x00f3e1a4]

    int handle = acquire(mutex, 1);
    int slab_size = g_slab_table[size_class].field0;    // [0x01266dc0 + 8*sc]
    if (handle == 2 * slab_size) {
        acquire(mutex, -slab_size);
    }
    int hash_index = handle % slab_size;                // signed div
    void **freelist = g_freelist[size_class];           // [0x0132cec8 + 4*sc]
    freelist[hash_index] = data;
}
```

**Globals referenced (each needs a name + struct from Ghidra):**

| Address | Role |
|---|---|
| `0x00f3e1a4` | IAT entry → `kernel32!InterlockedExchangeAdd` (confirmed). |
| `0x01266dc0` | **NOT a struct base** — it's the `imm32` literal in `MOV EAX, [ESI*8 + 0x01266dc0]`. The actual slab descriptor table starts at `0x01266dc8` (= base + size_class=1 × 8). The bytes at `0x01266dc0..0x01266dc7` happen to overlap the trailing `@@\0\0\0\0` of the `.?AVSqexIdAuthentication@Login@Sqex@@` RTTI TypeDescriptor name string by linker placement, NOT by struct overlap — the size_class=0 fast-path short-circuits before any read at this offset. Don't waste time looking for typeinfo at `0x01266dc0`; look at `0x01266dc8` for the first real slab descriptor (size_class=1). (Confirmed in Ghidra GUI 2026-05-02.) |
| `0x0132cec8` | Per-size-class free-list bucket pointer array (`int **`), indexed by `[size_class*4]`. |
| `0x0132cf1c` | Per-size-class atomic counter array (`long`), indexed by `[size_class*4]`. Targets of `InterlockedExchangeAdd`. |
| `0x005d1be9` | CRT `free` — the `size_class == 0` fast-path fallback. |

**Decoded body of FUN_0044d350 (Utf8StringFree, 105 B at file 0x4d350):**

```
arg = data ptr (cdecl); only the dword header at (data - 4) controls behavior.

if (data == NULL) return;
hdr = *(unsigned *)(data - 4);
size_class = hdr & 0xff;
if (size_class == 0) {
    free(data - 4);                                         // CRT fast path
    return;
}
// Slow path — atomic-counter-based circular freelist push.
counter = InterlockedExchangeAdd(&g_atomic_counters[size_class], 1);
slab_cap = g_slab_descriptors_minus8[size_class].capacity;  // [ESI*8 + 0x01266dc0]
                                                            // (== g_slab_descriptors[size_class-1].capacity
                                                            //  if you re-base; either way size_class>=1
                                                            //  reads valid memory)
if (counter == slab_cap * 2) {
    InterlockedExchangeAdd(&g_atomic_counters[size_class], -slab_cap);
}
g_freelist_buckets[size_class][counter % slab_cap] = data;
```

**Status:** `src/ffxivgame/sqex/Allocator.cpp` already implements this
shape and is at 99 % PARTIAL (104/105 B, 1 byte short — likely a
single-instruction-encoding choice difference).

**What would be useful from Ghidra (revised after 0x01266dc0 finding):**

1. **Confirm the size_class-1 base**: navigate to `0x01266dc8` in the
   data view, check whether Ghidra has it auto-typed as a struct. If
   so, capture the struct definition (likely 8 bytes per entry —
   capacity `int` + something at +4).
2. **Confirm the auxiliary-array bases used in `Utf8StringAlloc`**
   (separate from Free's tables): the alloc path uses `0x01266dc4`
   (size thresholds), `0x0132cf04` (producer counters), `0x0132cf20`
   (consumer counters), `0x0132cecc` (alloc freelist pointers). All
   are `[size_class*N + base]` indexed; the same RTTI-overlap caveat
   applies to bases that fall inside data sections.
3. **Decompiler output for `FUN_0044d500`** (Utf8StringAlloc) — at
   99 % PARTIAL too; if the decompile shows a structural choice we
   missed, that may close the last few bytes.

### `Utf8StringAlloc` @ RVA `0x0004d500` (225 B)

The counterpart — allocates a buffer from the same slab pool. Already
implemented in `src/ffxivgame/sqex/Allocator.cpp` and at 99 % PARTIAL
(222/225 B). Uses parallel arrays:

| Address | Role |
|---|---|
| `0x01266dc4` | Size-threshold table — `[ESI*8 + 0x01266dc4]` per size_class. (Same RTTI-overlap caveat as 0x01266dc0; the array starts at the post-RTTI region for valid size_class values.) |
| `0x01266dc8` | Capacity table (shared with Free's slab descriptors — `g_slab_descriptors[size_class].capacity`). |
| `0x0132cf04` | Producer counter array (`long`), targets of `InterlockedExchangeAdd`. |
| `0x0132cf20` | Consumer counter array (`long`). |
| `0x0132cecc` | Alloc freelist pointer array (`int **`). |

The `(int alloc_class, int zero_fill, unsigned size)` signature in the
PackRead.cpp call-site comment was wrong — the actual signature is
`void *Utf8StringAlloc(int size)` (only one arg). Callers that push
two extra args (PUSH 0; PUSH alloc_class) are passing dummies that the
function ignores.

### Allocator pair status

Both functions already implemented at 99 % PARTIAL. Closing the last
1-3 bytes per function is a single-instruction-encoding nudge — try
re-ordering temporaries, swapping `volatile`-vs-non-volatile loads,
or re-shaping the modulo expression. Ghidra GUI helps if it shows the
exact MOVZX/AND choice for the size-class extraction or the IMUL/SHL
choice for capacity-times-2.
