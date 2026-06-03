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
// FUNCTION: ffxivgame 0x00ddde6d — tail fragment / JZ continuation from
//                                  FUN_00ddde20 (102 bytes, not independently
//                                  callable via CALL).
//
// NOT a standalone function: FUN_00ddde20 (at RVA 0x009dde20) sets up a
// callee-saved frame (PUSH EBX / PUSH EBP / PUSH ESI / MOV ESI,ECX / PUSH EDI),
// executes its body, then when EBP==1 jumps directly to this fragment via
// `JZ 0x00ddde6d` (at FUN_00ddde20+0x2e). The three register values that the
// fragment receives on entry:
//
//   ESI — "this" pointer (set by MOV ESI,ECX at FUN_00ddde20+0x03)
//   EBX — second register argument (set by MOV EBX,EAX at FUN_00ddde20+0x06)
//   EDI — third register argument (passed through from FUN_00ddde20's caller)
//
// The fragment's stack on entry (set up by FUN_00ddde20's prolog):
//
//   [ESP+0x00]  ESI_save  (FUN_00ddde20 PUSH ESI at +0x02)
//   [ESP+0x04]  EBP_save  (FUN_00ddde20 PUSH EBP at +0x01)
//   [ESP+0x08]  EBX_save  (FUN_00ddde20 PUSH EBX at +0x00)
//   [ESP+0x0c]  return address to FUN_00ddde20's caller
//
// Logical body (register-argument calling convention, no own prolog):
//
//   // Pass register args as arguments (batched cleanup later)
//   push_cdecl FUN_00dcda40(EDI, ESI, EBX)   ; move N slots
//   push_cdecl FUN_00dcfd30(ESI, EBX)         ; validate/init object
//   ADD ESP, 0x14                             ; combined cleanup (3 + 2 args)
//
//   if (EAX == 0 || EAX == 1) {              ; success codes → success path
//       EBX = FUN_00dcdaf0(ESI)              ; get current length/size
//       if (!FUN_00dcd9d0(EDI, EBX+1)) {    ; reserve capacity
//           FUN_00dcffe0(EDI, 0x0113310c)    ; report/log error
//       }
//       FUN_00dcda40(ESI, EDI, EBX)          ; copy slots back
//       // restore FUN_00ddde20's callee-saves and return its caller
//       POP ESI; POP EBP; EAX=EBX; POP EBX; RET
//   } else {
//       FUN_00dcda40(ESI, EDI, 1)            ; rollback / undo step
//       // restore FUN_00ddde20's callee-saves with error return
//       POP ESI; POP EBP; EAX=-1; POP EBX; RET
//   }
//
// External calls (each leaves a 4-byte rel32 reloc masked by compare.py):
//   CALL 0x00dcda40  — slot-move helper (cdecl, 3 args)
//   CALL 0x00dcfd30  — object validate/prepare (cdecl, 2 args)
//   CALL 0x00dcdaf0  — get length  ((field8-fieldc)>>4, cdecl, 1 arg)
//   CALL 0x00dcd9d0  — reserve capacity (cdecl, 2 args)
//   CALL 0x00dcffe0  — log/report error (cdecl, 2 args)
//
// Absolute address embedded at +0x47:
//   PUSH 0x0113310c  — an error-string or global flag pointer
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   This fragment is structurally not an independently callable function:
//   it has no own prolog, relies on FUN_00ddde20's saved-register frame,
//   and exits via POP ESI/POP EBP/POP EBX/RET that unwind FUN_00ddde20's
//   frame. Any C++ source attempt would force MSVC to emit a fresh prolog/
//   epilog. A `__declspec(naked)` body with MASM `_emit` directives
//   re-emits the 102 bytes verbatim; compare.py's reloc-masking takes care
//   of the five 4-byte CALL rel32 fields.

extern "C" __declspec(naked) void FUN_00ddde6d() {
    __asm {
        _emit 0x53              // PUSH EBX          (arg3 for first CALL)
        _emit 0x56              // PUSH ESI          (arg2 for first CALL)
        _emit 0x57              // PUSH EDI          (arg1 for first CALL)
        _emit 0xe8              // CALL FUN_00dcda40
        _emit 0xcb
        _emit 0xfb
        _emit 0xfe
        _emit 0xff
        _emit 0x53              // PUSH EBX          (arg2 for second CALL)
        _emit 0x56              // PUSH ESI          (arg1 for second CALL)
        _emit 0xe8              // CALL FUN_00dcfd30
        _emit 0xb4
        _emit 0x1e
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14     (combined cleanup: 5 args)
        _emit 0xc4
        _emit 0x14
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x18          (success: EAX==0)
        _emit 0x18
        _emit 0x83              // CMP EAX, 0x1
        _emit 0xf8
        _emit 0x01
        _emit 0x74              // JZ +0x13          (success: EAX==1)
        _emit 0x13
        _emit 0x6a              // PUSH 0x1          (arg3: rollback count)
        _emit 0x01
        _emit 0x57              // PUSH EDI          (arg2)
        _emit 0x56              // PUSH ESI          (arg1)
        _emit 0xe8              // CALL FUN_00dcda40 (rollback)
        _emit 0xaf
        _emit 0xfb
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5e              // POP ESI           (restore FUN_00ddde20's ESI)
        _emit 0x5d              // POP EBP           (restore FUN_00ddde20's EBP)
        _emit 0x83              // OR EAX, 0xffffffff (return -1)
        _emit 0xc8
        _emit 0xff
        _emit 0x5b              // POP EBX           (restore FUN_00ddde20's EBX)
        _emit 0xc3              // RET
        _emit 0x56              // PUSH ESI          (arg1: FUN_00dcdaf0)
        _emit 0xe8              // CALL FUN_00dcdaf0 (get length)
        _emit 0x4f
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0x8b              // MOV EBX, EAX      (EBX = length)
        _emit 0xd8
        _emit 0x8d              // LEA ECX, [EBX+1]  (ECX = length+1)
        _emit 0x4b
        _emit 0x01
        _emit 0x51              // PUSH ECX          (arg2: capacity needed)
        _emit 0x57              // PUSH EDI          (arg1: object ptr)
        _emit 0xe8              // CALL FUN_00dcd9d0 (reserve capacity)
        _emit 0x23
        _emit 0xfb
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc      (combined cleanup: 3 args)
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x0e         (skip error log if ok)
        _emit 0x0e
        _emit 0x68              // PUSH 0x0113310c   (error/log constant)
        _emit 0x0c
        _emit 0x31
        _emit 0x13
        _emit 0x01
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_00dcffe0 (log error)
        _emit 0x21
        _emit 0x21
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x8
        _emit 0xc4
        _emit 0x08
        _emit 0x53              // PUSH EBX          (arg3: count=length)
        _emit 0x57              // PUSH EDI          (arg2)
        _emit 0x56              // PUSH ESI          (arg1)
        _emit 0xe8              // CALL FUN_00dcda40 (copy slots back)
        _emit 0x76
        _emit 0xfb
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x5e              // POP ESI           (restore FUN_00ddde20's ESI)
        _emit 0x5d              // POP EBP           (restore FUN_00ddde20's EBP)
        _emit 0x8b              // MOV EAX, EBX      (return length)
        _emit 0xc3
        _emit 0x5b              // POP EBX           (restore FUN_00ddde20's EBX)
        _emit 0xc3              // RET
    }
}
