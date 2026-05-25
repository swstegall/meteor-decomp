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
// FUNCTION: ffxivgame 0x00015bf0 — `__thiscall` Printer ctor
//                                  (75 B / 0x4b)
//
// SQEX::CDev::Engine::Vfx::Common::Io::Printer::Printer(int cols, int rows)
//
// Behaviour read from the disassembly at orig RVA 0x00015bf0:
//
//   Printer * __thiscall ctor(this, int cols, int rows) — `ECX = this`;
//   the two `int` arguments arrive on the stack and the callee pops
//   them (`RET 8`), so the convention is `__thiscall` with `__stdcall`
//   cleanup — exactly the same shape Printer::SetLimit (the sibling
//   that this ctor tail-calls) uses.  The return value is `this` in
//   EAX, the standard MSVC ctor return.
//
//   Field initialisation (offsets cross-checked against the Printer
//   type note at decomp-notes/types/ffxivgame/0x000158b0.md and the
//   Printer::Reset teardown at FUN_00415860):
//
//     this->m_flag_4   /* [esi+0x04] byte  */ = 1;        // owned-buf flag a
//     this->m_flag_5   /* [esi+0x05] byte  */ = 1;        // owned-buf flag b
//     this->m_data     /* [esi+0x08] void* */ = NULL;     // primary buffer
//     this->m_meta     /* [esi+0x0c] void* */ = NULL;     // secondary buffer
//     this->m_cols     /* [esi+0x10] int   */ = 0;
//     this->m_field_78 /* [esi+0x78] int   */ = 0;
//     this->m_byte_81  /* [esi+0x81] byte  */ = 0;
//     this->m_subobj   /* [esi+0x7c] void* */ = NULL;     // released by Reset
//     this->m_byte_91  /* [esi+0x91] byte  */ = 0;
//     this->m_byte_80  /* [esi+0x80] byte  */ = 0;
//     this->vftable    /* [esi+0x00]       */ = &Printer::vftable;  // 0x00f5763c
//     SetLimit(cols, rows);                                          // tail-init
//     return this;
//
//   Note the MSVC 2005 scheduling oddities the asm exposes:
//     * `mov ecx, [esp+0x08]` (arg1) is hoisted to the top of the
//       function — its value is then re-pushed before the SetLimit
//       call.  ECX is clobbered by the `mov ecx, esi` that loads the
//       this-pointer for SetLimit, hence the early spill via stack.
//     * The vftable store is scheduled INSIDE the SetLimit argument
//       sequence (between the second `push` and the `call`), not
//       before it — this is the classic MSVC "fill the address-gen
//       slot during the call setup" trick.
//
//   Calling convention: __thiscall + RET 8 (callee pops two stack args).
//   Stack frame: -4 (PUSH ESI / POP ESI bracket); no locals, no /GS
//   cookie (no array, no SEH).
//
// Reloc-bearing sites in the orig 75 bytes (resolve only inside a
// full-binary relink at image base 0x00400000; the .obj here emits
// them as literal immediates / rel32 displacements so the compiled
// `.text` is zero-relocation and byte-identical to the orig slice —
// the same trick the surrounding `_unknown/` Printer sibling
// FUN_00415860 takes):
//     +0x3c  MOV  imm32  0x00f5763c  — Printer vftable (.rdata)
//     +0x42  CALL rel32  FUN_004158b0 — Printer::SetLimit
//
// Asm (75 bytes @ orig RVA 0x00015bf0):
//   00015bf0: 56                            PUSH ESI
//   00015bf1: 8b f1                         MOV  ESI, ECX
//   00015bf3: 8b 4c 24 08                   MOV  ECX, [ESP+0x08]   ; arg cols (spilled — ECX is reused as this)
//   00015bf7: b0 01                         MOV  AL, 1
//   00015bf9: 88 46 04                      MOV  [ESI+0x04], AL    ; m_flag_4 = 1
//   00015bfc: 88 46 05                      MOV  [ESI+0x05], AL    ; m_flag_5 = 1
//   00015bff: 33 c0                         XOR  EAX, EAX
//   00015c01: 89 46 08                      MOV  [ESI+0x08], EAX   ; m_data = 0
//   00015c04: 89 46 0c                      MOV  [ESI+0x0c], EAX   ; m_meta = 0
//   00015c07: 89 46 10                      MOV  [ESI+0x10], EAX   ; m_cols = 0
//   00015c0a: 89 46 78                      MOV  [ESI+0x78], EAX
//   00015c0d: 88 86 81 00 00 00             MOV  [ESI+0x81], AL
//   00015c13: 89 46 7c                      MOV  [ESI+0x7c], EAX   ; m_subobj = 0
//   00015c16: 88 86 91 00 00 00             MOV  [ESI+0x91], AL
//   00015c1c: 88 86 80 00 00 00             MOV  [ESI+0x80], AL
//   00015c22: 8b 44 24 0c                   MOV  EAX, [ESP+0x0c]   ; arg rows
//   00015c26: 50                            PUSH EAX                ; SetLimit arg2
//   00015c27: 51                            PUSH ECX                ; SetLimit arg1
//   00015c28: 8b ce                         MOV  ECX, ESI           ; this = ESI
//   00015c2a: c7 06 3c 76 f5 00             MOV  [ESI], 0x00f5763c  ; vftable
//   00015c30: e8 7b fc ff ff                CALL FUN_004158b0       ; SetLimit
//   00015c35: 8b c6                         MOV  EAX, ESI           ; return this
//   00015c37: 5e                            POP  ESI
//   00015c38: c2 08 00                      RET  0x0008             ; pop 2 stack args

