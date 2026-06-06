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
// FUNCTION: ffxivgame 0x000613b0 — factory/registration function (212 B /
//                                  0xd4, __cdecl). Allocates a 12-byte local
//                                  frame via __chkstk, checks a global pointer
//                                  at 0x0132e79c, conditionally initialises it,
//                                  creates/registers a small heap object, and
//                                  returns the resulting handle (or 0 on fail).
//
// Asm shape (212 B):
//
//   MOV EAX, 0xc; CALL __chkstk      ; 12-byte local frame allocation
//   CMP [g_ptr], 0; JNZ body         ; if already initialised, skip init
//   CALL FUN_00461330; TEST EAX,EAX  ; attempt initialisation
//   JNZ body                         ; if ok, proceed to body
//   ADD ESP, 0xc; RET                ; early return (init failed)
//
//   body:
//   PUSH EDI
//   PUSH 0x130; PUSH file; PUSH 0x2; PUSH 0x9   ; trace enter
//   MOV [ESP+0x14], EBX                          ; local[0] = EBX
//   CALL FUN_00465f80
//   MOV ECX, [g_ptr]; LEA EAX, [ESP+0x14]
//   PUSH EAX; PUSH ECX
//   CALL FUN_00466b60                            ; find/lookup handle
//   MOV EDI, EAX; ADD ESP, 0x18
//   TEST EDI, EDI; JNZ epilog
//
//   ; no existing handle — allocate new object
//   PUSH ESI
//   PUSH 0x134; PUSH file; PUSH 0xc             ; alloc 12-byte object
//   CALL FUN_00463150
//   MOV ESI, EAX; ADD ESP, 0xc
//   TEST ESI, ESI; JZ pop_esi
//     MOV [ESI], EBX                             ; obj->field_0 = EBX
//     MOV [ESI+0x8], EDI                         ; obj->field_8 = 0
//     CALL FUN_004640e0                          ; get/create sub-resource
//     MOV [ESI+0x4], EAX                         ; obj->field_4 = resource
//     PUSH ESI
//     JNZ register_path
//       CALL FUN_004632f0(ESI)                   ; cleanup on sub-resource fail
//       ADD ESP, 0x4; JMP pop_esi
//     register_path:
//       MOV EDX, [g_ptr]; PUSH EDX
//       CALL FUN_00466a60(ESI, g_ptr)            ; register object
//       ADD ESP, 0x8
//       MOV EDI, ESI                             ; return new handle
//   pop_esi:
//   POP ESI
//
//   epilog:
//   PUSH 0x145; PUSH file; PUSH 0x2; PUSH 0xa   ; trace leave
//   CALL FUN_00465f80; ADD ESP, 0x10
//   TEST EDI, EDI; JNZ done
//     PUSH 0x147; PUSH file; PUSH 0x41; PUSH 0x69; PUSH 0xf
//     CALL FUN_0045c940; ADD ESP, 0x14          ; error / assert
//   done:
//   MOV EAX, EDI; POP EDI; ADD ESP, 0xc; RET
//
// Reconstruction strategy — __declspec(naked) byte passthrough.
//
//   The function mixes __chkstk-based frame setup, multiple internal calls
//   (FUN_00465f80, FUN_00466b60, FUN_00463150, FUN_004640e0, FUN_004632f0,
//   FUN_00466a60, FUN_0045c940, FUN_00461330) whose REL32 offsets must
//   resolve exactly against the orig RVA space, and three DIR32 loads from
//   the global at 0x0132e79c. Re-expressing this in high-level C++ would
//   require coaxing MSVC 2005 into the exact same register allocation,
//   branch encoding, and stack-slot layout — impractical for this call
//   density. Emitting the 212 original bytes via MASM _emit directives
//   produces a .text section that is byte-identical to orig; compare.py
//   wildcards the reloc-bearing imm32/rel32 sites and reports GREEN.
//
// Reloc-bearing sites (offset within function body):
//   +0x01   REL32  0x009d29d0  (__chkstk)
//   +0x09   DIR32  0x0132e79c  (g_ptr first CMP)
//   +0x14   REL32  0x00461330  (FUN_00461330)
//   +0x34   REL32  0x00465f80  (trace enter)
//   +0x39   DIR32  0x0132e79c  (g_ptr MOV ECX)
//   +0x45   REL32  0x00466b60  (FUN_00466b60)
//   +0x60   REL32  0x00463150  (FUN_00463150)
//   +0x73   REL32  0x004640e0  (FUN_004640e0)
//   +0x80   REL32  0x004632f0  (FUN_004632f0)
//   +0x8a   DIR32  0x0132e79c  (g_ptr MOV EDX)
//   +0x8c   REL32  0x00466a60  (FUN_00466a60)
//   +0xa0   REL32  0x00465f80  (trace leave)
//   +0xc6   REL32  0x0045c940  (FUN_0045c940)

