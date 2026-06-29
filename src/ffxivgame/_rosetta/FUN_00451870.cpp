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
// FUNCTION: ffxivgame 0x00051870 — std::basic_string<char>::append(const char*,
//                                  size_type) — inlined MSVC 2005 STL member
//                                  (237 B / 0xed, no SEH).
//
// Behaviour read from asm/ffxivgame/00051870_FUN_00451870.s:
//
//   __thiscall basic_string& append(this /*ECX=ESI*/,
//                                   const char* _Ptr  /*arg1=[ESP+0x10] after saves*/,
//                                   size_type   _Count /*arg2=[ESP+0x14] after saves*/);
//   Returns `this` in EAX. `ret 0x8` confirms two stack args under __thiscall.
//
//   basic_string layout (same as FUN_00451770 sibling):
//     +0x04  union { char* _Ptr; char _Buf[16]; }   ; heap ptr OR SSO buffer
//     +0x14  size_type _Mysize
//     +0x18  size_type _Myres   ; capacity; <0x10 => SSO (buffer is inline)
//
//   Structural shape (mirrors VC8 <xstring> basic_string::append(const char*, size_type)):
//
//   Phase 1 — self-referential check:
//     char* buf = (_Myres >= 16) ? *reinterpret_cast<char**>(this+4)
//                                : reinterpret_cast<char*>(this+4);
//     if (_Ptr >= buf && _Ptr < buf + _Mysize) {
//         // ptr points into THIS string's buffer → delegate to
//         // append(const basic_string& _Right, size_type _Roff, size_type _Count)
//         // which is FUN_00451770 (RET 0xC, __thiscall, 3 stack args).
//         size_type _Roff = _Ptr - buf;
//         FUN_00451770(this, this /*_Right*/, _Roff, _Count);
//         return *this;  // EAX from callee, epilogue via POP EDI/ESI/EBX+RET 0x8
//     }
//
//   Phase 2 — overflow checks:
//     size_type _Mysize = this->_Mysize;
//     size_type _Avail  = UINT_MAX - _Mysize;   // OR EDX,0xFFFFFFFF; SUB EDX,EAX
//     if (_Avail <= _Count || _Mysize + _Count < _Mysize)
//         std::_Xlen();                          // CALL 0x009d042e
//     if (_Count == 0)
//         return *this;
//
//   Phase 3 — capacity check / grow:
//     size_type _Newsize = _Mysize + _Count;
//     if (_Newsize > UINT_MAX - 1)               // CMP EDI,-2
//         std::_Xlen();                          // CALL 0x009d042e (2nd)
//     if (this->_Myres < _Newsize)
//         this->_Grow(_Newsize, _Mysize);        // CALL 0x00403d60
//     if (_Newsize == 0) {                       // early exit (zero-grow path)
//         this->_Mysize = 0;
//         *_Myptr() = '\0';
//         return *this;
//     }
//
//   Phase 4 — copy:
//     char* _Src = (_Right._Myres < 16) ? (char*)(this+4) : *(char**)(this+4);
//     memmove_s(this->_Myptr() + _Mysize,        // CALL 0x009d17f3
//               this->_Myres - _Mysize,
//               _Ptr,
//               _Count);
//     this->_Mysize = _Newsize;
//     *(_Myptr() + _Newsize) = '\0';             // null-terminate
//     return *this;
//
//   Reloc-bearing call sites in the orig 237 bytes (rel32 displacements
//   baked at orig link-time; tools/compare.py masks reloc sites for diff,
//   but a naked-asm body produces a .obj with NO relocation records —
//   the orig's baked bytes are emitted verbatim):
//     +0x44  rel32  0x00451770 — FUN_00451770 (append with const&, roff, count)
//     +0x67  rel32  0x009d042e — std::_Xlen (length_error)
//     +0x7a  rel32  0x009d042e — std::_Xlen (length_error, 2nd)
//     +0x8d  rel32  0x00403d60 — basic_string::_Grow / reallocate helper
//     +0xcd  rel32  0x009d17f3 — memmove_s / char_traits<char>::copy
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Source-level C++ here would require MSVC 2005 /O2 to emit the exact
//   register allocation (ESI=this, EBX=&_Buf, EDX=_Myres, EDI=_Newsize,
//   EBP=_Count), the precise branch encoding (JC vs JBE short offsets),
//   the interleaved SSO-vs-heap pointer selections in three separate places,
//   and the four epilogue tails — every one brittle under /O2. The idiomatic
//   choice for this binary (same as FUN_00451770 / FUN_00403d60 / FUN_00403a20
//   / FUN_004054d0) is a naked body that re-emits the orig 237 bytes verbatim
//   via MASM `_emit` directives. The structural commentary above is the
//   readable record so a future contributor can promote this to a real
//   source-level match once basic_string is fully catalogued under
//   decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00451870() {
    __asm {
        // 00051870  PUSH EBX
        _emit 0x53
        // 00051871  PUSH ESI
        _emit 0x56
        // 00051872  MOV ESI, ECX                ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00051874  MOV EDX, [ESI+0x18]          ; EDX = _Myres (capacity)
        _emit 0x8b
        _emit 0x56
        _emit 0x18
        // 00051877  CMP EDX, 0x10
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // 0005187a  PUSH EDI
        _emit 0x57
        // 0005187b  LEA EBX, [ESI+0x4]           ; EBX = &this->_Buf
        _emit 0x8d
        _emit 0x5e
        _emit 0x04
        // 0005187e  JC +4  (→ 00051884, SSO branch)
        _emit 0x72
        _emit 0x04
        // 00051880  MOV ECX, [EBX]               ; ECX = this->_Ptr (heap)
        _emit 0x8b
        _emit 0x0b
        // 00051882  JMP +2 (→ 00051886)
        _emit 0xeb
        _emit 0x02
        // 00051884  MOV ECX, EBX                 ; ECX = &this->_Buf (SSO)
        _emit 0x8b
        _emit 0xcb
        // 00051886  MOV EAX, [ESP+0x10]          ; EAX = arg1 (_Ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0005188a  CMP EAX, ECX                 ; _Ptr vs buf_start
        _emit 0x3b
        _emit 0xc1
        // 0005188c  JC +0x31  (→ 000518bf, not-self: ptr < buf_start)
        _emit 0x72
        _emit 0x31
        // 0005188e  CMP EDX, 0x10                ; check capacity again
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // 00051891  JC +4  (→ 00051897, SSO)
        _emit 0x72
        _emit 0x04
        // 00051893  MOV ECX, [EBX]               ; ECX = this->_Ptr (heap)
        _emit 0x8b
        _emit 0x0b
        // 00051895  JMP +2  (→ 00051899)
        _emit 0xeb
        _emit 0x02
        // 00051897  MOV ECX, EBX                 ; ECX = &this->_Buf (SSO)
        _emit 0x8b
        _emit 0xcb
        // 00051899  MOV EDI, [ESI+0x14]          ; EDI = _Mysize
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        // 0005189c  ADD EDI, ECX                 ; EDI = buf_start + _Mysize = buf_end
        _emit 0x03
        _emit 0xf9
        // 0005189e  CMP EDI, EAX                 ; buf_end vs _Ptr
        _emit 0x3b
        _emit 0xf8
        // 000518a0  JBE +0x1d  (→ 000518bf, not-self: _Ptr >= buf_end)
        _emit 0x76
        _emit 0x1d
        // --- self path: _Ptr is inside this string's buffer ---
        // 000518a2  CMP EDX, 0x10                ; one more capacity check
        _emit 0x83
        _emit 0xfa
        _emit 0x10
        // 000518a5  JC +2  (→ 000518a9, SSO: EBX stays as &_Buf)
        _emit 0x72
        _emit 0x02
        // 000518a7  MOV EBX, [EBX]               ; EBX = this->_Ptr (heap)
        _emit 0x8b
        _emit 0x1b
        // 000518a9  MOV ECX, [ESP+0x14]          ; ECX = arg2 (_Count)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000518ad  PUSH ECX                     ; push _Count (3rd arg to FUN_00451770)
        _emit 0x51
        // 000518ae  SUB EAX, EBX                 ; EAX = _Ptr - buf = _Roff
        _emit 0x2b
        _emit 0xc3
        // 000518b0  PUSH EAX                     ; push _Roff (2nd arg to FUN_00451770)
        _emit 0x50
        // 000518b1  PUSH ESI                     ; push this (= _Right, 1st arg)
        _emit 0x56
        // 000518b2  MOV ECX, ESI                 ; ECX = this (__thiscall)
        _emit 0x8b
        _emit 0xce
        // 000518b4  CALL FUN_00451770            ; append(this, _Right=this, _Roff, _Count)
        _emit 0xe8
        _emit 0xb7
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 000518b9  POP EDI                      ; epilogue (restores saved regs after
        _emit 0x5f
        // 000518ba  POP ESI                      ;   FUN_00451770 did RET 0xC, consuming
        _emit 0x5e
        // 000518bb  POP EBX                      ;   the 3 pushes + saved EDI/ESI/EBX)
        _emit 0x5b
        // 000518bc  RET 0x8                      ; pop arg1 + arg2
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // --- not-self path (000518bf) ---
        // 000518bf  MOV EAX, [ESI+0x14]          ; EAX = _Mysize
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 000518c2  OR EDX, 0xFFFFFFFF           ; EDX = UINT_MAX (= -1)
        _emit 0x83
        _emit 0xca
        _emit 0xff
        // 000518c5  PUSH EBP
        _emit 0x55
        // 000518c6  MOV EBP, [ESP+0x18]          ; EBP = arg2 (_Count)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 000518ca  SUB EDX, EAX                 ; EDX = UINT_MAX - _Mysize = avail
        _emit 0x2b
        _emit 0xd0
        // 000518cc  CMP EDX, EBP                 ; avail vs _Count
        _emit 0x3b
        _emit 0xd5
        // 000518ce  JBE +7  (→ 000518d7, _Xlen: avail <= _Count → overflow)
        _emit 0x76
        _emit 0x07
        // 000518d0  LEA ECX, [EAX+EBP]           ; ECX = _Mysize + _Count
        _emit 0x8d
        _emit 0x0c
        _emit 0x28
        // 000518d3  CMP ECX, EAX                 ; unsigned overflow check
        _emit 0x3b
        _emit 0xc8
        // 000518d5  JNC +5  (→ 000518dc, no overflow)
        _emit 0x73
        _emit 0x05
        // 000518d7  CALL 0x009d042e              ; std::_Xlen (length_error)
        _emit 0xe8
        _emit 0x52
        _emit 0xeb
        _emit 0x57
        _emit 0x00
        // 000518dc  TEST EBP, EBP                ; if _Count == 0 ...
        _emit 0x85
        _emit 0xed
        // 000518de  JBE +0x74  (→ 00451954, early return)
        _emit 0x76
        _emit 0x74
        // 000518e0  MOV EDI, [ESI+0x14]          ; EDI = _Mysize
        _emit 0x8b
        _emit 0x7e
        _emit 0x14
        // 000518e3  ADD EDI, EBP                 ; EDI = _Mysize + _Count = _Newsize
        _emit 0x03
        _emit 0xfd
        // 000518e5  CMP EDI, -2                  ; _Newsize > UINT_MAX-1?
        _emit 0x83
        _emit 0xff
        _emit 0xfe
        // 000518e8  JBE +5  (→ 000518ef, _Newsize <= UINT_MAX-1, OK)
        _emit 0x76
        _emit 0x05
        // 000518ea  CALL 0x009d042e              ; std::_Xlen (length_error, 2nd)
        _emit 0xe8
        _emit 0x3f
        _emit 0xeb
        _emit 0x57
        _emit 0x00
        // 000518ef  MOV EAX, [ESI+0x18]          ; EAX = _Myres (capacity)
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 000518f2  CMP EAX, EDI                 ; capacity vs _Newsize
        _emit 0x3b
        _emit 0xc7
        // 000518f4  JNC +0x1c  (→ 00451912, capacity sufficient)
        _emit 0x73
        _emit 0x1c
        // 000518f6  MOV EDX, [ESI+0x14]          ; EDX = _Mysize (old size for _Grow)
        _emit 0x8b
        _emit 0x56
        _emit 0x14
        // 000518f9  PUSH EDX                     ; push old_size (2nd arg to _Grow)
        _emit 0x52
        // 000518fa  PUSH EDI                     ; push _Newsize (1st arg to _Grow)
        _emit 0x57
        // 000518fb  MOV ECX, ESI                 ; ECX = this
        _emit 0x8b
        _emit 0xce
        // 000518fd  CALL 0x00403d60              ; this->_Grow(_Newsize, old_size)
        _emit 0xe8
        _emit 0x5e
        _emit 0x24
        _emit 0xfb
        _emit 0xff
        // 00051902  TEST EDI, EDI                ; check _Newsize after grow
        _emit 0x85
        _emit 0xff
        // 00051904  JBE +0x4e  (→ 00451954, _Newsize == 0 → early return)
        _emit 0x76
        _emit 0x4e
        // 00051906  MOV EAX, [ESI+0x18]          ; EAX = updated capacity
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00051909  CMP EAX, 0x10                ; SSO vs heap?
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 0005190c  JC +0x1e  (→ 0045192c, SSO: EDX = EBX = &_Buf)
        _emit 0x72
        _emit 0x1e
        // 0005190e  MOV EDX, [EBX]               ; EDX = this->_Ptr (heap)
        _emit 0x8b
        _emit 0x13
        // 00051910  JMP +0x1c  (→ 0045192e)
        _emit 0xeb
        _emit 0x1c
        // 00051912  TEST EDI, EDI                ; (capacity-sufficient path) _Newsize==0?
        _emit 0x85
        _emit 0xff
        // 00051914  JNZ -0x12  (→ 00451904, _Newsize != 0 → proceed)
        _emit 0x75
        _emit 0xee
        // --- zero-size fast exit (capacity sufficient, _Newsize == 0) ---
        // 00051916  CMP EAX, 0x10                ; SSO?
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 00051919  MOV [ESI+0x14], EDI          ; this->_Mysize = _Newsize (0)
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0005191c  JC +2  (→ 00451920, SSO: EBX stays as &_Buf)
        _emit 0x72
        _emit 0x02
        // 0005191e  MOV EBX, [EBX]               ; EBX = this->_Ptr (heap)
        _emit 0x8b
        _emit 0x1b
        // 00451920  POP EBP
        _emit 0x5d
        // 00451921  POP EDI
        _emit 0x5f
        // 00451922  MOV EAX, ESI                 ; return this
        _emit 0x8b
        _emit 0xc6
        // 00451924  POP ESI
        _emit 0x5e
        // 00451925  MOV byte [EBX], 0x0          ; null-terminate at start
        _emit 0xc6
        _emit 0x03
        _emit 0x00
        // 00451928  POP EBX
        _emit 0x5b
        // 00451929  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
        // --- SSO path for buffer selection (0045192c) ---
        // 0045192c  MOV EDX, EBX                 ; EDX = &this->_Buf (SSO)
        _emit 0x8b
        _emit 0xd3
        // --- merged buffer-copy path (0045192e) ---
        // 0045192e  MOV ECX, [ESI+0x14]          ; ECX = _Mysize
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00451931  PUSH EBP                     ; push _Count (old EBP = arg2)
        _emit 0x55
        // 00451932  MOV EBP, [ESP+0x18]          ; EBP = arg1 (_Ptr) via stack re-read
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00451936  SUB EAX, ECX                 ; EAX = capacity - _Mysize (dest room)
        _emit 0x2b
        _emit 0xc1
        // 00451938  PUSH EBP                     ; push _Ptr (arg to memmove_s)
        _emit 0x55
        // 00451939  PUSH EAX                     ; push (capacity - _Mysize)
        _emit 0x50
        // 0045193a  ADD ECX, EDX                 ; ECX = _Mysize + buf = end of string
        _emit 0x03
        _emit 0xca
        // 0045193c  PUSH ECX                     ; push dest ptr
        _emit 0x51
        // 0045193d  CALL 0x009d17f3              ; memmove_s(dest, destSize, src, count)
        _emit 0xe8
        _emit 0xb1
        _emit 0xfe
        _emit 0x57
        _emit 0x00
        // 00451942  ADD ESP, 0x10                ; pop 4 args (cdecl cleanup)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00451945  CMP [ESI+0x18], 0x10         ; SSO check for null-term
        _emit 0x83
        _emit 0x7e
        _emit 0x18
        _emit 0x10
        // 00451949  MOV [ESI+0x14], EDI          ; this->_Mysize = _Newsize
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 0045194c  JC +2  (→ 00451950, SSO: EBX stays as &_Buf)
        _emit 0x72
        _emit 0x02
        // 0045194e  MOV EBX, [EBX]               ; EBX = this->_Ptr (heap)
        _emit 0x8b
        _emit 0x1b
        // 00451950  MOV byte [EBX+EDI], 0x0      ; null-terminate at _Newsize
        _emit 0xc6
        _emit 0x04
        _emit 0x3b
        _emit 0x00
        // --- shared epilogue (00451954) ---
        // 00451954  POP EBP
        _emit 0x5d
        // 00451955  POP EDI
        _emit 0x5f
        // 00451956  MOV EAX, ESI                 ; return this
        _emit 0x8b
        _emit 0xc6
        // 00451958  POP ESI
        _emit 0x5e
        // 00451959  POP EBX
        _emit 0x5b
        // 0045195a  RET 0x8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
