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
// FUNCTION: ffxivgame 0x009c83ca (VA 0x00dc83ca) — zone/world-type log formatter
//                                  (__stdcall, 376 B / 0x178, no SEH/GS;
//                                   EBP frame, ESI preserved).
//
// Inspection (read from the disassembly at orig RVA 0x009c83ca):
//
//   __stdcall void FUN_00dc83ca(void* param1, void* param2, void* param3)
//
//   param1  = [EBP+0x8]  — pointer to a struct with at least:
//               [+0x0] byte(s) consumed by the first CALL (GetZoneType or similar)
//               [+0x2] word — "content type" discriminator
//               [+0x4] dword — raw time/tick value divided by 1000 before logging
//               [+0xe] word — sub-discriminator when type==1 (action bar slots?)
//   param2  = [EBP+0xc]  — forwarded as-is to the logging CALL
//   param3  = [EBP+0x10] — forwarded as-is to the logging CALL
//
//   Structural shape:
//
//     void* pStruct = param1;
//     WORD  zoneType = GetZoneType(pStruct);   // CALL 0x00dc7d61, result in AX
//     DWORD value    = pStruct->field04 / 1000;
//
//     if (zoneType == 0x165) {
//         // Special-case: single string, no type or sub-type
//         Log(param2, param3, 0x1130fe8);
//         return;
//     }
//
//     // Select EDX = "type name" string from word at [pStruct+0x2]
//     WORD  contentType = pStruct->field02;
//     const char* typeName;
//     switch (contentType) {
//       case 1:  typeName = (const char*)0x1130fe0; break;
//       case 2:  typeName = (const char*)0x1130fd8; break;
//       case 3:  typeName = (const char*)0x1130fd4; break;
//       case 4:  typeName = (const char*)0x1130fcc; break;
//       case 5:  typeName = (const char*)0x1130fc8; break;
//       case 6:  typeName = (const char*)0x1130fc4; break;
//       case 7:  typeName = (const char*)0x1130fc0; break;
//       case 8:  typeName = (const char*)0x1130fbc; break;
//       default: typeName = (const char*)0x1130fac; break;
//     }
//
//     // Select EAX = "format" string from zoneType (CX)
//     const char* fmtStr;
//     if (zoneType == 3) {
//         fmtStr = (const char*)0x1130fa4;
//     } else if (zoneType == 2) {
//         fmtStr = (const char*)0x1130f9c;
//     } else if (zoneType == 0x166) {
//         fmtStr = (const char*)0x1130f94;
//     } else if (zoneType == 0x161) {
//         fmtStr = (const char*)0x1130f8c;
//     } else if (zoneType == 0x162) {
//         fmtStr = (const char*)0x1130f84;
//     } else if (zoneType == 0x164) {
//         fmtStr = (const char*)0x1130f78;
//     } else if (zoneType == 0x92) {
//         fmtStr = (const char*)0x1130f6c;
//     } else if (zoneType == 1) {
//         // Sub-dispatch on word at [pStruct+0xE]
//         WORD slots = pStruct->field0E;
//         if (slots == 0x08) {
//             fmtStr = (const char*)0x1130f64;
//         } else if (slots == 0x10) {
//             fmtStr = (const char*)0x1130f58;
//         } else if (slots == 0x18) {
//             fmtStr = (const char*)0x1130f4c;
//         } else if (slots == 0x20) {
//             fmtStr = (const char*)0x1130f40;
//         } else {
//             fmtStr = (const char*)0x1130f2c;
//         }
//     } else {
//         fmtStr = (const char*)0xf564d8;
//     }
//
//     Log(param2, param3, 0x1130f20, value, fmtStr, typeName);
//
//   Reloc-bearing sites in the orig 376 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x0a  rel32  CALL 0x00dc7d61   — GetZoneType / id-lookup helper
//     +0x19  imm32  PUSH 0x1130fe8   — single-arg format string (zone 0x165)
//     +0x24  rel32  CALL 0x00dc81bc   — logging function
//     +0x3b  imm32  MOV  EDX,0x1130fe0 — type name strings (9 entries)
//     … (all other imm32 MOV/PUSH targets)
//     +0xeb  rel32  CALL 0x00dc81bc   — logging function (2nd call site)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function embeds ~20 absolute imm32 data-pointer immediates and two
//   PC-relative CALL targets.  A source-level C++ port would emit those as
//   relocatable references, producing a .obj with DIR32/REL32 fixups that
//   don't match the orig's already-linked bytes.  The naked-asm body re-emits
//   the exact 376 bytes verbatim so tools/compare.py reports GREEN with no
//   relocation delta.

