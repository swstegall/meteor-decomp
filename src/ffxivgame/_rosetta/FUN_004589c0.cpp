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
// FUNCTION: ffxivgame 0x000589c0 — __thiscall "clear pointer" helper (26 B)
//
// If the first DWORD of the object (this->m_pData) is non-null, passes it
// to FUN_0045b8b0 (a __cdecl wrapper) then nulls the field. Otherwise
// returns immediately.
//
// Asm (26 bytes @ 0x000589c0):
//   56                         PUSH ESI
//   8b f1                      MOV  ESI, ECX            ; ESI = this
//   8b 06                      MOV  EAX, [ESI]          ; EAX = this->m_pData
//   85 c0                      TEST EAX, EAX
//   74 0f                      JZ   +0xf (→ 0x4589d8)   ; if null, skip
//   50                         PUSH EAX                 ; arg to FUN_0045b8b0
//   e8 e1 2e 00 00             CALL FUN_0045b8b0        ; rel32 reloc
//   83 c4 04                   ADD  ESP, 4              ; __cdecl caller cleanup
//   c7 06 00 00 00 00          MOV  dword ptr [ESI], 0  ; m_pData = 0
//   5e                         POP  ESI
//   c3                         RET

class C589C0
{
public:
    void *m_pData;
    void clear();
};

extern "C" void FUN_0045b8b0(void *);

void C589C0::clear()
{
    if (m_pData)
    {
        FUN_0045b8b0(m_pData);
        m_pData = 0;
    }
}
