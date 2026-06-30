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
// FUNCTION: ffxivgame 0x004533e0 — wstring-fill-and-test helper
//                                  (__cdecl, 0xbc bytes / 188 B)
//
// Behaviour read from asm/ffxivgame/000533e0_FUN_004533e0.s:
//
//   __cdecl bool FUN_004533e0(SomeObj* obj);   // [ESP+4] = obj
//
//   1. Prolog: EH3-style SEH frame + /GS cookie (two XOR-with-ESP
//      cookie stores: one at [ESP+0x1c] before PUSH EBX, one pushed
//      onto the stack immediately after). FS:[0] is installed to point
//      at the three-dword SEH node at [ESP+0x28].
//
//   2. A std::wstring is default-constructed on the stack at [ESP+0x08]:
//        cap  = 7   → [ESP+0x20]
//        size = 0   → [ESP+0x1c]
//        buf[0] = L'\0' → word at [ESP+0x0c]
//      (The allocator field at [ESP+0x08]+0x00 is left uninitialised,
//      as in every other SSO wstring prolog in this binary.)
//
//   3. CALL __thiscall 0x00449000(this=obj, &wstring)
//      The EH state advances to 0 right before this call (to enable
//      the scope handler to destroy the wstring on unwind).
//
//   4. Get a pointer to the wstring data:
//        if (cap >= 8) ptr = *(wchar_t**)[ESP+0x0c];   // heap ptr
//        else          ptr = &[ESP+0x0c];               // SSO inline buf
//
//   5. CALL dword ptr [0x00f3e288](ptr)   (indirect via function ptr)
//      result != -1 && !(result & 0x10) && (result & 0x1) → BL = 1
//
//   6. EH state → -1 (out of guarded region).
//
//   7. If the wstring is heap-allocated (cap >= 8): call
//      0x0044d350(heap_ptr, (cap*2+2), 0xc) to release the buffer.
//      (cap*2+2 = (cap+1)*sizeof(wchar_t) = allocation size.)
//
//   8. Return BL (0 or 1).
//
// Reloc-bearing sites in the orig 188 bytes (masked by compare.py):
//   +0x03  scope_table imm32    (.rdata 0x00e58118)
//   +0x12  security_cookie      (.data  0x012ea8b0)
//   +0x1e  security_cookie      (.data  0x012ea8b0)
//   +0x50  CALL rel32           (.text  0x00449000)
//   +0x66  CALL [abs32]         (.data  0x00f3e288)
//   +0x97  CALL rel32           (.text  0x0044d350)
//   +0xb4  CALL rel32           (.text  0x009d20f4)  __security_check_cookie
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The EH3 SEH prolog (PUSH -1 / scope_table / FS-chain / cookie XOR
//   twice) plus the indirect CALL through [0x00f3e288] produce seven
//   relocations and an ESP-dependent encoding that no source-level
//   C++ rewrite can reproduce byte-exactly under /O2 /GS /EHsc.
//   The same naked-asm strategy is used by FUN_00401750, FUN_00406680,
//   FUN_00403f10, and all other SEH-prologue functions in the rosetta set.

extern "C" __declspec(naked) void FUN_004533e0()
{
    __asm {
        // --- EH3 SEH prolog + /GS cookie -----------------------------------
        _emit 0x6a              // PUSH -1                   (EH state = -1)
        _emit 0xff
        _emit 0x68              // PUSH scope_table          (reloc +0x03)
        _emit 0x18
        _emit 0x81
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, FS:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x83              // SUB ESP, 0x20
        _emit 0xec
        _emit 0x20
        _emit 0xa1              // MOV EAX, [__security_cookie]   (reloc +0x12)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89              // MOV [ESP+0x1c], EAX       (cookie_1)
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x53              // PUSH EBX
        _emit 0xa1              // MOV EAX, [__security_cookie]   (reloc +0x1e)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // XOR EAX, ESP
        _emit 0xc4
        _emit 0x50              // PUSH EAX                  (cookie_2, on stack)
        _emit 0x8d              // LEA EAX, [ESP+0x28]       (&SEH frame node)
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x64              // MOV FS:[0], EAX           (install SEH frame)
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // --- body ----------------------------------------------------------
        _emit 0x8b              // MOV ECX, [ESP+0x38]       (load obj / arg1)
        _emit 0x4c
        _emit 0x24
        _emit 0x38
        _emit 0x33              // XOR EBX, EBX              (EBX = 0 = false)
        _emit 0xdb
        _emit 0xc7              // MOV dword ptr [ESP+0x20], 7  (wstring.cap = 7)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESP+0x1c], EBX       (wstring.size = 0)
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        _emit 0x66              // MOV word ptr [ESP+0xc], BX (wstring.buf[0] = 0)
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x0c
        _emit 0x8d              // LEA EAX, [ESP+0x08]       (&wstring)
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x50              // PUSH EAX                  (arg to 0x449000)
        _emit 0x89              // MOV [ESP+0x34], EBX       (EH state → 0)
        _emit 0x5c
        _emit 0x24
        _emit 0x34
        _emit 0xe8              // CALL 0x00449000           (reloc +0x50)
        _emit 0xcc
        _emit 0x5b
        _emit 0xff
        _emit 0xff
        // --- get pointer to wstring data ------------------------------------
        _emit 0x83              // CMP [ESP+0x20], 8         (cap >= 8 = heap?)
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        _emit 0x08
        _emit 0x8b              // MOV EAX, [ESP+0x0c]       (heap ptr or buf[0..3])
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x73              // JNC +4                    (heap: use EAX as ptr)
        _emit 0x04
        _emit 0x8d              // LEA EAX, [ESP+0x0c]       (SSO: addr of inline buf)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX                  (arg to indirect call)
        _emit 0xff              // CALL dword ptr [0x00f3e288]  (reloc +0x66)
        _emit 0x15
        _emit 0x88
        _emit 0xe2
        _emit 0xf3
        _emit 0x00
        // --- check return value bits ----------------------------------------
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x74              // JZ +0x0a                  (fail: ret false)
        _emit 0x0a
        _emit 0xa8              // TEST AL, 0x10
        _emit 0x10
        _emit 0x75              // JNZ +6                    (fail)
        _emit 0x06
        _emit 0xa8              // TEST AL, 0x01
        _emit 0x01
        _emit 0x74              // JZ +2                     (fail)
        _emit 0x02
        _emit 0xb3              // MOV BL, 1                 (success: BL = true)
        _emit 0x01
        // --- reset EH state, conditional wstring heap free -----------------
        _emit 0x8b              // MOV EAX, [ESP+0x20]       (cap)
        _emit 0x44
        _emit 0x24
        _emit 0x20
        _emit 0x83              // CMP EAX, 8
        _emit 0xf8
        _emit 0x08
        _emit 0xc7              // MOV [ESP+0x30], -1        (EH state → -1)
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x72              // JC +0x14                  (SSO: skip free)
        _emit 0x14
        _emit 0x8b              // MOV EDX, [ESP+0x0c]       (heap ptr)
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x6a              // PUSH 0xc                  (arg3)
        _emit 0x0c
        _emit 0x8d              // LEA ECX, [EAX+EAX+2]     (cap*2+2 = alloc size)
        _emit 0x4c
        _emit 0x00
        _emit 0x02
        _emit 0x51              // PUSH ECX                  (arg2)
        _emit 0x52              // PUSH EDX                  (arg1 = heap ptr)
        _emit 0xe8              // CALL 0x0044d350           (reloc +0x97)
        _emit 0xd5
        _emit 0x9e
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0xc              (cdecl cleanup, 3 args)
        _emit 0xc4
        _emit 0x0c
        // --- epilog ---------------------------------------------------------
        _emit 0x8a              // MOV AL, BL                (return bool)
        _emit 0xc3
        _emit 0x8b              // MOV ECX, [ESP+0x28]       (old FS chain)
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x64              // MOV FS:[0], ECX           (restore FS)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX                   (pop cookie_2)
        _emit 0x5b              // POP EBX
        _emit 0x8b              // MOV ECX, [ESP+0x1c]       (cookie_1)
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        _emit 0x33              // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8              // CALL 0x009d20f4           (__security_check_cookie, reloc +0xb4)
        _emit 0x5c
        _emit 0xec
        _emit 0x57
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x2c
        _emit 0xc4
        _emit 0x2c
        _emit 0xc3              // RET
    }
}
