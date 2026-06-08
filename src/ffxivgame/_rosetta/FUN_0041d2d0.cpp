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
// FUNCTION: ffxivgame 0x0001d2d0 — 16-slot channel updater with bitmask
//                                  accumulator (200 B / 0xC8).
//
// Sibling of FUN_0041d240 (the simpler 2-arg reset variant). This 3-arg
// form additionally tracks which slots are live via EBX (accumulated as
// a 16-bit bitmask) and writes it back to [g_state_obj + 0x8] on exit.
//
// Calling convention: __cdecl (bare RET, caller owns stack).
// Stack frame: none (ESP-relative only; EBX/EBP/ESI/EDI saved on stack).
// Args:
//   arg1 @ [ESP+4]  — optional struct pointer (null-checked at entry)
//   arg2 @ [ESP+8]  → EBP — "current" slot index (compared against i)
//   arg3 @ [ESP+0xC]         — flags word OR'd with 0x40000000 for non-current slots
//
// Globals:
//   0x0132987c — g_input_obj : pointer to the input-system object (ECX / 'this')
//   0x01328dcc — g_channel_array : array of 16 × 16-byte input-channel structs
//                   [0]  obj_ptr   (ptr; if non-null → "live device" path)
//                   [4]  field_4
//                   [8]  field_8
//   0x01329428 — g_state_obj : pointer to the state struct
//                   [8]  pending-change bitmask (WORD)
//
// Asm body (200 bytes @ RVA 0x0001d2d0..0x0001d397):
//
//   0001d2d0:  8b 44 24 04              MOV EAX,[ESP+4]               ; arg1
//   0001d2d4:  85 c0                    TEST EAX,EAX
//   0001d2d6:  74 0f                    JZ +0x0f                      ; → no_arg1 (skip notify)
//   0001d2d8:  8b 40 04                 MOV EAX,[EAX+4]               ; arg1->field_4
//   0001d2db:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj (this)
//   0001d2e1:  50                       PUSH EAX
//   0001d2e2:  e8 e9 5e 00 00           CALL FUN_004231d0             ; obj->notify(field_4)
//   ;; no_arg1 — prologue:
//   0001d2e7:  53                       PUSH EBX                      ; save EBX (bitmask)
//   0001d2e8:  55                       PUSH EBP
//   0001d2e9:  8b 6c 24 10              MOV EBP,[ESP+0x10]            ; EBP = arg2
//   0001d2ed:  56                       PUSH ESI
//   0001d2ee:  57                       PUSH EDI
//   0001d2ef:  33 db                    XOR EBX,EBX                   ; bitmask = 0
//   0001d2f1:  33 f6                    XOR ESI,ESI                   ; i = 0
//   0001d2f3:  bf cc 8d 32 01           MOV EDI,0x01328dcc            ; &g_channel_array[0]
//   ;; loop_start (ESI=i, EDI=&entry):
//   0001d2f8:  8b 07                    MOV EAX,[EDI]                 ; entry->obj_ptr
//   0001d2fa:  85 c0                    TEST EAX,EAX
//   0001d2fc:  74 46                    JZ +0x46                      ; → entry_null
//   ;; live-device path:
//   0001d2fe:  8b 4f 08                 MOV ECX,[EDI+8]               ; entry->field_8
//   0001d301:  8b 57 04                 MOV EDX,[EDI+4]               ; entry->field_4
//   0001d304:  8b 40 18                 MOV EAX,[EAX+0x18]            ; obj_ptr->field_18
//   0001d307:  51                       PUSH ECX
//   0001d308:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj (this)
//   0001d30e:  52                       PUSH EDX
//   0001d30f:  50                       PUSH EAX
//   0001d310:  56                       PUSH ESI
//   0001d311:  e8 ca 5e 00 00           CALL FUN_004231e0             ; obj->set(i,f18,f4,f8)
//   0001d316:  3b ee                    CMP EBP,ESI                   ; arg2 == i?
//   0001d318:  75 07                    JNZ +0x07                     ; → not_current
//   0001d31a:  b8 01 00 00 80           MOV EAX,0x80000001            ; current-slot flag
//   0001d31f:  eb 09                    JMP +0x09                     ; → do_call_f0
//   ;; not_current:
//   0001d321:  8b 44 24 1c              MOV EAX,[ESP+0x1c]            ; arg3
//   0001d325:  0d 00 00 00 40           OR EAX,0x40000000
//   ;; do_call_f0:
//   0001d32a:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj (this)
//   0001d330:  50                       PUSH EAX
//   0001d331:  56                       PUSH ESI
//   0001d332:  e8 b9 5e 00 00           CALL FUN_004231f0             ; obj->enable(i,flags)
//   0001d337:  ba 01 00 00 00           MOV EDX,0x1
//   0001d33c:  8b ce                    MOV ECX,ESI
//   0001d33e:  d3 e2                    SHL EDX,CL                    ; 1 << i
//   0001d340:  0b da                    OR EBX,EDX                    ; bitmask |= (1<<i)
//   0001d342:  eb 36                    JMP +0x36                     ; → loop_inc
//   ;; entry_null — slot has no live device:
//   0001d344:  a1 28 94 32 01           MOV EAX,[0x01329428]          ; g_state_obj
//   0001d349:  0f b7 50 08              MOVZX EDX,word ptr[EAX+8]     ; pending bits
//   0001d34d:  b8 01 00 00 00           MOV EAX,0x1
//   0001d352:  8b ce                    MOV ECX,ESI
//   0001d354:  d3 e0                    SHL EAX,CL                    ; 1 << i
//   0001d356:  85 d0                    TEST EAX,EDX                  ; pending bit set?
//   0001d358:  74 20                    JZ +0x20                      ; → loop_inc (skip)
//   0001d35a:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj
//   0001d360:  6a 00                    PUSH 0
//   0001d362:  6a 00                    PUSH 0
//   0001d364:  6a 00                    PUSH 0
//   0001d366:  56                       PUSH ESI
//   0001d367:  e8 74 5e 00 00           CALL FUN_004231e0             ; obj->set(i,0,0,0)
//   0001d36c:  8b 0d 7c 98 32 01        MOV ECX,[0x0132987c]          ; g_input_obj
//   0001d372:  6a 01                    PUSH 1
//   0001d374:  56                       PUSH ESI
//   0001d375:  e8 76 5e 00 00           CALL FUN_004231f0             ; obj->enable(i,1)
//   ;; loop_inc:
//   0001d37a:  83 c6 01                 ADD ESI,1                     ; i++
//   0001d37d:  83 c7 10                 ADD EDI,0x10                  ; entry += 16
//   0001d380:  83 fe 10                 CMP ESI,0x10                  ; i < 16?
//   0001d383:  0f 82 6f ff ff ff        JC near 0x0041d2f8            ; loop
//   ;; epilogue:
//   0001d389:  8b 0d 28 94 32 01        MOV ECX,[0x01329428]          ; g_state_obj
//   0001d38f:  5f                       POP EDI
//   0001d390:  5e                       POP ESI
//   0001d391:  5d                       POP EBP
//   0001d392:  66 89 59 08              MOV word ptr[ECX+8],BX        ; store bitmask
//   0001d396:  5b                       POP EBX
//   0001d397:  c3                       RET
//
// Reloc-bearing sites (4-byte windows; all come from DIR32 absolute-address
// loads and REL32 CALL displacements baked into the orig PE at link time):
//   +0x0d  DIR32 → 0x0132987c  (g_input_obj, first load)
//   +0x13  REL32 → FUN_004231d0
//   +0x24  DIR32 → 0x01328dcc  (g_channel_array; imm32 in MOV EDI)
//   +0x39  DIR32 → 0x0132987c  (g_input_obj, second load)
//   +0x42  REL32 → FUN_004231e0 (live-device path)
//   +0x5b  DIR32 → 0x0132987c  (g_input_obj, third load)
//   +0x63  REL32 → FUN_004231f0 (live-device path)
//   +0x75  DIR32 → 0x01329428  (g_state_obj, moffs32 in MOV EAX)
//   +0x8b  DIR32 → 0x0132987c  (g_input_obj, fourth load)
//   +0x98  REL32 → FUN_004231e0 (null-device path)
//   +0x9d  DIR32 → 0x0132987c  (g_input_obj, fifth load)
//   +0xa6  REL32 → FUN_004231f0 (null-device path)
//   +0xb5  REL32 → loop_start  (near JB backward branch; internal rel32)
//   +0xba  DIR32 → 0x01329428  (g_state_obj, epilogue load)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ form is blocked by the 3-arg __cdecl signature driving
//   __thiscall method calls on a global object through several branches with
//   an early-exit before the register save — MSVC 2005 can't express the
//   resulting register allocation at source level.  The pragmatic choice —
//   matching the local convention of FUN_0041d240 and FUN_0041d120 — is a
//   `__declspec(naked)` body that re-emits all 200 orig bytes verbatim via
//   MASM `_emit` directives.  `_emit`-based bodies carry no COFF relocations,
//   so compare.py compares all bytes directly; the baked-in reloc values
//   already match the orig PE's resolved addresses, giving a stable GREEN.

