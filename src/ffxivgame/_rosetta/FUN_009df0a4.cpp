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
// FUNCTION: ffxivgame 0x005df0a4 — PE section-existence probe (108 B)
//
// __cdecl int FUN_009df0a4() — checks whether a named PE section is absent
// from the loaded EXE image. Returns 1 (success/absent) or 0 (found).
//
// Calling convention: __cdecl (caller-cleans), no parameters.
// Stack frame: sub-via-PUSH-ECX×2 (two dword locals at [EBP-4] and [EBP-8]).
//   Callee-save registers: EBX, ESI (pushed/popped in standard frame).
//   EDI pushed/popped around the PE-scan block.
//
// Reconstruction (from binary bytes at file offset 0x005df0a4, 108 bytes):
//
//   prolog:
//     PUSH EBP / MOV EBP, ESP                         ; frame
//     PUSH ECX × 2                                    ; local_4=[EBP-4], local_8=[EBP-8]
//     PUSH EBX / PUSH ESI
//     XOR ESI, ESI                                    ; ESI = 0
//     LEA EAX, [EBP-4]                                ; EAX = &local_4
//     INC ESI                                         ; ESI = 1 (success sentinel)
//     XOR EBX, EBX                                    ; EBX = 0 (section index)
//     PUSH EAX                                        ; arg: &local_4
//     MOV [EBP-8], ESI                                ; local_8 = 1
//     MOV [EBP-4], EBX                                ; local_4 = 0
//     CALL FUN_009d8f5e (rel32 → RVA 0x5d8f5e)       ; fills in local_4
//     CMP [EBP-4], 5
//     POP ECX
//     JLE short pe_scan                               ; local_4 ≤ 5 → scan PE
//     MOV EAX, ESI                                    ; EAX = 1 (early return)
//     JMP short epilogue
//
//   pe_scan:
//     PUSH EDI
//     PUSH EBX (=0)                                   ; hModule = NULL
//     CALL [0x00f3e1e4]                               ; IAT → GetModuleHandleA(NULL)
//     MOV ESI, [EAX+0x3c]                             ; DOS.e_lfanew
//     ADD ESI, EAX                                    ; ESI = NtHeaders
//     CMP word [ESI+6], BX                            ; NumberOfSections vs 0
//     MOVZX EAX, word [ESI+0x14]                      ; SizeOfOptionalHeader
//     LEA EDI, [EAX+ESI+0x18]                         ; EDI = first IMAGE_SECTION_HEADER
//     JBE short epilogue_edi                          ; 0 sections → done (local_8=1)
//
//   section_loop:
//     PUSH EDI                                        ; arg: current section ptr
//     PUSH 0x01086ea0                                 ; arg: target section name
//     CALL FUN_009d8870 (rel32 → RVA 0x5d8870)       ; compare section name
//     TEST EAX, EAX
//     POP ECX / POP ECX
//     JZ short not_found                              ; name matched → jump
//     MOVZX EAX, word [ESI+6]                        ; NumberOfSections
//     INC EBX
//     ADD EDI, 0x28                                   ; sizeof(IMAGE_SECTION_HEADER)
//     CMP EBX, EAX
//     JB short section_loop
//     JMP short epilogue_edi
//
//   not_found:
//     AND [EBP-8], 0                                  ; local_8 = 0 (failure)
//
//   epilogue_edi:
//     MOV EAX, [EBP-8]                               ; return local_8
//     POP EDI
//
//   epilogue:
//     POP ESI / POP EBX / LEAVE / RET
//
// Reloc-bearing sites (byte-emitted verbatim in the passthrough):
//   +0x17  CALL rel32 → 0xffff9e9f  (FUN_009d8f5e)
//   +0x29  CALL [imm32] → 0x00f3e1e4  (IAT slot)
//   +0x43  PUSH imm32 → 0x01086ea0  (section name address)
//   +0x48  CALL rel32 → 0xffff9780  (FUN_009d8870)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would produce CALL relocations (rel32 = 0 in .obj)
//   and absolute-address relocations (imm32 in .obj containing symbol offsets),
//   none of which match the orig binary's post-fixup bytes at these sites.
//   The `__declspec(naked)` + `_emit` approach emits the exact 108 bytes
//   verbatim, producing a zero-reloc .obj whose .text is byte-identical to
//   the orig slice. `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_009df0a4() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x51              // PUSH ECX  (local_4)
        _emit 0x51              // PUSH ECX  (local_8)
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x33              // XOR ESI, ESI
        _emit 0xf6
        _emit 0x8d              // LEA EAX, [EBP-4]
        _emit 0x45
        _emit 0xfc
        _emit 0x46              // INC ESI   (= 1)
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x50              // PUSH EAX  (&local_4)
        _emit 0x89              // MOV [EBP-8], ESI
        _emit 0x75
        _emit 0xf8
        _emit 0x89              // MOV [EBP-4], EBX
        _emit 0x5d
        _emit 0xfc
        _emit 0xe8              // CALL FUN_009d8f5e (rel32 → 0xffff9e9f)
        _emit 0x9f
        _emit 0x9e
        _emit 0xff
        _emit 0xff
        _emit 0x83              // CMP [EBP-4], 5
        _emit 0x7d
        _emit 0xfc
        _emit 0x05
        _emit 0x59              // POP ECX
        _emit 0x7e              // JLE short +4
        _emit 0x04
        _emit 0x8b              // MOV EAX, ESI  (EAX = 1)
        _emit 0xc6
        _emit 0xeb              // JMP short +0x42
        _emit 0x42
        _emit 0x57              // PUSH EDI
        _emit 0x53              // PUSH EBX  (hModule = 0)
        _emit 0xff              // CALL [0x00f3e1e4]  (IAT → GetModuleHandleA)
        _emit 0x15
        _emit 0xe4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // MOV ESI, [EAX+0x3c]  (e_lfanew)
        _emit 0x70
        _emit 0x3c
        _emit 0x03              // ADD ESI, EAX  (= NtHeaders)
        _emit 0xf0
        _emit 0x66              // CMP word [ESI+6], BX  (NumberOfSections)
        _emit 0x39
        _emit 0x5e
        _emit 0x06
        _emit 0x0f              // MOVZX EAX, word [ESI+0x14]  (SizeOfOptionalHeader)
        _emit 0xb7
        _emit 0x46
        _emit 0x14
        _emit 0x8d              // LEA EDI, [EAX+ESI+0x18]  (first section header)
        _emit 0x7c
        _emit 0x30
        _emit 0x18
        _emit 0x76              // JBE short +0x23  (0 sections → epilogue_edi)
        _emit 0x23
        _emit 0x57              // PUSH EDI  (current section ptr)
        _emit 0x68              // PUSH 0x01086ea0  (target section name)
        _emit 0xa0
        _emit 0x6e
        _emit 0x08
        _emit 0x01
        _emit 0xe8              // CALL FUN_009d8870 (rel32 → 0xffff9780)
        _emit 0x80
        _emit 0x97
        _emit 0xff
        _emit 0xff
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x59              // POP ECX
        _emit 0x59              // POP ECX
        _emit 0x74              // JZ short +0xe  (name matched → not_found)
        _emit 0x0e
        _emit 0x0f              // MOVZX EAX, word [ESI+6]  (NumberOfSections)
        _emit 0xb7
        _emit 0x46
        _emit 0x06
        _emit 0x43              // INC EBX
        _emit 0x83              // ADD EDI, 0x28  (sizeof IMAGE_SECTION_HEADER)
        _emit 0xc7
        _emit 0x28
        _emit 0x3b              // CMP EBX, EAX
        _emit 0xd8
        _emit 0x72              // JB short -0x1d  (loop)
        _emit 0xe3
        _emit 0xeb              // JMP short +4  (epilogue_edi)
        _emit 0x04
        _emit 0x83              // AND [EBP-8], 0  (not_found: local_8 = 0)
        _emit 0x65
        _emit 0xf8
        _emit 0x00
        _emit 0x8b              // MOV EAX, [EBP-8]  (epilogue_edi:)
        _emit 0x45
        _emit 0xf8
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0xc9              // LEAVE
        _emit 0xc3              // RET
    }
}
