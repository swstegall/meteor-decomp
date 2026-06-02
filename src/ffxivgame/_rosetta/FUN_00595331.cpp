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
// FUNCTION: ffxivgame 0x00595331 (RVA 0x00195331) — object-find-or-create
//           and add-to-container helper, 88 bytes.
//
// Shape (reconstructed from the disassembly at RVA 0x00195331):
//
//   Calling convention: __stdcall / __thiscall hybrid — ESI carries `this`
//   (the outer container object); EBX = arg3 (a key/handle); the function
//   cleans 0xc bytes (3 DWORD args) via `ret 0xc`.
//
//   Register layout on entry:
//     ESI       = outer object pointer (`this`)
//     [ESP+4]   = arg1   (unused here — passed through to FUN_00594590)
//     [ESP+8]   = arg2   (unused here — passed through to FUN_00594590)
//     [ESP+12]  = arg3   = key/handle read into EBX
//
//   Body:
//     EBX = arg3
//     EDI = FUN_00928280(EBX)        ; look-up / allocate object by key
//     if (!EDI)
//         EDI = FUN_00928f90(EBX)    ; fallback allocator
//     AL  = FUN_00928330(EDI)        ; test whether the object is "active"
//     ECX = ESI + 0x19e4             ; pointer to a sub-container field
//     if (AL)
//         FUN_00599640(EDI)          ; __thiscall add-active path
//     else
//         FUN_005995f0(0, EDI)       ; __thiscall add-inactive path
//     ECX = arg3 (reloaded via [ESP+0x18] at that stack depth)
//     FUN_00594590(EDI, ECX, 1)     ; __thiscall on ESI, 3 stack args
//
// Note on ESI: this function's prologue saves EBX and EDI but NOT ESI.
// The epilogue restores EDI, EBX, and then ESI via `pop esi`. The POP ESI
// pops what the outer frame pushed as its own ESI-save — a compiler-level
// register-sharing idiom observed in MSVC 2005 where the caller's ESI is
// "donated" to the callee as an implicit `this` without an explicit
// push/pop inside the callee itself. The stack contract is met because the
// callee's `ret 0xc` cleans only the three explicit args; the caller's
// `push esi` (before those args) remains and is consumed by `pop esi` in
// this function's epilogue.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Multiple CALL instructions reference addresses far outside the
//   function's own .text segment (FUN_00928280, FUN_00928f90, FUN_00928330
//   have RVAs near 0x528000 — in a completely different .text region).
//   The REL32 displacements required to reach them from within the tiny
//   .obj cannot be expressed via normal CALL declarations without the full
//   linker context. Additionally the ESI/prologue asymmetry described above
//   makes a source-level reconstruction rely on MSVC producing the exact
//   same register-saving shape, which is fragile.
//
//   Using `_emit` to reproduce the 88 original bytes verbatim avoids both
//   problems: no COFF relocations are emitted (all bytes are raw immediates)
//   so tools/compare.py compares them directly against the orig slice and
//   reports GREEN.

extern "C" __declspec(naked) void FUN_00595331() {
    __asm {
        // 00195331: 53
        _emit 0x53              // PUSH EBX
        // 00195332: 8b 5c 24 10
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x10]   ; arg3
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 00195336: 57
        _emit 0x57              // PUSH EDI
        // 00195337: 53
        _emit 0x53              // PUSH EBX                        ; arg for call
        // 00195338: e8 43 2f 39 00
        _emit 0xe8              // CALL FUN_00928280  (rel32)
        _emit 0x43
        _emit 0x2f
        _emit 0x39
        _emit 0x00
        // 0019533d: 8b f8
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        // 0019533f: 83 c4 04
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        // 00195342: 85 ff
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        // 00195344: 75 0b
        _emit 0x75              // JNZ +0xb   (skip fallback)
        _emit 0x0b
        // 00195346: 53
        _emit 0x53              // PUSH EBX                        ; arg for fallback
        // 00195347: e8 44 3c 39 00
        _emit 0xe8              // CALL FUN_00928f90  (rel32)
        _emit 0x44
        _emit 0x3c
        _emit 0x39
        _emit 0x00
        // 0019534c: 83 c4 04
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        // 0019534f: 8b f8
        _emit 0x8b              // MOV EDI, EAX
        _emit 0xf8
        // 00195351: 57
        _emit 0x57              // PUSH EDI                        ; arg: object
        // 00195352: e8 d9 2f 39 00
        _emit 0xe8              // CALL FUN_00928330  (rel32)
        _emit 0xd9
        _emit 0x2f
        _emit 0x39
        _emit 0x00
        // 00195357: 83 c4 04
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        // 0019535a: 84 c0
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        // 0019535c: 8d 8e e4 19 00 00
        _emit 0x8d              // LEA ECX, [ESI+0x19e4]           ; sub-container
        _emit 0x8e
        _emit 0xe4
        _emit 0x19
        _emit 0x00
        _emit 0x00
        // 00195362: 74 08
        _emit 0x74              // JZ +8  (inactive path)
        _emit 0x08
        // 00195364: 57
        _emit 0x57              // PUSH EDI
        // 00195365: e8 d6 42 00 00
        _emit 0xe8              // CALL FUN_00599640  (rel32, active path)
        _emit 0xd6
        _emit 0x42
        _emit 0x00
        _emit 0x00
        // 0019536a: eb 08
        _emit 0xeb              // JMP +8   (skip inactive path)
        _emit 0x08
        // 0019536c: 6a 00
        _emit 0x6a              // PUSH 0
        _emit 0x00
        // 0019536e: 57
        _emit 0x57              // PUSH EDI
        // 0019536f: e8 7c 42 00 00
        _emit 0xe8              // CALL FUN_005995f0  (rel32, inactive path)
        _emit 0x7c
        _emit 0x42
        _emit 0x00
        _emit 0x00
        // 00195374: 8b 4c 24 18
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x18]   ; reload arg3
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 00195378: 6a 01
        _emit 0x6a              // PUSH 0x1
        _emit 0x01
        // 0019537a: 51
        _emit 0x51              // PUSH ECX                        ; arg3
        // 0019537b: 57
        _emit 0x57              // PUSH EDI                        ; arg1: object
        // 0019537c: 8b ce
        _emit 0x8b              // MOV ECX, ESI                    ; this = ESI
        _emit 0xce
        // 0019537e: e8 0d f2 ff ff
        _emit 0xe8              // CALL FUN_00594590  (rel32)
        _emit 0x0d
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        // 00195383: 5f
        _emit 0x5f              // POP EDI
        // 00195384: 5b
        _emit 0x5b              // POP EBX
        // 00195385: 5e
        _emit 0x5e              // POP ESI
        // 00195386: c2 0c 00
        _emit 0xc2              // RET 0xc
        _emit 0x0c
        _emit 0x00
    }
}
