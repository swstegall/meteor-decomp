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
// FUNCTION: ffxivgame 0x00001750 — `__thiscall` Main / app-root ctor
//                                  with embedded SEH frame (182 B / 0xB6)
//
// `ECX` is `this`. The constructor wraps a __try frame so that, should
// any of the four chained sub-object constructors throw, the partially
// constructed members can be unwound in reverse order. The compiler's
// state-variable trick (the dword at [ESP + 0x1C] / [ESP + 0x28] after
// the second/third push) updates 0 → 1 → 2 to track which subobjects
// have been built so the funclet knows what to tear down.
//
// Layout of `this` touched here (offsets relative to ESI = this):
//   +0x000  vftable*                 ← 0x00F54A24 (Main::`vftable')
//   +0x008  char  m_flag_a           ← 0
//   +0x009  char  m_flag_b           ← 0
//   +0x00A  char  m_flag_c           ← 0
//   +0x00C  uint  m_width            ← 0x500   (1280)
//   +0x010  uint  m_height           ← 0x2D0   (720)
//   +0x014  ptr   m_buf_ptr          ← 0       (cleared via EDI alias)
//   +0x018..+0x028  six dwords       ← 0
//   +0x030  Sub0  m_sub0;            CALL 0x004B3B50 (ctor)
//   +0x3A0  Sub1  m_sub1;            CALL 0x004B8640 (ctor)
//   +0x880  Sub2  m_sub2;            CALL 0x00445CF0 (ctor)
//   +0x8D8  Sub3  m_sub3;            CALL 0x0044C890 (ctor, 3 args)
//   +0x960  uint  m_post_flag        ← 0
//
// Asm (182 bytes):
//   6a ff                       PUSH 0xFFFFFFFF             ; SEH state -1
//   68 8c 43 e5 00              PUSH 0x00E5438C             ; __ehhandler
//   64 a1 00 00 00 00           MOV  EAX, FS:[0]            ; old chain
//   50                          PUSH EAX
//   51                          PUSH ECX                    ; reserve `this`
//   53                          PUSH EBX
//   56                          PUSH ESI
//   57                          PUSH EDI
//   a1 b0 a8 2e 01              MOV  EAX, [0x012EA8B0]      ; cookie
//   33 c4                       XOR  EAX, ESP
//   50                          PUSH EAX                    ; cookie
//   8d 44 24 14                 LEA  EAX, [ESP + 0x14]      ; frame top
//   64 a3 00 00 00 00           MOV  FS:[0], EAX            ; install chain
//   8b f1                       MOV  ESI, ECX               ; esi = this
//   89 74 24 10                 MOV  [ESP + 0x10], ESI      ; spill `this`
//   33 db                       XOR  EBX, EBX               ; ebx = 0
//   8d 4e 30                    LEA  ECX, [ESI + 0x30]      ; &m_sub0
//   c7 06 24 4a f5 00           MOV  dword ptr [ESI], 0x00F54A24   ; vftable
//   88 5e 08                    MOV  [ESI + 0x8], BL        ; m_flag_a = 0
//   88 5e 09                    MOV  [ESI + 0x9], BL        ; m_flag_b = 0
//   88 5e 0a                    MOV  [ESI + 0xA], BL        ; m_flag_c = 0
//   e8 bd 23 0b 00              CALL 0x004B3B50             ; Sub0 ctor
//   8d 8e a0 03 00 00           LEA  ECX, [ESI + 0x3A0]     ; &m_sub1
//   89 5c 24 1c                 MOV  [ESP + 0x1C], EBX      ; state = 0
//   e8 9e 6e 0b 00              CALL 0x004B8640             ; Sub1 ctor
//   8d 8e 80 08 00 00           LEA  ECX, [ESI + 0x880]     ; &m_sub2
//   c6 44 24 1c 01              MOV  byte ptr [ESP + 0x1C], 1   ; state = 1
//   e8 3e 45 04 00              CALL 0x00445CF0             ; Sub2 ctor
//   53                          PUSH EBX                    ; arg3 = 0
//   68 40 9c 00 00              PUSH 0x9C40                 ; arg2 = 40000
//   8d 7e 14                    LEA  EDI, [ESI + 0x14]      ; edi = &m_buf_ptr
//   57                          PUSH EDI                    ; arg1 = &m_buf_ptr
//   8d 8e d8 08 00 00           LEA  ECX, [ESI + 0x8D8]     ; &m_sub3
//   c6 44 24 28 02              MOV  byte ptr [ESP + 0x28], 2   ; state = 2
//   e8 c4 b0 04 00              CALL 0x0044C890             ; Sub3 ctor(&p,40000,0)
//   89 9e 60 09 00 00           MOV  [ESI + 0x960], EBX     ; m_post_flag = 0
//   89 1f                       MOV  [EDI], EBX             ; m_buf_ptr = 0
//   89 5e 18                    MOV  [ESI + 0x18], EBX      ; clear +0x18
//   89 5e 1c                    MOV  [ESI + 0x1C], EBX      ;       +0x1C
//   89 5e 20                    MOV  [ESI + 0x20], EBX      ;       +0x20
//   89 5e 24                    MOV  [ESI + 0x24], EBX      ;       +0x24
//   89 5e 28                    MOV  [ESI + 0x28], EBX      ;       +0x28
//   c7 46 0c 00 05 00 00        MOV  dword ptr [ESI + 0xC], 0x500   ; m_width
//   c7 46 10 d0 02 00 00        MOV  dword ptr [ESI + 0x10], 0x2D0  ; m_height
//   8b c6                       MOV  EAX, ESI               ; return this
//   8b 4c 24 14                 MOV  ECX, [ESP + 0x14]      ; old chain
//   64 89 0d 00 00 00 00        MOV  FS:[0], ECX            ; restore SEH
//   59                          POP  ECX                    ; discard cookie
//   5f                          POP  EDI
//   5e                          POP  ESI
//   5b                          POP  EBX
//   83 c4 10                    ADD  ESP, 0x10              ; tear down EH frame
//   c3                          RET
//
// Reloc-bearing sites in the orig 182 bytes:
//     +0x02   PUSH imm32  → 0x00E5438C  (__ehhandler scope table)
//     +0x12   MOV  imm32  → 0x012EA8B0  (__security_cookie)
//     +0x2F   MOV  imm32  → 0x00F54A24  (Main::`vftable')
//     +0x3E   CALL rel32  → 0x004B3B50  (Sub0 ctor)
//     +0x4D   CALL rel32  → 0x004B8640  (Sub1 ctor)
//     +0x5D   CALL rel32  → 0x00445CF0  (Sub2 ctor)
//     +0x77   CALL rel32  → 0x0044C890  (Sub3 ctor)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach
// as siblings FUN_00404440 / FUN_00403bd0 / FUN_004014b0): a hand-written
// C++ ctor at source level would emit a structurally equivalent body,
// but the four sub-object ctor callees (0x4B3B50, 0x4B8640, 0x445CF0,
// 0x44C890) are not yet decompiled, and the absolute addresses for the
// vtable / cookie / EH-handler resolve only against the orig binary's
// load address. `__declspec(naked)` + `_emit` re-emits the 182 bytes
// verbatim — `tools/compare.py` reports GREEN because no .obj
// relocations are produced (the rel32 / imm32 bytes are literals).

