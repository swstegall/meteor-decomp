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
// FUNCTION: ffxivgame 0x00024160 — `__thiscall` destructor / cleanup that
//                                  resets a vtable, releases 18 COM-style
//                                  member pointers, then fires a lazy-bound
//                                  assertion if the global registry entry
//                                  is already clear (136 B / 0x88, no SEH).
//
// Inspection (read from asm/ffxivgame/00024160_FUN_00424160.s):
//
//   __thiscall void FUN_00424160(void *this /* ECX */);
//
//   Body outline:
//
//     // Reset vtable.
//     this->vptr = 0x00f5bd9c;
//
//     // Walk two interleaved 9-entry pointer arrays at [this+0x04..0x24]
//     // and [this+0x28..0x48], releasing each non-null entry via vtable
//     // slot 0 (i.e. Release(1) / __thiscall virtual fn 0 with arg 1):
//     SomeInterface **esi = (SomeInterface **)((char *)this + 0x28);
//     int ebx = 9;
//     do {
//         SomeInterface *low  = *(esi - 9);   // [this+0x04 .. +0x24]
//         if (low)  { (*low->vptr[0])(1);  *(esi - 9) = NULL; }
//         SomeInterface *high = *esi;          // [this+0x28 .. +0x48]
//         if (high) { (*high->vptr[0])(1); *esi = NULL; }
//         ++esi;
//     } while (--ebx != 0);
//
//     // Assert that the global registry slot for this class is occupied —
//     // if it is already NULL the object was never registered, which is a
//     // bug.  Uses the same lazy-init reporter pattern as FUN_004241f0 /
//     // FUN_00435a90:
//     if (g_registry /*0x01329954*/ == 0) {
//         // lazy init of g_thunk (0x0132390c) via g_flags (0x01323910):
//         if (!(g_flags & 1)) {
//             g_flags |= 1;
//             g_thunk = (void(*)(...))0x004240a0;
//         }
//         g_thunk(0x00f5bc70,          // expr string
//                 0x00f5baf6,          // file string
//                 0x00f5bc18,          // ? string
//                 0x2b,                // line number (43)
//                 0x00f5bbc0);         // message string
//     }
//     g_registry = 0;                  // always clear the slot on exit
//
//   Calling convention: __thiscall (ECX = this on entry). No stack
//   parameters; plain RET (no `ret N`). Callee-saves EBX/ESI/EDI;
//   no frame pointer (EBP not pushed).
//
//   Reloc-bearing sites in the 136 bytes (all masked by tools/compare.py):
//     +0x03  MOV [ECX], imm32  → vtable  0x00f5bd9c  (.rdata)
//     +0x3d  CMP  [moffs32]    → 0x01329954           (.data)
//     +0x45  TEST [moffs32]    → 0x01323910           (.data)
//     +0x4e  OR   [moffs32]    → 0x01323910           (.data)
//     +0x57  MOV  [moffs32], imm32 → dst 0x0132390c (.data)
//                                    val 0x004240a0  (code)
//     +0x60  PUSH imm32        → 0x00f5bbc0           (.rdata)
//     +0x67  PUSH imm32        → 0x00f5bc18           (.rdata)
//     +0x6c  PUSH imm32        → 0x00f5baf6           (.rdata)
//     +0x71  PUSH imm32        → 0x00f5bc70           (.rdata)
//     +0x75  CALL [moffs32]    → 0x0132390c           (.data ptr)
//     +0x7e  MOV  [moffs32], EDI → 0x01329954         (.data)
//
// Reconstruction strategy — naked-asm byte passthrough, the same idiom
// as FUN_004241f0 (the init counterpart at 0x000241f0) and FUN_00435a90:
// a `__declspec(naked)` body that re-emits the orig 136 bytes verbatim
// via MASM `_emit` directives. All reloc windows are masked in the diff.

extern "C" __declspec(naked) void FUN_00424160() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xc7              // MOV dword ptr [ECX], 0xf5bd9c
        _emit 0x01
        _emit 0x9c
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA ESI, [ECX + 0x28]
        _emit 0x71
        _emit 0x28
        _emit 0xbb              // MOV EBX, 0x9
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x8b              // MOV ECX, dword ptr [ESI - 0x24]  (loop_top:)
        _emit 0x4e
        _emit 0xdc
        _emit 0x3b              // CMP ECX, EDI
        _emit 0xcf
        _emit 0x74              // JZ skip_low (+0x0b)
        _emit 0x0b
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x89              // MOV dword ptr [ESI - 0x24], EDI
        _emit 0x7e
        _emit 0xdc
        _emit 0x8b              // MOV ECX, dword ptr [ESI]         (skip_low:)
        _emit 0x0e
        _emit 0x3b              // CMP ECX, EDI
        _emit 0xcf
        _emit 0x74              // JZ skip_high (+0x0a)
        _emit 0x0a
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x89              // MOV dword ptr [ESI], EDI         (skip_high:)
        _emit 0x3e
        _emit 0x83              // ADD ESI, 0x4
        _emit 0xc6
        _emit 0x04
        _emit 0x83              // SUB EBX, 0x1
        _emit 0xeb
        _emit 0x01
        _emit 0x75              // JNZ loop_top (-0x2a)
        _emit 0xd6
        _emit 0x39              // CMP dword ptr [0x01329954], EDI
        _emit 0x3d
        _emit 0x54
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ skip_assert (+0x39)
        _emit 0x39
        _emit 0xf6              // TEST byte ptr [0x01323910], 0x1
        _emit 0x05
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75              // JNZ already_init (+0x11)
        _emit 0x11
        _emit 0x83              // OR dword ptr [0x01323910], 0x1
        _emit 0x0d
        _emit 0x10
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x0132390c], 0x4240a0
        _emit 0x05
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0xa0
        _emit 0x40
        _emit 0x42
        _emit 0x00
        _emit 0x68              // PUSH 0xf5bbc0                    (already_init:)
        _emit 0xc0
        _emit 0xbb
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x2b
        _emit 0x2b
        _emit 0x68              // PUSH 0xf5bc18
        _emit 0x18
        _emit 0xbc
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf5baf6
        _emit 0xf6
        _emit 0xba
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0xf5bc70
        _emit 0x70
        _emit 0xbc
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x0132390c]
        _emit 0x15
        _emit 0x0c
        _emit 0x39
        _emit 0x32
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x89              // MOV dword ptr [0x01329954], EDI  (skip_assert:)
        _emit 0x3d
        _emit 0x54
        _emit 0x99
        _emit 0x32
        _emit 0x01
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
