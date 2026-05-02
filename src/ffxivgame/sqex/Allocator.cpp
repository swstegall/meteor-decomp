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

// Win32 kernel32 imports — the "global function pointer" at
// .data 0x00f3e1a4 is InterlockedExchangeAdd. Not a custom mutex.
// __declspec(dllimport) makes MSVC emit `CALL [iat_entry]` instead
// of a direct `CALL rel32`, matching orig's indirect-call style.
extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long *target, long add);

// CRT free at RVA 0x005d1be9.
extern "C" void __cdecl free(void *p);

// Per-size-class slab descriptor at .data 0x01266dc0 (16 entries × 8 B).
// Ghidra named the address relative to a nearby RTTI typedesc — the
// actual array starts at 0x01266dc0. Field at +0 is the capacity used
// for the modulo + 2x-comparison; field at +4 is unknown (not used by
// Utf8StringFree; possibly element-size for Utf8StringAlloc).
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

// FUNCTION: ffxivgame 0x0004d350 — Sqex::Memory::SlabFree (105 B)
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
        int slab_cap = g_slab_descriptors[size_class].capacity;
        if (counter == slab_cap * 2) {
            InterlockedExchangeAdd(&g_slab_counters[size_class], -slab_cap);
        }
        g_freelist_buckets[size_class][counter % slab_cap] = data;
    }
}
