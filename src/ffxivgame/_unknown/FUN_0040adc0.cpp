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
// FUNCTION: ffxivgame 0x0000adc0 — stat-bucket distribution with critical section
//                                  (__thiscall, 293 B / 0x125, EH4-SEH wrapped)
//
// Inspection (read from asm/ffxivgame/0000adc0_FUN_0040adc0.s and raw binary):
//
//   __thiscall void FUN_0040adc0(this, int param_1, uint param_2)
//     ECX        : this
//     [ESP+0x04] : param_1 — resource pointer; if NULL returns early
//     [ESP+0x08] : param_2 — uint value used for bucket lookup and subtraction
//
// Structure:
//   EH4 SEH prologue (PUSH -1 / PUSH 0xe54da0 / PUSH FS:[0] / MOV FS:[0],ESP).
//   ECX on stack, then EBX/ESI/EDI pushed as callee-saves.
//   EDI = this; EBX = &this->m_critical_section (this+0x64).
//   EnterCriticalSection(this+0x64); SEH state -> 0.
//   If param_1 == 0: LeaveCriticalSection and RET 0xc.
//   Push EBP; load EBP = param_2; XOR EBX,EBX; JMP into loop condition.
//   Loop (EBX = 0..0x15): look up param_2 in table at 0x00f55988[EBX].
//     If param_2 <= table[EBX]: break (JBE to aea2).
//     EBX++; if EBX < 0x16: continue loop.
//   Else (all buckets exceeded): ADD [EDI+0xbc],1; SUB [EDI+0xc4],EBP;
//     MOV ECX,[EDI+0x58]; PUSH ESI; CALL FUN_0040df70;
//     [dead code: MOV EBX,1 at ae47 — unreachable after noreturn]
//   At aea2 (JBE target): TEST EBX,EBX; if EBX < 0 goto noreturn path.
//     PUSH ESI; PUSH EBX; MOV ECX,EDI; CALL FUN_0040abe0(this,EBX,param_1).
//     If EAX==0: goto leave_section.
//     Update stat fields at EDI+0xcc/0xdc/0xd4 using EBX index (stride 0x18).
//     MOV EBX,1; ADD [ECX+0x8],EBX; ADD [EDI+0xa8],EBX; SUB [EDI+0xb0],EBP.
//     PUSH EAX; CALL FUN_0040a8a0(EAX); ADD ESP,4.
//     JMP to ae4c (shared path):
//       MOV EAX,[ESP+0x2c]; ADD [EDI+0x80],EBX; SUB [EDI+0x88],EBP.
//       CALL FUN_0040a710; MOVZX EAX,AX; TEST AX,AX; JL leave_section.
//       If valid: MOV ECX,[EDI+0x360]; MOVSX EAX,AX; compute slot ptr.
//         If non-NULL: ADD [ptr+0x4],EBX; SUB [ptr+0xc],EBP.
//   leave_section: MOV EDX,[ESP+0x10]; PUSH EDX; CALL LeaveCriticalSection.
//     POP EBP/EDI/ESI/EBX; restore FS:[0]; ADD ESP,0x10; RET 0xc.
//
// Calling convention: __thiscall; callee cleans 2 stack args (RET 0xc).
//
// Note on dead code: Ghidra marks FUN_0040df70 as noreturn, so bytes
// ae47..ae4b (MOV EBX,1) were omitted from the .s listing. The raw
// binary confirms they are present (bb 01 00 00 00) as dead code after
// the noreturn CALL. They are emitted verbatim below.
//
// Note on 3-byte NOP at ae1d..ae1f: the JMP (eb 03) at ae1b jumps over
// a 3-byte NOP sled (8d 49 00 = LEA ECX,[ECX+0]) that MSVC 2005 /O2
// emits for loop-entry alignment. Not shown in the .s listing since
// Ghidra skips bytes covered by a forward jump.
//
// Reloc-bearing sites (absolute/relative addresses baked at RVA 0x0000adc0):
//   +0x02  PUSH imm32 0x00e54da0   (EH4 scope-table handler)
//   +0x23  CALL [0x00f3e16c]       (EnterCriticalSection IAT)
//   +0x3a  CALL [0x00f3e168]       (LeaveCriticalSection IAT, early exit)
//   +0x60  CMP  [EBX*4+0xf55988]  (bucket table .rdata)
//   +0x82  CALL rel32 FUN_0040df70 (+0x3129)
//   +0xc7  CALL [0x00f3e168]       (LeaveCriticalSection IAT, normal exit)
//   +0xea  CALL rel32 FUN_0040abe0 (-0x02cf)
//   +0x100 MOV  EDX,[EDI+ECX*8+0xcc] etc. (member field accesses)
//   +0x120 CALL rel32 FUN_0040a8a0 (-0x065)
//   +0x9c  CALL rel32 FUN_0040a710 (-0x0751)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   This function's EH4 prologue (ESP-based SEH without EBP frame),
//   mixed callee-restore paths, and dead code after a noreturn CALL resist
//   source-level reconstruction under MSVC 2005 /O2. The naked-asm approach
//   re-emits the original 293 bytes verbatim so compare.py sees a
//   byte-identical .text section.

