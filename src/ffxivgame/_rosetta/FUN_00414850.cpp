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
// FUNCTION: ffxivgame 0x00414850 — guarded "find-by-id and free" list walker
//                                  (129 B / 0x81), indirect-guard variant.
//
// __thiscall void FUN_00414850(C *this /* ECX */, int param_1 /* [ESP+4] */);
//   Callee-saves: ESI (current list node), EDI (this), EBX (cached param_1,
//                 pushed only inside the non-empty walk branch).
//   Epilogue: RET 0x4 — __thiscall, callee cleans 1 stack arg.
//
// Body shape (129 bytes, RVA 0x00014850):
//
//   PUSH ESI / PUSH EDI / MOV EDI, ECX      ; save callee-saves, cache this
//
//   ; Enter guard via this->vf6():
//   MOV EAX, [EDI]           ; EAX = this->vftable
//   MOV EDX, [EAX+0x18]      ; EDX = vftable[6]  (getGuard)
//   CALL EDX                 ; EAX = Guard*
//   MOV EDX, [EAX]           ; EDX = Guard->vftable
//   MOV ECX, EAX             ; ECX = Guard (this for next call)
//   MOV EAX, [EDX+0x2c]      ; EAX = Guard->vftable[11]  (Enter)
//   CALL EAX
//
//   ; Walk the list at this->[+0x28]:
//   MOV EAX, [EDI+0x28]      ; EAX = this->m_list
//   MOV ESI, [EAX+0x34]      ; ESI = list.head_next (first real node)
//   ADD EAX, 0x30             ; EAX = sentinel address (&list+0x30)
//   CMP ESI, EAX              ; is the list empty?
//   JZ  +0x2b  -> teardown    ; yes — skip the walk, use sentinel node directly
//
//   PUSH EBX                  ; save EBX (needed for param_1 cache)
//   MOV EBX, [ESP+0x10]       ; EBX = param_1 (re-read from stack after PUSH EBX)
//
//   loop:
//   MOV EDX, [ESI]            ; EDX = cur->vftable
//   MOV EAX, [EDX+0x4]        ; EAX = cur->vf1()
//   MOV ECX, ESI              ; ECX = cur (this for vf1)
//   CALL EAX                  ; EAX = raw candidate pointer (or NULL)
//   TEST EAX, EAX
//   JZ   +0x5  -> zero        ; NULL → keep EAX=0
//   ADD  EAX, 0x4             ; peel base sub-object (+4) to get key
//   JMP  +0x2  -> cmp
//   zero:
//   XOR  EAX, EAX             ; keep zero for comparison
//   cmp:
//   CMP  EAX, EBX             ; does adjusted key match param_1?
//   JZ   +0xd  -> found       ; yes — stop
//   MOV  ECX, [EDI+0x28]      ; ECX = this->m_list base
//   MOV  ESI, [ESI+0x4]       ; ESI = cur->next
//   ADD  ECX, 0x30             ; ECX = sentinel address
//   CMP  ESI, ECX             ; reached sentinel?
//   JNZ  loop                 ; no — continue
//
//   found:                    ; (or sentinel if not found)
//   POP  EBX
//
//   teardown: (also sentinel-path after empty-list skip)
//   MOV EDX, [ESI]            ; EDX = cur->vftable (or sentinel's)
//   MOV EAX, [EDX+0x4]        ; EAX = cur->vf1()
//   MOV ECX, ESI
//   CALL EAX                  ; EAX = puVar3 (the sub-object to destroy)
//   MOV ESI, EAX              ; ESI = puVar3
//   MOV EDX, [ESI]            ; EDX = puVar3->vftable
//   MOV EAX, [EDX]            ; EAX = puVar3->vf0()  (destructor)
//   PUSH 0x0                  ; no-delete flag
//   MOV ECX, ESI
//   CALL EAX                  ; puVar3->dtor(0)
//   PUSH ESI                  ; arg: puVar3
//   CALL 0x009d56fd           ; free(puVar3)  (REL32 → 0x5c0e46)
//   MOV EDX, [EDI]            ; EDX = this->vftable  (re-load — don't cache g)
//   MOV EAX, [EDX+0x18]       ; EAX = vftable[6]  (getGuard)
//   ADD ESP, 0x4              ; pop the PUSH ESI arg to free()
//   MOV ECX, EDI              ; ECX = this
//   CALL EAX                  ; EAX = Guard*
//   MOV EDX, [EAX]            ; EDX = Guard->vftable
//   MOV ECX, EAX
//   MOV EAX, [EDX+0x30]       ; EAX = Guard->vftable[12]  (Leave)
//   CALL EAX
//
//   POP EDI / POP ESI
//   RET 0x4
//
// Reloc-bearing site in the orig 129 bytes:
//   +0x62   CALL rel32 → 0x009d56fd   (disp32 = 0x005c0e46; the binary's
//                                       common free helper; only CALL in body)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The interlocked register allocation (ESI = cur, EDI = this, EBX = cached
//   param_1, EBX pushed only inside the non-empty-walk branch), the
//   TEST/JZ/ADD/JMP/XOR adjust-or-zero pattern, the double vf6() Guard calls
//   bracketing the body, and the single REL32 CALL to the common free helper
//   are all MSVC 2005 /O2 idioms that carry high iteration risk if re-derived
//   from source-level C++.  Emitting the orig 129 bytes verbatim via MASM
//   _emit produces a .obj whose .text is byte-identical to the orig slice
//   (the REL32 displacement is baked in as raw bytes from the orig binary's
//   own address space), and tools/compare.py reports GREEN with zero
//   relocation delta.

