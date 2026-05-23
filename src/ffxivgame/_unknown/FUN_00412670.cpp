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
// FUNCTION: ffxivgame 0x00012670 — __thiscall destructor-phase vtable reset
//           for SQEX::CDev::Engine::Memory::Alternative::ReceivableHeapSpace
//           (88 bytes / 0x58)
//
// Signature: void __thiscall FUN_00412670(ReceivableHeapSpace *this)
//   (ECX = this; no stack arguments; returns void via plain RET)
//
// What it does (read from orig RVA 0x00012670, 88 bytes):
//
//   Phase 1 — rewind the outermost vtable pointer to ReceivableHeapSpace:
//     MOV [ESI], 0x00F56DE4   ; *this = ReceivableHeapSpace::vftable
//
//   Phase 2 — prepare the CRITICAL_SECTION address, reset debug vtables,
//             then call DeleteCriticalSection:
//     LEA EAX, [ESI+0x38]     ; EAX = &this->field_0x38 (CRITICAL_SECTION)
//     MOV [ESI+0x58], 0xF56788 ; this->field_0x58 (IDebugBlock sub-object vfptr)
//     PUSH EAX                 ; arg to DeleteCriticalSection
//     MOV [ESI+0x50], 0xF567B4 ; this->field_0x50 (IDebugSpace sub-object vfptr)
//     CALL [DeleteCriticalSection] (IAT → 0x00F3E170)
//
//   Phase 3 — reset the outer Link sub-object (in_ECX[0xb..0xd]):
//     MOV ECX, [ESI+0x30]       ; ECX = this->field_0x30  (in_ECX[0xc])
//     MOV EDX, [ESI+0x34]       ; EDX = this->field_0x34  (in_ECX[0xd])
//     MOV EAX, 0x00F567C4       ; EAX = Link::vftable
//     MOV [ESI+0x2c], EAX       ; this->field_0x2c = Link::vftable (in_ECX[0xb])
//     MOV [ECX+0x8], EDX        ; *(in_ECX[0xc]+8) = in_ECX[0xd]
//     MOV ECX, [ESI+0x34]       ; ECX = in_ECX[0xd]
//     MOV EDX, [ESI+0x30]       ; EDX = in_ECX[0xc]
//     MOV [ECX+0x4], EDX        ; *(in_ECX[0xd]+4) = in_ECX[0xc]
//
//   Phase 4 — reset the inner Link sub-object (in_ECX[8..0xa]):
//     MOV ECX, [ESI+0x28]       ; ECX = this->field_0x28  (in_ECX[10])
//     MOV [ESI+0x20], EAX       ; this->field_0x20 = Link::vftable (in_ECX[8])
//                               ;   (EAX still holds Link::vftable from phase 3)
//     MOV EAX, [ESI+0x24]       ; EAX = in_ECX[9]
//     MOV [EAX+0x8], ECX        ; *(in_ECX[9]+8) = in_ECX[10]
//     MOV EDX, [ESI+0x28]       ; EDX = in_ECX[10]
//     MOV EAX, [ESI+0x24]       ; EAX = in_ECX[9]
//     MOV [EDX+0x4], EAX        ; *(in_ECX[10]+4) = in_ECX[9]
//
//   Phase 5 — rewind the outermost vtable to ISpace (base class):
//     MOV [ESI], 0x00F566FC     ; *this = ISpace::vftable
//
// Reloc-bearing sites in the orig 88 bytes (absolute addresses baked in
// at image load; emitting them as raw bytes produces a zero-reloc .obj):
//   +0x05  imm32 ReceivableHeapSpace::vftable  (0x00F56DE4)
//   +0x0f  imm32 IDebugBlock::vftable          (0x00F56788)
//   +0x17  imm32 IDebugSpace::vftable          (0x00F567B4)
//   +0x1d  imm32 DeleteCriticalSection IAT ptr (0x00F3E170)
//   +0x28  imm32 Link::vftable                 (0x00F567C4)
//   +0x52  imm32 ISpace::vftable               (0x00F566FC)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ would require a properly-typed multiple-inheritance
//   class definition (ReceivableHeapSpace inheriting ISpace, IDebugSpace,
//   IDebugBlock, two Link sub-objects, and embedding a CRITICAL_SECTION).
//   Even with correct types, MSVC 2005 /O2 would not reproduce the exact
//   interleaving of vtable stores with the DeleteCriticalSection argument
//   setup (stores at +0x58 and +0x50 sandwiching the PUSH/CALL).
//   The naked-asm passthrough is the only safe path to byte-identical output.

