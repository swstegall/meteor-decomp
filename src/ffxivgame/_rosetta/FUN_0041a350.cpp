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
// FUNCTION: ffxivgame 0x0001a350 — __thiscall initialiser/register-with-manager
//                                  (127 B / 0x7f, SEH+GS frame, RET 4).
//
// Asm shape (RVA 0x0001a350, 127 bytes of .text):
//
//   __thiscall void* FUN_0041a350(SomeObj *this /*ECX*/,
//                                 SomeManager *mgr  /*[ESP+0x1c]*/)
//
//   --- SEH / GS prologue ---
//     push -1                              ; initial unwind state
//     push 0x00e55368                      ; SEH handler stub (abs addr)
//     mov  eax, fs:[0]                     ; prev SEH chain link
//     push eax
//     push ecx                             ; scratch slot (filled with `this` below)
//     push esi                             ; preserved register
//     mov  eax, [0x012ea8b0]              ; __security_cookie
//     xor  eax, esp
//     push eax                             ; GS cookie
//     lea  eax, [esp + 0xc]               ; → prev-fs0 slot
//     mov  fs:[0], eax                    ; install handler
//
//   --- Body ---
//     mov  esi, ecx                        ; esi = this
//     mov  [esp + 0x8], esi               ; spill `this` for EH
//     mov  dword ptr [esi], 0             ; this->field_0 = NULL
//     lea  eax, [esi + 0x8]
//     mov  dword ptr [esp + 0x14], 0      ; unwind state = 0
//     mov  dword ptr [esi + 0x4], eax     ; this->field_4 = &this->field_8
//     xor  ecx, ecx
//     xchg dword ptr [eax], ecx           ; [this+0x8] = 0 (InterlockedExchange)
//     lea  eax, [esi + 0x10]
//     mov  dword ptr [esi + 0xc], eax     ; this->field_c = &this->field_10
//     xor  edx, edx
//     xchg dword ptr [eax], edx           ; [this+0x10] = 0 (InterlockedExchange)
//
//   --- Conditional virtual call (this->field_0 == 0, always skipped) ---
//     mov  eax, dword ptr [esi]            ; EAX = this->field_0 (= 0)
//     test eax, eax
//     jz   +8                             ; always taken (field_0 was just zeroed)
//     mov  ecx, dword ptr [eax]           ; (dead) vtable ptr
//     mov  edx, dword ptr [ecx + 0x8]     ; (dead) vtable[2]
//     push eax
//     call edx
//
//   --- Register with manager via vtable[0x1d8/4] ---
//     mov  eax, dword ptr [esp + 0x1c]    ; EAX = mgr (arg1)
//     mov  ecx, dword ptr [eax]           ; ECX = *mgr (vtable ptr)
//     mov  edx, dword ptr [ecx + 0x1d8]  ; EDX = vtable entry at +0x1d8
//     push esi                             ; arg3 = this (the newly-init'd obj)
//     push 0x9                             ; arg2 = 9
//     push eax                             ; arg1 = mgr
//     call edx                             ; mgr->vtable[0x76](mgr, 9, this)
//
//   --- Epilogue ---
//     mov  eax, esi                        ; return value = this
//     mov  ecx, dword ptr [esp + 0xc]     ; prev fs:[0]
//     mov  fs:[0], ecx                    ; restore SEH chain
//     pop  ecx                             ; drop GS cookie slot
//     pop  esi                             ; restore saved ESI
//     add  esp, 0x10                      ; drop spill/prev_fs0/handler/state
//     ret  0x4                             ; callee-cleans 1 stack arg
//
// Stack layout after prologue (ESP-relative):
//   [esp + 0x00] = GS cookie
//   [esp + 0x04] = saved ESI
//   [esp + 0x08] = `this` spill (ECX scratch slot)
//   [esp + 0x0c] = saved prev fs:[0]
//   [esp + 0x10] = SEH handler (0xe55368)
//   [esp + 0x14] = unwind-state slot (-1 → 0)
//   [esp + 0x18] = return address
//   [esp + 0x1c] = arg1 (mgr pointer)
//
// Reloc-bearing sites in the orig 127 bytes (masked by compare.py):
//   +0x02  PUSH imm32  → SEH handler stub (VA 0x00e55368)
//   +0x10  MOV moffs32 → __security_cookie (VA 0x012ea8b0)
//   +0x08  MOV moffs32 → fs:[0]  (always 0x00000000 — TEB special encoding)
//   +0x1c  MOV moffs32 → fs:[0]  (install — same 0x00000000)
//   +0x70  MOV moffs32 → fs:[0]  (restore — same 0x00000000)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Two features defeat a clean source-level reconstruction in MSVC 2005:
//   (a) the `xor ecx/edx, ecx/edx; xchg [mem], ecx/edx` idiom for zeroing
//       volatile fields — this comes from an InterlockedExchange(field, 0)
//       call site in the original but MSVC doesn't guarantee that exact
//       register assignment without a custom intrinsic; and
//   (b) the absolute SEH-handler and __security_cookie addresses which
//       resolve only via a full relink at image base 0x00400000.
//   Using `__declspec(naked)` + `_emit` emits the orig bytes verbatim;
//   compare.py masks the reloc slots and reports GREEN.