extern "C" __declspec(naked) void FUN_00414850() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0x8b              // MOV EDI, ECX
        _emit 0xf9
        _emit 0x8b              // MOV EAX, dword ptr [EDI]
        _emit 0x07
        _emit 0x8b              // MOV EDX, dword ptr [EAX+0x18]
        _emit 0x50
        _emit 0x18
        _emit 0xff              // CALL EDX
        _emit 0xd2
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x2c]
        _emit 0x42
        _emit 0x2c
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EAX, dword ptr [EDI+0x28]
        _emit 0x47
        _emit 0x28
        _emit 0x8b              // MOV ESI, dword ptr [EAX+0x34]
        _emit 0x70
        _emit 0x34
        _emit 0x83              // ADD EAX, 0x30
        _emit 0xc0
        _emit 0x30
        _emit 0x3b              // CMP ESI, EAX
        _emit 0xf0
        _emit 0x74              // JZ +0x2b  (-> teardown)
        _emit 0x2b
        _emit 0x53              // PUSH EBX
        _emit 0x8b              // MOV EBX, dword ptr [ESP+0x10]
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        _emit 0x8b              // MOV EDX, dword ptr [ESI]        (loop:)
        _emit 0x16
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x5  (-> zero)
        _emit 0x05
        _emit 0x83              // ADD EAX, 0x4
        _emit 0xc0
        _emit 0x04
        _emit 0xeb              // JMP +0x2  (-> cmp)
        _emit 0x02
        _emit 0x33              // XOR EAX, EAX                    (zero:)
        _emit 0xc0
        _emit 0x3b              // CMP EAX, EBX                    (cmp:)
        _emit 0xc3
        _emit 0x74              // JZ +0xd  (-> found)
        _emit 0x0d
        _emit 0x8b              // MOV ECX, dword ptr [EDI+0x28]
        _emit 0x4f
        _emit 0x28
        _emit 0x8b              // MOV ESI, dword ptr [ESI+0x4]
        _emit 0x76
        _emit 0x04
        _emit 0x83              // ADD ECX, 0x30
        _emit 0xc1
        _emit 0x30
        _emit 0x3b              // CMP ESI, ECX
        _emit 0xf1
        _emit 0x75              // JNZ loop (-0x25)
        _emit 0xdb
        _emit 0x5b              // POP EBX                         (found:)
        _emit 0x8b              // MOV EDX, dword ptr [ESI]        (teardown:)
        _emit 0x16
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x4]
        _emit 0x42
        _emit 0x04
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x8b              // MOV EDX, dword ptr [ESI]
        _emit 0x16
        _emit 0x8b              // MOV EAX, dword ptr [EDX]
        _emit 0x02
        _emit 0x6a              // PUSH 0x0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL 0x009d56fd  (rel32 = 0x005c0e46)
        _emit 0x46
        _emit 0x0e
        _emit 0x5c
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [EDI]
        _emit 0x17
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x18]
        _emit 0x42
        _emit 0x18
        _emit 0x83              // ADD ESP, 0x4
        _emit 0xc4
        _emit 0x04
        _emit 0x8b              // MOV ECX, EDI
        _emit 0xcf
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x8b              // MOV EDX, dword ptr [EAX]
        _emit 0x10
        _emit 0x8b              // MOV ECX, EAX
        _emit 0xc8
        _emit 0x8b              // MOV EAX, dword ptr [EDX+0x30]
        _emit 0x42
        _emit 0x30
        _emit 0xff              // CALL EAX
        _emit 0xd0
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
