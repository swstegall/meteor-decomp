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
// Component::Install::InstallUnpacker::Unpack — slot-2 virtual method
// (Thread::Run override). 490 B at RVA 0x008c6700. Worker entry point
// for the chunk-extraction pipeline.
//
// Iteration history:
//
//   #1 [PARTIAL — 41 % match modulo relocations, 218/428 of our 428 B
//                 vs orig's 490 B]
//      First-pass translation. Treated the loop body as if it filled a
//      separate stack-allocated subobj (`char subobj_buf[0x58];
//      char *str_begin, *str_end;`) and called a stub Process() on it.
//      Frame allocated at SUB ESP, 0x138 (0x58 over orig's 0xe0).
//
//   #2 [now] — STRUCTURAL FIX based on cross-referencing PackRead.cpp
//      and FUN_00447450's body:
//
//      KEY INSIGHT: the "stack subobj at [ESP+0x38]" is NOT a separate
//      local. It's `pack_reader.m_subobj` — a Utf8String embedded in
//      PackRead at offset +0x1c. Pack_reader lives at [ESP+0x1c], so
//      its m_subobj naturally lands at [ESP+0x38].
//
//      KEY INSIGHT #2: FUN_00447450 is `Utf8String::operator=` (verified
//      by walking its 60-byte body — copies m_data via memcpy, calls
//      Reserve, copies m_field_c + m_flag_10). So the call
//      `LEA ECX, [ESI+0x48]; LEA EDX, [ESP+0x38]; PUSH EDX; CALL 0x00447450`
//      is `this->m_field_48 = pack_reader.m_subobj;` — a Utf8String
//      copy assignment from the chunk reader's subobj into the
//      InstallUnpacker's m_field_48 (which is itself a Utf8String).
//
//      KEY INSIGHT #3: the `[ESP+0x90]` and `[ESP+0x94]` reads in the
//      loop body are `pack_reader.m_buffer` and `pack_reader.m_field78`
//      (PackRead+0x74 and +0x78 — the heap-buffer begin/end pointers
//      named in PackRead.cpp). Not separate `str_begin`/`str_end`
//      locals.
//
//      Removing the bogus subobj_buf/str_begin/str_end locals shrinks
//      the frame by 0x58 bytes (target: SUB ESP, 0xe0).
//
// Field-level layout of `this` used by Unpack (cross-validated against
// InstallUnpackerHelpers.cpp's WaitForReady which uses the same class):
//
//   +0x00  vtable
//   +0x04..+0x37  unknown
//   +0x38..+0x47  ResourceQueue m_resource (drives WaitForReady)
//   +0x40  ChunkSource *m_field_40  (overlaps m_resource — unclear yet
//                                    if this is a separate offset or
//                                    if ChunkSource* IS m_resource's
//                                    first field; modeled as separate)
//   +0x44  InstallUnpackerOuter *m_field_44 (peer ptr — receiver for
//                                             the WaitForReady thiscall)
//   +0x48  Utf8String m_field_48 (assigned from pack_reader.m_subobj
//                                  each iteration)
//   +0x9c  char *m_field_9c  (data ptr, set from pack_reader.m_buffer)
//   +0xa0  int   m_field_a0  (length, set from m_field78 - m_buffer)
//   +0xa4  int   m_field_a4  (counter — multiplier accumulator)
//   +0xa8  long  m_field_a8  (atomic bail flag)

#include "../../../include/sqex/Utf8String.h"

extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long *target, long add);
extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long ms);

// _invalid_parameter_noinfo: matched at 0x009d22b4 in this binary.
// Declared without `extern "C"` so MSVC presumes it can throw — that
// triggers C++ EH frame setup in our function. (With `extern "C"`,
// /EHsc treats it as nothrow and elides the EH frame.)
void _invalid_parameter_noinfo(void);

// Empty-string placeholder global at .rdata 0x01110698.
extern char *PTR_01110698;

// Helper functions called from the loop. NO `extern "C"` — see comment
// on _invalid_parameter_noinfo above. The exact call shape is what
// matters for byte matching; the names are wildcarded by relocations.
void FUN_00cc6510(Utf8String *str, int *counter);

// PackRead view for THIS TU. Public m_subobj/m_buffer/m_field78 so we
// can take their addresses. The matched PackRead in
// src/ffxivgame/sqpack/PackRead.cpp has the same physical layout but
// keeps these private — different TUs see different access modes,
// which is fine since the linker only cares about offsets.
class PackRead {
public:
    PackRead(unsigned data, unsigned size);     // FUN_00d42800 (132 B)
    ~PackRead();                                  // FUN_00cc6670 (110 B GREEN)
    char ReadNext();                              // FUN_00d428b0 (27 B GREEN)

    // Padding to put m_subobj at offset +0x1c (matches PackRead.cpp).
    // ChunkRead<u32,u32> base spans [+0x00, +0x1c).
    char m_chunk_read_base[0x1c];                 // +0x00..+0x1b
    Utf8String m_subobj;                          // +0x1c..+0x6f (84 B)
    char m_pad[0x74 - 0x1c - 0x54];               // +0x70..+0x73 (4 B padding)
    char *m_buffer;                               // +0x74 — heap chunk-data begin
    char *m_field78;                              // +0x78 — heap chunk-data end
    char *m_field7c;                              // +0x7c
};

