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
// FUNCTION: ffxivgame 0x0000ead0 — __thiscall sentinel empty-check
//
// Asm (13 bytes):
//   8d 41 04     LEA EAX,[ECX + 0x4]     ; EAX = this + 4 (address of sentinel field)
//   33 c9        XOR ECX,ECX             ; zero ECX (this no longer needed; SETNZ scratch)
//   3b 00        CMP EAX,dword ptr [EAX] ; compare &field_0x4 vs field_0x4
//   0f 95 c1     SETNZ CL                ; CL = (not equal)
//   8a c1        MOV AL,CL              ; return bool in AL
//   c3           RET                     ; __thiscall, no stack args
//
// Classic intrusive linked-list sentinel check: the sentinel's next pointer
// at offset 4 points to itself when the list is empty. Returns true if the
// pointer does NOT point to itself (i.e., the list is non-empty).

struct FUN_0040ead0_s {
    void *m_dummy;  // offset 0x0
    void *m_head;   // offset 0x4 — self-referential when empty
    bool FUN_0040ead0();
};

bool FUN_0040ead0_s::FUN_0040ead0() {
    void **p = &m_head;
    return (void *)p != *p;
}
