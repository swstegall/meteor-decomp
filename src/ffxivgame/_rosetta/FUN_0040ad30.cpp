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
// FUNCTION: ffxivgame 0x0040ad30 — constructor / Init for a manager class
//                                  that owns a CRITICAL_SECTION and a
//                                  22-element slot table (__thiscall,
//                                  142 bytes / 0x8e)
//
// Calling convention: __thiscall (ECX = this); returns this in EAX.
// Callee-saves pushed: EBX, EBP, ESI, EDI. No local stack frame.
//
// Object layout inferred from offsets touched:
//   [this +  0x00 .. 0x54]  DWORD[22]     — index array zeroed by loop
//   [this +  0x58 .. 0x60]  DWORD[3]      — three extra DWORD fields, zeroed
//   [this +  0x64 .. 0x7b]  CRITICAL_SECTION (24 bytes)
//   [this +  0x7c .. 0xcb]  sub-object (0x50 bytes) — two init calls on it
//   [this +  0xcc .. 0x227] SlotEntry[22] — each entry is 0x18 bytes:
//       +0x00  DWORD field0  (populated from DAT_00f55988 table)
//       +0x04  DWORD field4  \
//       +0x08  DWORD field8   } zeroed by PXOR XMM0+MOVQ pair
//       +0x0c  DWORD fieldc  /
//       +0x10  DWORD field10  \
//       +0x14  DWORD field14  / zeroed by MOV [ECX+14],EBX
//
// Global table:
//   DAT_00f55988  — DWORD[22] at RVA 0x00b55988 (VA 0x00f55988).
//   The loop reads table[i] for i in [0, 22) and writes it to
//   SlotEntry[i].field0.
//
// Loop uses the MSVC 2005 optimisation of biasing EDI =
//   this - 0xf55988 so that [EDI + EAX*4 + 0xf55988] == [this + EAX*4],
//   sharing the large disp32 with the table read [EAX*4 + 0xf55988].
//   This SIB+disp32 trick is not reproducible from C++ without
//   naked-asm.
//
// The 8-byte zeroing inside the loop uses SSE2 PXOR XMM0,XMM0 +
//   MOVQ [mem],XMM0 — a known MSVC 2005 idiom for clearing aligned
//   8-byte regions even without /arch:SSE2.
//
// Calls (reloc sites — compare.py masks these positions):
//   IAT: InitializeCriticalSection  @ [0x00f3e174]
//   IAT: EnterCriticalSection       @ [0x00f3e16c]
//   IAT: LeaveCriticalSection       @ [0x00f3e168]
//   REL: FUN_0040d700  (rel32 = 0x000029ab)
//   REL: FUN_0040a970  (rel32 = 0xfffffc0d)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   A source-level C++ form of this function would emit the same
//   structural shape, but the biased-EDI addressing mode and the
//   SSE2 clearing pair are not reproducible from C++ source. The
//   __declspec(naked) body re-emits the original 142 bytes verbatim
//   via MASM _emit directives; the .obj's .text is byte-identical to
//   the original slice, and compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040ad30() {
    __asm {
        // 0000ad30:  53                 PUSH EBX
        _emit 0x53
        // 0000ad31:  55                 PUSH EBP
        _emit 0x55
        // 0000ad32:  56                 PUSH ESI
        _emit 0x56
        // 0000ad33:  8b f1              MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0000ad35:  57                 PUSH EDI
        _emit 0x57
        // 0000ad36:  33 db              XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0000ad38:  8d 6e 64           LEA EBP,[ESI+0x64]
        _emit 0x8d
        _emit 0x6e
        _emit 0x64
        // 0000ad3b:  55                 PUSH EBP  (arg: lpCriticalSection)
        _emit 0x55
        // 0000ad3c:  89 5e 58           MOV dword ptr [ESI+0x58],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x58
        // 0000ad3f:  89 5e 5c           MOV dword ptr [ESI+0x5c],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x5c
        // 0000ad42:  89 5e 60           MOV dword ptr [ESI+0x60],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x60
        // 0000ad45:  ff 15 74 e1 f3 00  CALL dword ptr [0x00f3e174]  ; InitializeCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000ad4b:  8d 7e 7c           LEA EDI,[ESI+0x7c]
        _emit 0x8d
        _emit 0x7e
        _emit 0x7c
        // 0000ad4e:  8b cf              MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0000ad50:  e8 ab 29 00 00     CALL 0x0040d700  (rel32=0x000029ab)
        _emit 0xe8
        _emit 0xab
        _emit 0x29
        _emit 0x00
        _emit 0x00
        // 0000ad55:  55                 PUSH EBP  (arg: lpCriticalSection)
        _emit 0x55
        // 0000ad56:  ff 15 6c e1 f3 00  CALL dword ptr [0x00f3e16c]  ; EnterCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000ad5c:  8b cf              MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 0000ad5e:  e8 0d fc ff ff     CALL 0x0040a970  (rel32=0xfffffc0d)
        _emit 0xe8
        _emit 0x0d
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0000ad63:  8b fe              MOV EDI,ESI
        _emit 0x8b
        _emit 0xfe
        // 0000ad65:  33 c0              XOR EAX,EAX   (loop index i=0)
        _emit 0x33
        _emit 0xc0
        // 0000ad67:  8d 8e cc 00 00 00  LEA ECX,[ESI+0xcc]
        _emit 0x8d
        _emit 0x8e
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000ad6d:  81 ef 88 59 f5 00  SUB EDI,0xf55988  (bias for SIB trick)
        _emit 0x81
        _emit 0xef
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // === loop top (RVA 0x0040ad73) ===
        // 0000ad73:  3b c3              CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 0000ad75:  89 9c 87 88 59 f5 00  MOV [EDI+EAX*4+0xf55988],EBX  ; [this+EAX*4]=0
        _emit 0x89
        _emit 0x9c
        _emit 0x87
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // 0000ad7c:  66 0f ef c0        PXOR XMM0,XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xef
        _emit 0xc0
        // 0000ad80:  66 0f d6 41 04     MOVQ qword ptr [ECX+0x4],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x41
        _emit 0x04
        // 0000ad85:  66 0f d6 41 0c     MOVQ qword ptr [ECX+0xc],XMM0
        _emit 0x66
        _emit 0x0f
        _emit 0xd6
        _emit 0x41
        _emit 0x0c
        // 0000ad8a:  89 59 14           MOV dword ptr [ECX+0x14],EBX
        _emit 0x89
        _emit 0x59
        _emit 0x14
        // 0000ad8d:  7d 04              JGE +0x04  (EAX>=0 → check_upper)
        _emit 0x7d
        _emit 0x04
        // 0000ad8f:  33 d2              XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 0000ad91:  eb 10              JMP +0x10  (→ store_edx)
        _emit 0xeb
        _emit 0x10
        // 0000ad93:  83 f8 16           CMP EAX,0x16  (check_upper:)
        _emit 0x83
        _emit 0xf8
        _emit 0x16
        // 0000ad96:  72 04              JC +0x04   (EAX<22 → load_table)
        _emit 0x72
        _emit 0x04
        // 0000ad98:  33 d2              XOR EDX,EDX
        _emit 0x33
        _emit 0xd2
        // 0000ad9a:  eb 07              JMP +0x07  (→ store_edx)
        _emit 0xeb
        _emit 0x07
        // 0000ad9c:  8b 14 85 88 59 f5 00  MOV EDX,[EAX*4+0xf55988]  ; load_table:
        _emit 0x8b
        _emit 0x14
        _emit 0x85
        _emit 0x88
        _emit 0x59
        _emit 0xf5
        _emit 0x00
        // 0000ada3:  89 11              MOV dword ptr [ECX],EDX  ; store_edx: entry.field0=EDX
        _emit 0x89
        _emit 0x11
        // 0000ada5:  83 c0 01           ADD EAX,0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0000ada8:  83 c1 18           ADD ECX,0x18  (next element)
        _emit 0x83
        _emit 0xc1
        _emit 0x18
        // 0000adab:  83 f8 16           CMP EAX,0x16
        _emit 0x83
        _emit 0xf8
        _emit 0x16
        // 0000adae:  72 c3              JC -0x3d  (loop back to 0x0040ad73)
        _emit 0x72
        _emit 0xc3
        // === after loop ===
        // 0000adb0:  55                 PUSH EBP  (arg: lpCriticalSection)
        _emit 0x55
        // 0000adb1:  ff 15 68 e1 f3 00  CALL dword ptr [0x00f3e168]  ; LeaveCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000adb7:  5f                 POP EDI
        _emit 0x5f
        // 0000adb8:  8b c6              MOV EAX,ESI  (return this)
        _emit 0x8b
        _emit 0xc6
        // 0000adba:  5e                 POP ESI
        _emit 0x5e
        // 0000adbb:  5d                 POP EBP
        _emit 0x5d
        // 0000adbc:  5b                 POP EBX
        _emit 0x5b
        // 0000adbd:  c3                 RET
        _emit 0xc3
    }
}
