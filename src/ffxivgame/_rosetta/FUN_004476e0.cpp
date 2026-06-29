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
// FUNCTION: ffxivgame 0x000476e0 — UTF-16-to-UTF-8 string assign helper
//                                  (__thiscall, 59 B / 0x3b, ret 4).
//
// Calling convention: __thiscall (ECX = this); one stack arg (const wchar_t* src);
//   callee cleans 4 bytes (RET 0x4).
//
// Object layout (offsets touched, see FUN_00447010):
//   [this + 0x00]  char* data;    — backing storage pointer
//
// Body:
//   1. PUSH EBX / MOV EBX,[ESP+8]   ; EBX = src (wchar_t*)
//   2. PUSH ESI / PUSH EDI
//   3. CALL FUN_00445ae0(EBX, 0)    ; measure: how many UTF-8 bytes needed?
//                                   ; (ESI = ECX = this, set after arg pushes)
//   4. ADD ESP,8 / MOV EDI,EAX      ; EDI = byte count
//   5. PUSH 1 / LEA EAX,[EDI+1] / PUSH EAX / MOV ECX,ESI
//      CALL FUN_00447010(count+1,1) ; __thiscall resize, RET 8
//   6. MOV ECX,[ESI] / PUSH ECX / PUSH EBX
//      CALL FUN_00445ae0(EBX, data) ; convert & copy UTF-16→UTF-8 into buffer
//   7. MOV EDX,[ESI] / ADD ESP,8
//      MOV byte ptr [EDI+EDX],0     ; data[count] = '\0'
//   8. POP EDI / POP ESI / POP EBX / RET 4
//
// Register allocation: EBX = src, ESI = this, EDI = count.
// The MOV ESI,ECX (this-save) is scheduled by MSVC *after* the two argument
// pushes for the first call — the same interleaved-move idiom seen in sibling
// FUN_00447720 (where EBX = this, ESI = count, EDI = src).
//
// CALL targets (REL32, wildcarded by tools/compare.py):
//   +0x0c  CALL FUN_00445ae0  (RVA 0x00045ae0 — UTF-16 measure/copy helper)
//   +0x1e  CALL FUN_00447010  (RVA 0x00047010 — growable-buf resize, __thiscall ret 8)
//   +0x27  CALL FUN_00445ae0  (RVA 0x00045ae0 — convert into dest buffer)
//
// Reconstruction strategy — naked-asm:
//   The MOV ESI,ECX interleaved between the two pre-call pushes and the
//   SIB-encoded [EDI+EDX*1] null-store cannot be driven from source-level C++
//   at /O2. Emit symbolically via `__declspec(naked)` with a raw _emit for the
//   SIB byte sequence; compare.py masks the three REL32 windows.
//   Mirrors the approach taken by sibling FUN_00447720.

extern "C" {
    int FUN_00445ae0(const wchar_t *, char *);   // UTF-16 measure/copy (cdecl)
    void FUN_00447010();                          // growable-buf resize (__thiscall ret 8)
}

extern "C" __declspec(naked) void FUN_004476e0() {
    __asm {
        // 000476e0: 53                PUSH EBX
        push    ebx
        // 000476e1: 8b 5c 24 08       MOV EBX,[ESP+0x8]  (src)
        mov     ebx, dword ptr [esp + 0x8]
        // 000476e5: 56                PUSH ESI
        push    esi
        // 000476e6: 57                PUSH EDI
        push    edi
        // 000476e7: 6a 00             PUSH 0x0
        push    0
        // 000476e9: 53                PUSH EBX
        push    ebx
        // 000476ea: 8b f1             MOV ESI,ECX   (this — interleaved after arg pushes)
        mov     esi, ecx
        // 000476ec: e8 ef e3 ff ff    CALL FUN_00445ae0
        call    FUN_00445ae0
        // 000476f1: 83 c4 08          ADD ESP,0x8
        add     esp, 8
        // 000476f4: 8b f8             MOV EDI,EAX   (count)
        mov     edi, eax
        // 000476f6: 6a 01             PUSH 0x1
        push    1
        // 000476f8: 8d 47 01          LEA EAX,[EDI+0x1]
        lea     eax, [edi + 1]
        // 000476fb: 50                PUSH EAX
        push    eax
        // 000476fc: 8b ce             MOV ECX,ESI   (this)
        mov     ecx, esi
        // 000476fe: e8 0d f9 ff ff    CALL FUN_00447010
        call    FUN_00447010
        // 00047703: 8b 0e             MOV ECX,[ESI]  (this->data)
        mov     ecx, dword ptr [esi]
        // 00047705: 51                PUSH ECX
        push    ecx
        // 00047706: 53                PUSH EBX
        push    ebx
        // 00047707: e8 d4 e3 ff ff    CALL FUN_00445ae0
        call    FUN_00445ae0
        // 0004770c: 8b 16             MOV EDX,[ESI]  (this->data reload)
        mov     edx, dword ptr [esi]
        // 0004770e: 83 c4 08          ADD ESP,0x8
        add     esp, 8
        // 00047711: c6 04 17 00       MOV byte ptr [EDI+EDX*1],0x0
        _emit   0xc6
        _emit   0x04
        _emit   0x17
        _emit   0x00
        // 00047715: 5f                POP EDI
        pop     edi
        // 00047716: 5e                POP ESI
        pop     esi
        // 00047717: 5b                POP EBX
        pop     ebx
        // 00047718: c2 04 00          RET 0x4
        ret     4
    }
}
