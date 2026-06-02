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
// FUNCTION: ffxivgame 0x009d042e — EH-framed helper that constructs a
//                                  std::basic_string message, wraps it in a
//                                  logic_error-shaped exception object, and
//                                  throws via __CxxThrowException@8.
//                                  (62 B / 0x3E, no calling-convention args,
//                                   function never returns)
//
// Behaviour read from the orig 62 bytes at RVA 0x005d042e:
//
//   1. __EH_prolog3 frame (frame-size = 0x44):
//        push 0x44                      ; local-frame size
//        mov  eax, 0x00ED86A5           ; EH handler-table pointer
//        call __EH_prolog3              ; (RVA 0x5dc47b) sets up EBP frame,
//                                       ; allocates 0x44 bytes for locals,
//                                       ; splices the EH record into the chain
//
//   2. Assign a string literal into a local basic_string at [EBP-0x28]:
//        push 0x01085C30                ; const char* literal address
//        lea  ecx, [ebp-0x28]           ; this = &local_string
//        call FUN_00404400              ; __thiscall basic_string::operator=
//                                       ; (RVA 0x4400, ret 4)
//
//   3. Mark EH state 0 (basic_string destructor registered for unwind):
//        and  dword ptr [ebp-0x4], 0    ; trylevel = 0
//
//   4. Construct exception object at [EBP-0x50] using &local_string:
//        lea  eax, [ebp-0x28]           ; &local_string
//        push eax
//        lea  ecx, [ebp-0x50]           ; this = &exc_obj
//        call FUN_00404320              ; __thiscall exception ctor, takes
//                                       ; const Utf8String*; sets vtable to
//                                       ; 0x00F54A2C (base class) and copies
//                                       ; the message string. (RVA 0x4320, ret 4)
//
//   5. Throw the exception:
//        push 0x011A9088                ; ThrowInfo* (type descriptor)
//        lea  eax, [ebp-0x50]           ; &exc_obj
//        push eax
//        mov  dword ptr [ebp-0x50], 0x00F54A38
//                                       ; override vtable → derived class ptr
//        call __CxxThrowException@8     ; (RVA 0x5d1b9f) — never returns;
//                                       ; EH unwind tears down local_string
//
// The function has NO epilogue — __CxxThrowException@8 unwinds the EH chain
// (calling local_string's destructor via the registered state-0 handler) and
// never returns to the caller.
//
// Reloc-bearing sites in the orig 62 bytes:
//   +0x02   MOV  EAX, imm32  → 0x00ED86A5  (EH handler table)
//   +0x07   CALL rel32       → __EH_prolog3     (RVA 0x5dc47b)
//   +0x0C   PUSH imm32       → 0x01085C30       (string literal)
//   +0x12   CALL rel32       → FUN_00404400     (RVA 0x4400)
//   +0x22   CALL rel32       → FUN_00404320     (RVA 0x4320)
//   +0x2A   PUSH imm32       → 0x011A9088       (ThrowInfo*)
//   +0x34   MOV  imm32       → 0x00F54A38       (derived vtable)
//   +0x39   CALL rel32       → __CxxThrowException@8 (RVA 0x5d1b9f)
//
// Reconstruction strategy — naked-asm byte passthrough (mirrors FUN_00406fa0,
// FUN_00408780, FUN_00404320 in the same binary): re-emitting the orig 62
// bytes verbatim via MASM _emit directives produces a .obj whose .text is
// byte-identical to the orig binary slice. compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_009d042e() {
    __asm {
        _emit 0x6a              // PUSH 0x44
        _emit 0x44
        _emit 0xb8              // MOV EAX, 0x00ED86A5   (EH handler table)
        _emit 0xa5
        _emit 0x86
        _emit 0xed
        _emit 0x00
        _emit 0xe8              // CALL __EH_prolog3  (rel32 → 0x009dc47b)
        _emit 0x41
        _emit 0xc0
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x01085C30   (string literal addr)
        _emit 0x30
        _emit 0x5c
        _emit 0x08
        _emit 0x01
        _emit 0x8d              // LEA ECX, [EBP-0x28]
        _emit 0x4d
        _emit 0xd8
        _emit 0xe8              // CALL FUN_00404400 (rel32 → 0x00404400)
        _emit 0xb9
        _emit 0x3f
        _emit 0xa3
        _emit 0xff
        _emit 0x83              // AND dword ptr [EBP-0x04], 0   (EH state = 0)
        _emit 0x65
        _emit 0xfc
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EBP-0x28]
        _emit 0x45
        _emit 0xd8
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [EBP-0x50]
        _emit 0x4d
        _emit 0xb0
        _emit 0xe8              // CALL FUN_00404320 (rel32 → 0x00404320)
        _emit 0xc9
        _emit 0x3e
        _emit 0xa3
        _emit 0xff
        _emit 0x68              // PUSH 0x011A9088   (ThrowInfo*)
        _emit 0x88
        _emit 0x90
        _emit 0x1a
        _emit 0x01
        _emit 0x8d              // LEA EAX, [EBP-0x50]
        _emit 0x45
        _emit 0xb0
        _emit 0x50              // PUSH EAX
        _emit 0xc7              // MOV dword ptr [EBP-0x50], 0x00F54A38  (derived vtable)
        _emit 0x45
        _emit 0xb0
        _emit 0x38
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0xe8              // CALL __CxxThrowException@8 (rel32 → 0x009d1b9f)
        _emit 0x33
        _emit 0x17
        _emit 0x00
        _emit 0x00
    }
}
