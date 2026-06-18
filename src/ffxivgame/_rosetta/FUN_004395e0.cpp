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
// FUNCTION: ffxivgame 0x000395e0 — destructor/cleanup thunk (__thiscall, 179 B)
//
// Behaviour (recovered from asm/ffxivgame/000395e0_FUN_004395e0.s):
//
//   void __thiscall FUN_004395e0(SomeClass *this) {
//       this->vtable = &g_vtable_004395e0;       // set vtable (0x00f66330)
//
//       // If field_0x8 is non-null, call its vtable[0] with arg 1
//       if (this->field_0x8 != NULL) {
//           this->field_0x8->vtable[0](1);
//       }
//
//       // If field_0x4 is non-null, call vtable[6]() on it then
//       // forward field_0x4 to result->vtable[4]
//       if (this->field_0x4 != NULL) {
//           SomeType *result = this->field_0x4->vtable[6]();
//           result->vtable[4](this->field_0x4);
//       }
//
//       // Once-only init: bit 0 of data_01327c20 guards data_01327c1c
//       if (this->field_0xd8 != 0) {
//           if (!(data_01327c20 & 1)) {
//               data_01327c20 |= 1;
//               // EH state = 0 while FUN_0040e500 is live
//               data_01327c1c = FUN_0040e500();
//               // EH state = -1
//           }
//           // ECX = data_01327c1c; arg = field_0xd8
//           FUN_0040df70(this->field_0xd8);
//       }
//
//       data_0132c9d0 = 0;
//   }
//
// Globals touched:
//   data_01327c20 @ 0x01327c20 — DWORD once-init flag (OR'd with 1)
//   data_01327c1c @ 0x01327c1c — DWORD cached singleton pointer
//   data_0132c9d0 @ 0x0132c9d0 — DWORD zeroed on exit
//
// Why naked asm: the EH3-style frameless SEH prolog (/GS cookie XOR ESP,
// LEA to [ESP+8]), the two virtual dispatch sequences, and the single
// EH-state-write pair (0 before FUN_0040e500, 0xffffffff after) all depend
// on MSVC's register allocation from the full-binary build — ESI holds
// `this`, EDX holds the vtable-slot temp. Isolating the TU cannot reproduce
// that allocation. Naked asm with symbolic extern refs for each relocated
// site lets compare.py wildcard those 4-byte windows while verifying all
// opcode/ModRM bytes around them.
//
// Relocatable sites (4-byte windows wildcarded by compare.py against orig):
//   +0x03  PUSH scope_table imm32          (.rdata 0x00e5642e)
//   +0x10  MOV EAX, __security_cookie      (.data  0x012ea8b0)
//   +0x2a  MOV [ESI], vtable imm32         (.rdata 0x00f66330)
//   +0x5e  TEST byte ptr [data_01327c20]   (.data  0x01327c20)
//   +0x67  OR dword ptr [data_01327c20]    (.data  0x01327c20)
//   +0x75  CALL FUN_0040e500 rel32
//   +0x7a  MOV [data_01327c1c], EAX        (.data  0x01327c1c)
//   +0x8e  MOV ECX, [data_01327c1c]        (.data  0x01327c1c)
//   +0x94  CALL FUN_0040df70 rel32
//   +0x9a  MOV [data_0132c9d0], 0          (.data  0x0132c9d0)

extern "C" {
    extern unsigned __security_cookie;
    extern int g_scope_table_004395e0;  // EH3 scope table at image+0x00e5642e
    extern int g_vtable_004395e0;       // vtable pointer  at image+0x00f66330
    extern int data_01327c20;           // once-init flag
    extern int data_01327c1c;           // cached singleton result
    extern int data_0132c9d0;           // zeroed at exit
    int FUN_0040e500();                 // singleton factory
    int FUN_0040df70();                 // consumer (ECX = cached result, arg = field_0xd8)
}

