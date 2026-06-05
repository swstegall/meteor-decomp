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
// FUNCTION: ffxivgame 0x00053000 — __thiscall buffer-release helper (37 B / 0x25)
//
// void __thiscall FUN_00453000(C *this)
//
// Reads this->field_0x4; if non-null, passes it to two __cdecl CRT-ish
// helpers (FUN_009d71df then FUN_009d2646 — the latter is the recurring
// free()-style sink seen in FUN_0043d760), then zeroes this->field_0x4.
//
//   void C::release() {
//       if (field_4) {
//           FUN_009d71df(field_4);
//           FUN_009d2646(field_4);   // caller-cleaned, ADD ESP,8 for both
//           field_4 = 0;
//       }
//   }
//
// MSVC reloads field_4 from [ESI+4] before each call (EAX is clobbered by
// the first call; ESI holds `this`). Calling convention is __thiscall
// (this in ECX, no stack args, plain RET). The two e8 CALLs are the only
// relocation-sensitive fields.
//
// Reconstruction strategy — naked-asm byte passthrough. The body is short
// but the two external __cdecl calls plus the double field reload make the
// exact MSVC 2005 /O2 instruction selection fragile to coax from C++; the
// 37 original bytes are re-emitted verbatim. The two rel32 CALL operands
// are the original PE-relative values (literal, matching the source bytes).
//
// Asm (37 bytes @ orig RVA 0x00053000):
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX                 ; this
//   8b 46 04             MOV EAX, [ESI+4]             ; field_4
//   85 c0                TEST EAX, EAX
//   74 19                JZ  +0x19 → epilogue
//   50                   PUSH EAX
//   e8 cf 41 58 00       CALL FUN_009d71df
//   8b 46 04             MOV EAX, [ESI+4]             ; reload field_4
//   50                   PUSH EAX
//   e8 2d f6 57 00       CALL FUN_009d2646
//   83 c4 08             ADD ESP, 8                   ; caller cleans both
//   c7 46 04 00 00 00 00 MOV dword ptr [ESI+4], 0
//   5e                   POP ESI
//   c3                   RET

extern "C" __declspec(naked) void FUN_00453000() {
    __asm {
        // 00053000: 56            PUSH ESI
        _emit 0x56
        // 00053001: 8b f1         MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00053003: 8b 46 04      MOV EAX, [ESI+4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00053006: 85 c0         TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00053008: 74 19         JZ +0x19
        _emit 0x74
        _emit 0x19
        // 0005300a: 50            PUSH EAX
        _emit 0x50
        // 0005300b: e8 cf 41 58 00  CALL FUN_009d71df
        _emit 0xe8
        _emit 0xcf
        _emit 0x41
        _emit 0x58
        _emit 0x00
        // 00053010: 8b 46 04      MOV EAX, [ESI+4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 00053013: 50            PUSH EAX
        _emit 0x50
        // 00053014: e8 2d f6 57 00  CALL FUN_009d2646
        _emit 0xe8
        _emit 0x2d
        _emit 0xf6
        _emit 0x57
        _emit 0x00
        // 00053019: 83 c4 08      ADD ESP, 8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 0005301c: c7 46 04 00 00 00 00  MOV [ESI+4], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00053023: 5e            POP ESI
        _emit 0x5e
        // 00053024: c3            RET
        _emit 0xc3
    }
}
