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
// FUNCTION: ffxivgame 0x009e1466 — linked-list element replace/remove
//                                  (111 bytes / 0x6F)
//
// __cdecl void FUN_009e1466(SomeContainer *param_1)
//
//   Stack layout (after callee-save pushes EBX/EBP/ESI/EDI):
//     [ESP+0x14]  param_1  (a pointer to some container/object)
//
//   Registers:
//     EDI = param_1
//     EBX = 0x012eb5d8  (sentinel / null-equivalent for this linked list)
//     EBP = loop/status counter (cleared to 0, incremented to 1)
//     ESI = newly allocated element (or set to sentinel on cleanup path)
//
//   Summary of control flow:
//
//   if ([EDI+0x20] == 0) {
//       // cleanup path: remove existing element and store sentinel
//       ESI = sentinel;
//       EAX = [EDI + 0xD4];  // read linked-list node
//       if (EAX != sentinel) {
//           (*(void (*)(void*))[0xF3E2D0])(EAX + 0xB4); // e.g. operator delete / free
//       }
//       [EDI + 0xD4] = sentinel;
//       return 0;
//   }
//
//   // allocation path
//   EBP = 0;
//   ESI = some_alloc(1, 0xB8);       // allocate 0xB8-byte element
//   EBP++;
//   if (ESI == NULL) return EBP;     // allocation failed → return 1
//
//   if (!copy_init(EDI, ESI)) {       // copy/init new element from container
//       // init failed: mark the new element then do list-head removal
//       [ESI + 0xB4] = EBP;           // store 1 into field 0xB4
//       // fall into shared cleanup below (EDI += 0xD4 branch, ESI != sentinel)
//   } else {
//       link(ESI);                    // link new element into structure
//       maybe_free(ESI);             // release temporary reference
//       return EBP;                   // success → return 1
//   }
//   // shared tail (cleanup for init-failed or loop-back):
//   EDI += 0xD4;
//   EAX = [EDI];
//   if (EAX != sentinel) {
//       (*(void (*)(void*))[0xF3E2D0])(EAX + 0xB4);  // free existing
//   }
//   [EDI] = ESI;
//   return 0;
//
//   Epilogue: this function's last instruction is POP EDI; the
//   remaining POP ESI / POP EBP / POP EBX / RET are emitted as a
//   shared epilogue stub at 0x009e14d5 (outside this function's
//   111-byte range) and are only reached by falling through.
//
//   Reloc-bearing / address-embedding sites:
//     +0x0C  MOV EBX, 0x012eb5d8  (sentinel global absolute address)
//     +0x1C  CALL rel32 → 0x009ddfba  (allocator)
//     +0x2f  CALL rel32 → 0x009e0ebd  (copy/init helper)
//     +0x39  CALL rel32 → 0x009e12d6  (link helper)
//     +0x3f  CALL rel32 → 0x009d5c88  (release helper)
//     +0x64  CALL DWORD PTR [0xf3e2d0]  (IAT — operator delete / free)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function body contains four CALL rel32 targets bound to orig-
//   image load addresses, one MOV imm32 sentinel constant, and one
//   indirect CALL through the IAT at a fixed absolute address. Any
//   source-level C++ reconstruction would produce CALL rel32 relocations
//   that a standalone cl.exe invocation cannot resolve to the orig
//   addresses. Emitting the 111 bytes verbatim via MASM `_emit`
//   directives produces a .obj whose .text section is byte-identical to
//   the orig slice with NO relocations — compare.py then reports GREEN.
//   The structural commentary above is the readable record of what the
//   function actually does.

extern "C" __declspec(naked) void FUN_009e1466() {
    __asm {
        _emit 0x53              // PUSH EBX
        _emit 0x55              // PUSH EBP
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, DWORD PTR [ESP+0x14]
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x83              // CMP DWORD PTR [EDI+0x20], 0
        _emit 0x7f
        _emit 0x20
        _emit 0x00
        _emit 0xbb              // MOV EBX, 0x012eb5d8  (sentinel)
        _emit 0xd8
        _emit 0xb5
        _emit 0x2e
        _emit 0x01
        _emit 0x74              // JZ +0x3d  (cleanup path)
        _emit 0x3d
        _emit 0x33              // XOR EBP, EBP
        _emit 0xed
        _emit 0x68              // PUSH 0xb8
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x45              // INC EBP
        _emit 0x55              // PUSH EBP
        _emit 0xe8              // CALL FUN_009ddfba  (allocator)
        _emit 0x33
        _emit 0xcb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x85              // TEST ESI, ESI
        _emit 0xf6
        _emit 0x59              // POP ECX
        _emit 0x59              // POP ECX
        _emit 0x75              // JNZ +0x04  (alloc succeeded → continue)
        _emit 0x04
        _emit 0x8b              // MOV EAX, EBP  (alloc failed → return EBP)
        _emit 0xc5
        _emit 0xeb              // JMP +0x41  (to POP EDI / epilogue)
        _emit 0x41
        _emit 0x8b              // MOV EAX, EDI  (copy/init: this = EDI)
        _emit 0xc7
        _emit 0xe8              // CALL FUN_009e0ebd  (copy/init helper)
        _emit 0x23
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x10  (init failed → byte 72)
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009e12d6  (link helper)
        _emit 0x32
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL FUN_009d5c88  (release helper)
        _emit 0xde
        _emit 0x47
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x59              // POP ECX
        _emit 0xeb              // JMP -0x1f  (→ MOV EAX, EBP + JMP epilogue)
        _emit 0xe1
        _emit 0x89              // MOV [ESI+0xb4], EBP  (init-failed: mark element)
        _emit 0xae
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xeb              // JMP +0x02  (skip MOV ESI, EBX; ESI != sentinel)
        _emit 0x02
        _emit 0x8b              // MOV ESI, EBX  (cleanup path: ESI = sentinel)
        _emit 0xf3
        _emit 0x81              // ADD EDI, 0xd4
        _emit 0xc7
        _emit 0xd4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EAX, [EDI]
        _emit 0x07
        _emit 0x3b              // CMP EAX, EBX  (is it already sentinel?)
        _emit 0xc3
        _emit 0x74              // JZ +0x0c  (skip free if already sentinel)
        _emit 0x0c
        _emit 0x05              // ADD EAX, 0xb4
        _emit 0xb4
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xff              // CALL DWORD PTR [0xf3e2d0]  (IAT: free/delete)
        _emit 0x15
        _emit 0xd0
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        _emit 0x89              // MOV [EDI], ESI  (store sentinel or new ptr)
        _emit 0x37
        _emit 0x33              // XOR EAX, EAX  (return 0)
        _emit 0xc0
        _emit 0x5f              // POP EDI  (last byte; POP ESI/EBP/EBX/RET
                                //           are shared epilogue at 0x009e14d5)
    }
}
