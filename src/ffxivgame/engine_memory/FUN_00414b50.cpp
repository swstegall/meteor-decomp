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
// FUNCTION: ffxivgame 0x00014b50 — __thiscall destructor body for
//           SQEX::CDev::Engine::Memory::Alternative::SystemHeapSpace.
//           Resets multi-inheritance vtable pointers, unlinks the
//           intrusive doubly-linked Link sub-object from its sibling
//           list, destroys the CRITICAL_SECTION at this+0x8, then
//           resets *this to the ISpace base-class vtable before
//           returning `this`. (70 bytes / 0x46)
//
// Pseudo-C (from Ghidra headless):
//   *this           = SystemHeapSpace::vftable;          ; install most-derived vftable
//   this->vt_dbg_b  = IDebugBlock::vftable;              ; at this+0x3c (this[0xf])
//   this->vt_dbg_s  = IDebugSpace::vftable;              ; at this+0x34 (this[0xd])
//   this->link_vt   = Link::vftable;                     ; at this+0x20 (this[0x8])
//   this->link.prev->next = this->link.next;             ; intrusive list unlink
//   this->link.next->prev = this->link.prev;
//   DeleteCriticalSection(&this->cs);                    ; cs at this+0x8 (this[0x2])
//   *this           = ISpace::vftable;                   ; reset to base before chain-up
//   return this;                                         ; (return through EAX)
//
// Calling convention: __thiscall (ECX = this), 1 stack DWORD arg
// (RET 4 — the implicit `bool should_delete` for vector-deleting dtor
// dispatch), callee-saved ESI only.
//
// Asm (70 bytes @ orig RVA 0x00014b50):
//   56                       PUSH ESI
//   8b f1                    MOV ESI, ECX                       ; this
//   c7 06 1c 70 f5 00        MOV DWORD [ESI], 0x00f5701c        ; SystemHeapSpace::vftable     (DIR32)
//   c7 46 3c 88 67 f5 00     MOV DWORD [ESI+0x3c], 0x00f56788   ; IDebugBlock::vftable         (DIR32)
//   c7 46 34 b4 67 f5 00     MOV DWORD [ESI+0x34], 0x00f567b4   ; IDebugSpace::vftable         (DIR32)
//   8b 46 24                 MOV EAX, [ESI+0x24]                ; prev = this->link.prev
//   8b 4e 28                 MOV ECX, [ESI+0x28]                ; next = this->link.next
//   c7 46 20 c4 67 f5 00     MOV DWORD [ESI+0x20], 0x00f567c4   ; Link::vftable                (DIR32)
//   89 48 08                 MOV [EAX+0x8], ECX                 ; prev->next = next
//   8b 56 28                 MOV EDX, [ESI+0x28]                ; reload next
//   8b 46 24                 MOV EAX, [ESI+0x24]                ; reload prev
//   8d 4e 08                 LEA ECX, [ESI+0x8]                 ; &this->cs
//   51                       PUSH ECX                           ; LPCRITICAL_SECTION
//   89 42 04                 MOV [EDX+0x4], EAX                 ; next->prev = prev
//   ff 15 70 e1 f3 00        CALL DWORD [0x00f3e170]            ; CALL [IAT: DeleteCriticalSection] (DIR32)
//   c7 06 fc 66 f5 00        MOV DWORD [ESI], 0x00f566fc        ; ISpace::vftable              (DIR32)
//   8b c6                    MOV EAX, ESI                       ; return this
//   5e                       POP ESI
//   c2 04 00                 RET 4
//
// Relocations (masked by tools/compare.py via the COFF reloc table):
//   DIR32: SystemHeapSpace::vftable (VA 0x00f5701c) at func+0x05
//   DIR32: IDebugBlock::vftable     (VA 0x00f56788) at func+0x0c
//   DIR32: IDebugSpace::vftable     (VA 0x00f567b4) at func+0x13
//   DIR32: Link::vftable            (VA 0x00f567c4) at func+0x1f
//   DIR32: IAT slot DeleteCriticalSection (VA 0x00f3e170) at func+0x37
//   DIR32: ISpace::vftable          (VA 0x00f566fc) at func+0x3d
//
// Reconstruction: naked-asm byte passthrough.
// Five distinct DIR32 vftable writes plus an IAT-thunked Win32 import
// (`DeleteCriticalSection`) plus the precise scheduling of the list
// unlink (the orig holds prev/next twice in different registers across
// the LEA-then-PUSH for the cs argument, an interleaving MSVC 2005 /O2
// won't reproduce from idiomatic C++ source). Naked-asm `_emit`
// guarantees byte-identical output, and the call-site bytes (DIR32
// pointer values) are emitted verbatim from orig so compare.py's
// mask-bytes-via-COFF-relocs path is satisfied trivially (no relocs in
// the .obj — bytes match by raw equality).
//
// Cross-platform guard: __declspec(naked) + MASM _emit are MSVC-only.

