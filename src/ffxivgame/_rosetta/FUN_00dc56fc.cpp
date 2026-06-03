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
// FUNCTION: ffxivgame 0x009c56fc (VA 0x00dc56fc) — FP-exception mask guard
//                                 (69 B / 0x45), no standard prologue.
//
// Inspection (read from disassembly at RVA 0x009c56fc):
//
//   No prologue/epilogue — the function exits via conditional jumps (JZ, JNZ)
//   or a tail JMP to FUN_00dc6550. No RET instruction.
//
//   Logic:
//     if ([DAT_0137b8e4] == 0) goto exit;      // global SSE-enabled flag
//     // Allocate 8 bytes scratch on stack
//     // Check SSE MXCSR exception mask bits [12:7]
//     STMXCSR [esp+4];                          // store MXCSR
//     EAX = [esp+4] & 0x1F80;                  // mask exception enable bits
//     if (EAX != 0x1F80) goto skip_x87;        // not all masked → skip x87
//     // Check x87 FPU control word exception mask bits [6:0]
//     FNSTCW [esp];                             // store x87 CW
//     AX = [esp] & 0x7F;                       // mask exception bits
//     // (CMP result sets flags for the joint JNZ below)
//     skip_x87:
//     LEA ESP, [ESP+8];                        // restore stack
//     if (ZF == 0) goto exit;                  // either CMP failed → exit
//     jmp FUN_00dc6550;                        // all masks set → handler
//   exit:                                       // VA 0x00dc5741 (next func)
//
// Byte layout (69 / 0x45 = 59 code + 10 CC padding):
//
//   off  bytes                  mnemonic
//   00   83 3d [DIR32] 00       CMP dword ptr [DAT_0137b8e4], 0
//   07   74 3c                  JZ  done          (+0x3c → off 0x45)
//   09   83 ec 08               SUB ESP, 8
//   0c   0f ae 5c 24 04         STMXCSR [ESP+4]
//   11   8b 44 24 04            MOV EAX, [ESP+4]
//   15   25 80 1f 00 00         AND EAX, 0x1f80
//   1a   3d 80 1f 00 00         CMP EAX, 0x1f80
//   1f   75 0f                  JNZ no_x87        (+0x0f → off 0x30)
//   21   d9 3c 24               FNSTCW [ESP]
//   24   66 8b 04 24            MOV AX, [ESP]
//   28   66 83 e0 7f            AND AX, 0x7f
//   2c   66 83 f8 7f            CMP AX, 0x7f
//   30   8d 64 24 08            LEA ESP, [ESP+8]  ← no_x87
//   34   75 0f                  JNZ done          (+0x0f → off 0x45)
//   36   e9 [REL32]             JMP FUN_00dc6550
//   3b   cc×10                  dead bytes (padding to align exit label)
//   45   (done: — exit label, off 0x45 from function start)
//
// Reconstruction strategy — naked asm with symbolic labels:
//
//   The function has no C-level prologue or epilogue and uses two SSE/x87
//   control instructions (STMXCSR, FNSTCW). The naked-asm approach avoids
//   all prologue codegen risks.
//
//   The 10 CC bytes after the JMP are necessary to place the `done:` label at
//   offset 0x45 from the function start, making the JZ (`74 3c`) and JNZ
//   (`75 0f`) short-jump encodings match the original binary exactly.

extern "C" int DAT_0137b8e4;
extern "C" void FUN_00dc6550();

extern "C" __declspec(naked) void FUN_00dc56fc() {
    __asm {
        cmp     dword ptr [DAT_0137b8e4], 0
        jz      done
        sub     esp, 8
        stmxcsr dword ptr [esp+4]
        mov     eax, dword ptr [esp+4]
        and     eax, 0x1f80
        cmp     eax, 0x1f80
        jnz     no_x87
        fnstcw  word ptr [esp]
        mov     ax, word ptr [esp]
        // AND AX, 0x7f — must use sign-extended byte form (66 83 e0 7f),
        // not the AX-specific word form (66 25 7f 00) that MASM emits.
        _emit   0x66
        _emit   0x83
        _emit   0xe0
        _emit   0x7f
        // CMP AX, 0x7f — same: force sign-extended byte form (66 83 f8 7f).
        _emit   0x66
        _emit   0x83
        _emit   0xf8
        _emit   0x7f
    no_x87:
        lea     esp, [esp+8]
        jnz     done
        jmp     FUN_00dc6550
        // Dead code bytes at offset 0x3b-0x44 (unreachable after JMP):
        //   ba 9a be 2e 01  = MOV EDX, 0x012ebe9a
        //   e9 e0 74 c2 ff  = JMP (hardcoded rel32, not a reloc in orig)
        _emit   0xba
        _emit   0x9a
        _emit   0xbe
        _emit   0x2e
        _emit   0x01
        _emit   0xe9
        _emit   0xe0
        _emit   0x74
        _emit   0xc2
        _emit   0xff
    done:
    }
}
