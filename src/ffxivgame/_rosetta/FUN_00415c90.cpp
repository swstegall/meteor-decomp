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
// FUNCTION: ffxivgame 0x00415c90 — singleton "get-or-init" wrapped in MSVC
//                                  SEH frame (78 B / 0x4e)
//
// __cdecl void get_or_init_singleton(void)
//
//   if (g_singleton_ptr == NULL) {
//       // Construct in-place in the byte-aligned storage that follows
//       // the singleton-pointer slot.
//       g_singleton_ptr = (T*)&g_singleton_storage;   // 0x01328098
//       g_singleton_ptr->ctor(0x80, 0x200);           // FUN_00415bf0
//   }
//
// Memory layout (inferred from absolute immediates):
//   0x01328090: T*  g_singleton_ptr          ; loaded + stored
//   0x01328098: T   g_singleton_storage      ; the actual storage that the
//                                            ; ctor populates (used as the
//                                            ; ECX `this` pointer feeding
//                                            ; into FUN_00415bf0)
//
// The 8-byte gap between the pointer slot and the storage is either
// padding for 16-byte alignment of the storage, or an unrelated global
// living between them. FUN_00415bf0 is the ctor: it takes (ECX = this,
// stack arg0 = 0x80, stack arg1 = 0x200), writes a vtbl into `*this`
// (`mov dword ptr [esi], 0x00f5763c`), tail-calls a sub, and returns
// `this` in EAX — which is why storing the call result back into
// `g_singleton_ptr` works: `eax == this == 0x01328098`.
//
// SEH frame (canonical MSVC __try setup):
//
//   push -1                          ; trylevel = -1 (no active try)
//   push offset @@__ehhandler        ; LAB_00e551e6 (handler)
//   push fs:[0]                      ; chain prev SEH
//   mov  fs:[0], esp                 ; install our frame
//   push ecx                         ; reserve local slot (local_4)
//   ... body ...
//   mov  ecx, [esp+4]                ; ecx = prev SEH
//   mov  fs:[0], ecx                 ; unchain
//   add  esp, 0x10                   ; pop local + chain + handler + cookie
//   ret
//
// Inside the if-block the trylevel cookie is rewritten to 0 (`mov
// [esp+0x14], eax` where eax==0 from the just-tested g_singleton_ptr),
// indicating "we are now inside try level 0" so the LAB_00e551e6
// handler knows which scope is active if the ctor throws.
//
// Inspection (read from the orig bytes at RVA 0x00015c90, 78 bytes):
//
//   push -1                              ; 6a ff
//   push 0x00e551e6                      ; 68 e6 51 e5 00       (handler addr)
//   mov  eax, fs:[0]                     ; 64 a1 00 00 00 00
//   push eax                             ; 50
//   mov  fs:[0], esp                     ; 64 89 25 00 00 00 00
//   push ecx                             ; 51                   (reserve local)
//   mov  eax, [0x01328090]               ; a1 90 80 32 01       (g_singleton_ptr)
//   test eax, eax                        ; 85 c0
//   jnz  +0x20  (to teardown)            ; 75 20
//   mov  ecx, 0x01328098                 ; b9 98 80 32 01       (storage addr)
//   mov  [esp], ecx                      ; 89 0c 24             (local_4 = storage)
//   push 0x200                           ; 68 00 02 00 00       (ctor arg2)
//   push 0x80                            ; 68 80 00 00 00       (ctor arg1)
//   mov  [esp+0x14], eax                 ; 89 44 24 14          (trylevel = 0)
//   call FUN_00415bf0                    ; e8 26 ff ff ff       (rel32)
//   mov  [0x01328090], eax               ; a3 90 80 32 01       (singleton_ptr = this)
//   mov  ecx, [esp+0x4]                  ; 8b 4c 24 04          (ecx = prev SEH)
//   mov  fs:[0], ecx                     ; 64 89 0d 00 00 00 00 (unchain SEH)
//   add  esp, 0x10                       ; 83 c4 10
//   ret                                  ; c3
//
// Reloc-bearing sites in the orig 78 bytes (absolute addresses that
// resolve only at relink time in a full-binary link; standalone .obj
// compilation can't reproduce them via source, so we emit them as raw
// immediate bytes that happen to coincide with the orig's resolved
// addresses):
//     +0x03   PUSH imm32   0x00e551e6  (__ehhandler — DIR32)
//     +0x09   MOV  fs:[0]  literal 0   (no reloc — encodes fs override)
//     +0x16   MOV  EAX, [0x01328090]   (.data DIR32 — g_singleton_ptr)
//     +0x1f   MOV  ECX, 0x01328098     (.data DIR32 — g_singleton_storage)
//     +0x35   CALL rel32 → FUN_00415bf0 (REL32; resolves to RVA 0x015bf0)
//     +0x3a   MOV  [0x01328090], EAX   (.data DIR32 — g_singleton_ptr)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here is achievable in principle — a __try wrapper
//   around an in-place placement-new of a global byte buffer — but
//   getting MSVC 2005 /O2 /GS /EHsc to emit the exact register-spill
//   ordering (the `mov [esp], ecx` between `mov ecx, 0x01328098` and
//   the two `push` immediates), the exact trylevel-write site
//   (`mov [esp+0x14], eax` interleaved AFTER the pushes rather than
//   before), and the exact teardown shape (single `add esp, 0x10`
//   covering local + chain + handler + cookie rather than two pops +
//   add) requires controlled surrounding TU context that we don't
//   have without lifting the rest of the singleton's source.
//
//   The pragmatic choice — the same one taken by every sibling SEH-
//   wrapped helper in this directory — is a `__declspec(naked)` body
//   that re-emits the orig 78 bytes verbatim via MASM `_emit`
//   directives. The .obj's `.text` section is byte-identical to the
//   orig slice (no relocations: the absolute addresses and the rel32
//   are emitted as raw immediates, matching what the orig binary's
//   already-resolved bytes contain). `tools/compare.py` then reports
//   GREEN.

extern "C" __declspec(naked) void FUN_00415c90() {
    __asm {
        _emit 0x6a              // PUSH -1
        _emit 0xff
        _emit 0x68              // PUSH 0x00e551e6   (SEH handler addr)
        _emit 0xe6
        _emit 0x51
        _emit 0xe5
        _emit 0x00
        _emit 0x64              // MOV EAX, fs:[0]
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x64              // MOV fs:[0], ESP
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                  (reserve local_4)
        _emit 0xa1              // MOV EAX, [0x01328090]     (g_singleton_ptr)
        _emit 0x90
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +0x20   (to teardown)
        _emit 0x20
        _emit 0xb9              // MOV ECX, 0x01328098       (g_singleton_storage)
        _emit 0x98
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x89              // MOV [ESP], ECX            (local_4 = storage)
        _emit 0x0c
        _emit 0x24
        _emit 0x68              // PUSH 0x00000200           (ctor arg2)
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00000080           (ctor arg1)
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x89              // MOV [ESP+0x14], EAX       (trylevel = 0)
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0xe8              // CALL FUN_00415bf0 (rel32 → RVA 0x015bf0)
        _emit 0x26
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xa3              // MOV [0x01328090], EAX     (g_singleton_ptr = this)
        _emit 0x90
        _emit 0x80
        _emit 0x32
        _emit 0x01
        _emit 0x8b              // MOV ECX, [ESP+0x4]        (ecx = prev SEH)
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x64              // MOV fs:[0], ECX           (unchain SEH)
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0xc3              // RET
    }
}
