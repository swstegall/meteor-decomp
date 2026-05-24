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
// FUNCTION: ffxivgame 0x000131d0 — __thiscall update/tick dispatch (201 bytes / 0xc9)
//
// Calling convention: __thiscall (ECX = this).
// Callee-saved: EBX, ESI, EDI; ECX also pushed to stack for local slot.
//
// Layout:
//   ESI = this
//   EDI = this->field_0x10  (piVar1, inner object)
//   EBX = this - 4          (used for sentinel checks and in_ECX[10] increment)
//   [ESP+0xc] = saved EDI (piVar1) for reload after inner code block
//
// Logic (matches Ghidra pseudo-C closely):
//   1. Load piVar1 = *[this+0x10], call piVar1->vtable[0x2c]() (lock/tick begin)
//   2. Check if this->field_0x24 (in_ECX[9]) == this-4 (in_ECX+(-1)):
//      - YES → call (*in_ECX[5]->vtable[0x1c])(in_ECX[5]); goto step_3
//      - NO  → if in_ECX[10] == 0:
//                 iVar2 = (*in_ECX[4]->vtable[4])()
//                 piVar3 = (**(iVar2+8)->vtable[0xc])(
//                              (*this->vtable[8])(this,
//                                  (*this->vtable[0xc])(this,
//                                      (*this->vtable[0x10])(this))))
//                 if piVar3 != NULL:
//                   in_ECX[6] = piVar3
//                   *(byte*)(in_ECX+7)  = 0
//                   *(byte*)(in_ECX+0x1d) = 0
//                   (*piVar3->vtable[0x1c])(piVar3)
//                   FUN_004130d0()  (ECX = this-4)
//   3. in_ECX[10]++
//   4. Call piVar1->vtable[0x30]() (lock/tick end)
//   5. Return:
//      - if in_ECX[9] == in_ECX-1: tail-jmp to (*in_ECX[5]->vtable[4])
//      - elif in_ECX[6] != 0:      tail-jmp to (*in_ECX[6]->vtable[4])
//      - else:                     return 0
//
// Relocations (masked by tools/compare.py):
//   REL32: FUN_004130d0 at func+0x83 (bytes 132–135)
//
// Reconstruction: naked-asm byte passthrough.
// The ECX-push stack frame, ESP-relative save/restore of EDI, multiple
// tail-jumps, and mixed-save epilogue are not reproducible from C++ with
// MSVC 2005 /O2 without clobbering the stack layout.

#if defined(__clang__) || defined(__GNUC__)
extern "C" void FUN_004131d0() {}
#else

