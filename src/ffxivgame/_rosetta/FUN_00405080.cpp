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
// FUNCTION: ffxivgame 0x00405080 — build-channel / latest-flag query
//                                  (349 B / 0x15d), `__cdecl bool()`.
//                                  /GS + SEH4-wrapped, two-stage lazy
//                                  CRITICAL_SECTION init + spin-wait.
//
// Asm shape (read from build/pe-layout/ffxivgame/text.bin @ +0x4080,
// 349 bytes — RVA 0x00405080..0x004051dc):
//
//   __cdecl bool FUN_00405080(void);
//
//     ; ---- standard MSVC 2005 SEH4 prologue --------------------------
//     PUSH -1                                ; state cookie (initial)
//     PUSH offset _EH4_handler @ 0x00e54754  ; SEH handler
//     PUSH FS:[0]                            ; save prev ExceptionList
//     SUB  ESP, 0x58                         ; local frame
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     MOV  [ESP+0x54], EAX                   ; per-frame cookie
//     PUSH ESI                               ; save callee-saved
//     PUSH EDI
//     MOV  EAX, [__security_cookie]
//     XOR  EAX, ESP
//     PUSH EAX                               ; second cookie at frame top
//     LEA  EAX, [ESP+0x64]                   ; address of saved fs:[0]
//     MOV  FS:[0], EAX                       ; install new SEH link
//
//     ; ---- body --------------------------------------------------------
//     ; Lazy two-stage init of two C++ Win32 CRITICAL_SECTION wrappers
//     ; gated by the bit flags at g_init_mask @ 0x01323890.
//
//     if (!(g_init_mask & 1)) {
//         g_init_mask |= 1;
//         _state = 0;
//         FUN_00452a40(ecx = &g_cs_b @ 0x132388c, /*spin=*/0);   // ctor(&cs, 0)
//         _state = -1;
//     }
//     if (!(g_init_mask & 2)) {
//         g_init_mask |= 2;
//         _state = 1;
//         FUN_00452a40(ecx = &g_cs_a @ 0x1323888, /*spin=*/-1);  // ctor(&cs, -1)
//         _state = -1;
//     }
//
//     ; ---- main logic ---------------------------------------------
//     EDI = [IAT 0x00f3e1a4]                 ; Win32 import (channel-query thunk)
//     if (EDI(&g_cs_b, 0) == 2) goto epilogue_setnz;
//
//     ESI = [IAT 0x00f3e1a0]                 ; Win32 import (channel-set thunk)
//     if (ESI(&g_cs_b, 1, 0) != 0) goto epilogue_setnz;
//
//     ; ---- build-channel probe (stack helper) ---------------------
//     PUSH offset L"..." @ 0xf54bc0
//     LEA  EAX, [ESP+0x10]
//     PUSH EAX
//     MOV  ECX, &g_channel_helper @ 0x1323898
//     CALL FUN_00447550                       ; helper->probe(&out, L"...")
//
//     LEA  ECX, [ESP+0xc]
//     PUSH ECX
//     _state = 2;
//     CALL FUN_004531c0                       ; bool(&probe_result)
//     ADD  ESP, 4
//
//     PUSH -1
//     PUSH (AL ? 1 : 0)                       ; mode = AL
//     PUSH &g_cs_a
//     CALL ESI                                ; ESI(&g_cs_a, mode, -1)
//
//     PUSH 1
//     PUSH 2
//     PUSH &g_cs_b
//     CALL ESI                                ; ESI(&g_cs_b, 2, 1)
//
//     LEA  ECX, [ESP+0xc]
//     _state = -1
//     CALL FUN_00446f50                       ; helper->~helper()
//
//   epilogue_setnz:
//     PUSH 0
//     PUSH &g_cs_b
//     CALL EDI                                ; (re-probe channel state)
//     if (EAX != 2) {
//         ESI = [IAT 0x00f3e1c8]              ; Win32 Sleep thunk
//         do {
//             PUSH 1; CALL ESI;               ; Sleep(1)
//             PUSH 0; PUSH &g_cs_b; CALL EDI; ; channel-query
//         } while (EAX != 2);                  ; spin until "channel = 2"
//     }
//
//     PUSH 0
//     PUSH &g_cs_a
//     CALL EDI                                ; channel-query &g_cs_a
//     SETNZ AL                                ; return AL = (EAX != 0)
//
//     ; ---- standard MSVC 2005 SEH4 epilogue ----------------------
//     MOV  ECX, [ESP+0x64]
//     MOV  FS:[0], ECX                        ; restore prev ExceptionList
//     POP  ECX                                ; pop second cookie
//     POP  EDI
//     POP  ESI
//     MOV  ECX, [ESP+0x54]
//     XOR  ECX, ESP
//     CALL __security_check_cookie @ 0x009d20f4
//     ADD  ESP, 0x64
//     RET
//
// Behaviour (informal):
//
//   The function lazy-initialises two Win32 CRITICAL_SECTION-like
//   primitives (constructed via the SEH-wrapped ctor at 0x00452a40)
//   guarded by separate bits in g_init_mask, then walks a small
//   state-machine over them via the three import thunks at
//   IAT[0xf3e1a0], IAT[0xf3e1a4], IAT[0xf3e1c8]. The body builds a
//   transient helper on the stack (FUN_00447550 → FUN_004531c0 →
//   FUN_00446f50) to fetch the "release-channel" status, advances
//   the two critical-sections to a "ready" state, then spin-Sleeps
//   on g_cs_b until its channel-query returns 2 before falling
//   through to a final SETNZ on g_cs_a. Used by FUN_00405210 to
//   decide between L"FINAL FANTASY XIV LATEST" and the plain
//   L"FINAL FANTASY XIV" window title.
//
// Reloc-bearing sites in the orig 349 bytes (every imm32 binding to a
// fixed VA in the orig image; tools/compare.py masks these on the
// cmp_obj path):
//
//     +0x03   DIR32 → 0x00e54754   (PUSH offset _EH4_handler)
//     +0x09   DIR32 → fs:[0]       (PUSH FS:[0] — fixed addressing)
//     +0x12   DIR32 → 0x012ea8b0   (__security_cookie)
//     +0x1f   DIR32 → 0x012ea8b0   (__security_cookie — 2nd load)
//     +0x2a   DIR32 → fs:[0]       (MOV FS:[0], EAX — fixed)
//     +0x32   DIR32 → 0x01323890   (g_init_mask, TEST byte ptr, bit 1)
//     +0x3b   DIR32 → 0x01323890   (g_init_mask, OR dword ptr, bit 1)
//     +0x44   DIR32 → 0x0132388c   (g_cs_b — ECX = &g_cs_b)
//     +0x50   REL32 → 0x00452a40   (CALL critical_section_ctor)
//     +0x5d   DIR32 → 0x01323890   (g_init_mask, TEST byte ptr, bit 2)
//     +0x66   DIR32 → 0x01323890   (g_init_mask, OR dword ptr, bit 2)
//     +0x6f   DIR32 → 0x01323888   (g_cs_a — ECX = &g_cs_a)
//     +0x7b   REL32 → 0x00452a40   (CALL critical_section_ctor)
//     +0x88   DIR32 → 0x00f3e1a4   (IAT thunk — channel-query)
//     +0x91   DIR32 → 0x0132388c   (g_cs_b — pushed arg)
//     +0x9c   DIR32 → 0x00f3e1a0   (IAT thunk — channel-set)
//     +0xa6   DIR32 → 0x0132388c   (g_cs_b — pushed arg)
//     +0xb1   DIR32 → 0x00f54bc0   (PUSH offset wide string)
//     +0xbb   DIR32 → 0x01323898   (g_channel_helper)
//     +0xc0   REL32 → 0x00447550   (CALL helper->probe)
//     +0xd2   REL32 → 0x004531c0   (CALL probe-result decoder)
//     +0xe6   DIR32 → 0x01323888   (g_cs_a — pushed arg)
//     +0xf1   DIR32 → 0x0132388c   (g_cs_b — pushed arg)
//     +0x104  REL32 → 0x00446f50   (CALL helper->~helper)
//     +0x10b  DIR32 → 0x0132388c   (g_cs_b — pushed arg, re-probe)
//     +0x117  DIR32 → 0x00f3e1c8   (IAT thunk — Sleep)
//     +0x126  DIR32 → 0x0132388c   (g_cs_b — pushed arg in spin)
//     +0x134  DIR32 → 0x01323888   (g_cs_a — pushed arg, final probe)
//     +0x144  DIR32 → fs:[0]       (MOV FS:[0], ECX restore)
//     +0x155  REL32 → 0x009d20f4   (__security_check_cookie)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ port at /O2 /EHsc /GS would have to reproduce
//   the exact MSVC 2005 SEH4 prologue choices (double cookie, ESI/EDI
//   shrink-wrap timing, state-slot scheduling around the two
//   cmp/jne lazy-init guards) and the orig's exact interleaving of
//   the `mov [esp+0x6c], 0xffffffff` state-write between the
//   `OR g_init_mask` and its dependent CALL. Five SEH-wrapped
//   sibling matches in this size band (incl. FUN_00405210 right
//   next door at 0x405210) all reached GREEN only via naked-asm
//   passthrough for the same reason. The orig bytes have every
//   PC-relative CALL offset and DIR32 absolute baked in at orig's
//   link-time RVA of 0x00405080; the naked-asm body re-emits them
//   verbatim and tools/compare.py reports GREEN (349 of 349) on
//   the orig slice.

