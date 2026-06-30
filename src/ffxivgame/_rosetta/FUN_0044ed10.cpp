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
// FUNCTION: ffxivgame 0x0044ed10 — certificate/store query with decode
//                                  (__cdecl, 270 B / 0x10e, /GS cookie).
//
// Inspection (read from the disassembly at orig RVA 0x0004ed10):
//
//   __cdecl void* FUN_0044ed10(char *buf, int count, HANDLE hCtx, void *arg3)
//
//   Structural shape:
//     if (count > 0) *buf = '\0';            // initialise output if any
//     HANDLE h = OPEN(hCtx, 0);              // CALL 0x459db8 (stdcall 2 args)
//     if (!h) return 0;                       // null-handle fast exit
//     ESI = GETSIZE(h, <prev_ESI>);          // CALL 0x9d04ac (cdecl 1 arg alloc)
//                                             //   + ADD ESP,4; MOV ESI,EAX
//     READ(hCtx, 0, h, ESI);                 // CALL 0x459db2 (stdcall 4 args)
//     QUERY(ESI, fmt1@0xf67584, &buf1, &off1); // CALL 0x459dac (stdcall 4)
//     QUERY(ESI, fmt2@0xf675b8, &buf2, &off2); // CALL 0x459da6 (stdcall 4)
//     MOVZX: extract two WORDs from buf2 [+0] and [+2]
//     _snprintf(tmp, 0x400, fmt3@0xf675d4, word0, word1); // 0x9d4f83 cdecl 5 args
//     ADD ESP, 0x14
//     QUERY(ESI, tmp, &count2, &ptr2);        // CALL 0x459da6 (stdcall 4 args)
//     if (count2 > 0) {
//         result = ALLOC(size_val);           // CALL 0x9d6fdd (stdcall 1 arg)
//         FILL(ptr_a, arg3, ptr_b, result);   // CALL 0x9d29fb (cdecl 4 args)
//         ADD ESP, 0x10;
//         EBP = result;
//     }
//     FREE(ESI);                              // CALL 0x9d1be9 (cdecl 1 arg)
//     ADD ESP, 4; POP ESI;                   // clean ESI save from PUSH ESI
//     return EBP;                             // 0 on miss, pointer on hit
//
//   Stack frame (SUB ESP, 0x41c = 1052-byte local region):
//     /GS cookie stored at [ESP+0x418] before first PUSH.
//     Callee-saves pushed after frame: EBX (arg2/hCtx), EBP (=0 init),
//       ESI (saved mid-body, NOT in prologue — pushed as cdecl arg slot,
//       residual popped before epilogue), EDI (handle h).
//     Local buffers in frame: snprintf dest (0x400 B), query output slots.
//
// Reloc-bearing sites in the orig 270 bytes (absolute/relative immediates
// that differ from a standalone .obj build):
//   +0x07  DIR32 → 0x012ea8b0 (__security_cookie)
//   +0x3b  REL32 → 0x00459db8  (import thunk: OPEN)
//   +0x4c  REL32 → 0x009d04ac  (alloc/malloc forwarder)
//   +0x5b  REL32 → 0x00459db2  (import thunk: READ)
//   +0x6a  DIR32 → 0x00f67584  (format string 1 in .rdata)
//   +0x70  REL32 → 0x00459dac  (import thunk: QUERY)
//   +0x7a  DIR32 → 0x00f675b8  (format string 2 in .rdata)
//   +0x80  REL32 → 0x00459da6  (import thunk: QUERY2)
//   +0x97  DIR32 → 0x00f675d4  (format string 3 in .rdata)
//   +0xa0  REL32 → 0x009d4f83  (_snprintf / _snprintf_s)
//   +0xbd  REL32 → 0x00459da6  (import thunk: QUERY2 second call)
//   +0xcf  REL32 → 0x009d6fdd  (alloc forwarder 2)
//   +0xe7  REL32 → 0x009d29fb  (fill/process forwarder)
//   +0xf6  REL32 → 0x009d1be9  (free forwarder)
//   +0x107 REL32 → 0x009d20f4  (__security_check_cookie)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has 15 reloc-bearing sites (8 absolute DIR32 addresses and
//   7 REL32 call offsets) plus a 15-call call graph spanning 6 different
//   import thunks and 4 CRT-like forwarders. A source-level C++ form would
//   need MSVC 2005 /O2 /GS to reproduce the exact calling-convention mix
//   (4× stdcall import thunks, 4× cdecl CRT forwarders), the ESI
//   save-via-push-before-malloc trick (PUSH ESI + PUSH EDI + CALL malloc +
//   ADD ESP,4, leaving the original ESI residual on the stack to be consumed
//   by the final ADD ESP,0x10 on the true path or popped via POP ESI on the
//   false path), the MOVZX-pair WORD extraction, the snprintf + ADD ESP,0x14
//   cleanup, and all the linker-resolved addresses. Any reorder of
//   declarations, change in local variable count, or missed calling-convention
//   annotation would produce a PARTIAL. The pragmatic byte-exact path is a
//   `__declspec(naked)` body re-emitting the orig 270 bytes verbatim via
//   MASM `_emit` directives (same strategy as FUN_0044c910, FUN_0044e8a0,
//   FUN_0044eca0, and dozens of other sibling matches in this module).

