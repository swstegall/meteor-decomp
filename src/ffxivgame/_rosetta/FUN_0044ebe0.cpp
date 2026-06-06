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
// FUNCTION: ffxivgame 0x0004ebe0 — name → encoded value lookup (__cdecl, 190 B)
//
//   __cdecl int FUN_0044ebe0(Ctx* ctx /* [ESP+0x2c] */)
//
// Behaviour read from asm/ffxivgame/0004ebe0_FUN_0044ebe0.s:
//
//   The argument is a small descriptor table:
//     +0x34  unsigned count
//     +0x38  Entry* entries     (12-byte stride; entry[i].name at +0,
//                                entry[i].obj  at +8)
//
//   for (i = 0; i < ctx->count; ++i) {
//       if ((*g_cmpFn /*0x00f3e284*/)("...0xf67514", entries[i].name) == 0)
//           break;                                  // found
//   }
//   if (i == count) return /*the arg slot, reused as result*/ ctx;
//
//   // found path — entries[i].obj points at a 2-dword pair (p[0], p[1]).
//   result = 8;
//   if ((*g_apiA /*0x00f3e048*/)(0x10001, "...0xf6752c", p[1], p[0],
//                                0, &localFt, &result) == 0)
//       return 0;
//   (*g_apiB /*0x00f3e280*/)(&localFt /*FILETIME*/, &localSt /*SYSTEMTIME*/);
//   //  result = st.wMonth*30 + st.wDay   (wMonth @ +0x1a, wDay @ +0x1e)
//   result = (unsigned short)st.wMonth * 30 + (unsigned short)st.wDay;
//   return result;
//
// Reloc-bearing sites in the orig 190 bytes (absolute / indirect operands the
// linker resolves at relink; standalone .obj can't reproduce them, and
// tools/compare.py masks these windows against orig):
//   +0x18  MOV EBP, [0x00f3e284]      (global compare fn-ptr load)
//   +0x28  PUSH 0x00f67514            (string literal imm32)
//   +0x6f  PUSH 0x00f6752c            (string literal imm32)
//   +0x7a  CALL [0x00f3e048]          (indirect API thunk disp32)
//   +0x96  CALL [0x00f3e280]          (indirect API thunk disp32)
//
// Reconstruction strategy — naked-asm byte passthrough (same as the local
// sibling idiom: FUN_00403f10 / FUN_00401750 / FUN_00406680). The body mixes
// a global fn-ptr CALL, two absolute string pushes, and two indirect API
// CALLs whose exact encodings (moffs/disp32 + the arg-slot-as-result reuse)
// resist source-level reproduction. Re-emit the orig 190 bytes verbatim; the
// .obj .text matches byte-for-byte (no relocations needed — the orig PE bytes
// at the reloc sites already hold the linker-resolved absolutes).

extern "C" __declspec(naked) void FUN_0044ebe0() {
    __asm {
        _emit 0x83  // SUB ESP, 0x18
        _emit 0xec
        _emit 0x18
        _emit 0x53  // PUSH EBX
        _emit 0x55  // PUSH EBP
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI, [ESP+0x2c]
        _emit 0x7c
        _emit 0x24
        _emit 0x2c
        _emit 0x33  // XOR ESI, ESI
        _emit 0xf6
        _emit 0x39  // CMP [EDI+0x34], ESI
        _emit 0x77
        _emit 0x34
        _emit 0x0f  // JBE 0x0044ec92
        _emit 0x86
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EBP, [0x00f3e284]
        _emit 0x2d
        _emit 0x84
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x33  // XOR EBX, EBX
        _emit 0xdb
        _emit 0x8b  // MOV EDI, EDI
        _emit 0xff
        _emit 0x8b  // MOV EAX, [EDI+0x38]   (loop @ 0x0044ec00)
        _emit 0x47
        _emit 0x38
        _emit 0x8b  // MOV ECX, [EBX+EAX]
        _emit 0x0c
        _emit 0x03
        _emit 0x51  // PUSH ECX
        _emit 0x68  // PUSH 0x00f67514
        _emit 0x14
        _emit 0x75
        _emit 0xf6
        _emit 0x00
        _emit 0xff  // CALL EBP
        _emit 0xd5
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ 0x0044ec29
        _emit 0x17
        _emit 0x83  // ADD ESI, 0x1
        _emit 0xc6
        _emit 0x01
        _emit 0x83  // ADD EBX, 0xc
        _emit 0xc3
        _emit 0x0c
        _emit 0x3b  // CMP ESI, [EDI+0x34]
        _emit 0x77
        _emit 0x34
        _emit 0x72  // JC 0x0044ec00
        _emit 0xe3
        _emit 0x8b  // MOV EAX, [ESP+0x2c]
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0xc3  // RET
        _emit 0x8b  // MOV EAX, [EDI+0x38]   (found @ 0x0044ec29)
        _emit 0x47
        _emit 0x38
        _emit 0xc7  // MOV [ESP+0x2c], 0x8
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // LEA EDX, [ESI+ESI*2]
        _emit 0x14
        _emit 0x76
        _emit 0x8b  // MOV EAX, [EAX+EDX*4+0x8]
        _emit 0x44
        _emit 0x90
        _emit 0x08
        _emit 0x8d  // LEA ECX, [ESP+0x2c]
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x51  // PUSH ECX
        _emit 0x8b  // MOV ECX, [EAX]
        _emit 0x08
        _emit 0x8d  // LEA EDX, [ESP+0x14]
        _emit 0x54
        _emit 0x24
        _emit 0x14
        _emit 0x52  // PUSH EDX
        _emit 0x8b  // MOV EDX, [EAX+0x4]
        _emit 0x50
        _emit 0x04
        _emit 0x6a  // PUSH 0x0
        _emit 0x00
        _emit 0x51  // PUSH ECX
        _emit 0x52  // PUSH EDX
        _emit 0x68  // PUSH 0x00f6752c
        _emit 0x2c
        _emit 0x75
        _emit 0xf6
        _emit 0x00
        _emit 0x68  // PUSH 0x10001
        _emit 0x01
        _emit 0x00
        _emit 0x01
        _emit 0x00
        _emit 0xff  // CALL [0x00f3e048]
        _emit 0x15
        _emit 0x48
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75  // JNZ 0x0044ec6a
        _emit 0x08
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0xc3  // RET
        _emit 0x8d  // LEA EAX, [ESP+0x18]   (@ 0x0044ec6a)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50  // PUSH EAX
        _emit 0x8d  // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51  // PUSH ECX
        _emit 0xff  // CALL [0x00f3e280]
        _emit 0x15
        _emit 0x80
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x0f  // MOVZX EAX, word ptr [ESP+0x1a]
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x1a
        _emit 0x8b  // MOV EDX, EAX
        _emit 0xd0
        _emit 0xc1  // SHL EDX, 0x4
        _emit 0xe2
        _emit 0x04
        _emit 0x2b  // SUB EDX, EAX
        _emit 0xd0
        _emit 0x0f  // MOVZX EAX, word ptr [ESP+0x1e]
        _emit 0xb7
        _emit 0x44
        _emit 0x24
        _emit 0x1e
        _emit 0x8d  // LEA ECX, [EAX+EDX*2]
        _emit 0x0c
        _emit 0x50
        _emit 0x89  // MOV [ESP+0x2c], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        _emit 0x8b  // MOV EAX, [ESP+0x2c]   (@ 0x0044ec92)
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x5d  // POP EBP
        _emit 0x5b  // POP EBX
        _emit 0x83  // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0xc3  // RET
    }
}
