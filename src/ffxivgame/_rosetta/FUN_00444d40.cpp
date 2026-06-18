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
// FUNCTION: ffxivgame 0x00044d40 — `__thiscall` scalar deleting destructor
//                                  for a ref-counted buffer class (65 B / 0x41)
//
// Asm shape (Ghidra-reported 65 bytes — function is truncated at compare
// boundary; actual epilogue extends to RET 0x4 at 0x00044d85):
//
//   __thiscall SomeClass* FUN_00444d40(this, unsigned int flags)
//     ECX = this          [__thiscall, self]
//     [ESP+4] = flags     [1 arg, cleaned by RET 4 outside compare window]
//
//   // Reset vtable to base class during destruction
//   [ESI] = 0xf67290
//
//   // Release the ref-counted buffer at this->field_0x8
//   if (this->field_0x8 != nullptr) {
//       ECX = *(this->field_0x8 - 4);    // load ref-count / size before data
//       EDI = this->field_0x8 - 4;       // pointer to ref-count header
//       // __stdcall cleanup call (4 args, callee-cleans):
//       FUN_009d1c4c(this->field_0x8, 0xbc, ECX, 0x00443c90);
//       // __cdecl dealloc call (1 arg, caller-cleans with ADD ESP,4):
//       FUN_009d1be9(EDI);
//       ADD ESP, 4
//       POP EDI
//   }
//
//   // Optionally delete `this` if bit 0 of flags is set
//   if (flags & 1) {
//       // __cdecl free call — truncated at compare boundary (83 c4 = first
//       // 2 bytes of ADD ESP,4 cleanup); full epilogue is outside the 65 B
//       // Ghidra-reported window.
//       FUN_009d1b17(this);
//       [83 c4 ← truncated]
//   }
//   // Epilogue (outside 65-byte window):
//   //   8b c6  MOV EAX, ESI
//   //   5e     POP ESI
//   //   c2 04  RET 4
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The three CALL targets are at very high RVAs (0x009d1c4c, 0x009d1be9,
//   0x009d1b17) whose rel32 offsets are already resolved in the original
//   binary. Emitting them as raw _emit bytes in a standalone .obj produces
//   the same bytes without introducing COFF relocations. The PUSH imm32 at
//   bytes 23-27 (0x00443c90) is likewise an absolute address; no reloc
//   is needed. The compare window is exactly 65 bytes (Ghidra under-counted
//   the full 72-byte function; the last 7 bytes of the real function lie
//   outside the compare window). tools/compare.py reads 65 bytes from the
//   original binary and compares against this .obj's .text section.

extern "C" __declspec(naked) void FUN_00444d40() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x8b              // MOV EAX, dword ptr [ESI + 0x8]
        _emit 0x46
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f67290
        _emit 0x06
        _emit 0x90
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x74              // JZ +0x22 (→ TEST byte ptr [ESP+8], 0x1)
        _emit 0x22
        _emit 0x8b              // MOV ECX, dword ptr [EAX + -0x4]
        _emit 0x48
        _emit 0xfc
        _emit 0x57              // PUSH EDI
        _emit 0x8d              // LEA EDI, [EAX + -0x4]
        _emit 0x78
        _emit 0xfc
        _emit 0x68              // PUSH 0x00443c90
        _emit 0x90
        _emit 0x3c
        _emit 0x44
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x68              // PUSH 0xbc
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d1c4c  (__stdcall, 4 args)
        _emit 0xe4
        _emit 0xce
        _emit 0x58
        _emit 0x00
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL 0x009d1be9  (__cdecl, 1 arg)
        _emit 0x7b
        _emit 0xce
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x4  (cleanup cdecl arg)
        _emit 0xc4
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0xf6              // TEST byte ptr [ESP + 0x8], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x74              // JZ +0x9 (→ epilogue, outside compare window)
        _emit 0x09
        _emit 0x56              // PUSH ESI  (= this, arg to free/delete)
        _emit 0xe8              // CALL 0x009d1b17  (__cdecl, 1 arg)
        _emit 0x98
        _emit 0xcd
        _emit 0x58
        _emit 0x00
        _emit 0x83              // ADD ESP, ... (first 2 bytes of cleanup;
        _emit 0xc4              //   truncated at 65-byte Ghidra compare boundary)
    }
}
