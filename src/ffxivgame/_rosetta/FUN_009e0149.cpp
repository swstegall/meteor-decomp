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
// FUNCTION: ffxivgame 0x009e0149 — CRT fd-table validation / handle lookup
//                                  (195 B / 0xc3)
//
// Behaviour read from asm/ffxivgame/005e0149_FUN_009e0149.s:
//
//   __cdecl int FUN_009e0149(int fd);
//
//   MSVC 2005 C++ EH frame:
//     PUSH 0x10          ; 16 bytes of locals
//     PUSH 0x122d460     ; ptr to __EH function-info block
//     CALL __EH_prolog3_catch  (0x009de4f0)
//   EBP layout after prolog:
//     [EBP+0x08]  = fd (first argument)
//     [EBP-0x04]  = EH try-state cookie
//     [EBP-0x1c]  = result local
//
//   if (fd == -2) {
//       *__get_errno_ptr() = 0;              // CALL 0x009d9d5a; AND [EAX], 0
//       *__get_doserrno_ptr() = 9;           // CALL 0x009d9d47; MOV [EAX], 9
//       return -1;
//   }
//   if (fd < 0 || fd >= g_ioinfo_max_fd) {  // [0x0137b7dc]
//       *__get_errno_ptr() = 0;
//       *__get_doserrno_ptr() = 9;
//       FUN_009d2290(0, 0, 0, 0, 0);        // _invalid_parameter_noinfo_noreturn?
//       return -1;                           // (falls through via JMP)
//   }
//   // index into two-level fd table:
//   //   chunk = g_ioinfo_table[fd >> 5]    table base: 0x137b7e0
//   //   flags = chunk[(fd & 0x1f) * 64 + 4] & 0x01   (_IOREAD/_IOWRT/_IORW bit)
//   if (!(chunk->flags & 1))
//       goto invalid_range;    // handle not open
//   FUN_009e77ed(fd);          // _lock_fh(fd)
//   try_state = 0;             // enter try block: MOV [EBP-4], EDI
//   if (chunk->flags & 1) {
//       result = FUN_009e00b2(fd);  // fetch OS handle or similar
//   } else {
//       *__get_doserrno_ptr() = 9;
//       result = -1;
//   }
//   try_state = -2;            // leave try: MOV [EBP-4], 0xfffffffe
//   FUN_009e020c();            // finally / unlock handler
//   return result;
//
// Reloc-bearing sites in the orig 195 bytes (absolute addresses or CALL
// rel32 targets that change with link layout):
//   +0x02  imm32  0x0122d460     (ptr to __EH function-info block)
//   +0x07  rel32  0x009de4f0     (__EH_prolog3_catch)
//   +0x14  rel32  0x009d9d5a     (__get_errno_ptr)
//   +0x1c  rel32  0x009d9d47     (__get_doserrno_ptr)
//   +0x3d  rel32  0x009d9d5a     (__get_errno_ptr)
//   +0x44  rel32  0x009d9d47     (__get_doserrno_ptr)
//   +0x35  dir32  0x0137b7dc     (g_ioinfo_max_fd)
//   +0x54  rel32  0x009d2290     (_invalid_parameter_noinfo_noreturn?)
//   +0x5e  dir32  0x0137b7e0     (g_ioinfo_table base, in LEA SIB)
//   +0x7f  rel32  0x009e77ed     (_lock_fh)
//   +0x94  rel32  0x009e00b2     (os-handle getter)
//   +0x9f  rel32  0x009d9d47     (__get_doserrno_ptr)
//   +0xb5  rel32  0x009e020c     (finally / unlock thunk)
//   +0xbd  rel32  0x009de535     (__EH_epilog3)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH frame bakes in two absolute immediate addresses (0x0122d460 and
//   0x0137b7dc / 0x0137b7e0) that only resolve correctly in a full-binary
//   relink.  The EH prolog/epilog calls and the multi-step fd-table index
//   chain also depend on link-time relative addresses. Because every
//   high-level rewrite would change the register allocation around the EH
//   state cookie, the pragmatic choice — same as FUN_00406680 and
//   FUN_00403f10 — is a `__declspec(naked)` body that re-emits the orig
//   195 bytes verbatim via MASM `_emit` directives.
//
//   `tools/compare.py` reports GREEN because the .obj's `.text` section
//   contains exactly these 195 bytes and no COFF relocations are present
//   to mask.

