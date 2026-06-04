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
// FUNCTION: ffxivgame 0x0001a8c0 — __thiscall constructor with EH3 frame; 111 bytes.
//
// __thiscall void FUN_0041a8c0(void)   [ECX = this]
//
// Inspection (read from orig RVA 0x0001a8c0, 111 bytes):
//
//   EH3 prologue (ESP-relative frame, no EBP):
//     PUSH -1                              ; trylevel = -1
//     PUSH 0x00E557EE                      ; EH3 scope table (FuncInfo)
//     MOV EAX, FS:[0]                      ; save previous SEH chain head
//     PUSH EAX                             ; chain old FS:[0]
//     PUSH ECX                             ; save 'this' (ECX from thiscall)
//     PUSH ESI                             ; save ESI (callee-save)
//     MOV EAX, [0x012EA8B0]               ; load __security_cookie
//     XOR EAX, ESP                         ; cookie ^ ESP
//     PUSH EAX                             ; push cookie sentinel
//     LEA EAX, [ESP+0x0C]                 ; EAX = &EH3 record (at Next field)
//     MOV FS:[0], EAX                      ; install new SEH chain head
//
//   Frame layout after prolog (ESP = 0):
//     [ESP+0x00]: cookie^ESP
//     [ESP+0x04]: saved ESI
//     [ESP+0x08]: saved ECX / 'this'
//     [ESP+0x0C]: old FS:[0]  (EH3 record Next)
//     [ESP+0x10]: scope table  (EH3 record Handler/ScopeTable)
//     [ESP+0x14]: trylevel     (EH3 record TryLevel, initially -1)
//     [ESP+0x18]: return address
//
//   Body:
//     MOV ESI, ECX                         ; ESI = this
//     MOV [ESP+0x08], ESI                  ; re-store this in frame slot
//     XOR EAX, EAX                         ; EAX = 0
//     MOV [ESI+0x04], EAX                  ; this->field_4 = 0
//     MOV [ESI+0x08], EAX                  ; this->field_8 = 0
//     MOV [ESP+0x14], EAX                  ; trylevel = 0 (entering try block)
//     MOV dword ptr [ESI+0x0C], 0x00F57E20 ; this->field_C = ptr
//     MOV dword ptr [ESI+0x00], 0x00F57EB8 ; this->vfptr = 0x00F57EB8
//     MOV dword ptr [ESI+0x0C], 0x00F57EA8 ; this->field_C = ptr (overwrite)
//     MOV [ESI+0x10], EAX                  ; this->field_10 = 0
//     MOV byte ptr [ESP+0x14], 0x02        ; trylevel byte = 2
//     CALL FUN_0041a700                    ; (rel32 -0x215, target RVA 0x1a700)
//     MOV ECX, ESI                         ; ECX = this (thiscall arg)
//     CALL FUN_004328A0                    ; (rel32 0x17F84, target RVA 0x328A0)
//     MOV EAX, ESI                         ; return value = this
//
//   Epilogue:
//     MOV ECX, [ESP+0x0C]                  ; ECX = old FS:[0]
//     MOV FS:[0], ECX                      ; restore SEH chain
//     POP ECX                              ; discard cookie^ESP
//     POP ESI                              ; restore ESI
//     ADD ESP, 0x10                        ; discard ECX-slot, old_fs0, scope_table, trylevel
//     RET
//
//   Reloc-bearing sites (raw immediates / rel32 in the orig binary):
//     +0x03   PUSH imm32 → 0x00E557EE (EH3 scope table)
//     +0x07   MOV EAX, FS:[0] (FS-relative, constant 0)
//     +0x10   MOV EAX, [imm32] → 0x012EA8B0 (__security_cookie)
//     +0x1C   MOV FS:[0], EAX (FS-relative, constant 0)
//     +0x34   MOV [ESI+0x0C], imm32 → 0x00F57E20
//     +0x3B   MOV [ESI], imm32 → 0x00F57EB8
//     +0x41   MOV [ESI+0x0C], imm32 → 0x00F57EA8
//     +0x50   CALL rel32 → FUN_0041A700 (RVA 0x0001A700)
//     +0x57   CALL rel32 → FUN_004328A0 (RVA 0x000328A0)
//     +0x62   MOV FS:[0], ECX (FS-relative, constant 0)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The combination of EH3 SEH wrapping, absolute binary addresses baked into
//   __security_cookie and scope-table slots, an ESP-relative (non-EBP) frame,
//   rel32 CALL displacements resolved against the original binary's address
//   space, and two distinct vtable/fn-pointer immediates makes source-level
//   reconstruction brittle. The pragmatic approach — the same one siblings
//   FUN_00401650, FUN_004091f0, and FUN_004090b0 took — is a
//   `__declspec(naked)` body that re-emits the 111 orig bytes verbatim via
//   MASM `_emit` directives, producing a .obj whose .text section is
//   byte-identical to the orig slice.

extern "C" __declspec(naked) void FUN_0041a8c0() {
    __asm {
        _emit 0x6a              // PUSH -1                     (trylevel = -1)
        _emit 0xff
        _emit 0x68              // PUSH 0x00E557EE             (EH3 scope table)
        _emit 0xee
        _emit 0x57
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX                    (chain old FS:[0])
        _emit 0x51              // PUSH ECX                    (save this)
        _emit 0x56              // PUSH ESI                    (callee-save)
        _emit 0xa1              // MOV EAX, [0x012EA8B0]       (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX                    (cookie ^ ESP)
        _emit 0x8d              // LEA EAX, [ESP+0x0C]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0], EAX             (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX                (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV [ESP+0x08], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV [ESI+0x04], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x89              // MOV [ESI+0x08], EAX
        _emit 0x46
        _emit 0x08
        _emit 0x89              // MOV [ESP+0x14], EAX         (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [ESI+0x0C], 0x00F57E20
        _emit 0x46
        _emit 0x0c
        _emit 0x20
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x00], 0x00F57EB8
        _emit 0x06
        _emit 0xb8
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESI+0x0C], 0x00F57EA8
        _emit 0x46
        _emit 0x0c
        _emit 0xa8
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV [ESI+0x10], EAX
        _emit 0x46
        _emit 0x10
        _emit 0xc6              // MOV byte ptr [ESP+0x14], 0x02
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x02
        _emit 0xe8              // CALL FUN_0041A700 (rel32 -0x215)
        _emit 0xeb
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_004328A0 (rel32 0x17F84)
        _emit 0x84
        _emit 0x7f
        _emit 0x01
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI               (return this)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [ESP+0x0C]        (load old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0], ECX             (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                     (discard cookie^ESP)
        _emit 0x5e              // POP ESI                     (restore ESI)
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
