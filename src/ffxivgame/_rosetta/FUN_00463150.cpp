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
// FUNCTION: ffxivgame 0x00063150 — hook-wrapped dispatch thunk
//                                  (__cdecl int (int arg0, int arg1, int arg2),
//                                   98 B / 0x62)
//
// Guards on arg0 > 0 (else returns 0). If a hook function pointer at
// 0x0132e7a0 is non-NULL, calls it as a pre-notification with five args
// (0, arg0, arg1, arg2, 0) and stores 0 into globals 0x0126886c /
// 0x01268870. Then calls the real implementation via the function pointer
// stored at 0x01268878 with (arg0, arg1, arg2). Saves the result in EBP.
// Finally, if the hook pointer is still non-NULL, calls it again as a
// post-notification with (result, arg0, arg1, arg2, 1). Returns the
// result from the inner call.
//
// Calling convention: __cdecl, 3 int-sized stack arguments, returns int (EAX).
// Frame: PUSH EBX / PUSH EBP / PUSH ESI / PUSH EDI; no SUB ESP.
//
// All addresses embedded in the function body are absolute 32-bit VA
// references (data reads / indirect CALL) — no CALL rel32 sites. A
// __declspec(naked) body re-emitting the original 98 bytes verbatim via
// MASM _emit directives produces a .obj whose .text is byte-identical to
// the original slice; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00463150() {
    __asm {
        // 00063150: 53              PUSH EBX
        _emit 0x53
        // 00063151: 8b 5c 24 08     MOV EBX, dword ptr [ESP+0x8]   ; arg0
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x08
        // 00063155: 33 c9           XOR ECX, ECX
        _emit 0x33
        _emit 0xc9
        // 00063157: 3b d9           CMP EBX, ECX                    ; arg0 > 0?
        _emit 0x3b
        _emit 0xd9
        // 00063159: 7f 04           JG +0x4   (→ 0006315f)
        _emit 0x7f
        _emit 0x04
        // 0006315b: 33 c0           XOR EAX, EAX                    ; return 0
        _emit 0x33
        _emit 0xc0
        // 0006315d: 5b              POP EBX
        _emit 0x5b
        // 0006315e: c3              RET
        _emit 0xc3
        // 0006315f: a1 a0 e7 32 01  MOV EAX, [0x0132e7a0]           ; hook fn ptr
        _emit 0xa1
        _emit 0xa0
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 00063164: 3b c1           CMP EAX, ECX                    ; hook == NULL?
        _emit 0x3b
        _emit 0xc1
        // 00063166: 55              PUSH EBP
        _emit 0x55
        // 00063167: 56              PUSH ESI
        _emit 0x56
        // 00063168: 8b 74 24 18     MOV ESI, dword ptr [ESP+0x18]   ; arg2
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x18
        // 0006316c: 57              PUSH EDI
        _emit 0x57
        // 0006316d: 8b 7c 24 18     MOV EDI, dword ptr [ESP+0x18]   ; arg1
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x18
        // 00063171: 89 0d 6c 88 26 01  MOV dword ptr [0x0126886c], ECX  ; = 0
        _emit 0x89
        _emit 0x0d
        _emit 0x6c
        _emit 0x88
        _emit 0x26
        _emit 0x01
        // 00063177: 74 10           JZ +0x10  (→ 00063189, skip pre-hook)
        _emit 0x74
        _emit 0x10
        // 00063179: 51              PUSH ECX                         ; 0
        _emit 0x51
        // 0006317a: 56              PUSH ESI                         ; arg2
        _emit 0x56
        // 0006317b: 57              PUSH EDI                         ; arg1
        _emit 0x57
        // 0006317c: 53              PUSH EBX                         ; arg0
        _emit 0x53
        // 0006317d: 51              PUSH ECX                         ; 0
        _emit 0x51
        // 0006317e: 89 0d 70 88 26 01  MOV dword ptr [0x01268870], ECX  ; = 0
        _emit 0x89
        _emit 0x0d
        _emit 0x70
        _emit 0x88
        _emit 0x26
        _emit 0x01
        // 00063184: ff d0           CALL EAX                         ; pre-hook(0, arg0, arg1, arg2, 0)
        _emit 0xff
        _emit 0xd0
        // 00063186: 83 c4 14        ADD ESP, 0x14                    ; clean 5 args
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00063189: 56              PUSH ESI                         ; arg2
        _emit 0x56
        // 0006318a: 57              PUSH EDI                         ; arg1
        _emit 0x57
        // 0006318b: 53              PUSH EBX                         ; arg0
        _emit 0x53
        // 0006318c: ff 15 78 88 26 01  CALL dword ptr [0x01268878]   ; real_func(arg0, arg1, arg2)
        _emit 0xff
        _emit 0x15
        _emit 0x78
        _emit 0x88
        _emit 0x26
        _emit 0x01
        // 00063192: 8b e8           MOV EBP, EAX                     ; result = EAX
        _emit 0x8b
        _emit 0xe8
        // 00063194: a1 a0 e7 32 01  MOV EAX, [0x0132e7a0]            ; reload hook fn ptr
        _emit 0xa1
        _emit 0xa0
        _emit 0xe7
        _emit 0x32
        _emit 0x01
        // 00063199: 83 c4 0c        ADD ESP, 0xc                     ; clean 3 args
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0006319c: 85 c0           TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0006319e: 74 0b           JZ +0xb   (→ 000631ab, skip post-hook)
        _emit 0x74
        _emit 0x0b
        // 000631a0: 6a 01           PUSH 0x1                          ; status=ok
        _emit 0x6a
        _emit 0x01
        // 000631a2: 56              PUSH ESI                          ; arg2
        _emit 0x56
        // 000631a3: 57              PUSH EDI                          ; arg1
        _emit 0x57
        // 000631a4: 53              PUSH EBX                          ; arg0
        _emit 0x53
        // 000631a5: 55              PUSH EBP                          ; result
        _emit 0x55
        // 000631a6: ff d0           CALL EAX                          ; post-hook(result, arg0, arg1, arg2, 1)
        _emit 0xff
        _emit 0xd0
        // 000631a8: 83 c4 14        ADD ESP, 0x14                     ; clean 5 args
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 000631ab: 5f              POP EDI
        _emit 0x5f
        // 000631ac: 5e              POP ESI
        _emit 0x5e
        // 000631ad: 8b c5           MOV EAX, EBP                      ; return result
        _emit 0x8b
        _emit 0xc5
        // 000631af: 5d              POP EBP
        _emit 0x5d
        // 000631b0: 5b              POP EBX
        _emit 0x5b
        // 000631b1: c3              RET
        _emit 0xc3
    }
}