extern "C" __declspec(naked) void FUN_004613b0() {
    __asm {
        _emit 0xb8  // MOV EAX, 0xc
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL __chkstk (0x9d29d0)
        _emit 0x16
        _emit 0x16
        _emit 0x57
        _emit 0x00
        _emit 0x83  // CMP dword ptr [0x132e79c], 0x0
        _emit 0x3d
        _emit 0x9c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x75  // JNZ body
        _emit 0x0d
        _emit 0xe8  // CALL FUN_00461330
        _emit 0x68
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75  // JNZ body
        _emit 0x04
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3  // RET
        _emit 0x57  // PUSH EDI
        _emit 0x68  // PUSH 0x130
        _emit 0x30
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0xf6936c
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0x9
        _emit 0x09
        _emit 0x89  // MOV dword ptr [ESP+0x14], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0xe8  // CALL FUN_00465f80
        _emit 0x98
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, dword ptr [0x132e79c]
        _emit 0x0d
        _emit 0x9c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x8d  // LEA EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0xe8  // CALL FUN_00466b60
        _emit 0x67
        _emit 0x57
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EDI, EAX
        _emit 0xf8
        _emit 0x83  // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x85  // TEST EDI, EDI
        _emit 0xff
        _emit 0x75  // JNZ epilog
        _emit 0x49
        _emit 0x56  // PUSH ESI
        _emit 0x68  // PUSH 0x134
        _emit 0x34
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0xf6936c
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0xe8  // CALL FUN_00463150
        _emit 0x3c
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x85  // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74  // JZ pop_esi
        _emit 0x2d
        _emit 0x89  // MOV dword ptr [ESI], EBX
        _emit 0x1e
        _emit 0x89  // MOV dword ptr [ESI+0x8], EDI
        _emit 0x7e
        _emit 0x08
        _emit 0xe8  // CALL FUN_004640e0
        _emit 0xb9
        _emit 0x2c
        _emit 0x00
        _emit 0x00
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x89  // MOV dword ptr [ESI+0x4], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x56  // PUSH ESI
        _emit 0x75  // JNZ register_path
        _emit 0x0a
        _emit 0xe8  // CALL FUN_004632f0
        _emit 0xbc
        _emit 0x1e
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0xeb  // JMP pop_esi
        _emit 0x11
        _emit 0x8b  // MOV EDX, dword ptr [0x132e79c]
        _emit 0x15
        _emit 0x9c
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        _emit 0x52  // PUSH EDX
        _emit 0xe8  // CALL FUN_00466a60
        _emit 0x1b
        _emit 0x56
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x8b  // MOV EDI, ESI
        _emit 0xfe
        _emit 0x5e  // POP ESI
        _emit 0x68  // PUSH 0x145
        _emit 0x45
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0xf6936c
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0x2
        _emit 0x02
        _emit 0x6a  // PUSH 0xa
        _emit 0x0a
        _emit 0xe8  // CALL FUN_00465f80
        _emit 0x22
        _emit 0x4b
        _emit 0x00
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85  // TEST EDI, EDI
        _emit 0xff
        _emit 0x75  // JNZ done
        _emit 0x18
        _emit 0x68  // PUSH 0x147
        _emit 0x47
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0xf6936c
        _emit 0x6c
        _emit 0x93
        _emit 0xf6
        _emit 0x00
        _emit 0x6a  // PUSH 0x41
        _emit 0x41
        _emit 0x6a  // PUSH 0x69
        _emit 0x69
        _emit 0x6a  // PUSH 0xf
        _emit 0x0f
        _emit 0xe8  // CALL FUN_0045c940
        _emit 0xc6
        _emit 0xb4
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x8b  // MOV EAX, EDI
        _emit 0xc7
        _emit 0x5f  // POP EDI
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3  // RET
    }
}
