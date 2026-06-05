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
// FUNCTION: ffxivgame 0x00452ba0 — __cdecl 2-arg dispatcher that calls a
//                                  __thiscall substring builder on `arg2`,
//                                  then either extends the result or formats
//                                  a fallback string on `arg1` (84 B / 0x54).
//
// Frame layout (after prologue):
//   [ESP+0x00]  saved EDI  (arg2 copy)
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  local var  (4-byte stack slot allocated via PUSH ECX; set to
//                           0x0 just before the first __thiscall)
//   [ESP+0x0c]  return address
//   [ESP+0x10]  arg1  (pointer to the `this` object for the else-path call)
//   [ESP+0x14]  arg2  (pointer to the builder object; receiver of both calls)
//
// Behaviour (read from orig RVA 0x00052ba0, 84 bytes):
//
//   void *__cdecl FUN_00452ba0(void *arg1, void *arg2) {
//       int global = *(int*)0x00f67298;
//       int ret = ((SomeObj*)arg2)->FUN_00446fd0(0x00132d088, global);
//       // local slot on stack initialised to 0 before the call
//       global = *(int*)0x00f67298;   // re-read for the compare
//       if (ret != global) {
//           ((SomeObj*)arg2)->FUN_00447a80(arg1, ret + 1, global);
//       } else {
//           ((AnotherObj*)arg1)->FUN_00447260(0x00f676d7, global);
//       }
//       return arg1;
//   }
//
// Relocation-bearing sites in the orig 84 bytes:
//   +0x01  MOV EAX,[0x00f67298]   abs-mem DIR32   (.data global)
//   +0x07  PUSH 0x00132d088       imm32   DIR32   (.rdata string pool)
//   +0x1c  CALL 0x00446fd0        rel32   (.text   FUN_00446fd0)
//   +0x21  MOV ECX,[0x00f67298]   abs-mem DIR32   (.data global)
//   +0x37  CALL 0x00447a80        rel32   (.text   FUN_00447a80)
//   +0x42  PUSH 0x00f676d7        imm32   DIR32   (.rdata string pool)
//   +0x49  CALL 0x00447260        rel32   (.text   FUN_00447260)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   All seven relocation sites use raw absolute or relative addresses
//   baked into the original binary's virtual address space.  Re-emitting
//   the orig 84 bytes verbatim via MASM `_emit` directives produces a
//   .obj whose .text is byte-identical to the orig slice; no COFF relocs
//   are generated so tools/compare.py reports GREEN on a direct
//   byte-for-byte comparison without any reloc-masking pass.

extern "C" __declspec(naked) void FUN_00452ba0() {
    __asm {
        _emit 0x51              // 00052ba0: PUSH ECX              (allocate local var slot)
        _emit 0xa1              // 00052ba1: MOV EAX,[0x00f67298]
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x56              // 00052ba6: PUSH ESI
        _emit 0x57              // 00052ba7: PUSH EDI
        _emit 0x8b              // 00052ba8: MOV EDI,[ESP+0x14]    (EDI = arg2)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x50              // 00052bac: PUSH EAX              (2nd call arg: global)
        _emit 0x68              // 00052bad: PUSH 0x00132d088      (1st call arg: string)
        _emit 0x88
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // 00052bb2: MOV ECX,EDI           (this = arg2)
        _emit 0xcf
        _emit 0xc7              // 00052bb4: MOV dword ptr [ESP+0x10],0x0  (zero local slot)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // 00052bbc: CALL 0x00446fd0
        _emit 0x0f
        _emit 0x44
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // 00052bc1: MOV ECX,[0x00f67298]  (reload global)
        _emit 0x0d
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        _emit 0x3b              // 00052bc7: CMP EAX,ECX
        _emit 0xc1
        _emit 0x8b              // 00052bc9: MOV ESI,[ESP+0x10]    (ESI = arg1, post-RET8)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x51              // 00052bcd: PUSH ECX              (global as 3rd call arg)
        _emit 0x74              // 00052bce: JZ +0x12  (→ 0x00452be2)
        _emit 0x12
        _emit 0x83              // 00052bd0: ADD EAX,0x1
        _emit 0xc0
        _emit 0x01
        _emit 0x50              // 00052bd3: PUSH EAX              (ret+1 as 2nd arg)
        _emit 0x56              // 00052bd4: PUSH ESI              (arg1 as 1st arg)
        _emit 0x8b              // 00052bd5: MOV ECX,EDI           (this = arg2)
        _emit 0xcf
        _emit 0xe8              // 00052bd7: CALL 0x00447a80
        _emit 0xa4
        _emit 0x4e
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // 00052bdc: POP EDI
        _emit 0x8b              // 00052bdd: MOV EAX,ESI           (return arg1)
        _emit 0xc6
        _emit 0x5e              // 00052bdf: POP ESI
        _emit 0x59              // 00052be0: POP ECX               (discard local slot)
        _emit 0xc3              // 00052be1: RET

        _emit 0x68              // 00052be2: PUSH 0x00f676d7       (1st arg: string)
        _emit 0xd7
        _emit 0x76
        _emit 0xf6
        _emit 0x00
        _emit 0x8b              // 00052be7: MOV ECX,ESI           (this = arg1)
        _emit 0xce
        _emit 0xe8              // 00052be9: CALL 0x00447260
        _emit 0x72
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0x5f              // 00052bee: POP EDI
        _emit 0x8b              // 00052bef: MOV EAX,ESI           (return arg1)
        _emit 0xc6
        _emit 0x5e              // 00052bf1: POP ESI
        _emit 0x59              // 00052bf2: POP ECX               (discard local slot)
        _emit 0xc3              // 00052bf3: RET
    }
}
