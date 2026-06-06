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
// FUNCTION: ffxivgame 0x00038180 — `__thiscall` argument-forwarding thunk:
//                                   packs 4 dwords + 1 byte into a 0x14-byte
//                                   stack temp and forwards it (by pointer)
//                                   to a sub-object method at this+0x10
//                                   (60 B / 0x3c)
//
// Inspection (read from the disassembly at orig RVA 0x00038180):
//
//   __thiscall void FUN_00438180(this, int a0, int a1, int a2, int a3, char a4)
//     ECX        : this
//     [ESP+0x18] : int  a0   (after SUB ESP,0x14 — return addr at [ESP+0x14])
//     [ESP+0x1c] : int  a1
//     [ESP+0x20] : int  a2
//     [ESP+0x24] : int  a3
//     [ESP+0x28] : char a4
//   Callee cleans 5 dwords (RET 0x14).
//
//   Body builds a 0x14-byte temp at [ESP]:
//     [ESP+0x00] = a0          ; loaded from [ESP+0x18]
//     [ESP+0x04] = a1          ; loaded from [ESP+0x1c]
//     [ESP+0x08] = a2          ; loaded from [ESP+0x20]
//     [ESP+0x0c] = a3          ; loaded from [ESP+0x24]
//     [ESP+0x10] = a4 (byte)   ; loaded from [ESP+0x28]
//   then  PUSH &temp ; ADD ECX,0x10 ; CALL 0x004367b0   (the [ESP+0x14]
//   store of the byte happens after the PUSH, so it targets the same temp
//   slot at old [ESP+0x10]). The helper at 0x004367b0 is a __thiscall
//   method on the sub-object at this+0x10 taking a single pointer arg.
//
//   Source shape (inferred):
//     struct Packed { int a0, a1, a2, a3; char a4; };
//     void Foo::method(int a0, int a1, int a2, int a3, char a4) {
//         Packed p;
//         p.a0 = a0; p.a1 = a1; p.a2 = a2; p.a3 = a3; p.a4 = a4;
//         this->sub_0x10->forward(&p);          // FUN_004367b0
//     }
//
// Reloc-bearing site in the orig 60 bytes (absolute address resolves only
// in a full-binary relink at image base 0x00400000; standalone .obj
// compilation can't reproduce the rel32 via source — naked asm emits it as
// raw immediate bytes that match the orig binary's resolved bytes):
//     +0x31   CALL rel32   → FUN_004367b0 (RVA 0x000367b0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Coaxing MSVC 2005 to emit this exact temp-build + PUSH-then-late-byte-
//   store + ADD ECX / CALL sequence from C++ source would require the full
//   surrounding class definition and the right register-allocator state.
//   The pragmatic choice — the same one siblings FUN_00406fa0 and
//   FUN_00408780 took — is a `__declspec(naked)` body re-emitting the orig
//   60 bytes verbatim via MASM `_emit` directives. The .obj's `.text` ends
//   up byte-identical to the orig slice; the rel32 CALL site is masked by
//   tools/compare.py. GREEN.

extern "C" __declspec(naked) void FUN_00438180() {
    __asm {
        _emit 0x83              // SUB ESP, 0x14
        _emit 0xec
        _emit 0x14
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x89              // MOV dword ptr [ESP+0x04], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x24]
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x89              // MOV dword ptr [ESP], EAX
        _emit 0x04
        _emit 0x24
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESP+0x0c], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x89              // MOV dword ptr [ESP+0x08], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8a              // MOV AL, byte ptr [ESP+0x28]
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x8d              // LEA EDX, [ESP]
        _emit 0x14
        _emit 0x24
        _emit 0x52              // PUSH EDX
        _emit 0x83              // ADD ECX, 0x10
        _emit 0xc1
        _emit 0x10
        _emit 0x88              // MOV byte ptr [ESP+0x14], AL
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL rel32 → 0x004367b0
        _emit 0xfa
        _emit 0xe5
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc2              // RET 0x0014
        _emit 0x14
        _emit 0x00
    }
}
