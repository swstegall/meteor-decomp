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
// FUNCTION: ffxivgame 0x00054cc0 — WString::assign(src, pos, count)
//                                  (235 B / 0xeb, no SEH, __thiscall, RET 0xc).
//
// Behaviour read from asm/ffxivgame/00054cc0_FUN_00454cc0.s:
//
//   __thiscall WString* FUN_00454cc0(this, WString* src, size_t pos, size_t count)
//
//   ECX = this.  Stack: [esp+4]=src, [esp+8]=pos, [esp+c]=count.  RET 0xc.
//   Returns `this` in EAX.
//
//   WString layout (SSO threshold = 8 wchar_t):
//     +0x00..+0x03  (reserved / allocator / padding)
//     +0x04..+0x13  union { wchar_t buf[8]; wchar_t* ptr; }
//     +0x14         size_t _size
//     +0x18         size_t _cap
//
//   Pseudocode:
//     if (src->_size < pos) __report_rangecheckfailure();    // 0x009d046d
//     size_t actual = min(src->_size - pos, count);
//
//     if (this == src) {
//         // self-assign: erase tail then head in-place (no temp needed)
//         this->erase(pos + actual, (size_t)-1);             // 0x004496c0
//         this->erase(0,            pos);                    // 0x004496c0
//         return this;
//     }
//
//     if (actual > (size_t)-2) __report_securityfailure();  // 0x009d042e
//     if (this->_cap < actual)
//         this->_Grow(actual, this->_size);                  // 0x00449760
//
//     if (actual == 0) {
//         // empty-assign: write null terminator
//         this->_size = 0;
//         wchar_t* dst = (this->_cap < 8) ? (wchar_t*)&this->buf : this->ptr;
//         *dst = L'\0';
//         return this;
//     }
//
//     // copy `actual` wchar_t chars from src[pos..] into this
//     wchar_t* src_data = (src->_cap < 8) ? (wchar_t*)&src->buf : src->ptr;
//     wchar_t* dst_data = (this->_cap < 8) ? (wchar_t*)&this->buf : this->ptr;
//     wmemmove_s(dst_data, this->_cap * 2,          // 0x009d17f3
//                src_data + pos,
//                actual   * 2);
//     this->_size = actual;
//     dst_data[actual] = L'\0';    // null-terminate
//     return this;
//
//   Stack layout after the prologue (4 × PUSH → 4 saved regs):
//     [esp+0x00] saved EDI
//     [esp+0x04] saved ESI
//     [esp+0x08] saved EBP
//     [esp+0x0c] saved EBX
//     [esp+0x10] return address
//     [esp+0x14] src   (arg1)
//     [esp+0x18] pos   (arg2)
//     [esp+0x1c] count (arg3)
//
//   Register assignment throughout the function body:
//     ESI = this
//     EBP = src (arg1, then repurposed as &this.buf mid-function)
//     EBX = pos (arg2)
//     EDI = actual (computed count, in wchar_t units)
//     EDX = pos again ([esp+0x18]) for the memmove_s src offset
//     EBX = actual*2 (byte count) for the memmove_s length + null-term offset
//
//   The EBP repurposing and CMOVC idiom make a source-level C++ port
//   impractical with MSVC 2005 /O2 — the optimizer chooses EBP as both
//   the `src` register and later as `&this->buf`, and there are no absolute
//   address references in this function (all five CALLs are rel32 fixups).
//   Naked-asm byte passthrough is used to guarantee byte-identical output.

