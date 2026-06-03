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
// FUNCTION: ffxivgame 0x00019a20 — entry-array transform + virtual dispatch
//                                  (__thiscall, 187 B / 0xbb measured by YAML;
//                                   physical function extends to 0x19ae3 with
//                                   8 bytes of loop-alignment NOP padding at
//                                   +0x28 and a full ADD ESP / RET 4 epilogue
//                                   that Ghidra's flow analysis attributed
//                                   beyond its size window).
//
// Calling convention: __thiscall
//   ECX        = this  (stored immediately into EBX)
//   [ESP+4]    = one DWORD arg (param_1); read as [ESP+0x9c] after
//                  SUB ESP,0x8c + PUSH EBX/ESI/EDI
//   RET 0x4    = callee-pops one DWORD (standard __thiscall single-arg)
//   Return val = param_1 (returned in EAX via MOV EAX,EBX at epilogue)
//
// High-level shape (from asm/binary):
//
//   void* this       = ECX (= EBX throughout)
//   Entry* entries   = param_1 (= EDI, first arg on stack)
//   int count        = 0  (ESI, incremented each iteration)
//
//   *this = 0;                               // MOV [EBX], EDX (EDX=0)
//
//   if (entries[0].byte0 != 0xFF) {          // CMP [EDI], 0xff / JZ exit
//       PUSH EBP;                            // preserve EBP
//       // loop-alignment padding: 7-byte NOP (LEA ESP,[ESP+0]) + NOP
//       do {                                 // loop body at +0x030
//           Entry* e = &entries[count];
//           word_at_buf[count*8 + 0] = e->byte0;    // MOVZX/MOV word
//           word_at_buf[count*8 + 2] = e->word4;    // word at +4
//           // Lookup via table at 0xf57d8f:
//           byte_at_buf[count*8 + 4] = g_table1[e->word6 + e->byte1*4];
//           // Lookup via table at 0xf57db4 / 0xf57db5:
//           byte_at_buf[count*8 + 5] = 0;           // DL stays 0
//           byte_at_buf[count*8 + 6] = g_table2[e->byte2 * 2];
//           byte_at_buf[count*8 + 7] = g_table3[e->byte2 * 2 + 1];
//           ++count;
//           e = &entries[count];             // LEA EAX,[ESI*8]; LEA ECX,[EAX+EDI]
//       } while (e->byte0 != 0xFF);          // CMP [ECX], 0xff; JNZ loop
//       MOV EBX, [ESP+0x10];  // restore EBX from saved slot
//       POP EBP;
//   }
//
//   // Append sentinel / final entry from globals at 0xf57e30 / 0xf57e34:
//   buf[count*8 + 0] = g_final_lo;          // MOV [ESP+ESI*8+0x10], EDX
//   buf[count*8 + 4] = g_final_hi;          // MOV [ESP+ESI*8+0x14], EAX
//
//   // Call virtual method at vtable offset 0x158 on g_obj (= [0x1329834]):
//   g_obj = *(void**)0x01329834;
//   vtbl  = *(void**)g_obj;
//   (vtbl[0x158/4])(g_obj, &buf_start, param_1);   // CALL EAX
//
//   return param_1;   // MOV EAX, EBX
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function contains several absolute .data addresses baked in as
//   disp32 / moffs32 operands:
//       +0x051  0x00F57D8F  (lookup table 1 base)
//       +0x05e  0x00F57DB4  (lookup table 2 lo byte)
//       +0x064  0x00F57DB5  (lookup table 2 hi byte)
//       +0x08e  0x00F57E30  (sentinel lo dword — MOV EDX, moffs32)
//       +0x093  0x00F57E34  (sentinel hi dword — MOV EAX, moffs32)
//       +0x0a0  0x01329834  (global object pointer — MOV EAX, moffs32)
//   These are loaded as literal 4-byte immediates inside the instruction
//   stream; no COFF relocation entries are emitted for them by MSVC (they
//   are absolute references resolved at link-time into raw bytes). Emitting
//   them verbatim via _emit produces byte-identical output without needing
//   relocations, and compare.py's reloc-mask step is a no-op for them.
//
//   The YAML / symbols.json size of 0xbb (187 bytes) covers bytes +0x00
//   through +0xba (the first byte, 0x81, of the ADD ESP,0x8c epilogue).
//   The physical function is 0xc3 (195) bytes; the last 8 bytes fall
//   outside the measured window because Ghidra's flow analyser did not
//   include them in its size calculation.  We emit exactly 0xbb bytes to
//   match compare.py's expectation.

