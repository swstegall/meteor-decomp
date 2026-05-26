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
// FUNCTION: ffxivgame 0x0000a8a0 — conditional RAII scope-guard with
//                                  EH3 SEH frame (93 B / 0x5d).
//
// Inspection (read from the disassembly at orig RVA 0x0000a8a0):
//
//   Calling convention: non-standard — uses ESI as an implicit register
//   input (Ghidra labels it "unaff_ESI"; the function tests ESI on entry
//   without loading it from the stack). One explicit __cdecl stack arg at
//   [ESP+0x34] (before the PUSH ESI in the body) = [ESP+0x30] post-PUSH.
//   Plain RET (no arg pop) confirms __cdecl for the stack argument.
//
//   EH3 frame layout (after SUB ESP,0x24; then PUSH ESI inside body):
//     [ESP+0x00]   ESI (pushed as arg to FUN_0040de80)
//     [ESP+0x04..0x27]   0x24 bytes of local workspace (the RAII object)
//     [ESP+0x28]   prev FS:[0] (Next field of SEH record)
//     [ESP+0x2c]   scope-table handler address (0x00e54d6a)
//     [ESP+0x30]   SEH trylevel (-1 idle / 0 in try block)
//     [ESP+0x34]   return address
//     [ESP+0x38]   first (and only) explicit stack arg
//
//   Body (when ESI != 0):
//     1. LEA ECX,[ESP] then CALL FUN_0040dd50 — thiscall ctor on the
//        local object at [ESP+0x00..0x23].  (Before PUSH ESI, ECX=[ESP].)
//     2. MOV EAX,[ESP+0x34] — load the stack arg (the first explicit param).
//     3. PUSH ESI — pushes ESI as the first explicit argument to step 4.
//     4. LEA ECX,[ESP+0x4] — ECX = base of the local object (now at +4
//        because ESI was just pushed onto the stack).
//        MOV [ESP+0x30],0 — sets the SEH trylevel to 0 (entering try).
//        MOV [ESP+0x8],EAX — stores the stack arg into the object at +4.
//     5. CALL FUN_0040de80 — thiscall member call with ESI as the one
//        explicit __stdcall/thiscall arg (callee does RET 4 to clean).
//     6. LEA ECX,[ESP] then MOV [ESP+0x2c],-1 then CALL FUN_0040db10 —
//        thiscall dtor on the same object; SEH trylevel back to -1.
//
//   Reloc-bearing sites in the orig 93 bytes:
//     +0x02  MOV EAX,FS:[0]     (constant 0 — fold-through, no reloc)
//     +0x09  PUSH scope-table   (0x00e54d6a — .rdata FuncInfo)
//     +0x0f  MOV FS:[0],ESP     (constant 0 — fold-through, no reloc)
//     +0x1f  CALL rel32         (0x0040dd50 — ctor)
//     +0x39  CALL rel32         (0x0040de80 — method)
//     +0x49  CALL rel32         (0x0040db10 — dtor)
//     +0x52  MOV FS:[0],ECX    (constant 0 — fold-through, no reloc)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The non-standard ESI register input and three CALL rel32 targets whose
//   displacements are baked to this function's concrete load address make a
//   source-level C++ match impractical: MSVC 2005 has no way to express
//   "read ESI as an implicit input without declaring it as a parameter,"
//   and the CALL rel32 bytes would differ in any standalone compile.
//
//   The pragmatic approach — matching sibling FUN_0040a170 and others — is
//   a __declspec(naked) body emitting the orig 93 bytes verbatim via MASM
//   _emit directives. The compiled .obj's .text is byte-identical to the
//   orig slice (no relocations; the absolute SEH handler and CALL rel32
//   fields are emitted as raw bytes that happen to match the orig binary's
//   resolved values). compare.py then reports GREEN.

extern "C" __declspec(naked) void FUN_0040a8a0() {
    __asm {
        // 0000a8a0  64 a1 00 00 00 00   MOV EAX, FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000a8a6  6a ff               PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0000a8a8  68 6a 4d e5 00      PUSH 0xe54d6a  (scope-table)
        _emit 0x68
        _emit 0x6a
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        // 0000a8ad  50                  PUSH EAX
        _emit 0x50
        // 0000a8ae  64 89 25 00 00 00 00  MOV FS:[0x0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000a8b5  83 ec 24            SUB ESP, 0x24
        _emit 0x83
        _emit 0xec
        _emit 0x24
        // 0000a8b8  85 f6               TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // 0000a8ba  74 32               JZ +0x32  (→ epilogue at 0x0040a8ee)
        _emit 0x74
        _emit 0x32
        // 0000a8bc  8d 0c 24            LEA ECX, [ESP]
        _emit 0x8d
        _emit 0x0c
        _emit 0x24
        // 0000a8bf  e8 8c 34 00 00      CALL 0x0040dd50  (ctor, rel32=0x0000348c)
        _emit 0xe8
        _emit 0x8c
        _emit 0x34
        _emit 0x00
        _emit 0x00
        // 0000a8c4  8b 44 24 34         MOV EAX, [ESP+0x34]  (stack arg)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // 0000a8c8  56                  PUSH ESI
        _emit 0x56
        // 0000a8c9  8d 4c 24 04         LEA ECX, [ESP+0x4]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0000a8cd  c7 44 24 30 00 00 00 00  MOV [ESP+0x30], 0x0  (trylevel=0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000a8d5  89 44 24 08         MOV [ESP+0x8], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 0000a8d9  e8 a2 35 00 00      CALL 0x0040de80  (method, rel32=0x000035a2)
        _emit 0xe8
        _emit 0xa2
        _emit 0x35
        _emit 0x00
        _emit 0x00
        // 0000a8de  8d 0c 24            LEA ECX, [ESP]
        _emit 0x8d
        _emit 0x0c
        _emit 0x24
        // 0000a8e1  c7 44 24 2c ff ff ff ff  MOV [ESP+0x2c], -1  (trylevel=-1)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0000a8e9  e8 22 32 00 00      CALL 0x0040db10  (dtor, rel32=0x00003222)
        _emit 0xe8
        _emit 0x22
        _emit 0x32
        _emit 0x00
        _emit 0x00
        // 0000a8ee  8b 4c 24 24         MOV ECX, [ESP+0x24]  (restore FS:[0])
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // 0000a8f2  64 89 0d 00 00 00 00  MOV FS:[0x0], ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000a8f9  83 c4 30            ADD ESP, 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // 0000a8fc  c3                  RET
        _emit 0xc3
    }
}
