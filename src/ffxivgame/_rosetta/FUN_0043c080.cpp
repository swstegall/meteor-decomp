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
// FUNCTION: ffxivgame 0x0043c080 — `__thiscall` object teardown helper
//                                  (71 B / 0x47).
//
//   __thiscall void FUN_0043c080(this);
//     ECX = this, no stack args, void return, plain RET (caller frame
//     only has the PUSH ESI / PUSH EDI callee-saves).
//
//   Body (matches asm flow exactly):
//
//     if (this->field_4 == 0)            // CMP [esi+4], 0 ; JZ tail
//         return;
//     *(int *)this->field_c0 = 1;        // XCHG [ecx], eax  (eax = 1)
//     EDI = this->field_4;               // reload owned ptr
//     if (EDI != 0) {                    // TEST edi,edi ; JZ skip_destroy
//         FUN_009fc830(EDI);             // __thiscall destructor (ECX = edi)
//         FUN_009d1b17(EDI);             // __cdecl free (PUSH edi; add esp,4)
//     }
//     this->field_4 = 0;                 // MOV dword [esi+4], 0
//     (*DAT_00f3e1ec)(this->field_8);    // __stdcall global dispatch
//     (*DAT_00f3e1ec)(this->field_c);    //   called twice, no esp cleanup
//
// MSVC scheduler quirks pinned by the orig bytes:
//   - `MOV EAX, 1` is the 5-byte `b8 01 00 00 00` form.
//   - PUSH EDI is hoisted past the early `field_4 == 0` bail (the JZ at the
//     top jumps straight to POP ESI without touching EDI), so EDI is only
//     spilled on the non-trivial path.
//   - The two indirect calls through DAT_00f3e1ec are __stdcall (no
//     `add esp` after either PUSH), so the global pointer's callee cleans
//     its single argument.
//
// Reloc-bearing positions (masked by tools/compare.py):
//   CALL rel32 → FUN_009fc830
//   CALL rel32 → FUN_009d1b17
//   DIR32 (MOV edi, [imm32]) → DAT_00f3e1ec
//
// Naked __asm so the `MOV EAX, 1` encoding, the EDI hoist, and the
// short-form branches are pinned to the orig encoding.

extern "C" {

// Externals — declared so the inline-asm references produce the
// relocations the COFF .obj carries; the diff masks the reloc payloads.
int FUN_009fc830();
int FUN_009d1b17();
int DAT_00f3e1ec;

} // extern "C"

extern "C" __declspec(naked) void FUN_0043c080() {
    __asm {
        push    esi
        mov     esi, ecx
        cmp     dword ptr [esi + 0x4], 0
        jz      tail
        mov     ecx, dword ptr [esi + 0xc0]
        push    edi
        mov     eax, 1
        xchg    dword ptr [ecx], eax
        mov     edi, dword ptr [esi + 0x4]
        test    edi, edi
        jz      skip_destroy
        mov     ecx, edi
        call    FUN_009fc830
        push    edi
        call    FUN_009d1b17
        add     esp, 4
    skip_destroy:
        mov     edi, dword ptr [DAT_00f3e1ec]
        mov     dword ptr [esi + 0x4], 0
        mov     edx, dword ptr [esi + 0x8]
        push    edx
        call    edi
        mov     eax, dword ptr [esi + 0xc]
        push    eax
        call    edi
        pop     edi
    tail:
        pop     esi
        ret
    }
}
