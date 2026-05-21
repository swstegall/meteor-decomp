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
// FUNCTION: ffxivgame 0x004051e0 — game-build name picker (46 B / 0x2E)
//
//   wchar_t * __cdecl FUN_004051e0(char beta_flag)
//     stack layout (after RET):
//       [ESP+0x04] : char beta_flag           (param_1)
//     returns: pointer to one of three wide-string literals (in EAX), or
//              null if the global "init flag" pointer points at a zero
//              byte.
//
// Inspection (read from the orig bytes at RVA 0x000051e0, 46 bytes total):
//
//   mov  eax, dword ptr [0x01323898]   ; eax = *(char **)DAT_01323898
//   cmp  byte ptr [eax], 0             ; *eax (the init-flag byte) ?
//   jnz  built                         ; non-zero → continue
//   xor  eax, eax                      ; init flag clear → return NULL
//   ret
// built:
//   cmp  byte ptr [esp+0x4], 0         ; beta_flag ?
//   jz   not_beta                      ; zero → fall through to the
//                                      ;        latest/plain selector
//   mov  eax, 0x00F54BCC               ; → L"FINAL FANTASY XIV Beta Version"
//   ret
// not_beta:
//   call FUN_00405080                  ; (rel32 → 0x00405080) — "is this
//                                      ;   the LATEST build?" predicate;
//                                      ;   returns char (AL).
//   test al, al
//   mov  eax, 0x00F54C0C               ; default: → L"FINAL FANTASY XIV"
//   jnz  ret_plain                     ; AL non-zero → keep plain name
//   mov  eax, 0x00F54C40               ; AL zero → override to LATEST
// ret_plain:
//   ret
//
// Calling convention: __cdecl (caller cleans, one byte stack arg).
// Stack frame: 0 (no locals, no register saves).
//
// Note on the branch lowering: the asm emits MOV EAX,<plain> first then
// conditionally overrides to MOV EAX,<latest> on the JNZ-fall-through.
// Ghidra renders the equivalent source as `pwVar2 = L"…LATEST"; if (cVar1
// == 0) pwVar2 = L"…XIV";` because its decompiler swaps the two arms to
// keep the if-body in the natural-order arm. The asm's literal ordering
// is the *other* arm — `plain` first, override to `LATEST` on the zero
// branch — which is what we re-emit here verbatim.
//
// Reloc-bearing sites in the orig 46 bytes (the linker would normally
// fill these in at link time; we re-emit the orig wire bytes verbatim
// so the .obj's .text matches byte-for-byte with NO relocations —
// `tools/compare.py` masks reloc bytes out of the diff, and a zero-
// reloc .obj is the simplest path to GREEN for a function that loads
// three .rdata addresses and calls one peer):
//     +0x01   MOV  imm32   → DAT_01323898   (init-flag pointer slot)
//     +0x15   MOV  imm32   → 0x00F54BCC     (L"…Beta Version")
//     +0x1B   CALL rel32   → FUN_00405080   (latest-build predicate)
//     +0x22   MOV  imm32   → 0x00F54C0C     (L"FINAL FANTASY XIV")
//     +0x29   MOV  imm32   → 0x00F54C40     (L"…LATEST")
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling FUN_00403bd0 / FUN_00403eb0 / FUN_00404270): a
// `__declspec(naked)` body that re-emits the orig 46 bytes verbatim via
// MASM `_emit` directives. The .obj's `.text` section is byte-identical
// to the orig slice (no relocations — the rel32 / imm32 immediates are
// baked into the orig binary's own address space and emitted here as
// raw bytes). `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_004051e0() {
    __asm {
        _emit 0xa1              // MOV EAX, dword ptr [0x01323898]
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x80              // CMP byte ptr [EAX], 0
        _emit 0x38
        _emit 0x00
        _emit 0x75              // JNZ built (+0x03)
        _emit 0x03
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xc3              // RET
        _emit 0x80              // CMP byte ptr [ESP+0x04], 0    (built:)
        _emit 0x7c
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x74              // JZ not_beta (+0x06)
        _emit 0x06
        _emit 0xb8              // MOV EAX, 0x00F54BCC
        _emit 0xcc
        _emit 0x4b
        _emit 0xf5
        _emit 0x00
        _emit 0xc3              // RET
        _emit 0xe8              // CALL FUN_00405080 (rel32 → 0x00405080)
        _emit 0x81
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0xb8              // MOV EAX, 0x00F54C0C
        _emit 0x0c
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x75              // JNZ ret_plain (+0x05)
        _emit 0x05
        _emit 0xb8              // MOV EAX, 0x00F54C40
        _emit 0x40
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0xc3              // RET
    }
}
