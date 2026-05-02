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
// Component::Install::ResourceQueue::TryEnqueue — producer-side
// claim-and-write for the ring queue at InstallUnpacker.m_resource.
//
// Lives in its own translation unit so MSVC can't inline it into
// callers (notably InstallUnpacker::WaitForReady, whose GREEN match
// at FUN_00cc6620 depends on `CALL FUN_008edbf0` being a real call).

#include "../../../include/install/ResourceQueue.h"

extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long *target, long add);
extern "C" __declspec(dllimport) long __stdcall InterlockedExchange(long *target, long value);
extern "C" __declspec(dllimport) long __stdcall InterlockedCompareExchange(long *target, long exch, long cmp);
extern "C" __declspec(dllimport) long __stdcall InterlockedIncrement(long *target);

// FUNCTION: ffxivgame 0x004edbf0 — ResourceQueue::TryEnqueue (122 B)
//
// Recovered from raw bytes 2026-05-02 (122 bytes at file offset
// 0x004edbf0; all five IAT entries identified the same day).
//
// __thiscall, RET 4 (one int* arg). Saves all four callee-preserved
// regs (EBX/EBP/ESI/EDI) and hoists `&m_cursor` into EBP.
//
// State machine:
//   1. Read m_cursor atomically.
//   2. Read m_state_arr[cursor] atomically.
//   3. If state == 0 (slot free):
//      a. Compute next = (cursor+1) % m_capacity.
//      b. CAS m_cursor: cursor → next.
//      c. On CAS success:
//           m_data_arr[cursor] = *value;
//           m_state_arr[cursor] = 1;     (atomic exchange)
//           ++m_counter;                  (atomic increment)
//         Return true.
//   4. Otherwise return false (caller spins via SwitchToThread).

char ResourceQueue::TryEnqueue(int *value) {
    long *cursor_addr = &m_cursor;
    long cursor = InterlockedExchangeAdd(cursor_addr, 0);
    int byte_off = cursor * 4;

    // Hoist the state-slot address into a local — encourages MSVC to
    // compute (m_state_arr + byte_off) BEFORE pushing the second arg,
    // matching orig's `ADD EAX, EBX; PUSH 0; PUSH EAX` ordering.
    long *state_slot = (long *)((char *)m_state_arr + byte_off);
    long entry_state = InterlockedExchangeAdd(state_slot, 0);
    if (entry_state == 0) {
        long next = cursor + 1;
        // `next >= m_capacity` keeps `next` on the LHS of the cmp so
        // MSVC emits `CMP EAX, [ESI+4]; JL +2; XOR EAX, EAX` rather
        // than the swapped `CMP mem, reg; JG`.
        if ((int)next >= m_capacity) next = 0;
        long swapped = InterlockedCompareExchange(cursor_addr, next, cursor);
        if (swapped == cursor) {
            *(int *)((char *)m_data_arr + byte_off) = *value;
            InterlockedExchange(
                (long *)((char *)m_state_arr + byte_off), 1);
            InterlockedIncrement(&m_counter);
            return 1;
        }
    }
    return 0;
}
