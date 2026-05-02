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
// for the chunk-extraction pipeline. Translated from Ghidra decompile
// 2026-05-02; iteration #1.

#include "../../../include/sqex/Utf8String.h"

extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long *target, long add);
extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long ms);

extern "C" void __cdecl _invalid_parameter_noinfo(void);

// Empty-string placeholder global at .rdata 0x01110698.
extern void *PTR_01110698;

// Forward declaration of two unmatched helpers. Their actual types
// don't matter for byte matching — orig emits `e8 rel32` calls with
// reloc-wildcarded targets. We just need symbols MSVC can resolve.
//
// FUN_00047450 is __thiscall: takes (this, void *outbuf). We declare
// it as a static member of a stub class to give it the __thiscall
// convention without needing a "real" enclosing type.
class SubObjAt1cStub {
public:
    void Process(void *outbuf);                 // FUN_00047450 (__thiscall)
};
extern "C" void __cdecl FUN_00cc6510(int p1, int *p2_out);

// Already-matched callees that we call from Unpack. ChunkSource has
// extra fields (m_field_60, m_field_2140) that Unpack reads via
// `(*field_40)+offset` — exposed as public members.
class ChunkSource {
public:
    int  AcquireChunk(int *out_data);          // FUN_008c5db0
    void ReleaseChunk(int handle);             // FUN_008c5e40
    char m_pad[0x60];
    long m_field_60;                            // +0x60
    char m_pad_64[0x2140 - 0x60 - 4];
    long m_field_2140;                          // +0x2140
};

// Local PackRead. The matched class lives in src/ffxivgame/sqpack/
// — we re-declare just the surface Unpack uses.
class PackRead {
public:
    PackRead(unsigned data, unsigned size);    // FUN_00d42800
    ~PackRead();                                // FUN_00cc6670
    char ReadNext();                            // FUN_00d428b0
private:
    char m_padding[0x80];                       // approximate — only sizeof
                                                // matters for the local
                                                // stack alloc, not field
                                                // layout from this TU
};

class InstallUnpackerOuter {
public:
    void Unpack();
    void WaitForReady(int handle);              // FUN_008c6620

private:
    // Layout below is for THIS TU's view only. Other TUs (Resource
    // Queue.cpp, InstallUnpackerHelpers.cpp) have their own layouts
    // for the same physical bytes. Matching is byte-level — only
    // the offsets need to align.
    char m_pad_00[0x40];                         // 0x00..0x3f (covers
                                                  // m_resource interior)
    ChunkSource *m_field_40;                     // +0x40
    InstallUnpackerOuter *m_field_44;            // +0x44 — peer pointer
    char m_field_48[0x9c - 0x48];                // +0x48 — opaque sub-object
    char *m_field_9c;                            // +0x9c
    int  m_field_a0;                             // +0xa0
    int  m_field_a4;                             // +0xa4
    long m_field_a8;                             // +0xa8
};

// FUNCTION: ffxivgame 0x008c6700 — InstallUnpacker::Unpack (490 B)
//
// PENDING (iteration #1) — translated literally from Ghidra
// decompile 2026-05-02. Many open questions: reg allocation, SEH
// state-byte placement, exact stack-frame layout. Expect
// significant mismatches on the first pass — calibration target
// is "compiles + has the right call sequence", not GREEN.

void InstallUnpackerOuter::Unpack() {
    long state = InterlockedExchangeAdd(&m_field_40->m_field_60, 0);
    if (state == 4) return;
    long bail_flag = InterlockedExchangeAdd(&m_field_a8, 0);
    if (bail_flag != 0) return;

    int chunk_data;
    int chunk_handle = m_field_40->AcquireChunk(&chunk_data);
    if (chunk_handle == 0) return;

    Utf8String chunk_name;                      // FUN_00045cf0
    PackRead pack_reader((unsigned)chunk_handle, (unsigned)chunk_data);  // FUN_00d42800

    int *counter_ptr = &m_field_a4;
    char subobj_buf[0x58];                      // 88-byte subobject
    char *str_begin = 0;
    char *str_end = 0;

    do {
        ((SubObjAt1cStub *)&m_field_48)->Process(subobj_buf);
        FUN_00cc6510((int)&m_field_48, counter_ptr);

        long mul = InterlockedExchangeAdd(&m_field_40->m_field_2140, 0);
        *counter_ptr = *counter_ptr * mul;

        char *data;
        if (str_begin == 0 || str_end == str_begin) {
            data = (char *)&PTR_01110698;
        } else {
            data = str_begin;
            // Dead branch retained from orig — MSVC emits this even
            // though the if-test is unreachable given the outer
            // condition. Likely a `_assert`-style check baked into a
            // template instantiation.
            if (str_end == str_begin) {
                _invalid_parameter_noinfo();
                data = str_begin;
            }
        }
        m_field_9c = data;
        int len = 0;
        if (str_begin != 0) {
            len = (int)(str_end - str_begin);
        }
        m_field_a0 = len;

        m_field_44->WaitForReady((int)((char *)this + 0x38));

        long pending = InterlockedExchangeAdd((long *)((char *)this + 0x3c), 0);
        while (pending == 1) {
            Sleep(0);
            pending = InterlockedExchangeAdd((long *)((char *)this + 0x3c), 0);
        }
    } while (pack_reader.ReadNext() != 0
             && InterlockedExchangeAdd(&m_field_a8, 0) == 0);

    m_field_40->ReleaseChunk(chunk_handle);
    // ~PackRead, ~Utf8String run via SEH unwind on scope exit.
}
