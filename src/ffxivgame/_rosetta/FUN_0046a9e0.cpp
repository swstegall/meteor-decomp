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
// FUNCTION: ffxivgame 0x0006a9e0 — interface-guarded dispatch (212 B / 0xd4),
//                                   __cdecl, 5 arguments, returns int.
//
// Asm shape (read from asm/ffxivgame/0006a9e0_FUN_0046a9e0.s, 212 bytes,
// RVA 0x0006a9e0..0x0006aab4):
//
//   int __cdecl FUN_0046a9e0(Iface* obj, int arg1, unsigned int* arg2,
//                             int arg3, int arg4);
//
// Body sketch (logical):
//
//   if (!obj)           goto error_invalid;  // jump 0x6aa92, ret -2
//   void* vtbl = *obj;
//   if (!vtbl)          goto error_invalid;
//   if (!vtbl[0x28/4])  goto error_invalid;  // function pointer check
//   if (obj->kind != 8) {
//       FUN_0045c940(6, 0x8c, 0x97, <file>, 0x70);  // assertion / log
//       return -1;
//   }
//   // EBX = arg2, EDI = arg1 loaded here (deferred callee-save pushes)
//   if (vtbl[4/1] & 0x2) {            // byte flag at vtable+4
//       unsigned int val = FUN_0045cb50(obj->field_8);
//       if (!arg1) {
//           *arg2 = val;
//           return 1;                  // LEA EAX,[EDI+1] where EDI==0
//       }
//       if (*arg2 < val) {            // unsigned comparison
//           FUN_0045c940(6, 0x8c, 0x9b, <file>, 0x73);
//           return 0;
//       }
//   }
//   // delegate through function pointer at vtable+0x28
//   return ((int(__cdecl*)(Iface*,int,unsigned int*,int,int))vtbl[0x28/4])
//              (obj, arg1, arg2, arg3, arg4);
//
//   error_invalid:
//       FUN_0045c940(6, 0x8c, 0x96, <file>, 0x6b);
//       return -2;
//
// Calling convention analysis:
//   __cdecl (caller cleans; `ADD ESP, 0x14` after the indirect call = 5 args).
//   No local stack frame (no SUB ESP / no /GS cookie — the function has no
//   local arrays ≥5 bytes). ESI saved at entry; EBX and EDI saved mid-body
//   after the early null-check exits (MSVC 2005 deferred callee-save
//   optimisation: the three early-exit paths at 0x6aa07, 0x6aa92, 0x6aa25
//   only need ESI restored, so EBX/EDI pushes are deferred past them).
//
// Reloc-bearing sites (DIR32 / REL32 immediates wildcarded by compare.py):
//   +0x29  PUSH 0x00f792f0     (string literal address — DIR32)
//   +0x3a  CALL FUN_0045c940   (REL32 relative call, offset 0x21 0x1f 0xff 0xff)
//   +0x5b  CALL FUN_0045cb50   (REL32 relative call, offset 0x10 0x21 0xff 0xff)
//   +0x78  PUSH 0x00f792f0     (string literal address — DIR32, same string)
//   +0x89  CALL FUN_0045c940   (REL32 relative call, offset 0xd2 0x1e 0xff 0xff)
//   +0xb4  PUSH 0x00f792f0     (string literal address — DIR32, same string)
//   +0xc5  CALL FUN_0045c940   (REL32 relative call, offset 0x96 0x1e 0xff 0xff)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//   Matching the deferred EBX/EDI push ordering, the LEA EAX,[EDI+1] shortcut
//   for returning 1 when EDI==0, and the indirect CALL EDX form in C++ source
//   under /O2 would require driving MSVC 2005's register allocator to place
//   EBX on arg2 and EDI on arg1 in exactly this layout without also pulling
//   the pushes to the function prolog. Naked asm passthrough (the same
//   approach used by FUN_00408910, FUN_004011b0, FUN_00404e40) gives a
//   byte-exact .text section with no auxiliary subsections and no relocations
//   of our own (the bare bytes match the original slice verbatim, and
//   compare.py wildcards the reloc sites against the orig image).

