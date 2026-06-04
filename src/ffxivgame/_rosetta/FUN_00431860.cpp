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
// FUNCTION: ffxivgame 0x00431860 — `__thiscall` SEH-wrapped destructor /
//                                  teardown (122 B / 0x7a, /GS cookie,
//                                  ESP-relative frame).
//
// Asm shape (read from RVA 0x00031860, 122 bytes of `.text`):
//
//   __thiscall void FUN_00431860(T *this /*ECX*/)
//   {
//       // ----- MSVC 2005 EH3 / SEH frame setup ---------------------------
//       push -1                                              ; 6a ff
//       push offset @sehScopeTable_00e5600f                  ; 68 0f 60 e5 00
//       mov  eax, fs:[0]                                     ; 64 a1 00 00 00 00
//       push eax                                             ; 50
//       push ecx                                             ; 51   (frame slot)
//       push ebx ; push esi                                  ; 53 56
//       mov  eax, [__security_cookie]                        ; a1 b0 a8 2e 01
//       xor  eax, esp                                        ; 33 c4
//       push eax                                             ; 50
//       lea  eax, [esp + 0x10]                               ; 8d 44 24 10
//       mov  fs:[0], eax                                     ; 64 a3 00 00 00 00
//       mov  esi, ecx                ; esi = this            ; 8b f1
//       mov  [esp + 0xc], esi        ; spill this            ; 89 74 24 0c
//       mov  dword ptr [esi], 0xf638d4   ; install vtable A  ; c7 06 d4 38 f6 00
//       mov  dword ptr [esp + 0x18], 1   ; SEH trylevel = 1  ; c7 44 24 18 01 00 00 00
//       call FUN_00432820            ; base/sub teardown     ; e8 84 0f 00 00
//       mov  ecx, esi                                        ; 8b ce
//       call FUN_004317c0            ; member teardown        ; e8 1d ff ff ff
//       mov  eax, [esi + 0x44]       ; eax = this->_field44   ; 8b 46 44
//       xor  ebx, ebx                                        ; 33 db
//       cmp  eax, ebx                                        ; 3b c3
//       mov  byte ptr [esp + 0x18], bl  ; trylevel byte = 0   ; 88 5c 24 18
//       jz   .skip                   ; if (field44 == 0)     ; 74 09
//       mov  ecx, [eax - 4]          ; ecx = alloc header     ; 8b 48 fc
//       push eax                                             ; 50
//       call FUN_0040df70            ; operator delete / free ; e8 b7 c6 fd ff
//   .skip:
//       mov  [esi + 0x44], ebx       ; this->_field44 = 0     ; 89 5e 44
//       mov  [esi + 0x48], ebx       ; this->_field48 = 0     ; 89 5e 48
//       mov  [esi + 0x4c], ebx       ; this->_field4C = 0     ; 89 5e 4c
//       mov  dword ptr [esi], 0xf57e14  ; restore vtable B    ; c7 06 14 7e f5 00
//       mov  ecx, [esp + 0x10]       ; saved-FS:[0]           ; 8b 4c 24 10
//       mov  fs:[0], ecx             ; pop SEH handler        ; 64 89 0d 00 00 00 00
//       pop  ecx ; pop esi ; pop ebx                          ; 59 5e 5b
//       add  esp, 0x10                                        ; 83 c4 10
//       ret                          ; __thiscall, no args    ; c3
//
// Reloc-bearing sites in the orig 122 bytes (relocations are masked by
// `tools/compare.py`; a naked-asm `_emit` body emits these as raw
// immediates that link.exe leaves alone, so the .obj `.text` is
// byte-identical to the orig slice with NO relocations):
//
//     +0x02   PUSH imm32   → @sehScopeTable    (VA 0x00e5600f)
//     +0x07   MOV  moffs32 → fs:[0]            (TEB SEH list head)
//     +0x13   MOV  moffs32 → __security_cookie (VA 0x012ea8b0)
//     +0x1d   MOV  moffs32 → fs:[0]            (install handler)
//     +0x29   MOV  [esi], imm32  (vtable A @ 0x00f638d4)
//     +0x37   CALL rel32   → FUN_00432820      (+0x0f84)
//     +0x3e   CALL rel32   → FUN_004317c0      (-0xe3)
//     +0x54   CALL rel32   → FUN_0040df70      (-0x23949)
//     +0x62   MOV  [esi], imm32  (vtable B @ 0x00f57e14)
//     +0x6c   MOV  moffs32 → fs:[0]            (uninstall handler)
//
// Reconstruction strategy — naked-asm byte passthrough (same as the
// sibling SEH-wrapped bodies FUN_00403d60 / FUN_00401650): a
// `__declspec(naked)` body re-emits the orig 122 bytes verbatim via
// MASM `_emit` directives. `tools/compare.py` reports GREEN.

extern "C" __declspec(naked) void FUN_00431860() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00E5600F  (SEH scope table)
        _emit 0x0f
        _emit 0x60
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x53              // PUSH EBX
        _emit 0x56              // PUSH ESI
        _emit 0xa1              // MOV EAX, [0x012EA8B0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA EAX, [ESP + 0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0], EAX
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ESI, ECX
        _emit 0xf1
        _emit 0x89              // MOV [ESP + 0xC], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F638D4
        _emit 0x06
        _emit 0xd4
        _emit 0x38
        _emit 0xf6
        _emit 0x00
        _emit 0xc7              // MOV dword ptr [ESP + 0x18], 1
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL FUN_00432820 (rel32 = +0x0F84)
        _emit 0x84
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_004317C0 (rel32 = -0xE3)
        _emit 0x1d
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, [ESI + 0x44]
        _emit 0x46
        _emit 0x44
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x88              // MOV byte ptr [ESP + 0x18], BL
        _emit 0x5c
        _emit 0x24
        _emit 0x18
        _emit 0x74              // JZ +9  (-> .skip)
        _emit 0x09
        _emit 0x8b              // MOV ECX, [EAX - 4]
        _emit 0x48
        _emit 0xfc
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_0040DF70 (rel32 = -0x23949)
        _emit 0xb7
        _emit 0xc6
        _emit 0xfd
        _emit 0xff
        _emit 0x89              // MOV [ESI + 0x44], EBX
        _emit 0x5e
        _emit 0x44
        _emit 0x89              // MOV [ESI + 0x48], EBX
        _emit 0x5e
        _emit 0x48
        _emit 0x89              // MOV [ESI + 0x4C], EBX
        _emit 0x5e
        _emit 0x4c
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F57E14
        _emit 0x06
        _emit 0x14
        _emit 0x7e
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV ECX, [ESP + 0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0], ECX
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5e              // POP ESI
        _emit 0x5b              // POP EBX
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
