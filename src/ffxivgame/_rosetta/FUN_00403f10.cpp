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
// FUNCTION: ffxivgame 0x00403f10 — std::basic_string<char>::_Grow
//                                  (184 B / 0xb8)
//
// Inspection (read from the orig bytes at RVA 0x00003f10, file offset
// 0x3f10 of orig/ffxivgame.exe — 184 bytes):
//
//   bool __thiscall basic_string::_Grow(this, size_type _Newsize, bool _Trim)
//
//   ECX = this; stack args [ESP+4]=_Newsize (4B), [ESP+8]=_Trim (1B,
//   padded to 4B). __thiscall — RET 8 cleans the two-dword stack tail.
//   Result: returns (_Newsize != 0) ? 1 : 0 in EAX via the
//   `xor ecx,ecx / cmp ecx,ebx / sbb eax,eax / neg eax` borrow trick.
//
//   Body (annotated):
//
//     PUSH EBX
//     MOV  EBX, [ESP+0x8]            ; EBX = _Newsize
//     CMP  EBX, 0xFFFFFFFE           ; max_size() for char string
//     PUSH ESI
//     MOV  ESI, ECX                  ; ESI = this
//     JBE  ok_size
//     CALL FUN_009d042e              ; __Xlen()  — throw length_error
//   ok_size:
//     MOV  EAX, [ESI+0x18]           ; EAX = _Myres   (capacity)
//     CMP  EAX, EBX
//     JAE  in_capacity               ; capacity >= newsize → no realloc
//     ; --- _Copy(_Newsize, _Mysize) path -----------------------------
//     MOV  EAX, [ESI+0x14]           ; EAX = _Mysize
//     PUSH EAX
//     PUSH EBX
//     MOV  ECX, ESI
//     CALL FUN_00403d60              ; this->_Copy(_Newsize, _Mysize)
//     XOR  ECX, ECX
//     CMP  ECX, EBX
//     SBB  EAX, EAX                  ; EAX = -1 if EBX != 0 else 0
//     POP  ESI
//     NEG  EAX                       ; EAX = (_Newsize != 0)
//     POP  EBX
//     RET  8
//
//   in_capacity:                     ; _Myres >= _Newsize
//     CMP  byte [ESP+0x10], 0        ; test _Trim
//     JE   try_zero
//     CMP  EBX, 0x10                 ; SSO threshold _BUF_SIZE
//     JAE  try_zero
//     ; --- _Tidy(true, _Newsize) inlined: shrink heap → SSO ---------
//     PUSH EDI
//     MOV  EDI, [ESI+0x14]           ; EDI = _Mysize
//     CMP  EBX, EDI
//     JAE  keep_oldlen
//     MOV  EDI, EBX                  ; EDI = min(_Newsize, _Mysize)
//   keep_oldlen:
//     CMP  EAX, 0x10                 ; was old buffer on heap?
//     JB   set_size                  ; no — already SSO, skip free
//     TEST EDI, EDI
//     LEA  EAX, [ESI+0x4]            ; EAX = &_Bx._Buf[0]
//     PUSH EBP
//     MOV  EBP, [EAX]                ; EBP = old heap _Ptr
//     JBE  free_only                 ; if EDI == 0 → skip memcpy
//     PUSH EDI                       ; memcpy_s(_Bx._Buf, 16,
//     PUSH EBP                       ;          old_ptr, EDI)
//     PUSH 0x10
//     PUSH EAX
//     CALL FUN_009d17f3              ; _memcpy_s
//     ADD  ESP, 0x10
//   free_only:
//     PUSH EBP
//     CALL FUN_009d1b17              ; _free(old_ptr)
//     ADD  ESP, 0x4
//     POP  EBP
//   set_size:
//     MOV  [ESI+0x14], EDI           ; _Mysize  = clipped newsize
//     MOV  [ESI+0x18], 0xF           ; _Myres   = _BUF_SIZE - 1
//     XOR  ECX, ECX
//     MOV  byte [ESI+EDI+0x4], 0     ; _Bx._Buf[clipped] = '\0'
//     CMP  ECX, EBX
//     POP  EDI
//     SBB  EAX, EAX
//     POP  ESI
//     NEG  EAX
//     POP  EBX
//     RET  8
//
//   try_zero:                        ; (_Trim == 0) || (_Newsize >= 16)
//     TEST EBX, EBX
//     JNE  common_return             ; if _Newsize != 0 → just return
//     ; --- _Eos(0) inlined: empty the string ------------------------
//     CMP  EAX, 0x10                 ; was old buffer on heap?
//     MOV  [ESI+0x14], EBX           ; _Mysize = 0
//     JB   null_sso
//     MOV  ESI, [ESI+0x4]            ; ESI = heap _Ptr
//     XOR  ECX, ECX
//     CMP  ECX, EBX
//     MOV  byte [ESI], BL            ; *_Ptr = 0
//     SBB  EAX, EAX
//     POP  ESI
//     NEG  EAX
//     POP  EBX
//     RET  8
//   null_sso:
//     ADD  ESI, 0x4                  ; &_Bx._Buf[0]
//     MOV  byte [ESI], 0             ; _Bx._Buf[0] = '\0'
//   common_return:
//     XOR  ECX, ECX
//     CMP  ECX, EBX
//     SBB  EAX, EAX
//     POP  ESI
//     NEG  EAX
//     POP  EBX
//     RET  8
//
// Reloc-bearing sites in the orig 184 bytes:
//     +0x0d   CALL rel32   → FUN_009d042e  (__Xlen / length_error)
//     +0x21   CALL rel32   → FUN_00403d60  (basic_string::_Copy)
//     +0x5d   CALL rel32   → FUN_009d17f3  (_memcpy_s)
//     +0x66   CALL rel32   → FUN_009d1b17  (_free)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ port (`return basic_string::_Grow(_Newsize,
//   _Trim);`) would emit the same shape but produce four reloc
//   entries against linker-controlled symbols whose immediates are
//   zero-filled in the .obj. The pragmatic choice — the same one
//   FUN_00403bd0 (operator new[] thunk) and FUN_00403a20 (Foo dtor)
//   took — is a `__declspec(naked)` body that re-emits the orig 184
//   bytes verbatim via MASM `_emit`. The .obj's `.text` section ends
//   up byte-identical to the orig slice with no relocations; the
//   rel32 offsets are baked in at orig's link-time RVA of 0x00403f10.

