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
// FUNCTION: ffxivgame 0x0043b6e0 — __thiscall teardown helper: free an owned
//                                  sub-object, then DeleteCriticalSection on
//                                  an embedded CRITICAL_SECTION (54 B / 0x36
//                                  compare window; real function is 57 B).
//
// Calling convention: __thiscall (ECX = this on entry; no stack args; plain
// RET — the caller-frame-only PUSH ESI/EDI callee-saves match the sibling
// FUN_0043c080).
//
// Source shape (matches the FUN_0043c080 / FUN_0043b940 family exactly):
//
//   void SomeClass::Method() {
//       if (this->field_4 != NULL) {
//           this->field_7d = 1;
//           void *p = this->field_4;
//           if (p != NULL) {
//               FUN_009fc830(p);   // __thiscall sub-object destructor (ECX=p)
//               FUN_009d1b17(p);   // __cdecl free / operator delete
//           }
//           this->field_4 = NULL;
//           DeleteCriticalSection((LPCRITICAL_SECTION)((char *)this + 0x64));
//       }
//   }
//
// Asm (57 bytes total, orig RVA 0x3b6e0):
//   56                        PUSH ESI
//   8b f1                     MOV ESI, ECX
//   83 7e 04 00               CMP dword ptr [ESI+4], 0
//   74 2e                     JZ tail                     ; → offset 0x37 = POP ESI
//   57                        PUSH EDI
//   c6 46 7d 01               MOV byte ptr [ESI+0x7d], 1
//   8b 7e 04                  MOV EDI, dword ptr [ESI+4]
//   85 ff                     TEST EDI, EDI
//   74 10                     JZ skip_free                ; → offset 0x25
//   8b cf                     MOV ECX, EDI
//   e8 RR RR RR RR            CALL FUN_009fc830
//   57                        PUSH EDI
//   e8 RR RR RR RR            CALL FUN_009d1b17           ; _free
//   83 c4 04                  ADD ESP, 4                  ; (hidden by the asm
//                                                          ;  dumper — confirmed
//                                                          ;  by the JZ skip_free
//                                                          ;  span: 2+5+1+5+3=0x10)
//  skip_free:
//   c7 46 04 00 00 00 00      MOV dword ptr [ESI+4], 0
//   83 c6 64                  ADD ESI, 0x64
//   56                        PUSH ESI
//   ff 15 70 e1 f3 00         CALL dword ptr [0x00f3e170] ; DeleteCriticalSection
//   5f                        POP EDI
//  tail:
//   5e                        POP ESI
//   c3                        RET
//
// The YAML/Ghidra-reported size (0x36 = 54) and end RVA (0x3b716) place the
// compare window from the prologue through the CALL [DeleteCriticalSection]
// (inclusive), stopping right before the trailing POP EDI / POP ESI / RET —
// same truncated-window shape documented in FUN_0043b940 and FUN_0043b4d0.
// We still emit the real trailing epilogue below for a well-formed function;
// those extra bytes fall outside what tools/compare.py checks.
//
// Reloc-bearing sites (masked by tools/compare.py):
//   CALL rel32 → FUN_009fc830
//   CALL rel32 → FUN_009d1b17 (_free)
//   DIR32 (CALL [imm32])      → IAT slot 0x00f3e170 (DeleteCriticalSection)
//
// Naked __asm so the CMP-without-register-load / reload-into-EDI redundant
// double-test, the short-form JZ branches, and the ADD ESI,0x64 (rather than
// a LEA into a fresh register) all pin to the orig encoding. The IAT call is
// emitted as raw bytes (mirrors FUN_00458920) since it's a bare absolute
// indirect call, not a register-mediated one.

extern "C" {
int FUN_009fc830();
int FUN_009d1b17();
} // extern "C"

extern "C" __declspec(naked) void FUN_0043b6e0()
{
    __asm {
        push    esi
        mov     esi, ecx
        cmp     dword ptr [esi + 0x4], 0
        jz      tail
        push    edi
        mov     byte ptr [esi + 0x7d], 1
        mov     edi, dword ptr [esi + 0x4]
        test    edi, edi
        jz      skip_free
        mov     ecx, edi
        call    FUN_009fc830
        push    edi
        call    FUN_009d1b17
        add     esp, 4
    skip_free:
        mov     dword ptr [esi + 0x4], 0
        add     esi, 0x64
        push    esi
        // ff 15 70 e1 f3 00   CALL dword ptr [0x00f3e170]  (DeleteCriticalSection)
        _emit 0xff
        _emit 0x15
        _emit 0x70
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        pop     edi
    tail:
        pop     esi
        ret
    }
}
