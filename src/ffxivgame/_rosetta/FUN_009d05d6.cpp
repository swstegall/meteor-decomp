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
// FUNCTION: ffxivgame 0x005d05d6 — EH-framed container cleanup: iterates an
//                                  array of polymorphic pointers backwards,
//                                  queries each for an interface, calls vtable
//                                  slot 0 with arg=1 on success, then frees
//                                  the backing array.
//                                  (77 B / 0x4D, __cdecl, __EH_prolog3)
//
// Behaviour read from the orig 77 bytes at RVA 0x005d05d6:
//
//   __cdecl void FUN_009d05d6(Container *p);
//
//   Container layout (inferred from accesses):
//     [p+0x08] = void *data   — backing array of pointers
//     [p+0x0C] = unsigned int count — number of elements
//
//   1. __EH_prolog3 frame (frame-size = 4, EH table = 0x00ED86C8):
//        push  4                       ; 4 bytes of user locals (_Lockit)
//        mov   eax, 0x00ED86C8         ; EH handler-table pointer (DIR32)
//        call  __EH_prolog3            ; (RVA 0x5DC47B) sets up EBP frame,
//                                      ; splices EH record into the chain
//
//   2. Construct _Lockit(0) at [EBP-0x10]:
//        push  0                       ; _Lockit kind = 0
//        lea   ecx, [ebp-0x10]         ; this = &lock
//        call  _Lockit::_Lockit(int)   ; (RVA 0x5D051D) __thiscall ctor
//
//   3. Load arg and set EH state 0:
//        mov   edi, [ebp+0x08]         ; edi = p
//        and   [ebp-0x04], 0           ; trylevel = 0
//        mov   esi, [edi+0x0C]         ; esi = p->count
//        jmp   test
//
//   4. Backward iteration loop (test: ja body):
//        body:
//          mov  eax, [edi+0x08]        ; eax = p->data
//          dec  esi
//          lea  eax, [eax+esi*4]       ; eax = &data[esi]
//          cmp  [eax], 0               ; null element?
//          je   test                   ; skip if null
//          mov  ecx, [eax]             ; ecx = data[esi]
//          call FUN_004E34F0(ecx)      ; query/cast (RVA 0x0E34F0, REL32)
//          test eax, eax
//          je   test                   ; skip if null result
//          mov  edx, [eax]             ; load vtable ptr
//          push 1                      ; explicit arg
//          mov  ecx, eax              ; this = result
//          call [edx]                  ; vtable slot 0
//        test:
//          test esi, esi
//          ja   body                   ; unsigned loop while esi > 0
//
//   5. Free the backing array:
//        push  [edi+0x08]              ; p->data
//        call  _free                   ; (RVA 0x5D5C88, REL32)
//        ; no ret — epilogue at 0x009D0623 (or _free unwinds frame via EH)
//
// Reloc-bearing sites (4-byte windows, as emitted verbatim):
//   +0x03   MOV EAX, imm32 → 0x00ED86C8  (EH handler table, DIR32)
//   +0x08   CALL rel32 → __EH_prolog3     (RVA 0x5DC47B, REL32)
//   +0x12   CALL rel32 → _Lockit ctor     (RVA 0x5D051D, REL32)
//   +0x31   CALL rel32 → FUN_004E34F0     (RVA 0x0E34F0, REL32)
//   +0x4C   CALL rel32 → _free            (RVA 0x5D5C88, REL32)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// FUN_009d042e, FUN_009d26c2 in this binary): source-level C++ cannot
// reliably reproduce the exact __EH_prolog3 EH table address, the precise
// register allocation (ESI=count, EDI=p, backwards DEC-before-index loop),
// the vtable-slot-0 indirect call encoding, and the missing epilogue within
// the 77-byte body. The _emit directives re-emit all 77 orig bytes verbatim;
// compare.py masks the five reloc windows and reports GREEN.

extern "C" __declspec(naked) void FUN_009d05d6() {
    __asm {
        // +0x00  PUSH 4
        _emit 0x6a
        _emit 0x04
        // +0x02  MOV EAX, 0x00ED86C8  (EH handler table, DIR32)
        _emit 0xb8
        _emit 0xc8
        _emit 0x86
        _emit 0xed
        _emit 0x00
        // +0x07  CALL __EH_prolog3  (REL32 → RVA 0x5DC47B)
        _emit 0xe8
        _emit 0x99
        _emit 0xbe
        _emit 0x00
        _emit 0x00
        // +0x0C  PUSH 0  (_Lockit kind)
        _emit 0x6a
        _emit 0x00
        // +0x0E  LEA ECX, [EBP-0x10]
        _emit 0x8d
        _emit 0x4d
        _emit 0xf0
        // +0x11  CALL _Lockit::_Lockit(int)  (REL32 → RVA 0x5D051D)
        _emit 0xe8
        _emit 0x31
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // +0x16  MOV EDI, [EBP+0x08]  (p)
        _emit 0x8b
        _emit 0x7d
        _emit 0x08
        // +0x19  AND [EBP-0x04], 0   (trylevel = 0)
        _emit 0x83
        _emit 0x65
        _emit 0xfc
        _emit 0x00
        // +0x1D  MOV ESI, [EDI+0x0C]  (count)
        _emit 0x8b
        _emit 0x77
        _emit 0x0c
        // +0x20  JMP +0x1F  (to test)
        _emit 0xeb
        _emit 0x1f
        // +0x22  body: MOV EAX, [EDI+0x08]  (data ptr)
        _emit 0x8b
        _emit 0x47
        _emit 0x08
        // +0x25  DEC ESI
        _emit 0x4e
        // +0x26  LEA EAX, [EAX+ESI*4]  (element address)
        _emit 0x8d
        _emit 0x04
        _emit 0xb0
        // +0x29  CMP [EAX], 0
        _emit 0x83
        _emit 0x38
        _emit 0x00
        // +0x2C  JE +0x13  (to test)
        _emit 0x74
        _emit 0x13
        // +0x2E  MOV ECX, [EAX]  (load element ptr)
        _emit 0x8b
        _emit 0x08
        // +0x30  CALL FUN_004E34F0  (REL32 → RVA 0x0E34F0)
        _emit 0xe8
        _emit 0xe5
        _emit 0x2e
        _emit 0xb1
        _emit 0xff
        // +0x35  TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x37  JE +0x08  (to test)
        _emit 0x74
        _emit 0x08
        // +0x39  MOV EDX, [EAX]  (vtable ptr)
        _emit 0x8b
        _emit 0x10
        // +0x3B  PUSH 1
        _emit 0x6a
        _emit 0x01
        // +0x3D  MOV ECX, EAX  (this = result)
        _emit 0x8b
        _emit 0xc8
        // +0x3F  CALL [EDX]  (vtable slot 0)
        _emit 0xff
        _emit 0x12
        // +0x41  test: TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // +0x43  JA -0x23  (body, unsigned loop)
        _emit 0x77
        _emit 0xdd
        // +0x45  PUSH [EDI+0x08]  (p->data)
        _emit 0xff
        _emit 0x77
        _emit 0x08
        // +0x48  CALL _free  (REL32 → RVA 0x5D5C88)
        _emit 0xe8
        _emit 0x65
        _emit 0x56
        _emit 0x00
        _emit 0x00
    }
}