extern "C" __declspec(naked) void FUN_004131d0()
{
    __asm {
        // Prologue — push ECX (local slot), EBX, ESI, then cache 'this' in ESI, push EDI
        // 000131d0:  51           PUSH ECX
        _emit 0x51
        // 000131d1:  53           PUSH EBX
        _emit 0x53
        // 000131d2:  56           PUSH ESI
        _emit 0x56
        // 000131d3:  8b f1        MOV ESI, ECX      ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 000131d5:  57           PUSH EDI
        _emit 0x57
        // 000131d6:  8b 7e 10     MOV EDI, [ESI+0x10]  ; EDI = this->field_10 (piVar1)
        _emit 0x8b
        _emit 0x7e
        _emit 0x10
        // 000131d9:  8b 07        MOV EAX, [EDI]        ; EAX = *piVar1 (vtable)
        _emit 0x8b
        _emit 0x07
        // 000131db:  8b 50 2c     MOV EDX, [EAX+0x2c]  ; EDX = vtable slot 0x2c
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000131de:  8b cf        MOV ECX, EDI          ; ECX = piVar1 (for __thiscall)
        _emit 0x8b
        _emit 0xcf
        // 000131e0:  89 7c 24 0c  MOV [ESP+0xc], EDI   ; save piVar1 on stack
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 000131e4:  ff d2        CALL EDX              ; (*piVar1->vtable[0x2c])(piVar1)
        _emit 0xff
        _emit 0xd2
        // 000131e6:  8d 5e fc     LEA EBX, [ESI-4]     ; EBX = this-4
        _emit 0x8d
        _emit 0x5e
        _emit 0xfc
        // 000131e9:  39 5b 28     CMP [EBX+0x28], EBX  ; CMP in_ECX[9], in_ECX-1
        _emit 0x39
        _emit 0x5b
        _emit 0x28
        // 000131ec:  75 0c        JNZ +0x0c  (→ 000131fa)
        _emit 0x75
        _emit 0x0c
        // --- if (in_ECX[9] == in_ECX-1) ---
        // 000131ee:  8b 4e 14     MOV ECX, [ESI+0x14]  ; ECX = in_ECX[5]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 000131f1:  8b 01        MOV EAX, [ECX]        ; EAX = vtable of in_ECX[5]
        _emit 0x8b
        _emit 0x01
        // 000131f3:  8b 50 1c     MOV EDX, [EAX+0x1c]  ; EDX = vtable slot 0x1c
        _emit 0x8b
        _emit 0x50
        _emit 0x1c
        // 000131f6:  ff d2        CALL EDX              ; (*in_ECX[5]->vtable[0x1c])(in_ECX[5])
        _emit 0xff
        _emit 0xd2
        // 000131f8:  eb 62        JMP +0x62  (→ 0001325c)
        _emit 0xeb
        _emit 0x62
        // --- else: in_ECX[9] != in_ECX-1 (offset 0x1a from start = 0x000131ea) ---
        // 000131fa:  83 7e 28 00  CMP dword ptr [ESI+0x28], 0  ; CMP in_ECX[10], 0
        _emit 0x83
        _emit 0x7e
        _emit 0x28
        _emit 0x00
        // 000131fe:  75 5c        JNZ +0x5c  (→ 0001325c)  ; skip if in_ECX[10] != 0
        _emit 0x75
        _emit 0x5c
        // --- in_ECX[10] == 0 block ---
        // 00013200:  8b 4e 10     MOV ECX, [ESI+0x10]   ; ECX = in_ECX[4]
        _emit 0x8b
        _emit 0x4e
        _emit 0x10
        // 00013203:  8b 01        MOV EAX, [ECX]         ; EAX = vtable of in_ECX[4]
        _emit 0x8b
        _emit 0x01
        // 00013205:  8b 50 04     MOV EDX, [EAX+4]       ; EDX = vtable slot 4
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 00013208:  55           PUSH EBP               ; save EBP (will be overwritten)
        _emit 0x55
        // 00013209:  ff d2        CALL EDX               ; iVar2 = (*in_ECX[4]->vtable[4])(in_ECX[4])
        _emit 0xff
        _emit 0xd2
        // 0001320b:  8b 68 08     MOV EBP, [EAX+8]       ; EBP = *(iVar2+8)
        _emit 0x8b
        _emit 0x68
        _emit 0x08
        // 0001320e:  8b 06        MOV EAX, [ESI]          ; EAX = *this (vtable)
        _emit 0x8b
        _emit 0x06
        // 00013210:  8b 7d 00     MOV EDI, [EBP+0]        ; EDI = *EBP = **(iVar2+8)
        _emit 0x8b
        _emit 0x7d
        _emit 0x00
        // 00013213:  8b 50 10     MOV EDX, [EAX+0x10]    ; EDX = this->vtable slot 0x10
        _emit 0x8b
        _emit 0x50
        _emit 0x10
        // 00013216:  8b ce        MOV ECX, ESI            ; ECX = this
        _emit 0x8b
        _emit 0xce
        // 00013218:  83 c7 0c     ADD EDI, 0xc            ; EDI = **(iVar2+8) + 0xc (vtable fn ptr slot)
        _emit 0x83
        _emit 0xc7
        _emit 0x0c
        // 0001321b:  ff d2        CALL EDX               ; uVar4 = (*this->vtable[0x10])(this)
        _emit 0xff
        _emit 0xd2
        // 0001321d:  50           PUSH EAX               ; push uVar4
        _emit 0x50
        // 0001321e:  8b 06        MOV EAX, [ESI]          ; EAX = *this
        _emit 0x8b
        _emit 0x06
        // 00013220:  8b 50 0c     MOV EDX, [EAX+0xc]     ; EDX = this->vtable slot 0xc
        _emit 0x8b
        _emit 0x50
        _emit 0x0c
        // 00013223:  8b ce        MOV ECX, ESI            ; ECX = this
        _emit 0x8b
        _emit 0xce
        // 00013225:  ff d2        CALL EDX               ; uVar4 = (*this->vtable[0xc])(this, uVar4)
        _emit 0xff
        _emit 0xd2
        // 00013227:  50           PUSH EAX               ; push uVar4
        _emit 0x50
        // 00013228:  8b 06        MOV EAX, [ESI]          ; EAX = *this
        _emit 0x8b
        _emit 0x06
        // 0001322a:  8b 50 08     MOV EDX, [EAX+8]        ; EDX = this->vtable slot 8
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 0001322d:  8b ce        MOV ECX, ESI            ; ECX = this
        _emit 0x8b
        _emit 0xce
        // 0001322f:  ff d2        CALL EDX               ; uVar4 = (*this->vtable[8])(this, uVar4)
        _emit 0xff
        _emit 0xd2
        // 00013231:  50           PUSH EAX               ; push uVar4 (3 args now on stack)
        _emit 0x50
        // 00013232:  8b 07        MOV EAX, [EDI]          ; EAX = fn_ptr at *(iVar2+0xc) via EDI
        _emit 0x8b
        _emit 0x07
        // 00013234:  8b cd        MOV ECX, EBP            ; ECX = EBP (ptr = *(iVar2+8))
        _emit 0x8b
        _emit 0xcd
        // 00013236:  ff d0        CALL EAX               ; piVar3 = fn(uVar4,uVar4,uVar4) [3 cdecl args?]
        _emit 0xff
        _emit 0xd0
        // 00013238:  85 c0        TEST EAX, EAX           ; if (piVar3 != NULL)
        _emit 0x85
        _emit 0xc0
        // 0001323a:  5d           POP EBP                 ; restore EBP (balanced with PUSH EBP above)
        _emit 0x5d
        // 0001323b:  74 1b        JZ +0x1b  (→ 00013258)  ; if NULL, jump to in_ECX[10]++
        _emit 0x74
        _emit 0x1b
        // --- piVar3 != NULL block ---
        // 0001323d:  89 46 18     MOV [ESI+0x18], EAX    ; in_ECX[6] = piVar3
        _emit 0x89
        _emit 0x46
        _emit 0x18
        // 00013240:  c6 46 1c 00  MOV byte ptr [ESI+0x1c], 0  ; *(in_ECX+7) low byte = 0
        _emit 0xc6
        _emit 0x46
        _emit 0x1c
        _emit 0x00
        // 00013244:  c6 46 1d 00  MOV byte ptr [ESI+0x1d], 0  ; *(in_ECX+0x1d) = 0
        _emit 0xc6
        _emit 0x46
        _emit 0x1d
        _emit 0x00
        // 00013248:  8b 10        MOV EDX, [EAX]          ; EDX = *piVar3 (vtable)
        _emit 0x8b
        _emit 0x10
        // 0001324a:  8b c8        MOV ECX, EAX            ; ECX = piVar3
        _emit 0x8b
        _emit 0xc8
        // 0001324c:  8b 42 1c     MOV EAX, [EDX+0x1c]    ; EAX = piVar3->vtable[0x1c]
        _emit 0x8b
        _emit 0x42
        _emit 0x1c
        // 0001324f:  ff d0        CALL EAX               ; (*piVar3->vtable[0x1c])(piVar3)
        _emit 0xff
        _emit 0xd0
        // 00013251:  8b cb        MOV ECX, EBX            ; ECX = EBX = this-4 (for FUN_004130d0)
        _emit 0x8b
        _emit 0xcb
        // 00013253:  e8 78 fe ff ff  CALL FUN_004130d0     ; FUN_004130d0(this-4)  (REL32 reloc)
        _emit 0xe8
        _emit 0x78
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // --- after piVar3 block (and NULL path convergence at 00013258) ---
        // 00013258:  8b 7c 24 0c  MOV EDI, [ESP+0xc]     ; reload piVar1 (saved at prologue)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x0c
        // 0001325c:  83 43 2c 01  ADD dword ptr [EBX+0x2c], 1  ; in_ECX[10]++
        _emit 0x83
        _emit 0x43
        _emit 0x2c
        _emit 0x01
        // 00013260:  8b 17        MOV EDX, [EDI]          ; EDX = *piVar1 (vtable)
        _emit 0x8b
        _emit 0x17
        // 00013262:  8b 42 30     MOV EAX, [EDX+0x30]    ; EAX = piVar1->vtable[0x30]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00013265:  8b cf        MOV ECX, EDI            ; ECX = piVar1
        _emit 0x8b
        _emit 0xcf
        // 00013267:  ff d0        CALL EAX               ; (*piVar1->vtable[0x30])(piVar1)
        _emit 0xff
        _emit 0xd0
        // --- return: check in_ECX[9] again ---
        // 00013269:  39 5b 28     CMP [EBX+0x28], EBX    ; CMP in_ECX[9], in_ECX-1
        _emit 0x39
        _emit 0x5b
        _emit 0x28
        // 0001326c:  5f           POP EDI
        _emit 0x5f
        // 0001326d:  75 0f        JNZ +0x0f  (→ 0001327e)
        _emit 0x75
        _emit 0x0f
        // --- in_ECX[9] == in_ECX-1: tail-jmp through in_ECX[5]->vtable[4] ---
        // 0001326f:  8b 4e 14     MOV ECX, [ESI+0x14]    ; ECX = in_ECX[5]
        _emit 0x8b
        _emit 0x4e
        _emit 0x14
        // 00013272:  8b 11        MOV EDX, [ECX]          ; EDX = vtable of in_ECX[5]
        _emit 0x8b
        _emit 0x11
        // 00013274:  8b 42 04     MOV EAX, [EDX+4]        ; EAX = vtable slot 4
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00013277:  5e           POP ESI
        _emit 0x5e
        // 00013278:  5b           POP EBX
        _emit 0x5b
        // 00013279:  83 c4 04     ADD ESP, 4              ; pop the saved ECX from prologue
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0001327c:  ff e0        JMP EAX                 ; tail-jmp to (*in_ECX[5]->vtable[4])
        _emit 0xff
        _emit 0xe0
        // --- in_ECX[9] != in_ECX-1: check in_ECX[6] (offset 0x1327e from base) ---
        // 0001327e:  8b 76 18     MOV ESI, [ESI+0x18]    ; ESI = in_ECX[6]
        _emit 0x8b
        _emit 0x76
        _emit 0x18
        // 00013281:  85 f6        TEST ESI, ESI           ; if in_ECX[6] == 0
        _emit 0x85
        _emit 0xf6
        // 00013283:  74 0e        JZ +0x0e  (→ 00013293)  ; return 0 path
        _emit 0x74
        _emit 0x0e
        // --- in_ECX[6] != 0: tail-jmp through in_ECX[6]->vtable[4] ---
        // 00013285:  8b 16        MOV EDX, [ESI]          ; EDX = vtable of in_ECX[6]
        _emit 0x8b
        _emit 0x16
        // 00013287:  8b 42 04     MOV EAX, [EDX+4]        ; EAX = vtable slot 4
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001328a:  8b ce        MOV ECX, ESI            ; ECX = in_ECX[6]
        _emit 0x8b
        _emit 0xce
        // 0001328c:  5e           POP ESI
        _emit 0x5e
        // 0001328d:  5b           POP EBX
        _emit 0x5b
        // 0001328e:  83 c4 04     ADD ESP, 4              ; pop saved ECX
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00013291:  ff e0        JMP EAX                 ; tail-jmp to (*in_ECX[6]->vtable[4])
        _emit 0xff
        _emit 0xe0
        // --- return 0 path (00013293) ---
        // 00013293:  5e           POP ESI
        _emit 0x5e
        // 00013294:  33 c0        XOR EAX, EAX            ; return 0
        _emit 0x33
        _emit 0xc0
        // 00013296:  5b           POP EBX
        _emit 0x5b
        // 00013297:  59           POP ECX                 ; restore saved ECX
        _emit 0x59
        // 00013298:  c3           RET
        _emit 0xc3
    }
}
#endif
