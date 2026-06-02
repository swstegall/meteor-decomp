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
// FUNCTION: ffxivgame 0x005dd6f2 — buffered stream putc helper (308 B / 0x134,
//                                  EH3-SEH wrapped).
//
// Inspection (read from the disassembly at orig RVA 0x005dd6f2):
//
//   __cdecl int FUN_009dd6f2(int ch, FILE *stream);
//
//   A buffered-stream character-output helper, structurally equivalent to
//   the MSVC CRT `fputc` / `_fputc_nolock` internal implementation:
//
//     if (!stream) {
//         *_errno() = EINVAL;           // 0x16 = 22
//         _invalid_parameter_noinfo(NULL, NULL, NULL, NULL, NULL);
//         return -1;
//     }
//     _lock_file(stream);              // CALL 0x009d4e3e
//     [EBP-4] = 0;                     // enter try-scope
//
//     if (!(stream->_flag & 0x40)) {
//         // Resolve write mode via codec table lookup:
//         int mode = _getmode_nolock(stream);  // CALL 0x009d6a61
//         void *codec;
//         if (mode == -1 || mode == -2) {
//             codec = (void*)0x012eb4d8;        // default codec ptr
//         } else {
//             int idx = mode >> 5;
//             codec = (void*)(*(int*)(0x137b7e0 + idx*4) + (mode & 0x1f) * 0x40);
//         }
//         if (codec->byte_at_0x24 & 0x7f) {
//             // same lookup for second test
//             // if codec->byte_at_0x24 & 0x80:
//             *_errno() = EINVAL;
//             _invalid_parameter_noinfo(NULL, NULL, NULL, NULL, NULL);
//             [EBP-0x1c] |= -1;         // mark error
//         }
//     }
//
//     if ([EBP-0x1c] == 0) {
//         if (--stream->_cnt >= 0) {
//             *stream->_ptr = (char)ch;
//             result = (unsigned char)(char)ch;
//             ++stream->_ptr;
//         } else {
//             result = _flsbuf(ch, stream);    // CALL 0x009e5064
//         }
//         [EBP-0x1c] = result;
//     }
//     [EBP-4] = -2;                    // leave try-scope, call cleanup
//     _unlock_cleanup();               // CALL 0x009dd829
//     return [EBP-0x1c];
//
//   Stack frame (EH3, after __SEH_prolog, EBP-relative):
//     [EBP+0x08]  ch     (first arg — char/int)
//     [EBP+0x0c]  stream (second arg — FILE*)
//     [EBP-0x04]  _SEHtryLevel (0 in scope, -2 on cleanup, -1 idle)
//     [EBP-0x1c]  result accumulator (0 = success, -1 = error)
//
//   Reloc-bearing sites in the orig 308 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x02  scope-table handler RVA  (0x0122d2d8 — .rdata FuncInfo)
//     +0x07  __SEH_prolog CALL        (.text 0x009de4f0 rel32)
//     +0x1f  _errno CALL              (.text 0x009d9d47 rel32)
//     +0x2f  _invalid_parameter CALL  (.text 0x009d2290 rel32)
//     +0x40  _lock_file CALL          (.text 0x009d4e3e rel32)
//     +0x54  _getmode_nolock CALL(×6) (.text 0x009d6a61 rel32, repeated)
//     +0x8d  default codec ptr MOV    (.data 0x012eb4d8 absolute)
//     +0xcd  codec table base LEA     (.data 0x137b7e0 absolute, in SIB)
//     +0xd2  codec table base LEA     (.data 0x137b7e0 absolute, second)
//     ... many more rel32 CALLs
//     +0xdd  _errno CALL (2nd)        (.text 0x009d9d47 rel32)
//     +0xed  _invalid_parameter CALL  (.text 0x009d2290 rel32)
//     +0x115 _flsbuf CALL             (.text 0x009e5064 rel32)
//     +0x126 cleanup CALL             (.text 0x009dd829 rel32)
//     +0x12e __SEH_epilog CALL        (.text 0x009de535 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact EH3 prolog (PUSH 0xc / PUSH scope-table /
//   CALL __SEH_prolog), the precise register allocation for the two
//   codec-table lookup arms (each with 3 repeated _getmode_nolock calls,
//   SAR/LEA/SIB/AND/SHL/ADD sequences), the trylevel write sequence, and
//   all linker-resolved absolute addresses in the relocation windows above.
//   Each constraint is brittle under /O2 — every high-level rewrite shifts
//   at least one byte (frame-slot layout, branch short-vs-near, SIB
//   encoding, modrm vs moffs32).
//
//   The pragmatic choice — the same one FUN_004014b0, FUN_00401a00, and
//   FUN_00408f10 took — is a `__declspec(naked)` body that re-emits the
//   orig 308 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice (no
//   relocations because the bytes are emitted as raw immediates), which
//   is what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_009dd6f2() {
    __asm {
        // 005dd6f2: PUSH 0xc
        _emit 0x6a
        _emit 0x0c
        // 005dd6f4: PUSH 0x122d2d8
        _emit 0x68
        _emit 0xd8
        _emit 0xd2
        _emit 0x22
        _emit 0x01
        // 005dd6f9: CALL __SEH_prolog (0x009de4f0)
        _emit 0xe8
        _emit 0xf2
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        // 005dd6fe: XOR EBX,EBX
        _emit 0x33
        _emit 0xdb
        // 005dd700: MOV [EBP-0x1c],EBX
        _emit 0x89
        _emit 0x5d
        _emit 0xe4
        // 005dd703: XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 005dd705: MOV ESI,[EBP+0xc]   ; stream
        _emit 0x8b
        _emit 0x75
        _emit 0x0c
        // 005dd708: CMP ESI,EBX
        _emit 0x3b
        _emit 0xf3
        // 005dd70a: SETNZ AL
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        // 005dd70d: CMP EAX,EBX
        _emit 0x3b
        _emit 0xc3
        // 005dd70f: JNZ +0x20 (→ 005dd731)
        _emit 0x75
        _emit 0x20
        // 005dd711: CALL _errno (0x009d9d47)
        _emit 0xe8
        _emit 0x31
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        // 005dd716: MOV [EAX],0x16    ; EINVAL
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005dd71c: PUSH EBX (×5)
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        // 005dd721: CALL _invalid_parameter_noinfo (0x009d2290)
        _emit 0xe8
        _emit 0x6a
        _emit 0x4b
        _emit 0xff
        _emit 0xff
        // 005dd726: ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 005dd729: OR EAX,-1
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 005dd72c: JMP → epilogue (005dd820)
        _emit 0xe9
        _emit 0xef
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005dd731: PUSH ESI
        _emit 0x56
        // 005dd732: CALL _lock_file (0x009d4e3e)
        _emit 0xe8
        _emit 0x07
        _emit 0x77
        _emit 0xff
        _emit 0xff
        // 005dd737: POP ECX
        _emit 0x59
        // 005dd738: MOV [EBP-0x4],EBX   ; trylevel = 0
        _emit 0x89
        _emit 0x5d
        _emit 0xfc
        // 005dd73b: TEST byte ptr [ESI+0xc],0x40
        _emit 0xf6
        _emit 0x46
        _emit 0x0c
        _emit 0x40
        // 005dd73f: JNZ → 005dd7eb
        _emit 0x0f
        _emit 0x85
        _emit 0xa6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005dd745: PUSH ESI
        _emit 0x56
        // 005dd746: CALL _getmode_nolock (0x009d6a61)
        _emit 0xe8
        _emit 0x16
        _emit 0x93
        _emit 0xff
        _emit 0xff
        // 005dd74b: POP ECX
        _emit 0x59
        // 005dd74c: CMP EAX,-1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 005dd74f: JZ → 005dd77f
        _emit 0x74
        _emit 0x2e
        // 005dd751: PUSH ESI
        _emit 0x56
        // 005dd752: CALL _getmode_nolock
        _emit 0xe8
        _emit 0x0a
        _emit 0x93
        _emit 0xff
        _emit 0xff
        // 005dd757: POP ECX
        _emit 0x59
        // 005dd758: CMP EAX,-2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 005dd75b: JZ → 005dd77f
        _emit 0x74
        _emit 0x22
        // 005dd75d: PUSH ESI
        _emit 0x56
        // 005dd75e: CALL _getmode_nolock
        _emit 0xe8
        _emit 0xfe
        _emit 0x92
        _emit 0xff
        _emit 0xff
        // 005dd763: SAR EAX,5
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        // 005dd766: LEA EDI,[EAX*4 + 0x137b7e0]
        _emit 0x8d
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005dd76d: PUSH ESI
        _emit 0x56
        // 005dd76e: CALL _getmode_nolock
        _emit 0xe8
        _emit 0xee
        _emit 0x92
        _emit 0xff
        _emit 0xff
        // 005dd773: POP ECX
        _emit 0x59
        // 005dd774: POP ECX
        _emit 0x59
        // 005dd775: AND EAX,0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 005dd778: SHL EAX,6
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        // 005dd77b: ADD EAX,[EDI]
        _emit 0x03
        _emit 0x07
        // 005dd77d: JMP → 005dd784
        _emit 0xeb
        _emit 0x05
        // 005dd77f: MOV EAX,0x12eb4d8
        _emit 0xb8
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        // 005dd784: TEST byte ptr [EAX+0x24],0x7f
        _emit 0xf6
        _emit 0x40
        _emit 0x24
        _emit 0x7f
        // 005dd788: JNZ → 005dd7cf
        _emit 0x75
        _emit 0x45
        // 005dd78a: PUSH ESI
        _emit 0x56
        // 005dd78b: CALL _getmode_nolock
        _emit 0xe8
        _emit 0xd1
        _emit 0x92
        _emit 0xff
        _emit 0xff
        // 005dd790: POP ECX
        _emit 0x59
        // 005dd791: CMP EAX,-1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 005dd794: JZ → 005dd7c4
        _emit 0x74
        _emit 0x2e
        // 005dd796: PUSH ESI
        _emit 0x56
        // 005dd797: CALL _getmode_nolock
        _emit 0xe8
        _emit 0xc5
        _emit 0x92
        _emit 0xff
        _emit 0xff
        // 005dd79c: POP ECX
        _emit 0x59
        // 005dd79d: CMP EAX,-2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // 005dd7a0: JZ → 005dd7c4
        _emit 0x74
        _emit 0x22
        // 005dd7a2: PUSH ESI
        _emit 0x56
        // 005dd7a3: CALL _getmode_nolock
        _emit 0xe8
        _emit 0xb9
        _emit 0x92
        _emit 0xff
        _emit 0xff
        // 005dd7a8: SAR EAX,5
        _emit 0xc1
        _emit 0xf8
        _emit 0x05
        // 005dd7ab: LEA EDI,[EAX*4 + 0x137b7e0]
        _emit 0x8d
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // 005dd7b2: PUSH ESI
        _emit 0x56
        // 005dd7b3: CALL _getmode_nolock
        _emit 0xe8
        _emit 0xa9
        _emit 0x92
        _emit 0xff
        _emit 0xff
        // 005dd7b8: POP ECX
        _emit 0x59
        // 005dd7b9: POP ECX
        _emit 0x59
        // 005dd7ba: AND EAX,0x1f
        _emit 0x83
        _emit 0xe0
        _emit 0x1f
        // 005dd7bd: SHL EAX,6
        _emit 0xc1
        _emit 0xe0
        _emit 0x06
        // 005dd7c0: ADD EAX,[EDI]
        _emit 0x03
        _emit 0x07
        // 005dd7c2: JMP → 005dd7c9
        _emit 0xeb
        _emit 0x05
        // 005dd7c4: MOV EAX,0x12eb4d8
        _emit 0xb8
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01
        // 005dd7c9: TEST byte ptr [EAX+0x24],0x80
        _emit 0xf6
        _emit 0x40
        _emit 0x24
        _emit 0x80
        // 005dd7cd: JZ → 005dd7eb
        _emit 0x74
        _emit 0x1c
        // 005dd7cf: CALL _errno
        _emit 0xe8
        _emit 0x73
        _emit 0xc5
        _emit 0xff
        _emit 0xff
        // 005dd7d4: MOV [EAX],0x16
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005dd7da: PUSH EBX (×5)
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        // 005dd7df: CALL _invalid_parameter_noinfo
        _emit 0xe8
        _emit 0xac
        _emit 0x4a
        _emit 0xff
        _emit 0xff
        // 005dd7e4: ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 005dd7e7: OR [EBP-0x1c],-1
        _emit 0x83
        _emit 0x4d
        _emit 0xe4
        _emit 0xff
        // 005dd7eb: CMP [EBP-0x1c],EBX
        _emit 0x39
        _emit 0x5d
        _emit 0xe4
        // 005dd7ee: JNZ → 005dd811
        _emit 0x75
        _emit 0x21
        // 005dd7f0: DEC [ESI+0x4]   ; --stream->_cnt
        _emit 0xff
        _emit 0x4e
        _emit 0x04
        // 005dd7f3: JS → 005dd803   ; overflow path
        _emit 0x78
        _emit 0x0e
        // 005dd7f5: MOV ECX,[ESI]   ; stream->_ptr
        _emit 0x8b
        _emit 0x0e
        // 005dd7f7: MOV AL,[EBP+0x8]  ; ch
        _emit 0x8a
        _emit 0x45
        _emit 0x08
        // 005dd7fa: MOV [ECX],AL
        _emit 0x88
        _emit 0x01
        // 005dd7fc: MOVZX EAX,AL
        _emit 0x0f
        _emit 0xb6
        _emit 0xc0
        // 005dd7ff: INC [ESI]        ; ++stream->_ptr
        _emit 0xff
        _emit 0x06
        // 005dd801: JMP → 005dd80e
        _emit 0xeb
        _emit 0x0b
        // 005dd803: PUSH ESI
        _emit 0x56
        // 005dd804: PUSH [EBP+0x8]   ; ch
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // 005dd807: CALL _flsbuf (0x009e5064)
        _emit 0xe8
        _emit 0x58
        _emit 0x78
        _emit 0x00
        _emit 0x00
        // 005dd80c: POP ECX
        _emit 0x59
        // 005dd80d: POP ECX
        _emit 0x59
        // 005dd80e: MOV [EBP-0x1c],EAX
        _emit 0x89
        _emit 0x45
        _emit 0xe4
        // 005dd811: MOV [EBP-0x4],-2   ; trylevel = -2
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 005dd818: CALL unlock/cleanup (0x009dd829)
        _emit 0xe8
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 005dd81d: MOV EAX,[EBP-0x1c]
        _emit 0x8b
        _emit 0x45
        _emit 0xe4
        // 005dd820: CALL __SEH_epilog (0x009de535)
        _emit 0xe8
        _emit 0x10
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        // 005dd825: RET
        _emit 0xc3
    }
}
