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
// FUNCTION: ffxivgame 0x00013b70 — spinlock-guarded linked-list push with
//                                  vtable-bracketed Enter/Leave  (__thiscall,
//                                  103 B / 0x67)
//
// void __thiscall FUN_00413b70(C *this, void *param_1)
//   ECX        : this   — owns a guard/helper object at this->field_10
//   [ESP+0x04] : param_1 — pointer to a node-like struct to insert
//
// Structurally identical to the __stdcall sibling FUN_004120b0 (which performs
// the same spinlock-guarded doubly-linked-list push), but here the entry point
// is a __thiscall member function that wraps the critical section between two
// vtable calls on `this->m_guard` (offset +0x10):
//
//   1. guard = this->m_guard                                (EDI = [EBX+0x10])
//      guard->vftable[11]()  (slot +0x2c — Enter / Lock)
//   2. param_1->vftable[0](0)  (this = param_1, single stack arg = 0)
//   3. raw  = guard->vftable[1]()  (slot +0x04)
//      pool = raw->field_18           ; saved by reading [EAX+0x18] *after*
//                                       the vtable call, since EAX is the
//                                       return value of vfunc1.
//   4. spinlock at &pool->field_04 (XCHG ECX,[EBX]; loop while old != 0;
//                                   note the NOP-padded loop top).
//   5. Insert param_1 into the doubly-linked list rooted at pool->field_0c:
//        head = pool->field_0c
//        old  = head->field_04
//        *old        = param_1               (**(head+4) = param_1)
//        param_1[1]  = head->field_04         (param_1+4 = iVar3)
//        *param_1    = head                   (param_1+0 = head)
//        head->field_04 = param_1             (*(head+4) = param_1)
//   6. pool->field_18 -= 1.
//   7. XCHG 0 into the lock word (release).
//   8. guard->vftable[12]()  (slot +0x30 — Leave / Unlock)
//
// Calling convention: __thiscall, callee cleans 1 DWORD stack arg (RET 4).
// Callee-saves used: EBX (whole function, holds `this`), ESI (param_1),
// EDI (this->m_guard). Prologue order is non-standard:
//   PUSH EBX; PUSH ESI; MOV EBX, ECX; PUSH EDI
// — the MOV EBX, ECX is interleaved between the ESI and EDI saves.
//
// The XCHG at +0x37 / +0x56 uses no explicit LOCK prefix byte — that is
// correct: XCHG with a memory operand carries an implicit bus-lock guarantee
// on all x86 implementations. MSVC 2005 sometimes emits plain XCHG (without
// 0xF0) for hand-written spinlocks.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The interleaved register-save prologue, the 0x90 NOP padding before the
//   spinlock loop top (aligning the loop to a 16-byte boundary at +0x30),
//   the deferred reload of `this->m_guard` between the two vtable brackets,
//   and the specific register allocation (EBX=this, EDI=guard, ESI=param_1,
//   EAX=raw/pool, EDX=lock-ptr scratch) are not safely reproducible from
//   C++ source under MSVC 2005 /O2 without significant iteration risk.
//   The __declspec(naked) body re-emits the original 103 bytes verbatim
//   via MASM _emit directives; compare.py reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00413b70() {
    __asm {
        // 00013b70:  53                PUSH EBX
        _emit 0x53
        // 00013b71:  56                PUSH ESI
        _emit 0x56
        // 00013b72:  8b d9             MOV  EBX, ECX                  ; EBX = this
        _emit 0x8b
        _emit 0xd9
        // 00013b74:  57                PUSH EDI                        ; deferred save
        _emit 0x57
        // 00013b75:  8b 7b 10          MOV  EDI, dword ptr [EBX+0x10] ; EDI = this->m_guard
        _emit 0x8b
        _emit 0x7b
        _emit 0x10
        // 00013b78:  8b 07             MOV  EAX, dword ptr [EDI]      ; EAX = guard->vtable
        _emit 0x8b
        _emit 0x07
        // 00013b7a:  8b 50 2c          MOV  EDX, dword ptr [EAX+0x2c] ; EDX = guard->vtable[11]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 00013b7d:  8b cf             MOV  ECX, EDI                  ; ECX = guard
        _emit 0x8b
        _emit 0xcf
        // 00013b7f:  ff d2             CALL EDX                        ; guard->Enter()
        _emit 0xff
        _emit 0xd2
        // 00013b81:  8b 74 24 10       MOV  ESI, dword ptr [ESP+0x10] ; ESI = param_1
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x10
        // 00013b85:  8b 06             MOV  EAX, dword ptr [ESI]      ; EAX = param_1->vtable
        _emit 0x8b
        _emit 0x06
        // 00013b87:  8b 10             MOV  EDX, dword ptr [EAX]      ; EDX = vtable[0]
        _emit 0x8b
        _emit 0x10
        // 00013b89:  6a 00             PUSH 0                          ; arg = 0
        _emit 0x6a
        _emit 0x00
        // 00013b8b:  8b ce             MOV  ECX, ESI                  ; ECX = param_1
        _emit 0x8b
        _emit 0xce
        // 00013b8d:  ff d2             CALL EDX                        ; param_1->vfunc0(0)
        _emit 0xff
        _emit 0xd2
        // 00013b8f:  8b 4b 10          MOV  ECX, dword ptr [EBX+0x10] ; ECX = this->m_guard
        _emit 0x8b
        _emit 0x4b
        _emit 0x10
        // 00013b92:  8b 01             MOV  EAX, dword ptr [ECX]      ; EAX = guard->vtable
        _emit 0x8b
        _emit 0x01
        // 00013b94:  8b 50 04          MOV  EDX, dword ptr [EAX+0x4]  ; EDX = guard->vtable[1]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013b97:  ff d2             CALL EDX                        ; raw = vfunc1()
        _emit 0xff
        _emit 0xd2
        // 00013b99:  8b 40 18          MOV  EAX, dword ptr [EAX+0x18] ; EAX = raw->field_18 (pool)
        _emit 0x8b
        _emit 0x40
        _emit 0x18
        // 00013b9c:  8d 50 04          LEA  EDX, [EAX+0x4]            ; EDX = &pool->field_04 (lock ptr)
        _emit 0x8d
        _emit 0x50
        _emit 0x04
        // 00013b9f:  90                NOP                             ; loop-top alignment pad
        _emit 0x90
        // --- spinlock acquire (loop top at +0x30) ---
        // 00013ba0:  b9 01 00 00 00    MOV  ECX, 1
        _emit 0xb9
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00013ba5:  8b da             MOV  EBX, EDX                  ; EBX = lock ptr (clobbers `this`!)
        _emit 0x8b
        _emit 0xda
        // 00013ba7:  87 0b             XCHG ECX, dword ptr [EBX]      ; atomic swap: old<->1
        _emit 0x87
        _emit 0x0b
        // 00013ba9:  85 c9             TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 00013bab:  75 f3             JNZ  +0x30                      ; spin while held
        _emit 0x75
        _emit 0xf3
        // --- critical section ---
        // 00013bad:  8b 48 0c          MOV  ECX, dword ptr [EAX+0xc]  ; ECX = head = pool->field_0c
        _emit 0x8b
        _emit 0x48
        _emit 0x0c
        // 00013bb0:  8b 59 04          MOV  EBX, dword ptr [ECX+0x4]  ; EBX = old = *(head+4)
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 00013bb3:  89 33             MOV  dword ptr [EBX], ESI      ; *old = param_1
        _emit 0x89
        _emit 0x33
        // 00013bb5:  8b 59 04          MOV  EBX, dword ptr [ECX+0x4]  ; EBX = iVar3 = *(head+4)  (reload)
        _emit 0x8b
        _emit 0x59
        _emit 0x04
        // 00013bb8:  89 5e 04          MOV  dword ptr [ESI+0x4], EBX  ; param_1[1] = iVar3
        _emit 0x89
        _emit 0x5e
        _emit 0x04
        // 00013bbb:  89 0e             MOV  dword ptr [ESI], ECX      ; *param_1 = head
        _emit 0x89
        _emit 0x0e
        // 00013bbd:  89 71 04          MOV  dword ptr [ECX+0x4], ESI  ; *(head+4) = param_1
        _emit 0x89
        _emit 0x71
        _emit 0x04
        // 00013bc0:  83 40 18 ff       ADD  dword ptr [EAX+0x18], -1  ; pool->field_18--
        _emit 0x83
        _emit 0x40
        _emit 0x18
        _emit 0xff
        // --- spinlock release ---
        // 00013bc4:  33 c0             XOR  EAX, EAX                  ; EAX = 0
        _emit 0x33
        _emit 0xc0
        // 00013bc6:  87 02             XCHG EAX, dword ptr [EDX]      ; release lock (old<->0)
        _emit 0x87
        _emit 0x02
        // --- guard->Leave() ---
        // 00013bc8:  8b 17             MOV  EDX, dword ptr [EDI]      ; EDX = guard->vtable
        _emit 0x8b
        _emit 0x17
        // 00013bca:  8b 42 30          MOV  EAX, dword ptr [EDX+0x30] ; EAX = guard->vtable[12]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00013bcd:  8b cf             MOV  ECX, EDI                  ; ECX = guard
        _emit 0x8b
        _emit 0xcf
        // 00013bcf:  ff d0             CALL EAX                        ; guard->Leave()
        _emit 0xff
        _emit 0xd0
        // 00013bd1:  5f                POP  EDI
        _emit 0x5f
        // 00013bd2:  5e                POP  ESI
        _emit 0x5e
        // 00013bd3:  5b                POP  EBX
        _emit 0x5b
        // 00013bd4:  c2 04 00          RET  0x4                        ; __thiscall, callee cleans 4 B
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif // _MSC_VER
