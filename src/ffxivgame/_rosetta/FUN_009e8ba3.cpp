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
// FUNCTION: ffxivgame 0x005e8ba3 — bit-array conditional clear (46 B / 0x2E)
//
// Asm (46 bytes @ orig RVA 0x005e8ba3):
//   0x00: 39 75 e4              CMP  dword ptr [EBP-0x1C], ESI
//   0x03: 74 28                 JZ   +0x28 → RET
//   0x05: 39 75 e0              CMP  dword ptr [EBP-0x20], ESI
//   0x08: 74 1b                 JZ   +0x1B → push_call
//   0x0A: 8b 07                 MOV  EAX, dword ptr [EDI]
//   0x0C: 8b c8                 MOV  ECX, EAX
//   0x0E: c1 f9 05              SAR  ECX, 5
//   0x11: 83 e0 1f              AND  EAX, 0x1F
//   0x14: c1 e0 06              SHL  EAX, 6
//   0x17: 8b 0c 8d e0 b7 37 01  MOV  ECX, dword ptr [ECX*4 + 0x0137B7E0]
//   0x1E: 8d 44 01 04           LEA  EAX, [ECX + EAX + 4]
//   0x22: 80 20 fe              AND  byte ptr [EAX], 0xFE
// push_call:
//   0x25: ff 37                 PUSH dword ptr [EDI]
//   0x27: e8 be ec ff ff        CALL FUN_005e788d (RVA 0x5e788d)
//   0x2C: 59                    POP  ECX
//   0x2D: c3                    RET
//
// Calling convention: cannot be cleanly inferred without the full frame
//   prologue/epilogue — this is a fragment visible from call sites. Uses
//   EBP-based frame (caller's), EDI as a pointer, ESI as a sentinel.
//
// Reconstruction: __declspec(naked) _emit byte passthrough.  The absolute
//   imm32 at offset 0x1A (0x0137B7E0) and the rel32 at offset 0x28
//   (→ FUN_005e788d) are relocation sites masked by tools/compare.py.

extern "C" __declspec(naked) void FUN_009e8ba3() {
    __asm {
        // 005e8ba3: 39 75 e4              CMP dword ptr [EBP-0x1C], ESI
        _emit 0x39
        _emit 0x75
        _emit 0xe4
        // 005e8ba6: 74 28                 JZ +0x28 (→ RET)
        _emit 0x74
        _emit 0x28
        // 005e8ba8: 39 75 e0              CMP dword ptr [EBP-0x20], ESI
        _emit 0x39
        _emit 0x75
        _emit 0xe0
        // 005e8bab: 74 1b                 JZ +0x1B (→ push_call)
        _emit 0x74
        _emit 0x1b
        // 005e8bad: 8b 07                 MOV EAX, dword ptr [EDI]
        _emit 0x8b
        _emit 0x07
        // 005e8baf: 8b c8                 MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // 005e8bb1: c1 f9 05              SAR ECX, 5
        _emit 0xc1
        _emit 0xf9
        _emit 0x05
        // 005e8bb4: 83 e0 1f              AND EAX, 0x1F
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 005e8bb7: c1 e0 06              SHL EAX, 6
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        // 005e8bba: 8b 0c 8d e0 b7 37 01 MOV ECX, dword ptr [ECX*4 + 0x0137B7E0]
        _emit 0x8b
        _emit 0x0c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005e8bc1: 8d 44 01 04           LEA EAX, [ECX + EAX + 4]
        _emit 0x8d
        _emit 0x44
        _emit 0x01
        _emit 0x04
        // 005e8bc5: 80 20 fe              AND byte ptr [EAX], 0xFE
        _emit 0x80
        _emit 0x20
        _emit 0xfe
        // push_call:
        // 005e8bc8: ff 37                 PUSH dword ptr [EDI]
        _emit 0xff
        _emit 0x37
        // 005e8bca: e8 be ec ff ff        CALL FUN_005e788d
        _emit 0xe8
        _emit 0xbe
        _emit 0xec
        _emit 0xff
        _emit 0xff
        // 005e8bcf: 59                    POP ECX
        _emit 0x59
        // 005e8bd0: c3                    RET
        _emit 0xc3
    }
}
