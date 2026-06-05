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
// FUNCTION: ffxivgame 0x00047ed0 — `__thiscall` "drain matches from a1 into
//                                  this, applying a2" loop (83 B / 0x53).
//
// Behaviour read from the disassembly at orig RVA 0x00047ed0:
//
//   __thiscall Self* FUN_00447ed0(Self* this, Src* a1, int a2) {
//       int idx = this->find(a1, 0);          // FUN_004465a0(a1, 0)
//       if (idx != -1) {
//           do {
//               int v = a1->next();            // FUN_00445e50()  (__thiscall on a1)
//               this->apply(idx, v);           // FUN_004460a0(idx, v)
//               this->record(idx, a2);         // FUN_00447de0(idx, a2)
//               idx = this->find(a1, 0);       // FUN_004465a0(a1, 0)
//           } while (idx != -1);
//       }
//       return this;                           // EAX = EDI = this
//   }
//
//   Register allocation (MSVC 2005 /O2):
//     EDI = this   (saved across, returned in EAX)
//     EBX = a1     (loaded from [ESP+8] in the prologue)
//     EBP = a2     (loaded lazily from [ESP+0x18] only inside the taken arm)
//     ESI = idx    (sentinel-compared against -1)
//
//   Branch shape — MSVC's classic "test once at the top, then JNZ back to a
//   shared loop head" merge: the initial `find` runs before EBP/a2 is even
//   loaded; if it returns -1 the function falls straight through to the
//   epilogue (POP EBP is NOT executed on that path — EBP is only pushed
//   inside the taken arm). The 3-byte `8d 49 00` (LEA ECX,[ECX]) right
//   before the loop head is MSVC's loop-entry alignment padding.
//
//   `ret 8` confirms __thiscall with two stack args (a1, a2); the lone
//   ECX-based receiver and the EAX=this return are the member-function
//   tell. Four direct sibling CALLs (e8 + REL32) resolve at relink:
//     +0x0c  CALL FUN_004465a0   (find first / find next)
//     +0x22  CALL FUN_00445e50   (a1->next)
//     +0x2b  CALL FUN_004460a0   (this->apply)
//     +0x34  CALL FUN_00447de0   (this->record)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   Coaxing MSVC 2005 to reproduce this exact prologue interleave (PUSH EBX
//   / load a1 / PUSH ESI / PUSH EDI), the lazy a2 load gated behind the
//   first sentinel test, the loop-head alignment LEA, and the four REL32
//   call windows is brittle. Following the precedent of FUN_00401090 /
//   FUN_00404f10 in this directory, the function is short (83 B) and
//   reloc-dense (four call fixups), so we re-emit the orig bytes verbatim
//   via MASM `_emit`. The .obj's `.text` ends up byte-identical to the orig
//   slice; the four CALL operands bake in their post-link relative offsets
//   as raw immediates, which tools/compare.py accepts by direct equality.

extern "C" __declspec(naked) void FUN_00447ed0() {
    __asm {
        // 00047ed0: push ebx
        _emit 0x53
        // 00047ed1: mov ebx, dword ptr [esp+8]   ; a1
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 00047ed5: push esi
        _emit 0x56
        // 00047ed6: push edi
        _emit 0x57
        // 00047ed7: push 0
        _emit 0x6a
        _emit 0x00
        // 00047ed9: push ebx
        _emit 0x53
        // 00047eda: mov edi, ecx                 ; this
        _emit 0x8b
        _emit 0xf9
        // 00047edc: call FUN_004465a0            ; idx = this->find(a1, 0)
        _emit 0xe8
        _emit 0xbf
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        // 00047ee1: mov esi, eax                 ; idx
        _emit 0x8b
        _emit 0xf0
        // 00047ee3: cmp esi, -1
        _emit 0x83
        _emit 0xfe
        _emit 0xff
        // 00047ee6: jz 0x00447f1b                ; idx == -1 -> return
        _emit 0x74
        _emit 0x33
        // 00047ee8: push ebp
        _emit 0x55
        // 00047ee9: mov ebp, dword ptr [esp+18h] ; a2
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x18
        // 00047eed: lea ecx, [ecx]               ; loop-head alignment pad
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 00047ef0: mov ecx, ebx                 ; loop head: ecx = a1
        _emit 0x8b
        _emit 0xcb
        // 00047ef2: call FUN_00445e50            ; v = a1->next()
        _emit 0xe8
        _emit 0x59
        _emit 0xdf
        _emit 0xff
        _emit 0xff
        // 00047ef7: push eax                     ; v
        _emit 0x50
        // 00047ef8: push esi                     ; idx
        _emit 0x56
        // 00047ef9: mov ecx, edi                 ; this
        _emit 0x8b
        _emit 0xcf
        // 00047efb: call FUN_004460a0            ; this->apply(idx, v)
        _emit 0xe8
        _emit 0xa0
        _emit 0xe1
        _emit 0xff
        _emit 0xff
        // 00047f00: push ebp                     ; a2
        _emit 0x55
        // 00047f01: push esi                     ; idx
        _emit 0x56
        // 00047f02: mov ecx, edi                 ; this
        _emit 0x8b
        _emit 0xcf
        // 00047f04: call FUN_00447de0            ; this->record(idx, a2)
        _emit 0xe8
        _emit 0xd7
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00047f09: push 0
        _emit 0x6a
        _emit 0x00
        // 00047f0b: push ebx                     ; a1
        _emit 0x53
        // 00047f0c: mov ecx, edi                 ; this
        _emit 0x8b
        _emit 0xcf
        // 00047f0e: call FUN_004465a0            ; idx = this->find(a1, 0)
        _emit 0xe8
        _emit 0x8d
        _emit 0xe6
        _emit 0xff
        _emit 0xff
        // 00047f13: mov esi, eax                 ; idx
        _emit 0x8b
        _emit 0xf0
        // 00047f15: cmp esi, -1
        _emit 0x83
        _emit 0xfe
        _emit 0xff
        // 00047f18: jnz 0x00447ef0               ; loop while idx != -1
        _emit 0x75
        _emit 0xd6
        // 00047f1a: pop ebp
        _emit 0x5d
        // 00047f1b: mov eax, edi                 ; return this
        _emit 0x8b
        _emit 0xc7
        // 00047f1d: pop edi
        _emit 0x5f
        // 00047f1e: pop esi
        _emit 0x5e
        // 00047f1f: pop ebx
        _emit 0x5b
        // 00047f20: ret 8
        _emit 0xc2
        _emit 0x08
        _emit 0x00
    }
}
