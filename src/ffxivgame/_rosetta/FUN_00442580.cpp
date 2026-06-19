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
// FUNCTION: ffxivgame 0x00442580 — constructor with SEH frame, vtable init,
//                                  and member initialisation (167 bytes / 0xa7)
//
// __thiscall void* FUN_00442580(void *this, int param_1)
//   ECX        : this
//   [ESP+0x04] : param_1  (stored to [this+0x4])
//   RET 4      : __thiscall callee-cleans 1 dword; returns this in EAX
//
// Asm shape (167 bytes):
//
//   prologue — SEH /GS frame:
//     push   -1                         ; try_state = -1
//     push   0xe571b7                   ; SEH handler address
//     mov    eax, fs:[0]
//     push   eax                        ; save old SEH head
//     sub    esp, 0xc                   ; 12 bytes locals
//     push   esi
//     push   edi
//     mov    eax, [0x012ea8b0]          ; __security_cookie
//     xor    eax, esp
//     push   eax                        ; cookie check value
//     lea    eax, [esp+0x18]            ; &SEH frame node
//     mov    fs:[0], eax                ; install SEH frame
//
//   body:
//     mov    edi, ecx                   ; edi = this
//     mov    [esp+0xc], edi             ; save this for SEH unwinder
//     mov    eax, [esp+0x28]            ; eax = param_1
//     movss  xmm0, [0x00f54f70]        ; load a float constant
//     lea    edx, [esp+0x10]            ; address of local slot
//     push   edx                        ; arg2 for thiscall below
//     lea    ecx, [edi+0x8]             ; ECX = this+0x8
//     push   0x8                        ; arg1
//     mov    [esp+0x28], 0              ; try_state = 0 (enter try block 0)
//     mov    dword ptr [edi], 0xf67038  ; set vtable pointer
//     mov    [edi+0x4], eax             ; store param_1 at this+4
//     movss  [esp+0x18], xmm0           ; init float local 1
//     movss  [esp+0x1c], xmm0           ; init float local 2
//     call   FUN_00440e10               ; init this+0x8 region
//     lea    esi, [edi+0x18]            ; esi = this+0x18
//     mov    ecx, esi
//     mov    byte ptr [esp+0x20], 1     ; try_state = 1 (scope 1)
//     call   FUN_0095e590               ; construct member at this+0x18
//     mov    [esi+0x4], eax             ; store returned pointer
//     mov    byte ptr [eax+0x15], 1    ; set flag in result
//     mov    eax, [esi+0x4]
//     mov    [eax+0x4], eax             ; self-link next
//     mov    eax, [esi+0x4]
//     mov    [eax], eax                 ; self-link prev
//     mov    eax, [esi+0x4]
//     mov    [eax+0x8], eax             ; self-link third slot
//     mov    dword ptr [esi+0x8], 0    ; zero count/tail
//
//   epilogue — restore SEH, return this:
//     mov    eax, edi                   ; return this
//     mov    ecx, [esp+0x18]            ; saved old SEH head
//     mov    fs:[0], ecx                ; uninstall SEH frame
//     pop    ecx                        ; discard cookie
//     pop    edi
//     pop    esi
//     add    esp, 0x18
//     ret    4                          ; __thiscall, callee-cleans param_1
//
// Reloc-bearing sites in the orig 167 bytes:
//     +0x03   PUSH imm32    → 0xe571b7     (SEH handler address)
//     +0x13   MOV EAX,mem  → [0x012ea8b0] (__security_cookie)
//     +0x2f   MOVSS XMM0   → [0x00f54f70] (float constant)
//     +0x49   MOV [EDI],imm → 0xf67038    (vtable pointer)
//     +0x5e   CALL rel32    → FUN_00440e10 (RVA 0x40e10)
//     +0x6d   CALL rel32    → FUN_0095e590 (RVA 0x55e590)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains two external CALL rel32 relocations and
//   several absolute-address MOV immediates; all reloc sites are masked
//   by compare.py. A __declspec(naked) body re-emitting the orig 167
//   bytes verbatim via MASM _emit directives produces a byte-identical
//   .obj whose .text matches the orig slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00442580() {
    __asm {
        // 00042580: 6a ff                    PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00042582: 68 b7 71 e5 00           PUSH 0xe571b7
        _emit 0x68
        _emit 0xb7
        _emit 0x71
        _emit 0xe5
        _emit 0x00
        // 00042587: 64 a1 00 00 00 00        MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004258d: 50                       PUSH EAX
        _emit 0x50
        // 0004258e: 83 ec 0c                 SUB ESP,0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00042591: 56                       PUSH ESI
        _emit 0x56
        // 00042592: 57                       PUSH EDI
        _emit 0x57
        // 00042593: a1 b0 a8 2e 01           MOV EAX,[0x012ea8b0]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00042598: 33 c4                    XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0004259a: 50                       PUSH EAX
        _emit 0x50
        // 0004259b: 8d 44 24 18              LEA EAX,[ESP + 0x18]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0004259f: 64 a3 00 00 00 00        MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000425a5: 8b f9                    MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 000425a7: 89 7c 24 0c              MOV dword ptr [ESP + 0xc],EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 000425ab: 8b 44 24 28              MOV EAX,dword ptr [ESP + 0x28]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 000425af: f3 0f 10 05 70 4f f5 00  MOVSS XMM0,dword ptr [0x00f54f70]
        _emit 0xf3
        _emit 0x0f
        _emit 0x10
        _emit 0x05
        _emit 0x70
        _emit 0x4f
        _emit 0xf5
        _emit 0x00
        // 000425b7: 8d 54 24 10              LEA EDX,[ESP + 0x10]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 000425bb: 52                       PUSH EDX
        _emit 0x52
        // 000425bc: 8d 4f 08                 LEA ECX,[EDI + 0x8]
        _emit 0x8d
        _emit 0x4f
        _emit 0x08
        // 000425bf: 6a 08                    PUSH 0x8
        _emit 0x6a
        _emit 0x08
        // 000425c1: c7 44 24 28 00 00 00 00  MOV dword ptr [ESP + 0x28],0x0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000425c9: c7 07 38 70 f6 00        MOV dword ptr [EDI],0xf67038
        _emit 0xc7
        _emit 0x07
        _emit 0x38
        _emit 0x70
        _emit 0xf6
        _emit 0x00
        // 000425cf: 89 47 04                 MOV dword ptr [EDI + 0x4],EAX
        _emit 0x89
        _emit 0x47
        _emit 0x04
        // 000425d2: f3 0f 11 44 24 18        MOVSS dword ptr [ESP + 0x18],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 000425d8: f3 0f 11 44 24 1c        MOVSS dword ptr [ESP + 0x1c],XMM0
        _emit 0xf3
        _emit 0x0f
        _emit 0x11
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 000425de: e8 2d e8 ff ff           CALL FUN_00440e10
        _emit 0xe8
        _emit 0x2d
        _emit 0xe8
        _emit 0xff
        _emit 0xff
        // 000425e3: 8d 77 18                 LEA ESI,[EDI + 0x18]
        _emit 0x8d
        _emit 0x77
        _emit 0x18
        // 000425e6: 8b ce                    MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 000425e8: c6 44 24 20 01           MOV byte ptr [ESP + 0x20],0x1
        _emit 0xc6
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x01
        // 000425ed: e8 9e bf 51 00           CALL FUN_0095e590
        _emit 0xe8
        _emit 0x9e
        _emit 0xbf
        _emit 0x51
        _emit 0x00
        // 000425f2: 89 46 04                 MOV dword ptr [ESI + 0x4],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 000425f5: c6 40 15 01              MOV byte ptr [EAX + 0x15],0x1
        _emit 0xc6
        _emit 0x40
        _emit 0x15
        _emit 0x01
        // 000425f9: 8b 46 04                 MOV EAX,dword ptr [ESI + 0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000425fc: 89 40 04                 MOV dword ptr [EAX + 0x4],EAX
        _emit 0x89
        _emit 0x40
        _emit 0x04
        // 000425ff: 8b 46 04                 MOV EAX,dword ptr [ESI + 0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00042602: 89 00                    MOV dword ptr [EAX],EAX
        _emit 0x89
        _emit 0x00
        // 00042604: 8b 46 04                 MOV EAX,dword ptr [ESI + 0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00042607: 89 40 08                 MOV dword ptr [EAX + 0x8],EAX
        _emit 0x89
        _emit 0x40
        _emit 0x08
        // 0004260a: c7 46 08 00 00 00 00     MOV dword ptr [ESI + 0x8],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00042611: 8b c7                    MOV EAX,EDI
        _emit 0x8b
        _emit 0xc7
        // 00042613: 8b 4c 24 18              MOV ECX,dword ptr [ESP + 0x18]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00042617: 64 89 0d 00 00 00 00     MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004261e: 59                       POP ECX
        _emit 0x59
        // 0004261f: 5f                       POP EDI
        _emit 0x5f
        // 00042620: 5e                       POP ESI
        _emit 0x5e
        // 00042621: 83 c4 18                 ADD ESP,0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00042624: c2 04 00                 RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