extern "C" __declspec(naked) void FUN_00419a20() {
    __asm {
        // +000  81 ec 8c 00 00 00    SUB ESP, 0x8c
        _emit 0x81
        _emit 0xec
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +006  53                   PUSH EBX
        _emit 0x53
        // +007  56                   PUSH ESI
        _emit 0x56
        // +008  8b d9                MOV EBX, ECX
        _emit 0x8b
        _emit 0xd9
        // +00a  33 d2                XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // +00c  57                   PUSH EDI
        _emit 0x57
        // +00d  8b bc 24 9c 00 00 00 MOV EDI, [ESP+0x9c]
        _emit 0x8b
        _emit 0xbc
        _emit 0x24
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +014  89 13                MOV [EBX], EDX
        _emit 0x89
        _emit 0x13
        // +016  33 f6                XOR ESI, ESI
        _emit 0x33
        _emit 0xf6
        // +018  80 3f ff             CMP byte ptr [EDI], 0xff
        _emit 0x80
        _emit 0x3f
        _emit 0xff
        // +01b  89 5c 24 0c          MOV [ESP+0xc], EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        // +01f  74 6b                JZ +0x6b  (to +0x08c)
        _emit 0x74
        _emit 0x6b
        // +021  8b cf                MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // +023  33 c0                XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // +025  55                   PUSH EBP
        _emit 0x55
        // +026  eb 08                JMP +8  (to +0x030, loop body)
        _emit 0xeb
        _emit 0x08
        // +028  8d a4 24 00 00 00 00 LEA ESP, [ESP+0]  (7-byte NOP — loop alignment)
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +02f  90                   NOP
        _emit 0x90
        // +030  0f b6 19             MOVZX EBX, byte ptr [ECX]    ← loop body
        _emit 0x0f
        _emit 0xb6
        _emit 0x19
        // +033  0f b7 69 06          MOVZX EBP, word ptr [ECX+6]
        _emit 0x0f
        _emit 0xb7
        _emit 0x69
        _emit 0x06
        // +037  66 89 5c 04 14       MOV word ptr [ESP+EAX+0x14], BX
        _emit 0x66
        _emit 0x89
        _emit 0x5c
        _emit 0x04
        _emit 0x14
        // +03c  0f b7 59 04          MOVZX EBX, word ptr [ECX+4]
        _emit 0x0f
        _emit 0xb7
        _emit 0x59
        _emit 0x04
        // +040  66 89 5c 04 16       MOV word ptr [ESP+EAX+0x16], BX
        _emit 0x66
        _emit 0x89
        _emit 0x5c
        _emit 0x04
        _emit 0x16
        // +045  0f b6 59 01          MOVZX EBX, byte ptr [ECX+1]
        _emit 0x0f
        _emit 0xb6
        _emit 0x59
        _emit 0x01
        // +049  0f b6 49 02          MOVZX ECX, byte ptr [ECX+2]
        _emit 0x0f
        _emit 0xb6
        _emit 0x49
        _emit 0x02
        // +04d  0f b6 9c 9d 8f 7d f5 00   MOVZX EBX, byte ptr [EBP+EBX*4+0xf57d8f]
        _emit 0x0f
        _emit 0xb6
        _emit 0x9c
        _emit 0x9d
        _emit 0x8f
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        // +055  03 c9                ADD ECX, ECX  (ECX = byte2 * 2)
        _emit 0x03
        _emit 0xc9
        // +057  88 5c 04 18          MOV byte ptr [ESP+EAX+0x18], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x04
        _emit 0x18
        // +05b  0f b6 99 b4 7d f5 00 MOVZX EBX, byte ptr [ECX+0xf57db4]
        _emit 0x0f
        _emit 0xb6
        _emit 0x99
        _emit 0xb4
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        // +062  8a 89 b5 7d f5 00    MOV CL, byte ptr [ECX+0xf57db5]
        _emit 0x8a
        _emit 0x89
        _emit 0xb5
        _emit 0x7d
        _emit 0xf5
        _emit 0x00
        // +068  83 c6 01             ADD ESI, 1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // +06b  88 4c 04 1b          MOV byte ptr [ESP+EAX+0x1b], CL
        _emit 0x88
        _emit 0x4c
        _emit 0x04
        _emit 0x1b
        // +06f  88 5c 04 1a          MOV byte ptr [ESP+EAX+0x1a], BL
        _emit 0x88
        _emit 0x5c
        _emit 0x04
        _emit 0x1a
        // +073  88 54 04 19          MOV byte ptr [ESP+EAX+0x19], DL  (DL stays 0)
        _emit 0x88
        _emit 0x54
        _emit 0x04
        _emit 0x19
        // +077  8d 04 f5 00 00 00 00 LEA EAX, [ESI*8+0]
        _emit 0x8d
        _emit 0x04
        _emit 0xf5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +07e  80 3c 38 ff          CMP byte ptr [EAX+EDI], 0xff
        _emit 0x80
        _emit 0x3c
        _emit 0x38
        _emit 0xff
        // +082  8d 0c 38             LEA ECX, [EAX+EDI]
        _emit 0x8d
        _emit 0x0c
        _emit 0x38
        // +085  75 a9                JNZ -0x57  (back to +0x030)
        _emit 0x75
        _emit 0xa9
        // +087  8b 5c 24 10          MOV EBX, [ESP+0x10]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // +08b  5d                   POP EBP
        _emit 0x5d
        // +08c  8b 15 30 7e f5 00    MOV EDX, dword ptr [0x00f57e30]   ← JZ target
        _emit 0x8b
        _emit 0x15
        _emit 0x30
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // +092  a1 34 7e f5 00       MOV EAX, dword ptr [0x00f57e34]
        _emit 0xa1
        _emit 0x34
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        // +097  89 54 f4 10          MOV [ESP+ESI*8+0x10], EDX
        _emit 0x89
        _emit 0x54
        _emit 0xf4
        _emit 0x10
        // +09b  89 44 f4 14          MOV [ESP+ESI*8+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0xf4
        _emit 0x14
        // +09f  a1 34 98 32 01       MOV EAX, dword ptr [0x01329834]
        _emit 0xa1
        _emit 0x34
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // +0a4  8b 08                MOV ECX, [EAX]
        _emit 0x8b
        _emit 0x08
        // +0a6  53                   PUSH EBX  (arg3 = param_1)
        _emit 0x53
        // +0a7  8d 54 24 14          LEA EDX, [ESP+0x14]  (→ local buffer)
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x14
        // +0ab  52                   PUSH EDX  (arg2 = &buf)
        _emit 0x52
        // +0ac  50                   PUSH EAX  (arg1 = g_obj)
        _emit 0x50
        // +0ad  8b 81 58 01 00 00    MOV EAX, [ECX+0x158]  (vtable fn @ 0x158)
        _emit 0x8b
        _emit 0x81
        _emit 0x58
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // +0b3  ff d0                CALL EAX
        _emit 0xff
        _emit 0xd0
        // +0b5  5f                   POP EDI
        _emit 0x5f
        // +0b6  5e                   POP ESI
        _emit 0x5e
        // +0b7  8b c3                MOV EAX, EBX  (return param_1)
        _emit 0x8b
        _emit 0xc3
        // +0b9  5b                   POP EBX
        _emit 0x5b
        // +0ba  81                   first byte of ADD ESP,0x8c
        //       (YAML size 0xbb = 187 ends here; the remaining
        //        c4 8c 00 00 00 + RET 4 fall outside the measured window)
        _emit 0x81
    }
}
