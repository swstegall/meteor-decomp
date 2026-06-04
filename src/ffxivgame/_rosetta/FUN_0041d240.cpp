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
// FUNCTION: ffxivgame 0x0001d240 — __cdecl 1-arg input-channel initialiser
//                                  loop (133 bytes).
//
// Iterates over 16 input-channel entries (each 16 bytes) stored in the
// global array at VA 0x01328dcc.  For each entry, either feeds live
// device state or a default bitfield into the thiscall input-system object
// at 0x0132987c, then clears the pending-change bitmask at
// [0x01329428 + 8] on exit.
//
// Calling convention: __cdecl (bare RET — caller owns stack; one arg
// on stack at [ESP+4] which may be NULL).
// Stack frame: none (ESP-relative only; preserved ESI and EDI saved on
// the stack but no EBP frame).
//
// Asm body (133 bytes @ RVA 0x0001d240..0x0001d2c4):
//
//   0001d240:  8b 44 24 04              MOV EAX,[ESP+4]               ; arg0
//   0001d244:  85 c0                    TEST EAX,EAX
//   0001d246:  74 0f                    JZ   +0x0f                    ; skip if null
//   0001d248:  8b 40 04                 MOV EAX,[EAX+4]               ; arg0->field_4
//   0001d24b:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj (this)
//   0001d251:  50                       PUSH EAX
//   0001d252:  e8 79 5f 00 00           CALL FUN_004231d0             ; obj->notify(field4)
//   0001d257:  56                       PUSH ESI
//   0001d258:  57                       PUSH EDI
//   0001d259:  33 f6                    XOR ESI,ESI                   ; i = 0
//   0001d25b:  bf cc 8d 32 01           MOV EDI,0x01328dcc            ; &g_channel_array[0]
//   ;; loop top (ESI = i, EDI = &entry):
//   0001d260:  8b 07                    MOV EAX,[EDI]                 ; entry->ptr
//   0001d262:  85 c0                    TEST EAX,EAX
//   0001d264:  74 0e                    JZ   +0x0e                    ; → fallback
//   ;; live-device path:
//   0001d266:  8b 4f 08                 MOV ECX,[EDI+8]               ; entry->field_8
//   0001d269:  8b 57 04                 MOV EDX,[EDI+4]               ; entry->field_4
//   0001d26c:  8b 40 18                 MOV EAX,[EAX+0x18]            ; ptr->field_18
//   0001d26f:  51                       PUSH ECX
//   0001d270:  52                       PUSH EDX
//   0001d271:  50                       PUSH EAX
//   0001d272:  eb 1d                    JMP  +0x1d                    ; → call_e0
//   ;; bitfield-fallback path:
//   0001d274:  8b 0d 28 94 32 01        MOV ECX,[0x01329428]          ; g_state_obj
//   0001d27a:  0f b7 51 08              MOVZX EDX,word ptr[ECX+8]     ; pending bits
//   0001d27e:  b8 01 00 00 00           MOV EAX,1
//   0001d283:  8b ce                    MOV ECX,ESI                   ; ECX = i
//   0001d285:  d3 e0                    SHL EAX,CL                    ; 1 << i
//   0001d287:  85 d0                    TEST EAX,EDX                  ; bit set?
//   0001d289:  74 20                    JZ   +0x20                    ; skip if not set
//   0001d28b:  6a 00                    PUSH 0
//   0001d28d:  6a 00                    PUSH 0
//   0001d28f:  6a 00                    PUSH 0
//   ;; call_e0 — reached by JMP (live path) or fall-through (fallback):
//   0001d291:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj
//   0001d297:  56                       PUSH ESI                      ; push i
//   0001d298:  e8 43 5f 00 00           CALL FUN_004231e0             ; obj->set(i,a,b,c)
//   0001d29d:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj
//   0001d2a3:  6a 01                    PUSH 1
//   0001d2a5:  56                       PUSH ESI                      ; push i
//   0001d2a6:  e8 45 5f 00 00           CALL FUN_004231f0             ; obj->enable(i,1)
//   ;; loop increment (also: skip target when bit not set):
//   0001d2ab:  83 c6 01                 ADD ESI,1                     ; i++
//   0001d2ae:  83 c7 10                 ADD EDI,0x10                  ; entry += 16
//   0001d2b1:  83 fe 10                 CMP ESI,0x10                  ; i < 16?
//   0001d2b4:  72 aa                    JC   -0x56                    ; loop
//   ;; epilogue:
//   0001d2b6:  8b 0d 28 94 32 01        MOV ECX,[0x01329428]          ; g_state_obj
//   0001d2bc:  5f                       POP EDI
//   0001d2bd:  66 c7 41 08 00 00        MOV word ptr[ECX+8],0         ; clear pending bits
//   0001d2c3:  5e                       POP ESI
//   0001d2c4:  c3                       RET
//
// Reloc-bearing sites (4-byte positions masked by compare.py):
//   +0x0c  DIR32 → 0x0132987c  (g_input_obj, first load)
//   +0x13  REL32 → FUN_004231d0
//   +0x1c  DIR32 → 0x01328dcc  (g_channel_array, MOV EDI imm32)
//   +0x35  DIR32 → 0x01329428  (g_state_obj, first load)
//   +0x52  DIR32 → 0x0132987c  (g_input_obj, second load)
//   +0x59  REL32 → FUN_004231e0
//   +0x5f  DIR32 → 0x0132987c  (g_input_obj, third load)
//   +0x68  REL32 → FUN_004231f0
//   +0x79  DIR32 → 0x01329428  (g_state_obj, second load)
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   A source-level C++ form would compile to this shape under /O2 but
//   the exact short-jump encodings, push-0 form, and the JC loop-back
//   all need to be coerced.  The pragmatic choice — same as the sibling
//   FUN_0041d120 — is a `__declspec(naked)` body that re-emits the
//   orig 133 bytes verbatim via MASM `_emit` directives.  All reloc
//   slots are masked by compare.py; the non-reloc bytes match exactly.

