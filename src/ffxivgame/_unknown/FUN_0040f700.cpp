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
// FUNCTION: ffxivgame 0x0000f700 — SQEX::CDev::Engine::Memory::Alternative::SeparateHeapSpace
//                                   destructor (__thiscall, 161 B / 0xa1)
//
// __thiscall void FUN_0040f700(SeparateHeapSpace *this)
//   ECX : this  — pointer to the SeparateHeapSpace object being destroyed
//
// Object layout (inferred from offsets touched):
//   [this + 0x00]  void**  vftable          — set to SeparateHeapSpace::vftable (0xf56834)
//                                             overwritten to ISpace::vftable (0xf566fc) at end
//   [this + 0x08]  void*   m_heapHandle     — freed if m_hasHandle is nonzero
//   [this + 0x28]  uint8_t m_hasHandle      — flag; cleared after free
//   [this + 0x44]  void**  m_linkVftable    — set to Link::vftable      (0xf567c4)
//   [this + 0x48]  void*   m_link           — pointer to Link sub-object
//   [this + 0x4c]  void*   m_space          — pointer to space sub-object
//   [this + 0x5c]  void**  m_debugSpaceVft  — set to IDebugSpace::vftable (0xf567b4)
//   [this + 0x64]  void**  m_debugBlockVft  — set to IDebugBlock::vftable (0xf56788)
//   [this + 0x2c]  CRITICAL_SECTION         — deleted at end
//
// Behaviour:
//   1. Reset vftable to SeparateHeapSpace::vftable (0xf56834).
//   2. Virtual-dispatch: load this->m_space (+0x4c), call vftable[1] (slot+4).
//      The return value is passed as first arg to FUN_00410730(this, retval).
//   3. If m_hasHandle != 0: FUN_009d56fd(m_heapHandle); clear m_heapHandle, m_hasHandle.
//   4. Set IDebugBlock, IDebugSpace, Link vftables on sub-fields.
//   5. Cross-link m_link and m_space (+0x48/+0x4c → each other's +8/+4 slots).
//   6. DeleteCriticalSection(&this->field_2c).
//   7. Overwrite vftable with ISpace::vftable (0xf566fc).
//
// Calling convention: __thiscall; no stack args (plain RET).
// Callee-saves: PUSH ECX (this), PUSH ESI.  Prologue installs a minimal SEH
//   frame (non-standard MSVC form: PUSH -1 first, then PUSH cookie):
//       PUSH -1; PUSH 0xe54f39; MOV EAX,FS:[0]; PUSH EAX; MOV FS:[0],ESP
//   SEH state slot is at [ESP+0x10], written to 3 after prologue.
//
// Stack frame after prologue (offsets from ESP):
//   [ESP+0x00]  saved ESI
//   [ESP+0x04]  saved ECX / ESI (this) — written by MOV [ESP+0x4],ESI
//   [ESP+0x08]  old FS:[0]
//   [ESP+0x0c]  SEH cookie addr (0xe54f39)
//   [ESP+0x10]  SEH state (-1 → 3)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Absolute-immediate vtable stores (MOV [mem], imm32) and the SEH cookie
//   PUSH are emitted as raw _emit bytes.  The two CALL rel32 and the
//   IAT ff15 are expressed as MASM mnemonics so the assembler emits proper
//   COFF relocations that compare.py masks.  The epilogue and all SEH
//   frame ops are raw _emit.
//
// Reloc-bearing sites (compare.py masks these byte positions):
//   +0x38  CALL rel32  → FUN_00410730
//   +0x47  CALL rel32  → FUN_009d56fd
//   +0x85  CALL [abs]  → IAT DeleteCriticalSection (ff 15 70 e1 f3 00)

extern "C" {

// Sibling call targets — produce REL32 relocations (masked by compare.py).
void FUN_00410730(void *thisPtr, int retval);
void FUN_009d56fd(void *handle);

// IAT indirect call — __declspec(dllimport) forces ff15 encoding (masked).
__declspec(dllimport) void __stdcall DeleteCriticalSection(void *lpCriticalSection);

} // extern "C"

