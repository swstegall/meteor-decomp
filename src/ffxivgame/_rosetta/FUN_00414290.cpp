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
// FUNCTION: ffxivgame 0x00014290 — __thiscall guarded release-and-tail-dispatch
//           (128 bytes; engine_memory module — multi-inheritance sub-object
//            offset +0x04 of the outer object, same shape family as
//            SQEX::CDev::Engine::Memory::Alternative — see
//            decomp-notes/types/ffxivgame/0x000139d0.md).
//
// Behaviour (from disassembly):
//   1. Invokes (this->_10)->vtable[0x2c] (slot 11) — e.g. a paired
//      "Begin" / claim on the contained sub-allocator.
//   2. Decrements this->_2c (a reference / claim counter).
//   3. If the counter hits zero, evaluates a small state machine on
//      three byte flags + one int flag and runs one of two cleanup
//      branches:
//        - this->_28 == 0 && this->_21 == 0  →  invokes
//          (this->_14)->vtable[0x20] (slot 8), then calls the local
//          helper FUN_004140e0 with `(this - 4)` (the outer object).
//        - else, if this->_23 != 0           →  clears flag this->_20,
//          calls FUN_00413940(this - 4), and if this->_1c != 0 also
//          chases a two-level indirect dispatch via
//          (this->_10)->vtable[1] → result+0x0c → vtable[4] and clears
//          this->_22.
//   4. Unconditionally tail-jumps through (this->_10)->vtable[0x30]
//      (slot 12) — the matching "End" / release.
//
// Calling convention: __thiscall, no stack args (virtual invocation).
//
// Reconstruction strategy — __declspec(naked) byte passthrough:
//   MSVC 2005 /O2 picks an unusual prologue order: PUSH EBX/EBP/ESI,
//   then several vtable loads + `this`-caching, only THEN PUSH EDI
//   interleaved between MOV EAX,[EBP] and MOV ECX,EBP at +0x0e. The
//   epilogue pops EDI/ESI, sets ECX=EBP, then pops EBP/EBX and tail-
//   JMPs through EDX — an indirect tail-call shape source-level MSVC
//   2005 codegen will not reproduce verbatim from a normal
//   `return ((*this->_10)->vtable[12])(this->_10);` because the
//   compiler does not tail-call indirect virtuals. Naked locks the
//   exact byte layout.

extern "C" void FUN_004140e0();
extern "C" void FUN_00413940();