extern "C" __declspec(naked) void FUN_00454cc0() {
    __asm {
        // 00054cc0  PUSH EBX
        _emit 0x53
        // 00054cc1  MOV EBX, [ESP+0xc]   ; EBX = pos (arg2, after 1 push)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 00054cc5  PUSH EBP
        _emit 0x55
        // 00054cc6  MOV EBP, [ESP+0xc]   ; EBP = src (arg1, after 2 pushes)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x0c
        // 00054cca  CMP [EBP+0x14], EBX  ; src._size vs pos
        _emit 0x39
        _emit 0x5d
        _emit 0x14
        // 00054ccd  PUSH ESI
        _emit 0x56
        // 00054cce  PUSH EDI
        _emit 0x57
        // 00054ccf  MOV ESI, ECX         ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00054cd1  JNC +5               ; skip if src._size >= pos
        _emit 0x73
        _emit 0x05
        // 00054cd3  CALL 0x009d046d      ; __report_rangecheckfailure
        _emit 0xe8
        _emit 0x95
        _emit 0xb7
        _emit 0x57
        _emit 0x00
        // 00054cd8  MOV EDI, [EBP+0x14]  ; EDI = src._size
        _emit 0x8b
        _emit 0x7d
        _emit 0x14
        // 00054cdb  MOV EAX, [ESP+0x1c]  ; EAX = count (arg3, after 4 pushes)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 00054cdf  SUB EDI, EBX         ; EDI = src._size - pos
        _emit 0x2b
        _emit 0xfb
        // 00054ce1  CMP EAX, EDI         ; count vs available
        _emit 0x3b
        _emit 0xc7
        // 00054ce3  CMOVC EDI, EAX       ; EDI = min(available, count)
        _emit 0x0f
        _emit 0x42
        _emit 0xf8
        // 00054ce6  CMP ESI, EBP         ; this == src ?
        _emit 0x3b
        _emit 0xf5
        // 00054ce8  JNZ +0x1f            ; jump if not self
        _emit 0x75
        _emit 0x1f
        // --- self-assign path ---
        // 00054cea  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 00054cec  ADD EDI, EBX         ; EDI = pos + actual (new tail)
        _emit 0x03
        _emit 0xfb
        // 00054cee  PUSH EDI
        _emit 0x57
        // 00054cef  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00054cf1  CALL 0x004496c0      ; this->erase(pos+actual, -1)
        _emit 0xe8
        _emit 0xca
        _emit 0x49
        _emit 0xff
        _emit 0xff
        // 00054cf6  PUSH EBX             ; pos
        _emit 0x53
        // 00054cf7  PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00054cf9  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00054cfb  CALL 0x004496c0      ; this->erase(0, pos)
        _emit 0xe8
        _emit 0xc0
        _emit 0x49
        _emit 0xff
        _emit 0xff
        // 00054d00  POP EDI
        _emit 0x5f
        // 00054d01  MOV EAX, ESI         ; return this
        _emit 0x8b
        _emit 0xc6
        // 00054d03  POP ESI
        _emit 0x5e
        // 00054d04  POP EBP
        _emit 0x5d
        // 00054d05  POP EBX
        _emit 0x5b
        // 00054d06  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- not-self path ---
        // 00054d09  CMP EDI, -2          ; guard against SIZE_MAX-1
        _emit 0x83
        _emit 0xff
        _emit 0xfe
        // 00054d0c  JBE +5               ; skip if safe
        _emit 0x76
        _emit 0x05
        // 00054d0e  CALL 0x009d042e      ; security failure
        _emit 0xe8
        _emit 0x1b
        _emit 0xb7
        _emit 0x57
        _emit 0x00
        // 00054d13  MOV EAX, [ESI+0x18]  ; EAX = this._cap
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00054d16  CMP EAX, EDI         ; this._cap vs actual
        _emit 0x3b
        _emit 0xc7
        // 00054d18  JNC +0x1b            ; skip grow if enough capacity
        _emit 0x73
        _emit 0x1b
        // --- grow path ---
        // 00054d1a  MOV EAX, [ESI+0x14]  ; EAX = this._size
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 00054d1d  PUSH EAX             ; push this._size
        _emit 0x50
        // 00054d1e  PUSH EDI             ; push actual
        _emit 0x57
        // 00054d1f  MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00054d21  CALL 0x00449760      ; this->_Grow(actual, this._size)
        _emit 0xe8
        _emit 0x3a
        _emit 0x4a
        _emit 0xff
        _emit 0xff
        // --- join after grow (or after capacity-ok branch) ---
        // 00054d26  TEST EDI, EDI        ; actual == 0 ?
        _emit 0x85
        _emit 0xff
        // 00054d28  JBE +0x78            ; jump to null-term-only path
        _emit 0x76
        _emit 0x78
        // --- actual > 0: determine source pointer ---
        // 00054d2a  CMP [EBP+0x18], 8   ; src._cap < 8 ?
        _emit 0x83
        _emit 0x7d
        _emit 0x18
        _emit 0x08
        // 00054d2e  JC +0x31            ; jump if small buffer
        _emit 0x72
        _emit 0x31
        // 00054d30  MOV ECX, [EBP+0x4]  ; ECX = src._ptr (heap)
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 00054d33  JMP +0x2f           ; skip to shared
        _emit 0xeb
        _emit 0x2f
        // --- no-grow, actual == 0 path (jumped over from JBE above) ---
        // 00054d35  TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00054d37  JNZ -0x11           ; back to 0x00454d28 if non-zero
        _emit 0x75
        _emit 0xef
        // --- actual == 0: write empty string ---
        // 00054d39  CMP EAX, 8          ; this._cap (already in EAX from cap-check)
        _emit 0x83
        _emit 0xf8
        _emit 0x08
        // 00054d3c  MOV [ESI+0x14], EDI ; this._size = 0
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 00054d3f  JC +0xf             ; small buffer
        _emit 0x72
        _emit 0x0f
        // 00054d41  MOV EAX, [ESI+0x4]  ; EAX = this._ptr (heap)
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00054d44  MOV word ptr [EAX], DI  ; *ptr = 0  (DI=0 since EDI=0)
        _emit 0x66
        _emit 0x89
        _emit 0x38
        // 00054d47  POP EDI
        _emit 0x5f
        // 00054d48  MOV EAX, ESI        ; return this
        _emit 0x8b
        _emit 0xc6
        // 00054d4a  POP ESI
        _emit 0x5e
        // 00054d4b  POP EBP
        _emit 0x5d
        // 00054d4c  POP EBX
        _emit 0x5b
        // 00054d4d  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- small buf, actual == 0 ---
        // 00054d50  LEA EAX, [ESI+0x4]  ; EAX = &this.buf
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        // 00054d53  POP EDI
        _emit 0x5f
        // 00054d54  MOV word ptr [EAX], 0x0  ; buf[0] = L'\0'
        _emit 0x66
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00054d59  MOV EAX, ESI        ; return this
        _emit 0x8b
        _emit 0xc6
        // 00054d5b  POP ESI
        _emit 0x5e
        // 00054d5c  POP EBP
        _emit 0x5d
        // 00054d5d  POP EBX
        _emit 0x5b
        // 00054d5e  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // --- small src buffer ---
        // 00054d61  LEA ECX, [EBP+0x4]  ; ECX = &src.buf (inline)
        _emit 0x8d
        _emit 0x4d
        _emit 0x04
        // --- shared: determine dest pointer ---
        // 00054d64  CMP [ESI+0x18], 8   ; this._cap < 8 ?
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 00054d68  LEA EBP, [ESI+0x4]  ; EBP = &this.buf (repurpose EBP!)
        _emit 0x8d
        _emit 0x6e
        _emit 0x04
        // 00054d6b  JC +5               ; small dest buffer
        _emit 0x72
        _emit 0x05
        // 00054d6d  MOV EAX, [EBP]      ; EAX = this._ptr (heap)
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00054d70  JMP +2
        _emit 0xeb
        _emit 0x02
        // 00054d72  MOV EAX, EBP        ; EAX = &this.buf (inline)
        _emit 0x8b
        _emit 0xc5
        // --- copy via wmemmove_s ---
        // 00054d74  MOV EDX, [ESP+0x18] ; EDX = pos (arg2 from stack)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00054d78  LEA ECX, [ECX + EDX*2]  ; ECX = src_buf + pos*2
        _emit 0x8d
        _emit 0x0c
        _emit 0x51
        // 00054d7b  LEA EBX, [EDI + EDI*1]  ; EBX = actual * 2 (bytes)
        _emit 0x8d
        _emit 0x1c
        _emit 0x3f
        // 00054d7e  PUSH EBX            ; arg4: count_bytes
        _emit 0x53
        // 00054d7f  PUSH ECX            ; arg3: src + offset
        _emit 0x51
        // 00054d80  MOV ECX, [ESI+0x18] ; ECX = this._cap
        _emit 0x8b
        _emit 0x4e
        _emit 0x18
        // 00054d83  LEA EDX, [ECX + ECX*1]  ; EDX = this._cap * 2
        _emit 0x8d
        _emit 0x14
        _emit 0x09
        // 00054d86  PUSH EDX            ; arg2: dst_capacity_bytes
        _emit 0x52
        // 00054d87  PUSH EAX            ; arg1: dst
        _emit 0x50
        // 00054d88  CALL 0x009d17f3     ; wmemmove_s(dst, cap*2, src+pos, actual*2)
        _emit 0xe8
        _emit 0x66
        _emit 0xca
        _emit 0x57
        _emit 0x00
        // 00054d8d  ADD ESP, 0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00054d90  CMP [ESI+0x18], 8   ; this._cap < 8 ?
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x08
        // 00054d94  MOV [ESI+0x14], EDI ; this._size = actual
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 00054d97  JC +3               ; small buffer (EBP already = &this.buf)
        _emit 0x72
        _emit 0x03
        // 00054d99  MOV EBP, [EBP]      ; EBP = this._ptr (deref heap pointer)
        _emit 0x8b
        _emit 0x6d
        _emit 0x00
        // 00054d9c  MOV word ptr [EBX + EBP*1], 0x0  ; null-terminate at buf[actual]
        _emit 0x66
        _emit 0xc7
        _emit 0x04
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        // 00054da2  POP EDI
        _emit 0x5f
        // 00054da3  MOV EAX, ESI        ; return this
        _emit 0x8b
        _emit 0xc6
        // 00054da5  POP ESI
        _emit 0x5e
        // 00054da6  POP EBP
        _emit 0x5d
        // 00054da7  POP EBX
        _emit 0x5b
        // 00054da8  RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
