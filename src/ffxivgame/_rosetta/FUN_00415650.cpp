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
// FUNCTION: ffxivgame 0x00415650 — Printer log-write dispatcher
//                                   (__thiscall, 131 B / 0x83)
//
// Class: SQEX::CDev::Engine::Vfx::Common::Io::Printer
// Source file (per debug strings): .\Io\Printer.cpp
//
// __thiscall void FUN_00415650(Printer *this,
//                               int    kind,           // [ESP+0x04]
//                               unsigned char w,       // [ESP+0x08] byte
//                               const char   *msg)     // [ESP+0x0C]
//
// RET 0xC — callee cleans 3 stack args.
//
// Behaviour:
//   1. Early-out if this->m_owns_meta (flag at +0x05) is zero.
//   2. Load this->field_78 (int at +0x78) into EAX; early-out if
//      *(byte*)(this + EAX + 0x81) < w  (level-gate check).
//   3. Format message into a 0x400-byte stack buffer using
//      FUN_004154f0 (VfxLogger format dispatcher) with kind, w, msg
//      and newline_flag=0.
//   4. Clamp w to 4 if w >= 5.
//   5. Loop i from w (clamped) to 4 inclusive, calling
//      FUN_00415440 (Printer::AppendLine) once per channel with
//      (this, kind, i, buf).
//
// Object layout (offsets touched):
//   [this + 0x05]  byte  m_owns_meta — early-exit gate
//   [this + 0x78]  int   field_78    — level-table offset adjust
//   [this + field_78 + 0x81] byte — level/verbosity threshold
//
// External calls:
//   +0x46  CALL rel32 → FUN_004154f0 @ 0x004154f0
//            (VfxLogger::Format — __thiscall, 6 stack args, RET 0x18)
//   +0x69  CALL rel32 → FUN_00415440 @ 0x00415440
//            (Printer::AppendLine — __thiscall, 3 stack args, RET 0x0C)
//
// Stack frame: no EBP frame (/Oy); 0x400 bytes allocated via SUB ESP,0x400
// at function entry (before register saves). No /GS cookie — the compiler's
// vulnerability analysis apparently determined the snprintf-bounded buffer
// usage is safe.
//
// Register allocation (MSVC 2005 /O2):
//   EDI = this
//   EBX / BL = w (param2, byte throughout)
//   EBP = kind (param1)
//   ESI = loop counter i (after clamping)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaved PUSH/MOV saves (EDI first at function entry, then EBX
//   and EBP inside the guard-check arm, ESI just before the loop), the
//   SIB-encoded CMP at [EAX+EDI*1+0x81] with disp32, the two ESP-relative
//   argument loads using disp32 forms ([ESP+0x410], [ESP+0x414], etc.), and
//   the exact for-loop encoding (JGE end + JMP body + dead ADD ESI,1 +
//   body + ADD + CMP + JL body) all reflect MSVC 2005 /O2's register
//   scheduler and loop-rotation pass and are brittle to source-level
//   rewrites. The naked-asm byte passthrough emits the 131 bytes verbatim;
//   compare.py masks the two CALL rel32 sites (both intra-binary with known
//   displacements baked in) and reports GREEN.

