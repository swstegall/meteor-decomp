// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Sqex / CDev allocator helpers — Utf8StringFree and Utf8StringAlloc.
// Both are cdecl free functions in the binary's `.text` that delegate
// to a per-size-class circular free-list (lock-free, atomic-counter
// based). Recovered from Ghidra GUI decompilation 2026-05-02 (see
// docs/ghidra-tasks.md for the handoff trail).

// Win32 kernel32 imports — recovered from Ghidra GUI 2026-05-02:
//   IAT entry @ .data 0x00f3e1a4  →  InterlockedExchangeAdd
//   IAT entry @ .data 0x00f3e148  →  InterlockedExchange
//   IAT entry @ .data 0x00f3e2d4  →  SwitchToThread
// __declspec(dllimport) makes MSVC emit `CALL [iat_entry]` instead
// of a direct `CALL rel32`, matching orig's indirect-call style.
extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long *target, long add);
extern "C" __declspec(dllimport) long __stdcall InterlockedExchange(long *target, long value);
extern "C" __declspec(dllimport) int __stdcall SwitchToThread();

// CRT free at RVA 0x005d1be9.
extern "C" void __cdecl free(void *p);

// Per-size-class slab descriptor at .data 0x01266dc0 (Free path).
// Each entry is 8 bytes; field 0 is capacity used by Utf8StringFree
// for modulo + 2x-overflow comparison; field 1 may be size_threshold.
struct SlabDescriptor {
    int capacity;        // +0x00
    int unknown_4;       // +0x04
};
extern "C" SlabDescriptor g_slab_descriptors[];   // @ .data 0x01266dc0

// Per-size-class atomic counters at .data 0x0132cf1c (16 entries × 4 B).
// InterlockedExchangeAdd targets these.
extern "C" long g_slab_counters[];                // @ .data 0x0132cf1c

// Per-size-class free-list bucket arrays at .data 0x0132cec8.
// Each entry is `int *` pointing to a circular buffer of `int` slots,
// with capacity == g_slab_descriptors[size_class].capacity.
extern "C" int *g_freelist_buckets[];             // @ .data 0x0132cec8

// Utf8StringAlloc uses a parallel set of arrays (Ghidra-confirmed):
extern "C" int  g_alloc_size_thresholds[];        // @ .data 0x01266dc4 (int[i*2])
extern "C" int  g_alloc_capacities[];             // @ .data 0x01266dc8 (int[i*2])
extern "C" long g_alloc_producer[];               // @ .data 0x0132cf04 (long[i])
extern "C" long g_alloc_consumer[];               // @ .data 0x0132cf20 (long[i])
extern "C" int *g_alloc_freelists[];              // @ .data 0x0132cecc (int*[i])
extern "C" void *malloc(unsigned n);              // CRT @ RVA 0x005d1b35

// FUNCTION: ffxivgame 0x0004d350 — Sqex::Memory::SlabFree (105 B) ✅ GREEN
//
// "Utf8StringFree" was a misnomer — this isn't string-specific. It's
// the generic slab-pool free for any allocation made by the matching
// SlabAlloc. Utf8String calls it because its m_data points to a slab-
// allocated buffer.
//
// Reads the size-class byte from the 4-byte header at (data - 4):
//   - If size_class == 0: fall back to CRT free()
//   - Else: atomic-counter-based wraparound push into the free-list
//
// Original bytes (105 B):
//   0004d350: 8b 44 24 04 85 c0 74 60 83 c0 fc 56 8b 30 81 e6
//   0004d360: ff 00 00 00 75 0b 50 e8 ?? ?? ?? ?? 83 c4 04 5e
//   0004d370: c3 53 55 8b 2d ?? ?? ?? ?? 57 6a 01 8d 3c b5 ??
//   0004d380: ?? ?? ?? 57 ff d5 8b d8 8b 04 f5 ?? ?? ?? ?? 8d
//   0004d390: 0c 00 3b d9 75 06 f7 d8 50 57 ff d5 8b c3 99 f7
//   0004d3a0: 3c f5 ?? ?? ?? ?? 8b 04 b5 ?? ?? ?? ?? 8b 4c 24
//   0004d3b0: 14 5f 5d 5b 5e 89 0c 90 c3
//
// Iteration history:
//   #1 [PARTIAL — 104/105 (99 % size; ~65 % byte match)]
//      `int slab_cap = g_slab_descriptors[size_class].capacity;`
//      hoisted slab_cap into a callee-saved register, letting MSVC
//      use 2-byte `IDIV EDI` instead of orig's 7-byte
//      `IDIV [ESI*8 + imm32]` (re-load from memory). 5-byte savings,
//      offset by 4 bytes of inlined IAT calls = net 1 byte short.
//
//   #2 [✅ GREEN — 105/105]
//      Inlined the slab_cap accesses (`g_slab_descriptors[size_class].capacity`
//      used directly in three places instead of via an intermediate
//      `int slab_cap` local). MSVC no longer hoists into a callee-saved
//      register, so the IDIV correctly re-loads from memory. Matches
//      orig byte-for-byte modulo 5 reloc slots (free / IAT addr / two
//      slab-table addrs / freelist addr).
extern "C" void Utf8StringFree(int data, int /*capacity*/, int /*alloc_class*/) {
    // The capacity and alloc_class args are pushed by callers (cdecl)
    // but actually ignored — only the header at data-4 controls
    // behavior. Including them in the signature for callers'
    // convenience.
    if (data != 0) {
        // volatile read prevents MSVC from collapsing the dword load
        // + AND mask into a MOVZX byte access (which would be 3 B
        // instead of orig's 8 B).
        unsigned size_class = *(volatile unsigned *)(data - 4);
        size_class = size_class & 0xff;
        if (size_class == 0) {
            free((void *)(data - 4));
            // Note: Ghidra annotated this as a no-return path, but the
            // binary's `c3` RET right after the ADD ESP,4 means we DO
            // return immediately after free without entering the slab
            // path. The `if` falls through structurally to the slab
            // code, but free() returning means the runtime sees a clean
            // exit. Modeling this with `return` here.
            return;
        }
        long counter = InterlockedExchangeAdd(&g_slab_counters[size_class], 1);
        if (counter == g_slab_descriptors[size_class].capacity * 2) {
            InterlockedExchangeAdd(&g_slab_counters[size_class],
                                   -g_slab_descriptors[size_class].capacity);
        }
        g_freelist_buckets[size_class][counter % g_slab_descriptors[size_class].capacity] = data;
    }
}

