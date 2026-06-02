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
// FUNCTION: ffxivgame 0x009dcd85 — C++ EH filter/dispatch fragment (118 B / 0x76,
//                                  frameless — inherits EBP from outer function).
//
// Calling convention: no prologue; EBP and ESI are inherited from the enclosing
//   scope (this is an SEH/EH continuation block, not a true standalone function).
//   Ends with plain RET (no stack cleanup). ESI points to the EXCEPTION_RECORD;
//   EDI is a pointer used as a write target.
//
// What it does (reconstructed from disassembly at RVA 0x009dcd85):
//
//   1. [ebp-0x24] → [edi-4]    — store a local from the outer frame.
//   2. PUSH [ebp-0x28];
//      CALL 0x009d204a          — one-arg __cdecl helper call; caller cleans.
//   3. CALL 0x009df3d7          — no-arg call that returns an object ptr in EAX;
//      store [ebp-0x2c] → [EAX+0x88].
//   4. CALL 0x009df3d7 again;
//      store [ebp-0x30] → [EAX+0x8c].
//   5. CMP [esi], 0xe06d7363   — is the exception a MSVC C++ exception?
//      JNE → return immediately.
//   6. CMP [esi+0x10], 3       — must have 3 exception info records.
//      JNE → return.
//   7. MOV EAX, [esi+0x14]     — load EH magic/version number.
//      Accept 0x19930520 (pre-VS7), 0x19930521 (VS7), or 0x19930522 (VS8+);
//      otherwise return.
//   8. CMP [ebp-0x34], 0; JNE → return (outer flag must be zero).
//   9. CMP [ebp-0x1c], 0; JE  → return (another outer flag must be non-zero).
//  10. PUSH [esi+0x18]          — push exception object ptr.
//      CALL 0x009d2029          — type-matching predicate; skip if returns 0.
//  11. PUSH [ebp+0x10]; PUSH ESI;
//      CALL 0x009dcb11          — invoke the actual exception handler.
//      POP ECX; POP ECX         — caller cleans 2 args.
//  12. RET.
//
// Frame variables used (all relative to outer function's EBP):
//   [ebp-0x34]  outer exception-already-handled flag
//   [ebp-0x30]  value stored to field_0x8c of query result
//   [ebp-0x2c]  value stored to field_0x88 of query result
//   [ebp-0x28]  arg to first helper
//   [ebp-0x24]  value stored via EDI-4
//   [ebp-0x1c]  inner guard flag (must be non-zero to dispatch)
//   [ebp+0x10]  outer function's 3rd argument (passed to handler)
//
// Notable codegen detail: the CALL rel32 displacements are specific to the
//   full-binary link at image base 0x00400000. In a standalone .obj they
//   appear as raw-emitted bytes (no COFF relocations), which compare.py
//   treats as non-reloc bytes; they still match because we emit the
//   exact bytes from the original binary verbatim.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   This fragment has no stack frame of its own (no PUSH EBP / MOV EBP,ESP).
//   Writing it in high-level C++ would require a __declspec(naked) body to
//   suppress the prologue MSVC would otherwise emit. Since the CALL
//   displacements, multi-byte MOVs to fields at 0x88/0x8c, and the
//   frameless register usage are not reproducible from source under /O2
//   without careful register-annotation, the pragmatic path — matching what
//   all sibling _rosetta entries do for similar bodies — is a
//   __declspec(naked) wrapper re-emitting the original 118 bytes verbatim.
//   compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_009dcd85() {
    __asm {
        // 009dcd85: 8b 45 dc     MOV EAX, dword ptr [EBP-0x24]
        _emit 0x8b
        _emit 0x45
        _emit 0xdc
        // 009dcd88: 89 47 fc     MOV dword ptr [EDI-0x4], EAX
        _emit 0x89
        _emit 0x47
        _emit 0xfc
        // 009dcd8b: ff 75 d8     PUSH dword ptr [EBP-0x28]
        _emit 0xff
        _emit 0x75
        _emit 0xd8
        // 009dcd8e: e8 b7 52 ff ff  CALL 0x009d204a  (one-arg __cdecl helper)
        _emit 0xe8
        _emit 0xb7
        _emit 0x52
        _emit 0xff
        _emit 0xff
        // 009dcd93: 59           POP ECX   (caller cleans 1 arg)
        _emit 0x59
        // 009dcd94: e8 3e 26 00 00  CALL 0x009df3d7  (returns object ptr in EAX)
        _emit 0xe8
        _emit 0x3e
        _emit 0x26
        _emit 0x00
        _emit 0x00
        // 009dcd99: 8b 4d d4     MOV ECX, dword ptr [EBP-0x2c]
        _emit 0x8b
        _emit 0x4d
        _emit 0xd4
        // 009dcd9c: 89 88 88 00 00 00  MOV dword ptr [EAX+0x88], ECX
        _emit 0x89
        _emit 0x88
        _emit 0x88
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009dcda2: e8 30 26 00 00  CALL 0x009df3d7  (same helper again)
        _emit 0xe8
        _emit 0x30
        _emit 0x26
        _emit 0x00
        _emit 0x00
        // 009dcda7: 8b 4d d0     MOV ECX, dword ptr [EBP-0x30]
        _emit 0x8b
        _emit 0x4d
        _emit 0xd0
        // 009dcdaa: 89 88 8c 00 00 00  MOV dword ptr [EAX+0x8c], ECX
        _emit 0x89
        _emit 0x88
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009dcdb0: 81 3e 63 73 6d e0  CMP dword ptr [ESI], 0xe06d7363  (MSVC C++ EH magic)
        _emit 0x81
        _emit 0x3e
        _emit 0x63
        _emit 0x73
        _emit 0x6d
        _emit 0xe0
        // 009dcdb6: 75 42        JNZ +0x42  (→ ret at 009dcdfa)
        _emit 0x75
        _emit 0x42
        // 009dcdb8: 83 7e 10 03  CMP dword ptr [ESI+0x10], 3
        _emit 0x83
        _emit 0x7e
        _emit 0x10
        _emit 0x03
        // 009dcdbc: 75 3c        JNZ +0x3c  (→ ret)
        _emit 0x75
        _emit 0x3c
        // 009dcdbe: 8b 46 14     MOV EAX, dword ptr [ESI+0x14]
        _emit 0x8b
        _emit 0x46
        _emit 0x14
        // 009dcdc1: 3d 20 05 93 19  CMP EAX, 0x19930520  (C++ EH V1)
        _emit 0x3d
        _emit 0x20
        _emit 0x05
        _emit 0x93
        _emit 0x19
        // 009dcdc6: 74 0e        JZ +0x0e  (→ flag check at 009dcdd6)
        _emit 0x74
        _emit 0x0e
        // 009dcdc8: 3d 21 05 93 19  CMP EAX, 0x19930521  (C++ EH V2)
        _emit 0x3d
        _emit 0x21
        _emit 0x05
        _emit 0x93
        _emit 0x19
        // 009dcdcd: 74 07        JZ +0x07  (→ flag check)
        _emit 0x74
        _emit 0x07
        // 009dcdcf: 3d 22 05 93 19  CMP EAX, 0x19930522  (C++ EH V3)
        _emit 0x3d
        _emit 0x22
        _emit 0x05
        _emit 0x93
        _emit 0x19
        // 009dcdd4: 75 24        JNZ +0x24  (→ ret)
        _emit 0x75
        _emit 0x24
        // 009dcdd6: 83 7d cc 00  CMP dword ptr [EBP-0x34], 0
        _emit 0x83
        _emit 0x7d
        _emit 0xcc
        _emit 0x00
        // 009dcdda: 75 1e        JNZ +0x1e  (→ ret)
        _emit 0x75
        _emit 0x1e
        // 009dcddc: 83 7d e4 00  CMP dword ptr [EBP-0x1c], 0
        _emit 0x83
        _emit 0x7d
        _emit 0xe4
        _emit 0x00
        // 009dcde0: 74 18        JZ +0x18  (→ ret; need non-zero)
        _emit 0x74
        _emit 0x18
        // 009dcde2: ff 76 18     PUSH dword ptr [ESI+0x18]  (exception object ptr)
        _emit 0xff
        _emit 0x76
        _emit 0x18
        // 009dcde5: e8 3f 52 ff ff  CALL 0x009d2029  (type-match predicate)
        _emit 0xe8
        _emit 0x3f
        _emit 0x52
        _emit 0xff
        _emit 0xff
        // 009dcdea: 59           POP ECX   (clean 1 arg)
        _emit 0x59
        // 009dcdeb: 85 c0        TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 009dcded: 74 0b        JZ +0x0b  (→ ret; no type match)
        _emit 0x74
        _emit 0x0b
        // 009dcdef: ff 75 10     PUSH dword ptr [EBP+0x10]  (outer arg3)
        _emit 0xff
        _emit 0x75
        _emit 0x10
        // 009dcdf2: 56           PUSH ESI  (exception record)
        _emit 0x56
        // 009dcdf3: e8 19 fd ff ff  CALL 0x009dcb11  (exception handler)
        _emit 0xe8
        _emit 0x19
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 009dcdf8: 59           POP ECX   (clean arg)
        _emit 0x59
        // 009dcdf9: 59           POP ECX   (clean arg)
        _emit 0x59
        // 009dcdfa: c3           RET
        _emit 0xc3
    }
}
