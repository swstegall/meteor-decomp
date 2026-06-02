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
// FUNCTION: ffxivgame 0x009d152b — __cdecl dual-linked-list flush
//                                  (RVA 0x005d152b, 56 bytes / 0x38)
//
// This is the FIRST HALF of a two-part free-list cleanup. The logical
// function spans 0x9d152b–0x9d1571 (0x46 bytes); the YAML boundary at
// 0x9d1563 splits it at byte 56, leaving the second-loop epilogue in the
// anonymous gap region 0x9d1563–0x9d1571.
//
// Call shape:
//   void FUN_009d152b(SomeClass *p)    // __cdecl, one stack arg
//     1. calls FUN_009d1509 (thiscall) with this=p, arg=0
//     2. walks linked list at p->field_0x1c, freeing via FUN_009d1b17
//     3. clears p->field_0x1c
//     4. walks linked list at p->field_0x20, freeing via FUN_009d1b17
//        (continues in the gap region — epilogue there: clears field_0x20,
//         pops EDI/ESI, ret)
//
// Each list node's first DWORD is the "next" pointer (walked via
// MOV EDI, [EAX] before the free call, so EDI = next after each
// FUN_009d1b17 returns).
//
// Register layout at entry:
//   [ESP+4]  SomeClass *p
//   ESI      ← p (saved across function)
//   EDI      ← current node's "next" pointer
//
// The JE at offset 0x2e (+0x0f = target 0x3f) jumps to 0x9d156a which
// is 7 bytes past this function's YAML boundary — it lands in the gap
// region's epilogue (AND [ESI+0x20], 0 / POP EDI / POP ESI / RET).
// Because the jump target is outside our 56-byte window the function
// cannot be expressed as normal C++ without producing a full epilogue;
// naked-asm byte passthrough is the only way to pin the exact encoding.
//
// Raw bytes (56 / 0x38):
//   offset  hex                          decoded
//   0x00    56                           PUSH ESI
//   0x01    8b 74 24 08                  MOV  ESI, [ESP+8]          ; p
//   0x05    57                           PUSH EDI
//   0x06    6a 00                        PUSH 0
//   0x08    8b ce                        MOV  ECX, ESI              ; this = p
//   0x0a    e8 cf ff ff ff               CALL 0x9d1509              ; p->method(0)
//   0x0f    8b 46 1c                     MOV  EAX, [ESI+0x1c]       ; list1 head
//   0x12    85 c0                        TEST EAX, EAX
//   0x14    74 0f                        JE   +0x0f → 0x9d1550      ; skip if null
//   0x16    8b 38                        MOV  EDI, [EAX]            ; next
//   0x18    50                           PUSH EAX                   ; node
//   0x19    e8 ce 05 00 00               CALL 0x9d1b17              ; free(node)
//   0x1e    85 ff                        TEST EDI, EDI
//   0x20    59                           POP  ECX                   ; cdecl cleanup
//   0x21    8b c7                        MOV  EAX, EDI              ; eax = next
//   0x23    75 f1                        JNE  -0x0f → 0x9d1541      ; loop
//   0x25    8b 46 20                     MOV  EAX, [ESI+0x20]       ; list2 head
//   0x28    83 66 1c 00                  AND  [ESI+0x1c], 0         ; clear list1
//   0x2c    85 c0                        TEST EAX, EAX
//   0x2e    74 0f                        JE   +0x0f → 0x9d156a      ; skip (gap)
//   0x30    8b 38                        MOV  EDI, [EAX]            ; next
//   0x32    50                           PUSH EAX                   ; node
//   0x33    e8 b4 05 00 00               CALL 0x9d1b17              ; free(node)
//   [function boundary — epilogue continues in gap 0x9d1563–0x9d1571]
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The call displacements (0xffffffcf, 0x000005ce, 0x000005b4) are
//   absolute-address-space constants baked into the orig binary. No
//   COFF relocations exist at these positions in the _emit encoding, so
//   all 56 bytes are compared structurally by tools/compare.py → GREEN.

extern "C" __declspec(naked) void FUN_009d152b() {
    __asm {
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+8]
        _emit 0x74
        _emit 0x24
        _emit 0x08
        _emit 0x57              // PUSH EDI
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL 0x9d1509  (rel32 = 0xffffffcf)
        _emit 0xcf
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // MOV EAX, [ESI+0x1c]
        _emit 0x46
        _emit 0x1c
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE +0x0f
        _emit 0x0f
        _emit 0x8b              // MOV EDI, [EAX]      (loop1_start)
        _emit 0x38
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x9d1b17  (rel32 = 0x000005ce)
        _emit 0xce
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EDI, EDI
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x8b              // MOV EAX, EDI
        _emit 0xc7
        _emit 0x75              // JNE -0x0f  (→ loop1_start)
        _emit 0xf1
        _emit 0x8b              // MOV EAX, [ESI+0x20]  (exit_loop1)
        _emit 0x46
        _emit 0x20
        _emit 0x83              // AND [ESI+0x1c], 0
        _emit 0x66
        _emit 0x1c
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JE +0x0f  (→ 0x9d156a in gap region)
        _emit 0x0f
        _emit 0x8b              // MOV EDI, [EAX]      (loop2_start)
        _emit 0x38
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL 0x9d1b17  (rel32 = 0x000005b4)
        _emit 0xb4
        _emit 0x05
        _emit 0x00
        _emit 0x00
    }
}