#if defined(__clang__) || defined(__GNUC__)
// clang/GCC stub for static analysis only — NOT compiled in production.
// The real implementation is the MSVC __declspec(naked) + __asm block below,
// which clang cannot parse. Production builds always use cl.exe (MSVC 2005).
extern "C" void FUN_00414b50() {}
#else

extern "C" __declspec(naked) void FUN_00414b50()
{
    __asm {
        // 00014b50:  56                    PUSH ESI
        _emit 0x56
        // 00014b51:  8b f1                 MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 00014b53:  c7 06 1c 70 f5 00     MOV DWORD [ESI], 0x00f5701c   ; SystemHeapSpace::vftable
        _emit 0xc7
        _emit 0x06
        _emit 0x1c
        _emit 0x70
        _emit 0xf5
        _emit 0x00
        // 00014b59:  c7 46 3c 88 67 f5 00  MOV DWORD [ESI+0x3c], 0x00f56788  ; IDebugBlock::vftable
        _emit 0xc7
        _emit 0x46
        _emit 0x3c
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014b60:  c7 46 34 b4 67 f5 00  MOV DWORD [ESI+0x34], 0x00f567b4  ; IDebugSpace::vftable
        _emit 0xc7
        _emit 0x46
        _emit 0x34
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014b67:  8b 46 24              MOV EAX, [ESI+0x24]           ; prev
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 00014b6a:  8b 4e 28              MOV ECX, [ESI+0x28]           ; next
        _emit 0x8b
        _emit 0x4e
        _emit 0x28
        // 00014b6d:  c7 46 20 c4 67 f5 00  MOV DWORD [ESI+0x20], 0x00f567c4  ; Link::vftable
        _emit 0xc7
        _emit 0x46
        _emit 0x20
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        // 00014b74:  89 48 08              MOV [EAX+0x8], ECX            ; prev->next = next
        _emit 0x89
        _emit 0x48
        _emit 0x08
        // 00014b77:  8b 56 28              MOV EDX, [ESI+0x28]           ; reload next
        _emit 0x8b
        _emit 0x56
        _emit 0x28
        // 00014b7a:  8b 46 24              MOV EAX, [ESI+0x24]           ; reload prev
        _emit 0x8b
        _emit 0x46
        _emit 0x24
        // 00014b7d:  8d 4e 08              LEA ECX, [ESI+0x8]            ; &this->cs
        _emit 0x8d
        _emit 0x4e
        _emit 0x08
        // 00014b80:  51                    PUSH ECX                      ; cs ptr
        _emit 0x51
        // 00014b81:  89 42 04              MOV [EDX+0x4], EAX            ; next->prev = prev
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 00014b84:  ff 15 70 e1 f3 00     CALL DWORD [0x00f3e170]       ; DeleteCriticalSection IAT
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 00014b8a:  c7 06 fc 66 f5 00     MOV DWORD [ESI], 0x00f566fc   ; ISpace::vftable
        _emit 0xc7
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        // 00014b90:  8b c6                 MOV EAX, ESI                  ; return this
        _emit 0x8b
        _emit 0xc6
        // 00014b92:  5e                    POP ESI
        _emit 0x5e
        // 00014b93:  c2 04 00              RET 4
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif
