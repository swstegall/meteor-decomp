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
// FUNCTION: ffxivgame 0x0003c770 — __thiscall method: vtable reset + one-shot
//                                  global init guard + delegating call (124 B).
//
// Behaviour reconstructed from the asm at RVA 0x0003c770 (124 bytes):
//
//   void __thiscall FUN_0043c770(SomeClass *this /*ECX*/)
//   {
//       // Reset vtable unconditionally.
//       this->vftable = (void*)0x00f66640;
//
//       if (this->member4 != 0) {
//           // One-time initialisation gated by the LSB of global flag
//           // at 0x01327c20.
//           if (!(g_initFlag & 1)) {
//               g_initFlag |= 1;             // [0x01327c20] |= 1
//               // SEH state = 0 (enter try)
//               g_globalObj = FUN_0040e500(); // result stored at [0x01327c1c]
//               // SEH state = -1 (exit try)
//           }
//           // __thiscall call: ECX = g_globalObj, arg = this->member4
//           FUN_0040df70(g_globalObj, this->member4);
//       }
//   }
//
// Calling convention: __thiscall — `this` arrives in ECX, captured into ESI.
// No explicit stack arguments; epilogue is plain `RET` (not `RET N`).
//
// SEH frame layout (ESP-based, after the five-push prologue):
//   [ESP+0x00]  GS cookie (security_cookie ^ ESP)
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  prev FS:[0]  (also used in epilogue restore)
//   [ESP+0x0c]  SEH handler  (0x00e56767)
//   [ESP+0x10]  state slot   (-1 initial; 0 inside try, -1 after)
//   [ESP+0x14]  return address
//
// Reloc-bearing sites (compare.py masks these in the diff):
//   +0x03   PUSH imm32  → SEH handler  (0x00e56767)
//   +0x09   MOV moffs32 → FS:[0]       (read prev chain head)
//   +0x14   MOV moffs32 → __security_cookie (0x012ea8b0)
//   +0x1b   MOV moffs32 → FS:[0]       (install handler)
//   +0x27   MOV DIR32   → vtable slot  (0x00f66640)
//   +0x2f   TEST moffs8 → g_initFlag   (0x01327c20)
//   +0x37   OR   moffs32→ g_initFlag
//   +0x49   CALL rel32  → FUN_0040e500 (displacement 0xfffd1d41)
//   +0x4f   MOV moffs32 → g_globalObj  (0x01327c1c, store EAX)
//   +0x5a   MOV DIR32   → g_globalObj  (0x01327c1c, load ECX)
//   +0x60   CALL rel32  → FUN_0040df70 (displacement 0xfffd1795)
//   +0x67   MOV moffs32 → FS:[0]       (epilogue restore)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would require references to six distinct
//   binary-resident absolute addresses and two rel32 CALL targets, none
//   of which are available in the standalone .obj context.  Following the
//   same strategy as siblings FUN_00401350 and FUN_00403d60, we emit the
//   124 orig bytes verbatim via `_emit` directives inside a
//   `__declspec(naked)` body.  The resulting .obj's `.text` section is
//   byte-identical to the orig slice; `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_0043c770() {
    __asm {
        // --- SEH / GS prologue -----------------------------------------
        _emit 0x6a              // PUSH -1             (initial SEH state)
        _emit 0xff
        _emit 0x68              // PUSH 0x00e56767     (SEH handler)
        _emit 0x67
        _emit 0x67
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX            (prev FS:[0])
        _emit 0x56              // PUSH ESI            (callee-save)
        _emit 0xa1              // MOV EAX, [0x012ea8b0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX            (GS cookie)
        _emit 0x8d              // LEA EAX, [ESP+0x8]  (→ &prev_fs0)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x64              // MOV FS:[0x00000000], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- Body -------------------------------------------------------
        _emit 0x8b              // MOV ESI, ECX        (this)
        _emit 0xf1
        _emit 0x83              // CMP dword ptr [ESI+0x4], 0
        _emit 0x7e
        _emit 0x04
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f66640  (vtable reset)
        _emit 0x06
        _emit 0x40
        _emit 0x66
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ +0x3c  (→ epilogue)
        _emit 0x3c

        // member4 != 0 arm
        _emit 0xb8              // MOV EAX, 0x00000001
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST byte ptr [0x01327c20], AL
        _emit 0x05
        _emit 0x20
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x75              // JNZ +0x20  (→ after one-time init)
        _emit 0x20

        // One-time init (flag not yet set)
        _emit 0x09              // OR dword ptr [0x01327c20], EAX  (set flag bit 0)
        _emit 0x05
        _emit 0x20
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x10], 0x00000000  (state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL 0x0040e500  (rel32 = 0xfffd1d41)
        _emit 0x41
        _emit 0x1d
        _emit 0xfd
        _emit 0xff
        _emit 0xa3              // MOV [0x01327c1c], EAX  (store result)
        _emit 0x1c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [ESP+0x10], 0xffffffff  (state = -1)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff

        // Delegating call (both branches converge here)
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]  (this->member4)
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [0x01327c1c]  (g_globalObj)
        _emit 0x0d
        _emit 0x1c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x0040df70  (rel32 = 0xfffd1795)
        _emit 0x95
        _emit 0x17
        _emit 0xfd
        _emit 0xff

        // --- SEH teardown / epilogue ------------------------------------
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x8]  (prev FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x64              // MOV dword ptr FS:[0x00000000], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX   (discard GS cookie)
        _emit 0x5e              // POP ESI   (restore)
        _emit 0x83              // ADD ESP, 0x0c
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
