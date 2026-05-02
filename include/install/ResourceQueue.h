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
// Component::Install::ResourceQueue — single-producer / single-consumer
// ring queue with a 4-byte payload per slot. Used by InstallUnpacker
// (member at +0x38) to serialise enqueue operations between the
// installer's main thread and its worker pool.
//
// Layout recovered from `ResourceQueue::TryEnqueue` at RVA 0x004edbf0
// (see src/ffxivgame/install/ResourceQueue.cpp).
//
// State semantics (per-slot atomic at m_state_arr[i]):
//   0 = free   — producer may claim
//   1 = filled — consumer has not yet drained
//
// The consumer-side dequeue lives elsewhere (likely in
// `InstallUnpacker::Unpack`'s post-`Sleep` re-check path). Once that
// is matched the names here may need a second pass.

#ifndef METEOR_DECOMP_INSTALL_RESOURCEQUEUE_H
#define METEOR_DECOMP_INSTALL_RESOURCEQUEUE_H

class ResourceQueue {
public:
    // FUN_008edbf0 @ RVA 0x004edbf0. Returns 1 on successful enqueue, 0
    // when either the slot at the current cursor is still filled or the
    // CAS to advance the cursor lost a race.
    char TryEnqueue(int *value);

private:
    char  m_pad_00[0x04];
    int   m_capacity;        // +0x04 — ring size, used to wrap m_cursor
    char  m_pad_08[0x04];
    long  m_cursor;          // +0x0c — atomic round-robin write index
    long  m_counter;         // +0x10 — atomic, incremented per enqueue
    long *m_state_arr;       // +0x14 — int[m_capacity], slot states
    int  *m_data_arr;        // +0x18 — int[m_capacity], slot payloads
};

#endif // METEOR_DECOMP_INSTALL_RESOURCEQUEUE_H
