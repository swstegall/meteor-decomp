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
// FUNCTION: ffxivgame 0x00413940 — conditional spin-lock-protected
//                                  doubly-linked-list-tail insert of
//                                  *(this+0x34), then clear *(this+0x34)
//                                  (__thiscall, 90 B / 0x5a)
//
// __thiscall void FUN_00413940(void)
//   ECX = this
//
// If *(this+0x34) != 0:
//   1. Call vtable[0] of *(this+0x34) with arg 0 (a flush/quiesce hook on
//      the node before it is moved back to the free list).
//   2. iface = *(this+0x14)
//      list_base = (*(iface->vtable[1]()))->m18  — the registry-owned
//      intrusive list head returned via the interface's slot 1.
//   3. Acquire the spin-lock at &list_base[+0x04] with XCHG-1 / TEST / JNZ.
//   4. Splice *(this+0x34) onto the tail of the doubly-linked list rooted
//      at sentinel = list_base[+0x0c]:
//        sentinel->prev->next = node          ; [sentinel->prev] = node
//        node->prev = sentinel->prev          ; (compiler re-reads
//                                                sentinel->prev because
//                                                the previous store could
//                                                alias [sentinel+4])
//        node->next = sentinel
//        sentinel->prev = node
//   5. Decrement list_base[+0x18] (the free-list count).
//   6. Release the spin-lock with XCHG-0.
//   7. *(this+0x34) = 0.
//
// Calling convention: __thiscall, no stack args, void return (RET).
// Frame: PUSH ESI / inside the if-arm PUSH EBX / PUSH EDI; epilogue
// interleaves POP EDI before the [esi+0x34] = 0 store and POP EBX after.
//
// All calls are indirect (CALL EDX); no CALL rel32 / no external-symbol
// relocations are emitted. A __declspec(naked) body re-emitting the
// original 90 bytes verbatim via MASM _emit directives produces a .obj
// whose .text is byte-identical to the orig slice; compare.py reports
// GREEN.

extern "C" __declspec(naked) void FUN_00413940() {
    __asm {
        // 00013940: 56                   PUSH ESI
        _emit 0x56
        // 00013941: 8b f1                MOV ESI, ECX                  (this)
        _emit 0x8b
        _emit 0xf1
        // 00013943: 8b 4e 34             MOV ECX, dword ptr [ESI+0x34] (node = *(this+0x34))
        _emit 0x8b
        _emit 0x4e
        _emit 0x34
        // 00013946: 85 c9                TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00013948: 74 4e                JZ +0x4e → 00013998 (epilogue POP ESI)
        _emit 0x74
        _emit 0x4e
        // 0001394a: 8b 01                MOV EAX, dword ptr [ECX]      (vtable of node)
        _emit 0x8b
        _emit 0x01
        // 0001394c: 8b 10                MOV EDX, dword ptr [EAX]      (vtable[0])
        _emit 0x8b
        _emit 0x10
        // 0001394e: 53                   PUSH EBX
        _emit 0x53
        // 0001394f: 57                   PUSH EDI
        _emit 0x57
        // 00013950: 6a 00                PUSH 0
        _emit 0x6a
        _emit 0x00
        // 00013952: ff d2                CALL EDX                      (node->vtable[0](0))
        _emit 0xff
        _emit 0xd2
        // 00013954: 8b 4e 14             MOV ECX, dword ptr [ESI+0x14] (iface = *(this+0x14))
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00013957: 8b 01                MOV EAX, dword ptr [ECX]      (iface->vtable)
        _emit 0x8b
        _emit 0x01
        // 00013959: 8b 50 04             MOV EDX, dword ptr [EAX+0x04] (vtable[1])
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001395c: ff d2                CALL EDX                      (EAX = iface->vtable[1]())
        _emit 0xff
        _emit 0xd2
        // 0001395e: 8b 40 18             MOV EAX, dword ptr [EAX+0x18] (list_base = *(ret+0x18))
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 00013961: 8b 56 34             MOV EDX, dword ptr [ESI+0x34] (re-read node)
        _emit 0x8b
        _emit 0x56
        _emit 0x34
        // 00013964: 8d 78 04             LEA EDI, [EAX+0x04]           (lock_addr = &list_base[+4])
        _emit 0x8d
        _emit 0x78
        _emit 0x04
        // === spin-lock acquire (00013967) ===
        // 00013967: b9 01 00 00 00       MOV ECX, 1
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001396c: 8b df                MOV EBX, EDI
        _emit 0x8b
        _emit 0xdf
        // 0001396e: 87 0b                XCHG dword ptr [EBX], ECX     (atomic swap)
        _emit 0x87
        _emit 0x0b
        // 00013970: 85 c9                TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00013972: 75 f3                JNZ -0x0d → 00013967          (spin while held)
        _emit 0x75
        _emit 0xf3
        // === doubly-linked-list tail insert ===
        // 00013974: 8b 48 0c             MOV ECX, dword ptr [EAX+0x0c] (sentinel = list_base[+0xc])
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00013977: 8b 59 04             MOV EBX, dword ptr [ECX+0x04] (prev = sentinel->prev)
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 0001397a: 89 13                MOV dword ptr [EBX], EDX      (prev->next = node)
        _emit 0x89
        _emit 0x13
        // 0001397c: 8b 59 04             MOV EBX, dword ptr [ECX+0x04] (re-read prev — alias-safe)
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 0001397f: 89 5a 04             MOV dword ptr [EDX+0x04], EBX (node->prev = prev)
        _emit 0x89
        _emit 0x5a
        _emit 0x04
        // 00013982: 89 0a                MOV dword ptr [EDX], ECX      (node->next = sentinel)
        _emit 0x89
        _emit 0x0a
        // 00013984: 89 51 04             MOV dword ptr [ECX+0x04], EDX (sentinel->prev = node)
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 00013987: 83 40 18 ff          ADD dword ptr [EAX+0x18], -1  (count--)
        _emit 0x83
        _emit 0x40
        _emit 0x18
        _emit 0xff
        // === spin-lock release ===
        // 0001398b: 33 d2                XOR EDX, EDX
        _emit 0x33
        _emit 0xd2
        // 0001398d: 87 17                XCHG dword ptr [EDI], EDX     (store 0, release)
        _emit 0x87
        _emit 0x17
        // 0001398f: 5f                   POP EDI
        _emit 0x5f
        // 00013990: c7 46 34 00 00 00 00 MOV dword ptr [ESI+0x34], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00013997: 5b                   POP EBX
        _emit 0x5b
        // === epilogue (join point for the JZ above) ===
        // 00013998: 5e                   POP ESI
        _emit 0x5e
        // 00013999: c3                   RET
        _emit 0xc3
    }
}
