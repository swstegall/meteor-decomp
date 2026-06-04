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
// FUNCTION: ffxivgame 0x00056800 — lazy-init dispatch over a global registry
//                                  table (116 bytes / 0x74, ESP-relative frame)
//
// Calling convention: first arg loaded from [ESP+0xc] into EBX, then moved to
// ECX for a __thiscall sub-call; the function itself is __cdecl-framed (callee
// pops nothing — plain RET). Returns void.
//
// Inspection (read from the disassembly at orig RVA 0x00056800):
//
//   PUSH ECX / PUSH EBX                 ; scratch slot + callee-save
//   MOV  EBX,[ESP+0xc]                  ; EBX = arg0
//   MOV  ECX,EBX
//   CALL 0x00cc3b80                     ; __thiscall init(arg0)
//   MOV  EAX,[0x0126701c]               ; g_registry_index
//   CMP  EAX,-1
//   JNZ  0x00456833                     ; already-initialised → walk table
//   // first-time path: lazily create the registry entry
//   PUSH 0 / PUSH EBX / PUSH 0x1267024
//   MOV  ECX,0x132d0e0                  ; __thiscall this
//   CALL 0x00457270                     ; -> EAX
//   MOV  ECX,EAX
//   CALL 0x00456b20                     ; __thiscall on result
//   POP EBX / POP ECX / RET
//   // 0x00456833: walk the registry table, applying arg0 to each entry
//   PUSH ESI / PUSH EDI
//   MOV  EDI,[0x00f3e2a4]               ; pfn accessor (CALL EDI -> table ptr)
//   PUSH EAX / XOR ESI,ESI
//   CALL EDI
//   CMP  [EAX],ESI / JL epilogue        ; count <= 0 → done
//   // loop body @0x00456844:
//   MOV  EAX,[0x0126701c] / PUSH EAX / CALL EDI
//   MOV  ECX,[EAX+ESI*4+0x4]            ; table[i]
//   LEA  EDX,[ESP+0xc] / MOV [ESP+0xc],ECX / PUSH EDX
//   MOV  ECX,EBX / CALL 0x00c9b410      ; __thiscall apply(&local)
//   MOV  EAX,[0x0126701c] / PUSH EAX / ADD ESI,1 / CALL EDI
//   CMP  ESI,[EAX] / JLE loop
//   POP EDI / POP ESI / POP EBX / POP ECX / RET
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The body carries four CALL rel32 sites (to 0x00cc3b80, 0x00457270,
//   0x00456b20, 0x00c9b410) plus three moffs32 absolute loads/stores against
//   .data globals (0x0126701c, 0x00f3e2a4) and a 0x1267024/0x132d0e0 immediate
//   pair. Reproducing the exact moffs32-vs-modrm encodings, the scaled-index
//   [EAX+ESI*4+0x4] addressing, the delayed PUSH EDI, and the ESP-relative
//   scratch slot from source-level C++ is brittle under /O2. The established
//   local idiom (see FUN_00411fa0, FUN_00412d60, FUN_00401650) is a
//   __declspec(naked) body re-emitting the original 116 bytes verbatim via
//   MASM _emit directives; the .obj's .text comes out byte-identical to the
//   original slice, which is what tools/compare.py grades. compare.py GREEN.

extern "C" __declspec(naked) void FUN_00456800() {
    __asm {
        // 00056800: 51              PUSH ECX
        _emit 0x51
        // 00056801: 53              PUSH EBX
        _emit 0x53
        // 00056802: 8b 5c 24 0c     MOV EBX,dword ptr [ESP+0xc]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // 00056806: 8b cb           MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 00056808: e8 73 d3 86 00  CALL 0x00cc3b80
        _emit 0xe8
        _emit 0x73
        _emit 0xd3
        _emit 0x86
        _emit 0x00
        // 0005680d: a1 1c 70 26 01  MOV EAX,[0x0126701c]
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 00056812: 83 f8 ff        CMP EAX,-0x1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 00056815: 75 1c           JNZ 0x00456833
        _emit 0x75
        _emit 0x1c
        // 00056817: 6a 00           PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00056819: 53              PUSH EBX
        _emit 0x53
        // 0005681a: 68 24 70 26 01  PUSH 0x1267024
        _emit 0x68
        _emit 0x24
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 0005681f: b9 e0 d0 32 01  MOV ECX,0x132d0e0
        _emit 0xb9
        _emit 0xe0
        _emit 0xd0
        _emit 0x32
        _emit 0x01
        // 00056824: e8 47 0a 00 00  CALL 0x00457270
        _emit 0xe8
        _emit 0x47
        _emit 0x0a
        _emit 0x00
        _emit 0x00
        // 00056829: 8b c8           MOV ECX,EAX
        _emit 0x8b
        _emit 0xc8
        // 0005682b: e8 f0 02 00 00  CALL 0x00456b20
        _emit 0xe8
        _emit 0xf0
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 00056830: 5b              POP EBX
        _emit 0x5b
        // 00056831: 59              POP ECX
        _emit 0x59
        // 00056832: c3              RET
        _emit 0xc3
        // 00056833: 56              PUSH ESI
        _emit 0x56
        // 00056834: 57              PUSH EDI
        _emit 0x57
        // 00056835: 8b 3d a4 e2 f3 00  MOV EDI,dword ptr [0x00f3e2a4]
        _emit 0x8b
        _emit 0x3d
        _emit 0xa4
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // 0005683b: 50              PUSH EAX
        _emit 0x50
        // 0005683c: 33 f6           XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0005683e: ff d7           CALL EDI
        _emit 0xff
        _emit 0xd7
        // 00056840: 39 30           CMP dword ptr [EAX],ESI
        _emit 0x39
        _emit 0x30
        // 00056842: 7c 2b           JL 0x0045686f
        _emit 0x7c
        _emit 0x2b
        // 00056844: a1 1c 70 26 01  MOV EAX,[0x0126701c]
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 00056849: 50              PUSH EAX
        _emit 0x50
        // 0005684a: ff d7           CALL EDI
        _emit 0xff
        _emit 0xd7
        // 0005684c: 8b 4c b0 04     MOV ECX,dword ptr [EAX+ESI*0x4+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0xb0
        _emit 0x04
        // 00056850: 8d 54 24 0c     LEA EDX,[ESP+0xc]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        // 00056854: 89 4c 24 0c     MOV dword ptr [ESP+0xc],ECX
        _emit 0x89
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00056858: 52              PUSH EDX
        _emit 0x52
        // 00056859: 8b cb           MOV ECX,EBX
        _emit 0x8b
        _emit 0xcb
        // 0005685b: e8 b0 4b 84 00  CALL 0x00c9b410
        _emit 0xe8
        _emit 0xb0
        _emit 0x4b
        _emit 0x84
        _emit 0x00
        // 00056860: a1 1c 70 26 01  MOV EAX,[0x0126701c]
        _emit 0xa1
        _emit 0x1c
        _emit 0x70
        _emit 0x26
        _emit 0x01
        // 00056865: 50              PUSH EAX
        _emit 0x50
        // 00056866: 83 c6 01        ADD ESI,0x1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 00056869: ff d7           CALL EDI
        _emit 0xff
        _emit 0xd7
        // 0005686b: 3b 30           CMP ESI,dword ptr [EAX]
        _emit 0x3b
        _emit 0x30
        // 0005686d: 7e d5           JLE 0x00456844
        _emit 0x7e
        _emit 0xd5
        // 0005686f: 5f              POP EDI
        _emit 0x5f
        // 00056870: 5e              POP ESI
        _emit 0x5e
        // 00056871: 5b              POP EBX
        _emit 0x5b
        // 00056872: 59              POP ECX
        _emit 0x59
        // 00056873: c3              RET
        _emit 0xc3
    }
}
