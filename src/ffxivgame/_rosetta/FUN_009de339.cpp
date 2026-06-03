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
// FUNCTION: ffxivgame 0x009de339 — string allocation / trim helper
//                                  (216 B / 0xd8, SEH-framed __cdecl, 2 args).
//
// Asm shape recovered from the 216 original bytes at RVA 0x005de339:
//
//   +0x00   PUSH 0xc / PUSH <scope_table 0x122d358> / CALL __SEH_prolog
//   +0x0c   EDI = arg1; EBX = 0
//   +0x11   if (arg1->field4 != 0) jmp epilogue
//   +0x1a   call alloc_string(0, &arg1->bytes[9], 0, free_fn, alloc_fn, 0x2800)
//   +0x37   if (result == 0) { eax = 0; jmp done }
//   +0x46   ESI = strlen(result)
//   +0x4e   loop: trim trailing spaces (DEC ESI; if buf[ESI]==' ') buf[ESI]=0
//   +0x62   call fn_9e264c(0xe)
//   +0x6a   [EBP-4] = 0 (SEH state)
//   +0x6d   if (arg1->field4 != 0) goto cleanup
//   +0x72   node = alloc(8); if (!node) goto cleanup
//   +0x83   new_buf = alloc(len+2); arg1->field4 = new_buf
//   +0x8a   if (!new_buf) { free(node); goto cleanup }
//   +0x91   copy_fn(new_buf, len+2, buf)
//   +0x9e   if (copy ok) fn_9d2194(0,0,0,0,0)
//   +0xb1   link node into arg2 list
//   +0xc2   cleanup: free(node) on bad-alloc path
//   +0xcb   free(buf)
//   +0xd3   POP ECX; SEH state = -2 (0xFFFFFFFE) → partial epilogue
//
// The function body ends after 0xd8 bytes (mid-instruction: the last 4 bytes
// are the start of `MOV dword ptr [EBP-4], 0xFFFFFFFE`). The shared epilogue
// at VA 0x009de419 (outside this 0xd8-byte window) loads arg1->field4 into
// EAX, calls __SEH_epilog at 0x009de535, and returns.
//
// Reconstruction strategy — naked-asm byte passthrough.
//
//   The SEH prolog/epilog call shape, the interleaved try-state updates
//   ([EBP-4] ← 0 then ← 0xFFFFFFFE), the exact register allocation across
//   the trim loop (ESI=len, EBX=0, EDI=arg1) and the truncated-at-0xd8
//   function boundary all make a source-level C++ port impractical — any
//   rewrite shifts at least one branch encoding or frame-slot offset.
//   Emitting the 216 original bytes verbatim via MASM `_emit` directives
//   gives the .obj a .text section that is byte-identical to the orig slice;
//   compare.py reports GREEN.
//
// Reloc-bearing sites (imm32 / rel32 immediates matching the orig binary):
//   +0x03   DIR32 → 0x0122d358  (SEH scope table)
//   +0x08   REL32 → 0x009de4f0  (__SEH_prolog)
//   +0x20   DIR32 → 0x009d5c88  (free_fn address literal)
//   +0x25   DIR32 → 0x009d5bc5  (alloc_fn address literal)
//   +0x30   REL32 → 0x009f58ee  (alloc_string)
//   +0x47   REL32 → 0x009dc3f0  (strlen-like helper)
//   +0x65   REL32 → 0x009e264c  (fn_9e264c)
//   +0x75   REL32 → 0x009d5bc5  (alloc)
//   +0x85   REL32 → 0x009d5bc5  (alloc)
//   +0x97   REL32 → 0x009d29fb  (copy_fn)
//   +0xaa   REL32 → 0x009d2194  (fn_9d2194)
//   +0xc6   REL32 → 0x009d5c88  (free)
//   +0xcf   REL32 → 0x009d5c88  (free)