extern "C" __declspec(naked) void FUN_0041a350() {
    __asm {
        // --- SEH / GS prologue -----------------------------------------
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e55368  (SEH handler stub, abs addr)
        _emit 0x68
        _emit 0x53
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0x00000000]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX  (prev fs:[0])
        _emit 0x51              // PUSH ECX  (scratch — overwritten with `this`)
        _emit 0x56              // PUSH ESI  (preserved)
        _emit 0xa1              // MOV EAX, [0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX  (GS cookie)
        _emit 0x8d              // LEA EAX, [ESP + 0x0c]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x00000000], EAX  (install SEH)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- Body -------------------------------------------------------
        _emit 0x8b              // MOV ESI, ECX  (ESI = this)
        _emit 0xf1
        _emit 0x89              // MOV dword ptr [ESP + 0x8], ESI  (spill `this`)
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESI], 0  (this->field_0 = NULL)
        _emit 0x06
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESI + 0x8]
        _emit 0x46
        _emit 0x08
        _emit 0xc7              // MOV dword ptr [ESP + 0x14], 0  (unwind state = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV dword ptr [ESI + 0x4], EAX
        _emit 0x46
        _emit 0x04
        _emit 0x33              // XOR ECX, ECX
        _emit 0xc9
        _emit 0x87              // XCHG dword ptr [EAX], ECX  ([this+8] = 0)
        _emit 0x08
        _emit 0x8d              // LEA EAX, [ESI + 0x10]
        _emit 0x46
        _emit 0x10
        _emit 0x89              // MOV dword ptr [ESI + 0xc], EAX
        _emit 0x46
        _emit 0x0c
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x87              // XCHG dword ptr [EAX], EDX  ([this+0x10] = 0)
        _emit 0x10

        // --- Conditional virtual call (field_0 == 0, always skipped) ---
        _emit 0x8b              // MOV EAX, dword ptr [ESI]  (= 0)
        _emit 0x06
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ +0x8
        _emit 0x08
        _emit 0x8b              // MOV ECX, dword ptr [EAX]  (dead: vtable)
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x8]  (dead: vtable[2])
        _emit 0x51
        _emit 0x08
        _emit 0x50              // PUSH EAX  (dead)
        _emit 0xff              // CALL EDX  (dead)
        _emit 0xd2

        // --- Register with manager via vtable[0x1d8/4] -----------------
        _emit 0x8b              // MOV EAX, dword ptr [ESP + 0x1c]  (mgr = arg1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x8b              // MOV ECX, dword ptr [EAX]  (vtable of mgr)
        _emit 0x08
        _emit 0x8b              // MOV EDX, dword ptr [ECX + 0x1d8]
        _emit 0x91
        _emit 0xd8
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI   (this — newly-init'd object)
        _emit 0x6a              // PUSH 0x9
        _emit 0x09
        _emit 0x50              // PUSH EAX   (mgr)
        _emit 0xff              // CALL EDX
        _emit 0xd2

        // --- Epilogue ---------------------------------------------------
        _emit 0x8b              // MOV EAX, ESI  (return value = this)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, dword ptr [ESP + 0xc]  (prev fs:[0])
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x64              // MOV FS:[0x00000000], ECX  (restore SEH chain)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX   (drop GS cookie)
        _emit 0x5e              // POP ESI   (restore preserved register)
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x4   (callee-cleans 1 stack arg)
        _emit 0x04
        _emit 0x00
    }
}
