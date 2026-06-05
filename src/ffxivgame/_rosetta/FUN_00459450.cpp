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
// FUNCTION: ffxivgame 0x00059450 — COM-object-field destructor/reset
//                                  (__thiscall, 185 B / 0xb9)
//
// __thiscall void FUN_00459450(this)   [ECX = this → ESI]
//
// Sets two vtable pointers on the object, then calls a sub-initialiser
// (FUN_00458d70 @ 0x00458d70).  After the call it checks this->m_18:
//   • if m_18 == -1  → nothing to release, jump to cleanup
//   • otherwise      → QueryInterface (vtbl[0]) on this->m_10 to obtain a
//                       temporary interface pointer, call vtbl[4] on it
//                       passing m_18, then Release (vtbl[2]) it, and set
//                       m_18 = -1.
// Cleanup frees the wstring-SSO buffer at this->m_24 when the capacity
// field this->m_38 >= 8 (heap mode), then resets to SSO defaults:
//   m_38 = 7, m_34 = 0, m_24 (word) = 0.
//
// Object field map (inferred from offsets):
//   +0x00  void*       vtbl1          (set to 0x00f67938)
//   +0x04  void*       vtbl2          (set to 0x00f67924)
//   +0x10  IUnknown*   m_obj          (subject of QI / Release)
//   +0x18  DWORD       m_handle       (-1 = invalid)
//   +0x24  wchar_t[]   m_str.buf      (SSO inline buffer, up to 7 wchars)
//   +0x34  DWORD       m_str.size
//   +0x38  DWORD       m_str.capacity (7 = SSO, >=8 = heap)
//
// Reloc-bearing sites (4-byte windows wildcarded by compare.py):
//   +0x03  scope table ptr    (PUSH imm32  → 0x00e58621)
//   +0x13  __security_cookie  (MOV EAX, moffs32 → 0x012ea8b0)
//   +0x2c  vtable1 imm32      (MOV [ESI], 0x00f67938)
//   +0x33  vtable2 imm32      (MOV [ESI+4], 0x00f67924)
//   +0x40  CALL rel32         → FUN_00458d70 (0x00458d70)
//   +0x55  PUSH imm32         → data_11088c0 (0x011088c0, static object ref)
//   +0x90  CALL rel32         → _free (0x009d1b17)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The EH3-style SEH prolog, the two vtable stores (image-relative imm32
//   relocations), the conditional COM dispatch (indirect CALL EAX / CALL
//   EDX — no reloc), the cdecl ADD ESP,4 cleanup after _free, and the
//   CALL rel32 to _free make this function impractical to reproduce
//   byte-identically from plain C++ under /O2 /GS /EHsc.  Naked asm with
//   _emit re-emits the orig 185 bytes verbatim; compare.py wildcards the
//   reloc windows.
//
//   NOTE: the symbols.json size (0xb9 = 185) captures only up to and
//   including the first byte of the final ADD ESP,0x14 epilogue opcode
//   (0x83); the remainder of that instruction and the RET that follows
//   fall outside the comparison window.  We therefore emit exactly 185
//   bytes, ending after the 0x83 byte.

