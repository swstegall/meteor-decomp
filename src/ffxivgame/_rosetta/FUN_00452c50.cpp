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
// FUNCTION: ffxivgame 0x00452c50 — container lookup-or-insert helper
//                                   (82 B / 0x52, __cdecl, returns arg1 ptr)
//
// Behaviour read from the disassembly at orig RVA 0x00052c50:
//
//   void* __cdecl FUN_00452c50(void* arg1, void* arg2) {
//       // PUSH ECX allocates a 4-byte temp slot on the stack (zeroed later).
//       // ESI = arg2 (container/object pointer, used as __thiscall `this`).
//
//       unsigned g_end = *(unsigned*)0x00f67298;   // pre-load sentinel / end()
//
//       // Search in arg2 container: __thiscall(arg2, 0x132d030, g_end, 0)
//       unsigned pos = FUN_00446fd0(arg2, 0x132d030, g_end, 0);
//
//       if (pos == *(unsigned*)0x00f67298) {
//           // Not found — insert arg2 into arg1's collection:
//           // __thiscall(arg1, arg2)
//           FUN_00447200(arg1, arg2);
//       } else {
//           // Found — update position: __thiscall(arg2, arg1, 0, pos+1)
//           FUN_00447a80(arg2, arg1, 0, pos + 1);
//       }
//       return arg1;
//   }
//
// Reloc-bearing sites (masked by tools/compare.py during the diff):
//   +0x02   DIR32  MOV EAX,[0x00f67298]   (sentinel global — first read)
//   +0x0d   imm32  PUSH 0x132d030         (data/string pointer)
//   +0x1c   REL32  CALL FUN_00446fd0      (find / lower_bound)
//   +0x22   DIR32  CMP EAX,[0x00f67298]   (sentinel global — second read)
//   +0x37   REL32  CALL FUN_00447a80      (insert-at-position / erase)
//   +0x49   REL32  CALL FUN_00447200      (insert / link)
//
// Frame layout at CALL 0x00446fd0:
//   PUSH ECX (temp local, later zeroed) + PUSH ESI (callee-save) → 2 dwords
//   before function args, so arg1 = [ESP+0xC], arg2 = [ESP+0x10] on entry.
//   The temp-ECX slot ([ESP+0xC] after two more pushes for args) is explicitly
//   zeroed with MOV [ESP+0C],0 — this is a local zero-init emitted by MSVC,
//   not a hidden argument to the callee.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Six reloc-bearing operands, a non-trivial two-exit control flow (JZ
//   branch + fall-through), and the unusual PUSH ECX / POP ECX frame idiom
//   all make a source-level port extremely fragile (MSVC would need to emit
//   the exact same temp-slot pattern, the same JZ displacement, and the same
//   thiscall ECX setup in both arms). Following the _rosetta/ precedent set
//   by FUN_00401090, FUN_00404f10, and FUN_004016d0, the 82 orig bytes are
//   re-emitted verbatim via MASM _emit directives; compare.py masks the six
//   reloc windows during the diff so the match is byte-identical.

extern "C" __declspec(naked) void FUN_00452c50() {
    __asm {
        // 00052c50: push ecx
        _emit 0x51
        // 00052c51: mov eax, dword ptr [0x00f67298]
        _emit 0xa1
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00052c56: push esi
        _emit 0x56
        // 00052c57: mov esi, dword ptr [esp+10h]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00052c5b: push eax
        _emit 0x50
        // 00052c5c: push 0x132d030
        _emit 0x68
        _emit 0x30
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 00052c61: mov ecx, esi
        _emit 0x8b
        _emit 0xce
        // 00052c63: mov dword ptr [esp+0ch], 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00052c6b: call FUN_00446fd0
        _emit 0xe8
        _emit 0x60
        _emit 0x43
        _emit 0xff
        _emit 0xff
        // 00052c70: cmp eax, dword ptr [0x00f67298]
        _emit 0x3b
        _emit 0x05
        _emit 0x98
        _emit 0x72
        _emit 0xf6
        _emit 0x00
        // 00052c76: jz +0x19 (→ 0x00452c91)
        _emit 0x74
        _emit 0x19
        // 00052c78: push edi
        _emit 0x57
        // 00052c79: mov edi, dword ptr [esp+10h]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 00052c7d: add eax, 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00052c80: push eax
        _emit 0x50
        // 00052c81: push 0
        _emit 0x6a
        _emit 0x00
        // 00052c83: push edi
        _emit 0x57
        // 00052c84: mov ecx, esi
        _emit 0x8b
        _emit 0xce
        // 00052c86: call FUN_00447a80
        _emit 0xe8
        _emit 0xf5
        _emit 0x4d
        _emit 0xff
        _emit 0xff
        // 00052c8b: mov eax, edi
        _emit 0x8b
        _emit 0xc7
        // 00052c8d: pop edi
        _emit 0x5f
        // 00052c8e: pop esi
        _emit 0x5e
        // 00052c8f: pop ecx
        _emit 0x59
        // 00052c90: ret
        _emit 0xc3
        // --- JZ target: not-found arm (0x00452c91) ---
        // 00052c91: push esi
        _emit 0x56
        // 00052c92: mov esi, dword ptr [esp+10h]
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00052c96: mov ecx, esi
        _emit 0x8b
        _emit 0xce
        // 00052c98: call FUN_00447200
        _emit 0xe8
        _emit 0x63
        _emit 0x45
        _emit 0xff
        _emit 0xff
        // 00052c9d: mov eax, esi
        _emit 0x8b
        _emit 0xc6
        // 00052c9f: pop esi
        _emit 0x5e
        // 00052ca0: pop ecx
        _emit 0x59
        // 00052ca1: ret
        _emit 0xc3
    }
}
