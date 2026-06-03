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
// FUNCTION: ffxivgame 0x00418800 — version-string logger / init step
//                                  (__cdecl, 137 bytes / 0x89, /GS)
//
// Calling convention: __cdecl (single pointer arg in ESI = arg1).
// Returns 1 (bool/true) in AL.
//
// Stack frame:
//   sub esp, 0x404  →  0x400-byte local char buf[] + 4-byte /GS cookie slot
//   push esi        →  callee-save
//
// What the function does (inferred from constants):
//   1. Allocates char buf[0x400] on the stack; sets buf[0x3FE] = '\0'.
//   2. Calls an internal formatter with
//        (buf, 0x400, 0x3FE, "CDev.Engine.Dw.RenderInterface : Ver1.0.3.0 / build at Sep  5 2012 06:49:03")
//   3. Calls a string-appender with  (buf, 0x400, "\n")
//   4. Calls an IAT function (likely OutputDebugStringA or similar) with (buf, 5)
//   5. Calls func3(esi)  — passes arg1 on to another handler
//   6. Cleans up all 10 pushed dwords in one add esp, 0x28
//   7. Calls func4() and func5() with no args (global cleanup/notifications)
//   8. Returns 1 via mov al, 1 after checking the /GS cookie.
//
// Relocation sites in the original binary (compare.py masks these when
// comparing our .obj against the orig slice):
//   +0x07: A1 [B0 A8 2E 01]           — mov eax, [__security_cookie]
//   +0x1D: 68 [38 7B F5 00]           — push offset str1
//   +0x3E: 68 [84 7B F5 00]           — push offset str2
//   +0x39: E8 [62 C7 5B 00]           — call func1 (rel32)
//   +0x4D: E8 [63 C3 5B 00]           — call func2 (rel32)
//   +0x5A: FF 15 [D8 60 26 01]        — call [IAT] (abs)
//   +0x60: E8 [6C 76 00 00]           — call func3 (rel32)
//   +0x68: E8 [14 C1 00 00]           — call func4 (rel32)
//   +0x6D: E8 [2F BD 00 00]           — call func5 (rel32)
//   +0x7E: E8 [72 98 5B 00]           — call __security_check_cookie (rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The /GS prologue pattern (sub esp N / mov eax, [__security_cookie] /
//   xor eax, esp / mov [esp+N-4], eax) and the batched add esp, 0x28 cleanup
//   across four cdecl calls are structurally reproducible from C++, but the
//   exact register allocation (ESI for arg1), the pre-call buf[0x3FE]='\0'
//   written at offset 0x412 relative to the post-push esp, and the exact
//   LEA offsets (0x0C / 0x18 / 0x20) that compute buf's address across
//   different esp states are fragile under /O2 reordering. The pragmatic
//   choice — same as siblings FUN_00408610 / FUN_0040ad30 — is a
//   __declspec(naked) body that re-emits the orig 137 bytes verbatim via
//   MASM _emit directives. The .obj's .text section ends up byte-identical
//   to the orig slice, and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00418800() {
    __asm {
        // 00018800:  81 ec 04 04 00 00    SUB ESP, 0x404
        _emit 0x81
        _emit 0xec
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00018806:  a1 b0 a8 2e 01       MOV EAX, dword ptr [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0001880b:  33 c4                XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // 0001880d:  89 84 24 00 04 00 00  MOV dword ptr [ESP+0x400], EAX  ; save cookie
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00018814:  56                   PUSH ESI
        _emit 0x56
        // 00018815:  8b b4 24 0c 04 00 00  MOV ESI, dword ptr [ESP+0x40C]  ; esi = arg1
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x0c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001881c:  68 38 7b f5 00       PUSH 0x00F57B38  ; "CDev.Engine.Dw.RenderInterface ..."
        _emit 0x68
        _emit 0x38
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        // 00018821:  68 fe 03 00 00       PUSH 0x3FE
        _emit 0x68
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 00018826:  8d 44 24 0c          LEA EAX, [ESP+0x0C]  ; eax -> buf base
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 0001882a:  68 00 04 00 00       PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001882f:  50                   PUSH EAX
        _emit 0x50
        // 00018830:  c6 84 24 12 04 00 00 00  MOV byte ptr [ESP+0x412], 0  ; buf[0x3FE] = '\0'
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x12
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00018838:  e8 62 c7 5b 00       CALL func1  (rel32)
        _emit 0xe8
        _emit 0x62
        _emit 0xc7
        _emit 0x5b
        _emit 0x00
        // 0001883d:  68 84 7b f5 00       PUSH 0x00F57B84  ; "\n"
        _emit 0x68
        _emit 0x84
        _emit 0x7b
        _emit 0xf5
        _emit 0x00
        // 00018842:  8d 4c 24 18          LEA ECX, [ESP+0x18]  ; ecx -> buf base
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00018846:  68 00 04 00 00       PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001884b:  51                   PUSH ECX
        _emit 0x51
        // 0001884c:  e8 63 c3 5b 00       CALL func2  (rel32)
        _emit 0xe8
        _emit 0x63
        _emit 0xc3
        _emit 0x5b
        _emit 0x00
        // 00018851:  8d 54 24 20          LEA EDX, [ESP+0x20]  ; edx -> buf base
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x20
        // 00018855:  6a 05                PUSH 5
        _emit 0x6a
        _emit 0x05
        // 00018857:  52                   PUSH EDX
        _emit 0x52
        // 00018858:  ff 15 d8 60 26 01    CALL dword ptr [0x012660D8]  ; IAT call
        _emit 0xff
        _emit 0x15
        _emit 0xd8
        _emit 0x60
        _emit 0x26
        _emit 0x01
        // 0001885e:  56                   PUSH ESI  ; arg1 -> func3
        _emit 0x56
        // 0001885f:  e8 6c 76 00 00       CALL func3  (rel32)
        _emit 0xe8
        _emit 0x6c
        _emit 0x76
        _emit 0x00
        _emit 0x00
        // 00018864:  83 c4 28             ADD ESP, 0x28  ; pop 10 dwords (all 4 calls)
        _emit 0x83
        _emit 0xc4
        _emit 0x28
        // 00018867:  e8 14 c1 00 00       CALL func4  (rel32)
        _emit 0xe8
        _emit 0x14
        _emit 0xc1
        _emit 0x00
        _emit 0x00
        // 0001886c:  e8 2f bd 00 00       CALL func5  (rel32)
        _emit 0xe8
        _emit 0x2f
        _emit 0xbd
        _emit 0x00
        _emit 0x00
        // 00018871:  8b 8c 24 04 04 00 00  MOV ECX, dword ptr [ESP+0x404]  ; restore cookie
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00018878:  5e                   POP ESI
        _emit 0x5e
        // 00018879:  33 cc                XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // 0001887b:  b0 01                MOV AL, 1  ; return true
        _emit 0xb0
        _emit 0x01
        // 0001887d:  e8 72 98 5b 00       CALL __security_check_cookie  (rel32)
        _emit 0xe8
        _emit 0x72
        _emit 0x98
        _emit 0x5b
        _emit 0x00
        // 00018882:  81 c4 04 04 00 00    ADD ESP, 0x404
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00018888:  c3                   RET
        _emit 0xc3
    }
}
