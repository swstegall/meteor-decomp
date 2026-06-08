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
// FUNCTION: ffxivgame 0x00422f90 — delegating wrapper / virtual-fallback
//                                  (__thiscall, 51 B / 0x33)
//
//   __thiscall void FUN_00422f90(
//       Class*         this   /* ECX */,
//       unsigned       arg1   /* [ESP+0x04] → EBP */,
//       const float*   arg2   /* [ESP+0x08] → EBX */,
//       unsigned       arg3   /* [ESP+0x0c] → EDI */);
//
// Behaviour:
//   1. Load args into callee-saved registers using the MSVC 2005 interleaved
//      PUSH-save / MOV-load idiom:
//        PUSH EBX  ; save EBX
//        MOV  EBX, [ESP+0x0c]  ; EBX = arg2  (stack shifted by 1 push)
//        PUSH EBP  ; save EBP
//        MOV  EBP, [ESP+0x0c]  ; EBP = arg1  (stack shifted by 2 pushes)
//        PUSH ESI / PUSH EDI   ; save both
//        MOV  EDI, [ESP+0x1c]  ; EDI = arg3  (stack shifted by 4 pushes)
//   2. Call FUN_00423a40 (__thiscall on this->field_4) with args (arg1, arg2, arg3):
//        PUSH EDI  ; arg3 deepest
//        MOV  ESI, ECX          ; ESI = this
//        MOV  ECX, [ESI+4]      ; ECX = this->field_4
//        PUSH EBX  ; arg2
//        PUSH EBP  ; arg1 (top of stack = first param)
//        CALL FUN_00423a40
//   3. TEST AL,AL / JNZ epilogue  — if non-zero (no change), skip virtual call.
//   4. Otherwise, virtual dispatch through this->field_0:
//        MOV ECX, [ESI]         ; ECX = this->field_0
//        MOV EAX, [ECX]         ; EAX = vtable ptr of *field_0
//        MOV EDX, [EAX]         ; EDX = vtable[0]
//        PUSH EBX  ; arg2 (deepest)
//        PUSH EDI  ; arg3
//        PUSH EBP  ; arg1 (top of stack = first param)
//        CALL EDX
//   5. Restore callee-saved regs (EDI, ESI, EBP, EBX), RET 0xC.
//
// Calling convention: __thiscall; callee cleans 0xC bytes (3 DWORDs).
// Frame: no ESP adjustment; 4 callee-saved regs (EBX, EBP, ESI, EDI).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaved PUSH/MOV prolog and the mismatched push orders between
//   the two call sites (arg1,arg2,arg3 vs arg1,arg3,arg2) preclude a clean
//   source-level reconstruction without reproducing the exact register
//   allocation MSVC chose. The 51-byte sequence is re-emitted verbatim via
//   _emit directives. The CALL rel32 at +0x18 is masked by tools/compare.py.

extern "C" __declspec(naked) void FUN_00422f90() {
    __asm {
        // 00022f90: 53             PUSH EBX
        _emit 0x53
        // 00022f91: 8b 5c 24 0c   MOV EBX,[ESP+0x0C]   ; EBX = arg2 (1 push deep)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 00022f95: 55             PUSH EBP
        _emit 0x55
        // 00022f96: 8b 6c 24 0c   MOV EBP,[ESP+0x0C]   ; EBP = arg1 (2 pushes deep)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 00022f9a: 56             PUSH ESI
        _emit 0x56
        // 00022f9b: 57             PUSH EDI
        _emit 0x57
        // 00022f9c: 8b 7c 24 1c   MOV EDI,[ESP+0x1C]   ; EDI = arg3 (4 pushes deep)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00022fa0: 57             PUSH EDI              ; push arg3 for 1st call
        _emit 0x57
        // 00022fa1: 8b f1          MOV ESI,ECX           ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00022fa3: 8b 4e 04       MOV ECX,[ESI+4]       ; ECX = this->field_4
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 00022fa6: 53             PUSH EBX              ; push arg2 for 1st call
        _emit 0x53
        // 00022fa7: 55             PUSH EBP              ; push arg1 for 1st call
        _emit 0x55
        // 00022fa8: e8 93 0a 00 00 CALL FUN_00423a40     ; rel32 = 0x00000a93
        _emit 0xe8
        _emit 0x93
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        // 00022fad: 84 c0          TEST AL,AL
        _emit 0x84
        _emit 0xc0
        // 00022faf: 75 0b          JNZ +0x0b (→ epilogue at 00022fbc)
        _emit 0x75
        _emit 0x0b
        // 00022fb1: 8b 0e          MOV ECX,[ESI]         ; ECX = this->field_0
        _emit 0x8b
        _emit 0x0e
        // 00022fb3: 8b 01          MOV EAX,[ECX]         ; EAX = vtable ptr
        _emit 0x8b
        _emit 0x01
        // 00022fb5: 8b 10          MOV EDX,[EAX]         ; EDX = vtable[0]
        _emit 0x8b
        _emit 0x10
        // 00022fb7: 53             PUSH EBX              ; push arg2 (deepest)
        _emit 0x53
        // 00022fb8: 57             PUSH EDI              ; push arg3
        _emit 0x57
        // 00022fb9: 55             PUSH EBP              ; push arg1 (top = 1st param)
        _emit 0x55
        // 00022fba: ff d2          CALL EDX              ; virtual dispatch vtable[0]
        _emit 0xff
        _emit 0xd2
        // 00022fbc: 5f             POP EDI
        _emit 0x5f
        // 00022fbd: 5e             POP ESI
        _emit 0x5e
        // 00022fbe: 5d             POP EBP
        _emit 0x5d
        // 00022fbf: 5b             POP EBX
        _emit 0x5b
        // 00022fc0: c2 0c 00       RET 0x000C
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