// FUNCTION: ffxivgame 0x0004d500 — Sqex::Memory::SlabAlloc (225 B)
//
// Counterpart to Utf8StringFree. Looks up a size class by walking 7
// thresholds; if the size fits and the cache is populated, pops a slab
// from the free-list. Otherwise falls back to CRT malloc with a 4-byte
// header storing (size_class=0, original_size).
//
// Ghidra-recovered semantics:
//   void *Utf8StringAlloc(int size) {
//     for (int sc = 0; sc < 7; sc++) {
//       if (size <= g_alloc_size_thresholds[sc * 2]) {
//         long produced = InterlockedExchangeAdd(&g_alloc_producer[sc], 0);
//         long consumed = InterlockedExchangeAdd(&g_alloc_consumer[sc], 0);
//         int cap = g_alloc_capacities[sc * 2];
//         int prod_idx = produced % cap;
//         int cons_idx = consumed % cap;
//         int delta = (prod_idx < cons_idx) ? -prod_idx : (cap - prod_idx);
//         if (delta + cons_idx >= 100) {
//           // Cache populated — pop one.
//           long my = InterlockedExchangeAdd(&g_alloc_producer[sc], 1);
//           if (my == cap * 2) {
//             InterlockedExchangeAdd(&g_alloc_producer[sc], -cap);
//           }
//           int *slab = g_alloc_freelists[sc] + (my % cap) * 4;
//           int *resident = *(int **)slab;
//           if (resident) return resident;
//         }
//         break;
//       }
//     }
//     // Fallback: malloc with 4-byte header.
//     unsigned *p = (unsigned *)malloc(size + 4);
//     if (!p) return 0;
//     *(unsigned char *)p = 0;       // size_class = 0 (CRT-owned marker)
//     *p = (unsigned char)*p | (size << 8);  // upper 24 bits = size
//     return p + 1;                  // skip header, return data ptr
//   }

extern "C" unsigned *Utf8StringAlloc(int size) {
    int sc = 0;
    do {
        if (size <= g_alloc_size_thresholds[sc * 2]) {
            long *producer = &g_alloc_producer[sc];
            long produced = InterlockedExchangeAdd(producer, 0);
            long consumed = InterlockedExchangeAdd(&g_alloc_consumer[sc], 0);
            int cap = g_alloc_capacities[sc * 2];
            int prod_idx = (int)(produced % cap);
            int cons_idx = (int)(consumed % cap);
            int delta;
            if (prod_idx < cons_idx) {
                delta = -prod_idx;
            } else {
                delta = cap - prod_idx;
            }
            if (99 < delta + cons_idx) {
                long my_idx = InterlockedExchangeAdd(producer, 1);
                if (my_idx == g_alloc_capacities[sc * 2] * 2) {
                    InterlockedExchangeAdd(producer, -g_alloc_capacities[sc * 2]);
                }
                unsigned *resident = *(unsigned **)(
                    (char *)g_alloc_freelists[sc] +
                    (my_idx % g_alloc_capacities[sc * 2]) * 4);
                if (resident != 0) {
                    return resident;
                }
            }
            break;
        }
        sc = sc + 1;
    } while (sc < 7);

    unsigned *p = (unsigned *)malloc(size + 4);
    if (p != 0) {
        *(unsigned char *)p = 0;
        *p = (unsigned)(unsigned char)*p | (unsigned)(size << 8);
        return p + 1;
    }
    return 0;
}