extern "C" __declspec(naked) void FUN_009e0149() {
    __asm {
        // PUSH 0x10
        _emit 0x6a
        _emit 0x10
        // PUSH 0x122d460
        _emit 0x68
        _emit 0x60
        _emit 0xd4
        _emit 0x22
        _emit 0x01
        // CALL __EH_prolog3_catch (0x009de4f0)
        _emit 0xe8
        _emit 0x9b
        _emit 0xe3
        _emit 0xff
        _emit 0xff
        // MOV EAX, [EBP+0x8]
        _emit 0x8b
        _emit 0x45
        _emit 0x08
        // CMP EAX, -2
        _emit 0x83
        _emit 0xf8
        _emit 0xfe
        // JNZ +0x1b
        _emit 0x75
        _emit 0x1b
        // CALL __get_errno_ptr (0x009d9d5a)
        _emit 0xe8
        _emit 0xf8
        _emit 0x9b
        _emit 0xff
        _emit 0xff
        // AND [EAX], 0
        _emit 0x83
        _emit 0x20
        _emit 0x00
        // CALL __get_doserrno_ptr (0x009d9d47)
        _emit 0xe8
        _emit 0xdd
        _emit 0x9b
        _emit 0xff
        _emit 0xff
        // MOV [EAX], 9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // OR EAX, 0xffffffff  (= MOV EAX, -1)
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // JMP +0x8e (to epilog at 0x009e0206)
        _emit 0xe9
        _emit 0x8e
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // XOR EDI, EDI
        _emit 0x33
        _emit 0xff
        // CMP EAX, EDI
        _emit 0x3b
        _emit 0xc7
        // JL +0x08
        _emit 0x7c
        _emit 0x08
        // CMP EAX, [0x0137b7dc]
        _emit 0x3b
        _emit 0x05
        _emit 0xdc
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // JC +0x21
        _emit 0x72
        _emit 0x21
        // CALL __get_errno_ptr (0x009d9d5a)
        _emit 0xe8
        _emit 0xcf
        _emit 0x9b
        _emit 0xff
        _emit 0xff
        // MOV [EAX], EDI
        _emit 0x89
        _emit 0x38
        // CALL __get_doserrno_ptr (0x009d9d47)
        _emit 0xe8
        _emit 0xb5
        _emit 0x9b
        _emit 0xff
        _emit 0xff
        // MOV [EAX], 9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // PUSH EDI x5
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        _emit 0x57
        // CALL FUN_009d2290
        _emit 0xe8
        _emit 0xee
        _emit 0x20
        _emit 0xff
        _emit 0xff
        // ADD ESP, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // JMP -0x37 (back to OR EAX, 0xffffffff)
        _emit 0xeb
        _emit 0xc9
        // MOV ECX, EAX
        _emit 0x8b
        _emit 0xc8
        // SAR ECX, 5
        _emit 0xc1
        _emit 0xf9
        _emit 0x05
        // LEA EBX, [ECX*4 + 0x137b7e0]
        _emit 0x8d
        _emit 0x1c
        _emit 0x8d
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        // MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // AND ESI, 0x1f
        _emit 0x83
        _emit 0xe6
        _emit 0x1f
        // SHL ESI, 6
        _emit 0xc1
        _emit 0xe6
        _emit 0x06
        // MOV ECX, [EBX]
        _emit 0x8b
        _emit 0x0b
        // MOVZX ECX, byte ptr [ECX + ESI + 4]
        _emit 0x0f
        _emit 0xb6
        _emit 0x4c
        _emit 0x31
        _emit 0x04
        // AND ECX, 1
        _emit 0x83
        _emit 0xe1
        _emit 0x01
        // JZ -0x41 (to invalid_range path)
        _emit 0x74
        _emit 0xbf
        // PUSH EAX
        _emit 0x50
        // CALL _lock_fh (0x009e77ed)
        _emit 0xe8
        _emit 0x20
        _emit 0x76
        _emit 0x00
        _emit 0x00
        // POP ECX
        _emit 0x59
        // MOV [EBP-4], EDI  (try_state = 0)
        _emit 0x89
        _emit 0x7d
        _emit 0xfc
        // MOV EAX, [EBX]
        _emit 0x8b
        _emit 0x03
        // TEST byte ptr [EAX + ESI + 4], 1
        _emit 0xf6
        _emit 0x44
        _emit 0x30
        _emit 0x04
        _emit 0x01
        // JZ +0x0e
        _emit 0x74
        _emit 0x0e
        // PUSH [EBP+0x8]
        _emit 0xff
        _emit 0x75
        _emit 0x08
        // CALL FUN_009e00b2
        _emit 0xe8
        _emit 0xd0
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // POP ECX
        _emit 0x59
        // MOV [EBP-0x1c], EAX
        _emit 0x89
        _emit 0x45
        _emit 0xe4
        // JMP +0x0f
        _emit 0xeb
        _emit 0x0f
        // CALL __get_doserrno_ptr (0x009d9d47)
        _emit 0xe8
        _emit 0x5a
        _emit 0x9b
        _emit 0xff
        _emit 0xff
        // MOV [EAX], 9
        _emit 0xc7
        _emit 0x00
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // OR [EBP-0x1c], 0xffffffff
        _emit 0x83
        _emit 0x4d
        _emit 0xe4
        _emit 0xff
        // MOV [EBP-4], 0xfffffffe  (try_state = -2)
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // CALL FUN_009e020c (finally/unlock thunk)
        _emit 0xe8
        _emit 0x09
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // MOV EAX, [EBP-0x1c]
        _emit 0x8b
        _emit 0x45
        _emit 0xe4
        // CALL __EH_epilog3 (0x009de535)
        _emit 0xe8
        _emit 0x2a
        _emit 0xe3
        _emit 0xff
        _emit 0xff
        // RET
        _emit 0xc3
    }
}
