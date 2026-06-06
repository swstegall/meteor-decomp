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
// FUNCTION: ffxivgame 0x00061da0 — binary-data hex-formatter dispatcher (190 B / 0xbe)
//
// Formats a tagged binary buffer into a debug/log output stream. The
// function dispatches on a "type" field from an input descriptor struct
// and prints the raw bytes in different hex layouts.
//
// Calling convention: __usercall — one stack arg + one register arg
//   [ESP+0x04]  : pointer to descriptor struct { int type; ...; byte* data; }
//                   struct->field_0x00 = type/size code (int)
//                   struct->field_0x08 = data pointer (byte*)
//   EBX          : output handle / context passed through to all print calls
//   EAX          : return value (1 on success, or type-derived value for type==8)
//
// Behaviour (from asm/ffxivgame/00061da0_FUN_00461da0.s):
//
//   pDesc = [ESP+4]
//   data  = pDesc->field_0x08   (in ESI after prolog)
//   type  = pDesc->field_0x00   (in EDI after prolog)
//
//   // First: emit a header/prefix via 0x00469b80(EBX, 0xf69528)
//   0x00469b80(EBX, "...");   // 2-arg call (handle, format/string)
//
//   if (type == 8) {
//       // Print 8 individual bytes, highest-index first loaded to
//       // avoid partial reg stalls, then push all 8 + format + handle:
//       0x00467e90(EBX, 0xf69510,
//           data[0], data[1], data[2], data[3],
//           data[4], data[5], data[6], data[7]);  // 10-arg call
//       return type - 7;   // = 1 (LEA EAX, [EDI-7] with EDI=8)
//   }
//
//   if (type == 32) {
//       // Print 16 pairs of bytes in a loop (32 bytes total):
//       for (int i = 0; i < 16; i++) {
//           EDX = (data[0] << 8) | data[1];   // DH=data[0], DL=data[1]
//           0x00467e90(EBX, 0xf6950c, EDX);   // print pair
//           data += 2;
//           if (i != 15) {
//               if (i == 7)
//                   0x00469b80(EBX, 0xf69508);   // mid-sequence separator
//               else
//                   0x00469b80(EBX, 0xf69504);   // normal separator
//           }
//       }
//       return 1;
//   }
//
//   // else (unknown type):
//   0x00467e90(EBX, 0xf694ec);   // print fallback string
//   return 1;
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function uses a non-standard __usercall convention (EBX is a
//   live register argument from the caller, never loaded or saved by this
//   function). MSVC 2005 has no source-level encoding for a non-ECX
//   register calling convention. Additionally, the 5 CALL rel32 sites and
//   6 PUSH imm32 (absolute .rdata addresses) are identity-preserving in
//   the naked _emit approach (compare.py masks reloc bytes). This is the
//   same strategy used by FUN_00406680 (EDI register arg) and others.
//
// Reloc-bearing sites (offsets from function start):
//   +0x0c   PUSH imm32   0xf69528  (.rdata — header format string)
//   +0x12   CALL rel32   0x00469b80 (print-string helper)
//   +0x46   PUSH imm32   0xf69510  (.rdata — 8-byte format string)
//   +0x4c   CALL rel32   0x00467e90 (formatted-output helper)
//   +0x69   PUSH imm32   0xf6950c  (.rdata — pair format string)
//   +0x6f   CALL rel32   0x00467e90
//   +0x7f   PUSH imm32   0xf69508  (.rdata — mid separator string)
//   +0x8b   PUSH imm32   0xf69504  (.rdata — normal separator string)
//   +0x91   CALL rel32   0x00469b80
//   +0xa9   PUSH imm32   0xf694ec  (.rdata — fallback string)
//   +0xaf   CALL rel32   0x00467e90

extern "C" __declspec(naked) void FUN_00461da0() {
    __asm {
        // 00061da0: MOV EAX, [ESP+4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 00061da4: PUSH ESI
        _emit 0x56
        // 00061da5: MOV ESI, [EAX+8]
        _emit 0x8b
        _emit 0x70
        _emit 0x08
        // 00061da8: PUSH EDI
        _emit 0x57
        // 00061da9: MOV EDI, [EAX]
        _emit 0x8b
        _emit 0x38
        // 00061dab: PUSH 0xf69528
        _emit 0x68
        _emit 0x28
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        // 00061db0: PUSH EBX
        _emit 0x53
        // 00061db1: CALL 0x00469b80
        _emit 0xe8
        _emit 0xca
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        // 00061db6: ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00061db9: CMP EDI, 8
        _emit 0x83
        _emit 0xff
        _emit 0x08
        // 00061dbc: JNZ +0x3b
        _emit 0x75
        _emit 0x3b
        // 00061dbe: MOVZX EAX, byte [ESI+7]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x07
        // 00061dc2: MOVZX ECX, byte [ESI+6]
        _emit 0x0f
        _emit 0xb6
        _emit 0x4e
        _emit 0x06
        // 00061dc6: MOVZX EDX, byte [ESI+5]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x05
        // 00061dca: PUSH EAX  (byte 7)
        _emit 0x50
        // 00061dcb: MOVZX EAX, byte [ESI+4]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x04
        // 00061dcf: PUSH ECX  (byte 6)
        _emit 0x51
        // 00061dd0: MOVZX ECX, byte [ESI+3]
        _emit 0x0f
        _emit 0xb6
        _emit 0x4e
        _emit 0x03
        // 00061dd4: PUSH EDX  (byte 5)
        _emit 0x52
        // 00061dd5: MOVZX EDX, byte [ESI+2]
        _emit 0x0f
        _emit 0xb6
        _emit 0x56
        _emit 0x02
        // 00061dd9: PUSH EAX  (byte 4)
        _emit 0x50
        // 00061dda: MOVZX EAX, byte [ESI+1]
        _emit 0x0f
        _emit 0xb6
        _emit 0x46
        _emit 0x01
        // 00061dde: PUSH ECX  (byte 3)
        _emit 0x51
        // 00061ddf: MOVZX ECX, byte [ESI]
        _emit 0x0f
        _emit 0xb6
        _emit 0x0e
        // 00061de2: PUSH EDX  (byte 2)
        _emit 0x52
        // 00061de3: PUSH EAX  (byte 1)
        _emit 0x50
        // 00061de4: PUSH ECX  (byte 0)
        _emit 0x51
        // 00061de5: PUSH 0xf69510
        _emit 0x68
        _emit 0x10
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        // 00061dea: PUSH EBX
        _emit 0x53
        // 00061deb: CALL 0x00467e90
        _emit 0xe8
        _emit 0xa0
        _emit 0x60
        _emit 0x00
        _emit 0x00
        // 00061df0: ADD ESP, 0x28
        _emit 0x83
        _emit 0xc4
        _emit 0x28
        // 00061df3: LEA EAX, [EDI-7]
        _emit 0x8d
        _emit 0x47
        _emit 0xf9
        // 00061df6: POP EDI
        _emit 0x5f
        // 00061df7: POP ESI
        _emit 0x5e
        // 00061df8: RET
        _emit 0xc3
        // 00061df9: CMP EDI, 0x20
        _emit 0x83
        _emit 0xff
        _emit 0x20
        // 00061dfc: JNZ +0x4a
        _emit 0x75
        _emit 0x4a
        // 00061dfe: XOR EDI, EDI  (loop counter = 0)
        _emit 0x33
        _emit 0xff
        // 00061e00: XOR EDX, EDX  (loop body start)
        _emit 0x33
        _emit 0xd2
        // 00061e02: MOV DH, [ESI]
        _emit 0x8a
        _emit 0x36
        // 00061e04: MOV DL, [ESI+1]
        _emit 0x8a
        _emit 0x56
        _emit 0x01
        // 00061e07: PUSH EDX
        _emit 0x52
        // 00061e08: PUSH 0xf6950c
        _emit 0x68
        _emit 0x0c
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        // 00061e0d: PUSH EBX
        _emit 0x53
        // 00061e0e: CALL 0x00467e90
        _emit 0xe8
        _emit 0x7d
        _emit 0x60
        _emit 0x00
        _emit 0x00
        // 00061e13: ADD ESP, 0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00061e16: ADD ESI, 2
        _emit 0x83
        _emit 0xc6
        _emit 0x02
        // 00061e19: CMP EDI, 7
        _emit 0x83
        _emit 0xff
        _emit 0x07
        // 00061e1c: JNZ +7
        _emit 0x75
        _emit 0x07
        // 00061e1e: PUSH 0xf69508
        _emit 0x68
        _emit 0x08
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        // 00061e23: JMP +0xa
        _emit 0xeb
        _emit 0x0a
        // 00061e25: CMP EDI, 0xf
        _emit 0x83
        _emit 0xff
        _emit 0x0f
        // 00061e28: JZ +0xe
        _emit 0x74
        _emit 0x0e
        // 00061e2a: PUSH 0xf69504
        _emit 0x68
        _emit 0x04
        _emit 0x95
        _emit 0xf6
        _emit 0x00
        // 00061e2f: PUSH EBX
        _emit 0x53
        // 00061e30: CALL 0x00469b80
        _emit 0xe8
        _emit 0x4b
        _emit 0x7d
        _emit 0x00
        _emit 0x00
        // 00061e35: ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00061e38: ADD EDI, 1
        _emit 0x83
        _emit 0xc7
        _emit 0x01
        // 00061e3b: CMP EDI, 0x10
        _emit 0x83
        _emit 0xff
        _emit 0x10
        // 00061e3e: JL -0x40
        _emit 0x7c
        _emit 0xc0
        // 00061e40: POP EDI
        _emit 0x5f
        // 00061e41: MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00061e46: POP ESI
        _emit 0x5e
        // 00061e47: RET
        _emit 0xc3
        // 00061e48: PUSH 0xf694ec
        _emit 0x68
        _emit 0xec
        _emit 0x94
        _emit 0xf6
        _emit 0x00
        // 00061e4d: PUSH EBX
        _emit 0x53
        // 00061e4e: CALL 0x00467e90
        _emit 0xe8
        _emit 0x3d
        _emit 0x60
        _emit 0x00
        _emit 0x00
        // 00061e53: ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00061e56: POP EDI
        _emit 0x5f
        // 00061e57: MOV EAX, 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00061e5c: POP ESI
        _emit 0x5e
        // 00061e5d: RET
        _emit 0xc3
    }
}
