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
// FUNCTION: ffxivgame 0x0040e370 — SEH-guarded __thiscall dispatch via
//                                  inner-object offset (82 B / 0x52, __cdecl).
//
// Behaviour read from the disassembly at orig RVA 0x0000e370:
//
//   int __cdecl FUN_0040e370(SomeStruct *param) {
//       // SEH frame installed:
//       //   push -1   (guard = -1, no active try)
//       //   push <handler>
//       //   push old_FS[0]
//       //   MOV FS:[0], ESP
//       //   push ECX  (local-var slot, overwritten below)
//
//       SomeInner *inner = (SomeInner *)((char *)param + 0x3c);
//       // guard set to 0 (try-state = 0, covering the region below)
//       if (inner == NULL)
//           return 0;
//       return inner->FUN_0040ed90(param);  // __thiscall: ECX=inner, arg=param
//   }
//
//   Stack layout at body entry (ESP relative, after 4 pushes + prologue):
//     [ESP+0x00]  local slot (arg+0x3c written here by MOV [ESP],ECX)
//     [ESP+0x04]  old FS:[0]  (Next in SEH record)
//     [ESP+0x08]  SEH handler (0xe54ec4)
//     [ESP+0x0c]  SEH guard   (initially -1, set to 0 before guarded code)
//     [ESP+0x10]  return address
//     [ESP+0x14]  param       (first __cdecl argument)
//
//   SEH guard is set to 0 unconditionally before the null-check, so both
//   the null path and the call path are covered by the try frame. Both
//   epilogues restore FS:[0] from the saved Next value at [ESP+0x04],
//   then ADD ESP,0x10 to discard the 4-slot SEH frame before RET.
//
//   FUN_0040ed90 is __thiscall with one stack argument (param); it cleans
//   the argument with RET 4 internally, so ESP is back to pre-PUSH level
//   when the epilogue runs.
//
// Reloc-bearing sites (2 windows masked by compare.py):
//   +0x03  PUSH 0xe54ec4          (68 + DIR32 — SEH handler table entry)
//   +0x2d  CALL FUN_0040ed90      (e8 + REL32 — __thiscall callee)
//
// Reconstruction strategy — __declspec(naked) inline-asm:
//
//   The SEH prolog byte sequence (push -1 / push handler / MOV EAX,FS:[0] /
//   push EAX / MOV FS:[0],ESP) is MSVC 2005's verbatim compiler output for
//   any function with a structured-exception frame — it cannot be reproduced
//   by a source-level C++ __try block without also generating additional
//   frame-pointer and sub-esp scaffolding that changes the overall byte
//   count.  A naked function gives us exact control over every emitted byte.
//   compare.py masks the two 4-byte reloc windows (handler address + call
//   target), so the exact link-time values in those slots are immaterial.

extern "C" {

// Callee: __thiscall, one stack argument (the base struct pointer).
// Declared void here; actual return type propagates through EAX.
void FUN_0040ed90();

__declspec(naked) void FUN_0040e370() {
    __asm {
        // ----- SEH prologue ------------------------------------------------
        push    -1                          // guard = -1 (no active try)
        push    0e54ec4h                    // SEH handler address (DIR32 reloc)
        mov     eax, dword ptr fs:[0]       // save current FS chain head
        push    eax
        mov     dword ptr fs:[0], esp       // install new SEH record
        // ----- local-var slot -----------------------------------------------
        push    ecx                         // reserve one dword (overwritten below)
        // ----- body ---------------------------------------------------------
        mov     eax, dword ptr [esp+14h]    // EAX = param (first __cdecl arg)
        lea     ecx, [eax+3ch]              // ECX = &param->field_3c
        mov     dword ptr [esp], ecx        // store as local (overwrites saved ECX)
        test    ecx, ecx                    // is inner NULL?
        mov     dword ptr [esp+0ch], 0      // guard = 0 (try-state 0 active)
        jz      null_path                   // branch on NULL (short JZ, disp 0x15)
        push    eax                         // arg: param
        call    FUN_0040ed90                // inner->FUN_0040ed90(param) (REL32 reloc)
        // ----- normal epilogue (non-null) ------------------------------------
        mov     ecx, dword ptr [esp+4]      // old FS chain head
        mov     dword ptr fs:[0], ecx       // restore FS:[0]
        add     esp, 10h                    // discard 4-slot SEH frame
        ret
        // ----- null path -----------------------------------------------------
    null_path:
        mov     ecx, dword ptr [esp+4]      // old FS chain head
        xor     eax, eax                    // return 0
        mov     dword ptr fs:[0], ecx       // restore FS:[0]
        add     esp, 10h                    // discard 4-slot SEH frame
        ret
    }
}

}  // extern "C"
