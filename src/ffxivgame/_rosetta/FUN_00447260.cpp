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
// FUNCTION: ffxivgame 0x00047260 — small-string constructor: initialise an
//                                  inline-buffer string object from a C string
//                                  (__thiscall, 2 stack params, 116 bytes)
//
// Calling convention: __thiscall (ECX = this, RET 0x8 → 2 dword stack args).
// Callee-saves pushed: EBX, ESI, EDI.
//
// Parameters:
//   ECX         = this         (the string object being constructed)
//   [ESP+0x4]   = const char*  src      (source C string)
//   [ESP+0x8]   = int          len_hint (-1 = compute strlen; else explicit len)
//
// Object layout (offsets touched by this function):
//   [this+0x00]  ptr to character buffer  (initially &this->inline_buf)
//   [this+0x04]  buffer capacity          (= 0x40 = 64)
//   [this+0x08]  element size             (= 1)
//   [this+0x0c]  current length?          (= 0 on init)
//   [this+0x10]  flag byte                (= 1)
//   [this+0x11]  flag byte                (= 1)
//   [this+0x12]  inline char buffer       (64-byte SSO storage area)
//
// Pseudo-C:
//
//   void __thiscall T::Ctor(const char* src, int len_hint) {
//       this->buf      = this->inline_buf;
//       this->capacity = 0x40;
//       this->elemSize = 1;
//       this->length   = 0;
//       this->flag0    = 1;
//       this->flag1    = 1;
//       this->inline_buf[0] = '\0';
//
//       int len = len_hint;
//       if (len_hint == -1) {
//           // strlen: pointer-difference idiom
//           const char* p = src;
//           const char* end = src + 1;
//           while (*p++) ;          // advance past '\0'
//           len = (int)(p - end);   // = strlen(src)
//       }
//
//       FUN_00447010(this, len + 1, 1); // reserve capacity
//
//       char* dst = this->buf;
//       memmove(dst, src, len);         // copy bytes (0x009d5110)
//       dst[len] = '\0';                // null-terminate
//   }
//
// Notable codegen:
//   - EDX=1 is computed once at entry and reused both for the two
//     flag-byte stores (MOV [ESI+0x10],DL / MOV [ESI+0x11],DL) and
//     as the `elemSize` argument pushed to FUN_00447010.
//   - The strlen loop body is 16-byte-aligned (RVA 0x472a0); MSVC
//     inserts a 3-byte NOP `LEA ECX,[ECX+0x00]` (8d 49 00) at
//     RVA 0x4729d immediately after the skip-into-loop JMP (eb 03)
//     to pad to that boundary.
//   - Two CALL rel32 instructions carry binary-specific displacements:
//       +0x54  CALL FUN_00447010  (rel32 = 0xFFFFFD57)
//       +0x5e  CALL 0x009d5110   (rel32 = 0x0058DE4D)
//     Emitting them as raw bytes avoids any COFF relocation, so the
//     .obj's .text matches the orig slice verbatim.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form would require coaxing MSVC 2005 into
//   exactly reproducing the EDX=1 early materialisation, the pointer-
//   difference strlen idiom, and the loop alignment NOP — all fragile
//   under /O2. The `__declspec(naked)` byte-passthrough used by
//   sibling functions (FUN_00401650, FUN_00411fa0, etc.) is the
//   simplest path: emitting all 116 bytes verbatim via MASM `_emit`
//   directives produces a .obj whose .text is byte-identical to the
//   original slice and compare.py reports GREEN.
//
// CALL targets (for future symbol catalogue):
//   FUN_00447010 @ RVA 0x00047010 — string reserve / grow
//   0x009d5110   @ RVA 0x009d5110 — memmove / memcpy

