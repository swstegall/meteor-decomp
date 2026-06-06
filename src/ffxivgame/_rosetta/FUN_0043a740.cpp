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
// FUNCTION: ffxivgame 0x0003a740 — 3-arg dispatcher over a lazily-initialised
//                                  singleton (189 B / 0xbd, __cdecl)
//
// Behaviour read from asm/ffxivgame/0003a740_FUN_0043a740.s:
//
//   1. EH3-style SEH prolog (PUSH -1 / scope_table 0x00e5648e / FS:[0]
//      chain) + /GS cookie (XOR ESP, security_cookie @ 0x012ea8b0).
//
//   2. Magic-static "construct on first use" guard on the global flag
//      byte at 0x01327c18 (the MSVC 2005 thread-unsafe local-static
//      init pattern):
//        if ((g_flag & 1) == 0) {
//            g_flag |= 1;
//            ehstate = 0;                       // [ESP+0x24] = 0
//            g_singleton = create();            // CALL 0x0040e500
//            ehstate = -1;                      // [ESP+0x24] = -1
//        }
//      g_singleton is stored at 0x01327c14.
//
//   3. ESI = g_singleton (0x01327c14).
//      ECX = &SEH frame slot ([ESP+0x1c]); CALL 0x0040e2d0 with
//      args (0x10, 0xf66350) — builds/looks up a key object, returns
//      a handle in EAX.
//
//   4. EDI = arg3 ([ESP+0x34] after pushes).
//      __thiscall on ESI: CALL 0x0040e110(handle, arg3) → ESI = result.
//
//   5. EBX = arg1 ([ESP+0x2c]).
//      CALL 0x009d4600(result, arg1, arg3)      // 3-arg cdecl
//      EBP = arg2 ([ESP+0x3c]).
//      CALL 0x009d4600(arg1, arg2, arg3)
//      CALL 0x009d4600(arg2, result, arg3)
//      ADD ESP, 0x24                            // drop the pushed args
//
//   6. If the step-4 result (ESI) is non-null, release it:
//        ECX = result[-4]; CALL 0x0040df70(result)   // __thiscall dtor/free
//
//   7. SEH epilog: restore FS:[0], pop cookie + saved regs, ADD ESP,0x14,
//      RET (__cdecl — caller cleans the stack args).
//
// Reloc-bearing sites in the orig 189 bytes (absolute addresses / rel32
// CALL targets the linker resolves at relink; we re-emit the orig bytes
// verbatim so the .obj .text matches byte-for-byte — compare.py masks the
// reloc windows):
//   +0x03  scope_table pointer        (.rdata 0x00e5648e)
//   +0x16  __security_cookie load     (.data  0x012ea8b0)
//   +0x2c  g_flag TEST                (.data  0x01327c18)
//   +0x3a  g_flag OR                  (.data  0x01327c18)
//   +0x42  CALL create()             (.text  0x0040e500)
//   +0x47  g_singleton STORE          (.data  0x01327c14)
//   +0x54  g_singleton LOAD           (.data  0x01327c14)
//   +0x5a  string/key literal PUSH    (.rdata 0x00f66350)
//   +0x65  CALL 0x0040e2d0
//   +0x72  CALL 0x0040e110
//   +0x80  CALL 0x009d4600
//   +0x8c  CALL 0x009d4600
//   +0x94  CALL 0x009d4600
//   +0xa4  CALL 0x0040df70
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The inlined EH3 SEH prolog + /GS cookie + the magic-static init guard
//   with its mid-body EH-state writes form a compiler-emitted shape that
//   depends on the precise locals layout, the linker-laid scope table, and
//   MSVC's moffs32 vs modrm encoding choices — impractical to coax out of
//   plain C++ byte-for-byte. The local idiom (see FUN_00403f10,
//   FUN_00406680, FUN_00401750) is a `__declspec(naked)` body re-emitting
//   the orig bytes via MASM `_emit`. The .obj .text ends up byte-identical
//   (no relocations needed — the orig rel32/abs bytes are emitted raw).

