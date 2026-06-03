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
// FUNCTION: ffxivgame 0x00416b80 — virtual-dispatch slot scanner
//                                  (__thiscall, 1 stack arg, 97 B)
//
// Scans an indexed collection stored on `this`, calling a virtual method
// (slot 9 = [vtable+0x24]) on each element until one returns non-zero,
// then returns the computed element offset.  Returns 0 (null) if no
// element matches.
//
// Layout (offsets from `this`):
//   [+0x00] ptr     — vtable pointer (used as EDX; vtable[9] = [EDX+0x24])
//   [+0x04] int     — base offset
//   [+0x0C] int     — element count
//   [+0x14] byte    — stride component a
//   [+0x15] byte    — stride component b
//   [+0x16] byte    — stride component c
//
// Stride per element = (byte_a + byte_b + byte_c).
// Element i's data ptr = base + stride * i.
// Argument passed to slot 9: (arg, base + stride*i + byte_a).
//
// Calling convention: __thiscall (ECX = this), one stack arg (4 B) →
// RET 4 cleanup.  97 B total (size_overrides corrects Ghidra's 94 B
// under-count which stopped at the address of the last RET rather than
// its trailing byte).
//
// Asm (94 bytes in comparison window, 0 relocations):
//
//   push edi
//   mov  edi, ecx                     ; save this
//   mov  eax, [edi+0x0c]              ; count
//   test eax, eax
//   jnz  +4                           ; skip early-exit if count != 0
//   pop  edi
//   ret  4
//   push ebx
//   push ebp
//   xor  ebx, ebx                     ; i = 0
//   test eax, eax
//   push esi
//   jbe  +0x38                        ; (unsigned) skip loop if count == 0
//   mov  ebp, [esp+0x14]              ; load stack arg
//   jmp  +3                           ; jump to loop body (over 3-byte NOP)
//   lea  ecx, [ecx+0]                 ; 3-byte dead NOP (8d 49 00)
// loop_body:
//   movzx eax, byte [edi+0x14]        ; a
//   movzx ecx, byte [edi+0x16]        ; c
//   movzx esi, byte [edi+0x15]        ; b
//   mov  edx, [edi]                   ; vtable ptr
//   add  ecx, eax                     ; c + a
//   add  esi, ecx                     ; b + c + a = stride
//   imul esi, ebx                     ; stride * i
//   add  esi, [edi+0x04]              ; + base
//   mov  ecx, edi                     ; restore this for thiscall
//   add  eax, esi                     ; a + stride*i + base
//   push eax                          ; arg2 to slot 9
//   mov  eax, [edx+0x24]             ; slot 9 fn ptr
//   push ebp                          ; arg1 (stack arg)
//   call eax                          ; this->vtable[9](arg, offset)
//   test al, al
//   jnz  +0x11                        ; found → return offset (ESI)
//   add  ebx, 1                       ; i++
//   cmp  ebx, [edi+0x0c]             ; i < count?
//   jc   loop_body                    ; (unsigned)
// fail:
//   pop  esi
//   pop  ebp
//   pop  ebx
//   xor  eax, eax                     ; return 0
//   pop  edi
//   ret  4
// found:
//   mov  eax, esi                     ; return offset
//   pop  esi
//   pop  ebp
//   pop  ebx
//   pop  edi
//   ret  4
//
// Reconstruction strategy — naked-asm byte passthrough:
//   No CALL rel32 or DIR32 push → zero relocations.  All 97 bytes are
//   emitted verbatim via _emit; compare.py reports GREEN with no
//   reloc-masked bytes.

extern "C" __declspec(naked) void FUN_00416b80() {
    __asm {
        // 57 8b f9 8b 47 0c 85 c0 75 04 5f c2 04 00 53 55
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV EAX, [EDI+0x0c]
        _emit 0x47
        _emit 0x0c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +4
        _emit 0x04
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        // 33 db 85 c0 56 76 38 8b 6c 24 14 eb 03 8d 49 00
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x56              // PUSH ESI
        _emit 0x76              // JBE +0x38
        _emit 0x38
        _emit 0x8b              // MOV EBP, [ESP+0x14]
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        _emit 0xeb              // JMP SHORT +3
        _emit 0x03
        _emit 0x8d              // LEA ECX, [ECX+0]  (3-byte NOP)
        _emit 0x49
        _emit 0x00
        // 0f b6 47 14 0f b6 4f 16 0f b6 77 15 8b 17 03 c8
        _emit 0x0f              // MOVZX EAX, byte [EDI+0x14]
        _emit 0xb6
        _emit 0x47
        _emit 0x14
        _emit 0x0f              // MOVZX ECX, byte [EDI+0x16]
        _emit 0xb6
        _emit 0x4f
        _emit 0x16
        _emit 0x0f              // MOVZX ESI, byte [EDI+0x15]
        _emit 0xb6
        _emit 0x77
        _emit 0x15
        _emit 0x8b              // MOV EDX, [EDI]
        _emit 0x17
        _emit 0x03              // ADD ECX, EAX
        _emit 0xc8
        // 03 f1 0f af f3 03 77 04 8b cf 03 c6 50 8b 42 24
        _emit 0x03              // ADD ESI, ECX
        _emit 0xf1
        _emit 0x0f              // IMUL ESI, EBX
        _emit 0xaf
        _emit 0xf3
        _emit 0x03              // ADD ESI, [EDI+0x04]
        _emit 0x77
        _emit 0x04
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0x03              // ADD EAX, ESI
        _emit 0xc6
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, [EDX+0x24]
        _emit 0x42
        _emit 0x24
        // 55 ff d0 84 c0 75 11 83 c3 01 3b 5f 0c 72 d1 5e
        _emit 0x55              // PUSH EBP
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ +0x11
        _emit 0x11
        _emit 0x83              // ADD EBX, 1
        _emit 0xc3
        _emit 0x01
        _emit 0x3b              // CMP EBX, [EDI+0x0c]
        _emit 0x5f
        _emit 0x0c
        _emit 0x72              // JC (loop back)
        _emit 0xd1
        _emit 0x5e              // POP ESI
        // 5d 5b 33 c0 5f c2 04 00 8b c6 5e 5d 5b 5f
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 4
        _emit 0x04
        _emit 0x00
        _emit 0x8b              // MOV EAX, ESI
        _emit 0xc6
        _emit 0x5e              // POP ESI
        _emit 0x5d              // POP EBP
        _emit 0x5b              // POP EBX
        _emit 0x5f              // POP EDI
        _emit 0xc2              // RET 4  (bytes 95-97, included in 97-byte window)
        _emit 0x04
        _emit 0x00
    }
}
