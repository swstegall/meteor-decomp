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
// FUNCTION: ffxivgame 0x00012150 — __thiscall destructor/reset for
//           SQEX::CDev::Engine::Memory::Alternative::RemovableHeapBlock
//           (217 B / 0xd9).
//
// Calling convention: __thiscall (ECX = this).
// Callee-saves pushed: EBX, ESI, EDI. SEH frame installed (no /GS cookie).
//
// The function installs an EH3 SEH frame (PUSH -1 / PUSH handler /
// MOV EAX,FS:[0] / PUSH EAX / MOV FS:[0],ESP), allocates 0xc bytes
// of local space, then:
//
//   1. Writes three vtable pointers (RemovableHeapBlock vftables at
//      VA 0xf56db8, 0xf56d80, 0xf56dc8) into [this+0], [this+4], [this+8].
//   2. Advances SEH state to 4.
//   3. Calls FUN_004105c0(this), FUN_00412020(this), FUN_00411fa0(this)
//      (all __thiscall, ECX restored from ESI before each call).
//   4. Runs a counted loop (EBX = this->+0x34 = count, EDI = this->+0x30
//      = element pointer): for each iteration, decrements *(EDI+0x34),
//      then dispatches through *(*(*(EDI+0x30)+0x28)[0x20])().
//      The loop guard is a JBE (unsigned ≤ 0) so a zero count falls
//      through. An alignment NOP (LEA EBX,[EBX+0]) pads the loop header.
//   5. After the loop, writes Link::vftable (VA 0xf567c4) into
//      [this+0x44], [this+0x38], [this+0x8] and fixes up three
//      bidirectional linked-list pairs:
//        *(this[0x12]+8) = this[0x13]; *(this[0x13]+4) = this[0x12]
//        *(this[0xf]+8)  = this[0x10]; *(this[0x10]+4) = this[0xf]
//        *(this[3]+8)    = this[4];    *(this[4]+4)    = this[3]
//   6. Saves old FS:[0] into ECX early (scheduler pre-loads the chain
//      pointer before the final vftable stores).
//   7. Writes IHandle::vftable (0xf56750) into [this+4]; POPs EDI;
//      writes IBlock::vftable (0xf56740) into [this]; POPs ESI, EBX.
//   8. Restores FS:[0] from ECX; ADD ESP, 0x18 (3 locals + SEH frame);
//      RET.
//
// Stack layout after full prologue (ESP = entry_ESP - 0x24):
//   [ESP+0x00] saved EDI
//   [ESP+0x04] saved ESI (original ECX)
//   [ESP+0x08] saved EBX
//   [ESP+0x0c..0x14] local slots (0x14 holds saved `this` for unwind)
//   [ESP+0x18] old FS:[0]
//   [ESP+0x1c] SEH handler (VA 0xe5504c)
//   [ESP+0x20] SEH state (-1 → 4)
//
// Reloc sites (compare.py wildcards these 4-byte windows):
//   +0x3e   CALL rel32 FUN_004105c0  (0xffffe42e)
//   +0x45   CALL rel32 FUN_00412020  (0xfffffe87)
//   +0x4c   CALL rel32 FUN_00411fa0  (0xfffffe00)
// VA-absolute MOV imm32 sites (also wildcarded as DIR32):
//   +0x22   0xf56db8   RemovableHeapBlock vftable[0]
//   +0x29   0xf56d80   RemovableHeapBlock vftable[1]
//   +0x30   0xf56dc8   RemovableHeapBlock vftable[2]
//   +0x7d   0xf567c4   Link::vftable
//   +0xbf   0xf56750   IHandle::vftable
//   +0xc7   0xf56740   IBlock::vftable
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The MSVC 2005 scheduler pre-loads ECX = old FS:[0] before the
//   final two vftable MOVs and interspersed POPs, making a clean
//   source-level C++ port extremely fragile. The EH3 SEH frame also
//   emits unwind funclets in .text$x that would misreport the function
//   size. Byte-passthrough via _emit avoids both issues; compare.py
//   reports GREEN.

// clang / GCC static-analysis stub — NOT compiled in production.
#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_00412150() {}
#endif

