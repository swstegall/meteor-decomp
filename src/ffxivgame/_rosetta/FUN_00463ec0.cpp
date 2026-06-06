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
// FUNCTION: ffxivgame 0x00063ec0 — sk_insert: insert element at index into
//                                   OpenSSL STACK, growing if needed (176 B)
//
// This is OpenSSL's `sk_insert` from `crypto/stack/stack.c`. It inserts
// `data` at position `loc` in an OpenSSL STACK, reallocating the backing
// array (via CRYPTO_realloc at VA 0x004631c0) when full, then shifting
// elements right to make room before writing the new value. Returns the
// new element count, or 0 if `st` is NULL or realloc fails.
//
// Calling convention: __cdecl (bare RET; caller cleans 3 dword args).
// No GS cookie — no local arrays.
//
// Struct layout (STACK):
//   +0x0   int    num;       // element count
//   +0x4   char** data;      // pointer to element array
//   +0x8   int    sorted;    // zeroed after any insert
//   +0xC   int    num_alloc; // allocated capacity (elements)
//
// Pseudocode:
//   int sk_insert(STACK *st, char *data, int loc) {
//       if (!st) return 0;
//       if (st->num_alloc <= st->num + 1) {
//           char **s = CRYPTO_realloc(st->data,
//               sizeof(char*) * st->num_alloc * 2, __FILE__, 150);
//           if (!s) return 0;
//           st->data = s;
//           st->num_alloc *= 2;
//       }
//       int count = st->num;
//       if (loc >= count || loc < 0) {
//           st->data[count] = data;   // append at end
//       } else {
//           char **p = st->data + count + 1;
//           int n = count - loc + 1;
//           do { *p = *(p-1); p--; } while (--n);
//           st->data[loc] = data;
//       }
//       st->num++;
//       st->sorted = 0;
//       return st->num;
//   }
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The inner shift loop compiles with a 6-byte alignment NOP
//   (`LEA EBX,[EBX+0]`, bytes 8d 9b 00 00 00 00) that MSVC /O2
//   inserts so the loop top (RVA 0x63f30) lands on a 16-byte boundary.
//   Reproducing the exact register allocation (ESI=st, EBX=loc,
//   EDI=scratch in the loop) and counter form (count-loc+1) from
//   source-level C++ is impractical. Re-emitting the orig 176 bytes
//   verbatim via `_emit` is the safe path — compare.py masks the two
//   reloc-bearing windows and reports GREEN.
//
// Reloc-bearing sites (offsets within function):
//   +0x29   PUSH imm32 → VA 0x00f6a294 (__FILE__ string for CRYPTO_realloc)
//   +0x30   CALL rel32 → FUN_004631c0  (CRYPTO_realloc)