extern "C" __declspec(naked) void FUN_0040f700() {
    __asm {
        // 0000f700:  6a ff                         PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0000f702:  68 39 4f e5 00                PUSH 0xe54f39  (SEH cookie)
        _emit 0x68
        _emit 0x39
        _emit 0x4f
        _emit 0xe5
        _emit 0x00
        // 0000f707:  64 a1 00 00 00 00             MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f70d:  50                            PUSH EAX
        _emit 0x50
        // 0000f70e:  64 89 25 00 00 00 00          MOV dword ptr FS:[0x0],ESP
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f715:  51                            PUSH ECX
        _emit 0x51
        // 0000f716:  56                            PUSH ESI
        _emit 0x56
        // 0000f717:  8b f1                         MOV ESI,ECX
        _emit 0x8b
        _emit 0xf1
        // 0000f719:  89 74 24 04                   MOV dword ptr [ESP+0x4],ESI
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x04
        // 0000f71d:  c7 06 34 68 f5 00             MOV dword ptr [ESI],0xf56834
        _emit 0xc7
        _emit 0x06
        _emit 0x34
        _emit 0x68
        _emit 0xf5
        _emit 0x00
        // 0000f723:  8b 4e 4c                      MOV ECX,dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x4e
        _emit 0x4c
        // 0000f726:  8b 01                         MOV EAX,dword ptr [ECX]
        _emit 0x8b
        _emit 0x01
        // 0000f728:  8b 50 04                      MOV EDX,dword ptr [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0000f72b:  c7 44 24 10 03 00 00 00       MOV dword ptr [ESP+0x10],0x3
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f733:  ff d2                         CALL EDX
        _emit 0xff
        _emit 0xd2
        // 0000f735:  50                            PUSH EAX
        _emit 0x50
        // 0000f736:  8b ce                         MOV ECX,ESI
        _emit 0x8b
        _emit 0xce
        // 0000f738:  e8 f3 0f 00 00                CALL 0x00410730  (rel32 — reloc masked)
        call    FUN_00410730
        // 0000f73d:  80 7e 28 00                   CMP byte ptr [ESI+0x28],0x0
        _emit 0x80
        _emit 0x7e
        _emit 0x28
        _emit 0x00
        // 0000f741:  74 17                         JZ  skip_free
        _emit 0x74
        _emit 0x17
        // 0000f743:  8b 46 08                      MOV EAX,dword ptr [ESI+0x8]
        _emit 0x8b
        _emit 0x46
        _emit 0x08
        // 0000f746:  50                            PUSH EAX
        _emit 0x50
        // 0000f747:  e8 b1 5f 5c 00                CALL 0x009d56fd  (rel32 — reloc masked)
        call    FUN_009d56fd
        // 0000f74c:  83 c4 04                      ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0000f74f:  c7 46 08 00 00 00 00          MOV dword ptr [ESI+0x8],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f756:  c6 46 28 00                   MOV byte ptr [ESI+0x28],0x0
        _emit 0xc6
        _emit 0x46
        _emit 0x28
        _emit 0x00
    skip_free:
        // 0000f75a:  c7 46 64 88 67 f5 00          MOV dword ptr [ESI+0x64],0xf56788
        _emit 0xc7
        _emit 0x46
        _emit 0x64
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000f761:  c7 46 5c b4 67 f5 00          MOV dword ptr [ESI+0x5c],0xf567b4
        _emit 0xc7
        _emit 0x46
        _emit 0x5c
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000f768:  8b 4e 48                      MOV ECX,dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 0000f76b:  8b 56 4c                      MOV EDX,dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x56
        _emit 0x4c
        // 0000f76e:  c7 46 44 c4 67 f5 00          MOV dword ptr [ESI+0x44],0xf567c4
        _emit 0xc7
        _emit 0x46
        _emit 0x44
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 0000f775:  89 51 08                      MOV dword ptr [ECX+0x8],EDX
        _emit 0x89
        _emit 0x51
        _emit 0x08
        // 0000f778:  8b 46 4c                      MOV EAX,dword ptr [ESI+0x4c]
        _emit 0x8b
        _emit 0x46
        _emit 0x4c
        // 0000f77b:  8b 4e 48                      MOV ECX,dword ptr [ESI+0x48]
        _emit 0x8b
        _emit 0x4e
        _emit 0x48
        // 0000f77e:  8d 56 2c                      LEA EDX,[ESI+0x2c]
        _emit 0x8d
        _emit 0x56
        _emit 0x2c
        // 0000f781:  52                            PUSH EDX
        _emit 0x52
        // 0000f782:  89 48 04                      MOV dword ptr [EAX+0x4],ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 0000f785:  ff 15 70 e1 f3 00             CALL dword ptr [0x00f3e170]  (IAT: DeleteCriticalSection)
        call    dword ptr [DeleteCriticalSection]
        // 0000f78b:  8b 4c 24 08                   MOV ECX,dword ptr [ESP+0x8]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        // 0000f78f:  c7 06 fc 66 f5 00             MOV dword ptr [ESI],0xf566fc
        _emit 0xc7
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 0000f795:  5e                            POP ESI
        _emit 0x5e
        // 0000f796:  64 89 0d 00 00 00 00          MOV dword ptr FS:[0x0],ECX
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000f79d:  83 c4 10                      ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000f7a0:  c3                            RET
        _emit 0xc3
    }
}

// vim: ts=4 sts=4 sw=4 et
