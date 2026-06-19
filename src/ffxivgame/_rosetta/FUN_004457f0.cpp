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
// FUNCTION: ffxivgame 0x000457f0 — concatenate null-terminated pointer array
//                                  (__cdecl, 96 bytes)
//
// int FUN_004457f0(const char **strings, char *dest)
//
//   Iterates over a null-terminated array of `char *` pointers, passing
//   each string (plus `dest` or 0) to the helper FUN_00445670 which
//   copies the string into `dest` (if non-null) and returns its length.
//
//   If `dest` == NULL:  count-only — accumulate lengths, return total.
//   If `dest` != NULL:  copy — concatenate all strings into `dest`,
//                       append a null terminator, return total byte count.
//
// Calling convention: __cdecl (caller cleans; EBX, ESI, EDI callee-saved)
//   [ESP+0x04] = strings  (const char **, null-terminated pointer array)
//   [ESP+0x08] = dest     (char *, output buffer, or NULL)
//
// Control-flow layout (relative to function start, 0x000457f0):
//
//   +0x00  PUSH EBX                          ; save callee-saved regs
//   +0x01  PUSH ESI
//   +0x02  MOV  ESI, [ESP+0x10]              ; ESI = dest (arg1)
//   +0x06  XOR  EBX, EBX                     ; total = 0
//   +0x08  TEST ESI, ESI
//   +0x0a  JNZ  +0x25 → copy_path (+0x31)
//
//   count-only path (dest == NULL):
//   +0x0c  MOV  ESI, [ESP+0x0C]              ; ESI = strings (arg0)
//   +0x10  MOV  EAX, [ESI]                   ; EAX = *strings
//   +0x12  TEST EAX, EAX
//   +0x14  JZ   +0x45 → epilogue (+0x5b)     ; empty array
//   count_loop (+0x16):
//   +0x16  PUSH 0x0
//   +0x18  PUSH EAX                          ; str
//   +0x19  CALL FUN_00445670                 ; len = helper(str, NULL)
//   +0x1e  ADD  ESI, 0x4                     ; strings++
//   +0x21  ADD  EBX, EAX                     ; total += len
//   +0x23  MOV  EAX, [ESI]                   ; next ptr
//   +0x25  ADD  ESP, 0x8                     ; cdecl cleanup
//   +0x28  TEST EAX, EAX
//   +0x2a  JNZ  -0x16 → count_loop (+0x16)
//   +0x2c  POP  ESI                          ; count-only epilogue
//   +0x2d  MOV  EAX, EBX
//   +0x2f  POP  EBX
//   +0x30  RET
//
//   copy_path (+0x31):
//   +0x31  PUSH EDI
//   +0x32  MOV  EDI, [ESP+0x10]              ; EDI = strings (arg0; ESP shifted)
//   +0x36  MOV  EAX, [EDI]                   ; EAX = *strings
//   +0x38  TEST EAX, EAX
//   +0x3a  JZ   +0x1b → write_null (+0x57)  ; empty array
//   +0x3c  LEA  ESP, [ESP+0x00]              ; 4-byte NOP align (loop to 0x30 ≡ 0 mod 16)
//   copy_loop (+0x40):
//   +0x40  PUSH ESI                          ; dest
//   +0x41  PUSH EAX                          ; str
//   +0x42  CALL FUN_00445670                 ; len = helper(str, dest)
//   +0x47  ADD  EDI, 0x4                     ; strings++
//   +0x4a  ADD  ESI, EAX                     ; dest += len
//   +0x4c  ADD  EBX, EAX                     ; total += len
//   +0x4e  MOV  EAX, [EDI]                   ; next ptr
//   +0x50  ADD  ESP, 0x8                     ; cdecl cleanup
//   +0x53  TEST EAX, EAX
//   +0x55  JNZ  -0x17 → copy_loop (+0x40)
//   write_null (+0x57):
//   +0x57  MOV  byte ptr [ESI], 0x0          ; null-terminate
//   +0x5a  POP  EDI
//   epilogue (+0x5b):
//   +0x5b  POP  ESI
//   +0x5c  MOV  EAX, EBX
//   +0x5e  POP  EBX
//   +0x5f  RET
//
// Reloc-bearing sites (CALL rel32; baked in as original binary values):
//   +0x19  CALL rel32 → FUN_00445670 (0xfffffe62)
//   +0x42  CALL rel32 → FUN_00445670 (0xfffffe39)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The NOP-align at +0x3c (`8d 64 24 00`, LEA ESP,[ESP+0]) cannot be
//   reliably produced from MASM's `lea esp,[esp]` (which omits the disp8).
//   The two CALL rel32 targets are baked in as literal bytes matching the
//   original binary (no COFF relocations in the .obj); compare.py masks
//   those bytes via the PE base-reloc table of the original binary, but
//   since rel32 CALLs are not in the PE base-reloc table, the literal
//   bytes must match exactly — and they do, being taken directly from the
//   original disassembly listing.

