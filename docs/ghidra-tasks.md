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

| Address | Likely role |
|---|---|
| `0x00f3e1a4` | Function pointer — looks like a "mutex acquire / atomic-add" primitive. The CALL via `EBP` after `MOV EBP, [0x00f3e1a4]` is invoked twice with different signatures. |
| `0x01266dc0` | Array of 8-byte slab descriptors, indexed by `size_class * 8`. Field at offset 0 holds the slab capacity (used in modulo + comparison-with-2x). |
| `0x0132cec8` | Array of 4-byte pointers (probably `void **` per size class) — the free-list bucket array. |
| `0x0132cf1c` | Array of mutex objects, indexed by `size_class * 4`. Probably a small struct (4 B handle each). |
| `0x005d1be9` | CRT `free` (or a wrapper). Used as the size-class-0 fall-back. |

**What to retrieve from Ghidra:**

1. Decompiler output for `FUN_004d350` (Ghidra's auto-name).
2. RTTI / typeinfo strings near the globals listed above. The globals
   may belong to a `Sqex::Memory::SlabAllocator` or
   `CDev::Engine::Memory` family — typeinfo would give us names.
3. Field-level annotations on the slab descriptor: there's at least
   one int at offset 0; could be more.

### `Utf8StringAlloc` @ RVA `0x0004d500` (225 B)

The counterpart — allocates a buffer from the same slab pool. Uses
the same global tables at `0x01266dc0`, `0x0132cec8`, etc. plus
additional globals at `0x012ce6c4`, `0x012ce6c8`, `0x0132cef0`, etc.

**Signature** (from PackRead.cpp call site):
```
extern "C" void *Utf8StringAlloc(int alloc_class, int zero_fill, unsigned size);
```

The function is too long for hand-decode (225 B with multi-branch +
size-rounding logic). Ghidra's decompiler can recover it cleanly.

**What to retrieve:**

1. Decompiler output for `FUN_004d500`.
2. Identification of the slab-size table around offset `0x18-0x20`
   (the `LEA EDI, [ESI*4 + 0x012c6dc4]` etc. patterns).
3. Whether it allocates from a per-size-class pool or falls back to
   CRT `malloc` for size_class = 0.

### Once the above is retrieved

Both functions become matching candidates with:
- The named globals (`g_mutex_fn`, `g_slab_table`, `g_freelist`, etc.)
- A struct definition for the slab descriptor
- The CRT free / malloc identifications

Drop the names back into this file and the next session can write the
two matching templates as a separate `src/ffxivgame/sqex/Allocator.cpp`
file.
