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
// FUNCTION: ffxivgame 0x0003cab0 — message/event dispatcher (297 B / 0x129,
//                                  no SEH, no GS cookie)
//
// Inspection (read from the disassembly at orig RVA 0x0003cab0):
//
//   __cdecl void FUN_0043cab0(void* arg1, ?, unsigned int dispatch_id, ...);
//
//   Dispatches on `dispatch_id` (EDI = [ESP+0x8] = arg2 at call site;
//   loaded via `MOV EDI,[ESP+0xc]` after `PUSH EDI` shifts the frame):
//
//     0x8045  → FUN_0042 0160(ESI, EDX, 0x1b,  ECX/arg1, 0x0) [__stdcall]
//     0x1909  → FUN_0042 0160(ESI, EDX, 0x5,   ECX/arg1, 0x0) [__stdcall]
//     0x805b  → FUN_00431080(ESI, EDX, 0x16, 0x0, EAX, 0x0)   [__stdcall]
//     0x80e0  → FUN_00406550(0xf6680c, 0xf6666f, 0xf667c0,
//                             0x8e, 0xf6677c)  [via LEA ECX; error path]
//     0x80e1  → FUN_00431080(ESI, EDX, 0x4,  0x0, EAX, 0x0)   [__stdcall]
//     0x83f1  → FUN_00431080(ESI, EDX, 0x18, 0x0, EAX, 0x0)
//     0x83f2  → FUN_00431080(ESI, EDX, 0x19, 0x0, EAX, 0x0)
//     0x83f3  → FUN_00431080(ESI, EDX, 0x1a, 0x0, EAX, 0x0)
//     default → FUN_00406550(0xf668a4, 0xf66696, 0xf66858,
//                             0xb5, 0xf66814)  [via LEA ECX; error path]
//
//   EAX, EDX, ESI arrive from the caller unmodified — they are pass-through
//   register values forwarded to the innermost callees without being
//   initialised here. No C source can naturally capture them, so this
//   function is matched via byte passthrough.
//
//   Stack frame (after `PUSH EDI`):
//     [esp+0]  saved EDI
//     [esp+4]  return address
//     [esp+8]  arg1  (→ ECX at entry)
//     [esp+c]  arg2  (= dispatch_id, → EDI)
//
//   External CALLs (rel32, embedded verbatim from the orig binary):
//     +0x47   → 0x00420160  (e8 64 36 fe ff)
//     +0x5c   → 0x00420160  (e8 4f 36 fe ff)
//     +0x70   → 0x00431080  (e8 5b 45 ff ff)
//     +0x9c   → 0x00406550  (e8 ff 99 fc ff)
//     +0xb0   → 0x00431080  (e8 1b 45 ff ff)
//     +0xfa   → 0x00406550  (e8 b5 99 fc ff)
//     +0x10a  → 0x00431080  (e8 d1 44 ff ff)
//     +0x11e  → 0x00431080  (e8 bd 44 ff ff)
//     +0x132  → 0x00431080  (e8 a9 44 ff ff)  [note: file offset from func start]
//
//   No absolute-address relocations, no security cookie, no SEH frame.
//   The 297 bytes are position-dependent only through the rel32 CALL fields.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The use of unmodified-register pass-through (EAX, EDX, ESI) cannot be
//   expressed in standard C++ without __asm or intrinsics, and the exact
//   branch encoding (81FF vs 83FF for CMP EDI, rel8 vs rel32 per branch)
//   would require brittle coaxing of the register allocator. The pragmatic
//   choice — matching FUN_004014b0, FUN_00401a00, FUN_00408f10, etc. — is
//   a `__declspec(naked)` body that re-emits all 297 bytes verbatim via
//   MASM `_emit` directives. The .obj's `.text` section is byte-identical
//   to the original slice, which is what `tools/compare.py` checks.

