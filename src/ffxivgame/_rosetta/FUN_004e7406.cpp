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
// FUNCTION: ffxivgame 0x000e7406 — SEH __except handler body
//                                  (71 B / 0x47)
//
// This is the body of a __try/__except handler executed after the filter
// at Catch@004e73d2 evaluates and returns. At this point:
//   ESI = [ebp+8] (the first argument / "this" of the enclosing function,
//                  loaded by the 3 bytes immediately before this fragment)
//   EBP = enclosing function's frame pointer
//
// Flow:
//   edx = [ebp-0x14]                 ; local: a flags value (may be 0)
//   ecx = [esi]                      ; dereference vtable pointer in esi
//   ecx = [ecx+4]                    ; vtable slot 1 (adjustment offset)
//   ecx = ecx + esi                  ; adjust this pointer
//   if (edx != 0) {
//       eax = [ecx+8]                ; load field at +8
//       eax |= edx                   ; apply flags
//       if ([ecx+0x28] == 0)
//           eax |= 4;                ; extra flag when sub-field is zero
//       FUN_004e5130(eax, 0);        ; call with computed flags + 0
//   }
//   [ebp-4] = 0xffffffff;            ; reset SEH guard cookie to "no handler"
//   ecx = &[ebp-0x24];              ; load address of a local RAII object
//   FUN_004e6620(ecx);               ; call destructor / cleanup
//   eax = esi;                       ; return the original "this" / arg
//   // epilogue — restore SEH frame and callee-saved regs
//   ecx = [ebp-0xc];
//   fs:[0] = ecx;                    ; unchain SEH frame
//   pop ecx, edi, esi, ebx;
//   mov esp, ebp; pop ebp; ret
//
// Calling convention: N/A — this is a naked fragment; it is reached via
// the SEH machinery, not by a direct CALL.  The function ends with a
// standard __cdecl epilogue (ret, no stack-arg cleanup).
//
// Reloc-bearing positions:
//   off 0x1f  CALL rel32 → FUN_004e5130  (displacement 0xffffdd06, hardcoded)
//   off 0x2f  CALL rel32 → FUN_004e6620  (displacement 0xfffff1e7, hardcoded)
//
// Reconstruction strategy: __declspec(naked) + _emit passthrough.
// The displacements are hardcoded (matching the orig binary's relative
// offsets) rather than using symbolic CALL forms, because this fragment
// is reached through the SEH dispatch rather than as a first-class
// linker-visible function. Both call targets are within the same module
// and their relative distances are fixed in the orig PE.

extern "C" __declspec(naked) void FUN_004e7406() {
    __asm {
        // 000e7406: mov edx, dword ptr [ebp - 0x14]
        _emit 0x8b
        _emit 0x55
        _emit 0xec
        // 000e7409: mov ecx, dword ptr [esi]
        _emit 0x8b
        _emit 0x0e
        // 000e740b: mov ecx, dword ptr [ecx + 0x4]
        _emit 0x8b
        _emit 0x49
        _emit 0x04
        // 000e740e: add ecx, esi
        _emit 0x03
        _emit 0xce
        // 000e7410: test edx, edx
        _emit 0x85
        _emit 0xd2
        // 000e7412: je +0x16 -> 000e742a
        _emit 0x74
        _emit 0x16
        // 000e7414: mov eax, dword ptr [ecx + 0x8]
        _emit 0x8b
        _emit 0x41
        _emit 0x08
        // 000e7417: or eax, edx
        _emit 0x0b
        _emit 0xc2
        // 000e7419: cmp dword ptr [ecx + 0x28], 0
        _emit 0x83
        _emit 0x79
        _emit 0x28
        _emit 0x00
        // 000e741d: jne +0x3 -> 000e7422
        _emit 0x75
        _emit 0x03
        // 000e741f: or eax, 4
        _emit 0x83
        _emit 0xc8
        _emit 0x04
        // 000e7422: push 0
        _emit 0x6a
        _emit 0x00
        // 000e7424: push eax
        _emit 0x50
        // 000e7425: call FUN_004e5130  (rel32 displacement: 0xffffdd06)
        _emit 0xe8
        _emit 0x06
        _emit 0xdd
        _emit 0xff
        _emit 0xff
        // 000e742a: mov dword ptr [ebp - 0x4], 0xffffffff
        _emit 0xc7
        _emit 0x45
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 000e7431: lea ecx, [ebp - 0x24]
        _emit 0x8d
        _emit 0x4d
        _emit 0xdc
        // 000e7434: call FUN_004e6620  (rel32 displacement: 0xfffff1e7)
        _emit 0xe8
        _emit 0xe7
        _emit 0xf1
        _emit 0xff
        _emit 0xff
        // 000e7439: mov eax, esi
        _emit 0x8b
        _emit 0xc6
        // 000e743b: mov ecx, dword ptr [ebp - 0xc]
        _emit 0x8b
        _emit 0x4d
        _emit 0xf4
        // 000e743e: mov dword ptr fs:[0], ecx
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000e7445: pop ecx
        _emit 0x59
        // 000e7446: pop edi
        _emit 0x5f
        // 000e7447: pop esi
        _emit 0x5e
        // 000e7448: pop ebx
        _emit 0x5b
        // 000e7449: mov esp, ebp
        _emit 0x8b
        _emit 0xe5
        // 000e744b: pop ebp
        _emit 0x5d
        // 000e744c: ret
        _emit 0xc3
    }
}
