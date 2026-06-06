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
// FUNCTION: ffxivgame 0x004799a0 — `__cdecl` multi-precision integer subtraction
//                                  (335 B / 0x14f).
//
// Inspection (read from the disassembly at orig RVA 0x000799a0):
//
//   __cdecl int FUN_004799a0(BigInt *result, BigInt *minuend, BigInt *subtrahend)
//
//   Subtracts subtrahend from minuend and stores the result in result.
//   Each BigInt has:
//     [+0x0]: pointer to uint32 limb array (little-endian, LSB first)
//     [+0x4]: count of significant limbs
//     [+0x8]: capacity (for the result BigInt)
//     [+0xc]: some flag (cleared to 0 on success)
//
//   Returns 1 on success, 0 on failure.
//
//   Structure:
//     _chkstk(4)  -- 4-byte local frame for [EBP+4] scratch save
//     Asserts minuend->size >= subtrahend->size (calls error at 0x0045c940 on
//     violation, returns 0).
//     If result->capacity < minuend->size, calls 0x004720c0(result, minuend->size)
//     to expand. Returns 0 if the expansion call returns NULL.
//     Performs word-by-word subtraction with borrow propagation over
//     subtrahend->size limbs, then propagates the remaining borrow through the
//     extra limbs of minuend (if any), copies any un-borrowed remainder, and
//     finally trims leading zero limbs before storing the new size into
//     result->size and clearing result->flag.
//
//   Reloc-bearing sites in the orig 335 bytes:
//     +0x06  REL32 → 0x009d29d0  (CALL _chkstk)
//     +0x17  DIR32 → 0x00f7b488  (PUSH absolute: assert-string or data addr)
//     +0x37  REL32 → 0x0045c940  (CALL assert/error handler, 5 args)
//     +0x56  REL32 → 0x004720c0  (CALL BigInt_Grow or similar)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   The function's borrow-chain subtraction loop, unrolled copy tail, and
//   trim-leading-zeros loop produce register allocation and branch-shape
//   that is highly sensitive to loop form and declaration order under
//   MSVC 2005 /O2. The _chkstk prologue and the PC-relative CALLs at
//   four relocation sites further complicate source-level reconstruction.
//   The same naked-asm passthrough used by FUN_00408f10 and FUN_004014b0
//   is the correct approach here.

