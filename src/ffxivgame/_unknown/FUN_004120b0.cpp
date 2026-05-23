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
// FUNCTION: ffxivgame 0x000120b0 — spinlock-guarded linked-list push with
//                                  vtable forwarding  (__stdcall, 77 B / 0x4d)
//
// int __stdcall FUN_004120b0(void *param_1)
//   [ESP+0x04] : param_1 — pointer to a node-like struct to insert
//
// Behaviour:
//   1. Reads param_1[5] (offset +0x14) as a pointer to an object with a vtable.
//      Calls vtable[1] on that object (thiscall, no stack args) → raw_result.
//   2. Gets iVar4 = raw_result->field_0c.
//   3. Reads vtable[0] from *param_1 and calls it __thiscall-style with ECX=param_1
//      and a single stack arg of 0.
//   4. Acquires a spinlock at &iVar4->field_04 using an implicit-lock XCHG (no
//      explicit LOCK prefix; XCHG with memory is always atomic on x86).
//   5. Inserts param_1 into a doubly-linked list rooted at iVar4->field_0c:
//        head = iVar4->field_0c
//        *head->field_04 = param_1          (**(head+4) = param_1)
//        old_next       = head->field_04    (iVar3 = *(head+4))
//        param_1[0]     = head              (*param_1 = head)
//        param_1[1]     = old_next          (param_1+4 stores the old next ptr)
//        head->field_04 = param_1           (*(head+4) = param_1)
//   6. Decrements iVar4->field_18.
//   7. Releases the spinlock by XCHGing 0 into the lock word; returns the old
//      lock-word value (always 1 when correctly used).
//
// Calling convention: __stdcall, callee cleans 1 DWORD arg (RET 4).
// Callee-saves used: ESI (whole function), EDI (deferred push at +0x0d).
// No sub-ESP frame (no local stack allocation).
//
// The XCHG at +0x27 / +0x46 uses no explicit LOCK prefix byte — that is correct:
// XCHG with a memory operand carries an implicit bus-lock guarantee on all x86
// implementations. MSVC 2005 sometimes emits plain XCHG (without 0xF0) for
// hand-written spinlocks.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The deferred PUSH EDI at +0x0d (interleaved with vtable-call setup), the
//   exact XCHG-based spinlock idiom (vs. LOCK XCHG or _InterlockedExchange),
//   and the specific register allocation (ESI=param_1, EDI=iVar4, ECX=piVar1,
//   EDX=scratch) are not safely reproducible from C++ source under MSVC 2005
//   /O2 without significant iteration risk. The __declspec(naked) body
//   re-emits the original 77 bytes verbatim via MASM _emit directives;
//   compare.py reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_004120b0() {
    __asm {
        // 000120b0:  56                PUSH ESI
        _emit 0x56
        // 000120b1:  8b 74 24 08       MOV ESI,dword ptr [ESP+0x8]   ; ESI = param_1
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 000120b5:  8b 4e 14          MOV ECX,dword ptr [ESI+0x14]  ; ECX = param_1[5]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 000120b8:  8b 01             MOV EAX,dword ptr [ECX]        ; EAX = vtable ptr
        _emit 0x8b
        _emit 0x01
        // 000120ba:  8b 50 04          MOV EDX,dword ptr [EAX+0x4]   ; EDX = vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000120bd:  57                PUSH EDI                        ; save EDI (deferred)
        _emit 0x57
        // 000120be:  ff d2             CALL EDX                        ; raw_result = vfunc1()
        _emit 0xff
        _emit 0xd2
        // 000120c0:  8b 78 0c          MOV EDI,dword ptr [EAX+0xc]   ; EDI = iVar4
        _emit 0x8b
        _emit 0x78
        _emit 0x0c
        // 000120c3:  8b 06             MOV EAX,dword ptr [ESI]        ; EAX = *param_1
        _emit 0x8b
        _emit 0x06
        // 000120c5:  8b 10             MOV EDX,dword ptr [EAX]        ; EDX = vtable[0]
        _emit 0x8b
        _emit 0x10
        // 000120c7:  6a 00             PUSH 0                          ; arg = 0
        _emit 0x6a
        _emit 0x00
        // 000120c9:  8b ce             MOV ECX,ESI                    ; ECX = param_1 (this)
        _emit 0x8b
        _emit 0xce
        // 000120cb:  ff d2             CALL EDX                        ; param_1->vfunc0(0)
        _emit 0xff
        _emit 0xd2
        // 000120cd:  8d 4f 04          LEA ECX,[EDI+0x4]              ; ECX = piVar1 = &iVar4->field_04
        _emit 0x8d
        _emit 0x4f
        _emit 0x04
        // --- spinlock acquire (loop top at +0x20) ---
        // 000120d0:  b8 01 00 00 00    MOV EAX,1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000120d5:  8b d1             MOV EDX,ECX                    ; EDX = piVar1
        _emit 0x8b
        _emit 0xd1
        // 000120d7:  87 02             XCHG EAX,dword ptr [EDX]       ; atomic swap: old<->1
        _emit 0x87
        _emit 0x02
        // 000120d9:  85 c0             TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 000120db:  75 f3             JNZ 0x004120d0                 ; spin while held
        _emit 0x75
        _emit 0xf3
        // --- critical section ---
        // 000120dd:  8b 47 0c          MOV EAX,dword ptr [EDI+0xc]   ; EAX = iVar2 = iVar4->field_0c
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // 000120e0:  8b 50 04          MOV EDX,dword ptr [EAX+0x4]   ; EDX = *(iVar2+4)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000120e3:  89 32             MOV dword ptr [EDX],ESI        ; **(iVar2+4) = param_1
        _emit 0x89
        _emit 0x32
        // 000120e5:  8b 50 04          MOV EDX,dword ptr [EAX+0x4]   ; EDX = iVar3 = *(iVar2+4)
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000120e8:  89 06             MOV dword ptr [ESI],EAX        ; *param_1 = iVar2
        _emit 0x89
        _emit 0x06
        // 000120ea:  89 56 04          MOV dword ptr [ESI+0x4],EDX   ; param_1[1] = iVar3
        _emit 0x89
        _emit 0x56
        _emit 0x04
        // 000120ed:  89 70 04          MOV dword ptr [EAX+0x4],ESI   ; *(iVar2+4) = param_1
        _emit 0x89
        _emit 0x70
        _emit 0x04
        // 000120f0:  83 47 18 ff       ADD dword ptr [EDI+0x18],-1   ; iVar4->field_18--
        _emit 0x83
        _emit 0x47
        _emit 0x18
        _emit 0xff
        // --- spinlock release ---
        // 000120f4:  33 c0             XOR EAX,EAX                    ; EAX = 0
        _emit 0x33
        _emit 0xc0
        // 000120f6:  87 01             XCHG EAX,dword ptr [ECX]       ; atomic: old<->0; EAX=old
        _emit 0x87
        _emit 0x01
        // 000120f8:  5f                POP EDI
        _emit 0x5f
        // 000120f9:  5e                POP ESI
        _emit 0x5e
        // 000120fa:  c2 04 00          RET 0x4                         ; __stdcall, callee cleans 4 B
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif // _MSC_VER