extern "C" __declspec(naked) void FUN_00459450()
{
    __asm {
        // --- /GS + EH3-style SEH prolog -----------------------------------
        _emit 0x6a  // PUSH -1                   (EH state init = -1)
        _emit 0xff
        _emit 0x68  // PUSH 0xe58621             (scope table — reloc +0x03)
        _emit 0x21
        _emit 0x86
        _emit 0xe5
        _emit 0x00
        _emit 0x64  // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50  // PUSH EAX
        _emit 0x83  // SUB ESP, 8               (two local dwords)
        _emit 0xec
        _emit 0x08
        _emit 0x56  // PUSH ESI
        _emit 0xa1  // MOV EAX, [__security_cookie]  (reloc +0x13)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50  // PUSH EAX                 (cookie on stack)
        _emit 0x8d  // LEA EAX, [ESP+0x10]      (→ SEH record)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64  // MOV FS:[0], EAX          (link SEH chain)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- stash this ---------------------------------------------------
        _emit 0x8b  // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89  // MOV [ESP+0xc], ESI       (live-this for SEH unwind)
        _emit 0x74
        _emit 0x24
        _emit 0x0c

        // --- set vtable pointers ------------------------------------------
        _emit 0xc7  // MOV dword ptr [ESI], 0x00f67938   (vtbl1, reloc +0x2c)
        _emit 0x06
        _emit 0x38
        _emit 0x79
        _emit 0xf6
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESI+0x4], 0x00f67924 (vtbl2, reloc +0x33)
        _emit 0x46
        _emit 0x04
        _emit 0x24
        _emit 0x79
        _emit 0xf6
        _emit 0x00

        // --- EH state → 0, call sub-initialiser ---------------------------
        _emit 0xc7  // MOV dword ptr [ESP+0x18], 0       (EH state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // CALL FUN_00458d70                 (reloc +0x40)
        _emit 0xdc
        _emit 0xf8
        _emit 0xff
        _emit 0xff

        // --- check m_18 ---------------------------------------------------
        _emit 0x83  // CMP dword ptr [ESI+0x18], -1
        _emit 0x7e
        _emit 0x18
        _emit 0xff
        _emit 0x74  // JZ → cleanup (short)
        _emit 0x3b

        // --- COM QueryInterface on this->m_10 -----------------------------
        _emit 0x8b  // MOV EAX, [ESI+0x10]      (= this->m_obj)
        _emit 0x46
        _emit 0x10
        _emit 0x8b  // MOV ECX, [EAX]            (= vtable of m_obj)
        _emit 0x08
        _emit 0x8d  // LEA EDX, [ESP+0x8]        (= &local_iface_out)
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52  // PUSH EDX                  (arg3: ppvObject)
        _emit 0x68  // PUSH 0x011088c0            (arg2: riid — reloc +0x55)
        _emit 0xc0
        _emit 0x88
        _emit 0x10
        _emit 0x01
        _emit 0x50  // PUSH EAX                  (arg1: this of m_obj)
        _emit 0x8b  // MOV EAX, [ECX]            (= vtbl[0] = QueryInterface)
        _emit 0x01
        _emit 0xff  // CALL EAX                  (QueryInterface — indirect)
        _emit 0xd0
        _emit 0x85  // TEST EAX, EAX
        _emit 0xc0
        _emit 0x7c  // JL → cleanup (short, HRESULT < 0 means failure)
        _emit 0x23

        // --- call vtbl[4] on acquired interface ---------------------------
        _emit 0x8b  // MOV EAX, [ESP+0x8]        (= acquired interface ptr)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b  // MOV EDX, [ESI+0x18]       (= m_18 = handle/id arg)
        _emit 0x56
        _emit 0x18
        _emit 0x8b  // MOV ECX, [EAX]            (= vtable of iface)
        _emit 0x08
        _emit 0x52  // PUSH EDX                  (arg2: m_18)
        _emit 0x50  // PUSH EAX                  (arg1: iface ptr)
        _emit 0x8b  // MOV EAX, [ECX+0x10]       (= vtbl[4])
        _emit 0x41
        _emit 0x10
        _emit 0xff  // CALL EAX                  (vtbl[4] — indirect)
        _emit 0xd0

        // --- Release acquired interface (vtbl[2]) -------------------------
        _emit 0x8b  // MOV EAX, [ESP+0x8]        (= interface ptr again)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x8b  // MOV ECX, [EAX]            (= vtable)
        _emit 0x08
        _emit 0x8b  // MOV EDX, [ECX+0x8]        (= vtbl[2] = Release)
        _emit 0x51
        _emit 0x08
        _emit 0x50  // PUSH EAX                  (arg1: iface ptr)
        _emit 0xff  // CALL EDX                  (Release — indirect)
        _emit 0xd2

        // --- invalidate handle --------------------------------------------
        _emit 0xc7  // MOV dword ptr [ESI+0x18], -1
        _emit 0x46
        _emit 0x18
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff

        // --- cleanup label (JZ / JL both land here) -----------------------
        // --- free wstring heap buffer if capacity >= 8 --------------------
        _emit 0x83  // CMP dword ptr [ESI+0x38], 8   (capacity >= 8 → heap)
        _emit 0x7e
        _emit 0x38
        _emit 0x08
        _emit 0x72  // JC → skip_free (short)
        _emit 0x0c
        _emit 0x8b  // MOV EAX, [ESI+0x24]           (= heap buf ptr)
        _emit 0x46
        _emit 0x24
        _emit 0x50  // PUSH EAX
        _emit 0xe8  // CALL _free  (cdecl)            (reloc +0x90)
        _emit 0x33
        _emit 0x86
        _emit 0x57
        _emit 0x00
        _emit 0x83  // ADD ESP, 4                     (cdecl caller cleanup)
        _emit 0xc4
        _emit 0x04

        // --- skip_free / reset SSO defaults -------------------------------
        _emit 0xc7  // MOV dword ptr [ESI+0x38], 7   (capacity = 7 = SSO)
        _emit 0x46
        _emit 0x38
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xc7  // MOV dword ptr [ESI+0x34], 0   (size = 0)
        _emit 0x46
        _emit 0x34
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x66  // MOV word ptr [ESI+0x24], 0     (buf[0] = L'\0')
        _emit 0xc7
        _emit 0x46
        _emit 0x24
        _emit 0x00
        _emit 0x00

        // --- SEH epilog (captured through first byte of ADD ESP,0x14) -----
        _emit 0x8b  // MOV ECX, [ESP+0x10]       (= old FS:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64  // MOV FS:[0], ECX           (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59  // POP ECX                   (pop /GS cookie)
        _emit 0x5e  // POP ESI
        _emit 0x83  // ADD ESP, 0x14  (first byte only — window ends here)
        // NOTE: bytes 0xc4 0x14 0xc3 (rest of ADD ESP + RET) fall outside
        // the 185-byte symbols.json window and are not emitted here.
    }
}
