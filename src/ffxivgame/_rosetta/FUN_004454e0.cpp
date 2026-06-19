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
// FUNCTION: ffxivgame 0x004454e0 — __thiscall backward byte comparison
//                                  of two string-like objects (58 bytes).
//
// Layout (inferred):
//   StringLike (ECX = this, EDX = other):
//     +0x00  char *data   (pointer to character buffer)
//     +0x04  ???          (not accessed here)
//     +0x08  int   size   (buffer size; loop covers indices 0 .. size-2)
//
// Source shape (inferred):
//
//   bool StringLike::equals(const StringLike *other) const {
//       if (other->size - 1 != size - 1)
//           return false;
//       for (int i = size - 2; i >= 0; i--) {
//           if (data[i] != other->data[i])
//               return false;
//       }
//       return true;
//   }
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg `other`;
// callee cleans 4 bytes via `ret 4`).
//
// Frame:
//   PUSH ESI ; PUSH EDI     ; callee-saves only — no local stack allocation
//
// Structural notes:
//   - `return_false` at 0x004454f6 is both a fall-through (size mismatch)
//     AND the target of the backward JNZ from the loop (byte mismatch).
//   - The size comparison uses `SUB ESI, 1` + `LEA EDI, [EAX-1]` rather
//     than a direct CMP of the raw sizes — MSVC reused EAX (unmodified)
//     as the eventual loop index by computing `this->size - 1` into EDI
//     via LEA (which does not clobber EAX).
//   - `ADD EAX, -2` (not `SUB EAX, 2`) is the imm8-signed encoding MSVC
//     chose for "start loop at size-2".
//   - ECX is clobbered by `MOV CL, [ESI + EAX]` inside the loop body;
//     this is safe because `this->data` was already captured in ESI before
//     the clobber.
//   - `JS return_true` handles the size == 0 or size == 1 fast-path (no
//     bytes to compare → trivially equal).
//
// Asm (58 bytes, RVA 0x000454e0 – 0x0004551a):
//   8b 54 24 04   MOV  EDX, [ESP+4]             ; other
//   8b 41 08      MOV  EAX, [ECX+8]             ; this->size
//   56            PUSH ESI
//   8b 72 08      MOV  ESI, [EDX+8]             ; other->size
//   57            PUSH EDI
//   83 ee 01      SUB  ESI, 1                   ; other->size - 1
//   8d 78 ff      LEA  EDI, [EAX-1]             ; this->size - 1
//   3b f7         CMP  ESI, EDI
//   74 07         JZ   equal_size               ; +7 (forward)
// return_false:                                 ; fall-through + backward JNZ target
//   5f            POP  EDI
//   32 c0         XOR  AL, AL
//   5e            POP  ESI
//   c2 04 00      RET  4
// equal_size:
//   83 c0 fe      ADD  EAX, -2                  ; loop start index
//   78 11         JS   return_true              ; +17 (forward) — size <= 1
//   8b 12         MOV  EDX, [EDX]               ; other->data
//   8b 31         MOV  ESI, [ECX]               ; this->data
// loop_body:
//   8a 0c 06      MOV  CL, [ESI + EAX*1]
//   3a 0c 02      CMP  CL, [EDX + EAX*1]
//   75 e8         JNZ  return_false             ; -24 (backward)
//   83 e8 01      SUB  EAX, 1
//   79 f3         JNS  loop_body               ; -13 (backward)
// return_true:
//   5f            POP  EDI
//   b0 01         MOV  AL, 1
//   5e            POP  ESI
//   c2 04 00      RET  4

extern "C" __declspec(naked) bool FUN_004454e0() {
    __asm {
        mov     edx, dword ptr [esp + 4]
        mov     eax, dword ptr [ecx + 8]
        push    esi
        mov     esi, dword ptr [edx + 8]
        push    edi
        sub     esi, 1
        lea     edi, [eax - 1]
        cmp     esi, edi
        jz      equal_size
    return_false:
        pop     edi
        xor     al, al
        pop     esi
        ret     4
    equal_size:
        add     eax, -2
        js      return_true
        mov     edx, dword ptr [edx]
        mov     esi, dword ptr [ecx]
    loop_body:
        mov     cl, byte ptr [esi + eax*1]
        cmp     cl, byte ptr [edx + eax*1]
        jnz     return_false
        sub     eax, 1
        jns     loop_body
    return_true:
        pop     edi
        mov     al, 1
        pop     esi
        ret     4
    }
}