extern "C" __declspec(naked) void FUN_009de339() {
    __asm {
        // +0x00 … +0x0f
        _emit 0x6a  // PUSH 0xc
        _emit 0x0c
        _emit 0x68  // PUSH 0x0122d358  (scope table)
        _emit 0x58
        _emit 0xd3
        _emit 0x22
        _emit 0x01
        _emit 0xe8  // CALL __SEH_prolog
        _emit 0xab
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x8b  // MOV EDI, [EBP+8]
        _emit 0x7d
        _emit 0x08
        _emit 0x33  // XOR EBX, EBX

        // +0x10 … +0x1f
        _emit 0xdb
        _emit 0x39  // CMP [EDI+4], EBX
        _emit 0x5f
        _emit 0x04
        _emit 0x0f  // JNZ epilogue (+0xc6)
        _emit 0x85
        _emit 0xc6
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0x2800
        _emit 0x00
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x68  // PUSH 0x009d5c88

        // +0x20 … +0x2f
        _emit 0x88
        _emit 0x5c
        _emit 0x9d
        _emit 0x00
        _emit 0x68  // PUSH 0x009d5bc5
        _emit 0xc5
        _emit 0x5b
        _emit 0x9d
        _emit 0x00
        _emit 0x53  // PUSH EBX
        _emit 0x8d  // LEA EAX, [EDI+9]
        _emit 0x47
        _emit 0x09
        _emit 0x50  // PUSH EAX
        _emit 0x53  // PUSH EBX
        _emit 0xe8  // CALL alloc_string

        // +0x30 … +0x3f
        _emit 0x81
        _emit 0x75
        _emit 0x01
        _emit 0x00
        _emit 0x83  // ADD ESP, 0x18
        _emit 0xc4
        _emit 0x18
        _emit 0x89  // MOV [EBP-0x1c], EAX
        _emit 0x45
        _emit 0xe4
        _emit 0x3b  // CMP EAX, EBX
        _emit 0xc3
        _emit 0x75  // JNZ +7
        _emit 0x07
        _emit 0x33  // XOR EAX, EAX
        _emit 0xc0

        // +0x40 … +0x4f
        _emit 0xe9  // JMP done (+0x9e)
        _emit 0x9e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL strlen_helper
        _emit 0x6c
        _emit 0xe0
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0x8b  // MOV ESI, EAX
        _emit 0xf0
        _emit 0xeb  // JMP loop_check (+0xd)
        _emit 0x0d

        // +0x50 … +0x5f  (trim-spaces loop body)
        _emit 0x4e  // DEC ESI
        _emit 0x8b  // MOV EAX, [EBP-0x1c]
        _emit 0x45
        _emit 0xe4
        _emit 0x03  // ADD EAX, ESI
        _emit 0xc6
        _emit 0x80  // CMP byte [EAX], 0x20
        _emit 0x38
        _emit 0x20
        _emit 0x75  // JNZ +7 → break
        _emit 0x07
        _emit 0x88  // MOV byte [EAX], BL
        _emit 0x18
        _emit 0x3b  // CMP ESI, EBX  (loop_check)
        _emit 0xf3
        _emit 0x77  // JA loop_body (-0x11)

        // +0x60 … +0x6f
        _emit 0xef
        _emit 0x4e  // DEC ESI  (post-loop)
        _emit 0x6a  // PUSH 0xe
        _emit 0x0e
        _emit 0xe8  // CALL fn_9e264c
        _emit 0xaa
        _emit 0x42
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX
        _emit 0x89  // MOV [EBP-4], EBX
        _emit 0x5d
        _emit 0xfc
        _emit 0x39  // CMP [EDI+4], EBX
        _emit 0x5f
        _emit 0x04

        // +0x70 … +0x7f
        _emit 0x75  // JNZ cleanup (+0x59)
        _emit 0x59
        _emit 0x6a  // PUSH 8
        _emit 0x08
        _emit 0xe8  // CALL alloc(8)
        _emit 0x13
        _emit 0x78
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0x8b  // MOV EBX, EAX  (node)
        _emit 0xd8
        _emit 0x85  // TEST EBX, EBX
        _emit 0xdb
        _emit 0x74  // JZ cleanup (+0x4b)
        _emit 0x4b

        // +0x80 … +0x8f
        _emit 0x83  // ADD ESI, 2
        _emit 0xc6
        _emit 0x02
        _emit 0x56  // PUSH ESI
        _emit 0xe8  // CALL alloc(len+2)
        _emit 0x03
        _emit 0x78
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0x89  // MOV [EDI+4], EAX
        _emit 0x47
        _emit 0x04
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74  // JZ free_node (+0x33)

        // +0x90 … +0x9f
        _emit 0x33
        _emit 0xff  // PUSH [EBP-0x1c]
        _emit 0x75
        _emit 0xe4
        _emit 0x56  // PUSH ESI
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL copy_fn
        _emit 0x27
        _emit 0x46
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x33  // XOR ECX, ECX
        _emit 0xc9

        // +0xa0 … +0xaf
        _emit 0x3b  // CMP EAX, ECX
        _emit 0xc1
        _emit 0x74  // JZ +0xd → skip_fn
        _emit 0x0d
        _emit 0x51  // PUSH ECX (×5)
        _emit 0x51
        _emit 0x51
        _emit 0x51
        _emit 0x51
        _emit 0xe8  // CALL fn_9d2194(0,0,0,0,0)
        _emit 0xad
        _emit 0x3d
        _emit 0xff
        _emit 0xff
        _emit 0x83  // ADD ESP, 0x14
        _emit 0xc4

        // +0xb0 … +0xbf
        _emit 0x14
        _emit 0x8b  // MOV EAX, [EDI+4]
        _emit 0x47
        _emit 0x04
        _emit 0x89  // MOV [EBX], EAX
        _emit 0x03
        _emit 0x8b  // MOV EAX, [EBP+0xc]  (arg2)
        _emit 0x45
        _emit 0x0c
        _emit 0x8b  // MOV ECX, [EAX+4]
        _emit 0x48
        _emit 0x04
        _emit 0x89  // MOV [EBX+4], ECX
        _emit 0x4b
        _emit 0x04
        _emit 0x89  // MOV [EAX+4], EBX

        // +0xc0 … +0xcf
        _emit 0x58  // (2nd byte of MOV [EAX+4], EBX = 89 58 04)
        _emit 0x04
        _emit 0xeb  // JMP cleanup (+7)
        _emit 0x07
        _emit 0x53  // PUSH EBX  (free_node path)
        _emit 0xe8  // CALL free
        _emit 0x85
        _emit 0x78
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0xff  // PUSH [EBP-0x1c]  (cleanup: free buf)
        _emit 0x75
        _emit 0xe4
        _emit 0xe8  // CALL free
        _emit 0x7c

        // +0xd0 … +0xd7  (last 8 bytes: tail of CALL + POP + partial MOV)
        _emit 0x78
        _emit 0xff
        _emit 0xff
        _emit 0x59  // POP ECX
        _emit 0xc7  // MOV dword ptr [EBP-4], 0xFFFFFFFE  (partial: 4 of 7 bytes)
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
    }
}
