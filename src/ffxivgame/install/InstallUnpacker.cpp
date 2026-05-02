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
// for the chunk-extraction pipeline. See docs/install-unpacker.md for
// the structural recon trail.
//
// Match strategy: this is a multi-iteration target. The body has a
// large SEH frame (0xe0 byte locals + cookie save), 11 distinct call
// targets (10 of them now matched), a stack-allocated Utf8String at
// [ESP+0x9c] AND a stack-allocated PackRead at [ESP+0x24], an
// EDI-hoisted InterlockedExchangeAdd, and 3 bail paths to teardown.
// See the accompanying scaffolding below.

#include "../../../include/sqex/Utf8String.h"
#include "../../../include/install/ResourceQueue.h"

extern "C" __declspec(dllimport) long __stdcall InterlockedExchangeAdd(long *target, long add);
extern "C" __declspec(dllimport) void __stdcall Sleep(unsigned long ms);

// Forward declarations of all 11 callees in offset order.
//
// Already matched / declared:
class ChunkSource {
public:
    int  AcquireChunk(int *out_data);          // FUN_008c5db0
    void ReleaseChunk(int handle);             // FUN_008c5e40
};

// PackRead has its own header in the sqpack tree; redeclare the
// surface InstallUnpacker::Unpack uses.
class PackRead {
public:
    PackRead(const void *data, unsigned size); // FUN_00d42800
    ~PackRead();                               // FUN_00cc6670
    int ReadNext();                            // FUN_00d428b0
};

// Unmatched — declarations-only for the relocs.
extern "C" void __cdecl FUN_00047450(void *p);     // "SubObjAt1c::Process"
                                                    // (also called from
                                                    // PackRead::ProcessChunk)
class InstallUnpackerHelper {
public:
    void ConfigParser(int *out);  // FUN_00cc6510 — 343 B Utf8String
                                  // config-line parser
};

extern "C" void __cdecl _invalid_parameter_noinfo(void);

class InstallUnpackerOuter {
public:
    void Unpack();                                // FUN_00cc6700 — THIS
private:
    char m_pad_00[0x38];
    ResourceQueue m_resource;                     // +0x38
    char m_pad_after_rq[/* tbd */];
    void *m_field_40;                             // ChunkSource ptr
    char m_pad_44[0x60];                          // up to +0xa4
    void *m_field_a4;                             // ConfigParser ptr
    char m_field_a8[/* tbd */];                   // probed at start
    /* ... more fields up to size derived from Unpack ... */
};

// FUNCTION: ffxivgame 0x008c6700 — InstallUnpacker::Unpack (490 B)
//
// PENDING — needs Ghidra decompile to translate. Will be added in
// a subsequent iteration once the structural decompile is on hand.
//
// Stack frame layout (recovered from byte-level analysis):
//   SUB ESP, 0xe0    ; 224 bytes of locals
//   PUSH EBX, EBP, ESI, EDI    ; 16 more bytes of saves
//   PUSH cookie XOR             ; 4 more bytes
//
//   Local Utf8String at [ESP+0x9c] (after pushes)
//   Local PackRead at [ESP+0x24]
//   Local handle at [ESP+0x18] (out param of AcquireChunk)
//   SEH state byte at [ESP+0xfc] (set 0x00 → 0x01 around PackRead ctor)
//   Various locals at [ESP+0x90], [ESP+0x94], [ESP+0xa0]
//
// Bail paths to teardown:
//   - [this+0x40].state == 4   (offset 0x4e jumps to 0x166)
//   - [this+0xa8] != 0          (offset 0x62 jumps to 0x153)
//   - AcquireChunk returns 0    (offset 0x79 jumps to 0x134)
//
// Inner loop (offset ~0xc8 to ~0x180):
//   FUN_00047450(&[ESP+0x38])
//   InstallUnpackerHelper::ConfigParser(&[ESI+0xa4])  → FUN_00cc6510
//   InterlockedExchangeAdd on a counter
//   _invalid_parameter_noinfo()  (size_check_cookie spillover)
//   WaitForReady(&[ESI+0x38])
//   InterlockedExchangeAdd
//   Sleep(0)  (yield)
//   InterlockedExchangeAdd
//   PackRead::ReadNext()
//   InterlockedExchangeAdd
//   loop continuation check (offset 0x183 jumps back to 0xc4)
//
// Teardown:
//   ReleaseChunk(handle)
//   PackRead::~PackRead
//   Utf8String::~Utf8String
//   SEH unwind
//   __security_check_cookie
//   ADD ESP, 0xec; RET
