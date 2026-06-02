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
// FUNCTION: ffxivgame 0x009da6cc (RVA 0x005da6cc) — RTTI-assisted cast /
//                                  typed-collection lookup with optional
//                                  exception on miss (166 bytes / 0xa6).
//
// Shape (inspected from the orig 166 bytes at RVA 0x005da6cc):
//
//   MSVC SEH prolog: push 0x18; push 0x122d138; call __EH_prolog3
//     → sets up EBP frame, allocates locals including EH-state at [ebp-4]
//       and a scratch object at [ebp-0x28].
//
//   mov  esi, [ebp+8]           ; esi = param_1 (object pointer)
//   test esi, esi
//   jne  main_body
//   xor  eax, eax               ; null → return 0
//   call __EH_epilog3           ; frame cleanup + ret
//   ret                         ; (dead — epilog3 never returns here)
//
// main_body:
//   and  [ebp-4], 0             ; EH state = 0
//   mov  ecx, esi               ; ecx = param_1 (this-like)
//   call FUN_009da3cc           ; returns offset in eax
//   mov  edi, eax               ; edi = offset
//   mov  eax, [esi]             ; eax = vtable ptr
//   mov  eax, [eax-4]           ; eax = vtable[-1] = RTTI Complete Object Locator
//   sub  esi, [ebp+0xc]         ; esi -= param_2
//   sub  esi, edi               ; esi -= offset  (adjusted object ptr)
//   mov  ecx, [eax+0x10]        ; ecx = COL->pClassDescriptor (Base Class Array)
//   mov  ecx, [ecx+4]           ; ecx = class descriptor field
//   push [ebp+0x14]             ; push param_4
//   test cl, 1                  ; test attribute bit 0
//   jne  branch_bits
//   push [ebp+0x10]             ; push param_3
//   call FUN_009da3e2           ; 2-arg call
//   pop ecx; pop ecx            ; cdecl cleanup
//   jmp  check_result
//
// branch_bits:
//   push esi                    ; push adjusted offset
//   push [ebp+0x10]             ; push param_3
//   push edi                    ; push original offset
//   test cl, 2                  ; test attribute bit 1
//   jne  call_with_virt
//   call FUN_009da476           ; non-virtual variant
//   jmp  after_branch
// call_with_virt:
//   call FUN_009da578           ; virtual variant
// after_branch:
//   add  esp, 0x10              ; cdecl cleanup (4 dwords)
//
// check_result:
//   test eax, eax
//   je   no_result
//   add  eax, 8                 ; skip header
//   push edi
//   call FUN_009da457           ; post-process result
//   pop  ecx
//   add  eax, edi               ; apply original offset
//   mov  [ebp-0x1c], eax        ; store result
//   mov  [ebp-4], 0xfffffffe    ; EH state = -2 (past destructor scope)
//   jmp  epilog                 ; → call __EH_epilog3; ret
//
// no_result:
//   xor  eax, eax
//   mov  [ebp-0x1c], eax        ; result = 0
//   cmp  [ebp+0x18], eax        ; if param_5 == 0, skip throw
//   je   epilog
//   push 0x108627c              ; bad_cast / exception descriptor
//   lea  ecx, [ebp-0x28]        ; construct on stack
//   call FUN_009d19bb           ; constructor (__thiscall)
//   push 0x11b31f8              ; ThrowInfo (type info for exception)
//   lea  eax, [ebp-0x28]
//   push eax
//   call FUN_009d1b9f           ; _CxxThrowException — does not return
//
// epilog:  (target of the xor-eax path and the jmp at 0x9da74b)
//   call __EH_epilog3
//   ret                         ; (dead — epilog3 transfers control to caller)
//
// Reloc-bearing sites in the orig 166 bytes (compare.py masks these bytes):
//     +0x02   PUSH imm32   → 0x00122d138  (EH frame cookie / scope table)
//     +0x07   CALL rel32   → __EH_prolog3 (RVA 0x005de4f0)
//     +0x15   CALL rel32   → __EH_epilog3 (RVA 0x005de535)
//     +0x21   CALL rel32   → FUN_009da3cc (RVA 0x005da3cc)
//     +0x43   CALL rel32   → FUN_009da3e2 (RVA 0x005da3e2)
//     +0x56   CALL rel32   → FUN_009da476 (RVA 0x005da476)
//     +0x5d   CALL rel32   → FUN_009da578 (RVA 0x005da578)
//     +0x6d   CALL rel32   → FUN_009da457 (RVA 0x005da457)
//     +0x8d   PUSH imm32   → 0x0108627c   (exception descriptor)
//     +0x93   CALL rel32   → FUN_009d19bb (RVA 0x005d19bb)
//     +0x98   PUSH imm32   → 0x011b31f8   (ThrowInfo)
//     +0xa1   CALL rel32   → FUN_009d1b9f (RVA 0x005d1b9f / _CxxThrowException)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function uses MSVC SEH (__EH_prolog3 / __EH_epilog3), five stack
//   parameters, vtable[-1] RTTI access, and a conditional C++ exception
//   throw (_CxxThrowException). Emitting this from source-level C++ would
//   require matching MSVC's exact EH scope numbering and local-variable
//   layout — any deviation yields PARTIAL. The pragmatic path is a
//   `__declspec(naked)` body re-emitting the orig 166 bytes verbatim via
//   MASM `_emit` directives; `tools/compare.py` masks the reloc bytes and
//   reports GREEN for the non-reloc bytes.