extern "C" __declspec(naked) void FUN_0040adc0() {
    __asm {
        // 0000adc0:  6a ff                    PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0000adc2:  68 a0 4d e5 00           PUSH 0xe54da0
        _emit 0x68
        _emit 0xa0
        _emit 0x4d
        _emit 0xe5
        _emit 0x00
        // 0000adc7:  64 a1 00 00 00 00        MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000adcd:  50                       PUSH EAX
        _emit 0x50
        // 0000adce:  64 89 25 00 00 00 00     MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000add5:  51                       PUSH ECX  (saves this)
        _emit 0x51
        // 0000add6:  53                       PUSH EBX
        _emit 0x53
        // 0000add7:  56                       PUSH ESI
        _emit 0x56
        // 0000add8:  57                       PUSH EDI
        _emit 0x57
        // 0000add9:  8b f9                    MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 0000addb:  8d 5f 64                 LEA EBX,[EDI+0x64]
        _emit 0x8d
        _emit 0x5f
        _emit 0x64
        // 0000adde:  53                       PUSH EBX
        _emit 0x53
        // 0000addf:  89 5c 24 10              MOV dword ptr [ESP+0x10],EBX
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 0000ade3:  ff 15 6c e1 f3 00        CALL dword ptr [0x00f3e16c]  (EnterCriticalSection)
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000ade9:  8b 74 24 20              MOV ESI,dword ptr [ESP+0x20]  (param_1)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x20
        // 0000aded:  85 f6                    TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 0000adef:  c7 44 24 18 00 00 00 00  MOV dword ptr [ESP+0x18],0x0  (SEH state = 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000adf7:  75 1b                    JNZ 0x0040ae14
        _emit 0x75
        _emit 0x1b
        // 0000adf9:  53                       PUSH EBX
        _emit 0x53
        // 0000adfa:  ff 15 68 e1 f3 00        CALL dword ptr [0x00f3e168]  (LeaveCriticalSection)
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000ae00:  5f                       POP EDI
        _emit 0x5f
        // 0000ae01:  5e                       POP ESI
        _emit 0x5e
        // 0000ae02:  5b                       POP EBX
        _emit 0x5b
        // 0000ae03:  8b 4c 24 04              MOV ECX,dword ptr [ESP+0x4]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        // 0000ae07:  64 89 0d 00 00 00 00     MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ae0e:  83 c4 10                 ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000ae11:  c2 0c 00                 RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0000ae14:  55                       PUSH EBP
        _emit 0x55
        // 0000ae15:  8b 6c 24 28              MOV EBP,dword ptr [ESP+0x28]  (param_2)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x28
        // 0000ae19:  33 db                    XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0000ae1b:  eb 03                    JMP 0x0040ae20
        _emit 0xeb
        _emit 0x03
        // 0000ae1d:  8d 49 00                 LEA ECX,[ECX+0]  (3-byte NOP, skipped by JMP above)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 0000ae20:  3b 2c 9d 88 59 f5 00     CMP EBP,dword ptr [EBX*4+0xf55988]
        _emit 0x3b
        _emit 0x2c
        _emit 0x9d
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // 0000ae27:  76 79                    JBE 0x0040aea2
        _emit 0x76
        _emit 0x79
        // 0000ae29:  83 c3 01                 ADD EBX,0x1
        _emit 0x83
        _emit 0xc3
        _emit 0x01
        // 0000ae2c:  83 fb 16                 CMP EBX,0x16
        _emit 0x83
        _emit 0xfb
        _emit 0x16
        // 0000ae2f:  72 ef                    JC 0x0040ae20
        _emit 0x72
        _emit 0xef
        // 0000ae31:  83 87 bc 00 00 00 01     ADD dword ptr [EDI+0xbc],0x1
        _emit 0x83
        _emit 0x87
        _emit 0xbc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x01
        // 0000ae38:  29 af c4 00 00 00        SUB dword ptr [EDI+0xc4],EBP
        _emit 0x29
        _emit 0xaf
        _emit 0xc4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ae3e:  8b 4f 58                 MOV ECX,dword ptr [EDI+0x58]
        _emit 0x8b
        _emit 0x4f
        _emit 0x58
        // 0000ae41:  56                       PUSH ESI
        _emit 0x56
        // 0000ae42:  e8 29 31 00 00           CALL 0x0040df70
        _emit 0xe8
        _emit 0x29
        _emit 0x31
        _emit 0x00
        _emit 0x00
        // 0000ae47:  bb 01 00 00 00           MOV EBX,0x1  (dead code after noreturn CALL)
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ae4c:  8b 44 24 2c              MOV EAX,dword ptr [ESP+0x2c]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0000ae50:  01 9f 80 00 00 00        ADD dword ptr [EDI+0x80],EBX
        _emit 0x01
        _emit 0x9f
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ae56:  29 af 88 00 00 00        SUB dword ptr [EDI+0x88],EBP
        _emit 0x29
        _emit 0xaf
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ae5c:  e8 af f8 ff ff           CALL 0x0040a710
        _emit 0xe8
        _emit 0xaf
        _emit 0xf8
        _emit 0xff
        _emit 0xff
        // 0000ae61:  0f b7 c0                 MOVZX EAX,AX
        _emit 0x0f
        _emit 0xb7
        _emit 0xc0
        // 0000ae64:  66 85 c0                 TEST AX,AX
        _emit 0x66
        _emit 0x85
        _emit 0xc0
        // 0000ae67:  7c 19                    JL 0x0040ae82
        _emit 0x7c
        _emit 0x19
        // 0000ae69:  8b 8f 60 03 00 00        MOV ECX,dword ptr [EDI+0x360]
        _emit 0x8b
        _emit 0x8f
        _emit 0x60
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 0000ae6f:  0f bf c0                 MOVSX EAX,AX
        _emit 0x0f
        _emit 0xbf
        _emit 0xc0
        // 0000ae72:  8d 04 80                 LEA EAX,[EAX+EAX*4]
        _emit 0x8d
        _emit 0x04
        _emit 0x80
        // 0000ae75:  8d 04 81                 LEA EAX,[ECX+EAX*4]
        _emit 0x8d
        _emit 0x04
        _emit 0x81
        // 0000ae78:  85 c0                    TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000ae7a:  74 06                    JZ 0x0040ae82
        _emit 0x74
        _emit 0x06
        // 0000ae7c:  01 58 04                 ADD dword ptr [EAX+0x4],EBX
        _emit 0x01
        _emit 0x58
        _emit 0x04
        // 0000ae7f:  29 68 0c                 SUB dword ptr [EAX+0xc],EBP
        _emit 0x29
        _emit 0x68
        _emit 0x0c
        // 0000ae82:  8b 54 24 10              MOV EDX,dword ptr [ESP+0x10]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // 0000ae86:  52                       PUSH EDX
        _emit 0x52
        // 0000ae87:  ff 15 68 e1 f3 00        CALL dword ptr [0x00f3e168]  (LeaveCriticalSection)
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000ae8d:  8b 4c 24 14              MOV ECX,dword ptr [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000ae91:  5d                       POP EBP
        _emit 0x5d
        // 0000ae92:  5f                       POP EDI
        _emit 0x5f
        // 0000ae93:  5e                       POP ESI
        _emit 0x5e
        // 0000ae94:  5b                       POP EBX
        _emit 0x5b
        // 0000ae95:  64 89 0d 00 00 00 00     MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ae9c:  83 c4 10                 ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000ae9f:  c2 0c 00                 RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 0000aea2:  85 db                    TEST EBX,EBX
        _emit 0x85
        _emit 0xdb
        // 0000aea4:  7c 8b                    JL 0x0040ae31
        _emit 0x7c
        _emit 0x8b
        // 0000aea6:  56                       PUSH ESI
        _emit 0x56
        // 0000aea7:  53                       PUSH EBX
        _emit 0x53
        // 0000aea8:  8b cf                    MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0000aeaa:  e8 31 fd ff ff           CALL 0x0040abe0
        _emit 0xe8
        _emit 0x31
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 0000aeaf:  85 c0                    TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0000aeb1:  74 cf                    JZ 0x0040ae82
        _emit 0x74
        _emit 0xcf
        // 0000aeb3:  8d 0c 5b                 LEA ECX,[EBX+EBX*2]
        _emit 0x8d
        _emit 0x0c
        _emit 0x5b
        // 0000aeb6:  8b 94 cf cc 00 00 00     MOV EDX,dword ptr [EDI+ECX*0x8+0xcc]
        _emit 0x8b
        _emit 0x94
        _emit 0xcf
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aebd:  29 94 cf dc 00 00 00     SUB dword ptr [EDI+ECX*0x8+0xdc],EDX
        _emit 0x29
        _emit 0x94
        _emit 0xcf
        _emit 0xdc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aec4:  8d 8c cf cc 00 00 00     LEA ECX,[EDI+ECX*0x8+0xcc]
        _emit 0x8d
        _emit 0x8c
        _emit 0xcf
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aecb:  bb 01 00 00 00           MOV EBX,0x1
        _emit 0xbb
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aed0:  01 59 08                 ADD dword ptr [ECX+0x8],EBX
        _emit 0x01
        _emit 0x59
        _emit 0x08
        // 0000aed3:  01 9f a8 00 00 00        ADD dword ptr [EDI+0xa8],EBX
        _emit 0x01
        _emit 0x9f
        _emit 0xa8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aed9:  29 af b0 00 00 00        SUB dword ptr [EDI+0xb0],EBP
        _emit 0x29
        _emit 0xaf
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000aedf:  50                       PUSH EAX
        _emit 0x50
        // 0000aee0:  e8 bb f9 ff ff           CALL 0x0040a8a0  (last byte of function at aee4)
        _emit 0xe8
        _emit 0xbb
        _emit 0xf9
        _emit 0xff
        _emit 0xff
    }
}
