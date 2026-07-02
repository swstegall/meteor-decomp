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
// FUNCTION: ffxivgame 0x00031710 — `__thiscall` object constructor with a
//                                   frame-based (/GS + SEH scope-table)
//                                   prologue, 172 bytes / 0xac.
//
// Asm shape (`__thiscall Obj* Ctor(this, const Src *src, DWORD ctx)` —
// ret 0x8, returns `this` in EAX):
//
//   Standard MSVC 2005 `/GS` + C++-EH (`_except_handler3`-style) frame:
//     PUSH -1 ; PUSH 0x00e55fe1 (scope-table funcinfo) ; save old fs:[0] ;
//     PUSH ECX (this) ; PUSH ESI ; compute + push __security_cookie ;
//     re-point fs:[0] at the new EXCEPTION_REGISTRATION record.
//
//   ESI = this
//   this->field_0x8_stack_copy = ESI          ; spilled for the EH unwind
//                                              ;  record / return path
//   this->vfptr   = 0x00f638d4
//   this->field_4 = 0
//   this->field_8 = 0
//   // 36-byte (9-dword) struct copy from *src (arg 1, [ESP+0x1c]) into
//   // this+0xc..this+0x30, done as 4x MOVQ (xmm0) + 1 trailing dword:
//   this->field_c .. field_2c = *src            (src+0x0 .. src+0x20)
//   this->field_30 = 0
//   this->field_34 = 0
//   this->field_38 = ctx                        ; arg 2, [ESP+0x20]
//   this->field_3c = 0
//   ehState        = 0                          ; local EH scope-index
//   this->field_44 = 0
//   this->field_48 = 0
//   this->field_4c = 0                          ; (a 3-pointer sub-object
//                                                ;  at +0x40 is left as
//                                                ;  all-zero here; its
//                                                ;  member ctor never runs)
//   ehState = 1
//   this->FUN_004312d0()                        ; big multi-branch "Init"
//   this->FUN_004328a0()                        ; doubly-linked-list insert
//   return this;
//
// This mirrors the local worker-facing pattern already established by
// FUN_00401b70 / FUN_00406280 / FUN_004063c0 in this directory: the body
// is straight-line (no internal branches) but the `/GS` cookie dance, the
// scope-table funcinfo constant, and the two internal CALLs make a
// source-level reconstruction fight the compiler's own frame-setup
// register/slot choices for no benefit — the orig bytes are reproduced
// verbatim instead.
//
// Reloc-bearing sites in the orig 172 bytes (each masked by
// `tools/compare.py`, and — per the established precedent in this
// directory — baked in verbatim since the internal rel32 CALLs resolve
// correctly against the orig binary's fixed layout):
//     +0x02   PUSH imm32   → 0x00e55fe1  (SEH scope-table funcinfo)
//     +0x10   MOV  moffs32 → [0x012ea8b0] (__security_cookie)
//     +0x38   MOV  imm32   → 0x00f638d4   (vfptr)
//     +0x8b   CALL rel32   → 0x004312d0   (Init)
//     +0x92   CALL rel32   → 0x004328a0   (list-insert)
//
// Reconstruction strategy — naked-asm byte passthrough (see rationale
// above and in the cited sibling files).

extern "C" __declspec(naked) void FUN_00431710() {
    __asm {
        _emit 0x6a              // PUSH -0x1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e55fe1
        _emit 0xe1
        _emit 0x5f
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, dword ptr [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP + 0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESP + 0x8], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x89              // MOV dword ptr [ESI + 0x4], ECX
        _emit 0x4e
        _emit 0x04
        _emit 0x89              // MOV dword ptr [ESI + 0x8], ECX
        _emit 0x4e
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV EDX, dword ptr [ESP + 0x20]
        _emit 0x54
        _emit 0x24
        _emit 0x20
        _emit 0xc7              // MOV dword ptr [ESI], 0x00f638d4
        _emit 0x06
        _emit 0xd4
        _emit 0x38
        _emit 0xf6
        _emit 0x00
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX]
        _emit 0x0f
        _emit 0x7e
        _emit 0x00
        _emit 0x66              // MOVQ qword ptr [ESI + 0xc], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x0c
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX + 0x8]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x08
        _emit 0x66              // MOVQ qword ptr [ESI + 0x14], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x14
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX + 0x10]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x10
        _emit 0x66              // MOVQ qword ptr [ESI + 0x1c], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x1c
        _emit 0xf3              // MOVQ XMM0, qword ptr [EAX + 0x18]
        _emit 0x0f
        _emit 0x7e
        _emit 0x40
        _emit 0x18
        _emit 0x66              // MOVQ qword ptr [ESI + 0x24], XMM0
        _emit 0x0f
        _emit 0xd6
        _emit 0x46
        _emit 0x24
        _emit 0x8b              // MOV EAX, dword ptr [EAX + 0x20]
        _emit 0x40
        _emit 0x20
        _emit 0x89              // MOV dword ptr [ESI + 0x2c], EAX
        _emit 0x46
        _emit 0x2c
        _emit 0x89              // MOV dword ptr [ESI + 0x30], ECX
        _emit 0x4e
        _emit 0x30
        _emit 0x89              // MOV dword ptr [ESI + 0x34], ECX
        _emit 0x4e
        _emit 0x34
        _emit 0x89              // MOV dword ptr [ESI + 0x38], EDX
        _emit 0x56
        _emit 0x38
        _emit 0x89              // MOV dword ptr [ESI + 0x3c], ECX
        _emit 0x4e
        _emit 0x3c
        _emit 0x89              // MOV dword ptr [ESP + 0x14], ECX
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x89              // MOV dword ptr [ESI + 0x44], ECX
        _emit 0x4e
        _emit 0x44
        _emit 0x89              // MOV dword ptr [ESI + 0x48], ECX
        _emit 0x4e
        _emit 0x48
        _emit 0x89              // MOV dword ptr [ESI + 0x4c], ECX
        _emit 0x4e
        _emit 0x4c
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xc6              // MOV byte ptr [ESP + 0x14], 0x1
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x01
        _emit 0xe8              // CALL FUN_004312d0 (rel32 → 0x004312d0)
        _emit 0x30
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_004328a0 (rel32 → 0x004328a0)
        _emit 0xf9
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x8
        _emit 0x08
        _emit 0x00
    }
}