// ChunkSource view for THIS TU — public m_field_60/m_field_2140 so the
// loop can probe them via InterlockedExchangeAdd.
class ChunkSource {
public:
    int  AcquireChunk(int *out_data);             // FUN_008c5db0
    void ReleaseChunk(int handle);                // FUN_008c5e40

    char m_pad_00[0x60];
    long m_field_60;                              // +0x60
    char m_pad_64[0x2140 - 0x60 - 4];
    long m_field_2140;                            // +0x2140
};

class InstallUnpackerOuter {
public:
    void Unpack();
    void WaitForReady(int handle);                // FUN_008c6620 (peer-on-this+0x38)

private:
    char m_pad_00[0x40];                          // +0x00..+0x3f (covers m_resource interior)
    ChunkSource *m_field_40;                      // +0x40
    InstallUnpackerOuter *m_field_44;             // +0x44 — peer pointer
    Utf8String m_field_48;                        // +0x48 — copied-into Utf8String
    char *m_field_9c;                             // +0x9c
    int  m_field_a0;                              // +0xa0
    int  m_field_a4;                              // +0xa4
    long m_field_a8;                              // +0xa8
};

// FUNCTION: ffxivgame 0x008c6700 — InstallUnpacker::Unpack (490 B)
//
// Producer-consumer worker. Spins waiting for chunks, processes each,
// signals release. See docs/install-unpacker.md for the high-level
// state machine.
//
// Prologue shape (orig, 26 B):
//   PUSH -1; PUSH offset SEH_handler; PUSH FS:[0]; SUB ESP, 0xe0
//   MOV EAX, [__security_cookie]; XOR EAX, ESP; MOV [ESP+0xdc], EAX
//   PUSH EBX; PUSH EBP; PUSH ESI; PUSH EDI
//   MOV EAX, [__security_cookie]; XOR EAX, ESP; PUSH EAX
//   LEA EAX, [ESP+0xf4]; MOV FS:[0], EAX
//   MOV EDI, [InterlockedExchangeAdd]   ; held in EDI throughout
//   MOV ESI, ECX                         ; ESI = this

void InstallUnpackerOuter::Unpack() {
    // ---- bail probes ----
    long state = InterlockedExchangeAdd(&m_field_40->m_field_60, 0);
    if (state == 4) return;
    long bail = InterlockedExchangeAdd(&m_field_a8, 0);
    if (bail != 0) return;

    // ---- acquire chunk ----
    int chunk_data;
    int chunk_handle = m_field_40->AcquireChunk(&chunk_data);
    if (chunk_handle == 0) return;

    // ---- per-chunk locals (these drive frame layout) ----
    // Order matters: pack_reader is declared SECOND so MSVC places it
    // at the LOWER stack address (closer to ESP), matching orig's
    // [ESP+0x1c] for pack_reader and [ESP+0x9c] for chunk_name.
    Utf8String chunk_name;                                // [ESP+0x9c]
    PackRead   pack_reader(chunk_handle, chunk_data);     // [ESP+0x1c]

    int *counter_ptr = &m_field_a4;
    long *pending_ptr = (long *)((char *)this + 0x3c);

    // ---- chunk-iteration loop ----
    do {
        // m_field_48 = pack_reader.m_subobj  (Utf8String operator=)
        m_field_48 = pack_reader.m_subobj;

        // Helper that processes the assigned string + writes to counter.
        FUN_00cc6510(&m_field_48, counter_ptr);

        // Multiplier from the chunk source's atomic counter at +0x2140.
        long mul = InterlockedExchangeAdd(&m_field_40->m_field_2140, 0);
        *counter_ptr = *counter_ptr * mul;

        // Compute the data ptr + length from pack_reader's heap buffer.
        char *begin = pack_reader.m_buffer;
        char *end   = pack_reader.m_field78;
        char *data;
        int   len;
        if (begin == 0) {
            data = PTR_01110698;
            len  = 0;
        } else if (end == begin) {
            data = PTR_01110698;
            len  = 0;
        } else {
            // Bounds check — orig calls _invalid_parameter_noinfo and
            // reloads begin/end if the diff is non-positive. Modeled as
            // a JA in orig (unsigned compare). The reload is preserved
            // because MSVC may spill begin/end across the call.
            unsigned diff = (unsigned)(end - begin);
            if (diff == 0 || diff > 0x7fffffffu) {
                _invalid_parameter_noinfo();
                begin = pack_reader.m_buffer;
                end   = pack_reader.m_field78;
            }
            data = begin;
            len  = (int)(end - begin);
        }
        m_field_9c = data;
        m_field_a0 = len;

        // Tell the peer to wait until our resource flag is set.
        m_field_44->WaitForReady((int)((char *)this + 0x38));

        // Sleep-wait while pending == 1.
        long pending = InterlockedExchangeAdd(pending_ptr, 0);
        while (pending == 1) {
            Sleep(0);
            pending = InterlockedExchangeAdd(pending_ptr, 0);
        }
    } while (pack_reader.ReadNext()
             && InterlockedExchangeAdd(&m_field_a8, 0) == 0);

    // ---- release chunk ----
    m_field_40->ReleaseChunk(chunk_handle);

    // ~PackRead and ~Utf8String run via SEH unwind on scope exit.
    // (Implicit; compiler-generated.)
}