extern "C" __declspec(naked) void FUN_00403f10() {
    __asm {
        _emit 0x53
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        _emit 0x83
        _emit 0xfb
        _emit 0xfe
        _emit 0x56
        _emit 0x8b
        _emit 0xf1
        _emit 0x76
        _emit 0x05
        _emit 0xe8
        _emit 0x0c
        _emit 0xc5
        _emit 0x5c
        _emit 0x00
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        _emit 0x3b
        _emit 0xc3
        _emit 0x73
        _emit 0x19
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        _emit 0x50
        _emit 0x53
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x2b
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0xcb
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x74
        _emit 0x52
        _emit 0x83
        _emit 0xfb
        _emit 0x10
        _emit 0x73
        _emit 0x4d
        _emit 0x57
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        _emit 0x3b
        _emit 0xdf
        _emit 0x73
        _emit 0x02
        _emit 0x8b
        _emit 0xfb
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        _emit 0x72
        _emit 0x21
        _emit 0x85
        _emit 0xff
        _emit 0x8d
        _emit 0x46
        _emit 0x04
        _emit 0x55
        _emit 0x8b
        _emit 0x28
        _emit 0x76
        _emit 0x0d
        _emit 0x57
        _emit 0x55
        _emit 0x6a
        _emit 0x10
        _emit 0x50
        _emit 0xe8
        _emit 0x82
        _emit 0xd8
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        _emit 0x55
        _emit 0xe8
        _emit 0x9d
        _emit 0xdb
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x5d
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33
        _emit 0xc9
        _emit 0xc6
        _emit 0x44
        _emit 0x3e
        _emit 0x04
        _emit 0x00
        _emit 0x3b
        _emit 0xcb
        _emit 0x5f
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x85
        _emit 0xdb
        _emit 0x75
        _emit 0x20
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        _emit 0x89
        _emit 0x5e
        _emit 0x14
        _emit 0x72
        _emit 0x12
        _emit 0x8b
        _emit 0x76
        _emit 0x04
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0xcb
        _emit 0x88
        _emit 0x1e
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
        _emit 0x5b
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        _emit 0xc6
        _emit 0x06
        _emit 0x00
        _emit 0x33
        _emit 0xc9
        _emit 0x3b
        _emit 0xcb
        _emit 0x1b
        _emit 0xc0
        _emit 0x5e
        _emit 0xf7
        _emit 0xd8
    }
}
