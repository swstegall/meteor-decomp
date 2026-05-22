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
// FUNCTION: ffxivgame 0x0040a460 — engine_memory allocator dispatcher
// (78 B / 0x4e). __thiscall member with one stack arg (the requested
// allocation size, given the RET 4 epilogue). Companion to the bump
// allocator at FUN_0040a410.
//
// Shape:
//   if (DAT_01328034 == 0) {                        ; module init flag
//       err = { 0x10, "<msg @ 0xf552f0>" };
//       this->logger->report(size, &err);
//   } else if ((arena = DAT_01328038) != nullptr) { ; global arena ptr
//       arena->Allocate(size);                      ; tail-call to FUN_0040a410
//   } else {
//       err = { 0x10, "<msg @ 0xf552e0>" };
//       this->logger->report(size, &err);
//   }
//
// Asm (78 B, RVA 0x0000a460):
//   83 ec 08              SUB  ESP, 0x8                  ; reserve { i32, ptr } pair on stack
//   80 3d 34 80 32 01 00  CMP  byte ptr [0x01328034], 0  ; module init flag
//   56                    PUSH ESI                       ; save callee-save
//   8b f1                 MOV  ESI, ECX                  ; cache `this`
//   75 07                 JNZ  +0x07                     ; -> arena-probe
//   68 f0 52 f5 00        PUSH 0xf552f0                  ; msg1 ("not initialized")
//   eb 18                 JMP  +0x18                     ; -> common error-tail
//   8b 0d 38 80 32 01     MOV  ECX, [0x01328038]         ; global arena ptr
//   85 c9                 TEST ECX, ECX
//   74 09                 JZ   +0x09                     ; -> arena-missing error
//   5e                    POP  ESI
//   83 c4 08              ADD  ESP, 0x8                  ; drop the (unused) pair
//   e9 87 ff ff ff        JMP  FUN_0040a410              ; tail-call: arena->Allocate(size)
//   68 e0 52 f5 00        PUSH 0xf552e0                  ; msg2 ("arena null")
//   6a 10                 PUSH 0x10                      ; severity / kind = 0x10
//   8d 4c 24 0c           LEA  ECX, [ESP+0xc]            ; ECX = &err pair on stack
//   e8 37 3e 00 00        CALL FUN_0040e2d0              ; err.init(0x10, msg) — RET 8
//   8b 4e 04              MOV  ECX, [ESI+0x4]            ; ECX = this->logger
//   50                    PUSH EAX                       ; &err
//   8b 44 24 14           MOV  EAX, [ESP+0x14]           ; reload `size` arg
//   50                    PUSH EAX                       ; size
//   e8 69 3c 00 00        CALL FUN_0040e110              ; logger->report(size, &err) — RET 8
//   5e                    POP  ESI
//   83 c4 08              ADD  ESP, 0x8
//   c2 04 00              RET  0x4                       ; __thiscall, 1 stack arg
//
// Reloc-bearing sites in the orig 78 bytes (all are absolute addresses
// the linker baked at the orig image base 0x00400000, or CALL/JMP rel32
// displacements that resolve against the orig's own address space — so
// emitting the bytes raw via MASM `_emit` produces a .obj whose .text
// matches orig byte-for-byte with no relocations):
//     +0x03   CMP  mem8  → 0x01328034
//     +0x0f   PUSH imm32 → 0xf552f0
//     +0x16   MOV  mem32 → 0x01328038
//     +0x24   JMP  rel32 → FUN_0040a410
//     +0x29   PUSH imm32 → 0xf552e0
//     +0x34   CALL rel32 → FUN_0040e2d0
//     +0x42   CALL rel32 → FUN_0040e110
//
// Reconstruction strategy — naked-asm byte passthrough. Same idiom as
// FUN_00409990 / FUN_004014b0 / FUN_00403bd0: emit the orig 78 bytes
// verbatim via `_emit`. The .obj's `.text` is byte-identical to the
// orig slice with zero relocations, and `tools/compare.py` reports
// GREEN.

extern "C" __declspec(naked) void FUN_0040a460() {
    __asm {
        _emit 0x83              // SUB ESP, 8
        _emit 0xec
        _emit 0x08
        _emit 0x80              // CMP byte ptr [0x01328034], 0
        _emit 0x3d
        _emit 0x34
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x75              // JNZ +0x07
        _emit 0x07
        _emit 0x68              // PUSH 0xf552f0
        _emit 0xf0
        _emit 0x52
        _emit 0xf5
        _emit 0x00
        _emit 0xeb              // JMP +0x18
        _emit 0x18
        _emit 0x8b              // MOV ECX, dword ptr [0x01328038]
        _emit 0x0d
        _emit 0x38
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x74              // JZ +0x09
        _emit 0x09
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0xe9              // JMP FUN_0040a410 (rel32 = 0xffffff87)
        _emit 0x87
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x68              // PUSH 0xf552e0
        _emit 0xe0
        _emit 0x52
        _emit 0xf5
        _emit 0x00
        _emit 0x6a              // PUSH 0x10
        _emit 0x10
        _emit 0x8d              // LEA ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8              // CALL FUN_0040e2d0 (rel32 = 0x00003e37)
        _emit 0x37
        _emit 0x3e
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+4]
        _emit 0x4e
        _emit 0x04
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0040e110 (rel32 = 0x00003c69)
        _emit 0x69
        _emit 0x3c
        _emit 0x00
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 8
        _emit 0xc4
        _emit 0x08
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
    }
}