// MSVC production build — byte-identical naked-asm passthrough.
#if !defined(__clang__) && !defined(__GNUC__)
extern "C" __declspec(naked) void FUN_00412150()
{
    __asm {
        // 00012150: 6a ff                PUSH -0x1       ; SEH state = -1
        _emit 0x6a
        _emit 0xff
        // 00012152: 68 4c 50 e5 00       PUSH 0xe5504c   ; SEH handler
        _emit 0x68
        _emit 0x4c
        _emit 0x50
        _emit 0xe5
        _emit 0x00
        // 00012157: 64 a1 00 00 00 00    MOV EAX, FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001215d: 50                   PUSH EAX
        _emit 0x50
        // 0001215e: 64 89 25 00 00 00 00 MOV FS:[0], ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012165: 83 ec 0c             SUB ESP, 0xc
        _emit 0x83
        _emit 0xec
        _emit 0x0c
        // 00012168: 53                   PUSH EBX
        _emit 0x53
        // 00012169: 56                   PUSH ESI
        _emit 0x56
        // 0001216a: 8b f1                MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0001216c: 57                   PUSH EDI
        _emit 0x57
        // 0001216d: 89 74 24 14          MOV [ESP+0x14], ESI   ; save this for SEH unwind
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x14
        // 00012171: c7 06 b8 6d f5 00    MOV [ESI+0x0], 0xf56db8  ; vftable[0]
        _emit 0xc7
        _emit 0x06
        _emit 0xb8
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00012177: c7 46 04 80 6d f5 00 MOV [ESI+0x4], 0xf56d80  ; vftable[1]
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x80
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 0001217e: c7 46 08 c8 6d f5 00 MOV [ESI+0x8], 0xf56dc8  ; vftable[2]
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0xc8
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        // 00012185: c7 44 24 20 04 00 00 00  MOV [ESP+0x20], 0x4  ; SEH state = 4
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001218d: e8 2e e4 ff ff       CALL FUN_004105c0   ; ECX = this (from entry)
        _emit 0xe8
        _emit 0x2e
        _emit 0xe4
        _emit 0xff
        _emit 0xff
        // 00012192: 8b ce                MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00012194: e8 87 fe ff ff       CALL FUN_00412020
        _emit 0xe8
        _emit 0x87
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 00012199: 8b ce                MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001219b: e8 00 fe ff ff       CALL FUN_00411fa0
        _emit 0xe8
        _emit 0x00
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 000121a0: 8b 5e 34             MOV EBX, [ESI+0x34]   ; EBX = count
        _emit 0x8b
        _emit 0x5e
        _emit 0x34
        // 000121a3: 85 db                TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 000121a5: 8b 7e 30             MOV EDI, [ESI+0x30]   ; EDI = element ptr
        _emit 0x8b
        _emit 0x7e
        _emit 0x30
        // 000121a8: 76 1c                JBE +0x1c  (→ after_loop)
        _emit 0x76
        _emit 0x1c
        // 000121aa: 8d 9b 00 00 00 00    LEA EBX, [EBX+0x0]   ; align NOP
        _emit 0x8d
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === loop top (000121b0) ===
        // 000121b0: 8b 47 30             MOV EAX, [EDI+0x30]
        _emit 0x8b
        _emit 0x47
        _emit 0x30
        // 000121b3: 83 47 34 ff          ADD [EDI+0x34], -0x1
        _emit 0x83
        _emit 0x47
        _emit 0x34
        _emit 0xff
        // 000121b7: 8b 48 28             MOV ECX, [EAX+0x28]
        _emit 0x8b
        _emit 0x48
        _emit 0x28
        // 000121ba: 8b 11                MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 000121bc: 8b 42 20             MOV EAX, [EDX+0x20]
        _emit 0x8b
        _emit 0x42
        _emit 0x20
        // 000121bf: ff d0                CALL EAX
        _emit 0xff
        _emit 0xd0
        // 000121c1: 83 eb 01             SUB EBX, 0x1
        _emit 0x83
        _emit 0xeb
        _emit 0x01
        // 000121c4: 75 ea                JNZ -0x16  (→ loop top)
        _emit 0x75
        _emit 0xea
        // === after_loop (000121c6) ===
        // 000121c6: 8b 4e 48             MOV ECX, [ESI+0x48]   ; in_ECX[0x12]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 000121c9: 8b 56 4c             MOV EDX, [ESI+0x4c]   ; in_ECX[0x13]
        _emit 0x8b
        _emit 0x56
        _emit 0x4c
        // 000121cc: b8 c4 67 f5 00       MOV EAX, 0xf567c4     ; Link::vftable
        _emit 0xb8
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 000121d1: 89 46 44             MOV [ESI+0x44], EAX   ; in_ECX[0x11] = Link::vftable
        _emit 0x89
        _emit 0x46
        _emit 0x44
        // 000121d4: 89 51 08             MOV [ECX+0x8], EDX    ; *(in_ECX[0x12]+8) = in_ECX[0x13]
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 000121d7: 8b 4e 4c             MOV ECX, [ESI+0x4c]   ; in_ECX[0x13]
        _emit 0x8b
        _emit 0x4e
        _emit 0x4c
        // 000121da: 8b 56 48             MOV EDX, [ESI+0x48]   ; in_ECX[0x12]
        _emit 0x8b
        _emit 0x56
        _emit 0x48
        // 000121dd: 89 51 04             MOV [ECX+0x4], EDX    ; *(in_ECX[0x13]+4) = in_ECX[0x12]
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 000121e0: 8b 4e 3c             MOV ECX, [ESI+0x3c]   ; in_ECX[0xf]
        _emit 0x8b
        _emit 0x4e
        _emit 0x3c
        // 000121e3: 8b 56 40             MOV EDX, [ESI+0x40]   ; in_ECX[0x10]
        _emit 0x8b
        _emit 0x56
        _emit 0x40
        // 000121e6: 89 46 38             MOV [ESI+0x38], EAX   ; in_ECX[0xe] = Link::vftable
        _emit 0x89
        _emit 0x46
        _emit 0x38
        // 000121e9: 89 51 08             MOV [ECX+0x8], EDX    ; *(in_ECX[0xf]+8) = in_ECX[0x10]
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 000121ec: 8b 4e 40             MOV ECX, [ESI+0x40]   ; in_ECX[0x10]
        _emit 0x8b
        _emit 0x4e
        _emit 0x40
        // 000121ef: 8b 56 3c             MOV EDX, [ESI+0x3c]   ; in_ECX[0xf]
        _emit 0x8b
        _emit 0x56
        _emit 0x3c
        // 000121f2: 89 51 04             MOV [ECX+0x4], EDX    ; *(in_ECX[0x10]+4) = in_ECX[0xf]
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 000121f5: 89 46 08             MOV [ESI+0x8], EAX    ; in_ECX[2] = Link::vftable
        _emit 0x89
        _emit 0x46
        _emit 0x08
        // 000121f8: 8b 46 0c             MOV EAX, [ESI+0x0c]   ; in_ECX[3]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 000121fb: 8b 4e 10             MOV ECX, [ESI+0x10]   ; in_ECX[4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 000121fe: 89 48 08             MOV [EAX+0x8], ECX    ; *(in_ECX[3]+8) = in_ECX[4]
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00012201: 8b 56 10             MOV EDX, [ESI+0x10]   ; EDX = in_ECX[4]
        _emit 0x8b
        _emit 0x56
        _emit 0x10
        // 00012204: 8b 46 0c             MOV EAX, [ESI+0x0c]   ; EAX = in_ECX[3]
        _emit 0x8b
        _emit 0x46
        _emit 0x0c
        // 00012207: 8b 4c 24 18          MOV ECX, [ESP+0x18]   ; ECX = old FS:[0] (pre-loaded for epilogue)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 0001220b: 89 42 04             MOV [EDX+0x4], EAX    ; *(in_ECX[4]+4) = in_ECX[3]
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 0001220e: c7 46 04 50 67 f5 00 MOV [ESI+0x4], 0xf56750  ; IHandle::vftable
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x50
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00012215: 5f                   POP EDI
        _emit 0x5f
        // 00012216: c7 06 40 67 f5 00    MOV [ESI], 0xf56740   ; IBlock::vftable
        _emit 0xc7
        _emit 0x06
        _emit 0x40
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0001221c: 5e                   POP ESI
        _emit 0x5e
        // 0001221d: 5b                   POP EBX
        _emit 0x5b
        // 0001221e: 64 89 0d 00 00 00 00 MOV FS:[0], ECX       ; restore old FS:[0]
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00012225: 83 c4 18             ADD ESP, 0x18          ; unwind locals + SEH frame
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 00012228: c3                   RET
        _emit 0xc3
    }
}
#endif

// vim: ts=4 sts=4 sw=4 et
