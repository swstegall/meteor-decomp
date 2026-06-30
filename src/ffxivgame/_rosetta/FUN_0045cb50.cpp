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
// FUNCTION: ffxivgame 0x0045cb50 — null-guard chain + tail-call dispatch (31 B, __cdecl).
//
// Loads the first stack argument (a pointer to some object), walks two
// levels of indirection (obj->field_0c->field_30), and—if both inner
// pointers are non-null—tail-calls the function pointer at field_30 with
// the original first argument preserved at [ESP+4].  Returns 0 if any
// pointer in the chain is null.
//
//   Logical C shape:
//
//     typedef int (__cdecl *PFN)(void*);
//     struct Inner  { /* ... */ PFN fn;  /* +0x30 */ };
//     struct Outer  { /* ... */ Inner* inner; /* +0x0c */ };
//
//     int __cdecl FUN_0045cb50(Outer* obj) {
//         if (!obj)            return 0;
//         Inner* inner = obj->inner;
//         if (!inner)          return 0;
//         PFN fn = inner->fn;
//         if (!fn)             return 0;
//         return fn(obj);      // tail call — JMP EAX
//     }
//
//   MSVC 2005 does not emit a tail-call JMP for an indirect call through a
//   local function-pointer variable in this context; the naked form
//   reproduces the exact byte sequence (no relocations).
//
// Disassembly (31 bytes, RVA 0x0005cb50–0x0005cb6e):
//
//   0005cb50:  8b 4c 24 04        MOV  ECX, [ESP+0x4]      ; obj
//   0005cb54:  85 c9              TEST ECX, ECX
//   0005cb56:  74 14              JZ   null_ret             ; +0x14
//   0005cb58:  8b 41 0c           MOV  EAX, [ECX+0xc]      ; obj->inner
//   0005cb5b:  85 c0              TEST EAX, EAX
//   0005cb5d:  74 0d              JZ   null_ret             ; +0x0d
//   0005cb5f:  8b 40 30           MOV  EAX, [EAX+0x30]     ; inner->fn
//   0005cb62:  85 c0              TEST EAX, EAX
//   0005cb64:  74 06              JZ   null_ret             ; +0x06
//   0005cb66:  89 4c 24 04        MOV  [ESP+0x4], ECX      ; rewrite arg1
//   0005cb6a:  ff e0              JMP  EAX                  ; tail call
//   0005cb6c:  33 c0              XOR  EAX, EAX             ; return 0
//   0005cb6e:  c3                 RET

extern "C" __declspec(naked) void FUN_0045cb50() {
    __asm {
        mov  ecx, dword ptr [esp + 0x4]       // 8b 4c 24 04   obj
        test ecx, ecx                          // 85 c9
        jz   null_ret                          // 74 14
        mov  eax, dword ptr [ecx + 0xc]       // 8b 41 0c      obj->inner
        test eax, eax                          // 85 c0
        jz   null_ret                          // 74 0d
        mov  eax, dword ptr [eax + 0x30]      // 8b 40 30      inner->fn
        test eax, eax                          // 85 c0
        jz   null_ret                          // 74 06
        mov  dword ptr [esp + 0x4], ecx       // 89 4c 24 04   rewrite arg1
        jmp  eax                               // ff e0         tail call
    null_ret:
        xor  eax, eax                          // 33 c0         return 0
        ret                                    // c3
    }
}
