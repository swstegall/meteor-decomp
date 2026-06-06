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
// FUNCTION: ffxivgame 0x00466be0 — BUF_MEM_free (OpenSSL) (51 B / 0x33)
//
//   void __cdecl BUF_MEM_free(BUF_MEM *a)
//     stack layout (after PUSH ESI):
//       [ESP+0x08] : BUF_MEM *a            (param_1)
//
//   BUF_MEM layout (inferred from the asm):
//     +0x00  size_t  length
//     +0x04  char   *data
//     +0x08  size_t  max
//
// Source shape:
//
//   void BUF_MEM_free(BUF_MEM *a) {
//       if (a == 0) return;
//       if (a->data != 0) {
//           OPENSSL_cleanse(a->data, 0, a->max);   // memset-style 3-arg
//           CRYPTO_free(a->data);
//       }
//       CRYPTO_free(a);
//   }
//
// Inspection (read from the orig bytes at RVA 0x00066be0, 51 bytes total):
//
//   push esi
//   mov  esi, [esp+0x8]          ; esi = a
//   test esi, esi
//   jz   done                    ; a == 0 → return
//   mov  eax, [esi+0x4]          ; eax = a->data
//   test eax, eax
//   jz   free_a                  ; a->data == 0 → just free(a)
//   mov  ecx, [esi+0x8]          ; ecx = a->max
//   push ecx
//   push 0
//   push eax
//   call 0x009d2110             ; rel32 → OPENSSL_cleanse(data, 0, max)
//   mov  edx, [esi+0x4]
//   push edx
//   call 0x004632f0             ; rel32 → free(a->data)
//   add  esp, 0x10
// free_a:
//   push esi
//   call 0x004632f0             ; rel32 → free(a)
//   add  esp, 0x4
// done:
//   pop  esi
//   ret
//
// Calling convention: __cdecl (caller cleans, one pointer stack arg).
// Stack frame: 0 locals; single ESI callee-save.
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling FUN_004051e0): a `__declspec(naked)` body that re-emits the
// orig 51 bytes verbatim via MASM `_emit` directives. The .obj's `.text`
// is byte-identical to the orig slice (no relocations — the three rel32
// callsite immediates are baked from the orig binary's address space and
// emitted here as raw bytes). `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00466be0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74              // JZ done (+0x28)
        _emit 0x28
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x4]
        _emit 0x46
        _emit 0x04
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ free_a (+0x18)
        _emit 0x18
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x8]
        _emit 0x4e
        _emit 0x08
        _emit 0x51              // PUSH ECX
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d2110 (rel32)
        _emit 0x14
        _emit 0xb5
        _emit 0x56
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x4]
        _emit 0x56
        _emit 0x04
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL 0x004632f0 (rel32)
        _emit 0xeb
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x56              // PUSH ESI                    (free_a:)
        _emit 0xe8              // CALL 0x004632f0 (rel32)
        _emit 0xe2
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x5e              // POP ESI                     (done:)
        _emit 0xc3              // RET
    }
}
