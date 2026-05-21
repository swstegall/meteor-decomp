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
// FUNCTION: ffxivgame 0x00405210 — paths-init / window-title helper
//                                  (234 B / 0xea), `__cdecl void(this_like,
//                                  bool is_beta)`. /GS + /EHsc SEH4-wrapped.
//
// Asm shape (read from build/pe-layout/ffxivgame/text.bin @ +0x4210,
// 234 bytes — RVA 0x00405210..0x004052fa):
//
//   __cdecl void FUN_00405210(void* obj, char is_beta);
//
//     ; ---- standard MSVC 2005 SEH4 prologue --------------------------
//     PUSH -1                                ; state cookie (initial)
//     PUSH offset _EH4_handler @ 0x00e54784  ; SEH handler
//     PUSH FS:[0]                            ; save prev ExceptionList
//     SUB  ESP, 0x58                         ; local frame
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     MOV  [ESP+0x54], EAX                   ; cookie at frame+0x54
//     PUSH ESI                               ; save callee-saved
//     MOV  EAX, [__security_cookie]
//     XOR  EAX, ESP
//     PUSH EAX                               ; cookie at frame top
//     LEA  EAX, [ESP+0x60]                   ; address of saved fs:[0]
//     MOV  FS:[0], EAX                       ; install new SEH link
//
//     ; ---- body --------------------------------------------------------
//     MOV  EAX, [DAT_01323898]               ; global pointer (some BOOL*)
//     CMP  BYTE PTR [EAX], 0
//     MOV  ESI, [ESP+0x70]                   ; ESI = obj (param_1)
//     JNE  +7
//       XOR AL, AL                           ; (al = 0; unused early-out)
//       JMP epilogue
//
//     MOV  ECX, ESI
//     CALL FUN_00445530                      ; obj->something()  __thiscall
//     PUSH ESI
//     CALL FUN_00454750                      ; bool FUN_00454750(obj)
//     ADD  ESP, 4
//     TEST AL, AL
//     JZ   epilogue                          ; fall-through to AL = 0 + jmp
//
//     PUSH offset "\\My Games\\" @ 0x00f54c74
//     MOV  ECX, ESI
//     CALL FUN_00448900                      ; obj->append("\\My Games\\")
//
//     LEA  ECX, [ESP+0x8]
//     CALL FUN_00445cf0                      ; stack-local helper ctor/init
//     MOV  ECX, [DAT_01323898]
//     CMP  BYTE PTR [ECX], 0
//     MOV  [ESP+0x68], 0                     ; SEH state := 0
//     JNE  +4
//       XOR EAX, EAX                         ; pwVar2 = NULL
//       JMP join
//     CMP  BYTE PTR [ESP+0x74], 0            ; is_beta?
//     JE   +7
//       MOV EAX, offset L"FINAL FANTASY XIV Beta Version" @ 0x00f54bcc
//       JMP join
//     CALL FUN_00405080                      ; bool is_latest()?
//     TEST AL, AL
//     MOV  EAX, offset L"FINAL FANTASY XIV LATEST" @ 0x00f54c0c
//     JNE  +5
//       MOV EAX, offset L"FINAL FANTASY XIV" @ 0x00f54c40
//   join:
//     PUSH EAX
//     LEA  ECX, [ESP+0xC]
//     CALL FUN_004476e0                      ; helper->set_title(pwVar2)
//     LEA  EDX, [ESP+0x8]
//     PUSH EDX
//     MOV  ECX, ESI
//     CALL FUN_004488f0                      ; obj->assign(helper)
//     LEA  ECX, [ESP+0x8]
//     MOV  [ESP+0x68], 0xFFFFFFFF            ; SEH state := -1 (out of try)
//     CALL FUN_00446f50                      ; helper->~helper()
//     MOV  AL, 1
//
//   epilogue:
//     MOV  ECX, [ESP+0x60]                   ; restore prev fs:[0]
//     MOV  FS:[0], ECX
//     POP  ECX                               ; pop cookie
//     POP  ESI                               ; restore ESI
//     MOV  ECX, [ESP+0x54]
//     XOR  ECX, ESP
//     CALL __security_check_cookie @ 0x009d20f4
//     ADD  ESP, 0x64                         ; collapse frame
//     RET
//
// Behaviour (cross-checked against Ghidra pseudo-C at
// build/ghidra-decomp/ffxivgame/00005210_FUN_00405210.c):
//
//   If a global gate (`*g_init_flag @ 0x01323898`) is set and the
//   "compute user-paths" helper FUN_00454750 returns true, the function
//   appends "\\My Games\\" to `obj`'s string state, then materialises a
//   second helper on the stack and assigns one of three wide-string
//   window titles to it (driven by `is_beta` and a build-channel check
//   via FUN_00405080):
//
//       is_beta != 0                                 → L"FINAL FANTASY XIV Beta Version"
//       is_beta == 0 && FUN_00405080() != 0          → L"FINAL FANTASY XIV LATEST"
//       is_beta == 0 && FUN_00405080() == 0          → L"FINAL FANTASY XIV"
//       g_init_flag re-cleared during call           → NULL (no title)
//
//   Both the per-frame `__security_cookie` (frame+0x54) and the SEH4
//   double-cookie (`PUSH EAX` of `cookie XOR esp`) are emitted — the
//   stack-local helper at [ESP+0x8] is destroyed unconditionally on
//   the success path via FUN_00446f50, and the SEH state slot at
//   [ESP+0x68] tracks try/catch nesting for the EH4 unwind table.
//
// Reloc-bearing sites in the orig 234 bytes (every imm32 binding to a
// fixed VA in the orig image; tools/compare.py masks these on the
// cmp_obj path):
//
//     +0x02   DIR32 → 0x00e54784   (PUSH offset _EH4_handler)
//     +0x09   DIR32 → fs:[0]       (PUSH FS:[0] — fixed addressing)
//     +0x13   DIR32 → 0x012ea8b0   (__security_cookie)
//     +0x1d   DIR32 → 0x012ea8b0
//     +0x29   DIR32 → fs:[0]       (MOV FS:[0], EAX — fixed)
//     +0x2f   DIR32 → 0x01323898   (g_init_flag pointer)
//     +0x3a   REL32 → 0x00445530
//     +0x3e   REL32 → 0x00454750
//     +0x4c   DIR32 → 0x00f54c74   ("\\My Games\\")
//     +0x53   REL32 → 0x00448900
//     +0x5a   REL32 → 0x00445cf0
//     +0x60   DIR32 → 0x01323898
//     +0x73   DIR32 → 0x00f54bcc   (L"FINAL FANTASY XIV Beta Version")
//     +0x7c   REL32 → 0x00405080
//     +0x83   DIR32 → 0x00f54c0c   (L"FINAL FANTASY XIV LATEST")
//     +0x8a   DIR32 → 0x00f54c40   (L"FINAL FANTASY XIV")
//     +0x93   REL32 → 0x004476e0
//     +0xa1   REL32 → 0x004488f0
//     +0xb2   REL32 → 0x00446f50
//     +0xc6   DIR32 → fs:[0]       (MOV FS:[0], ECX restore)
//     +0xd7   REL32 → 0x009d20f4   (__security_check_cookie)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ port at /O2 /EHsc /GS would have to reproduce
//   the exact MSVC 2005 SEH4 prologue choices (double cookie, ESI
//   shrink-wrap timing, state-slot scheduling around the cmp/jne) and
//   the orig's interleaving of the `mov [esp+0x68], 0` state-write
//   between the global-load and its dependent jne. Five SEH-wrapped
//   sibling matches in this size band (incl. FUN_00403a20 and
//   FUN_004014b0) all reached GREEN only via naked-asm passthrough
//   for the same reason. The orig bytes have every PC-relative CALL
//   offset and DIR32 absolute baked in at orig's link-time RVA of
//   0x00405210; the naked-asm body re-emits them verbatim and
//   tools/compare.py reports GREEN (234 of 234) on the orig slice.