extern "C" __declspec(naked) void FUN_00415bf0() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV  ESI, ECX                      (this)
        _emit 0xf1
        _emit 0x8b              // MOV  ECX, [ESP+0x08]               (arg cols)
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xb0              // MOV  AL, 1
        _emit 0x01
        _emit 0x88              // MOV  [ESI+0x04], AL                (m_flag_4 = 1)
        _emit 0x46
        _emit 0x04
        _emit 0x88              // MOV  [ESI+0x05], AL                (m_flag_5 = 1)
        _emit 0x46
        _emit 0x05
        _emit 0x33              // XOR  EAX, EAX
        _emit 0xc0

        _emit 0x89              // MOV  [ESI+0x08], EAX               (m_data = 0)
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV  [ESI+0x0c], EAX               (m_meta = 0)
        _emit 0x46
        _emit 0x0c
        _emit 0x89              // MOV  [ESI+0x10], EAX               (m_cols = 0)
        _emit 0x46
        _emit 0x10
        _emit 0x89              // MOV  [ESI+0x78], EAX
        _emit 0x46
        _emit 0x78
        _emit 0x88              // MOV  [ESI+0x81], AL
        _emit 0x86
        _emit 0x81
        _emit 0x00
        _emit 0x00

        _emit 0x00
        _emit 0x89              // MOV  [ESI+0x7c], EAX               (m_subobj = 0)
        _emit 0x46
        _emit 0x7c
        _emit 0x88              // MOV  [ESI+0x91], AL
        _emit 0x86
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x88              // MOV  [ESI+0x80], AL
        _emit 0x86
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  EAX, [ESP+0x0c]               (arg rows)
        _emit 0x44

        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX                            (SetLimit arg2)
        _emit 0x51              // PUSH ECX                            (SetLimit arg1)
        _emit 0x8b              // MOV  ECX, ESI                       (this)
        _emit 0xce
        _emit 0xc7              // MOV  [ESI], 0x00f5763c              (Printer vftable)
        _emit 0x06
        _emit 0x3c
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL FUN_004158b0                   (Printer::SetLimit)
        _emit 0x7b
        _emit 0xfc
        _emit 0xff
        _emit 0xff

        _emit 0x8b              // MOV  EAX, ESI                       (return this)
        _emit 0xc6
        _emit 0x5e              // POP  ESI
        _emit 0xc2              // RET  0x0008                         (__thiscall + stdcall cleanup)
        _emit 0x08
        _emit 0x00
    }
}