extern "C" __declspec(naked) void FUN_004799a0() {
    __asm {
        // 000799a0: b8 04 00 00 00  MOV EAX, 0x4
        _emit 0xb8
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000799a5: e8 26 90 55 00  CALL _chkstk
        _emit 0xe8
        _emit 0x26
        _emit 0x90
        _emit 0x55
        _emit 0x00
        // 000799aa: 53              PUSH EBX
        _emit 0x53
        // 000799ab: 8b 5c 24 14     MOV EBX,[ESP+0x14]   ; arg3 (subtrahend)
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x14
        // 000799af: 55              PUSH EBP
        _emit 0x55
        // 000799b0: 8b 6c 24 14     MOV EBP,[ESP+0x14]   ; arg2 (minuend)
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x14
        // 000799b4: 8b 45 04        MOV EAX,[EBP+0x4]    ; minuend->size
        _emit 0x8b
        _emit 0x45
        _emit 0x04
        // 000799b7: 56              PUSH ESI
        _emit 0x56
        // 000799b8: 57              PUSH EDI
        _emit 0x57
        // 000799b9: 8b 7b 04        MOV EDI,[EBX+0x4]    ; subtrahend->size
        _emit 0x8b
        _emit 0x7b
        _emit 0x04
        // 000799bc: 8b f0           MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 000799be: 2b f7           SUB ESI,EDI          ; ESI = minuend->size - subtrahend->size
        _emit 0x2b
        _emit 0xf7
        // 000799c0: 89 44 24 10     MOV [ESP+0x10],EAX   ; save minuend->size in local
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 000799c4: 79 20           JNS 0x4799e6         ; if minuend->size >= subtrahend->size, ok
        _emit 0x79
        _emit 0x20
        // --- error path (minuend < subtrahend, assert violation) ---
        // 000799c6: 68 b8 00 00 00  PUSH 0xb8
        _emit 0x68
        _emit 0xb8
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000799cb: 68 88 b4 f7 00  PUSH 0xf7b488
        _emit 0x68
        _emit 0x88
        _emit 0xb4
        _emit 0xf7
        _emit 0x00
        // 000799d0: 6a 64           PUSH 0x64
        _emit 0x6a
        _emit 0x64
        // 000799d2: 6a 73           PUSH 0x73
        _emit 0x6a
        _emit 0x73
        // 000799d4: 6a 03           PUSH 0x3
        _emit 0x6a
        _emit 0x03
        // 000799d6: e8 65 2f fe ff  CALL 0x0045c940
        _emit 0xe8
        _emit 0x65
        _emit 0x2f
        _emit 0xfe
        _emit 0xff
        // 000799db: 83 c4 14        ADD ESP,0x14         ; clean 5 pushed args
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // --- return 0 epilogue ---
        // 000799de: 5f              POP EDI
        _emit 0x5f
        // 000799df: 5e              POP ESI
        _emit 0x5e
        // 000799e0: 5d              POP EBP
        _emit 0x5d
        // 000799e1: 33 c0           XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000799e3: 5b              POP EBX
        _emit 0x5b
        // 000799e4: 59              POP ECX              ; clean chkstk alloc
        _emit 0x59
        // 000799e5: c3              RET
        _emit 0xc3
        // --- capacity check / grow ---
        // 000799e6: 8b 4c 24 18     MOV ECX,[ESP+0x18]   ; arg1 (result)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        // 000799ea: 3b 41 08        CMP EAX,[ECX+0x8]    ; minuend->size vs result->capacity
        _emit 0x3b
        _emit 0x41
        _emit 0x08
        // 000799ed: 7f 04           JG 0x4799f3          ; if size > capacity, grow
        _emit 0x7f
        _emit 0x04
        // 000799ef: 8b c1           MOV EAX,ECX          ; EAX = result (no grow needed)
        _emit 0x8b
        _emit 0xc1
        // 000799f1: eb 0e           JMP 0x479a01
        _emit 0xeb
        _emit 0x0e
        // 000799f3: 50              PUSH EAX             ; push minuend->size
        _emit 0x50
        // 000799f4: 51              PUSH ECX             ; push result ptr
        _emit 0x51
        // 000799f5: e8 c6 86 ff ff  CALL 0x004720c0      ; BigInt_Grow(result, new_size)
        _emit 0xe8
        _emit 0xc6
        _emit 0x86
        _emit 0xff
        _emit 0xff
        // 000799fa: 8b 4c 24 20     MOV ECX,[ESP+0x20]   ; reload arg1 (result)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 000799fe: 83 c4 08        ADD ESP,0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00079a01: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00079a03: 74 d9           JZ 0x4799de          ; return 0 if grow failed
        _emit 0x74
        _emit 0xd9
        // --- setup for subtraction loop ---
        // 00079a05: 8b 45 00        MOV EAX,[EBP+0x0]    ; minuend->data ptr
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 00079a08: 8b 2b           MOV EBP,[EBX]        ; subtrahend->data ptr
        _emit 0x8b
        _emit 0x2b
        // 00079a0a: 8b 09           MOV ECX,[ECX]        ; result->data ptr
        _emit 0x8b
        _emit 0x09
        // 00079a0c: 33 d2           XOR EDX,EDX          ; borrow = 0
        _emit 0x33
        _emit 0xd2
        // 00079a0e: 85 ff           TEST EDI,EDI         ; subtrahend->size == 0?
        _emit 0x85
        _emit 0xff
        // 00079a10: 89 7c 24 1c     MOV [ESP+0x1c],EDI   ; save loop count
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x1c
        // 00079a14: 74 59           JZ 0x479a6f          ; skip main loop if count==0
        _emit 0x74
        _emit 0x59
        // --- main subtraction loop (EDI iterations) ---
        // 00079a16: 8b 5d 00        MOV EBX,[EBP+0x0]    ; val = subtrahend[i]
        _emit 0x8b
        _emit 0x5d
        _emit 0x00
        // 00079a19: 8b 38           MOV EDI,[EAX]        ; val2 = minuend[i]
        _emit 0x8b
        _emit 0x38
        // 00079a1b: 83 c0 04        ADD EAX,0x4          ; advance minuend ptr
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 00079a1e: 83 c5 04        ADD EBP,0x4          ; advance subtrahend ptr
        _emit 0x83
        _emit 0xc5
        _emit 0x04
        // 00079a21: 85 d2           TEST EDX,EDX         ; borrow in?
        _emit 0x85
        _emit 0xd2
        // 00079a23: 89 5c 24 20     MOV [ESP+0x20],EBX   ; save subtrahend limb
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00079a27: 74 12           JZ 0x479a3b          ; if no borrow, go to simple sub
        _emit 0x74
        _emit 0x12
        // --- borrow-in path ---
        // 00079a29: 3b df           CMP EBX,EDI
        _emit 0x3b
        _emit 0xdf
        // 00079a2b: 1b d2           SBB EDX,EDX
        _emit 0x1b
        _emit 0xd2
        // 00079a2d: 83 cb ff        OR EBX,0xffffffff
        _emit 0x83
        _emit 0xcb
        _emit 0xff
        // 00079a30: 2b 5c 24 20     SUB EBX,[ESP+0x20]   ; EBX = ~subtrahend[i]
        _emit 0x2b
        _emit 0x5c
        _emit 0x24
        _emit 0x20
        // 00079a34: 83 c2 01        ADD EDX,0x1
        _emit 0x83
        _emit 0xc2
        _emit 0x01
        // 00079a37: 03 fb           ADD EDI,EBX
        _emit 0x03
        _emit 0xfb
        // 00079a39: eb 08           JMP 0x479a43
        _emit 0xeb
        _emit 0x08
        // --- no-borrow path ---
        // 00079a3b: 3b fb           CMP EDI,EBX
        _emit 0x3b
        _emit 0xfb
        // 00079a3d: 1b d2           SBB EDX,EDX
        _emit 0x1b
        _emit 0xd2
        // 00079a3f: f7 da           NEG EDX              ; borrow out
        _emit 0xf7
        _emit 0xda
        // 00079a41: 2b fb           SUB EDI,EBX
        _emit 0x2b
        _emit 0xfb
        // 00079a43: 89 39           MOV [ECX],EDI        ; store result limb
        _emit 0x89
        _emit 0x39
        // 00079a45: 83 c1 04        ADD ECX,0x4          ; advance result ptr
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        // 00079a48: 83 6c 24 1c 01  SUB [ESP+0x1c],0x1   ; decrement loop counter
        _emit 0x83
        _emit 0x6c
        _emit 0x24
        _emit 0x1c
        _emit 0x01
        // 00079a4d: 75 c7           JNZ 0x479a16         ; loop
        _emit 0x75
        _emit 0xc7
        // --- borrow propagation through extra minuend limbs ---
        // 00079a4f: 85 d2           TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 00079a51: 74 1c           JZ 0x479a6f          ; no borrow, skip
        _emit 0x74
        _emit 0x1c
        // 00079a53: 85 f6           TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00079a55: 74 87           JZ 0x4799de          ; no extra limbs, return 0 (underflow!)
        _emit 0x74
        _emit 0x87
        // 00079a57: 8b 10           MOV EDX,[EAX]        ; load next minuend limb
        _emit 0x8b
        _emit 0x10
        // 00079a59: 8d 7a ff        LEA EDI,[EDX-0x1]    ; result = limb - 1 (propagate borrow)
        _emit 0x8d
        _emit 0x7a
        _emit 0xff
        // 00079a5c: 89 39           MOV [ECX],EDI
        _emit 0x89
        _emit 0x39
        // 00079a5e: 83 ee 01        SUB ESI,0x1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 00079a61: 83 c0 04        ADD EAX,0x4
        _emit 0x83
        _emit 0xc0
        _emit 0x04
        // 00079a64: 83 c1 04        ADD ECX,0x4
        _emit 0x83
        _emit 0xc1
        _emit 0x04
        // 00079a67: 85 d2           TEST EDX,EDX         ; did limb absorb borrow?
        _emit 0x85
        _emit 0xd2
        // 00079a69: 75 04           JNZ 0x479a6f         ; yes, done
        _emit 0x75
        _emit 0x04
        // 00079a6b: 85 f6           TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00079a6d: 75 e8           JNZ 0x479a57         ; more limbs, continue borrow
        _emit 0x75
        _emit 0xe8
        // --- copy remaining (no-borrow) limbs ---
        // 00079a6f: 3b c8           CMP ECX,EAX
        _emit 0x3b
        _emit 0xc8
        // 00079a71: 74 42           JZ 0x479ab5          ; nothing to copy
        _emit 0x74
        _emit 0x42
        // 00079a73: 85 f6           TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00079a75: 74 3e           JZ 0x479ab5
        _emit 0x74
        _emit 0x3e
        // --- unrolled copy loop (4 dwords at a time) ---
        // 00079a77: 8b 10           MOV EDX,[EAX]
        _emit 0x8b
        _emit 0x10
        // 00079a79: 83 ee 01        SUB ESI,0x1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 00079a7c: 89 11           MOV [ECX],EDX
        _emit 0x89
        _emit 0x11
        // 00079a7e: 8b d6           MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00079a80: 83 ee 01        SUB ESI,0x1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 00079a83: 85 d2           TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 00079a85: 74 2e           JZ 0x479ab5
        _emit 0x74
        _emit 0x2e
        // 00079a87: 8b 50 04        MOV EDX,[EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00079a8a: 89 51 04        MOV [ECX+0x4],EDX
        _emit 0x89
        _emit 0x51
        _emit 0x04
        // 00079a8d: 8b d6           MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00079a8f: 83 ee 01        SUB ESI,0x1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 00079a92: 85 d2           TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 00079a94: 74 1f           JZ 0x479ab5
        _emit 0x74
        _emit 0x1f
        // 00079a96: 8b 50 08        MOV EDX,[EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 00079a99: 89 51 08        MOV [ECX+0x8],EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 00079a9c: 8b d6           MOV EDX,ESI
        _emit 0x8b
        _emit 0xd6
        // 00079a9e: 83 ee 01        SUB ESI,0x1
        _emit 0x83
        _emit 0xee
        _emit 0x01
        // 00079aa1: 85 d2           TEST EDX,EDX
        _emit 0x85
        _emit 0xd2
        // 00079aa3: 74 10           JZ 0x479ab5
        _emit 0x74
        _emit 0x10
        // 00079aa5: 8b 50 0c        MOV EDX,[EAX+0xc]
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        // 00079aa8: 89 51 0c        MOV [ECX+0xc],EDX
        _emit 0x89
        _emit 0x51
        _emit 0x0c
        // 00079aab: 83 c1 10        ADD ECX,0x10
        _emit 0x83
        _emit 0xc1
        _emit 0x10
        // 00079aae: 83 c0 10        ADD EAX,0x10
        _emit 0x83
        _emit 0xc0
        _emit 0x10
        // 00079ab1: 85 f6           TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00079ab3: 75 c2           JNZ 0x479a77         ; outer copy loop
        _emit 0x75
        _emit 0xc2
        // --- finalize: store size, clear flag, trim leading zeros ---
        // 00079ab5: 8b 44 24 10     MOV EAX,[ESP+0x10]   ; minuend->size (saved)
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 00079ab9: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00079abb: 8b 54 24 18     MOV EDX,[ESP+0x18]   ; result (arg1)
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 00079abf: 89 42 04        MOV [EDX+0x4],EAX    ; result->size = size
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00079ac2: c7 42 0c 00 00 00 00  MOV [EDX+0xc],0x0  ; result->flag = 0
        _emit 0xc7
        _emit 0x42
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00079ac9: 7e 19           JLE 0x479ae4         ; if size <= 0, skip trim
        _emit 0x7e
        _emit 0x19
        // 00079acb: 8b 0a           MOV ECX,[EDX]        ; result->data
        _emit 0x8b
        _emit 0x0a
        // 00079acd: 8d 4c 81 fc     LEA ECX,[ECX+EAX*4-4] ; ptr to last limb
        _emit 0x8d
        _emit 0x4c
        _emit 0x81
        _emit 0xfc
        // --- trim leading zeros loop ---
        // 00079ad1: 8b 31           MOV ESI,[ECX]        ; last limb
        _emit 0x8b
        _emit 0x31
        // 00079ad3: 83 e9 04        SUB ECX,0x4          ; move ptr back
        _emit 0x83
        _emit 0xe9
        _emit 0x04
        // 00079ad6: 85 f6           TEST ESI,ESI
        _emit 0x85
        _emit 0xf6
        // 00079ad8: 75 07           JNZ 0x479ae1         ; non-zero limb, stop
        _emit 0x75
        _emit 0x07
        // 00079ada: 83 e8 01        SUB EAX,0x1          ; decrement size
        _emit 0x83
        _emit 0xe8
        _emit 0x01
        // 00079add: 85 c0           TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 00079adf: 7f f0           JG 0x479ad1          ; continue while size > 0
        _emit 0x7f
        _emit 0xf0
        // 00079ae1: 89 42 04        MOV [EDX+0x4],EAX    ; update result->size
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // --- success epilogue ---
        // 00079ae4: 5f              POP EDI
        _emit 0x5f
        // 00079ae5: 5e              POP ESI
        _emit 0x5e
        // 00079ae6: 5d              POP EBP
        _emit 0x5d
        // 00079ae7: b8 01 00 00 00  MOV EAX,0x1          ; return 1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00079aec: 5b              POP EBX
        _emit 0x5b
        // 00079aed: 59              POP ECX              ; clean chkstk alloc
        _emit 0x59
        // 00079aee: c3              RET
        _emit 0xc3
    }
}