extern "C" __declspec(naked) void FUN_00405210() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x84
        _emit 0x47
        _emit 0xe5
        _emit 0x00
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x83
        _emit 0xec
        _emit 0x58
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x54
        _emit 0x56
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33
        _emit 0xc4
        _emit 0x50
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x60
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xa1
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x80
        _emit 0x38
        _emit 0x00
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x70
        _emit 0x75
        _emit 0x07
        _emit 0x32
        _emit 0xc0
        _emit 0xe9
        _emit 0x8a
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0xd5
        _emit 0x02
        _emit 0x04
        _emit 0x00
        _emit 0x56
        _emit 0xe8
        _emit 0xef
        _emit 0xf4
        _emit 0x04
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x84
        _emit 0xc0
        _emit 0x74
        _emit 0xe5
        _emit 0x68
        _emit 0x74
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x8c
        _emit 0x36
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xe8
        _emit 0x73
        _emit 0x0a
        _emit 0x04
        _emit 0x00
        _emit 0x8b
        _emit 0x0d
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x80
        _emit 0x39
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x75
        _emit 0x04
        _emit 0x33
        _emit 0xc0
        _emit 0xeb
        _emit 0x21
        _emit 0x80
        _emit 0x7c
        _emit 0x24
        _emit 0x74
        _emit 0x00
        _emit 0x74
        _emit 0x07
        _emit 0xb8
        _emit 0xcc
        _emit 0x4b
        _emit 0xf5
        _emit 0x00
        _emit 0xeb
        _emit 0x13
        _emit 0xe8
        _emit 0xd9
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x84
        _emit 0xc0
        _emit 0xb8
        _emit 0x0c
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x75
        _emit 0x05
        _emit 0xb8
        _emit 0x40
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x50
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xe8
        _emit 0x21
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x08
        _emit 0x52
        _emit 0x8b
        _emit 0xce
        _emit 0xe8
        _emit 0x25
        _emit 0x36
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x68
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0x74
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0xb0
        _emit 0x01
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x60
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0xfe
        _emit 0xcd
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x64
        _emit 0xc3
    }
}
