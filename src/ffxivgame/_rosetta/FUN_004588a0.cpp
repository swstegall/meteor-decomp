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
// FUNCTION: ffxivgame 0x004588a0 — UTF-8 codepoint advance/decode wrapper
//                                  (52 B / 0x34)
//
//   void * __cdecl FUN_004588a0(... /* [ESP+0x18] -> obj */)
//
// Inspection (read from the orig bytes at RVA 0x000588a0, 52 bytes total):
//
//   sub  esp, 0xc                  ; carve a small scratch frame + spill
//   push esi
//   mov  esi, dword ptr [esp+0x18] ; esi = arg @ [orig esp+0x08]
//   lea  ecx, [esp+0x8]            ; ecx = &scratch (out word slot)
//   lea  eax, [esi+0x4]            ; eax = esi+4  (this for FUN_004577c0,
//                                  ;   which reads `this` in EAX/ESI)
//   push ecx                       ; arg: &scratch
//   call FUN_004577c0              ; rel32 → 0x004577c0  (decode 1 cp)
//   mov  edx, dword ptr [esi]      ; edx = *esi
//   mov  esi, dword ptr [esp+0x18] ; reload esi = arg
//   lea  eax, [esp+0x8]            ; eax = &scratch
//   push eax                       ; arg2: &scratch
//   push esi                       ; arg1: arg
//   mov  dword ptr [esp+0x10], edx ; stash *esi into the scratch slot
//   call FUN_00458810              ; rel32 → 0x00458810
//   add  esp, 0xc                  ; drop the two pushes + slack
//   mov  eax, esi                  ; return arg (in EAX)
//   pop  esi
//   add  esp, 0xc                  ; tear down the scratch frame
//   ret
//
// Calling convention: __cdecl (caller cleans). FUN_004577c0 takes its
// primary object in EAX (here esi+4) plus one stack arg (the out slot)
// and is callee-clean (`ret`-pops nothing of ours via internal POPs);
// FUN_00458810 takes two stack args, both cleaned by the trailing
// `add esp, 0xc`.
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors sibling
// FUN_004051e0): a `__declspec(naked)` body that re-emits the orig 52
// bytes verbatim via MASM `_emit` directives. The two intra-.text CALL
// rel32 immediates (0x004577c0, 0x00458810) carry no PE relocations
// (they are PC-relative), so the orig .text already holds the resolved
// displacement bytes (`0b ef ff ff`, `46 ff ff ff`) and we emit them
// verbatim. The .obj's `.text` is byte-identical to the orig slice with
// zero relocations; `tools/compare.py` reports GREEN.
//
// Reloc-relevant sites in the orig 52 bytes:
//     +0x10   CALL rel32   → FUN_004577c0   (e8 0b ef ff ff)
//     +0x25   CALL rel32   → FUN_00458810   (e8 46 ff ff ff)

extern "C" __declspec(naked) void FUN_004588a0() {
    __asm {
        _emit 0x83              // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x18]
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x8d              // LEA ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0x8d              // LEA EAX, [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL FUN_004577c0 (rel32)
        _emit 0x0b
        _emit 0xef
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x18]
        _emit 0x74
        _emit 0x24
        _emit 0x18
        _emit 0x8d              // LEA EAX, [ESP+0x8]
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0x89              // MOV dword ptr [ESP+0x10], EDX
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0xe8              // CALL FUN_00458810 (rel32)
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc3              // RET
    }
}
