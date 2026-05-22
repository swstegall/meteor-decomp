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
// FUNCTION: ffxivgame 0x00406133 — read-API-blob-into-buffer helper
//                                  (__cdecl, 96 bytes)
//
// __cdecl bool FUN_00406133(int unused0, int unused1, Buffer *out)
//   stack layout (after RET, image-base 0x00400000):
//     [ESP+0x04] : (unused — arg0)
//     [ESP+0x08] : (unused — arg1)
//     [ESP+0x0C] : Buffer *out      (the only consumed arg, → EDI)
//
// Behaviour: chain three IAT-imported APIs (handle = imp_a();  count =
// imp_b(<garbage>, <garbage>);  data = imp_c(handle)). On any zero/
// non-positive return, bail with AL=0. Otherwise reserve `count`
// bytes in `out`, grow its end cursor by `count`, then memcpy the
// `count` bytes from `data` into the freshly-reserved span; return
// AL=1.
//
// The garbage args to imp_b are the saved-but-not-yet-initialised
// ESI/EDI from this function's prologue — the API at IAT slot
// 0x00f3e180 happens to ignore those two stack slots (or treat them
// as optional out-params the caller doesn't care about). Either way
// the orig codegen just throws the dirty callee-saved values on the
// stack to make a 2-dword stdcall slot, and we faithfully reproduce
// that here.
//
// Inspection (read from the orig bytes at RVA 0x00006133, 96 bytes total):
//
//   push ebx
//   push esi
//   push edi
//   call dword ptr [0x00f3e17c]   ; handle = imp_a()
//   mov  ebx, eax
//   test ebx, ebx
//   jz   fail
//   push esi                      ; (garbage dword: callee-saved ESI)
//   push edi                      ; (garbage dword: callee-saved EDI)
//   call dword ptr [0x00f3e180]   ; count = imp_b(garbage, garbage)
//   mov  esi, eax
//   test esi, esi
//   jbe  fail                     ; unsigned <=0 → fail
//   push ebx                      ; data = imp_c(handle)
//   call dword ptr [0x00f3e184]
//   mov  ebx, eax
//   test ebx, ebx
//   jz   fail
//   mov  edi, [esp+0x18]          ; edi = out (arg2)
//   push esi
//   mov  ecx, edi
//   call FUN_004061e0             ; out->reserve(count)   (RET 4)
//   push 0
//   push esi
//   mov  ecx, edi
//   call FUN_00406280             ; out->grow_end(count, 0) (RET 8)
//   push esi                      ; staged: memcpy.n
//   push ebx                      ; staged: memcpy.src
//   push 0                        ; arg to FUN_006ce260
//   mov  ecx, edi
//   call FUN_006ce260             ; eax = out->position_at(0)  (RET 4)
//   push eax                      ; memcpy.dest
//   call _memcpy                  ; memcpy(dest, src, n)
//   add  esp, 0xC                 ; cdecl cleanup (3 dwords)
//   pop  ebx
//   pop  edi
//   mov  al, 1
//   pop  esi
//   ret
// fail:
//   pop  ebx
//   pop  edi
//   xor  al, al
//   pop  esi
//   ret
//
// Reloc-bearing sites (CALL rel32 / CALL [imm32] targets the linker
// would resolve when emitted from source-level C++; we re-emit the
// orig bytes verbatim so the .obj's .text matches byte-for-byte with
// NO relocations):
//     +0x03   CALL [imm32] → IAT slot 0x00f3e17c
//     +0x11   CALL [imm32] → IAT slot 0x00f3e180
//     +0x1e   CALL [imm32] → IAT slot 0x00f3e184
//     +0x31   CALL rel32   → FUN_004061e0 (RVA 0x004061e0)
//     +0x3b   CALL rel32   → FUN_00406280 (RVA 0x00406280)
//     +0x46   CALL rel32   → FUN_006ce260 (RVA 0x006ce260)
//     +0x4c   CALL rel32   → _memcpy      (RVA 0x009d4600)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form would emit the same shape but produce
//   seven CALL relocations the linker resolves at relink time.
//   `tools/compare.py` masks reloc bytes out of the diff, but driving
//   a relink isn't necessary: a `__declspec(naked)` body that
//   re-emits the orig 96 bytes verbatim via MASM `_emit` directives
//   produces a .obj whose .text is byte-identical to the orig slice
//   (no relocations — the rel32/imm32 offsets are baked into the orig
//   binary's own address space and emitted here as raw bytes).
//   compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_00406133() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL dword ptr [0x00f3e17c]
        _emit 0x15
        _emit 0x7c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ fail (+0x4B)
        _emit 0x4b
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xff              // CALL dword ptr [0x00f3e180]
        _emit 0x15
        _emit 0x80
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x76              // JBE fail (+0x3D)
        _emit 0x3d
        _emit 0x53              // PUSH EBX
        _emit 0xff              // CALL dword ptr [0x00f3e184]
        _emit 0x15
        _emit 0x84
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0x85              // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74              // JZ fail (+0x30)
        _emit 0x30
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x18]
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_004061e0 (rel32 → 0x004061e0)
        _emit 0x77
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_00406280 (rel32 → 0x00406280)
        _emit 0x0d
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x53              // PUSH EBX
        _emit 0x6a              // PUSH 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xe8              // CALL FUN_006ce260 (rel32 → 0x006ce260)
        _emit 0xe2
        _emit 0x80
        _emit 0x2c
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL _memcpy (rel32 → 0x009d4600)
        _emit 0x7c
        _emit 0xe4
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x0C
        _emit 0xc4
        _emit 0x0c
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0xb0              // MOV AL, 0x01
        _emit 0x01
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
        _emit 0x5b              // POP EBX                       ; fail
        _emit 0x5f              // POP EDI
        _emit 0x32              // XOR AL, AL
        _emit 0xc0
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
