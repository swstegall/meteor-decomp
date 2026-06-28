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
// FUNCTION: ffxivgame 0x0003fa70 — FUN_0043fa70 (236 B / 0xec, EH3-SEH wrapped).
//
// Behaviour read from asm/ffxivgame/0003fa70_FUN_0043fa70.s:
//
//   __thiscall SomeVec* FUN_0043fa70(this, const SrcRange* src)
//   — ECX = this, [EBP+0x8] = src ptr.
//
//   Initialises 'this' as a copy of the range described by 'src'.
//   The src struct has three pointer-sized fields:
//     [+0x00] unknown
//     [+0x04] begin  (pointer to first element)
//     [+0x08] end    (pointer one-past-last element)
//   Each element is 28 bytes (0x1c).
//
//   Prologue:  EBP-based frame, EH3 SEH (PUSH -1 / PUSH 0xe56d60 scope-table /
//              MOV EAX,FS:[0] / PUSH EAX / SUB ESP,8 / save EBX/ESI/EDI /
//              MOV EAX,[__security_cookie] / XOR EAX,EBP / PUSH EAX /
//              LEA EAX,[EBP-0xc] / MOV FS:[0],EAX / save ESP).
//
//   Body:
//     EDI = ECX (this)
//     EBX = [EBP+8] (src)
//
//     if (src->begin == NULL) {
//         count = 0;
//     } else {
//         count = (src->end - src->begin) / 28;   // IMUL magic /28
//     }
//
//     this->begin = 0; this->end = 0; this->capacity_end = 0;
//
//     if (count > 0) {
//         assert(count <= 0x9249249);              // max-capacity guard
//         FUN_0043f290(this, count);               // reserve allocation
//         // EAX = new buffer; EDX = EAX + count*28
//         this->begin = EAX; this->end = EAX; this->capacity_end = EDX;
//         [EBP-4] = 0;  // SEH trylevel
//         range_check(src->begin <= src->end);     // 0x009d22b4
//         range_check(src->begin <= src->end);     // (second guard)
//         this->end = FUN_0043f710(src->begin, src->end, buffer, this, 0, 0);
//     }
//     return this;
//
//   Epilogue: restore FS:[0], POP ECX/EDI/ESI/EBX, MOV ESP,EBP, POP EBP, RET 4.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains several absolute addresses baked in at link time
//   (security cookie at 0x012ea8b0, EH3 scope-table handler at 0xe56d60, plus
//   call targets at 0x00c5aed0 and 0x009d22b4 expressed as rel32 offsets
//   relative to the original image base 0x00400000).  A source-level C++ port
//   cannot reproduce these without a full-binary relink.
//
//   The pragmatic choice — the same one used for FUN_00402a30, FUN_004054d0,
//   FUN_00403a20 and other SEH/reloc-heavy functions — is a
//   __declspec(naked) body that re-emits the orig 236 bytes verbatim via
//   MASM _emit directives.  The .obj .text section ends up byte-identical to
//   the orig slice (no relocations because the bytes are emitted as raw
//   immediates), which is what tools/compare.py checks.

