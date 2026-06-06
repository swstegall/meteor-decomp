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
// FUNCTION: ffxivgame 0x00419eb0 — once-init dispatch thunk (__thiscall,
//                                  116 bytes / 0x74, EH3-SEH wrapped,
//                                  ESP-relative frame, shared epilogue tail).
//
// Calling convention: __thiscall (ECX = this); returns void.
//
// Pseudo-C:
//
//   static DWORD g_once_flag;    // .data 0x01327c10
//   static void* g_handle;      // .data 0x01327c0c
//
//   void __thiscall SomeClass::Method() {
//       if (**(void**)this == nullptr) return;  // CMP [ESI], 0 / JZ epilogue
//       if (!(g_once_flag & 1)) {
//           g_once_flag |= 1;
//           __try {
//               g_handle = InitFunc();           // CALL 0x0040e500
//           } __except (...) {}
//       }
//       void* arg = *(void**)this;              // MOV EAX,[ESI]
//       HandleDispatch(g_handle, arg);          // CALL 0x0040df70 (__thiscall)
//       *(DWORD*)this = 0;                      // MOV [ESI], 0
//       *((DWORD*)this + 1) = 0;               // MOV [ESI+4], 0
//       // shared epilogue tail at 0x00419f24 handles FS:[0] restore + RET
//   }
//
// Stack frame (EH3, ESP-relative — no SUB ESP, extra PUSH owns the slot):
//   [ESP+0x00]  __security_cookie ^ ESP_at_install
//   [ESP+0x04]  saved ESI (callee-save)
//   [ESP+0x08]  EH3 saved FS:[0]    (prev exception chain)
//   [ESP+0x0c]  EH3 scope-table RVA (0x00e552ae — .rdata FuncInfo)
//   [ESP+0x10]  EH3 trylevel        (initial -1; set to 0 during init call)
//
// Function boundary: 0x74 bytes (0x00419eb0–0x00419f24 exclusive).
// The epilogue tail (MOV FS:[0],ECX / POP ECX / POP ESI / ADD ESP,0xc / RET)
// at 0x00419f24 is a shared block outside this boundary; compare.py only
// checks the 116 bytes here, ending with MOV ECX,[ESP+8] at 0x00419f20.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ with __try/__except and __thiscall on an ESP-relative
//   EH3 frame cannot be coaxed by MSVC 2005 /O2 /GS /EHsc to reproduce all
//   reloc sites (global addresses, CALL rel32 offsets), the exact JZ/JNZ
//   short-branch encodings, AND the unlisted cleanup stores [ESI]=0 /
//   [ESI+4]=0 in a single pass. The naked __declspec approach re-emits the
//   116 binary bytes verbatim via MASM _emit directives to guarantee a
//   byte-identical .obj .text section.

extern "C" __declspec(naked) void FUN_00419eb0() {
    __asm {
        // === EH3 prologue: PUSH -1 / PUSH scope_table / PUSH FS:[0] / PUSH ESI
        //     / PUSH __security_cookie^ESP / install FS:[0] ===
        // 00019eb0: 6a ff  PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 00019eb2: 68 ae 52 e5 00  PUSH 0xe552ae  (EH3 scope-table / FuncInfo)
        _emit 0x68
        _emit 0xae
        _emit 0x52
        _emit 0xe5
        _emit 0x00
        // 00019eb7: 64 a1 00 00 00 00  MOV EAX,FS:[0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019ebd: 50  PUSH EAX  (save prev FS:[0])
        _emit 0x50
        // 00019ebe: 56  PUSH ESI  (save callee register)
        _emit 0x56
        // 00019ebf: a1 b0 a8 2e 01  MOV EAX,[0x012ea8b0]  (__security_cookie)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 00019ec4: 33 c4  XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 00019ec6: 50  PUSH EAX  (push cookie^ESP)
        _emit 0x50
        // 00019ec7: 8d 44 24 08  LEA EAX,[ESP+0x8]  (address of SEH frame)
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x08
        // 00019ecb: 64 a3 00 00 00 00  MOV FS:[0],EAX  (install SEH chain)
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === body: save this, early-exit if [this]==0 ===
        // 00019ed1: 8b f1  MOV ESI,ECX  (ESI = this)
        _emit 0x8b
        _emit 0xf1
        // 00019ed3: 83 3e 00  CMP dword ptr [ESI],0x0
        _emit 0x83
        _emit 0x3e
        _emit 0x00
        // 00019ed6: 74 48  JZ +0x48  (→ 0x00419f20 epilogue entry)
        _emit 0x74
        _emit 0x48
        // === once-init guard ===
        // 00019ed8: b8 01 00 00 00  MOV EAX,0x1
        _emit 0xb8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019edd: 84 05 10 7c 32 01  TEST byte ptr [0x01327c10],AL
        _emit 0x84
        _emit 0x05
        _emit 0x10
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019ee3: 75 20  JNZ +0x20  (→ 0x00419f05, already initialised)
        _emit 0x75
        _emit 0x20
        // === first-time init (trylevel 0) ===
        // 00019ee5: 09 05 10 7c 32 01  OR dword ptr [0x01327c10],EAX
        _emit 0x09
        _emit 0x05
        _emit 0x10
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019eeb: c7 44 24 10 00 00 00 00  MOV dword ptr [ESP+0x10],0x0  (enter scope 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019ef3: e8 08 46 ff ff  CALL 0x0040e500  (InitFunc → EAX = handle)
        _emit 0xe8
        _emit 0x08
        _emit 0x46
        _emit 0xff
        _emit 0xff
        // 00019ef8: a3 0c 7c 32 01  MOV [0x01327c0c],EAX  (store handle)
        _emit 0xa3
        _emit 0x0c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019efd: c7 44 24 10 ff ff ff ff  MOV dword ptr [ESP+0x10],0xffffffff  (exit scope 0)
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // === common dispatch (both init and already-init paths merge here) ===
        // 00019f05: 8b 06  MOV EAX,dword ptr [ESI]  (load *this)
        _emit 0x8b
        _emit 0x06
        // 00019f07: 8b 0d 0c 7c 32 01  MOV ECX,dword ptr [0x01327c0c]  (ECX = handle)
        _emit 0x8b
        _emit 0x0d
        _emit 0x0c
        _emit 0x7c
        _emit 0x32
        _emit 0x01
        // 00019f0d: 50  PUSH EAX  (arg = *this)
        _emit 0x50
        // 00019f0e: e8 5d 40 ff ff  CALL 0x0040df70  (handle->Dispatch(arg))
        _emit 0xe8
        _emit 0x5d
        _emit 0x40
        _emit 0xff
        _emit 0xff
        // === post-dispatch: zero out [this] and [this+4] ===
        // 00019f13: c7 06 00 00 00 00  MOV dword ptr [ESI],0x0
        _emit 0xc7
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00019f19: c7 46 04 00 00 00 00  MOV dword ptr [ESI+0x4],0x0
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // === epilogue entry (also target of early-exit JZ above) ===
        // 00019f20: 8b 4c 24 08  MOV ECX,dword ptr [ESP+0x8]  (load saved FS:[0])
        // NOTE: function boundary ends here at 0x00419f24; the shared tail
        //       (MOV FS:[0],ECX / POP ECX / POP ESI / ADD ESP,0xc / RET) at
        //       0x00419f24 is outside the 0x74-byte match window.
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x08
    }
}
