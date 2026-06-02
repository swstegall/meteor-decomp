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
// FUNCTION: ffxivgame 0x009d8eb3 — non-standard foreach-call-pointer loop,
//                                  24 B / 0x18.
//
// Called from FUN_009d902c (the singleton initialiser) with a bespoke
// register+stack hybrid convention:
//
//   mov   eax, <begin>   ; begin pointer in EAX (not on the stack)
//   push  <end>          ; end pointer on the stack
//   call  FUN_009d8eb3
//   pop   ecx            ; caller cleans the one stack slot
//
// The function iterates [begin, end) over a dense array of void(*)()
// function pointers, calling each non-null entry in forward order.
// EAX carries the begin pointer; [ESP+8] (after PUSH ESI inside the
// function) holds the end pointer. RET (no immediate) — the caller owns
// stack cleanup.
//
// Because the convention is not expressible in standard MSVC cdecl /
// stdcall / thiscall syntax, the function is reproduced verbatim via
// _emit directives; tools/compare.py reports GREEN with no masking needed
// (there are no IMAGE_REL_I386_* relocations in this 24-byte slice —
// every jump target is a short relative offset baked into the encoding).
//
// Asm (24 bytes, RVA 0x005d8eb3 .. 0x005d8eca):
//
//   005d8eb3:  56               PUSH ESI
//   005d8eb4:  8b f0            MOV  ESI, EAX            ; ESI = begin
//   005d8eb6:  eb 0b            JMP  SHORT +0x0b          ; → loop_check
//   005d8eb8:  8b 06            MOV  EAX, [ESI]           ; load fn ptr
//   005d8eba:  85 c0            TEST EAX, EAX
//   005d8ebc:  74 02            JZ   SHORT +0x02          ; → skip_call
//   005d8ebe:  ff d0            CALL EAX                  ; invoke fn()
//   005d8ec0:  83 c6 04         ADD  ESI, 4               ; advance ptr
//   005d8ec3:  3b 74 24 08      CMP  ESI, [ESP+8]         ; vs. end
//   005d8ec7:  72 ef            JC   SHORT -0x11          ; → loop_body
//   005d8ec9:  5e               POP  ESI
//   005d8eca:  c3               RET

extern "C" __declspec(naked) void FUN_009d8eb3() {
    __asm {
        _emit 0x56              // push esi
        _emit 0x8b
        _emit 0xf0              // mov esi, eax
        _emit 0xeb
        _emit 0x0b              // jmp short +0x0b (→ loop_check)
        // loop_body:
        _emit 0x8b
        _emit 0x06              // mov eax, [esi]
        _emit 0x85
        _emit 0xc0              // test eax, eax
        _emit 0x74
        _emit 0x02              // jz short +0x02 (→ skip_call)
        _emit 0xff
        _emit 0xd0              // call eax
        // skip_call:
        _emit 0x83
        _emit 0xc6
        _emit 0x04              // add esi, 4
        // loop_check:
        _emit 0x3b
        _emit 0x74
        _emit 0x24
        _emit 0x08              // cmp esi, [esp+8]
        _emit 0x72
        _emit 0xef              // jc short -0x11 (→ loop_body)
        _emit 0x5e              // pop esi
        _emit 0xc3              // ret
    }
}
