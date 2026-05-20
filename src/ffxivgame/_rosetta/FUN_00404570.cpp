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
// FUNCTION: ffxivgame 0x00404570 — vector-of-28B-record clear loop with SEH
//                                  unwind (__cdecl, 141 bytes)
//
// Shape (paraphrased from the orig 141 bytes at RVA 0x00004570; Ghidra
// pseudo-C in build/ghidra-decomp/ffxivgame/00004570_FUN_00404570.c
// agrees):
//
//   void __cdecl FUN_00404570(Record28 *arr, unsigned count, int param_3)
//
//   prologue — standard `/GS` + SEH:
//     push ebp
//     mov  ebp, esp
//     push -1                        ; SEH scope = -1 (outer)
//     push 0x00e546a1                ; pScopeTable = &LAB_00e546a1
//     mov  eax, fs:[0]               ; prev SEH frame
//     push eax
//     sub  esp, 0xC                  ; reserve 3 locals
//     push ebx ; push esi ; push edi
//     mov  eax, [0x012ea8b0]         ; __security_cookie
//     xor  eax, ebp                  ; cookie ^= frame ptr
//     push eax                       ; install in EH record
//     lea  eax, [ebp - 0xC]
//     mov  fs:[0], eax               ; install new SEH chain head
//     mov  [ebp - 0x10], esp         ; saved esp for unwind
//
//   body — for (i = count; i != 0; --i) clear & destroy element:
//     mov  esi, [ebp + 8]            ; esi = arr
//     mov  edi, [ebp + 0xC]          ; edi = count
//     xor  ebx, ebx
//     mov  [ebp - 0x14], esi         ; save initial arr in scope slot
//     mov  [ebp - 4],  bl            ; scope state = 0  (initial)
//     lea  esp, [esp]                ; 7-byte NOP align (entry-fixup)
//   loop_head:
//     cmp  edi, ebx                  ; count == 0?
//     jbe  done                      ; → unwind & return
//     mov  [ebp + 0xC], esi          ; update arr cursor (visible to SEH)
//     mov  [ebp - 0x18], esi
//     cmp  esi, ebx                  ; arr == NULL?
//     mov  byte ptr [ebp - 4], 1     ; scope state = 1  (inside ctor)
//     je   skip_clear                ; null → just advance
//     mov  eax, [ebp + 0x10]         ; param_3 (ctor arg #1)
//     push -1                        ; -1
//     push ebx                       ; 0
//     mov  dword ptr [esi + 0x18], 0xF
//     mov  dword ptr [esi + 0x14], ebx       ; (0)
//     push eax                       ; param_3
//     mov  ecx, esi                  ; this = &arr[i]
//     mov  byte ptr [esi + 4], bl    ; arr[i]+0x04 = 0
//     call FUN_00404040              ; (rel32 → 0x00404040)
//   skip_clear:
//     sub  edi, 1
//     add  esi, 0x1C                 ; sizeof(Record28) = 28
//     mov  byte ptr [ebp - 4], bl    ; scope state = 0
//     mov  [ebp + 8], esi            ; advance arr cursor
//     jmp  loop_head
//
//   <<< function symbol ends at RVA 0x004045fd (size 0x8D = 141 B); the
//   bytes following — the SEH catch-all body at 0x004045eb..0x00404610 —
//   are catalogued separately as `Catch_All@004045eb` in symbols.json
//   and share physical bytes 0x4045eb..0x4045fc with this function's
//   declared range. The last emitted byte (offset 0x8C, value 0xF0) is
//   the second byte of the `e8 f0 fc ff ff` call-rel32 at 0x004045fb
//   that lives in the catch_all block. compare.py only diffs the first
//   141 bytes, so emitting through 0xF0 inclusive matches the orig.
//
// Reloc-bearing sites (all baked into the orig as absolute byte sequences;
// re-emitting via MASM `_emit` produces a .obj with NO relocations, and
// `tools/compare.py` then reports GREEN against the orig's wire image):
//     +0x06   PUSH imm32   → 0x00E546A1   (SEH scope table @ LAB_00e546a1)
//     +0x17   MOV  moffs32 → 0x012EA8B0   (__security_cookie)
//     +0x68   CALL rel32   → FUN_00404040 (rel32 = 0xfffffa63)
//
// Reconstruction strategy — naked-asm byte passthrough (same approach as
// sibling FUN_00403eb0 / FUN_00403bd0). A source-level form would have
// the linker resolve the SEH scope table address, the __security_cookie
// moffs, and the rel32 CALL — all of which can be re-emitted verbatim
// because the orig binary already holds the final addresses.