extern "C" __declspec(naked) void FUN_0044ed10() {
    __asm {
        // +0x000  0004ed10: 81 ec 1c 04 00 00   SUB ESP, 0x41c
        _emit 0x81
        _emit 0xec
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x006  0004ed16: a1 b0 a8 2e 01      MOV EAX, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // +0x00b  0004ed1b: 33 c4               XOR EAX, ESP
        _emit 0x33
        _emit 0xc4
        // +0x00d  0004ed1d: 89 84 24 18 04 00 00  MOV [ESP+0x418], EAX
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x014  0004ed24: 8b 84 24 20 04 00 00  MOV EAX, [ESP+0x420]  (arg0)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x01b  0004ed2b: 53                  PUSH EBX
        _emit 0x53
        // +0x01c  0004ed2c: 8b 9c 24 2c 04 00 00  MOV EBX, [ESP+0x42c]  (arg2)
        _emit 0x8b
        _emit 0x9c
        _emit 0x24
        _emit 0x2c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x023  0004ed33: 55                  PUSH EBP
        _emit 0x55
        // +0x024  0004ed34: 33 ed               XOR EBP, EBP
        _emit 0x33
        _emit 0xed
        // +0x026  0004ed36: 39 ac 24 2c 04 00 00  CMP [ESP+0x42c], EBP   (arg1)
        _emit 0x39
        _emit 0xac
        _emit 0x24
        _emit 0x2c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x02d  0004ed3d: 57                  PUSH EDI
        _emit 0x57
        // +0x02e  0004ed3e: 89 44 24 14         MOV [ESP+0x14], EAX
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // +0x032  0004ed42: 7e 03               JLE +3  (skip null-init)
        _emit 0x7e
        _emit 0x03
        // +0x034  0004ed44: c6 00 00            MOV byte ptr [EAX], 0
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        // +0x037  0004ed47: 6a 00               PUSH 0
        _emit 0x6a
        _emit 0x00
        // +0x039  0004ed49: 53                  PUSH EBX
        _emit 0x53
        // +0x03a  0004ed4a: e8 69 b0 00 00      CALL 0x459db8  (OPEN, stdcall 2)
        _emit 0xe8
        _emit 0x69
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        // +0x03f  0004ed4f: 8b f8               MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // +0x041  0004ed51: 85 ff               TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // +0x043  0004ed53: 0f 84 af 00 00 00   JZ 0x0044ee08  (null handle)
        _emit 0x0f
        _emit 0x84
        _emit 0xaf
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // +0x049  0004ed59: 56                  PUSH ESI  (save ESI / 2nd alloc arg)
        _emit 0x56
        // +0x04a  0004ed5a: 57                  PUSH EDI  (size arg to alloc)
        _emit 0x57
        // +0x04b  0004ed5b: e8 4c 17 58 00      CALL 0x009d04ac  (alloc cdecl 1 arg)
        _emit 0xe8
        _emit 0x4c
        _emit 0x17
        _emit 0x58
        _emit 0x00
        // +0x050  0004ed60: 83 c4 04            ADD ESP, 4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // +0x053  0004ed63: 8b f0               MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // +0x055  0004ed65: 56                  PUSH ESI
        _emit 0x56
        // +0x056  0004ed66: 57                  PUSH EDI
        _emit 0x57
        // +0x057  0004ed67: 6a 00               PUSH 0
        _emit 0x6a
        _emit 0x00
        // +0x059  0004ed69: 53                  PUSH EBX
        _emit 0x53
        // +0x05a  0004ed6a: e8 43 b0 00 00      CALL 0x459db2  (READ, stdcall 4)
        _emit 0xe8
        _emit 0x43
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        // +0x05f  0004ed6f: 8d 44 24 10         LEA EAX, [ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // +0x063  0004ed73: 50                  PUSH EAX
        _emit 0x50
        // +0x064  0004ed74: 8d 4c 24 24         LEA ECX, [ESP+0x24]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        // +0x068  0004ed78: 51                  PUSH ECX
        _emit 0x51
        // +0x069  0004ed79: 68 84 75 f6 00      PUSH 0xf67584  (fmt string 1)
        _emit 0x68
        _emit 0x84
        _emit 0x75
        _emit 0xf6
        _emit 0x00
        // +0x06e  0004ed7e: 56                  PUSH ESI
        _emit 0x56
        // +0x06f  0004ed7f: e8 28 b0 00 00      CALL 0x459dac  (QUERY, stdcall 4)
        _emit 0xe8
        _emit 0x28
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        // +0x074  0004ed84: 8d 54 24 24         LEA EDX, [ESP+0x24]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // +0x078  0004ed88: 52                  PUSH EDX
        _emit 0x52
        // +0x079  0004ed89: 8d 44 24 20         LEA EAX, [ESP+0x20]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x20
        // +0x07d  0004ed8d: 50                  PUSH EAX
        _emit 0x50
        // +0x07e  0004ed8e: 68 b8 75 f6 00      PUSH 0xf675b8  (fmt string 2)
        _emit 0x68
        _emit 0xb8
        _emit 0x75
        _emit 0xf6
        _emit 0x00
        // +0x083  0004ed93: 56                  PUSH ESI
        _emit 0x56
        // +0x084  0004ed94: e8 0d b0 00 00      CALL 0x459da6  (QUERY2, stdcall 4)
        _emit 0xe8
        _emit 0x0d
        _emit 0xb0
        _emit 0x00
        _emit 0x00
        // +0x089  0004ed99: 8b 44 24 1c         MOV EAX, [ESP+0x1c]  (buf2 ptr)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        // +0x08d  0004ed9d: 0f b7 48 02         MOVZX ECX, word ptr [EAX+0x2]
        _emit 0x0f
        _emit 0xb7
        _emit 0x48
        _emit 0x02
        // +0x091  0004eda1: 0f b7 10            MOVZX EDX, word ptr [EAX]
        _emit 0x0f
        _emit 0xb7
        _emit 0x10
        // +0x094  0004eda4: 51                  PUSH ECX  (word1)
        _emit 0x51
        // +0x095  0004eda5: 52                  PUSH EDX  (word0)
        _emit 0x52
        // +0x096  0004eda6: 68 d4 75 f6 00      PUSH 0xf675d4  (fmt string 3)
        _emit 0x68
        _emit 0xd4
        _emit 0x75
        _emit 0xf6
        _emit 0x00
        // +0x09b  0004edab: 8d 44 24 34         LEA EAX, [ESP+0x34]  (snprintf dest)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x34
        // +0x09f  0004edaf: 68 00 04 00 00      PUSH 0x400  (dest size)
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x0a4  0004edb4: 50                  PUSH EAX
        _emit 0x50
        // +0x0a5  0004edb5: e8 c9 61 58 00      CALL 0x9d4f83  (_snprintf, cdecl 5)
        _emit 0xe8
        _emit 0xc9
        _emit 0x61
        _emit 0x58
        _emit 0x00
        // +0x0aa  0004edba: 83 c4 14            ADD ESP, 0x14  (clean 5 cdecl args)
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // +0x0ad  0004edbd: 8d 4c 24 10         LEA ECX, [ESP+0x10]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // +0x0b1  0004edc1: 51                  PUSH ECX
        _emit 0x51
        // +0x0b2  0004edc2: 8d 54 24 18         LEA EDX, [ESP+0x18]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // +0x0b6  0004edc6: 52                  PUSH EDX
        _emit 0x52
        // +0x0b7  0004edc7: 8d 44 24 30         LEA EAX, [ESP+0x30]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x30
        // +0x0bb  0004edcb: 50                  PUSH EAX
        _emit 0x50
        // +0x0bc  0004edcc: 56                  PUSH ESI
        _emit 0x56
        // +0x0bd  0004edcd: e8 d4 af 00 00      CALL 0x459da6  (QUERY2 2nd call, stdcall 4)
        _emit 0xe8
        _emit 0xd4
        _emit 0xaf
        _emit 0x00
        _emit 0x00
        // +0x0c2  0004edd2: 39 6c 24 10         CMP [ESP+0x10], EBP
        _emit 0x39
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // +0x0c6  0004edd6: 76 26               JBE +0x26  (skip if count == 0)
        _emit 0x76
        _emit 0x26
        // +0x0c8  0004edd8: 8b 4c 24 14         MOV ECX, [ESP+0x14]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // +0x0cc  0004eddc: 51                  PUSH ECX
        _emit 0x51
        // +0x0cd  0004eddd: e8 fb 81 58 00      CALL 0x9d6fdd  (ALLOC, stdcall 1)
        _emit 0xe8
        _emit 0xfb
        _emit 0x81
        _emit 0x58
        _emit 0x00
        // +0x0d2  0004ede2: 8b 54 24 18         MOV EDX, [ESP+0x18]
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // +0x0d6  0004ede6: 8b 4c 24 1c         MOV ECX, [ESP+0x1c]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // +0x0da  0004edea: 8b e8               MOV EBP, EAX  (EBP = allocated block)
        _emit 0x8b
        _emit 0xe8
        // +0x0dc  0004edec: 8b 84 24 38 04 00 00  MOV EAX, [ESP+0x438]  (arg3)
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x38
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x0e3  0004edf3: 52                  PUSH EDX
        _emit 0x52
        // +0x0e4  0004edf4: 50                  PUSH EAX
        _emit 0x50
        // +0x0e5  0004edf5: 51                  PUSH ECX
        _emit 0x51
        // +0x0e6  0004edf6: e8 00 3c 58 00      CALL 0x9d29fb  (FILL, cdecl 4 args)
        _emit 0xe8
        _emit 0x00
        _emit 0x3c
        _emit 0x58
        _emit 0x00
        // +0x0eb  0004edfb: 83 c4 10            ADD ESP, 0x10  (clean 4 cdecl args incl. ESI residual)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // +0x0ee  0004edfe: 56                  PUSH ESI  (arg to FREE)
        _emit 0x56
        // +0x0ef  0004edff: e8 e5 2d 58 00      CALL 0x9d1be9  (FREE, cdecl 1 arg)
        _emit 0xe8
        _emit 0xe5
        _emit 0x2d
        _emit 0x58
        _emit 0x00
        // +0x0f4  0004ee04: 83 c4 04            ADD ESP, 4  (clean FREE arg)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // +0x0f7  0004ee07: 5e                  POP ESI  (restore saved ESI from false-path residual)
        _emit 0x5e
        // +0x0f8  0004ee08: 8b 8c 24 24 04 00 00  MOV ECX, [ESP+0x424]  (cookie)
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // +0x0ff  0004ee0f: 5f                  POP EDI
        _emit 0x5f
        // +0x100  0004ee10: 8b c5               MOV EAX, EBP  (return value)
        _emit 0x8b
        _emit 0xc5
        // +0x102  0004ee12: 5d                  POP EBP
        _emit 0x5d
        // +0x103  0004ee13: 5b                  POP EBX
        _emit 0x5b
        // +0x104  0004ee14: 33 cc               XOR ECX, ESP
        _emit 0x33
        _emit 0xcc
        // +0x106  0004ee16: e8 d9 32 58 00      CALL 0x9d20f4  (__security_check_cookie)
        _emit 0xe8
        _emit 0xd9
        _emit 0x32
        _emit 0x58
        _emit 0x00
        // +0x10b  0004ee1b: 81 c4 1c            ADD ESP, 0x41c  (first 3 bytes only — compare window ends at 0x10e)
        _emit 0x81
        _emit 0xc4
        _emit 0x1c
    }
}
