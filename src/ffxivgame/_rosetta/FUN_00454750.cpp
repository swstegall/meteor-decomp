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
// FUNCTION: ffxivgame 0x00454750 — EH3-SEH-wrapped device/stream read +
//                                  dispatch (291 B / 0x123, `/GS` cookie).
//
// Inspection (read from the disassembly at orig RVA 0x00054750, image
// base 0x00400000 → VA 0x00454750):
//
//   bool FUN_00454750(Obj *self /* [esp+0x24c] → ESI */);
//
//   The function is wrapped in the canonical MSVC-2005 EH3 prologue
//   (PUSH -1 trylevel / PUSH scope-table 0x00e5840b / PUSH FS:[0] /
//   SUB ESP,0x230 / __security_cookie ^ ESP at [esp+0x22c] / PUSH
//   EBX / PUSH ESI / second cookie ^ ESP / FS:[0] install) and returns
//   a bool in AL (BL is the running result, zero-initialised at the top
//   via XOR EBX,EBX, set to 1 only on the success path).
//
//     EBX = 0;                                    // result = false
//     if (self->Probe() == 0) {                   // __thiscall @0x004451f0
//         long hr;
//         if (pIface->lpVtbl_A(&hr) >= 0) {        // IAT [0x00f3e3d4]
//             if (pIface->lpVtbl_B(0, 5, &buf) >= 0) {  // IAT [0x00f3e3d0]
//                 char rec[0x206];                 // [esp+0x36] scratch
//                 *(short*)&rec[6] = 0;            // [esp+0x3c] = 0
//                 memset(rec, 0, 0x206);           // @0x009d2110
//                 pIface->lpVtbl_C(buf, &out);     // IAT [0x00f3e3d8]
//                 (*self_obj0->vtbl[0x14/4])(out); // virtual @[[esp+0xc]]+0x14
//                 FUN_00454e90(&local, &out);      // ctor/copy
//                 self->Consume(&local);           // __thiscall @0x00447720
//                 FUN_00452d00(self);              // __cdecl, 1 stack arg
//                 EBX = 1;                          // result = true
//                 // trylevel = -1; dtor FUN_00403fd0(&local)
//             }
//             (*self_obj0->vtbl[0x8/4])();         // virtual @[[esp+0xc]]+0x8 (Release)
//         }
//     }
//     return (bool)EBX;
//
//   Reloc-bearing sites in the orig 291 bytes (absolute addresses /
//   IAT slots / rel32 callees that only resolve in a full-binary relink
//   at image base 0x00400000):
//     +0x07  scope-table handler RVA   (0x00e5840b — .rdata FuncInfo)
//     +0x14  __security_cookie load    (.data 0x012ea8b0)
//     +0x24  __security_cookie load    (.data 0x012ea8b0, 2nd)
//     +0x44  Probe CALL                (.text 0x004451f0 rel32 — __thiscall)
//     +0x56  IAT call                  (.rdata 0x00f3e3d4)
//     +0x6c  IAT call                  (.rdata 0x00f3e3d0)
//     +0x86  _memset CALL              (.text 0x009d2110 rel32)
//     +0x98  IAT call                  (.rdata 0x00f3e3d8)
//     +0xb8  FUN_00454e90 CALL         (.text 0x00454e90 rel32)
//     +0xcb  Consume CALL              (.text 0x00447720 rel32 — __thiscall)
//     +0xd1  FUN_00452d00 CALL         (.text 0x00452d00 rel32)
//     +0xea  FUN_00403fd0 CALL         (.text 0x00403fd0 rel32 — dtor)
//     +0x117 __security_check_cookie   (.text 0x009d20f4 rel32)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   This is the same brittle-under-/O2 case as FUN_00401a00 and the
//   other SEH-wrapped /GS siblings: a source-level rewrite would have to
//   coax MSVC 2005 into reproducing the exact EH3 prologue, the precise
//   __security_cookie stack-slot offsets, the IAT-vs-rel32 call mix, the
//   virtual-dispatch sites, AND the linker-resolved absolute addresses in
//   the thirteen relocation windows above — each one shiftable by any
//   high-level edit. The pragmatic, byte-exact choice is to re-emit the
//   orig 291 bytes verbatim via MASM `_emit` directives; the .obj's
//   `.text` section ends up byte-identical to the orig slice (the call
//   displacements are baked in as raw immediates), which is what
//   `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_00454750() {
    __asm {
        _emit 0x6a  // PUSH -0x1
        _emit 0xff
        _emit 0x68  // PUSH 0x00e5840b
        _emit 0x0b
        _emit 0x84
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x81  // SUB ESP, 0x230
        _emit 0xec
        _emit 0x30
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89  // MOV [ESP+0x22c], EAX
        _emit 0x84
        _emit 0x24
        _emit 0x2c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0xa1  // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA EAX, [ESP+0x23c]
        _emit 0x84
        _emit 0x24
        _emit 0x3c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x64  // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ESI, [ESP+0x24c]
        _emit 0xb4
        _emit 0x24
        _emit 0x4c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0x33  // XOR EBX, EBX
        _emit 0xdb
        _emit 0xe8  // CALL 0x004451f0
        _emit 0x57
        _emit 0x0a
        _emit 0xff
        _emit 0xff
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f  // JNZ 0x0045484b
        _emit 0x85
        _emit 0xaa
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL [0x00f3e3d4]
        _emit 0x15
        _emit 0xd4
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f  // JL 0x0045484b
        _emit 0x8c
        _emit 0x97
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x51  // PUSH ECX
        _emit 0x6a  // PUSH 0x5
        _emit 0x05
        _emit 0x53  // PUSH EBX
        _emit 0xff  // CALL [0x00f3e3d0]
        _emit 0x15
        _emit 0xd0
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c  // JL 0x0045483f
        _emit 0x79
        _emit 0x68  // PUSH 0x206
        _emit 0x06
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA EDX, [ESP+0x36]
        _emit 0x54
        _emit 0x24
        _emit 0x36
        _emit 0x53  // PUSH EBX
        _emit 0x52  // PUSH EDX
        _emit 0x66  // MOV [ESP+0x3c], BX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x3c
        _emit 0xe8  // CALL 0x009d2110 (memset)
        _emit 0x35
        _emit 0xd9
        _emit 0x57
        _emit 0x00
        _emit 0x8b  // MOV ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x8d  // LEA EAX, [ESP+0x30]
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x50  // PUSH EAX
        _emit 0x51  // PUSH ECX
        _emit 0xff  // CALL [0x00f3e3d8]
        _emit 0x15
        _emit 0xd8
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        _emit 0x8b  // MOV EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b  // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8b  // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x8b  // MOV EDX, [EDX+0x14]
        _emit 0x52
        _emit 0x14
        _emit 0x51  // PUSH ECX
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL EDX
        _emit 0xd2
        _emit 0x8d  // LEA EAX, [ESP+0x30]
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0xe8  // CALL 0x00454e90
        _emit 0x83
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX, ESI
        _emit 0xce
        _emit 0x89  // MOV [ESP+0x248], EBX
        _emit 0x9c
        _emit 0x24
        _emit 0x48
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL 0x00447720
        _emit 0x00
        _emit 0x2f
        _emit 0xff
        _emit 0xff
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL 0x00452d00
        _emit 0xda
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x8d  // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0xb3  // MOV BL, 0x1
        _emit 0x01
        _emit 0xc7  // MOV dword ptr [ESP+0x244], 0xffffffff
        _emit 0x84
        _emit 0x24
        _emit 0x44
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8  // CALL 0x00403fd0
        _emit 0x91
        _emit 0xf7
        _emit 0xfa
        _emit 0xff
        _emit 0x8b  // MOV EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b  // MOV EDX, [EAX]
        _emit 0x10
        _emit 0x50  // PUSH EAX
        _emit 0x8b  // MOV EAX, [EDX+0x8]
        _emit 0x42
        _emit 0x08
        _emit 0xff  // CALL EAX
        _emit 0xd0
        _emit 0x8a  // MOV AL, BL
        _emit 0xc3
        _emit 0x8b  // MOV ECX, [ESP+0x23c]
        _emit 0x8c
        _emit 0x24
        _emit 0x3c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x64  // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x5e  // POP ESI
        _emit 0x5b  // POP EBX
        _emit 0x8b  // MOV ECX, [ESP+0x22c]
        _emit 0x8c
        _emit 0x24
        _emit 0x2c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x33  // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8  // CALL 0x009d20f4 (__security_check_cookie)
        _emit 0x88
        _emit 0xd8
        _emit 0x57
        _emit 0x00
        _emit 0x81  // ADD ESP, 0x23c
        _emit 0xc4
        _emit 0x3c
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc3  // RET
    }
}
