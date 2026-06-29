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
// FUNCTION: ffxivgame 0x00047950 — byte-buffer push_back (append one char)
//                                  (__thiscall, 39 B / 0x27, ret 4).
//
// Appends a single byte/char to the null-terminated byte buffer managed by
// a custom growable-buffer object whose layout (two fields touched here) is:
//
//   struct GrowBuf {
//       char *data;    // [this+0x00]  — buffer pointer (may move on realloc)
//       // ...         // [this+0x04]  — not touched here
//       int   count;   // [this+0x08]  — total bytes including null terminator
//   };
//
// The count field stores (string_length + 1); i.e. an empty string has
// count == 1 with data[0] == '\0'. push_back:
//   1. Caches old count into EDI.
//   2. Calls FUN_00447010(old_count + 1, 1) [__thiscall, RET 8] to grow the
//      buffer — this sets count = old_count + 1 and may reallocate data.
//   3. Stores the new char at data[old_count - 1] (= former null position).
//   4. Stores 0 at data[old_count] (new null terminator position).
//
// Calling convention: __thiscall (ECX = this); one 4-byte stack arg (the
// char to append, widened to DWORD by caller); callee cleans 4 bytes (RET 4).
//
// Register allocation: ESI = this, EDI = old count.
// The two byte stores use SIB encodings that MASM would encode differently
// from the original, so they are emitted verbatim with _emit directives.
// compare.py masks the single REL32 callsite (FUN_00447010 at +0x0d).
//
// Asm (39 bytes @ orig RVA 0x00047950):
//   56                     PUSH ESI
//   57                     PUSH EDI
//   8b f1                  MOV ESI, ECX          ; this
//   8b 7e 08               MOV EDI, [ESI+0x8]    ; EDI = old count
//   6a 01                  PUSH 0x1              ; arg2
//   8d 47 01               LEA EAX, [EDI+0x1]    ; new_count
//   50                     PUSH EAX              ; arg1
//   e8 ae f6 ff ff         CALL FUN_00447010      ; grow (thiscall, ret 8)
//   8b 0e                  MOV ECX, [ESI]         ; ECX = this->data
//   8a 54 24 0c            MOV DL, [ESP+0xC]      ; DL = char arg
//   88 54 0f ff            MOV [EDI+ECX*1-0x1],DL ; data[old_count-1] = char
//   8b 06                  MOV EAX, [ESI]         ; EAX = this->data (reload)
//   c6 04 38 00            MOV byte ptr [EAX+EDI*1],0x0  ; data[old_count] = '\0'
//   5f                     POP EDI
//   5e                     POP ESI
//   c2 04 00               RET 0x4

extern "C" void FUN_00447010();   // growable-buf resize (__thiscall, ret 8)

extern "C" __declspec(naked) void FUN_00447950() {
    __asm {
        // 00047950: 56             PUSH ESI
        push    esi
        // 00047951: 57             PUSH EDI
        push    edi
        // 00047952: 8b f1          MOV ESI, ECX
        mov     esi, ecx
        // 00047954: 8b 7e 08       MOV EDI, [ESI+0x8]
        mov     edi, dword ptr [esi + 0x8]
        // 00047957: 6a 01          PUSH 1
        push    1
        // 00047959: 8d 47 01       LEA EAX, [EDI+1]
        lea     eax, [edi + 1]
        // 0004795c: 50             PUSH EAX
        push    eax
        // 0004795d: e8 ae f6 ff ff CALL FUN_00447010 (rel32 — masked by compare.py)
        call    FUN_00447010
        // 00047962: 8b 0e          MOV ECX, [ESI]
        mov     ecx, dword ptr [esi]
        // 00047964: 8a 54 24 0c    MOV DL, [ESP+0xC]
        mov     dl, byte ptr [esp + 0x0c]
        // 00047968: 88 54 0f ff    MOV byte ptr [EDI+ECX*1-1], DL
        // SIB: ModRM=54 {mod=01,reg=2(DL),rm=4(SIB)}, SIB=0f {scale=0,idx=1(ECX),base=7(EDI)}, disp8=ff(-1)
        _emit   0x88
        _emit   0x54
        _emit   0x0f
        _emit   0xff
        // 0004796c: 8b 06          MOV EAX, [ESI]
        mov     eax, dword ptr [esi]
        // 0004796e: c6 04 38 00    MOV byte ptr [EAX+EDI*1], 0
        // SIB: ModRM=04 {mod=00,reg=0,rm=4(SIB)}, SIB=38 {scale=0,idx=7(EDI),base=0(EAX)}, imm8=0
        _emit   0xc6
        _emit   0x04
        _emit   0x38
        _emit   0x00
        // 00047972: 5f             POP EDI
        pop     edi
        // 00047973: 5e             POP ESI
        pop     esi
        // 00047974: c2 04 00       RET 4
        ret     4
    }
}
