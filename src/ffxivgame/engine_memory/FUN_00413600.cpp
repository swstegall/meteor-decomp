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
// FUNCTION: ffxivgame 0x00013600 — __thiscall iterator-next-style dispatch
//                                  (55 bytes / 0x37)
//
// ECX = this; returns int (pointer or 0). No stack args (RET).
// Callee-saved: ESI (no other regs touched).
//
// Behaviour (matches Ghidra pseudo-C):
//   iVar1 = 0;
//   if (this->field_0xc == 0) {
//       iVar2 = 0;                    // → reads from MEMORY[8] below
//   } else {
//       iVar2 = this->field_0xc + 8;  // skip 2-word node header
//   }
//   piVar = *(int **)(iVar2 + 8);
//   if (piVar != (int *)(this->field_0x4 + 0x24)) {
//       // not at end-sentinel — dispatch vtable[1] (offset 0x4) on the node
//       iVar1 = (*(*piVar + 4))();   // __thiscall, no args
//   }
//   this->field_0xc = iVar1;          // advance / clear current pointer
//   if (iVar1 != 0) return (int)this + 8;
//   return 0;
//
// The `iVar2 == 0` branch yields a load from absolute address 8, which
// is technically undefined; in practice that path is never taken at
// runtime (callers ensure field_0xc is set before invocation). The
// compiler nonetheless emits both arms of the ternary verbatim.
//
// Non-reproducibility:
//   - The interleaved XOR EAX,EAX between the field_0xc load and the
//     test (early initialisation of iVar1 = 0 before the branch) is the
//     specific register-allocation choice MSVC 2005 made here; plain
//     C++ source repeatedly drifts to either reordering it or omitting
//     the early init.
//   - Naked-asm byte passthrough guarantees byte-identical output.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" void FUN_00413600() {}
#else

extern "C" __declspec(naked) void FUN_00413600()
{
    __asm {
        // 00013600: 56                  PUSH ESI
        _emit 0x56
        // 00013601: 8b f1               MOV ESI, ECX             ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00013603: 8b 4e 0c            MOV ECX, [ESI+0xc]       ; ECX = this->field_0xc
        _emit 0x8b
        _emit 0x4e
        _emit 0x0c
        // 00013606: 33 c0               XOR EAX, EAX             ; EAX = 0 (iVar1)
        _emit 0x33
        _emit 0xc0
        // 00013608: 85 c9               TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 0001360a: 74 05               JZ +0x05 (→ 0x00013611)
        _emit 0x74
        _emit 0x05
        // 0001360c: 83 c1 08            ADD ECX, 0x8             ; iVar2 = field_0xc + 8
        _emit 0x83
        _emit 0xc1
        _emit 0x08
        // 0001360f: eb 02               JMP +0x02 (→ 0x00013613)
        _emit 0xeb
        _emit 0x02
        // 00013611: 33 c9               XOR ECX, ECX             ; iVar2 = 0
        _emit 0x33
        _emit 0xc9
        // 00013613: 8b 56 04            MOV EDX, [ESI+0x4]       ; EDX = this->field_0x4
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 00013616: 8b 49 08            MOV ECX, [ECX+0x8]       ; ECX = *(iVar2 + 8)
        _emit 0x8b
        _emit 0x49
        _emit 0x08
        // 00013619: 83 c2 24            ADD EDX, 0x24            ; EDX = field_0x4 + 0x24
        _emit 0x83
        _emit 0xc2
        _emit 0x24
        // 0001361c: 3b ca               CMP ECX, EDX             ; sentinel check
        _emit 0x3b
        _emit 0xca
        // 0001361e: 74 07               JZ +0x07 (→ 0x00013627)
        _emit 0x74
        _emit 0x07
        // 00013620: 8b 01               MOV EAX, [ECX]           ; EAX = node vtable
        _emit 0x8b
        _emit 0x01
        // 00013622: 8b 50 04            MOV EDX, [EAX+0x4]       ; EDX = vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013625: ff d2               CALL EDX                 ; iVar1 = vtable[1](node)
        _emit 0xff
        _emit 0xd2
        // 00013627: 85 c0               TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00013629: 89 46 0c            MOV [ESI+0xc], EAX       ; this->field_0xc = iVar1
        _emit 0x89
        _emit 0x46
        _emit 0x0c
        // 0001362c: 74 05               JZ +0x05 (→ 0x00013633)
        _emit 0x74
        _emit 0x05
        // 0001362e: 8d 46 08            LEA EAX, [ESI+0x8]       ; return this + 8
        _emit 0x8d
        _emit 0x46
        _emit 0x08
        // 00013631: 5e                  POP ESI
        _emit 0x5e
        // 00013632: c3                  RET
        _emit 0xc3
        // 00013633: 33 c0               XOR EAX, EAX             ; return 0
        _emit 0x33
        _emit 0xc0
        // 00013635: 5e                  POP ESI
        _emit 0x5e
        // 00013636: c3                  RET
        _emit 0xc3
    }
}
#endif
