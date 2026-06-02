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
// FUNCTION: ffxivgame 0x005e47bc — __stdcall MSVC C++ SEH exception filter
//
// This function is an SEH filter that identifies MSVC C++ exceptions in-flight.
// It takes a pointer to an EXCEPTION_RECORD* (i.e. EXCEPTION_RECORD**) as its
// only argument, checks three conditions, and if all pass calls terminate()
// (the in-binary terminate at RVA 0x005de45a).  It always returns 0
// (EXCEPTION_CONTINUE_SEARCH) — the terminate call is noreturn in practice,
// but is not declared as such here, so MSVC emits dead epilogue code afterward.
//
// Conditions checked:
//   1. ExceptionCode == 0xe06d7363  (MSVC C++ SEH exception code)
//   2. NumberParameters == 3
//   3. ExceptionInformation[0] is one of the recognised MSVC magic signatures:
//        0x19930520  (VC5/6 CXX exception info magic)
//        0x19930521  (VC7/8 CXX exception info magic)
//        0x19930522  (VC9+ CXX exception info magic)
//        0x01994000  (additional magic seen in this binary)
//
// EXCEPTION_RECORD offsets used:
//   +0x00  ExceptionCode      (DWORD)
//   +0x04  ExceptionFlags     (DWORD, not accessed)
//   +0x08  ChainedRecord ptr  (DWORD, not accessed)
//   +0x0c  ExceptionAddress   (DWORD, not accessed)
//   +0x10  NumberParameters   (DWORD, checked == 3)
//   +0x14  ExceptionInfo[0]   (DWORD, the MSVC magic)
//
// Calling convention: __stdcall — callee cleans 1 DWORD (ret 4).
//
// Asm (61 bytes = 0x3d):
//   8b 44 24 04              MOV EAX, [ESP+4]          ; ppExcRec
//   8b 00                    MOV EAX, [EAX]             ; p = *ppExcRec
//   81 38 63 73 6d e0        CMP [EAX], 0xe06d7363      ; ExceptionCode
//   75 2a                    JNZ end
//   83 78 10 03              CMP [EAX+0x10], 3          ; NumberParameters
//   75 24                    JNZ end
//   8b 40 14                 MOV EAX, [EAX+0x14]        ; magic
//   3d 20 05 93 19           CMP EAX, 0x19930520
//   74 15                    JZ  call_it
//   3d 21 05 93 19           CMP EAX, 0x19930521
//   74 0e                    JZ  call_it
//   3d 22 05 93 19           CMP EAX, 0x19930522
//   74 07                    JZ  call_it
//   3d 00 40 99 01           CMP EAX, 0x01994000
//   75 05                    JNZ end
// call_it:
//   e8 66 9c ff ff           CALL FUN_009de45a          ; terminate
// end:
//   33 c0                    XOR EAX, EAX
//   c2 04 00                 RET 4

struct ExcRecord {
    unsigned long  ExceptionCode;
    unsigned long  ExceptionFlags;
    void*          ChainedRecord;
    void*          ExceptionAddress;
    unsigned long  NumberParameters;
    unsigned long  ExceptionInfo[15];
};

extern "C" void FUN_009de45a();  // terminate — not declared noreturn

extern "C" int __stdcall FUN_009e47bc(ExcRecord** ppExcRec)
{
    ExcRecord* p = *ppExcRec;
    if (p->ExceptionCode == 0xe06d7363UL && p->NumberParameters == 3) {
        unsigned long magic = p->ExceptionInfo[0];
        if (magic == 0x19930520UL ||
            magic == 0x19930521UL ||
            magic == 0x19930522UL ||
            magic == 0x01994000UL) {
            FUN_009de45a();
        }
    }
    return 0;
}