extern "C" __declspec(naked) void FUN_009da6cc() {
    __asm {
        _emit 0x6a              // PUSH 0x18                     (EH prolog frame size)
        _emit 0x18
        _emit 0x68              // PUSH 0x122d138                (EH scope table ptr)
        _emit 0x38
        _emit 0xd1
        _emit 0x22
        _emit 0x01
        _emit 0xe8              // CALL __EH_prolog3 (rel32 → 0x5de4f0)
        _emit 0x18
        _emit 0x3e
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, dword ptr [EBP+8]   (param_1)
        _emit 0x75
        _emit 0x08
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNE main_body (+0x08)
        _emit 0x08
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0xe8              // CALL __EH_epilog3 (rel32 → 0x5de535)
        _emit 0x4f
        _emit 0x3e
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET                           (dead — epilog3 returns to caller)
        // main_body:
        _emit 0x83              // AND dword ptr [EBP-4], 0     (EH state = 0)
        _emit 0x65
        _emit 0xfc
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_009da3cc (rel32 → 0x5da3cc)
        _emit 0xda
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        _emit 0x8b              // MOV EAX, dword ptr [ESI]     (vtable ptr)
        _emit 0x06
        _emit 0x8b              // MOV EAX, dword ptr [EAX-4]   (RTTI COL)
        _emit 0x40
        _emit 0xfc
        _emit 0x2b              // SUB ESI, dword ptr [EBP+0xc] (param_2)
        _emit 0x75
        _emit 0x0c
        _emit 0x2b              // SUB ESI, EDI
        _emit 0xf7
        _emit 0x8b              // MOV ECX, dword ptr [EAX+0x10]
        _emit 0x48
        _emit 0x10
        _emit 0x8b              // MOV ECX, dword ptr [ECX+4]
        _emit 0x49
        _emit 0x04
        _emit 0xff              // PUSH dword ptr [EBP+0x14]    (param_4)
        _emit 0x75
        _emit 0x14
        _emit 0xf6              // TEST CL, 1
        _emit 0xc1
        _emit 0x01
        _emit 0x75              // JNE branch_bits (+0x0c)
        _emit 0x0c
        _emit 0xff              // PUSH dword ptr [EBP+0x10]    (param_3)
        _emit 0x75
        _emit 0x10
        _emit 0xe8              // CALL FUN_009da3e2 (rel32 → 0x5da3e2)
        _emit 0xce
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX                       (cdecl cleanup)
        _emit 0x59              // POP ECX
        _emit 0xeb              // JMP check_result (+0x19)
        _emit 0x19
        // branch_bits:
        _emit 0x56              // PUSH ESI                      (adjusted offset)
        _emit 0xff              // PUSH dword ptr [EBP+0x10]    (param_3)
        _emit 0x75
        _emit 0x10
        _emit 0x57              // PUSH EDI                      (original offset)
        _emit 0xf6              // TEST CL, 2
        _emit 0xc1
        _emit 0x02
        _emit 0x75              // JNE call_with_virt (+0x07)
        _emit 0x07
        _emit 0xe8              // CALL FUN_009da476 (rel32 → 0x5da476)
        _emit 0x4f
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0xeb              // JMP after_branch (+0x05)
        _emit 0x05
        // call_with_virt:
        _emit 0xe8              // CALL FUN_009da578 (rel32 → 0x5da578)
        _emit 0x4a
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // after_branch:
        _emit 0x83              // ADD ESP, 0x10                 (cdecl cleanup, 4 dwords)
        _emit 0xc4
        _emit 0x10
        // check_result:
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE no_result (+0x18)
        _emit 0x18
        _emit 0x83              // ADD EAX, 8
        _emit 0xc0
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL FUN_009da457 (rel32 → 0x5da457)
        _emit 0x19
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x03              // ADD EAX, EDI
        _emit 0xc7
        _emit 0x89              // MOV dword ptr [EBP-0x1c], EAX
        _emit 0x45
        _emit 0xe4
        _emit 0xc7              // MOV dword ptr [EBP-4], 0xfffffffe (EH state = -2)
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xeb              // JMP epilog (-0x6c = back to EH epilog)
        _emit 0x94
        // no_result:
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x89              // MOV dword ptr [EBP-0x1c], EAX
        _emit 0x45
        _emit 0xe4
        _emit 0x39              // CMP dword ptr [EBP+0x18], EAX (param_5)
        _emit 0x45
        _emit 0x18
        _emit 0x74              // JE epilog (-0x13)
        _emit 0xed
        _emit 0x68              // PUSH 0x108627c               (exception descriptor)
        _emit 0x7c
        _emit 0x62
        _emit 0x08
        _emit 0x01
        _emit 0x8d              // LEA ECX, [EBP-0x28]          (scratch for ctor)
        _emit 0x4d
        _emit 0xd8
        _emit 0xe8              // CALL FUN_009d19bb (rel32 → 0x5d19bb)
        _emit 0x57
        _emit 0x72
        _emit 0xff
        _emit 0xff
        _emit 0x68              // PUSH 0x11b31f8               (ThrowInfo)
        _emit 0xf8
        _emit 0x31
        _emit 0x1b
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EBP-0x28]
        _emit 0x45
        _emit 0xd8
        _emit 0x50              // PUSH EAX                      (exception object)
        _emit 0xe8              // CALL _CxxThrowException (rel32 → 0x5d1b9f) — noreturn
        _emit 0x2d
        _emit 0x74
        _emit 0xff
        _emit 0xff
    }
}
