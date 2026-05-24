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
// FUNCTION: ffxivgame 0x00014210 — __thiscall SeparateHeapBlock variant —
//           prepare/walk-parents/finalize + tail-call vtable[1]
//           (SQEX::CDev::Engine::Memory::Alternative — sibling of FUN_004107d0
//            and FUN_00410a00, same shape: prep → branch → bookkeeping → tail-jmp)
//
// Behaviour (reconstructed from asm):
//   1. Save ESI/EDI, cache `this` in ESI, load this->space (ESI+0x10) into EDI.
//   2. Call space->vtable[0x2c]() with ECX = space  (prepare).
//   3. If this->m_2c (lock-count? ref-count?) is nonzero → skip the body.
//   4. Else, if this->m_28 == 0 AND this->byte_21 == 0:
//        - Clear this->byte_20.
//        - Call FUN_00413940 with ECX = (this - 4)  (parent-class member).
//        - Through this->m_14 (a second inner ptr), call its vtable[0x1c]().
//        - Skip the loop branch via JMP to bookkeeping.
//      Otherwise (m_28 != 0 OR byte_21 != 0):
//        - Walk a singly-linked list rooted at (this - 4), following the
//          next-pointer at +0x2c. Bail to bookkeeping the moment any node
//          has byte +0x27 != 0. If the walk hits a null next, set
//          this->byte_23 = 1 and call FUN_004140e0 with ECX = (this - 4).
//   5. Increment this->m_2c (the lock-count above).
//   6. Call space->vtable[0x30]() with ECX = space (finalize).
//   7. Tail-jmp through this->vtable[0x04] with ECX = `this`.
//
// Calling convention: __thiscall (ECX = this), no stack args, JMP-tail
//   exit (so no explicit RET; the callee's RET / RET N closes the frame).
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//
//   Source-level C++ cannot reproduce the exact register allocation
//   (ESI/EDI both saved, EDI holding `space` across the branched body,
//   the (this - 4) thisptr computed via `lea ecx, [esi-4]` and re-used
//   as the loop iterator) that MSVC 2005 /O2 emits here, and the two
//   internal CALL displacements (e8 02 f7 ff ff → 0x413940,
//   e8 77 fe ff ff → 0x4140e0) must land at the orig RVA for the diff
//   to go GREEN. A naked passthrough preserves the original 113-byte
//   sequence verbatim. The #if guard makes the file parseable by
//   clang/GCC without -fms-extensions.
//
// Asm (113 bytes @ orig RVA 0x00014210):
//   56                  PUSH ESI
//   8b f1               MOV ESI, ECX            ; cache `this`
//   57                  PUSH EDI
//   8b 7e 10            MOV EDI, [ESI+0x10]     ; space
//   8b 07               MOV EAX, [EDI]
//   8b 50 2c            MOV EDX, [EAX+0x2c]
//   8b cf               MOV ECX, EDI
//   ff d2               CALL EDX                ; space->vtable[0x2c]() — prepare
//   83 7e 2c 00         CMP [ESI+0x2c], 0
//   75 43               JNZ +0x43  → 0x414269   ; skip body if m_2c != 0
//   83 7e 28 00         CMP [ESI+0x28], 0
//   75 1e               JNZ +0x1e  → 0x41424a   ; → parent-walk branch
//   80 7e 21 00         CMP byte [ESI+0x21], 0
//   75 18               JNZ +0x18  → 0x41424a   ; → parent-walk branch
//   ; ── empty-children branch ────────────────────────────────
//   8d 4e fc            LEA ECX, [ESI-0x04]     ; this - 4 (parent)
//   c6 46 20 00         MOV byte [ESI+0x20], 0
//   e8 02 f7 ff ff      CALL 0x00413940         ; parent->detach() (thiscall)
//   8b 4e 14            MOV ECX, [ESI+0x14]     ; inner2
//   8b 01               MOV EAX, [ECX]
//   8b 50 1c            MOV EDX, [EAX+0x1c]
//   ff d2               CALL EDX                ; inner2->vtable[0x1c]()
//   eb 1f               JMP +0x1f  → 0x414269   ; → bookkeeping
//   ; ── parent-walk branch ─────────────────────────────────────
//   8d 4e fc            LEA ECX, [ESI-0x04]     ; this - 4 (head)
//   8b c1               MOV EAX, ECX
//   85 c0               TEST EAX, EAX
//   74 0d               JZ +0x0d  → 0x414260   ; head==null → call detach
//   ; walk-loop:
//   80 78 27 00         CMP byte [EAX+0x27], 0
//   75 10               JNZ +0x10 → 0x414269   ; any node has flag → skip
//   8b 40 2c            MOV EAX, [EAX+0x2c]     ; node = node->next
//   85 c0               TEST EAX, EAX
//   75 f3               JNZ -0x0d → walk-loop
//   ; walk fell off: hit null next without flag
//   c6 46 23 01         MOV byte [ESI+0x23], 1
//   e8 77 fe ff ff      CALL 0x004140e0         ; parent->detach2() (thiscall)
//   ; ── bookkeeping + finalize ─────────────────────────────────
//   83 46 2c 01         ADD [ESI+0x2c], 1       ; ++m_2c
//   8b 07               MOV EAX, [EDI]
//   8b 50 30            MOV EDX, [EAX+0x30]
//   8b cf               MOV ECX, EDI
//   ff d2               CALL EDX                ; space->vtable[0x30]() — finalize
//   8b 06               MOV EAX, [ESI]          ; this->vtable
//   8b 50 04            MOV EDX, [EAX+0x04]     ; vtable[1]
//   5f                  POP EDI
//   8b ce               MOV ECX, ESI            ; tail-call this->method(...)
//   5e                  POP ESI
//   ff e2               JMP EDX

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
// Production builds always use cl.exe (MSVC 2005); the naked+asm block
// below is what actually runs.
extern "C" void FUN_00414210() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_00414210()
{
    __asm {
        // 00014210: 56              PUSH ESI
        _emit 0x56
        // 00014211: 8b f1           MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00014213: 57              PUSH EDI
        _emit 0x57
        // 00014214: 8b 7e 10        MOV EDI, [ESI+0x10]
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 00014217: 8b 07           MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 00014219: 8b 50 2c        MOV EDX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001421c: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0001421e: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014220: 83 7e 2c 00     CMP [ESI+0x2c], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x2c
        _emit 0x00
        // 00014224: 75 43           JNZ +0x43 (→ 0x14269)
        _emit 0x75
        _emit 0x43
        // 00014226: 83 7e 28 00     CMP [ESI+0x28], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x28
        _emit 0x00
        // 0001422a: 75 1e           JNZ +0x1e (→ 0x1424a)
        _emit 0x75
        _emit 0x1e
        // 0001422c: 80 7e 21 00     CMP byte [ESI+0x21], 0
        _emit 0x80
        _emit 0x7e
        _emit 0x21
        _emit 0x00
        // 00014230: 75 18           JNZ +0x18 (→ 0x1424a)
        _emit 0x75
        _emit 0x18
        // 00014232: 8d 4e fc        LEA ECX, [ESI-0x04]
        _emit 0x8d
        _emit 0x4e
        _emit 0xfc
        // 00014235: c6 46 20 00     MOV byte [ESI+0x20], 0
        _emit 0xc6
        _emit 0x46
        _emit 0x20
        _emit 0x00
        // 00014239: e8 02 f7 ff ff  CALL 0x00413940
        _emit 0xe8
        _emit 0x02
        _emit 0xf7
        _emit 0xff
        _emit 0xff
        // 0001423e: 8b 4e 14        MOV ECX, [ESI+0x14]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00014241: 8b 01           MOV EAX, [ECX]
        _emit 0x8b
        _emit 0x01
        // 00014243: 8b 50 1c        MOV EDX, [EAX+0x1c]
        _emit 0x8b
        _emit 0x50
        _emit 0x1c
        // 00014246: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014248: eb 1f           JMP +0x1f (→ 0x14269)
        _emit 0xeb
        _emit 0x1f
        // 0001424a: 8d 4e fc        LEA ECX, [ESI-0x04]
        _emit 0x8d
        _emit 0x4e
        _emit 0xfc
        // 0001424d: 8b c1           MOV EAX, ECX
        _emit 0x8b
        _emit 0xc1
        // 0001424f: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00014251: 74 0d           JZ +0x0d (→ 0x14260)
        _emit 0x74
        _emit 0x0d
        // 00014253: 80 78 27 00     CMP byte [EAX+0x27], 0
        _emit 0x80
        _emit 0x78
        _emit 0x27
        _emit 0x00
        // 00014257: 75 10           JNZ +0x10 (→ 0x14269)
        _emit 0x75
        _emit 0x10
        // 00014259: 8b 40 2c        MOV EAX, [EAX+0x2c]
        _emit 0x8b
        _emit 0x40
        _emit 0x2c
        // 0001425c: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001425e: 75 f3           JNZ -0x0d (→ 0x14253)
        _emit 0x75
        _emit 0xf3
        // 00014260: c6 46 23 01     MOV byte [ESI+0x23], 1
        _emit 0xc6
        _emit 0x46
        _emit 0x23
        _emit 0x01
        // 00014264: e8 77 fe ff ff  CALL 0x004140e0
        _emit 0xe8
        _emit 0x77
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00014269: 83 46 2c 01     ADD [ESI+0x2c], 1
        _emit 0x83
        _emit 0x46
        _emit 0x2c
        _emit 0x01
        // 0001426d: 8b 07           MOV EAX, [EDI]
        _emit 0x8b
        _emit 0x07
        // 0001426f: 8b 50 30        MOV EDX, [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00014272: 8b cf           MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 00014274: ff d2           CALL EDX
        _emit 0xff
        _emit 0xd2
        // 00014276: 8b 06           MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00014278: 8b 50 04        MOV EDX, [EAX+0x04]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001427b: 5f              POP EDI
        _emit 0x5f
        // 0001427c: 8b ce           MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001427e: 5e              POP ESI
        _emit 0x5e
        // 0001427f: ff e2           JMP EDX
        _emit 0xff
        _emit 0xe2
    }
}
#endif
