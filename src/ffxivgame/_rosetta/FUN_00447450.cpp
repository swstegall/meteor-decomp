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
// FUNCTION: ffxivgame 0x00447450 — __thiscall copy-assign helper for a
//                                  growable-buffer object (60 B / 0x3c).
//
// __thiscall (returns this*) FUN_00447450(this, SomeClass* other):
//   ECX = this; one 4-byte stack arg (other); callee cleans 4 bytes (`ret 4`).
//
// Object layout (offsets touched, consistent with sibling FUN_00447010):
//   [this + 0x00]  void*         data;      // backing storage pointer
//   [this + 0x08]  unsigned int  size;      // current byte count
//   [this + 0x0c]  int           field_0c;  // unknown field (copied verbatim)
//   [this + 0x10]  char          flagA;     // flag (cleared to 0 by Resize)
//
// Behaviour (recovered from asm @ 0x00447450):
//
//   SomeClass* method(SomeClass* other) {
//       if (other != this) {
//           // Resize this buffer to hold other's data, clearing flagA.
//           this->Resize(other->size, 1);             // FUN_00447010
//           // Copy bytes: memcpy(this->data, other->data, this->size).
//           FUN_009d4600(this->data, other->data, this->size);
//           // Copy remaining fields.
//           this->flagA    = other->flagA;             // byte at +0x10
//           this->field_0c = other->field_0c;          // dword at +0x0c
//       }
//       return this;
//   }
//
// CALL targets (both REL32 — wildcarded by tools/compare.py):
//   +0x12  CALL FUN_00447010  — __thiscall Resize(newSize, clearFlag), ret 8
//   +0x21  CALL FUN_009d4600  — CRT memcpy(__cdecl, 3 args); caller cleans 0xc
//
// Reconstruction strategy — `__declspec(naked)` asm:
//   The function is short (60 bytes) with a fixed register layout
//   (ESI = this, EDI = other, ECX held live through the first CALL).
//   Naked asm reproduces the original instruction sequence exactly;
//   the two REL32 call-offset words are masked by compare.py.
//
// Asm (60 bytes):
//   56                       PUSH ESI
//   57                       PUSH EDI
//   8b 7c 24 0c              MOV  EDI, [ESP+0xc]           ; other
//   8b f1                    MOV  ESI, ECX                  ; this
//   3b fe                    CMP  EDI, ESI
//   74 29                    JZ   done                      ; this == other → skip
//   8b 47 08                 MOV  EAX, [EDI+0x8]            ; other->size
//   6a 01                    PUSH 0x1                        ; clearFlag
//   50                       PUSH EAX                        ; newSize
//   e8 ?? ?? ?? ??           CALL FUN_00447010               ; this->Resize(…)
//   8b 4e 08                 MOV  ECX, [ESI+0x8]            ; this->size (post-resize)
//   8b 17                    MOV  EDX, [EDI]                 ; other->data
//   8b 06                    MOV  EAX, [ESI]                 ; this->data
//   51                       PUSH ECX                        ; n
//   52                       PUSH EDX                        ; src
//   50                       PUSH EAX                        ; dst
//   e8 ?? ?? ?? ??           CALL FUN_009d4600               ; memcpy(…)
//   8a 4f 10                 MOV  CL, [EDI+0x10]            ; other->flagA
//   88 4e 10                 MOV  [ESI+0x10], CL             ; this->flagA = …
//   8b 57 0c                 MOV  EDX, [EDI+0xc]            ; other->field_0c
//   83 c4 0c                 ADD  ESP, 0xc                   ; caller-cleanup memcpy args
//   89 56 0c                 MOV  [ESI+0xc], EDX             ; this->field_0c = …
// done:
//   5f                       POP  EDI
//   8b c6                    MOV  EAX, ESI                   ; return this
//   5e                       POP  ESI
//   c2 04 00                 RET  0x4

extern "C" void FUN_00447010();   // __thiscall Resize(this, newSize, clearFlag) — ret 8
extern "C" void FUN_009d4600();   // CRT memcpy(dst, src, n) — __cdecl, ret 0

extern "C" __declspec(naked) void FUN_00447450() {
    __asm {
        push    esi
        push    edi
        mov     edi, dword ptr [esp + 0x0c]
        mov     esi, ecx
        cmp     edi, esi
        jz      done
        mov     eax, dword ptr [edi + 0x08]
        push    1
        push    eax
        call    FUN_00447010
        mov     ecx, dword ptr [esi + 0x08]
        mov     edx, dword ptr [edi]
        mov     eax, dword ptr [esi]
        push    ecx
        push    edx
        push    eax
        call    FUN_009d4600
        mov     cl, byte ptr [edi + 0x10]
        mov     byte ptr [esi + 0x10], cl
        mov     edx, dword ptr [edi + 0x0c]
        add     esp, 0x0c
        mov     dword ptr [esi + 0x0c], edx
    done:
        pop     edi
        mov     eax, esi
        pop     esi
        ret     4
    }
}