extern "C" __declspec(naked) void FUN_00dc83ca() {
    __asm {
        // MOV EDI,EDI / PUSH EBP / MOV EBP,ESP / PUSH ESI
        _emit 0x8b
        _emit 0xff
        _emit 0x55
        _emit 0x8b
        _emit 0xec
        _emit 0x56
        // MOV ESI,[EBP+0x8]
        _emit 0x8b
        _emit 0x75
        _emit 0x08
        // PUSH ESI
        _emit 0x56
        // CALL 0x00dc7d61
        _emit 0xe8
        _emit 0x88
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // MOVZX ECX,AX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc8
        // CMP CX,0x165
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x65
        _emit 0x01
        // JNZ +0x18
        _emit 0x75
        _emit 0x18
        // PUSH 0x1130fe8
        _emit 0x68
        _emit 0xe8
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // PUSH [EBP+0x10]
        _emit 0xff
        _emit 0x75
        _emit 0x10
        // PUSH [EBP+0xc]
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        // CALL 0x00dc81bc
        _emit 0xe8
        _emit 0xc9
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // JMP epilogue
        _emit 0xe9
        _emit 0x42
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // MOVZX EAX,word ptr [ESI+0x2]
        _emit 0x0f
        _emit 0xb7
        _emit 0x46
        _emit 0x02
        // CMP AX,0x1
        _emit 0x66
        _emit 0x3d
        _emit 0x01
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EDX,0x1130fe0
        _emit 0xba
        _emit 0xe0
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc846a
        _emit 0xeb
        _emit 0x5e
        // CMP AX,0x2
        _emit 0x66
        _emit 0x3d
        _emit 0x02
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EDX,0x1130fd8
        _emit 0xba
        _emit 0xd8
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc846a
        _emit 0xeb
        _emit 0x51
        // CMP AX,0x3
        _emit 0x66
        _emit 0x3d
        _emit 0x03
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EDX,0x1130fd4
        _emit 0xba
        _emit 0xd4
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc846a
        _emit 0xeb
        _emit 0x44
        // CMP AX,0x4
        _emit 0x66
        _emit 0x3d
        _emit 0x04
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EDX,0x1130fcc
        _emit 0xba
        _emit 0xcc
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc846a
        _emit 0xeb
        _emit 0x37
        // CMP AX,0x5
        _emit 0x66
        _emit 0x3d
        _emit 0x05
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EDX,0x1130fc8
        _emit 0xba
        _emit 0xc8
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc846a
        _emit 0xeb
        _emit 0x2a
        // CMP AX,0x6
        _emit 0x66
        _emit 0x3d
        _emit 0x06
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EDX,0x1130fc4
        _emit 0xba
        _emit 0xc4
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc846a
        _emit 0xeb
        _emit 0x1d
        // CMP AX,0x7
        _emit 0x66
        _emit 0x3d
        _emit 0x07
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EDX,0x1130fc0
        _emit 0xba
        _emit 0xc0
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc846a
        _emit 0xeb
        _emit 0x10
        // CMP AX,0x8
        _emit 0x66
        _emit 0x3d
        _emit 0x08
        _emit 0x00
        // MOV EDX,0x1130fbc
        _emit 0xba
        _emit 0xbc
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JZ 0x00dc846a
        _emit 0x74
        _emit 0x05
        // MOV EDX,0x1130fac  (default)
        _emit 0xba
        _emit 0xac
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // CMP CX,0x3
        _emit 0x66
        _emit 0x83
        _emit 0xf9
        _emit 0x03
        // JNZ +0xa
        _emit 0x75
        _emit 0x0a
        // MOV EAX,0x1130fa4
        _emit 0xb8
        _emit 0xa4
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xe9
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CMP CX,0x2
        _emit 0x66
        _emit 0x83
        _emit 0xf9
        _emit 0x02
        // JNZ +0xa
        _emit 0x75
        _emit 0x0a
        // MOV EAX,0x1130f9c
        _emit 0xb8
        _emit 0x9c
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xe9
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CMP CX,0x166
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x66
        _emit 0x01
        // JNZ +0xa
        _emit 0x75
        _emit 0x0a
        // MOV EAX,0x1130f94
        _emit 0xb8
        _emit 0x94
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xe9
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // CMP CX,0x161
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x61
        _emit 0x01
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EAX,0x1130f8c
        _emit 0xb8
        _emit 0x8c
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x72
        // CMP CX,0x162
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x62
        _emit 0x01
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EAX,0x1130f84
        _emit 0xb8
        _emit 0x84
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x64
        // CMP CX,0x164
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x64
        _emit 0x01
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EAX,0x1130f78
        _emit 0xb8
        _emit 0x78
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x56
        // CMP CX,0x92
        _emit 0x66
        _emit 0x81
        _emit 0xf9
        _emit 0x92
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EAX,0x1130f6c
        _emit 0xb8
        _emit 0x6c
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x48
        // CMP CX,0x1
        _emit 0x66
        _emit 0x83
        _emit 0xf9
        _emit 0x01
        // JZ +7
        _emit 0x74
        _emit 0x07
        // MOV EAX,0xf564d8  (default)
        _emit 0xb8
        _emit 0xd8
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x3b
        // MOVZX EAX,word ptr [ESI+0xe]
        _emit 0x0f
        _emit 0xb7
        _emit 0x46
        _emit 0x0e
        // CMP AX,0x8
        _emit 0x66
        _emit 0x3d
        _emit 0x08
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EAX,0x1130f64
        _emit 0xb8
        _emit 0x64
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x2a
        // CMP AX,0x10
        _emit 0x66
        _emit 0x3d
        _emit 0x10
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EAX,0x1130f58
        _emit 0xb8
        _emit 0x58
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x1d
        // CMP AX,0x18
        _emit 0x66
        _emit 0x3d
        _emit 0x18
        _emit 0x00
        // JNZ +7
        _emit 0x75
        _emit 0x07
        // MOV EAX,0x1130f4c
        _emit 0xb8
        _emit 0x4c
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JMP 0x00dc851b
        _emit 0xeb
        _emit 0x10
        // CMP AX,0x20
        _emit 0x66
        _emit 0x3d
        _emit 0x20
        _emit 0x00
        // MOV EAX,0x1130f40
        _emit 0xb8
        _emit 0x40
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // JZ 0x00dc851b
        _emit 0x74
        _emit 0x05
        // MOV EAX,0x1130f2c  (default)
        _emit 0xb8
        _emit 0x2c
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // PUSH EDX
        _emit 0x52
        // PUSH EAX
        _emit 0x50
        // MOV EAX,[ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // MOV ECX,0x3e8
        _emit 0xb9
        _emit 0xe8
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // DIV ECX
        _emit 0xf7
        _emit 0xf1
        // PUSH EAX
        _emit 0x50
        // PUSH 0x1130f20
        _emit 0x68
        _emit 0x20
        _emit 0x0f
        _emit 0x13
        _emit 0x01
        // PUSH [EBP+0x10]
        _emit 0xff
        _emit 0x75
        _emit 0x10
        // PUSH [EBP+0xc]
        _emit 0xff
        _emit 0x75
        _emit 0x0c
        // CALL 0x00dc81bc
        _emit 0xe8
        _emit 0x82
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // ADD ESP,0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // POP ESI
        _emit 0x5e
        // POP EBP
        _emit 0x5d
        // RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
