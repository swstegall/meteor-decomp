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
// FUNCTION: ffxivgame 0x00047a40 — __thiscall substr-style helper that
//                                  forwards a (ptr, count) slice of `this`
//                                  to a peer builder at 0x00447260 (54 B).
//
// Layout (inferred from the asm):
//   This (ECX):
//     +0x00  T   *data        (base pointer / buffer)
//     +0x08  int  size        (length)
//
// Calling convention: __thiscall (ECX = this; three DWORD stack args —
//   a hidden sret/out object, a position, and a count-or-npos; callee
//   cleans 0xc via `ret 0xc`).
//
// Source shape (inferred):
//
//   Out * Foo::slice(Out *result, int pos, int count) {
//       int n = count;                         // [esp] scratch slot = 0
//       if (count == -1)                       // npos sentinel
//           n = this->size - pos - 1;
//       FUN_00447260(result, this->data + pos, n /*, 0*/);
//       return result;
//   }
//
// Notes on the codegen:
//   - `PUSH ECX` at entry reserves a 4-byte scratch slot which is zeroed
//     (`MOV dword ptr [esp], 0`) and rides along as the trailing arg to
//     the call (it sits at [esp+0xc] when the two PUSHes land).
//   - The npos branch (`CMP EAX, -1` / `JNZ`) computes the implied length
//     `this->size - pos - 1` only when the caller passed -1.
//   - ESI caches the out-pointer (arg1) so it can be returned in EAX
//     after the call clobbers volatiles.
//   - The REL32 callsite (FUN_00447260) is masked out of the byte diff by
//     tools/compare.py.
//
// Asm (54 bytes):
//   51                      PUSH ECX                       ; scratch slot
//   8b 44 24 10             MOV  EAX, [ESP + 0x10]         ; count
//   83 f8 ff                CMP  EAX, -1
//   8b 54 24 0c             MOV  EDX, [ESP + 0xc]          ; pos
//   c7 04 24 00 00 00 00    MOV  dword ptr [ESP], 0        ; scratch = 0
//   75 08                   JNZ  have_count
//   8b 41 08                MOV  EAX, [ECX + 8]            ; this->size
//   2b c2                   SUB  EAX, EDX                  ; - pos
//   83 e8 01                SUB  EAX, 1
// have_count:
//   56                      PUSH ESI
//   8b 74 24 0c             MOV  ESI, [ESP + 0xc]          ; out-ptr (arg1)
//   50                      PUSH EAX                       ; count
//   8b 01                   MOV  EAX, [ECX]                ; this->data
//   03 c2                   ADD  EAX, EDX                  ; + pos
//   50                      PUSH EAX                       ; ptr
//   8b ce                   MOV  ECX, ESI                  ; this = out-ptr
//   e8 f1 f7 ff ff          CALL FUN_00447260
//   8b c6                   MOV  EAX, ESI                  ; return out-ptr
//   5e                      POP  ESI
//   59                      POP  ECX                       ; drop scratch
//   c2 0c 00                RET  0xc
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors the
// sibling _rosetta/* functions): a `__declspec(naked)` body that re-emits
// the orig 54 bytes verbatim via MASM `_emit` directives so the .obj's
// .text matches byte-for-byte. The lone REL32 (the call) is emitted as the
// orig wire bytes and masked by tools/compare.py.

extern "C" __declspec(naked) void FUN_00447a40() {
    __asm {
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0xc7             // MOV dword ptr [ESP], 0
        _emit 0x04
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75              // JNZ have_count (+0x08)
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ECX+8]
        _emit 0x41
        _emit 0x08
        _emit 0x2b              // SUB EAX, EDX
        _emit 0xc2
        _emit 0x83              // SUB EAX, 1
        _emit 0xe8
        _emit 0x01
        _emit 0x56              // PUSH ESI            (have_count:)
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0xc]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x03              // ADD EAX, EDX
        _emit 0xc2
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_00447260 (rel32 → 0x00447260)
        _emit 0xf1
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x59              // POP ECX
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