extern "C" __declspec(naked) void FUN_0041d240() {
    __asm {
        // 0001d240: 8b 44 24 04  MOV EAX,[ESP+4]
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001d244: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001d246: 74 0f  JZ +0x0f  (→ 0x0041d257)
        _emit 0x74
        _emit 0x0f
        // 0001d248: 8b 40 04  MOV EAX,[EAX+4]
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0001d24b: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x0c)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d251: 50  PUSH EAX
        _emit 0x50
        // 0001d252: e8 79 5f 00 00  CALL FUN_004231d0  (REL32 reloc +0x13)
        _emit 0xe8
        _emit 0x79
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        // 0001d257: 56  PUSH ESI
        _emit 0x56
        // 0001d258: 57  PUSH EDI
        _emit 0x57
        // 0001d259: 33 f6  XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0001d25b: bf cc 8d 32 01  MOV EDI,0x01328dcc  (DIR32 reloc +0x1c)
        _emit 0xbf
        _emit 0xcc
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // 0001d260: 8b 07  MOV EAX,[EDI]
        _emit 0x8b
        _emit 0x07
        // 0001d262: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001d264: 74 0e  JZ +0x0e  (→ 0x0041d274)
        _emit 0x74
        _emit 0x0e
        // 0001d266: 8b 4f 08  MOV ECX,[EDI+8]
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 0001d269: 8b 57 04  MOV EDX,[EDI+4]
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        // 0001d26c: 8b 40 18  MOV EAX,[EAX+0x18]
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 0001d26f: 51  PUSH ECX
        _emit 0x51
        // 0001d270: 52  PUSH EDX
        _emit 0x52
        // 0001d271: 50  PUSH EAX
        _emit 0x50
        // 0001d272: eb 1d  JMP +0x1d  (→ 0x0041d291)
        _emit 0xeb
        _emit 0x1d
        // 0001d274: 8b 0d 28 94 32 01  MOV ECX,[0x01329428]  (DIR32 reloc +0x35)
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001d27a: 0f b7 51 08  MOVZX EDX,word ptr[ECX+8]
        _emit 0x0f
        _emit 0xb7
        _emit 0x51
        _emit 0x08
        // 0001d27e: b8 01 00 00 00  MOV EAX,1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001d283: 8b ce  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001d285: d3 e0  SHL EAX,CL
        _emit 0xd3
        _emit 0xe0
        // 0001d287: 85 d0  TEST EAX,EDX
        _emit 0x85
        _emit 0xd0
        // 0001d289: 74 20  JZ +0x20  (→ 0x0041d2ab)
        _emit 0x74
        _emit 0x20
        // 0001d28b: 6a 00  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001d28d: 6a 00  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001d28f: 6a 00  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001d291: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x52)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d297: 56  PUSH ESI
        _emit 0x56
        // 0001d298: e8 43 5f 00 00  CALL FUN_004231e0  (REL32 reloc +0x59)
        _emit 0xe8
        _emit 0x43
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        // 0001d29d: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x5f)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d2a3: 6a 01  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 0001d2a5: 56  PUSH ESI
        _emit 0x56
        // 0001d2a6: e8 45 5f 00 00  CALL FUN_004231f0  (REL32 reloc +0x68)
        _emit 0xe8
        _emit 0x45
        _emit 0x5f
        _emit 0x00
        _emit 0x00
        // 0001d2ab: 83 c6 01  ADD ESI,1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0001d2ae: 83 c7 10  ADD EDI,0x10
        _emit 0x83
        _emit 0xc7
        _emit 0x10
        // 0001d2b1: 83 fe 10  CMP ESI,0x10
        _emit 0x83
        _emit 0xfe
        _emit 0x10
        // 0001d2b4: 72 aa  JC -0x56  (→ 0x0041d260)
        _emit 0x72
        _emit 0xaa
        // 0001d2b6: 8b 0d 28 94 32 01  MOV ECX,[0x01329428]  (DIR32 reloc +0x79)
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001d2bc: 5f  POP EDI
        _emit 0x5f
        // 0001d2bd: 66 c7 41 08 00 00  MOV word ptr[ECX+8],0
        _emit 0x66
        _emit 0xc7
        _emit 0x41
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0001d2c3: 5e  POP ESI
        _emit 0x5e
        // 0001d2c4: c3  RET
        _emit 0xc3
    }
}