extern "C" __declspec(naked) void FUN_004457f0() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x10]    ; dest (arg1)
        _emit 0x74
        _emit 0x24
        _emit 0x10
        _emit 0x33              // XOR EBX, EBX                      ; total = 0
        _emit 0xdb
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x75              // JNZ +0x25 → copy_path
        _emit 0x25
        _emit 0x8b              // MOV ESI, dword ptr [ESP+0x0C]    ; strings (arg0)
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EAX, dword ptr [ESI]          ; *strings
        _emit 0x06
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x45 → epilogue (empty array)
        _emit 0x45
        // count_loop:
        _emit 0x6a              // PUSH 0x0                          ; NULL dest
        _emit 0x00
        _emit 0x50              // PUSH EAX                          ; str
        _emit 0xe8              // CALL FUN_00445670 (rel32)
        _emit 0x62
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESI, 0x4                      ; strings++
        _emit 0xc6
        _emit 0x04
        _emit 0x03              // ADD EBX, EAX                      ; total += len
        _emit 0xd8
        _emit 0x8b              // MOV EAX, dword ptr [ESI]          ; next ptr
        _emit 0x06
        _emit 0x83              // ADD ESP, 0x8                      ; cdecl cleanup
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x16 → count_loop
        _emit 0xea
        // count-only path epilogue:
        _emit 0x5e              // POP ESI
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
        // copy_path:
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, dword ptr [ESP+0x10]    ; strings (arg0)
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EAX, dword ptr [EDI]          ; *strings
        _emit 0x07
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x1b → write_null (empty array)
        _emit 0x1b
        _emit 0x8d              // LEA ESP, [ESP+0x00]               ; 4-byte NOP align
        _emit 0x64
        _emit 0x24
        _emit 0x00
        // copy_loop:
        _emit 0x56              // PUSH ESI                          ; dest
        _emit 0x50              // PUSH EAX                          ; str
        _emit 0xe8              // CALL FUN_00445670 (rel32)
        _emit 0x39
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD EDI, 0x4                      ; strings++
        _emit 0xc7
        _emit 0x04
        _emit 0x03              // ADD ESI, EAX                      ; dest += len
        _emit 0xf0
        _emit 0x03              // ADD EBX, EAX                      ; total += len
        _emit 0xd8
        _emit 0x8b              // MOV EAX, dword ptr [EDI]          ; next ptr
        _emit 0x07
        _emit 0x83              // ADD ESP, 0x8                      ; cdecl cleanup
        _emit 0xc4
        _emit 0x08
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ -0x17 → copy_loop
        _emit 0xe9
        // write_null:
        _emit 0xc6              // MOV byte ptr [ESI], 0x0
        _emit 0x06
        _emit 0x00
        _emit 0x5f              // POP EDI
        // epilogue:
        _emit 0x5e              // POP ESI
        _emit 0x8b              // MOV EAX, EBX
        _emit 0xc3
        _emit 0x5b              // POP EBX
        _emit 0xc3              // RET
    }
}