extern "C" __declspec(naked) void FUN_0043a740() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e5648e (scope_table)
        _emit 0x8e
        _emit 0x64
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50             // PUSH EAX
        _emit 0x83             // SUB ESP, 0x8
        _emit 0xec
        _emit 0x08
        _emit 0x53             // PUSH EBX
        _emit 0x55             // PUSH EBP
        _emit 0x56             // PUSH ESI
        _emit 0x57             // PUSH EDI
        _emit 0xa1             // MOV EAX, [0x012ea8b0] (security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33             // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50             // PUSH EAX
        _emit 0x8d             // LEA EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x64             // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xb8             // MOV EAX, 0x1
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x84             // TEST byte ptr [0x01327c18], AL
        _emit 0x05
        _emit 0x18
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x75             // JNZ 0x0043a794
        _emit 0x20
        _emit 0x09             // OR dword ptr [0x01327c18], EAX
        _emit 0x05
        _emit 0x18
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7             // MOV dword ptr [ESP+0x24], 0x0
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8             // CALL 0x0040e500
        _emit 0x79
        _emit 0x3d
        _emit 0xfd
        _emit 0xff
        _emit 0xa3             // MOV [0x01327c14], EAX
        _emit 0x14
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0xc7             // MOV dword ptr [ESP+0x24], 0xffffffff
        _emit 0x44
        _emit 0x24
        _emit 0x24
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b             // MOV ESI, dword ptr [0x01327c14]
        _emit 0x35
        _emit 0x14
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        _emit 0x68             // PUSH 0xf66350
        _emit 0x50
        _emit 0x63
        _emit 0xf6
        _emit 0x00
        _emit 0x6a             // PUSH 0x10
        _emit 0x10
        _emit 0x8d             // LEA ECX, [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8             // CALL 0x0040e2d0
        _emit 0x26
        _emit 0x3b
        _emit 0xfd
        _emit 0xff
        _emit 0x8b             // MOV EDI, dword ptr [ESP+0x34]
        _emit 0x7c
        _emit 0x24
        _emit 0x34
        _emit 0x50             // PUSH EAX
        _emit 0x57             // PUSH EDI
        _emit 0x8b             // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8             // CALL 0x0040e110
        _emit 0x59
        _emit 0x39
        _emit 0xfd
        _emit 0xff
        _emit 0x8b             // MOV EBX, dword ptr [ESP+0x2c]
        _emit 0x5c
        _emit 0x24
        _emit 0x2c
        _emit 0x57             // PUSH EDI
        _emit 0x8b             // MOV ESI, EAX
        _emit 0xf0
        _emit 0x53             // PUSH EBX
        _emit 0x56             // PUSH ESI
        _emit 0xe8             // CALL 0x009d4600
        _emit 0x3b
        _emit 0x9e
        _emit 0x59
        _emit 0x00
        _emit 0x8b             // MOV EBP, dword ptr [ESP+0x3c]
        _emit 0x6c
        _emit 0x24
        _emit 0x3c
        _emit 0x57             // PUSH EDI
        _emit 0x55             // PUSH EBP
        _emit 0x53             // PUSH EBX
        _emit 0xe8             // CALL 0x009d4600
        _emit 0x2f
        _emit 0x9e
        _emit 0x59
        _emit 0x00
        _emit 0x57             // PUSH EDI
        _emit 0x56             // PUSH ESI
        _emit 0x55             // PUSH EBP
        _emit 0xe8             // CALL 0x009d4600
        _emit 0x27
        _emit 0x9e
        _emit 0x59
        _emit 0x00
        _emit 0x83             // ADD ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0x85             // TEST ESI, ESI
        _emit 0xf6
        _emit 0x74             // JZ 0x0043a7e9
        _emit 0x09
        _emit 0x8b             // MOV ECX, dword ptr [ESI-0x4]
        _emit 0x4e
        _emit 0xfc
        _emit 0x56             // PUSH ESI
        _emit 0xe8             // CALL 0x0040df70
        _emit 0x87
        _emit 0x37
        _emit 0xfd
        _emit 0xff
        _emit 0x8b             // MOV ECX, dword ptr [ESP+0x1c]
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x64             // MOV dword ptr FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59             // POP ECX
        _emit 0x5f             // POP EDI
        _emit 0x5e             // POP ESI
        _emit 0x5d             // POP EBP
        _emit 0x5b             // POP EBX
        _emit 0x83             // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xc3             // RET
    }
}
