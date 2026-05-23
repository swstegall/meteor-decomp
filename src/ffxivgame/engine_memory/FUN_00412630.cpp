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
// FUNCTION: ffxivgame 0x00012630 — __thiscall, no stack args (RET), 55 bytes
//           Advance-current-node iterator step.
//
// ESI = this (cached throughout)
// ECX = field_0C on entry → iVar2 after branch → *(iVar2+8) = node obj ptr
// EAX = result (0 initially, virtual-call return value if called)
// EDX = *(this+4) → sentinel (*(this+4)+0x20) → vtable[1] fn ptr
//
// Returns: (this+8) cast to INodeObj* if a next node was found, else 0.
//
// Asm (55 bytes @ orig RVA 0x00012630):
//   56              PUSH ESI
//   8b f1           MOV ESI, ECX
//   8b 4e 0c        MOV ECX, [ESI+0Ch]    ; ECX = field_0C
//   33 c0           XOR EAX, EAX          ; result = 0
//   85 c9           TEST ECX, ECX
//   74 05           JZ  +5  → xor_ecx
//   83 c1 08        ADD ECX, 8            ; iVar2 = field_0C + 8
//   eb 02           JMP +2  → after
//   33 c9           XOR ECX, ECX          ; xor_ecx: iVar2 = 0
//   8b 56 04        MOV EDX, [ESI+4]      ; after: EDX = *(this+4)
//   8b 49 08        MOV ECX, [ECX+8]      ; ECX = *(iVar2+8) = node ptr
//   83 c2 20        ADD EDX, 0x20         ; sentinel = *(this+4) + 0x20
//   3b ca           CMP ECX, EDX
//   74 07           JZ  +7  → test_eax    ; skip call if at sentinel
//   8b 01           MOV EAX, [ECX]        ; vtable of node
//   8b 50 04        MOV EDX, [EAX+4]      ; vtable[1] fn ptr
//   ff d2           CALL EDX              ; call vtable[1](ECX=node)
//   85 c0           TEST EAX, EAX         ; test_eax:
//   89 46 0c        MOV [ESI+0Ch], EAX    ; field_0C = result
//   74 05           JZ  +5  → ret_null
//   8d 46 08        LEA EAX, [ESI+8]      ; return this+8
//   5e              POP ESI
//   c3              RET
//   33 c0           XOR EAX, EAX          ; ret_null: explicit zero
//   5e              POP ESI
//   c3              RET

// INodeObj: object stored in the node chain; vtable[1] advances to next.
struct INodeObj {
    virtual INodeObj *vf0() = 0;
    virtual INodeObj *next() = 0;
};

// CHandle: iterator/handle into the engine_memory linked structure.
//   field_4  — raw base address; sentinel = field_4 + 0x20
//   field_8  — handle value returned on success (as this+8)
//   field_0C — pointer to the current INodeObj node (null = exhausted)
struct CHandle {
    int       field_0;
    int       field_4;
    int       field_8;
    INodeObj *field_0C;

    INodeObj *FUN_00412630();
};

INodeObj *CHandle::FUN_00412630() {
    int curr = (int)field_0C;
    INodeObj *result = 0;
    int iVar2;
    if (curr)
        iVar2 = curr + 8;
    else
        iVar2 = 0;
    INodeObj *node = *(INodeObj **)(iVar2 + 8);
    INodeObj *sentinel = (INodeObj *)(field_4 + 0x20);
    if (node != sentinel)
        result = node->next();
    field_0C = result;
    if (result)
        return (INodeObj *)((char *)this + 8);
    return 0;
}
