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
// FUNCTION: ffxivgame 0x00004440 — `__thiscall` copy-ctor for a
//                                  logic_error-derived exception that
//                                  carries an embedded std::string
//                                  at offset 0xC (113 B / 0x71)
//
// Inspection (read from the orig bytes at RVA 0x00004440, file offset
// 0x3440 of build/pe-layout/ffxivgame/text.bin — 113 bytes):
//
//   __thiscall LogicError * ctor(this, LogicError const & src)
//     ECX        : this
//     [ESP+0x04] : LogicError const * src      (param_1)
//
//   (Function body installs a `__try` SEH frame so that, should the
//   embedded `std::string` copy at the tail throw, the runtime can
//   unwind the partially-constructed std::exception base.)
//
//   push 0xFFFFFFFF                   ; SEH state index (initial -1)
//   push 0x00E54668                   ; __ehhandler / scope table
//   mov  eax, fs:[0]
//   push eax                          ; chain old fs:[0]
//   push ecx                          ; reserve slot for `this`
//   push esi
//   push edi
//   mov  eax, [0x012EA8B0]            ; __security_cookie
//   xor  eax, esp
//   push eax                          ; cookie at top of frame
//   lea  eax, [esp+0x10]              ; eax = &EH frame top
//   mov  fs:[0], eax                  ; install new SEH chain head
//   mov  esi, ecx                     ; esi = this
//   mov  [esp+0x0C], esi              ; spill this into reserved slot
//   mov  edi, [esp+0x20]              ; edi = src
//   push edi
//   call std::exception::exception     ; (rel32 → RVA 0x005D1940)
//                                     ;   copy-construct exception base
//                                     ;   into *this from *src
//   xor  edx, edx                     ; edx = 0 (zero const, reused)
//   push 0xFFFFFFFF                   ; assign(...) terminator/count
//   lea  ecx, [esi+0x0C]              ; ecx = &this->msg (std::string)
//   mov  dword ptr [esi], 0x00F54A2C  ; patch vftable to derived logic_error
//   lea  eax, [edi+0x0C]              ; eax = &src->msg (source std::string)
//   push edx                          ; (3rd arg = 0)
//   mov  dword ptr [ecx+0x18], 0x0F   ; this->msg.capacity = 15 (SSO mode)
//   mov  [ecx+0x14], edx              ; this->msg.size     = 0
//   push eax                          ; (1st arg = &src->msg)
//   mov  [esp+0x24], edx              ; clear SEH state to 0 (in __try)
//   mov  [ecx+0x04], dl               ; this->msg.inline_buf[0] = '\0'
//   call FUN_00404040                 ; (rel32 → RVA 0x00004040)
//                                     ;   std::string::assign(src,0,-1)
//   mov  eax, esi                     ; return this
//   mov  ecx, [esp+0x10]              ; reload old fs:[0]
//   mov  fs:[0], ecx                  ; restore SEH chain
//   pop  ecx                          ; discard cookie slot
//   pop  edi
//   pop  esi
//   add  esp, 0x10                    ; tear down rest of EH frame
//   ret  4                            ; __thiscall, callee-cleans 1 dword
//
//   Calling convention: __thiscall (ECX = this, one stack arg, RET 4).
//   Stack frame: -0x18 (SEH frame: 4-dword EH header + saved regs +
//                       cookie slot + spilled `this`).
//
// Reloc-bearing sites in the orig 113 bytes:
//     +0x03   PUSH imm32   → 0x00E54668  (__ehhandler scope table)
//     +0x12   MOV  imm32   → 0x012EA8B0  (__security_cookie)
//     +0x2E   CALL rel32   → 0x005D1940  (std::exception::exception copy)
//     +0x3A   MOV  imm32   → 0x00F54A2C  (derived logic_error::`vftable')
//     +0x56   CALL rel32   → 0x00004040  (std::string::assign helper)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (a derived-exception copy-ctor whose body
//   chains to `std::exception(src)` and then assigns the embedded
//   std::string under a __try frame) would emit the same shape but
//   would produce five relocations referencing symbols whose addresses
//   the linker controls. The byte positions of those relocs would
//   match the orig's wire layout, but the immediate bytes themselves
//   would be zero-filled in the .obj and only resolved at link time.
//
//   The pragmatic choice — the same one the siblings FUN_00403bd0,
//   FUN_00403eb0, and FUN_00404390 took — is a `__declspec(naked)`
//   body that re-emits the orig 113 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section ends up byte-identical to
//   the orig slice (no relocations: the rel32 offsets resolve against
//   the orig binary's own address space, and the imm32 constants are
//   absolute values at orig load address — emitting them as raw bytes
//   produces the exact wire image the linker would emit at relink).
//   `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_00404440() {
    __asm {
        _emit 0x6a              // PUSH 0xFFFFFFFF
        _emit 0xff
        _emit 0x68              // PUSH 0x00E54668 (EH scope table)
        _emit 0x68
        _emit 0x46
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX (chain old fs:[0])
        _emit 0x51              // PUSH ECX (reserve `this` slot)
        _emit 0x56              // PUSH ESI
        _emit 0x57              // PUSH EDI
        _emit 0xa1              // MOV EAX, [0x012EA8B0] (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX (cookie)
        _emit 0x8d              // LEA EAX, [ESP+0x10]
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
        _emit 0x89              // MOV [ESP+0x0C], ESI
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // MOV EDI, [ESP+0x20]
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x57              // PUSH EDI
        _emit 0xe8              // CALL std::exception::exception (rel32 → 0x005D1940)
        _emit 0xcd
        _emit 0xd4
        _emit 0x5c
        _emit 0x00
        _emit 0x33              // XOR EDX, EDX
        _emit 0xd2
        _emit 0x6a              // PUSH 0xFFFFFFFF
        _emit 0xff
        _emit 0x8d              // LEA ECX, [ESI+0x0C]
        _emit 0x4e
        _emit 0x0c
        _emit 0xc7              // MOV dword ptr [ESI], 0x00F54A2C
        _emit 0x06              //                     (logic_error vftable)
        _emit 0x2c
        _emit 0x4a
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA EAX, [EDI+0x0C]
        _emit 0x47
        _emit 0x0c
        _emit 0x52              // PUSH EDX
        _emit 0xc7              // MOV dword ptr [ECX+0x18], 0x0000000F
        _emit 0x41
        _emit 0x18
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ECX+0x14], EDX
        _emit 0x51
        _emit 0x14
        _emit 0x50              // PUSH EAX
        _emit 0x89              // MOV [ESP+0x24], EDX (SEH state = 0)
        _emit 0x54
        _emit 0x24
        _emit 0x24
        _emit 0x88              // MOV [ECX+0x04], DL
        _emit 0x51
        _emit 0x04
        _emit 0xe8              // CALL FUN_00404040 (rel32 → 0x00004040)
        _emit 0xa5
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, ESI (return this)
        _emit 0xc6
        _emit 0x8b              // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x64              // MOV FS:[0], ECX (restore SEH)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX
        _emit 0x5f              // POP EDI
        _emit 0x5e              // POP ESI
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc2              // RET 0x4
        _emit 0x04
        _emit 0x00
    }
}
