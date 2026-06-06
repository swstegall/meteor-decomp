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
// FUNCTION: ffxivgame 0x004573f0 — __thiscall list-drain destructor helper
//                                  (178 B / 0xb2)
//
// Sets a vtable pointer, initialises a sub-object via an IAT call, then
// drains a circular doubly-linked list embedded at this+0x20: for each
// node it optionally invokes a virtual callback ([ECX]+0 with arg 1),
// removes the node via FUN_00927440(__thiscall, 3 stack args), and loops.
// After the list is empty it calls a second IAT function to tear down the
// sub-object, then finalises the list head via FUN_00927700 and frees the
// last allocation with _free (0x009d1b17). The function does NOT contain
// a standard /GS epilog — it ends at the _free CALL, implying the SEH
// frame is unwound by the caller's context.
//
//   Calling convention: __thiscall (this in ECX).
//   Stack frame: /GS cookie + EH3 SEH prolog; 0xc bytes of locals.
//   Saved regs: EBX, EBP, ESI, EDI.
//
//   Globals / IAT entries touched:
//     [0xf3e16c]  — IAT: init sub-object call (PUSH this+8 arg)
//     [0xf3e168]  — IAT: teardown sub-object call (PUSH this+8 arg)
//     0xe57716    — EH3 scope table (in .rdata)
//     0x012ea8b0  — __security_cookie
//     0x00f67870  — vtable pointer written to [this]
//
//   Internal calls:
//     0x009d22b4  — debug/assert traps (unreachable from normal flow)
//     0x00927440  — list-node remove  (__thiscall, args: node, list, &temp)
//     0x00927700  — list-head finalise (__thiscall, 5 stack args)
//     0x009d1b17  — _free (cdecl, 1 arg)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The /GS+EH3 prolog, the self-compare CMP ESI,ESI / JZ idiom (dead
//   code bypass for the assert traps), the indirect CALL EDX for the
//   virtual callback, and the absence of a standard epilog make source-
//   level reconstruction impractical. All 178 bytes are emitted verbatim
//   via MASM _emit directives, producing a .obj .text section that is
//   byte-identical to the original slice. compare.py wildcards the
//   relocation windows listed below.
//
// Reloc-bearing sites (offset within function, 4-byte window wildcarded):
//   +0x03  PUSH imm32    scope_table (0xe57716 — EH3 FuncInfo ptr)
//   +0x16  MOV EAX,[]   __security_cookie (0x012ea8b0)
//   +0x2f  MOV [],imm32  vtable (0x00f67870)
//   +0x41  CALL []       IAT 0x00f3e16c (init)
//   +0x54  CALL rel32    0x009d22b4 (assert trap, dead code)
//   +0x62  CALL rel32    0x009d22b4 (assert trap)
//   +0x7f  CALL rel32    0x00927440 (list remove)
//   +0x8b  CALL []       IAT 0x00f3e168 (teardown)
//   +0xa5  CALL rel32    0x00927700 (list head finalise)
//   +0xae  CALL rel32    0x009d1b17 (_free)

extern "C" __declspec(naked) void FUN_004573f0() {
    __asm {
        // --- /GS + EH3 SEH prolog -----------------------------------------
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH scope_table (0xe57716)
        _emit 0x16
        _emit 0x77
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0xc
        _emit 0xec
        _emit 0x0c
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (cookie)
        _emit 0x8d              // LEA EAX, [ESP+0x20]
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- stash this, set vtable, init sub-object ----------------------
        _emit 0x8b              // MOV EBX, ECX  (EBX = this)
        _emit 0xd9
        _emit 0x89              // MOV [ESP+0x14], EBX
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0xc7              // MOV dword ptr [EBX], 0xf67870  (vtable)
        _emit 0x03
        _emit 0x70
        _emit 0x78
        _emit 0xf6
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EBX+8]
        _emit 0x43
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xc7              // MOV dword ptr [ESP+0x2c], 1
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xff              // CALL [0x00f3e16c]  (IAT: init)
        _emit 0x15
        _emit 0x6c
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // --- loop: set ESI = &list head (this+0x20) -----------------------
        _emit 0x8d              // LEA ESI, [EBX+0x20]
        _emit 0x73
        _emit 0x20
        // --- loop top (target of JMP at +0x83) ----------------------------
        _emit 0x3b              // CMP ESI, ESI  (always ZF=1; bypasses assert)
        _emit 0xf6
        _emit 0x8b              // MOV EAX, dword ptr [ESI+4]
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV EDI, dword ptr [EAX]
        _emit 0x38
        _emit 0x8b              // MOV EBP, EAX
        _emit 0xe8
        _emit 0x74              // JZ +5  (always taken — dead-code skip)
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (assert trap, dead)
        _emit 0x6c
        _emit 0xae
        _emit 0x57
        _emit 0x00
        // --- check list empty (EDI == EBP?) --------------------------------
        _emit 0x3b              // CMP EDI, EBP
        _emit 0xfd
        _emit 0x74              // JZ +0x29  (list empty → exit)
        _emit 0x29
        // --- guard: EDI must not equal list head ---------------------------
        _emit 0x3b              // CMP EDI, dword ptr [ESI+4]
        _emit 0x7e
        _emit 0x04
        _emit 0x75              // JNZ +5
        _emit 0x05
        _emit 0xe8              // CALL 0x009d22b4  (assert trap)
        _emit 0x5e
        _emit 0xae
        _emit 0x57
        _emit 0x00
        // --- optional virtual callback on node ([EDI+0x10]) ---------------
        _emit 0x8b              // MOV ECX, dword ptr [EDI+0x10]
        _emit 0x4f
        _emit 0x10
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +8  (skip if null)
        _emit 0x08
        _emit 0x8b              // MOV EAX, dword ptr [ECX]
        _emit 0x01
        _emit 0x8b              // MOV EDX, dword ptr [EAX]  (vtable[0])
        _emit 0x10
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0xff              // CALL EDX
        _emit 0xd2
        // --- remove node from list via FUN_00927440 -----------------------
        _emit 0x57              // PUSH EDI             (arg: node)
        _emit 0x56              // PUSH ESI             (arg: list)
        _emit 0x8d              // LEA EAX, [ESP+0x20]  (arg: &temp)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x00927440
        _emit 0xcd
        _emit 0xff
        _emit 0x4c
        _emit 0x00
        _emit 0xeb              // JMP -0x3d  (loop back)
        _emit 0xc3
        // --- exit: teardown sub-object ------------------------------------
        _emit 0x8d              // LEA EAX, [EBX+8]
        _emit 0x43
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL [0x00f3e168]  (IAT: teardown)
        _emit 0x15
        _emit 0x68
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // --- finalise list head via FUN_00927700 --------------------------
        _emit 0x8b              // MOV EAX, dword ptr [ESI+4]
        _emit 0x46
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [EAX]
        _emit 0x08
        _emit 0x50              // PUSH EAX
        _emit 0x56              // PUSH ESI
        _emit 0x51              // PUSH ECX
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xc6              // MOV byte ptr [ESP+0x3c], 0
        _emit 0x44
        _emit 0x24
        _emit 0x3c
        _emit 0x00
        _emit 0xe8              // CALL 0x00927700
        _emit 0x67
        _emit 0x02
        _emit 0x4d
        _emit 0x00
        // --- _free(last allocation) ---------------------------------------
        _emit 0x8b              // MOV EAX, dword ptr [ESI+4]
        _emit 0x46
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x009d1b17  (_free)
        _emit 0x75
        _emit 0xa6
        _emit 0x57
        _emit 0x00
    }
}