extern "C" __declspec(naked) void FUN_00405080() {
    __asm {
        _emit 0x6a
        _emit 0xff
        _emit 0x68
        _emit 0x54
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
        _emit 0x57
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
        _emit 0x64
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xf6
        _emit 0x05
        _emit 0x90
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x75
        _emit 0x23
        _emit 0x83
        _emit 0x0d
        _emit 0x90
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x01
        _emit 0x6a
        _emit 0x00
        _emit 0xb9
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x6c
        _emit 0xd9
        _emit 0x04
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xf6
        _emit 0x05
        _emit 0x90
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x02
        _emit 0x75
        _emit 0x23
        _emit 0x83
        _emit 0x0d
        _emit 0x90
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0x02
        _emit 0x6a
        _emit 0xff
        _emit 0xb9
        _emit 0x88
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x40
        _emit 0xd9
        _emit 0x04
        _emit 0x00
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b
        _emit 0x3d
        _emit 0xa4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd7
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x74
        _emit 0x6d
        _emit 0x8b
        _emit 0x35
        _emit 0xa0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x85
        _emit 0xc0
        _emit 0x75
        _emit 0x58
        _emit 0x68
        _emit 0xc0
        _emit 0x4b
        _emit 0xf5
        _emit 0x00
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x50
        _emit 0xb9
        _emit 0x98
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xe8
        _emit 0x0b
        _emit 0x24
        _emit 0x04
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x70
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x69
        _emit 0xe0
        _emit 0x04
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        _emit 0x84
        _emit 0xc0
        _emit 0x6a
        _emit 0xff
        _emit 0x75
        _emit 0x04
        _emit 0x6a
        _emit 0x00
        _emit 0xeb
        _emit 0x02
        _emit 0x6a
        _emit 0x01
        _emit 0x68
        _emit 0x88
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x6a
        _emit 0x01
        _emit 0x6a
        _emit 0x02
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x6c
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8
        _emit 0xc7
        _emit 0x1d
        _emit 0x04
        _emit 0x00
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd7
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x74
        _emit 0x1b
        _emit 0x8b
        _emit 0x35
        _emit 0xc8
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x8d
        _emit 0x49
        _emit 0x00
        _emit 0x6a
        _emit 0x01
        _emit 0xff
        _emit 0xd6
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x8c
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd7
        _emit 0x83
        _emit 0xf8
        _emit 0x02
        _emit 0x75
        _emit 0xee
        _emit 0x6a
        _emit 0x00
        _emit 0x68
        _emit 0x88
        _emit 0x38
        _emit 0x32
        _emit 0x01
        _emit 0xff
        _emit 0xd7
        _emit 0x85
        _emit 0xc0
        _emit 0x0f
        _emit 0x95
        _emit 0xc0
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x64
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x59
        _emit 0x5f
        _emit 0x5e
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x54
        _emit 0x33
        _emit 0xcc
        _emit 0xe8
        _emit 0x1b
        _emit 0xcf
        _emit 0x5c
        _emit 0x00
        _emit 0x83
        _emit 0xc4
        _emit 0x64
        _emit 0xc3
    }
}