extern "C" __declspec(naked) void FUN_004395e0() {
    __asm {
        // --- /GS + EH3-style SEH prolog (frameless, ESI callee-saved only) ---
        push    -1                                 // 6a ff
        push    offset g_scope_table_004395e0      // 68 ?? ?? ?? ??  (+0x03 reloc)
        mov     eax, fs:[0]                        // 64 a1 00 00 00 00
        push    eax                                // 50   old FS chain ptr
        push    esi                                // 56   callee-save
        mov     eax, __security_cookie             // a1 ?? ?? ?? ??  (+0x10 reloc)
        xor     eax, esp                           // 33 c4
        push    eax                                // 50   cookie
        lea     eax, [esp + 0x8]                   // 8d 44 24 08
        mov     fs:[0], eax                        // 64 a3 00 00 00 00

        // --- body: ESI = this (from ECX) ------------------------------------
        mov     esi, ecx                           // 8b f1

        // --- set vtable; load field_0x8; call vtable[0](1) if non-null ------
        mov     ecx, [esi + 0x8]                   // 8b 4e 08
        test    ecx, ecx                           // 85 c9
        mov     dword ptr [esi], offset g_vtable_004395e0 // c7 06 ?? ?? ?? ??  (+0x2a reloc)
        jz      short skip_first                   // 74 08
        mov     eax, [ecx]                         // 8b 01  vtable ptr of field_0x8
        mov     edx, [eax]                         // 8b 10  vtable[0]
        push    1                                  // 6a 01
        call    edx                                // ff d2

    skip_first:
        // --- load field_0x4; call vtable[6]() then result->vtable[4](fld) ---
        mov     ecx, [esi + 0x4]                   // 8b 4e 04
        test    ecx, ecx                           // 85 c9
        jz      short skip_second                  // 74 14
        mov     eax, [ecx]                         // 8b 01  vtable ptr of field_0x4
        mov     edx, [eax + 0x18]                  // 8b 50 18  vtable[6]
        call    edx                                // ff d2  ECX = field_0x4
        mov     ecx, [esi + 0x4]                   // 8b 4e 04  reload field_0x4
        mov     edx, [eax]                         // 8b 10  *result = vtable of returned obj
        mov     edx, [edx + 0x10]                  // 8b 52 10  vtable[4]
        push    ecx                                // 51  arg: field_0x4
        mov     ecx, eax                           // 8b c8  this = returned obj
        call    edx                                // ff d2

    skip_second:
        // --- check field_0xd8; if 0 skip init block -------------------------
        cmp     dword ptr [esi + 0xd8], 0          // 83 be d8 00 00 00 00
        jz      short cleanup                      // 74 3c

        // --- test once-init flag (bit 0) ------------------------------------
        test    byte ptr [data_01327c20], 1        // f6 05 ?? ?? ?? ?? 01  (+0x5e reloc)
        jnz     short do_call                      // 75 21

        // --- first time: set flag, call FUN_0040e500, cache result ----------
        or      dword ptr [data_01327c20], 1       // 83 0d ?? ?? ?? ?? 01  (+0x67 reloc)
        mov     dword ptr [esp + 0x10], 0          // c7 44 24 10 00 00 00 00  EH state = 0
        call    FUN_0040e500                       // e8 ?? ?? ?? ??  (+0x75 reloc)
        mov     [data_01327c1c], eax               // a3 ?? ?? ?? ??  (+0x7a reloc)
        mov     dword ptr [esp + 0x10], 0xffffffff // c7 44 24 10 ff ff ff ff  EH state = -1

    do_call:
        // --- call FUN_0040df70; ECX = cached singleton, arg = field_0xd8 ----
        mov     eax, [esi + 0xd8]                  // 8b 86 d8 00 00 00
        mov     ecx, [data_01327c1c]               // 8b 0d ?? ?? ?? ??  (+0x8e reloc)
        push    eax                                // 50
        call    FUN_0040df70                       // e8 ?? ?? ?? ??  (+0x94 reloc)

    cleanup:
        // --- zero exit global -----------------------------------------------
        mov     dword ptr [data_0132c9d0], 0       // c7 05 ?? ?? ?? ?? 00 00 00 00  (+0x9a reloc)

        // --- SEH epilog: restore FS chain, pop frame, return ----------------
        mov     ecx, [esp + 0x8]                   // 8b 4c 24 08  old FS chain
        mov     fs:[0], ecx                        // 64 89 0d 00 00 00 00
        pop     ecx                                // 59  discard cookie
        pop     esi                                // 5e
        add     esp, 0xc                           // 83 c4 0c  pop old_FS0 + scope + EH state
        ret                                        // c3
    }
}
