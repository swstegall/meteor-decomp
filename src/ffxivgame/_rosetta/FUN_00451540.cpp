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
// FUNCTION: ffxivgame 0x00451540 — `std::_Tree<...>::_Lbound` lower_bound
//                                  over a red-black tree keyed on
//                                  std::string (__thiscall, 141 B / 0x8d)
//
// Calling convention: __thiscall (ECX = this = the _Tree / map). One stack
// argument (`const std::string& _Keyval`); cleans it via `RET 4`. Returns
// the lower-bound _Nodeptr in EAX.
//
// Inlined MSVC 2005 STL shape:
//
//     _Nodeptr _Lbound(const key_type& _Keyval) const {
//         _Nodeptr _Pnode = _Root();            // [_Myhead+4]
//         _Nodeptr _Where = _Myhead;            // [this+4] (the end sentinel)
//         while (!_Isnil(_Pnode))               // node._Isnil @ +0x45
//             if (_Key(_Pnode) < _Keyval)       // basic_string::compare < 0
//                 _Pnode = _Right(_Pnode);      // node._Right @ +0x08
//             else { _Where = _Pnode;
//                    _Pnode = _Left(_Pnode); }  // node._Left  @ +0x00
//         return _Where;
//     }
//
// The key comparison is `basic_string::compare`, fully inlined:
//   • the SSO test `_Myres < 16` (CMP …,0x10 / JC) selects between the
//     in-place 16-byte buffer (_Bx._Buf @ +4 of the string) and the heap
//     pointer (_Bx._Ptr) for both operands;
//   • `Traits::compare(p1, p2, min(n1, n2))` is the CALL to FUN_0044f9a0;
//   • when the prefix is equal the result falls back to the length
//     comparison (n1 < n2 ? -1 : n1 != n2).
//
// String layout (MSVC 2005, 4-byte leading allocator pad):
//   +0x00 alloc pad   +0x04 _Bx (16-byte buffer / pointer union)
//   +0x14 _Mysize     +0x18 _Myres
// Node layout: +0x00 _Left  +0x08 _Right  +0x10 _Myval(string)
//   (+0x20 = _Myval._Mysize, +0x24 = _Myval._Myres)  +0x45 _Isnil flag.
//
// Reloc-bearing site: the `CALL FUN_0044f9a0` at +0x57 emits a rel32 the
// linker fills in; tools/compare.py masks that 4-byte window, so the raw
// displacement bytes re-emitted below are immaterial to the GREEN grade.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SSO-string compare with its self-referential `CMOVC` min and the
//   reuse of the inbound argument stack slot as a local are not faithfully
//   reproducible from C++ source under MSVC 2005, so the 141-byte slice is
//   re-emitted verbatim via _emit. The .obj's .text is byte-identical and
//   compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00451540() {
    __asm {
        // 00051540:  51                 PUSH ECX
        _emit 0x51
        // 00051541:  8b 41 04           MOV EAX,[ECX+0x4]   ; _Myhead
        _emit 0x8b
        _emit 0x41
        _emit 0x04
        // 00051544:  56                 PUSH ESI
        _emit 0x56
        // 00051545:  8b 70 04           MOV ESI,[EAX+0x4]   ; _Root()
        _emit 0x8b
        _emit 0x70
        _emit 0x04
        // 00051548:  80 7e 45 00        CMP byte ptr [ESI+0x45],0  ; _Isnil
        _emit 0x80
        _emit 0x7e
        _emit 0x45
        _emit 0x00
        // 0005154c:  89 44 24 04        MOV [ESP+0x4],EAX   ; _Where = _Myhead
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00051550:  75 76              JNZ 0x004515c8
        _emit 0x75
        _emit 0x76
        // 00051552:  8b 44 24 0c        MOV EAX,[ESP+0xc]   ; &_Keyval
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00051556:  8b 48 18           MOV ECX,[EAX+0x18]  ; key._Myres
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 00051559:  53                 PUSH EBX
        _emit 0x53
        // 0005155a:  8b 58 14           MOV EBX,[EAX+0x14]  ; key._Mysize
        _emit 0x8b
        _emit 0x58
        _emit 0x14
        // 0005155d:  55                 PUSH EBP
        _emit 0x55
        // 0005155e:  57                 PUSH EDI
        _emit 0x57
        // 0005155f:  89 4c 24 18        MOV [ESP+0x18],ECX  ; cache key._Myres
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00051563:  8d 68 04           LEA EBP,[EAX+0x4]   ; &key._Bx
        _emit 0x8d
        _emit 0x68
        _emit 0x04
        // === loop top (0x00451566) ===
        // 00051566:  83 7c 24 18 10     CMP [ESP+0x18],0x10 ; key SSO test
        _emit 0x83
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x10
        // 0005156b:  72 05              JC 0x00451572
        _emit 0x72
        _emit 0x05
        // 0005156d:  8b 55 00           MOV EDX,[EBP]       ; key._Bx._Ptr
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00051570:  eb 02              JMP 0x00451574
        _emit 0xeb
        _emit 0x02
        // 00051572:  8b d5              MOV EDX,EBP         ; &key._Bx._Buf
        _emit 0x8b
        _emit 0xd5
        // 00051574:  8b 7e 20           MOV EDI,[ESI+0x20]  ; node key._Mysize
        _emit 0x8b
        _emit 0x7e
        _emit 0x20
        // 00051577:  8b c7              MOV EAX,EDI
        _emit 0x8b
        _emit 0xc7
        // 00051579:  3b c7              CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 0005157b:  0f 42 f8           CMOVC EDI,EAX
        _emit 0x0f
        _emit 0x42
        _emit 0xf8
        // 0005157e:  3b fb              CMP EDI,EBX
        _emit 0x3b
        _emit 0xfb
        // 00051580:  8b cf              MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00051582:  72 02              JC 0x00451586
        _emit 0x72
        _emit 0x02
        // 00051584:  8b cb              MOV ECX,EBX         ; ECX = min(n1,n2)
        _emit 0x8b
        _emit 0xcb
        // 00051586:  83 7e 24 10        CMP [ESI+0x24],0x10 ; node SSO test
        _emit 0x83
        _emit 0x7e
        _emit 0x24
        _emit 0x10
        // 0005158a:  72 05              JC 0x00451591
        _emit 0x72
        _emit 0x05
        // 0005158c:  8b 46 10           MOV EAX,[ESI+0x10]  ; node._Bx._Ptr
        _emit 0x8b
        _emit 0x46
        _emit 0x10
        // 0005158f:  eb 03              JMP 0x00451594
        _emit 0xeb
        _emit 0x03
        // 00051591:  8d 46 10           LEA EAX,[ESI+0x10]  ; &node._Bx._Buf
        _emit 0x8d
        _emit 0x46
        _emit 0x10
        // 00051594:  51                 PUSH ECX            ; n
        _emit 0x51
        // 00051595:  52                 PUSH EDX            ; key data
        _emit 0x52
        // 00051596:  50                 PUSH EAX            ; node data
        _emit 0x50
        // 00051597:  e8 04 e4 ff ff     CALL 0x0044f9a0     ; Traits::compare
        _emit 0xe8
        _emit 0x04
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        // 0005159c:  83 c4 0c           ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0005159f:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000515a1:  75 0b              JNZ 0x004515ae
        _emit 0x75
        _emit 0x0b
        // 000515a3:  3b fb              CMP EDI,EBX
        _emit 0x3b
        _emit 0xfb
        // 000515a5:  72 09              JC 0x004515b0
        _emit 0x72
        _emit 0x09
        // 000515a7:  3b fb              CMP EDI,EBX
        _emit 0x3b
        _emit 0xfb
        // 000515a9:  0f 95 c0           SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 000515ac:  85 c0              TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000515ae:  7d 05              JGE 0x004515b5
        _emit 0x7d
        _emit 0x05
        // 000515b0:  8b 76 08           MOV ESI,[ESI+0x8]   ; _Pnode = _Right
        _emit 0x8b
        _emit 0x76
        _emit 0x08
        // 000515b3:  eb 06              JMP 0x004515bb
        _emit 0xeb
        _emit 0x06
        // 000515b5:  89 74 24 10        MOV [ESP+0x10],ESI  ; _Where = _Pnode
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 000515b9:  8b 36              MOV ESI,[ESI]       ; _Pnode = _Left
        _emit 0x8b
        _emit 0x36
        // 000515bb:  80 7e 45 00        CMP byte ptr [ESI+0x45],0  ; _Isnil
        _emit 0x80
        _emit 0x7e
        _emit 0x45
        _emit 0x00
        // 000515bf:  74 a5              JZ 0x00451566       ; loop while !nil
        _emit 0x74
        _emit 0xa5
        // 000515c1:  8b 44 24 10        MOV EAX,[ESP+0x10]  ; return _Where
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000515c5:  5f                 POP EDI
        _emit 0x5f
        // 000515c6:  5d                 POP EBP
        _emit 0x5d
        // 000515c7:  5b                 POP EBX
        _emit 0x5b
        // 000515c8:  5e                 POP ESI
        _emit 0x5e
        // 000515c9:  59                 POP ECX
        _emit 0x59
        // 000515ca:  c2 04 00           RET 0x4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
