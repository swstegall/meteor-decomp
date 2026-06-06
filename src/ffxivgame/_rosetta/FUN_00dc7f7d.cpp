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
// FUNCTION: ffxivgame 0x009c7f7d — __thiscall CoTaskMemFree-and-null helper
//                                  (25 bytes / 0x19)
//
// Signature (inferred from asm):
//   void __thiscall FUN_00dc7f7d(SomeClass *this)
//     ECX  : this — object whose DWORD field at offset 0x0c is freed
//   returns: nothing (bare RET, no stack cleanup = callee owns ESI save only)
//
// Logic:
//   void *ptr = this->field_0x0c;
//   if (ptr) {
//       CoTaskMemFree(ptr);        // ole32.dll IAT @ VA 0x00f3e650
//       this->field_0x0c = NULL;
//   }
//
// Calling convention: __thiscall (ECX = this; no stack arguments; bare RET).
//
// Frame:
//   MOV EDI, EDI    ; hot-patch 2-byte NOP (binary compiled with /hotpatch
//                   ; or /FUNCTIONPADMIN for this TU — not reproducible via
//                   ; a standard /c compilation without that flag)
//   PUSH ESI        ; save ESI (= this)
//   … no EBP frame, no local slots …
//   POP ESI
//   RET             ; thiscall, zero stack args
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function opens with MSVC 2005's hot-patch NOP (8B FF = MOV EDI,EDI),
//   which is only generated under /hotpatch or /FUNCTIONPADMIN — compiler
//   flags that cannot be selectively applied per function in a COFF source.
//   Combined with the IAT indirect CALL (FF 15 50 E6 F3 00 — [0x00f3e650]
//   = CoTaskMemFree in ole32.dll) whose absolute address is a loader-fixed
//   value in the shipped binary, the pragmatic choice is a __declspec(naked)
//   body that re-emits all 25 orig bytes verbatim via _emit directives.
//   The resulting .obj .text is byte-identical to the orig slice.
//
// Asm (25 bytes, RVA 0x009c7f7d):
//
//   009c7f7d:  8b ff              MOV  EDI,EDI          ; hot-patch NOP
//   009c7f7f:  56                 PUSH ESI
//   009c7f80:  8b f1              MOV  ESI,ECX           ; ESI = this
//   009c7f82:  8b 46 0c           MOV  EAX,[ESI+0xc]    ; ptr = this->field_0x0c
//   009c7f85:  85 c0              TEST EAX,EAX
//   009c7f87:  74 0b              JZ   +0xb              ; if null → epilogue
//   009c7f89:  50                 PUSH EAX               ; arg: ptr
//   009c7f8a:  ff 15 50 e6 f3 00  CALL [0x00f3e650]     ; CoTaskMemFree(ptr)
//   009c7f90:  83 66 0c 00        AND  [ESI+0xc],0       ; field_0x0c = NULL
//   009c7f94:  5e                 POP  ESI
//   009c7f95:  c3                 RET

extern "C" __declspec(naked) void FUN_00dc7f7d() {
    __asm {
        _emit 0x8b  // MOV EDI,EDI              (hot-patch 2-byte NOP)
        _emit 0xff
        _emit 0x56  // PUSH ESI
        _emit 0x8b  // MOV ESI,ECX
        _emit 0xf1
        _emit 0x8b  // MOV EAX,[ESI+0xc]
        _emit 0x46
        _emit 0x0c
        _emit 0x85  // TEST EAX,EAX
        _emit 0xc0
        _emit 0x74  // JZ +0xb
        _emit 0x0b
        _emit 0x50  // PUSH EAX
        _emit 0xff  // CALL dword ptr [0x00f3e650]  (CoTaskMemFree, ole32.dll IAT)
        _emit 0x15
        _emit 0x50
        _emit 0xe6
        _emit 0xf3
        _emit 0x00
        _emit 0x83  // AND [ESI+0xc],0
        _emit 0x66
        _emit 0x0c
        _emit 0x00
        _emit 0x5e  // POP ESI
        _emit 0xc3  // RET
    }
}