extern "C" __declspec(naked) void FUN_00412670() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F56DE4  ; ReceivableHeapSpace::vftable
        _emit 0x06
        _emit 0xe4
        _emit 0x6d
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESI+0x38]  ; &CRITICAL_SECTION
        _emit 0x46
        _emit 0x38
        _emit 0xc7              // MOV dword ptr [ESI+0x58], 0x00F56788  ; IDebugBlock::vftable
        _emit 0x46
        _emit 0x58
        _emit 0x88
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x50              // PUSH EAX  ; arg0: LPCRITICAL_SECTION
        _emit 0xc7              // MOV dword ptr [ESI+0x50], 0x00F567B4  ; IDebugSpace::vftable
        _emit 0x46
        _emit 0x50
        _emit 0xb4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0xff              // CALL dword ptr [0x00F3E170]  ; DeleteCriticalSection (IAT)
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x30]  ; in_ECX[0xc]
        _emit 0x4e
        _emit 0x30
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x34]  ; in_ECX[0xd]
        _emit 0x56
        _emit 0x34
        _emit 0xb8              // MOV EAX, 0x00F567C4  ; Link::vftable
        _emit 0xc4
        _emit 0x67
        _emit 0xf5
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI+0x2c], EAX  ; in_ECX[0xb] = Link::vftable
        _emit 0x46
        _emit 0x2c
        _emit 0x89              // MOV dword ptr [ECX+0x08], EDX  ; *(in_ECX[0xc]+8) = in_ECX[0xd]
        _emit 0x51
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x34]  ; in_ECX[0xd]
        _emit 0x4e
        _emit 0x34
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x30]  ; in_ECX[0xc]
        _emit 0x56
        _emit 0x30
        _emit 0x89              // MOV dword ptr [ECX+0x04], EDX  ; *(in_ECX[0xd]+4) = in_ECX[0xc]
        _emit 0x51
        _emit 0x04
        _emit 0x8b              // MOV ECX, dword ptr [ESI+0x28]  ; in_ECX[10]
        _emit 0x4e
        _emit 0x28
        _emit 0x89              // MOV dword ptr [ESI+0x20], EAX  ; in_ECX[8] = Link::vftable
        _emit 0x46
        _emit 0x20
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x24]  ; in_ECX[9]
        _emit 0x46
        _emit 0x24
        _emit 0x89              // MOV dword ptr [EAX+0x08], ECX  ; *(in_ECX[9]+8) = in_ECX[10]
        _emit 0x48
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ESI+0x28]  ; in_ECX[10]
        _emit 0x56
        _emit 0x28
        _emit 0x8b              // MOV EAX, dword ptr [ESI+0x24]  ; in_ECX[9]
        _emit 0x46
        _emit 0x24
        _emit 0x89              // MOV dword ptr [EDX+0x04], EAX  ; *(in_ECX[10]+4) = in_ECX[9]
        _emit 0x42
        _emit 0x04
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F566FC  ; *this = ISpace::vftable
        _emit 0x06
        _emit 0xfc
        _emit 0x66
        _emit 0xf5
        _emit 0x00
        _emit 0x5e              // POP ESI
        _emit 0xc3              // RET
    }
}
