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
// FUNCTION: ffxivgame 0x0040ddd0 — __thiscall range membership test
//                                  (52 bytes / 0x34).
//
// Checks whether `param` falls within the slot range described by the
// record at `*(this+4)`:
//
//   struct RangeInfo {
//       int   base;    // +0x00  lower bound
//       int   _pad04;  // +0x04
//       short count;   // +0x08  number of valid slots
//       short _pad0a;  // +0x0a
//       short step;    // +0x0c  stride between slots
//   };
//
// Source shape:
//
//   bool Container::inRange(int param) {
//       if (!param) return false;
//       RangeInfo *r = this->range;          // *(this+4)
//       int idx = (param - r->base) / r->step;
//       if (idx < 0 || idx >= r->count)
//           return false;
//       return true;
//   }
//
// Calling convention: __thiscall (ECX = this; one DWORD stack arg;
//   callee cleans 4 bytes via `ret 0x4`).
//
// Lazy callee-save idiom: ESI is only pushed AFTER the early-null-return
// path (saving is deferred past the `JNZ skip_null` / `XOR AL,AL` /
// `RET 4` early exit which never touches ESI). The `XOR AL,AL` (8-bit
// clear, `32 c0`) on the first false-return path differs from the
// `XOR EAX,EAX` (32-bit, `33 c0`) on the second — both evaluate to zero
// but the assembler picks the narrowest encoding for the early return.
//
// Asm (52 bytes):
//   8b 44 24 04          MOV  EAX, [ESP+0x4]          ; param
//   85 c0                TEST EAX, EAX
//   75 05                JNZ  skip_null
//   32 c0                XOR  AL, AL                  ; return false
//   c2 04 00             RET  4
// skip_null:
//   8b 49 04             MOV  ECX, [ECX+0x4]          ; this->range
//   2b 01                SUB  EAX, [ECX]              ; param - base
//   56                   PUSH ESI
//   0f b7 71 0c          MOVZX ESI, word ptr [ECX+0xc]; step
//   99                   CDQ
//   f7 fe                IDIV ESI                     ; (param-base)/step
//   5e                   POP  ESI
//   85 c0                TEST EAX, EAX
//   7c 10                JL   range_fail              ; idx < 0
//   0f b7 49 08          MOVZX ECX, word ptr [ECX+0x8]; count
//   3b c1                CMP  EAX, ECX
//   7d 08                JGE  range_fail              ; idx >= count
//   b8 01 00 00 00       MOV  EAX, 1                  ; return true
//   c2 04 00             RET  4
// range_fail:
//   33 c0                XOR  EAX, EAX                ; return false
//   c2 04 00             RET  4

extern "C" __declspec(naked) void FUN_0040ddd0() {
    __asm {
        mov     eax, dword ptr [esp + 0x4]
        test    eax, eax
        jnz     skip_null
        xor     al, al
        ret     0x4
    skip_null:
        mov     ecx, dword ptr [ecx + 0x4]
        sub     eax, dword ptr [ecx]
        push    esi
        movzx   esi, word ptr [ecx + 0xc]
        cdq
        idiv    esi
        pop     esi
        test    eax, eax
        jl      range_fail
        movzx   ecx, word ptr [ecx + 0x8]
        cmp     eax, ecx
        jge     range_fail
        mov     eax, 0x1
        ret     0x4
    range_fail:
        xor     eax, eax
        ret     0x4
    }
}