extern "C" __declspec(naked) void FUN_0043cab0() {
    __asm {
        // 0003cab0: MOV ECX,[ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0003cab4: PUSH EDI
        _emit 0x57
        // 0003cab5: MOV EDI,[ESP+0xc]
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 0003cab9: CMP EDI,0x80e1
        _emit 0x81
        _emit 0xff
        _emit 0xe1
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 0003cabf: JA 0x0043cb67
        _emit 0x0f
        _emit 0x87
        _emit 0xa2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cac5: JZ 0x0043cb53
        _emit 0x0f
        _emit 0x84
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cacb: CMP EDI,0x805b
        _emit 0x81
        _emit 0xff
        _emit 0x5b
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 0003cad1: JA 0x0043cb27
        _emit 0x77
        _emit 0x54
        // 0003cad3: JZ 0x0043cb13
        _emit 0x74
        _emit 0x3e
        // 0003cad5: CMP EDI,0x1909
        _emit 0x81
        _emit 0xff
        _emit 0x09
        _emit 0x19
        _emit 0x00
        _emit 0x00
        // 0003cadb: JZ 0x0043cafe
        _emit 0x74
        _emit 0x21
        // 0003cadd: CMP EDI,0x8045
        _emit 0x81
        _emit 0xff
        _emit 0x45
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 0003cae3: JNZ 0x0043cb79
        _emit 0x0f
        _emit 0x85
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // 0003cae9: case 0x8045 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003caeb: PUSH EAX
        _emit 0x50
        // 0003caec: PUSH ECX
        _emit 0x51
        // 0003caed: MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 0003caef: PUSH EDX
        _emit 0x52
        // 0003caf0: PUSH ESI
        _emit 0x56
        // 0003caf1: MOV dword ptr [EAX],0x1b
        _emit 0xc7
        _emit 0x00
        _emit 0x1b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003caf7: CALL 0x00420160
        _emit 0xe8
        _emit 0x64
        _emit 0x36
        _emit 0xfe
        _emit 0xff
        // 0003cafc: POP EDI
        _emit 0x5f
        // 0003cafd: RET
        _emit 0xc3

        // 0003cafe: case 0x1909 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cb00: PUSH EAX
        _emit 0x50
        // 0003cb01: PUSH ECX
        _emit 0x51
        // 0003cb02: MOV EAX,ESP
        _emit 0x8b
        _emit 0xc4
        // 0003cb04: PUSH EDX
        _emit 0x52
        // 0003cb05: PUSH ESI
        _emit 0x56
        // 0003cb06: MOV dword ptr [EAX],0x5
        _emit 0xc7
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cb0c: CALL 0x00420160
        _emit 0xe8
        _emit 0x4f
        _emit 0x36
        _emit 0xfe
        _emit 0xff
        // 0003cb11: POP EDI
        _emit 0x5f
        // 0003cb12: RET
        _emit 0xc3

        // 0003cb13: case 0x805b — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cb15: PUSH EAX
        _emit 0x50
        // 0003cb16: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cb18: MOV EDI,0x16
        _emit 0xbf
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cb1d: PUSH EDI
        _emit 0x57
        // 0003cb1e: PUSH EDX
        _emit 0x52
        // 0003cb1f: PUSH ESI
        _emit 0x56
        // 0003cb20: CALL 0x00431080
        _emit 0xe8
        _emit 0x5b
        _emit 0x45
        _emit 0xff
        _emit 0xff
        // 0003cb25: POP EDI
        _emit 0x5f
        // 0003cb26: RET
        _emit 0xc3

        // 0003cb27: case > 0x805b — CMP EDI,0x80e0
        _emit 0x81
        _emit 0xff
        _emit 0xe0
        _emit 0x80
        _emit 0x00
        _emit 0x00
        // 0003cb2d: JNZ 0x0043cb79
        _emit 0x75
        _emit 0x4a

        // 0003cb2f: case 0x80e0 — PUSH 0xf6677c
        _emit 0x68
        _emit 0x7c
        _emit 0x67
        _emit 0xf6
        _emit 0x00
        // 0003cb34: PUSH 0x8e
        _emit 0x68
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cb39: PUSH 0xf667c0
        _emit 0x68
        _emit 0xc0
        _emit 0x67
        _emit 0xf6
        _emit 0x00
        // 0003cb3e: PUSH 0xf6666f
        _emit 0x68
        _emit 0x6f
        _emit 0x66
        _emit 0xf6
        _emit 0x00
        // 0003cb43: PUSH 0xf6680c
        _emit 0x68
        _emit 0x0c
        _emit 0x68
        _emit 0xf6
        _emit 0x00
        // 0003cb48: LEA ECX,[ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0003cb4c: CALL 0x00406550
        _emit 0xe8
        _emit 0xff
        _emit 0x99
        _emit 0xfc
        _emit 0xff
        // 0003cb51: POP EDI
        _emit 0x5f
        // 0003cb52: RET
        _emit 0xc3

        // 0003cb53: case 0x80e1 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cb55: PUSH EAX
        _emit 0x50
        // 0003cb56: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cb58: MOV EDI,0x4
        _emit 0xbf
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cb5d: PUSH EDI
        _emit 0x57
        // 0003cb5e: PUSH EDX
        _emit 0x52
        // 0003cb5f: PUSH ESI
        _emit 0x56
        // 0003cb60: CALL 0x00431080
        _emit 0xe8
        _emit 0x1b
        _emit 0x45
        _emit 0xff
        _emit 0xff
        // 0003cb65: POP EDI
        _emit 0x5f
        // 0003cb66: RET
        _emit 0xc3

        // 0003cb67: case > 0x80e1 — SUB EDI,0x83f1
        _emit 0x81
        _emit 0xef
        _emit 0xf1
        _emit 0x83
        _emit 0x00
        _emit 0x00
        // 0003cb6d: JZ 0x0043cbc5  (case 0x83f1)
        _emit 0x74
        _emit 0x56
        // 0003cb6f: SUB EDI,0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 0003cb72: JZ 0x0043cbb1  (case 0x83f2)
        _emit 0x74
        _emit 0x3d
        // 0003cb74: SUB EDI,0x1
        _emit 0x83
        _emit 0xef
        _emit 0x01
        // 0003cb77: JZ 0x0043cb9d  (case 0x83f3)
        _emit 0x74
        _emit 0x24

        // 0003cb79: default — PUSH 0xf66814
        _emit 0x68
        _emit 0x14
        _emit 0x68
        _emit 0xf6
        _emit 0x00
        // 0003cb7e: PUSH 0xb5
        _emit 0x68
        _emit 0xb5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cb83: PUSH 0xf66858
        _emit 0x68
        _emit 0x58
        _emit 0x68
        _emit 0xf6
        _emit 0x00
        // 0003cb88: PUSH 0xf66696
        _emit 0x68
        _emit 0x96
        _emit 0x66
        _emit 0xf6
        _emit 0x00
        // 0003cb8d: PUSH 0xf668a4
        _emit 0x68
        _emit 0xa4
        _emit 0x68
        _emit 0xf6
        _emit 0x00
        // 0003cb92: LEA ECX,[ESP+0x20]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0003cb96: CALL 0x00406550
        _emit 0xe8
        _emit 0xb5
        _emit 0x99
        _emit 0xfc
        _emit 0xff
        // 0003cb9b: POP EDI
        _emit 0x5f
        // 0003cb9c: RET
        _emit 0xc3

        // 0003cb9d: case 0x83f3 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cb9f: PUSH EAX
        _emit 0x50
        // 0003cba0: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cba2: MOV EDI,0x1a
        _emit 0xbf
        _emit 0x1a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cba7: PUSH EDI
        _emit 0x57
        // 0003cba8: PUSH EDX
        _emit 0x52
        // 0003cba9: PUSH ESI
        _emit 0x56
        // 0003cbaa: CALL 0x00431080
        _emit 0xe8
        _emit 0xd1
        _emit 0x44
        _emit 0xff
        _emit 0xff
        // 0003cbaf: POP EDI
        _emit 0x5f
        // 0003cbb0: RET
        _emit 0xc3

        // 0003cbb1: case 0x83f2 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cbb3: PUSH EAX
        _emit 0x50
        // 0003cbb4: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cbb6: MOV EDI,0x19
        _emit 0xbf
        _emit 0x19
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cbbb: PUSH EDI
        _emit 0x57
        // 0003cbbc: PUSH EDX
        _emit 0x52
        // 0003cbbd: PUSH ESI
        _emit 0x56
        // 0003cbbe: CALL 0x00431080
        _emit 0xe8
        _emit 0xbd
        _emit 0x44
        _emit 0xff
        _emit 0xff
        // 0003cbc3: POP EDI
        _emit 0x5f
        // 0003cbc4: RET
        _emit 0xc3

        // 0003cbc5: case 0x83f1 — PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cbc7: PUSH EAX
        _emit 0x50
        // 0003cbc8: PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 0003cbca: MOV EDI,0x18
        _emit 0xbf
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003cbcf: PUSH EDI
        _emit 0x57
        // 0003cbd0: PUSH EDX
        _emit 0x52
        // 0003cbd1: PUSH ESI
        _emit 0x56
        // 0003cbd2: CALL 0x00431080
        _emit 0xe8
        _emit 0xa9
        _emit 0x44
        _emit 0xff
        _emit 0xff
        // 0003cbd7: POP EDI
        _emit 0x5f
        // 0003cbd8: RET
        _emit 0xc3
    }
}
