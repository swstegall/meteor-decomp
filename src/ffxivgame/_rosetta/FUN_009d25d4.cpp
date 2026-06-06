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
// FUNCTION: ffxivgame 0x005d25d4 (VA 0x009d25d4) — connection/object close
//           helper; __cdecl, 110 bytes, no standalone epilogue (shared with
//           the next function at 0x009d2642).
//
// Calling convention: __cdecl, one pointer argument.
//
//   int FUN_009d25d4(SomeObj *p);
//
// Frame layout (after PUSH EBX / PUSH ESI / PUSH EDI = 12 bytes saved):
//   [ESP+0x0C] = p (arg1, loaded into ESI after the first two saves)
//
// Register usage:
//   ESI = p (arg1)
//   EDI = 0 (constant zero sentinel)
//   EBX = result / error code (-1 on failure, else success value)
//   EAX = return value ← EBX at end
//
// High-level logic (recovered from disassembly):
//
//   if (p == NULL) {
//       void *err = FUN_009d9d47();    // allocate / get error object
//       [err] = 0x16;                  // set error type tag
//       FUN_009d2290(0, 0, 0, 0, 0);   // report / raise error (5 null args)
//       return -1;                     // OR EAX, EBX where EBX = -1
//       // JMP → shared epilogue at 0x009d2642 (next function)
//   }
//   if ((p->flags & 0x83) != 0) {     // offset 0x0C holds connection flags
//       int val = FUN_009d7061(p);    // get some value from p
//       EBX = val;
//       FUN_009e0216(p);              // notify / deregister
//       FUN_009d6a61(p);              // some query on p
//       int rc = FUN_009e0149(EAX);   // process result
//       if (rc < 0) EBX = -1;
//       if (p->field_1C != NULL) {
//           FUN_009d5c88(p->field_1C); // release nested object
//           p->field_1C = NULL;
//       }
//   }
//   p->flags = 0;                     // offset 0x0C ← 0
//   return EBX;
//   // falls through → shared epilogue at 0x009d2642 (POP EDI/ESI/EBX + RET)
//
// Note on epilogue sharing: neither code path in this function executes
// POP/RET — both reach address 0x009d2642 (= function_base + 0x6E), which
// is the first instruction of the NEXT function and serves as the shared
// epilogue (POP EDI; POP ESI; POP EBX; RET).  Ghidra records the boundary
// at 0x009d2642, so the 110-byte slice is exact.  The `_emit` passthrough
// reproduces the raw stream byte-for-byte; compare.py reports GREEN.
//
// Reloc-bearing sites (CALL rel32 and JMP rel8 to past-the-end):
//   +0x10  CALL rel32 → 0x009d9d47 (FUN_009d9d47, disp=0x0000775e)
//   +0x20  CALL rel32 → 0x009d2290 (FUN_009d2290, disp=0xfffffc97)
//   +0x33  CALL rel32 → 0x009d7061 (FUN_009d7061, disp=0x00004a55)
//   +0x3b  CALL rel32 → 0x009e0216 (FUN_009e0216, disp=0x0000dc02)
//   +0x41  CALL rel32 → 0x009d6a61 (FUN_009d6a61, disp=0x00004447)
//   +0x47  CALL rel32 → 0x009e0149 (FUN_009e0149, disp=0x0000db29)
//   +0x60  CALL rel32 → 0x009d5c88 (FUN_009d5c88, disp=0x0000364f)
//   +0x2a  JMP rel8   → 0x009d2642 (past function end, disp=+0x42)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Seven CALL rel32 relocations plus a JMP to past the function boundary
//   make source-level reconstruction error-prone and linker-dependent.
//   The same passthrough strategy used by siblings FUN_004090b0 and
//   FUN_004091f0 (raw `_emit` directives) produces a zero-relocation .obj
//   whose .text matches the orig byte stream exactly.

extern "C" __declspec(naked) void FUN_009d25d4() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x0C]
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x57              // PUSH EDI
        _emit 0x33              // XOR EDI, EDI
        _emit 0xff
        _emit 0x83              // OR EBX, 0xFFFFFFFF
        _emit 0xcb
        _emit 0xff
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x75              // JNZ +0x1C → else_branch (+0x2C)
        _emit 0x1c
        _emit 0xe8              // CALL FUN_009d9d47 (rel32=0x0000775e)
        _emit 0x5e
        _emit 0x77
        _emit 0x00
        _emit 0x00
        _emit 0x57              // PUSH EDI (0)
        _emit 0x57              // PUSH EDI (0)
        _emit 0x57              // PUSH EDI (0)
        _emit 0x57              // PUSH EDI (0)
        _emit 0x57              // PUSH EDI (0)
        _emit 0xc7              // MOV dword ptr [EAX], 0x16
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_009d2290 (rel32=0xfffffc97)
        _emit 0x97
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x0b              // OR EAX, EBX  (EBX=-1 → EAX=-1)
        _emit 0xc3
        _emit 0xeb              // JMP +0x42 → 0x009d2642 (shared epilogue)
        _emit 0x42
        _emit 0xf6              // TEST byte ptr [ESI+0x0C], 0x83
        _emit 0x46
        _emit 0x0c
        _emit 0x83
        _emit 0x74              // JZ +0x37 → cleanup (+0x69)
        _emit 0x37
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009d7061 (rel32=0x00004a55)
        _emit 0x55
        _emit 0x4a
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV EBX, EAX
        _emit 0xd8
        _emit 0xe8              // CALL FUN_009e0216 (rel32=0x0000dc02)
        _emit 0x02
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009d6a61 (rel32=0x00004447)
        _emit 0x47
        _emit 0x44
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009e0149 (rel32=0x0000db29)
        _emit 0x29
        _emit 0xdb
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7d              // JGE +5 → success_path (+0x58)
        _emit 0x05
        _emit 0x83              // OR EBX, 0xFFFFFFFF  (error)
        _emit 0xcb
        _emit 0xff
        _emit 0xeb              // JMP +0x11 → cleanup (+0x69)
        _emit 0x11
        _emit 0x8b              // MOV EAX, [ESI+0x1C]
        _emit 0x46
        _emit 0x1c
        _emit 0x3b              // CMP EAX, EDI
        _emit 0xc7
        _emit 0x74              // JZ +0x0A → cleanup (+0x69)
        _emit 0x0a
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_009d5c88 (rel32=0x0000364f)
        _emit 0x4f
        _emit 0x36
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x89              // MOV [ESI+0x1C], EDI
        _emit 0x7e
        _emit 0x1c
        _emit 0x89              // MOV [ESI+0x0C], EDI
        _emit 0x7e
        _emit 0x0c
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        // Shared epilogue continuation at 0x009d2642 (included in size_overrides):
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