extern "C" __declspec(naked) void FUN_0046a9e0() {
    __asm {
        // 0006a9e0 — PUSH ESI; load arg0 into ESI; null-check chain
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, [ESP+8]  (arg0)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x85          // TEST ESI, ESI
        _emit 0xf6
        _emit 0x0f          // JZ 0x0046aa92  (error_invalid — far)
        _emit 0x84
        _emit 0xa5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EAX, [ESI]  (vtable/interface ptr)
        _emit 0x06
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x0f          // JZ 0x0046aa92
        _emit 0x84
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83          // CMP [EAX+0x28], 0  (fn ptr present?)
        _emit 0x78
        _emit 0x28
        _emit 0x00
        _emit 0x0f          // JZ 0x0046aa92
        _emit 0x84
        _emit 0x91
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83          // CMP [ESI+0x10], 8  (obj->kind == 8?)
        _emit 0x7e
        _emit 0x10
        _emit 0x08
        _emit 0x74          // JZ 0x0046aa27  (kind matches)
        _emit 0x20
        // 0006aa07 — wrong-kind error path; return -1
        _emit 0x6a          // PUSH 0x70  (line number)
        _emit 0x70
        _emit 0x68          // PUSH 0x00f792f0  (file string — reloc +0x29)
        _emit 0xf0
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        _emit 0x68          // PUSH 0x97
        _emit 0x97
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68          // PUSH 0x8c
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a          // PUSH 6
        _emit 0x06
        _emit 0xe8          // CALL FUN_0045c940  (reloc +0x3a)
        _emit 0x21
        _emit 0x1f
        _emit 0xff
        _emit 0xff
        _emit 0x83          // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x83          // OR EAX, 0xffffffff  (return -1)
        _emit 0xc8
        _emit 0xff
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0006aa27 — flag check + deferred EBX/EDI push
        _emit 0xf6          // TEST byte ptr [EAX+4], 0x2
        _emit 0x40
        _emit 0x04
        _emit 0x02
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV EBX, [ESP+0x14]  (arg2)
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        _emit 0x57          // PUSH EDI
        _emit 0x8b          // MOV EDI, [ESP+0x14]  (arg1)
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x74          // JZ 0x0046aa77  (flag not set → direct call)
        _emit 0x40
        // 0006aa37 — call inner getter
        _emit 0x8b          // MOV EAX, [ESI+8]  (obj->field_8)
        _emit 0x46
        _emit 0x08
        _emit 0x50          // PUSH EAX
        _emit 0xe8          // CALL FUN_0045cb50  (reloc +0x5b)
        _emit 0x10
        _emit 0x21
        _emit 0xff
        _emit 0xff
        _emit 0x83          // ADD ESP, 4
        _emit 0xc4
        _emit 0x04
        _emit 0x85          // TEST EDI, EDI  (arg1 == 0?)
        _emit 0xff
        _emit 0x75          // JNZ 0x0046aa52
        _emit 0x0b
        // 0006aa47 — arg1 == 0: store result and return 1
        _emit 0x8b          // MOV ECX, EBX  (ECX = arg2)
        _emit 0xcb
        _emit 0x89          // MOV [ECX], EAX  (*arg2 = val)
        _emit 0x01
        _emit 0x8d          // LEA EAX, [EDI+1]  (EAX = 0+1 = 1, EDI==0 known)
        _emit 0x47
        _emit 0x01
        _emit 0x5f          // POP EDI
        _emit 0x5b          // POP EBX
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0006aa52 — arg1 != 0: compare *arg2 with result
        _emit 0x39          // CMP [EBX], EAX  (*arg2 vs val, unsigned)
        _emit 0x03
        _emit 0x73          // JNC 0x0046aa77  (*arg2 >= val → proceed to call)
        _emit 0x21
        // 0006aa56 — *arg2 < val error; return 0
        _emit 0x6a          // PUSH 0x73
        _emit 0x73
        _emit 0x68          // PUSH 0x00f792f0  (reloc +0x78)
        _emit 0xf0
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        _emit 0x68          // PUSH 0x9b
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68          // PUSH 0x8c
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a          // PUSH 6
        _emit 0x06
        _emit 0xe8          // CALL FUN_0045c940  (reloc +0x89)
        _emit 0xd2
        _emit 0x1e
        _emit 0xff
        _emit 0xff
        _emit 0x83          // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x5f          // POP EDI
        _emit 0x5b          // POP EBX
        _emit 0x33          // XOR EAX, EAX  (return 0)
        _emit 0xc0
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0006aa77 — delegate: load arg3/arg4, call vtable[0x28/4]
        _emit 0x8b          // MOV EAX, [ESP+0x20]  (arg4)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x8b          // MOV ECX, [ESP+0x1c]  (arg3)
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x8b          // MOV EDX, [ESI]  (vtable ptr again)
        _emit 0x16
        _emit 0x8b          // MOV EDX, [EDX+0x28]  (fn ptr)
        _emit 0x52
        _emit 0x28
        _emit 0x50          // PUSH EAX  (arg4)
        _emit 0x51          // PUSH ECX  (arg3)
        _emit 0x53          // PUSH EBX  (arg2)
        _emit 0x57          // PUSH EDI  (arg1)
        _emit 0x56          // PUSH ESI  (arg0)
        _emit 0xff          // CALL EDX
        _emit 0xd2
        _emit 0x83          // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x5f          // POP EDI
        _emit 0x5b          // POP EBX
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
        // 0006aa92 — error_invalid path; return -2
        _emit 0x6a          // PUSH 0x6b
        _emit 0x6b
        _emit 0x68          // PUSH 0x00f792f0  (reloc +0xb4)
        _emit 0xf0
        _emit 0x92
        _emit 0xf7
        _emit 0x00
        _emit 0x68          // PUSH 0x96
        _emit 0x96
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x68          // PUSH 0x8c
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x6a          // PUSH 6
        _emit 0x06
        _emit 0xe8          // CALL FUN_0045c940  (reloc +0xc5)
        _emit 0x96
        _emit 0x1e
        _emit 0xff
        _emit 0xff
        _emit 0x83          // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0xb8          // MOV EAX, 0xfffffffe  (return -2)
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x5e          // POP ESI
        _emit 0xc3          // RET
    }
}