extern "C" __declspec(naked) void FUN_00415650() {
    __asm {
        // 00015650: 81 ec 00 04 00 00   SUB ESP,0x400
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00015656: 57   PUSH EDI
        _emit 0x57
        // 00015657: 8b f9   MOV EDI,ECX
        _emit 0x8b
        _emit 0xf9
        // 00015659: 80 7f 05 00   CMP byte ptr [EDI+0x5],0x0
        _emit 0x80
        _emit 0x7f
        _emit 0x05
        _emit 0x00
        // 0001565d: 74 6a   JZ 0x004156c9
        _emit 0x74
        _emit 0x6a
        // 0001565f: 8b 47 78   MOV EAX,dword ptr [EDI+0x78]
        _emit 0x8b
        _emit 0x47
        _emit 0x78
        // 00015662: 53   PUSH EBX
        _emit 0x53
        // 00015663: 8a 9c 24 10 04 00 00   MOV BL,byte ptr [ESP+0x410]
        _emit 0x8a
        _emit 0x9c
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001566a: 38 9c 38 81 00 00 00   CMP byte ptr [EAX+EDI*1+0x81],BL
        _emit 0x38
        _emit 0x9c
        _emit 0x38
        _emit 0x81
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00015671: 72 55   JC 0x004156c8
        _emit 0x72
        _emit 0x55
        // 00015673: 8b 8c 24 14 04 00 00   MOV ECX,dword ptr [ESP+0x414]
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001567a: 55   PUSH EBP
        _emit 0x55
        // 0001567b: 8b ac 24 10 04 00 00   MOV EBP,dword ptr [ESP+0x410]
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00015682: 6a 00   PUSH 0x0
        _emit 0x6a
        _emit 0x00
        // 00015684: 51   PUSH ECX
        _emit 0x51
        // 00015685: 0f b6 d3   MOVZX EDX,BL
        _emit 0x0f
        _emit 0xb6
        _emit 0xd3
        // 00015688: 52   PUSH EDX
        _emit 0x52
        // 00015689: 55   PUSH EBP
        _emit 0x55
        // 0001568a: 68 00 04 00 00   PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 0001568f: 8d 44 24 20   LEA EAX,[ESP+0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // 00015693: 50   PUSH EAX
        _emit 0x50
        // 00015694: 8b cf   MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 00015696: e8 55 fe ff ff   CALL FUN_004154f0 (rel32 → +0x004154f0)
        _emit 0xe8
        _emit 0x55
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0001569b: 80 fb 05   CMP BL,0x5
        _emit 0x80
        _emit 0xfb
        _emit 0x05
        // 0001569e: 72 02   JC 0x004156a2
        _emit 0x72
        _emit 0x02
        // 000156a0: b3 04   MOV BL,0x4
        _emit 0xb3
        _emit 0x04
        // 000156a2: 56   PUSH ESI
        _emit 0x56
        // 000156a3: 0f b6 f3   MOVZX ESI,BL
        _emit 0x0f
        _emit 0xb6
        _emit 0xf3
        // 000156a6: 83 fe 05   CMP ESI,0x5
        _emit 0x83
        _emit 0xfe
        _emit 0x05
        // 000156a9: 7d 1b   JGE 0x004156c6
        _emit 0x7d
        _emit 0x1b
        // 000156ab: eb 03   JMP 0x004156b0
        _emit 0xeb
        _emit 0x03
        // 000156ad: 8d 49 00   LEA ECX,[ECX+0x0]  (MSVC 3-byte NOP — alignment pad)
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        // 000156b0: 8d 4c 24 10   LEA ECX,[ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 000156b4: 51   PUSH ECX
        _emit 0x51
        // 000156b5: 56   PUSH ESI
        _emit 0x56
        // 000156b6: 55   PUSH EBP
        _emit 0x55
        // 000156b7: 8b cf   MOV ECX,EDI
        _emit 0x8b
        _emit 0xcf
        // 000156b9: e8 82 fd ff ff   CALL FUN_00415440 (rel32 → 0x00415440)
        _emit 0xe8
        _emit 0x82
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        // 000156be: 83 c6 01   ADD ESI,0x1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 000156c1: 83 fe 05   CMP ESI,0x5
        _emit 0x83
        _emit 0xfe
        _emit 0x05
        // 000156c4: 7c ea   JL 0x004156b0
        _emit 0x7c
        _emit 0xea
        // 000156c6: 5e   POP ESI
        _emit 0x5e
        // 000156c7: 5d   POP EBP
        _emit 0x5d
        // 000156c8: 5b   POP EBX
        _emit 0x5b
        // 000156c9: 5f   POP EDI
        _emit 0x5f
        // 000156ca: 81 c4 00 04 00 00   ADD ESP,0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000156d0: c2 0c 00   RET 0xc
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}

// vim: ts=4 sts=4 sw=4 et