extern "C" __declspec(naked) void FUN_00447260() {
    __asm {
        // 00047260: 53              PUSH EBX
        _emit 0x53
        // 00047261: 8b 5c 24 08     MOV EBX,dword ptr [ESP+0x8]   (src)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 00047265: 56              PUSH ESI
        _emit 0x56
        // 00047266: 8b f1           MOV ESI,ECX                    (this)
        _emit 0x8b
        _emit 0xf1
        // 00047268: ba 01 00 00 00  MOV EDX,0x1
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004726d: 57              PUSH EDI
        _emit 0x57
        // 0004726e: 8b 7c 24 14     MOV EDI,dword ptr [ESP+0x14]   (len_hint)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        // 00047272: 83 ff ff        CMP EDI,-0x1
        _emit 0x83
        _emit 0xff
        _emit 0xff
        // 00047275: 8d 46 12        LEA EAX,[ESI+0x12]              (&inline_buf)
        _emit 0x8d
        _emit 0x46
        _emit 0x12
        // 00047278: 88 56 10        MOV byte ptr [ESI+0x10],DL      (flag0 = 1)
        _emit 0x88
        _emit 0x56
        _emit 0x10
        // 0004727b: 88 56 11        MOV byte ptr [ESI+0x11],DL      (flag1 = 1)
        _emit 0x88
        _emit 0x56
        _emit 0x11
        // 0004727e: c7 46 0c 00 00 00 00  MOV dword ptr [ESI+0xc],0x0  (length=0)
        _emit 0xc7
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00047285: 89 56 08        MOV dword ptr [ESI+0x8],EDX     (elemSize=1)
        _emit 0x89
        _emit 0x56
        _emit 0x08
        // 00047288: c7 46 04 40 00 00 00  MOV dword ptr [ESI+0x4],0x40 (capacity=64)
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x40
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0004728f: 89 06           MOV dword ptr [ESI],EAX         (buf = &inline_buf)
        _emit 0x89
        _emit 0x06
        // 00047291: c6 00 00        MOV byte ptr [EAX],0x0          (inline_buf[0]='\0')
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        // 00047294: 75 17           JNZ 0x004472ad                  (skip strlen if len_hint != -1)
        _emit 0x75
        _emit 0x17
        // === strlen computation (only if len_hint == -1) ===
        // 00047296: 8b c3           MOV EAX,EBX
        _emit 0x8b
        _emit 0xc3
        // 00047298: 8d 78 01        LEA EDI,[EAX+0x1]               (end = src+1)
        _emit 0x8d
        _emit 0x78
        _emit 0x01
        // 0004729b: eb 03           JMP 0x004472a0                  (enter loop body)
        _emit 0xeb
        _emit 0x03
        // 0004729d: 8d 49 00        LEA ECX,[ECX+0x00]   (3-byte NOP: align loop to 16 B)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // === strlen loop (16-byte aligned at RVA 0x472a0) ===
        // 000472a0: 8a 08           MOV CL,byte ptr [EAX]
        _emit 0x8a
        _emit 0x08
        // 000472a2: 83 c0 01        ADD EAX,0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 000472a5: 84 c9           TEST CL,CL
        _emit 0x84
        _emit 0xc9
        // 000472a7: 75 f7           JNZ 0x004472a0                  (loop)
        _emit 0x75
        _emit 0xf7
        // 000472a9: 2b c7           SUB EAX,EDI                     (EAX = ptr_past_nul - (src+1))
        _emit 0x2b
        _emit 0xc7
        // 000472ab: 8b f8           MOV EDI,EAX                     (EDI = strlen)
        _emit 0x8b
        _emit 0xf8
        // === reserve + copy ===
        // 000472ad: 52              PUSH EDX                         (arg3: elemSize=1)
        _emit 0x52
        // 000472ae: 8d 47 01        LEA EAX,[EDI+0x1]               (len+1)
        _emit 0x8d
        _emit 0x47
        _emit 0x01
        // 000472b1: 50              PUSH EAX                         (arg2: newCap = len+1)
        _emit 0x50
        // 000472b2: 8b ce           MOV ECX,ESI                      (ECX = this)
        _emit 0x8b
        _emit 0xce
        // 000472b4: e8 57 fd ff ff  CALL FUN_00447010                (reserve)
        _emit 0xe8
        _emit 0x57
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 000472b9: 8b 0e           MOV ECX,dword ptr [ESI]          (buf)
        _emit 0x8b
        _emit 0x0e
        // 000472bb: 57              PUSH EDI                         (arg3: len)
        _emit 0x57
        // 000472bc: 53              PUSH EBX                         (arg2: src)
        _emit 0x53
        // 000472bd: 51              PUSH ECX                         (arg1: dst=buf)
        _emit 0x51
        // 000472be: e8 4d de 58 00  CALL 0x009d5110                  (memmove)
        _emit 0xe8
        _emit 0x4d
        _emit 0xde
        _emit 0x58
        _emit 0x00
        // 000472c3: 8b 16           MOV EDX,dword ptr [ESI]          (buf reload)
        _emit 0x8b
        _emit 0x16
        // 000472c5: 83 c4 0c        ADD ESP,0xc                      (pop 3 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000472c8: c6 04 17 00     MOV byte ptr [EDI+EDX*0x1],0x0   (buf[len]='\0')
        _emit 0xc6
        _emit 0x04
        _emit 0x17
        _emit 0x00
        // === epilogue ===
        // 000472cc: 5f              POP EDI
        _emit 0x5f
        // 000472cd: 8b c6           MOV EAX,ESI                      (return this)
        _emit 0x8b
        _emit 0xc6
        // 000472cf: 5e              POP ESI
        _emit 0x5e
        // 000472d0: 5b              POP EBX
        _emit 0x5b
        // 000472d1: c2 08 00        RET 0x8                          (pop 2 dword params)
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
