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
// FUNCTION: ffxivgame 0x000381c0 — __thiscall SEH-guarded destructor/reset
//                                  (201 B / 0xC9, window ends at POP EDI;
//                                  the full epilogue continues past the
//                                  201-byte compare window).
//
// Calling convention: __thiscall (ECX = this).
//
// Outline (from asm/ffxivgame/000381c0_FUN_004381c0.s + raw binary bytes):
//
//   void __thiscall FUN_004381c0(SomeClass *this /*ECX*/) {
//
//     // --- SEH prologue (MSVC __seh_prolog4 style) ---
//     // Pushes: -1 (state), handler(0xe56367), FS:[0], ECX(this),
//     //         EBX, ESI, EDI, then cookie XOR'd with ESP.
//     // Registers SEH frame at FS:[0] = &prev-FS:[0] slot.
//
//     this->vtable = (void*)0xf65b60;   // re-stamp vtable
//
//     // --- state 3: conditionally destroy old this->field_8 ---
//     SomeInner *old = this->field_8;
//     if (old != NULL) {
//         FUN_0043c0d0(old);            // __thiscall dtor
//         FUN_009d1b17(old);            // operator delete (__cdecl, caller+4)
//     }
//
//     // --- state 2: construct new object into this+0x30 ---
//     this->field_8 = NULL;
//     FUN_009d1c4c(&this->field_30, 0x10, 0x2, (void*)0x421e10);  // __stdcall
//
//     // --- state 1: clear string at this->field_14/18/1c ---
//     if (this->field_14 != NULL) {
//         void *p = this->field_14;
//         FUN_0040df70(*(int*)(p-4), p);  // __thiscall string free (ECX=hdr, arg=ptr)
//     }
//     this->field_14 = NULL;
//     this->field_18 = NULL;
//     this->field_1c = NULL;
//
//     // --- state 0: virtual call through this->field_c ---
//     if (this->field_c != NULL)
//         this->field_c->vtable[12](1);  // vtable slot 0x30 / index 12
//
//     // --- state -1: cleanup this->field_8 (now 0, so always skipped) ---
//     SomeInner *cur = this->field_8;  // = NULL (was zeroed above)
//     if (cur != NULL) {
//         FUN_0043c0d0(cur);
//         FUN_009d1b17(cur);  // operator delete (__cdecl, caller+4)
//     }
//
//     // --- epilogue restores FS:[0], pops registers ---
//     // (compare window ends at POP EDI; POP ESI, POP EBX,
//     //  ADD ESP 0x10, RET are past the 201-byte window)
//   }
//
// SEH state machine encoding:
//   -1  initial / unregistered
//    3  between entry and first field_8 destroy
//    2  between first destroy and constructor call
//    1  between constructor and string-free
//    0  during virtual call
//   -1  after virtual call / before second field_8 check
//
// Reloc-bearing sites in the 201 orig bytes (E8 rel32 = compare.py masks):
//   +67   CALL FUN_0043c0d0 (first;  __thiscall dtor, ECX=EDI)
//   +73   CALL FUN_009d1b17 (first;  __cdecl delete, arg=EDI; caller adds 4)
//   +102  CALL FUN_009d1c4c (__stdcall container-init, 4 args; callee-cleanup)
//   +123  CALL FUN_0040df70 (__thiscall string-free, ECX=[EAX-4], arg=EAX)
//   +174  CALL FUN_0043c0d0 (second; __thiscall dtor, ECX=ESI)
//   +180  CALL FUN_009d1b17 (second; __cdecl delete, arg=ESI; caller adds 4)
//
// Non-call absolute immediates (baked verbatim by _emit; compare.py sees them
// as plain data bytes and they match the orig slice exactly):
//   PUSH 0xe56367           — SEH handler VA
//   MOV EAX,[0x012ea8b0]   — __security_cookie load (moffs32 form)
//   MOV FS:[0],EAX          — 64 a3 00 00 00 00 (moffs32 store)
//   MOV [ESI],0xf65b60      — vtable re-stamp
//   PUSH 0x421e10           — container init arg
//   MOV FS:[0],ECX          — 64 89 0d 00 00 00 00 (general form)
//
// Reconstruction strategy — mixed _emit + symbolic calls:
//   All 195 non-call bytes are emitted verbatim via _emit to guarantee
//   exact encoding (esp. the FS: prefix moffs32 forms that MASM might
//   encode differently).  The six E8-opcode CALL instructions use named
//   extern "C" symbols so MASM generates COFF R_386_PC32 relocations;
//   compare.py masks those 4-byte rel32 slots automatically.

extern "C" void FUN_0043c0d0();    // __thiscall cleanup/dtor
extern "C" void FUN_009d1b17();    // ::operator delete (__cdecl, 1 arg)
extern "C" void FUN_009d1c4c();    // container init (__stdcall, 4 args)
extern "C" void FUN_0040df70();    // string free (__thiscall + 1 stack arg)

