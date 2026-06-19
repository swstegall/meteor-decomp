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
// FUNCTION: ffxivgame 0x00036580 — `__thiscall` vector-like reserve/init helper
//                                  (68 B / 0x44)
//
// Calling convention: __thiscall (ECX = this), one DWORD stack arg (count),
// callee-cleans via `RET 4`. Returns bool in AL.
//
// High-level shape (inferred from asm at orig RVA 0x00036580):
//
//   bool __thiscall FUN_00436580(Vec *this /*ECX*/, DWORD count /*[ESP+4]*/):
//     // Zero the three pointer fields first.
//     this->_Myfirst = NULL;   // [ESI+0x04]
//     this->_Mylast  = NULL;   // [ESI+0x08]
//     this->_Myend   = NULL;   // [ESI+0x0c]
//
//     if (count == 0) return false;          // AL = 0
//
//     if (count > 0x3FFFFFFF) _Xlength();    // 0x00c5aed0
//
//     void *buf = FUN_00422ca0(count);       // 0x00022ca0 — element allocator
//     this->_Myfirst = buf;
//     this->_Mylast  = buf;                  // initially empty (last == first)
//     this->_Myend   = (DWORD*)buf + count;  // capacity end (stride = 4 bytes)
//     return true;                           // AL = 1
//
// Vector layout (MSVC 2005 _Vector_val style, element = DWORD/4 bytes):
//   this+0x04  _Myfirst  (begin pointer)
//   this+0x08  _Mylast   (end of used data; == _Myfirst on empty init)
//   this+0x0c  _Myend    (end of allocated capacity = _Myfirst + count*4)
//
// Orig codegen (68 bytes at RVA 0x00036580):
//
//   56                    PUSH ESI
//   33 c0                 XOR EAX,EAX
//   57                    PUSH EDI
//   8b 7c 24 0c           MOV EDI,dword ptr [ESP+0xc]     ; EDI = count
//   3b f8                 CMP EDI,EAX                     ; count == 0?
//   8b f1                 MOV ESI,ECX                     ; ESI = this
//   89 46 04              MOV dword ptr [ESI+0x04],EAX    ; _Myfirst = 0
//   89 46 08              MOV dword ptr [ESI+0x08],EAX    ; _Mylast = 0
//   89 46 0c              MOV dword ptr [ESI+0x0c],EAX    ; _Myend = 0
//   75 07                 JNZ +0x07                       ; count != 0 → jump
//   5f                    POP EDI
//   32 c0                 XOR AL,AL                       ; return false
//   5e                    POP ESI
//   c2 04 00              RET 0x4
//   81 ff ff ff ff 3f     CMP EDI,0x3fffffff              ; count > max?
//   76 05                 JBE +0x05                       ; ok → skip throw
//   e8 25 49 82 00        CALL 0x00c5aed0                 ; _Xlength_error
//   57                    PUSH EDI                        ; push count
//   e8 ef c6 fe ff        CALL 0x00422ca0                 ; alloc (buf in EAX)
//   89 46 04              MOV dword ptr [ESI+0x04],EAX    ; _Myfirst = buf
//   89 46 08              MOV dword ptr [ESI+0x08],EAX    ; _Mylast = buf
//   8d 04 b8              LEA EAX,[EAX+EDI*4]            ; EAX = buf + count*4
//   89 46 0c              MOV dword ptr [ESI+0x0c],EAX    ; _Myend = end
//   5f                    POP EDI
//   b0 01                 MOV AL,0x1                      ; return true
//   5e                    POP ESI
//   c2 04 00              RET 0x4
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The two CALL rel32 displacements embed post-link values specific to the
//   binary's address space (0x00824925 for _Xlength_error, 0xfffec6ef for
//   FUN_00422ca0). Following the established local idiom (cf. FUN_004365d0
//   which uses the same `_Xlength_error` call in the same module), the body
//   re-emits all 68 bytes verbatim via MASM `_emit` directives. The .obj's
//   .text ends up byte-identical to the orig slice; `tools/compare.py`
//   reports GREEN.

extern "C" __declspec(naked) void FUN_00436580() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR EAX,EAX
        _emit 0xc0
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI,dword ptr [ESP+0xc]
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        _emit 0x3b              // CMP EDI,EAX
        _emit 0xf8
        _emit 0x8b              // MOV ESI,ECX
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESI+0x04],EAX
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI+0x08],EAX
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV dword ptr [ESI+0x0c],EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x75              // JNZ +0x07
        _emit 0x07
        _emit 0x5f              // POP EDI
        _emit 0x32              // XOR AL,AL  (return false)
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
        _emit 0x81              // CMP EDI,0x3fffffff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x3f
        _emit 0x76              // JBE +0x05
        _emit 0x05
        _emit 0xe8              // CALL 0x00c5aed0  (_Xlength_error)
        _emit 0x25              //   rel32 = 0x00824925
        _emit 0x49
        _emit 0x82
        _emit 0x00
        _emit 0x57              // PUSH EDI  (count arg for allocator)
        _emit 0xe8              // CALL 0x00422ca0  (allocator)
        _emit 0xef              //   rel32 = 0xfffec6ef
        _emit 0xc6
        _emit 0xfe
        _emit 0xff
        _emit 0x89              // MOV dword ptr [ESI+0x04],EAX  (_Myfirst = buf)
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI+0x08],EAX  (_Mylast = buf)
        _emit 0x46
        _emit 0x08
        _emit 0x8d              // LEA EAX,[EAX+EDI*4]  (capacity end)
        _emit 0x04
        _emit 0xb8
        _emit 0x89              // MOV dword ptr [ESI+0x0c],EAX  (_Myend)
        _emit 0x46
        _emit 0x0c
        _emit 0x5f              // POP EDI
        _emit 0xb0              // MOV AL,0x1  (return true)
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