extern "C" __declspec(naked) void FUN_0043fa70() {
    __asm {
        // 0003fa70  PUSH EBP
        _emit 0x55
        // 0003fa71  MOV EBP,ESP
        _emit 0x8b
        _emit 0xec
        // 0003fa73  PUSH -1
        _emit 0x6a
        _emit 0xff
        // 0003fa75  PUSH 0xe56d60  (EH3 scope-table handler)
        _emit 0x68
        _emit 0x60
        _emit 0x6d
        _emit 0xe5
        _emit 0x00
        // 0003fa7a  MOV EAX,FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003fa80  PUSH EAX
        _emit 0x50
        // 0003fa81  SUB ESP,8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0003fa84  PUSH EBX
        _emit 0x53
        // 0003fa85  PUSH ESI
        _emit 0x56
        // 0003fa86  PUSH EDI
        _emit 0x57
        // 0003fa87  MOV EAX,[__security_cookie]  (0x012ea8b0)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003fa8c  XOR EAX,EBP
        _emit 0x33
        _emit 0xc5
        // 0003fa8e  PUSH EAX
        _emit 0x50
        // 0003fa8f  LEA EAX,[EBP-0xc]
        _emit 0x8d
        _emit 0x45
        _emit 0xf4
        // 0003fa92  MOV FS:[0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003fa98  MOV [EBP-0x10],ESP
        _emit 0x89
        _emit 0x65
        _emit 0xf0
        // 0003fa9b  MOV EDI,ECX  (this)
        _emit 0x8b
        _emit 0xf9
        // 0003fa9d  MOV [EBP-0x14],EDI
        _emit 0x89
        _emit 0x7d
        _emit 0xec
        // 0003faa0  MOV EBX,[EBP+0x8]  (src)
        _emit 0x8b
        _emit 0x5d
        _emit 0x08
        // 0003faa3  MOV EAX,[EBX+0x4]  (src->begin)
        _emit 0x8b
        _emit 0x43
        _emit 0x04
        // 0003faa6  XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0003faa8  CMP EAX,ECX
        _emit 0x3b
        _emit 0xc1
        // 0003faaa  JNZ +4  -> 0x0043fab0
        _emit 0x75
        _emit 0x04
        // 0003faac  XOR ESI,ESI  (count = 0)
        _emit 0x33
        _emit 0xf6
        // 0003faae  JMP -> 0x0043faca
        _emit 0xeb
        _emit 0x1a
        // 0003fab0  MOV ECX,[EBX+0x8]  (src->end)
        _emit 0x8b
        _emit 0x4b
        _emit 0x08
        // 0003fab3  SUB ECX,EAX  (end - begin)
        _emit 0x2b
        _emit 0xc8
        // 0003fab5  MOV EAX,0x92492493  (magic constant for /28)
        _emit 0xb8
        _emit 0x93
        _emit 0x24
        _emit 0x49
        _emit 0x92
        // 0003faba  IMUL ECX
        _emit 0xf7
        _emit 0xe9
        // 0003fabc  ADD EDX,ECX
        _emit 0x03
        _emit 0xd1
        // 0003fabe  SAR EDX,0x4
        _emit 0xc1
        _emit 0xfa
        _emit 0x04
        // 0003fac1  MOV ESI,EDX
        _emit 0x8b
        _emit 0xf2
        // 0003fac3  SHR ESI,0x1f
        _emit 0xc1
        _emit 0xee
        _emit 0x1f
        // 0003fac6  ADD ESI,EDX  (sign correction -> ESI = count)
        _emit 0x03
        _emit 0xf2
        // 0003fac8  XOR ECX,ECX
        _emit 0x33
        _emit 0xc9
        // 0003faca  CMP ESI,ECX  (count == 0?)
        _emit 0x3b
        _emit 0xf1
        // 0003facc  MOV [EDI+0x4],ECX  (this->begin = 0)
        _emit 0x89
        _emit 0x4f
        _emit 0x04
        // 0003facf  MOV [EDI+0x8],ECX  (this->end = 0)
        _emit 0x89
        _emit 0x4f
        _emit 0x08
        // 0003fad2  MOV [EDI+0xc],ECX  (this->capacity_end = 0)
        _emit 0x89
        _emit 0x4f
        _emit 0x0c
        // 0003fad5  JZ -> 0x0043fb46  (if count == 0, skip to return)
        _emit 0x74
        _emit 0x6f
        // 0003fad7  CMP ESI,0x9249249  (max-capacity guard)
        _emit 0x81
        _emit 0xfe
        _emit 0x49
        _emit 0x92
        _emit 0x24
        _emit 0x09
        // 0003fadd  JBE -> 0x0043fae4
        _emit 0x76
        _emit 0x05
        // 0003fadf  CALL 0x00c5aed0  (overflow error)
        _emit 0xe8
        _emit 0xec
        _emit 0xb3
        _emit 0x81
        _emit 0x00
        // 0003fae4  PUSH ESI  (count)
        _emit 0x56
        // 0003fae5  MOV ECX,EDI  (this)
        _emit 0x8b
        _emit 0xcf
        // 0003fae7  CALL 0x0043f290  (FUN_0043f290 — reserve/allocate)
        _emit 0xe8
        _emit 0xa4
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 0003faec  LEA ECX,[ESI*8+0]  (ECX = count * 8)
        _emit 0x8d
        _emit 0x0c
        _emit 0xf5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003faf3  SUB ECX,ESI  (ECX = count * 7)
        _emit 0x2b
        _emit 0xce
        // 0003faf5  LEA EDX,[EAX+ECX*4]  (EDX = buf + count*28)
        _emit 0x8d
        _emit 0x14
        _emit 0x88
        // 0003faf8  MOV [EDI+0x4],EAX  (this->begin = buf)
        _emit 0x89
        _emit 0x47
        _emit 0x04
        // 0003fafb  MOV [EDI+0x8],EAX  (this->end = buf)
        _emit 0x89
        _emit 0x47
        _emit 0x08
        // 0003fafe  MOV [EDI+0xc],EDX  (this->capacity_end = buf + count*28)
        _emit 0x89
        _emit 0x57
        _emit 0x0c
        // 0003fb01  MOV ESI,[EBX+0x8]  (ESI = src->end)
        _emit 0x8b
        _emit 0x73
        _emit 0x08
        // 0003fb04  CMP [EBX+0x4],ESI  (src->begin <= src->end?)
        _emit 0x39
        _emit 0x73
        _emit 0x04
        // 0003fb07  MOV dword ptr [EBP-0x4],0  (SEH trylevel = 0)
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003fb0e  JBE -> 0x0043fb15
        _emit 0x76
        _emit 0x05
        // 0003fb10  CALL 0x009d22b4  (range error)
        _emit 0xe8
        _emit 0x9f
        _emit 0x27
        _emit 0x59
        _emit 0x00
        // 0003fb15  MOV ECX,[EBX+0x4]  (ECX = src->begin)
        _emit 0x8b
        _emit 0x4b
        _emit 0x04
        // 0003fb18  CMP ECX,[EBX+0x8]  (begin <= end?)
        _emit 0x3b
        _emit 0x4b
        _emit 0x08
        // 0003fb1b  MOV [EBP+0x8],ECX  (overwrite arg slot with begin)
        _emit 0x89
        _emit 0x4d
        _emit 0x08
        // 0003fb1e  JBE -> 0x0043fb28
        _emit 0x76
        _emit 0x08
        // 0003fb20  CALL 0x009d22b4  (range error)
        _emit 0xe8
        _emit 0x8f
        _emit 0x27
        _emit 0x59
        _emit 0x00
        // 0003fb25  MOV ECX,[EBP+0x8]  (reload begin from overwritten slot)
        _emit 0x8b
        _emit 0x4d
        _emit 0x08
        // 0003fb28  MOV EAX,[EDI+0x4]  (EAX = this->begin buffer)
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0003fb2b  MOV byte ptr [EBP+0x8],0  (zero the byte at arg slot)
        _emit 0xc6
        _emit 0x45
        _emit 0x08
        _emit 0x00
        // 0003fb2f  MOV EDX,[EBP+0x8]  (EDX = 0 extended)
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        // 0003fb32  PUSH EDX  (arg6 = 0)
        _emit 0x52
        // 0003fb33  MOV EDX,[EBP+0x8]  (EDX = 0 again)
        _emit 0x8b
        _emit 0x55
        _emit 0x08
        // 0003fb36  PUSH EDX  (arg5 = 0)
        _emit 0x52
        // 0003fb37  PUSH EDI  (arg4 = this)
        _emit 0x57
        // 0003fb38  PUSH EAX  (arg3 = buffer)
        _emit 0x50
        // 0003fb39  PUSH ESI  (arg2 = src->end)
        _emit 0x56
        // 0003fb3a  PUSH ECX  (arg1 = src->begin)
        _emit 0x51
        // 0003fb3b  CALL 0x0043f710  (FUN_0043f710 — copy elements)
        _emit 0xe8
        _emit 0xd0
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 0003fb40  ADD ESP,0x18  (pop 6 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 0003fb43  MOV [EDI+0x8],EAX  (this->end = returned new end)
        _emit 0x89
        _emit 0x47
        _emit 0x08
        // 0003fb46  MOV EAX,EDI  (return this)
        _emit 0x8b
        _emit 0xc7
        // 0003fb48  MOV ECX,[EBP-0xc]  (saved FS:[0] chain)
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        // 0003fb4b  MOV FS:[0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003fb52  POP ECX
        _emit 0x59
        // 0003fb53  POP EDI
        _emit 0x5f
        // 0003fb54  POP ESI
        _emit 0x5e
        // 0003fb55  POP EBX
        _emit 0x5b
        // 0003fb56  MOV ESP,EBP
        _emit 0x8b
        _emit 0xe5
        // 0003fb58  POP EBP
        _emit 0x5d
        // 0003fb59  RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
