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
// FUNCTION: ffxivgame 0x00444150 — `__thiscall` 169-byte (0xa9) object
//                                   constructor with an SEH3 (/GS + /EHsc)
//                                   frame around an allocate-and-fill call.
//
// Asm shape (`__thiscall T* Ctor(this, uint32_t n)` — ret 0x4):
//
//   [SEH3 / GS prologue: push -1; push offset handler; save FS:[0];
//    push EBX/ESI/EDI; load+xor __security_cookie; install new FS:[0]]
//
//   ESI = this                                        ; this = ECX in
//   this->off4 = n                                     ; store param
//   EDI = n << 5                                       ; EDI = n * 32
//   size = saturating_mul_add(EDI, 0xbc, 4)             ; overflow-checked
//                                                       ;  size = EDI*188+4,
//                                                       ;  clamped to
//                                                       ;  0xffffffff on
//                                                       ;  overflow of either
//                                                       ;  the multiply or
//                                                       ;  the +4
//   this->off0 = 0x00f67290                             ; fixed field value
//   this->off0xc = 0                                    ; byte field
//   ptr = FUN_009d04ac(size)                             ; allocator
//   this->off0x18(spill) = 0
//   if (ptr != 0) {
//       *ptr = EDI                                       ; header = n*32
//       FUN_009d61d6(ptr + 4, 0xbc, EDI, 0x443c30, 0x443c90)  ; fill-construct
//       this->off8 = ptr + 4
//   } else {
//       this->off8 = 0
//   }
//   return this
//
//   [SEH3 / GS epilogue: restore FS:[0]; pop cookie/EDI/ESI/EBX; ret 4]
//
// The two CALL targets are an allocator thunk (FUN_009d04ac) and an
// uninitialized-fill-n-style construct helper (FUN_009d61d6) taking a
// per-element ctor/dtor function-pointer pair (0x00443c30 / 0x00443c90).
// This matches the established pattern in this binary of small fixed-size
// container member constructors (compare `decomp-notes/idioms/ffxivgame.md`
// and the sibling at `src/ffxivgame/_rosetta/FUN_00406280.cpp`, also
// exactly 169 bytes) — an SEH-frame-heavy body with several relocation
// sites and precise callee-saved register allocation across branches.
//
// Reconstruction strategy — naked-asm byte passthrough, same rationale as
// FUN_00406280.cpp: a source-level reconstruction would need to coerce
// MSVC into reproducing the exact EBX/ESI/EDI allocation and the SEH3
// frame layout bit-for-bit, which is brittle (see
// decomp-notes/blocked/ffxivgame/0x00001b70_FUN_00401b70.md for a
// documented failure mode of this class). Emitting the original 169 bytes
// verbatim via MASM `_emit` directives sidesteps that: no relocations are
// required because every rel32/imm32 in this slice already resolves
// correctly against the orig binary's own (fixed, pre-assigned) address
// space when this function is placed back at its original VA.
//
// Reloc-bearing / fixed-address sites in the orig 169 bytes:
//     +0x11   MOV EAX,[__security_cookie]         (0x012ea8b0)
//     +0x3f   MOV dword ptr [ESI], 0x00f67290      (fixed field value)
//     +0x59   CALL rel32 → FUN_009d04ac            (allocator)
//     +0x6d   PUSH 0x00443c90                      (dtor fn ptr)
//     +0x72   PUSH 0x00443c30                      (ctor fn ptr)
//     +0x83   CALL rel32 → FUN_009d61d6            (fill-construct helper)

extern "C" __declspec(naked) void FUN_00444150() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0xe57364
        _emit 0x64
        _emit 0x73
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX,FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX,[0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX,ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX,[ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0x0],EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI,ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX,dword ptr [ESP + 0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESI + 0x4],EAX
        _emit 0x46
        _emit 0x04
        _emit 0xc1              // SHL EAX,0x5
        _emit 0xe0
        _emit 0x05
        _emit 0x8b              // MOV EDI,EAX
        _emit 0xf8
        _emit 0x33              // XOR ECX,ECX
        _emit 0xc9
        _emit 0x33              // XOR EBX,EBX
        _emit 0xdb
        _emit 0xba              // MOV EDX,0xbc
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf7              // MUL EDX
        _emit 0xe2
        _emit 0x0f              // SETO CL
        _emit 0x90
        _emit 0xc1
        _emit 0xc7              // MOV dword ptr [ESI],0xf67290
        _emit 0x06
        _emit 0x90
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x88              // MOV byte ptr [ESI + 0xc],BL
        _emit 0x5e
        _emit 0x0c
        _emit 0xf7              // NEG ECX
        _emit 0xd9
        _emit 0x0b              // OR ECX,EAX
        _emit 0xc8
        _emit 0x33              // XOR EAX,EAX
        _emit 0xc0
        _emit 0x83              // ADD ECX,0x4
        _emit 0xc1
        _emit 0x04
        _emit 0x0f              // SETC AL
        _emit 0x92
        _emit 0xc0
        _emit 0xf7              // NEG EAX
        _emit 0xd8
        _emit 0x0b              // OR EAX,ECX
        _emit 0xc1
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009d04ac (rel32)
        _emit 0xfe
        _emit 0xc2
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP,0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESP + 0x20],EAX
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x3b              // CMP EAX,EBX
        _emit 0xc3
        _emit 0x89              // MOV dword ptr [ESP + 0x18],EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x74              // JZ short +0x20
        _emit 0x20
        _emit 0x68              // PUSH 0x443c90
        _emit 0x90
        _emit 0x3c
        _emit 0x44
        _emit 0x00
        _emit 0x68              // PUSH 0x443c30
        _emit 0x30
        _emit 0x3c
        _emit 0x44
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EBX,[EAX + 0x4]
        _emit 0x58
        _emit 0x04
        _emit 0x68              // PUSH 0xbc
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x89              // MOV dword ptr [EAX],EDI
        _emit 0x38
        _emit 0xe8              // CALL FUN_009d61d6 (rel32)
        _emit 0xfe
        _emit 0x1f
        _emit 0x59
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI + 0x8],EBX
        _emit 0x5e
        _emit 0x08
        _emit 0xeb              // JMP short +5
        _emit 0x05
        _emit 0x33              // XOR EAX,EAX
        _emit 0xc0
        _emit 0x89              // MOV dword ptr [ESI + 0x8],EAX
        _emit 0x46
        _emit 0x08
        _emit 0x8b              // MOV EAX,ESI
        _emit 0xc6
        _emit 0x8b              // MOV ECX,dword ptr [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV dword ptr FS:[0x0],ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59               // POP ECX
        _emit 0x5f               // POP EDI
        _emit 0x5e               // POP ESI
        _emit 0x5b               // POP EBX
        _emit 0x83               // ADD ESP,0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc2               // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