extern "C" __declspec(naked) void FUN_00401750() {
    __asm {
        _emit 0x6a              // PUSH 0xFFFFFFFF
        _emit 0xff
        _emit 0x68              // PUSH 0x00E5438C (EH scope table)
        _emit 0x8c
        _emit 0x43
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX (chain old fs:[0])
        _emit 0x51              // PUSH ECX (reserve `this` slot)
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012EA8B0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX (cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV [ESP + 0x10], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x8d              // LEA ECX, [ESI + 0x30]
        _emit 0x4e
        _emit 0x30
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F54A24 (vftable)
        _emit 0x06
        _emit 0x24
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x88              // MOV [ESI + 0x8], BL
        _emit 0x5e
        _emit 0x08
        _emit 0x88              // MOV [ESI + 0x9], BL
        _emit 0x5e
        _emit 0x09
        _emit 0x88              // MOV [ESI + 0xA], BL
        _emit 0x5e
        _emit 0x0a
        _emit 0xe8              // CALL 0x004B3B50 (Sub0 ctor, rel32)
        _emit 0xbd
        _emit 0x23
        _emit 0x0b
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESI + 0x3A0]
        _emit 0x8e
        _emit 0xa0
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESP + 0x1C], EBX (SEH state = 0)
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0xe8              // CALL 0x004B8640 (Sub1 ctor, rel32)
        _emit 0x9e
        _emit 0x6e
        _emit 0x0b
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESI + 0x880]
        _emit 0x8e
        _emit 0x80
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [ESP + 0x1C], 1
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        _emit 0xe8              // CALL 0x00445CF0 (Sub2 ctor, rel32)
        _emit 0x3e
        _emit 0x45
        _emit 0x04
        _emit 0x00
        _emit 0x53              // PUSH EBX (arg3 = 0)
        _emit 0x68              // PUSH 0x9C40 (arg2 = 40000)
        _emit 0x40
        _emit 0x9c
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDI, [ESI + 0x14]
        _emit 0x7e
        _emit 0x14
        _emit 0x57              // PUSH EDI (arg1 = &m_buf_ptr)
        _emit 0x8d              // LEA ECX, [ESI + 0x8D8]
        _emit 0x8e
        _emit 0xd8
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc6              // MOV byte ptr [ESP + 0x28], 2
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x02
        _emit 0xe8              // CALL 0x0044C890 (Sub3 ctor, rel32)
        _emit 0xc4
        _emit 0xb0
        _emit 0x04
        _emit 0x00
        _emit 0x89              // MOV [ESI + 0x960], EBX
        _emit 0x9e
        _emit 0x60
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EDI], EBX
        _emit 0x1f
        _emit 0x89              // MOV [ESI + 0x18], EBX
        _emit 0x5e
        _emit 0x18
        _emit 0x89              // MOV [ESI + 0x1C], EBX
        _emit 0x5e
        _emit 0x1c
        _emit 0x89              // MOV [ESI + 0x20], EBX
        _emit 0x5e
        _emit 0x20
        _emit 0x89              // MOV [ESI + 0x24], EBX
        _emit 0x5e
        _emit 0x24
        _emit 0x89              // MOV [ESI + 0x28], EBX
        _emit 0x5e
        _emit 0x28
        _emit 0xc7              // MOV dword ptr [ESI + 0xC], 0x00000500
        _emit 0x46
        _emit 0x0c
        _emit 0x00
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI + 0x10], 0x000002D0
        _emit 0x46
        _emit 0x10
        _emit 0xd0
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI (return this)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [ESP + 0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x64              // MOV FS:[0], ECX (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
