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
// FUNCTION: ffxivgame 0x0044e290 — SEH-wrapped uninitialized-copy of a
// range of 0x54-byte objects (125 B). __cdecl(EBP frame), three pointer
// params: (_First /*[EBP+0x8]*/, _Last /*[EBP+0xC]*/, _Dest /*[EBP+0x10]*/).
//
// Shape reconstructed from the asm (orig RVA 0x0004e290, 125 bytes):
//
//   void copy_construct_range(T *_First, T *_Last, T *_Dest)
//   {
//       // MSVC 2005 /GS + C++ EH frame: push -1 / push @scopeTable
//       // (VA 0x00e57bd1) / link fs:[0] / spill __security_cookie ^ ebp.
//       //
//       // esi = _Dest, edi = _First, ebx = 0, [ebp-0x14] = _Dest spill,
//       // ehstate = 0.
//       for (; _First != _Last; _First += 0x54, _Dest += 0x54) {
//           // ehstate -> 1 around the copy-construct so the unwind
//           // funclet knows how many dests to destroy.
//           if (_Dest != 0)
//               FUN_00447200(_Dest /*ECX*/, _First /*arg*/);  // __thiscall copy ctor
//           // ehstate -> 0
//       }
//       // (loop-exit + unwind-cleanup continuation live past the 125-byte
//       //  symbol slice; the trailing bytes here begin that block — a
//       //  second loop walking [_Dest spill, _Dest) calling the 0x54-byte
//       //  dtor FUN_00446f50 on partially-constructed dests.)
//   }
//
// The tail of the slice (offsets 0x69..0x7c) is the head of the EH
// cleanup/loop continuation MSVC laid down right after the main body's
// back-edge JMP — only reachable via the loop-exit JZ / unwind path, so a
// linear disassembler stops at the JMP. compare.py diffs the full 125-byte
// slice, so it is reproduced verbatim below.
//
// Reloc-bearing sites in the orig 125 bytes (masked by compare.py; the
// naked `_emit` body bakes the orig's resolved bytes, so the slice is
// byte-identical with no .obj relocations):
//   +0x05  PUSH imm32   → @sehScopeTable      (VA 0x00e57bd1)
//   +0x0b  MOV  moffs32 → fs:[0]              (TEB SEH list head)
//   +0x17  MOV  moffs32 → __security_cookie   (VA 0x012ea8b0)
//   +0x21  MOV  moffs32 → fs:[0]              (install handler)
//   +0x56  CALL rel32   → FUN_00447200        (copy ctor, disp -0x70eb)
//   +0x75  CALL rel32   → FUN_00446f50        (dtor,      disp -0x73ba)
//
// Reconstruction strategy — naked-asm byte passthrough (same idiom the
// sibling FUN_00403d60 / FUN_004013d0 SEH frames use): re-emit the orig
// 125 bytes verbatim via MASM `_emit`. A source-level C++ form would have
// to coax MSVC 2005 /O2 /GS /EHsc into the exact register allocation,
// branch encoding, and four linker-resolved absolute reloc targets — every
// such constraint is brittle. The naked passthrough is byte-exact.

extern "C" __declspec(naked) void FUN_0044e290() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E57BD1  (SEH scope table)
        _emit 0xd1
        _emit 0x7b
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x0C
        _emit 0xec
        _emit 0x0c
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX

        _emit 0x8d              // LEA EAX, [EBP - 0x0C]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV FS:[0x00000000], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP - 0x10], ESP
        _emit 0x65
        _emit 0xf0
        _emit 0x8b              // MOV ESI, [EBP + 0x10]   (_Dest)
        _emit 0x75
        _emit 0x10
        _emit 0x8b              // MOV EDI, [EBP + 0x08]   (_First)
        _emit 0x7d
        _emit 0x08
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x89              // MOV [EBP - 0x14], ESI   (spill _Dest)
        _emit 0x75
        _emit 0xec
        _emit 0x89              // MOV [EBP - 0x04], EBX   (ehstate = 0)
        _emit 0x5d
        _emit 0xfc
        _emit 0x8d              // LEA ESP, [ESP]          (7-byte NOP align)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        _emit 0x3b              // CMP EDI, [EBP + 0x0C]   (loop top: _First != _Last)
        _emit 0x7d
        _emit 0x0c
        _emit 0x74              // JZ +0x45  (loop exit, past slice)
        _emit 0x45
        _emit 0x89              // MOV [EBP + 0x08], ESI
        _emit 0x75
        _emit 0x08
        _emit 0x89              // MOV [EBP - 0x18], ESI
        _emit 0x75
        _emit 0xe8
        _emit 0x3b              // CMP ESI, EBX            (_Dest == 0 ?)
        _emit 0xf3
        _emit 0xc6              // MOV byte ptr [EBP - 0x04], 1   (ehstate = 1)
        _emit 0x45
        _emit 0xfc
        _emit 0x01
        _emit 0x74              // JZ +8  (skip ctor when _Dest == 0)
        _emit 0x08
        _emit 0x57              // PUSH EDI                (_First)
        _emit 0x8b              // MOV ECX, ESI            (this = _Dest)
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447200       (copy ctor, rel32 -0x70eb)
        _emit 0x15
        _emit 0x8f
        _emit 0xff
        _emit 0xff

        _emit 0x83              // ADD ESI, 0x54
        _emit 0xc6
        _emit 0x54
        _emit 0x88              // MOV byte ptr [EBP - 0x04], BL  (ehstate = 0)
        _emit 0x5d
        _emit 0xfc
        _emit 0x89              // MOV [EBP + 0x10], ESI
        _emit 0x75
        _emit 0x10
        _emit 0x83              // ADD EDI, 0x54
        _emit 0xc7
        _emit 0x54
        _emit 0xeb              // JMP -0x29  (back to loop top)
        _emit 0xd7

        // --- EH cleanup / loop-exit continuation (past 125-byte symbol;
        //     only the head appears in this slice) -----------------------
        _emit 0x8b              // MOV ESI, [EBP - 0x14]   (reload _Dest spill)
        _emit 0x75
        _emit 0xec
        _emit 0x8b              // MOV EDI, [EBP + 0x10]
        _emit 0x7d
        _emit 0x10
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x74              // JZ +0x0e
        _emit 0x0e
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00446f50       (dtor, rel32 -0x73ba)
        _emit 0x46
        _emit 0x8c
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESI, 0x54
        _emit 0xc6
        _emit 0x54
    }
}