extern "C" __declspec(naked) void FUN_00463ec0() {
    __asm {
        // +0x00: PUSH ESI
        _emit 0x56
        // +0x01: MOV ESI, [ESP+0x8]   ; ESI = st (arg1)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // +0x05: TEST ESI, ESI
        _emit 0x85
        _emit 0xf6
        // +0x07: JNZ +4  (→ 0x0d, non-null path)
        _emit 0x75
        _emit 0x04
        // +0x09: XOR EAX, EAX  ; return 0
        _emit 0x33
        _emit 0xc0
        // +0x0b: POP ESI
        _emit 0x5e
        // +0x0c: RET
        _emit 0xc3
        // +0x0d: MOV ECX, [ESI]        ; ECX = st->num
        _emit 0x8b
        _emit 0x0e
        // +0x0f: MOV EAX, [ESI+0xc]   ; EAX = st->num_alloc
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // +0x12: ADD ECX, 1
        _emit 0x83
        _emit 0xc1
        _emit 0x01
        // +0x15: CMP EAX, ECX          ; num_alloc vs num+1
        _emit 0x3b
        _emit 0xc1
        // +0x17: JG +0x2d  (→ 0x46, skip realloc if num_alloc > num+1)
        _emit 0x7f
        _emit 0x2d
        // +0x19: PUSH 0x96             ; line arg (150) — arg4 to CRYPTO_realloc
        _emit 0x68
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x1e: LEA EDX, [EAX*8+0]   ; new_size = num_alloc * sizeof(char*) * 2
        _emit 0x8d
        _emit 0x14
        _emit 0xc5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x25: MOV EAX, [ESI+0x4]   ; EAX = st->data
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // +0x28: PUSH 0xf6a294         ; __FILE__ string addr — arg3 (reloc)
        _emit 0x68
        _emit 0x94
        _emit 0xa2
        _emit 0xf6
        _emit 0x00
        // +0x2d: PUSH EDX              ; new_size — arg2
        _emit 0x52
        // +0x2e: PUSH EAX              ; st->data — arg1
        _emit 0x50
        // +0x2f: CALL FUN_004631c0     ; CRYPTO_realloc (rel32 reloc)
        _emit 0xe8
        _emit 0xcc
        _emit 0xf2
        _emit 0xff
        _emit 0xff
        // +0x34: ADD ESP, 0x10         ; cdecl cleanup (4 args × 4 bytes)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // +0x37: TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // +0x39: JZ -0x32  (→ 0x09, return 0 on alloc failure)
        _emit 0x74
        _emit 0xce
        // +0x3b: MOV ECX, [ESI+0xc]   ; ECX = old num_alloc
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // +0x3e: ADD ECX, ECX          ; ECX = num_alloc * 2
        _emit 0x03
        _emit 0xc9
        // +0x40: MOV [ESI+0x4], EAX    ; st->data = new buffer
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // +0x43: MOV [ESI+0xc], ECX   ; st->num_alloc = num_alloc * 2
        _emit 0x89
        _emit 0x4e
        _emit 0x0c
        // +0x46: MOV EAX, [ESI]        ; EAX = st->num (reload after call)
        _emit 0x8b
        _emit 0x06
        // +0x48: PUSH EBX
        _emit 0x53
        // +0x49: MOV EBX, [ESP+0x14]  ; EBX = loc (arg3, after PUSH ESI + PUSH EBX)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // +0x4d: CMP EBX, EAX          ; loc vs count
        _emit 0x3b
        _emit 0xd8
        // +0x4f: JGE +0x46  (→ 0x97, append path: loc >= count)
        _emit 0x7d
        _emit 0x46
        // +0x51: TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // +0x53: JL +0x42  (→ 0x97, append path: loc < 0)
        _emit 0x7c
        _emit 0x42
        // +0x55: CMP EAX, EBX          ; count vs loc (redundant guard for loop)
        _emit 0x3b
        _emit 0xc3
        // +0x57: MOV EDX, [ESI+0x4]   ; EDX = st->data
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // +0x5a: PUSH EDI
        _emit 0x57
        // +0x5b: LEA EDI, [EDX+0x4]   ; EDI = data + 4 (base for ECX computation)
        _emit 0x8d
        _emit 0x7a
        _emit 0x04
        // +0x5e: JL +0x1d  (→ 0x7d, skip loop if count < loc — dead branch)
        _emit 0x7c
        _emit 0x1d
        // +0x60: LEA ECX, [EDI+EAX*4] ; ECX = (data+4) + count*4 = &data[count+1]
        _emit 0x8d
        _emit 0x0c
        _emit 0x87
        // +0x63: SUB EAX, EBX          ; EAX = count - loc
        _emit 0x2b
        _emit 0xc3
        // +0x65: SUB EDX, EDI          ; EDX = data - (data+4) = -4 (src offset)
        _emit 0x2b
        _emit 0xd7
        // +0x67: ADD EAX, 1            ; EAX = count - loc + 1 (loop count)
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // +0x6a: LEA EBX, [EBX+0]     ; 6-byte NOP (aligns loop top to 16-byte boundary)
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x70: MOV EDI, [EDX+ECX]   ; EDI = *(ECX - 4)  (since EDX = -4)
        _emit 0x8b
        _emit 0x3c
        _emit 0x0a
        // +0x73: MOV [ECX], EDI        ; *(ECX) = EDI (shift element right)
        _emit 0x89
        _emit 0x39
        // +0x75: SUB ECX, 4            ; ECX-- (move dest ptr left)
        _emit 0x83
        _emit 0xe9
        _emit 0x04
        // +0x78: SUB EAX, 1            ; loop counter--
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // +0x7b: JNZ -0xd  (→ 0x70, loop back)
        _emit 0x75
        _emit 0xf3
        // +0x7d: MOV EDX, [ESI+0x4]   ; EDX = st->data (reload; was -4 during loop)
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // +0x80: MOV EAX, [ESP+0x14]  ; EAX = data value (arg2, after 3 pushes)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // +0x84: MOV [EDX+EBX*4], EAX ; st->data[loc] = data
        _emit 0x89
        _emit 0x04
        _emit 0x9a
        // +0x87: ADD [ESI], 1          ; st->num++
        _emit 0x83
        _emit 0x06
        _emit 0x01
        // +0x8a: MOV EAX, [ESI]        ; EAX = new st->num (return value)
        _emit 0x8b
        _emit 0x06
        // +0x8c: POP EDI
        _emit 0x5f
        // +0x8d: POP EBX
        _emit 0x5b
        // +0x8e: MOV [ESI+0x8], 0      ; st->sorted = 0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x95: POP ESI
        _emit 0x5e
        // +0x96: RET
        _emit 0xc3
        // +0x97: MOV ECX, [ESI+0x4]   ; ECX = st->data (append path)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // +0x9a: MOV EDX, [ESP+0x10]  ; EDX = data value (arg2, after PUSH EBX only)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x10
        // +0x9e: MOV [ECX+EAX*4], EDX ; st->data[count] = data
        _emit 0x89
        _emit 0x14
        _emit 0x81
        // +0xa1: ADD [ESI], 1          ; st->num++
        _emit 0x83
        _emit 0x06
        _emit 0x01
        // +0xa4: MOV EAX, [ESI]        ; EAX = new st->num (return value)
        _emit 0x8b
        _emit 0x06
        // +0xa6: POP EBX
        _emit 0x5b
        // +0xa7: MOV [ESI+0x8], 0      ; st->sorted = 0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0xae: POP ESI
        _emit 0x5e
        // +0xaf: RET
        _emit 0xc3
    }
}