extern "C" __declspec(naked) void FUN_00414290()
{
    __asm {
        // 00414290: PUSH EBX
        push ebx
        // 00414291: PUSH EBP
        push ebp
        // 00414292: PUSH ESI
        push esi
        // 00414293: MOV ESI, ECX                 ; cache `this`
        mov esi, ecx
        // 00414295: MOV EBP, [ESI+0x10]          ; ebp = this->_10
        mov ebp, [esi + 0x10]
        // 00414298: MOV EAX, [EBP+0]             ; eax = vtable(this->_10)
        mov eax, [ebp + 0]
        // 0041429b: MOV EDX, [EAX+0x2c]          ; edx = vtable[11]
        mov edx, [eax + 0x2c]
        // 0041429e: PUSH EDI
        push edi
        // 0041429f: MOV ECX, EBP                 ; ecx = this->_10
        mov ecx, ebp
        // 004142a1: CALL EDX                     ; (this->_10)->vtable[11]()
        call edx
        // 004142a3: ADD DWORD PTR [ESI+0x2c], -1 ; --this->_2c
        add dword ptr [esi + 0x2c], -1
        // 004142a7: LEA EDI, [ESI-4]             ; edi = this - 4 (outer)
        lea edi, [esi - 4]
        // 004142aa: XOR EBX, EBX                 ; ebx = 0 (zero constant)
        xor ebx, ebx
        // 004142ac: CMP [ESI+0x2c], EBX          ; this->_2c == 0?
        cmp [esi + 0x2c], ebx
        // 004142af: JNE end                      ; → tail-dispatch
        jne L_end
        // 004142b1: CMP [ESI+0x28], EBX          ; this->_28 == 0?
        cmp [esi + 0x28], ebx
        // 004142b4: JNE else                     ; → flag-23 branch
        jne L_else
        // 004142b6: CMP BYTE PTR [ESI+0x21], BL  ; this->_21 == 0?
        cmp byte ptr [esi + 0x21], bl
        // 004142b9: JNE else
        jne L_else
        // 004142bb: MOV ECX, [ESI+0x14]
        mov ecx, [esi + 0x14]
        // 004142be: MOV EAX, [ECX]
        mov eax, [ecx]
        // 004142c0: MOV EDX, [EAX+0x20]          ; vtable[8] on this->_14
        mov edx, [eax + 0x20]
        // 004142c3: CALL EDX
        call edx
        // 004142c5: MOV ECX, EDI                 ; ecx = this - 4
        mov ecx, edi
        // 004142c7: CALL FUN_004140e0
        call FUN_004140e0
        // 004142cc: JMP end
        jmp L_end
    L_else:
        // 004142ce: CMP BYTE PTR [ESI+0x23], BL  ; this->_23 == 0?
        cmp byte ptr [esi + 0x23], bl
        // 004142d1: JE end
        je L_end
        // 004142d3: MOV ECX, EDI
        mov ecx, edi
        // 004142d5: MOV BYTE PTR [ESI+0x20], BL  ; this->_20 = 0
        mov byte ptr [esi + 0x20], bl
        // 004142d8: CALL FUN_00413940
        call FUN_00413940
        // 004142dd: MOV EDI, [ESI+0x1c]          ; edi = this->_1c
        mov edi, [esi + 0x1c]
        // 004142e0: CMP EDI, EBX                 ; this->_1c == 0?
        cmp edi, ebx
        // 004142e2: MOV BYTE PTR [ESI+0x23], BL  ; this->_23 = 0
        mov byte ptr [esi + 0x23], bl
        // 004142e5: JE end
        je L_end
        // 004142e7: MOV ECX, [ESI+0x10]
        mov ecx, [esi + 0x10]
        // 004142ea: MOV [ESI+0x1c], EBX          ; this->_1c = 0
        mov [esi + 0x1c], ebx
        // 004142ed: MOV EAX, [ECX]
        mov eax, [ecx]
        // 004142ef: MOV EDX, [EAX+0x4]           ; vtable[1] on this->_10
        mov edx, [eax + 0x4]
        // 004142f2: CALL EDX
        call edx
        // 004142f4: MOV ECX, [EAX+0xc]           ; inner +0xc
        mov ecx, [eax + 0x0c]
        // 004142f7: MOV EAX, [ECX]
        mov eax, [ecx]
        // 004142f9: MOV EDX, [EAX+0x10]          ; vtable[4]
        mov edx, [eax + 0x10]
        // 004142fc: PUSH EDI                     ; arg = old this->_1c
        push edi
        // 004142fd: CALL EDX
        call edx
        // 004142ff: MOV BYTE PTR [ESI+0x22], BL  ; this->_22 = 0
        mov byte ptr [esi + 0x22], bl
    L_end:
        // 00414302: MOV EAX, [EBP+0]             ; reload vtable(this->_10)
        mov eax, [ebp + 0]
        // 00414305: MOV EDX, [EAX+0x30]          ; edx = vtable[12]
        mov edx, [eax + 0x30]
        // 00414308: POP EDI
        pop edi
        // 00414309: POP ESI
        pop esi
        // 0041430a: MOV ECX, EBP                 ; ecx = this->_10
        mov ecx, ebp
        // 0041430c: POP EBP
        pop ebp
        // 0041430d: POP EBX
        pop ebx
        // 0041430e: JMP EDX                      ; tail-call vtable[12]
        jmp edx
    }
}