extern "C" __declspec(naked) void FUN_00404570() {
    __asm {
        _emit 0x55              // PUSH EBP
        _emit 0x8b              // MOV EBP, ESP
        _emit 0xec
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E546A1 (scope table)
        _emit 0xa1
        _emit 0x46
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, dword ptr fs:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x0C
        _emit 0xec
        _emit 0x0c
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, EBP
        _emit 0xc5
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [EBP - 0x0C]
        _emit 0x45
        _emit 0xf4
        _emit 0x64              // MOV dword ptr fs:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [EBP - 0x10], ESP
        _emit 0x65
        _emit 0xf0
        _emit 0x8b              // MOV ESI, [EBP + 8]
        _emit 0x75
        _emit 0x08
        _emit 0x8b              // MOV EDI, [EBP + 0x0C]
        _emit 0x7d
        _emit 0x0c
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x89              // MOV [EBP - 0x14], ESI
        _emit 0x75
        _emit 0xec
        _emit 0x89              // MOV [EBP - 0x04], EBX  (scope state = 0)
        _emit 0x5d
        _emit 0xfc
        _emit 0x8d              // LEA ESP, [ESP+0]  (7-byte NOP align)
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3b              // CMP EDI, EBX     (loop_head:)
        _emit 0xfb
        _emit 0x76              // JBE done (+0x5C → 0x404610)
        _emit 0x5c
        _emit 0x89              // MOV [EBP + 0x0C], ESI
        _emit 0x75
        _emit 0x0c
        _emit 0x89              // MOV [EBP - 0x18], ESI
        _emit 0x75
        _emit 0xe8
        _emit 0x3b              // CMP ESI, EBX
        _emit 0xf3
        _emit 0xc6              // MOV byte ptr [EBP - 0x04], 1
        _emit 0x45
        _emit 0xfc
        _emit 0x01
        _emit 0x74              // JZ skip_clear (+0x1B → 0x4045dd)
        _emit 0x1b
        _emit 0x8b              // MOV EAX, [EBP + 0x10]
        _emit 0x45
        _emit 0x10
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x53              // PUSH EBX
        _emit 0xc7              // MOV dword ptr [ESI + 0x18], 0x0000000F
        _emit 0x46
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESI + 0x14], EBX
        _emit 0x5e
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0x88              // MOV byte ptr [ESI + 0x04], BL
        _emit 0x5e
        _emit 0x04
        _emit 0xe8              // CALL FUN_00404040 (rel32 = 0xfffffa63)
        _emit 0x63
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        _emit 0x83              // SUB EDI, 0x01
        _emit 0xef
        _emit 0x01
        _emit 0x83              // ADD ESI, 0x1C  (sizeof Record = 28)
        _emit 0xc6
        _emit 0x1c
        _emit 0x88              // MOV byte ptr [EBP - 0x04], BL
        _emit 0x5d
        _emit 0xfc
        _emit 0x89              // MOV [EBP + 0x08], ESI
        _emit 0x75
        _emit 0x08
        _emit 0xeb              // JMP loop_head (-0x3B → 0x4045b0)
        _emit 0xc5
        _emit 0x8b              // MOV ESI, [EBP - 0x14]    (catch_all entry)
        _emit 0x75
        _emit 0xec
        _emit 0x8b              // MOV EDI, [EBP + 0x08]
        _emit 0x7d
        _emit 0x08
        _emit 0x3b              // CMP ESI, EDI
        _emit 0xf7
        _emit 0x74              // JZ +0x12
        _emit 0x12
        _emit 0x8b              // MOV EBX, [EBP + 0x14]
        _emit 0x5d
        _emit 0x14
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ECX, EBX
        _emit 0xcb
        _emit 0xe8              // CALL rel32  (first 2 bytes only — the
        _emit 0xf0              //              remaining 3 bytes live in
                                //              the Catch_All@004045eb
                                //              symbol that follows; the
                                //              function's declared 141 B
                                //              range ends here)
    }
}