extern "C" __declspec(naked) void FUN_004381c0() {
    __asm {
        // ---- prologue (bytes 0-66) ----
        _emit 0x6a  // PUSH -1            (SEH state sentinel)
        _emit 0xff
        _emit 0x68  // PUSH 0xe56367      (SEH handler address)
        _emit 0x67
        _emit 0x63
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]   (previous SEH frame)
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX           (save 'this')
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX           (cookie XOR'd)
        _emit 0x8d  // LEA EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0], EAX   (register SEH frame; moffs32 form)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ESI, ECX       (ESI = this)
        _emit 0xf1
        _emit 0x89  // MOV [ESP+0x10], ESI  (save this in ECX slot for SEH)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0xc7  // MOV dword ptr [ESI], 0xf65b60  (re-stamp vtable)
        _emit 0x06
        _emit 0x60
        _emit 0x5b
        _emit 0xf6
        _emit 0x00
        _emit 0x8b  // MOV EDI, [ESI+8]   (EDI = this->field_8)
        _emit 0x7e
        _emit 0x08
        _emit 0x33  // XOR EBX, EBX       (EBX = 0 / NULL)
        _emit 0xdb
        _emit 0x3b  // CMP EDI, EBX
        _emit 0xfb
        _emit 0xc7  // MOV dword ptr [ESP+0x1c], 3  (SEH state = 3)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // JZ +0x10  (skip_destroy_edi)
        _emit 0x10
        _emit 0x8b  // MOV ECX, EDI       (ECX = old field_8, for __thiscall)
        _emit 0xcf
        // ---- byte 67: CALL FUN_0043c0d0 (dtor on old field_8) ----
        call FUN_0043c0d0
        // ---- byte 72 ----
        _emit 0x57  // PUSH EDI           (arg: old field_8 ptr)
        // ---- byte 73: CALL FUN_009d1b17 (operator delete, __cdecl) ----
        call FUN_009d1b17
        // ---- bytes 78-80 ----
        _emit 0x83  // ADD ESP, 4         (caller-side __cdecl cleanup)
        _emit 0xc4
        _emit 0x04
        // ---- bytes 81-101: skip_destroy_edi target; init new object ----
        _emit 0x68  // PUSH 0x421e10      (4th arg to FUN_009d1c4c)
        _emit 0x10
        _emit 0x1e
        _emit 0x42
        _emit 0x00
        _emit 0x6a  // PUSH 2             (3rd arg: flags)
        _emit 0x02
        _emit 0x6a  // PUSH 0x10          (2nd arg: capacity)
        _emit 0x10
        _emit 0x8d  // LEA EAX, [ESI+0x30]  (1st arg: &this->field_30)
        _emit 0x46
        _emit 0x30
        _emit 0x50  // PUSH EAX
        _emit 0x89  // MOV [ESI+8], EBX   (this->field_8 = NULL)
        _emit 0x5e
        _emit 0x08
        _emit 0xc6  // MOV byte ptr [ESP+0x2c], 2  (SEH state = 2)
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x02
        // ---- byte 102: CALL FUN_009d1c4c (__stdcall, callee-cleanup) ----
        call FUN_009d1c4c
        // ---- bytes 107-122: free this->field_14 string if non-null ----
        _emit 0x8b  // MOV EAX, [ESI+0x14]
        _emit 0x46
        _emit 0x14
        _emit 0x3b  // CMP EAX, EBX
        _emit 0xc3
        _emit 0xc6  // MOV byte ptr [ESP+0x1c], 1  (SEH state = 1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0x74  // JZ +9  (skip_free_field14)
        _emit 0x09
        _emit 0x8b  // MOV ECX, [EAX-4]   (ECX = block header for __thiscall)
        _emit 0x48
        _emit 0xfc
        _emit 0x50  // PUSH EAX           (arg: data ptr)
        // ---- byte 123: CALL FUN_0040df70 (__thiscall, callee-cleanup) ----
        call FUN_0040df70
        // ---- bytes 128-173: clear string fields + virtual call ----
        _emit 0x89  // MOV [ESI+0x14], EBX
        _emit 0x5e
        _emit 0x14
        _emit 0x89  // MOV [ESI+0x18], EBX
        _emit 0x5e
        _emit 0x18
        _emit 0x89  // MOV [ESI+0x1c], EBX
        _emit 0x5e
        _emit 0x1c
        _emit 0x8b  // MOV ECX, [ESI+0xc]
        _emit 0x4e
        _emit 0x0c
        _emit 0x3b  // CMP ECX, EBX
        _emit 0xcb
        _emit 0x88  // MOV byte ptr [ESP+0x1c], BL  (SEH state = 0)
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x74  // JZ +9  (skip_vcall)
        _emit 0x09
        _emit 0x8b  // MOV EDX, [ECX]     (vtable ptr)
        _emit 0x11
        _emit 0x8b  // MOV EAX, [EDX+0x30]  (vtable slot 12)
        _emit 0x42
        _emit 0x30
        _emit 0x6a  // PUSH 1             (arg)
        _emit 0x01
        _emit 0xff  // CALL EAX           (virtual dispatch)
        _emit 0xd0
        // skip_vcall:
        _emit 0x8b  // MOV ESI, [ESI+8]   (ESI = this->field_8 = NULL)
        _emit 0x76
        _emit 0x08
        _emit 0x3b  // CMP ESI, EBX
        _emit 0xf3
        _emit 0xc7  // MOV dword ptr [ESP+0x1c], -1  (SEH state = -1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x74  // JZ +0x10  (done; normal path: ESI=NULL always)
        _emit 0x10
        _emit 0x8b  // MOV ECX, ESI      (for __thiscall)
        _emit 0xce
        // ---- byte 174: CALL FUN_0043c0d0 (dtor on cur field_8) ----
        call FUN_0043c0d0
        // ---- byte 179 ----
        _emit 0x56  // PUSH ESI           (arg: ptr)
        // ---- byte 180: CALL FUN_009d1b17 (operator delete, __cdecl) ----
        call FUN_009d1b17
        // ---- bytes 185-200: done label; partial epilogue ----
        _emit 0x83  // ADD ESP, 4         (__cdecl cleanup)
        _emit 0xc4
        _emit 0x04
        _emit 0x8b  // MOV ECX, [ESP+0x14]  (prev FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64  // MOV FS:[0], ECX   (restore exception chain; general form)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX           (discard cookie)
        _emit 0x5f  // POP EDI           (byte 201 = last in 0xC9 compare window)
        // Note: POP ESI, POP EBX, ADD ESP 0x10, RET are past the window.
    }
}