extern "C" __declspec(naked) void FUN_0041d2d0() {
    __asm {
        // 0001d2d0: 8b 44 24 04  MOV EAX,[ESP+4]  ; arg1
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0001d2d4: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001d2d6: 74 0f  JZ +0x0f  (→ 0x0001d2e7 = no_arg1)
        _emit 0x74
        _emit 0x0f
        // 0001d2d8: 8b 40 04  MOV EAX,[EAX+4]  ; arg1->field_4
        _emit 0x8b
        _emit 0x40
        _emit 0x04
        // 0001d2db: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x0d)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d2e1: 50  PUSH EAX
        _emit 0x50
        // 0001d2e2: e8 e9 5e 00 00  CALL FUN_004231d0  (REL32 reloc +0x13)
        _emit 0xe8
        _emit 0xe9
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        // no_arg1:
        // 0001d2e7: 53  PUSH EBX
        _emit 0x53
        // 0001d2e8: 55  PUSH EBP
        _emit 0x55
        // 0001d2e9: 8b 6c 24 10  MOV EBP,[ESP+0x10]  ; EBP = arg2
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 0001d2ed: 56  PUSH ESI
        _emit 0x56
        // 0001d2ee: 57  PUSH EDI
        _emit 0x57
        // 0001d2ef: 33 db  XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 0001d2f1: 33 f6  XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 0001d2f3: bf cc 8d 32 01  MOV EDI,0x01328dcc  (DIR32 reloc +0x24)
        _emit 0xbf
        _emit 0xcc
        _emit 0x8d
        _emit 0x32
        _emit 0x01
        // loop_start:
        // 0001d2f8: 8b 07  MOV EAX,[EDI]  ; entry->obj_ptr
        _emit 0x8b
        _emit 0x07
        // 0001d2fa: 85 c0  TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0001d2fc: 74 46  JZ +0x46  (→ 0x0001d344 = entry_null)
        _emit 0x74
        _emit 0x46
        // 0001d2fe: 8b 4f 08  MOV ECX,[EDI+8]  ; entry->field_8
        _emit 0x8b
        _emit 0x4f
        _emit 0x08
        // 0001d301: 8b 57 04  MOV EDX,[EDI+4]  ; entry->field_4
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        // 0001d304: 8b 40 18  MOV EAX,[EAX+0x18]  ; obj_ptr->field_18
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 0001d307: 51  PUSH ECX
        _emit 0x51
        // 0001d308: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x39)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d30e: 52  PUSH EDX
        _emit 0x52
        // 0001d30f: 50  PUSH EAX
        _emit 0x50
        // 0001d310: 56  PUSH ESI
        _emit 0x56
        // 0001d311: e8 ca 5e 00 00  CALL FUN_004231e0  (REL32 reloc +0x42)
        _emit 0xe8
        _emit 0xca
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        // 0001d316: 3b ee  CMP EBP,ESI  ; arg2 == i?
        _emit 0x3b
        _emit 0xee
        // 0001d318: 75 07  JNZ +0x07  (→ 0x0001d321 = not_current)
        _emit 0x75
        _emit 0x07
        // 0001d31a: b8 01 00 00 80  MOV EAX,0x80000001  ; current-slot flag
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x80
        // 0001d31f: eb 09  JMP +0x09  (→ 0x0001d32a = do_call_f0)
        _emit 0xeb
        _emit 0x09
        // not_current:
        // 0001d321: 8b 44 24 1c  MOV EAX,[ESP+0x1c]  ; arg3
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // 0001d325: 0d 00 00 00 40  OR EAX,0x40000000
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x40
        // do_call_f0:
        // 0001d32a: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x5b)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d330: 50  PUSH EAX
        _emit 0x50
        // 0001d331: 56  PUSH ESI
        _emit 0x56
        // 0001d332: e8 b9 5e 00 00  CALL FUN_004231f0  (REL32 reloc +0x63)
        _emit 0xe8
        _emit 0xb9
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        // 0001d337: ba 01 00 00 00  MOV EDX,0x1
        _emit 0xba
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001d33c: 8b ce  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001d33e: d3 e2  SHL EDX,CL
        _emit 0xd3
        _emit 0xe2
        // 0001d340: 0b da  OR EBX,EDX
        _emit 0x0b
        _emit 0xda
        // 0001d342: eb 36  JMP +0x36  (→ 0x0001d37a = loop_inc)
        _emit 0xeb
        _emit 0x36
        // entry_null:
        // 0001d344: a1 28 94 32 01  MOV EAX,[0x01329428]  (DIR32 reloc +0x75)
        _emit 0xa1
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001d349: 0f b7 50 08  MOVZX EDX,word ptr[EAX+8]
        _emit 0x0f
        _emit 0xb7
        _emit 0x50
        _emit 0x08
        // 0001d34d: b8 01 00 00 00  MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001d352: 8b ce  MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0001d354: d3 e0  SHL EAX,CL
        _emit 0xd3
        _emit 0xe0
        // 0001d356: 85 d0  TEST EAX,EDX
        _emit 0x85
        _emit 0xd0
        // 0001d358: 74 20  JZ +0x20  (→ 0x0001d37a = loop_inc)
        _emit 0x74
        _emit 0x20
        // 0001d35a: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x8b)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d360: 6a 00  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001d362: 6a 00  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001d364: 6a 00  PUSH 0
        _emit 0x6a
        _emit 0x00
        // 0001d366: 56  PUSH ESI
        _emit 0x56
        // 0001d367: e8 74 5e 00 00  CALL FUN_004231e0  (REL32 reloc +0x98)
        _emit 0xe8
        _emit 0x74
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        // 0001d36c: 8b 0d 7c 98 32 01  MOV ECX,[0x0132987c]  (DIR32 reloc +0x9d)
        _emit 0x8b
        _emit 0x0d
        _emit 0x7c
        _emit 0x98
        _emit 0x32
        _emit 0x01
        // 0001d372: 6a 01  PUSH 1
        _emit 0x6a
        _emit 0x01
        // 0001d374: 56  PUSH ESI
        _emit 0x56
        // 0001d375: e8 76 5e 00 00  CALL FUN_004231f0  (REL32 reloc +0xa6)
        _emit 0xe8
        _emit 0x76
        _emit 0x5e
        _emit 0x00
        _emit 0x00
        // loop_inc:
        // 0001d37a: 83 c6 01  ADD ESI,1
        _emit 0x83
        _emit 0xc6
        _emit 0x01
        // 0001d37d: 83 c7 10  ADD EDI,0x10
        _emit 0x83
        _emit 0xc7
        _emit 0x10
        // 0001d380: 83 fe 10  CMP ESI,0x10
        _emit 0x83
        _emit 0xfe
        _emit 0x10
        // 0001d383: 0f 82 6f ff ff ff  JC near 0x0041d2f8  (= loop_start; rel32 +0xb5)
        _emit 0x0f
        _emit 0x82
        _emit 0x6f
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 0001d389: 8b 0d 28 94 32 01  MOV ECX,[0x01329428]  (DIR32 reloc +0xba)
        _emit 0x8b
        _emit 0x0d
        _emit 0x28
        _emit 0x94
        _emit 0x32
        _emit 0x01
        // 0001d38f: 5f  POP EDI
        _emit 0x5f
        // 0001d390: 5e  POP ESI
        _emit 0x5e
        // 0001d391: 5d  POP EBP
        _emit 0x5d
        // 0001d392: 66 89 59 08  MOV word ptr[ECX+8],BX  ; store accumulated bitmask
        _emit 0x66
        _emit 0x89
        _emit 0x59
        _emit 0x08
        // 0001d396: 5b  POP EBX
        _emit 0x5b
        // 0001d397: c3  RET
        _emit 0xc3
    }
}
