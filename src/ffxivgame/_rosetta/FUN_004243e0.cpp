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
// FUNCTION: ffxivgame 0x000243e0 — __thiscall destructor-like method with
//                                   SEH + /GS cookie frame (100 B / 0x64)
//
// __thiscall void FUN_004243e0(void)
//   ECX = this
//
// Logical body:
//   1. Install SEH frame with handler 0xe55e2b and /GS cookie
//      (XOR of __security_cookie @ 0x012ea8b0 with ESP).
//   2. ESI = this; save this into [ESP+0x08] (callee-save ECX slot).
//   3. *(this+0x00) = 0xf5bd20   ; set vtable pointer
//   4. ECX = *(this+0x04)        ; load sub-object pointer
//   5. [ESP+0x14] = 0            ; transition SEH state to 0
//   6. if (ECX != NULL):
//          EDX = **ECX           ; vtable[0] of sub-object
//          PUSH 1
//          CALL EDX              ; (*vtable[0])(sub, 1)
//   7. *(this+0x04) = 0          ; clear sub-object pointer
//   8. *(this+0x08) = 0xf5bd1c   ; set secondary pointer / sentinel
//   9. Restore FS:[0], POP ECX, POP ESI, ADD ESP,0x10, RET
//
// Stack frame layout (relative to ESP after prologue):
//   [ESP+0x00]  /GS cookie (XOR'd __security_cookie ^ ESP)
//   [ESP+0x04]  saved ESI
//   [ESP+0x08]  saved ECX / this (overwritten with ESI at body start)
//   [ESP+0x0c]  saved FS:[0]  ← SEH registration Next ptr
//   [ESP+0x10]  0xe55e2b      ← SEH registration Handler ptr
//   [ESP+0x14]  -1 / 0        ← SEH state word
//
// There are no CALL rel32 instructions (the only call is CALL EDX, indirect),
// so the naked _emit byte stream is byte-identical to the original and
// compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_004243e0() {
    __asm {
        // 000243e0: 6a ff           PUSH -0x1          ; SEH state = -1
        _emit 0x6a
        _emit 0xff
        // 000243e2: 68 2b 5e e5 00  PUSH 0xe55e2b      ; SEH handler address
        _emit 0x68
        _emit 0x2b
        _emit 0x5e
        _emit 0xe5
        _emit 0x00
        // 000243e7: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]  ; save prev SEH chain
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000243ed: 50              PUSH EAX           ; push prev FS:[0]
        _emit 0x50
        // 000243ee: 51              PUSH ECX           ; push this (__thiscall)
        _emit 0x51
        // 000243ef: 56              PUSH ESI           ; push ESI
        _emit 0x56
        // 000243f0: a1 b0 a8 2e 01  MOV EAX,[0x012ea8b0] ; load __security_cookie
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 000243f5: 33 c4           XOR EAX,ESP        ; XOR cookie ^ ESP
        _emit 0x33
        _emit 0xc4
        // 000243f7: 50              PUSH EAX           ; push /GS cookie
        _emit 0x50
        // 000243f8: 8d 44 24 0c     LEA EAX,[ESP+0xc]  ; EAX = &SEH registration
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 000243fc: 64 a3 00 00 00 00  MOV FS:[0x0],EAX ; install SEH frame
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00024402: 8b f1           MOV ESI,ECX        ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 00024404: 89 74 24 08     MOV [ESP+0x8],ESI  ; save this into stack
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x08
        // 00024408: c7 06 20 bd f5 00  MOV [ESI],0xf5bd20  ; *(this+0) = vtable
        _emit 0xc7
        _emit 0x06
        _emit 0x20
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        // 0002440e: 8b 4e 04        MOV ECX,[ESI+0x4]  ; ECX = *(this+4)
        _emit 0x8b
        _emit 0x4e
        _emit 0x04
        // 00024411: 85 c9           TEST ECX,ECX       ; sub-object null?
        _emit 0x85
        _emit 0xc9
        // 00024413: c7 44 24 14 00 00 00 00  MOV [ESP+0x14],0  ; SEH state = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002441b: 74 08           JZ +0x08           ; skip if NULL
        _emit 0x74
        _emit 0x08
        // 0002441d: 8b 01           MOV EAX,[ECX]      ; EAX = vtable of sub-obj
        _emit 0x8b
        _emit 0x01
        // 0002441f: 8b 10           MOV EDX,[EAX]      ; EDX = vtable[0]
        _emit 0x8b
        _emit 0x10
        // 00024421: 6a 01           PUSH 0x1           ; arg: delete=true
        _emit 0x6a
        _emit 0x01
        // 00024423: ff d2           CALL EDX           ; (*vtable[0])(sub, 1)
        _emit 0xff
        _emit 0xd2
        // 00024425: c7 46 04 00 00 00 00  MOV [ESI+0x4],0  ; *(this+4) = NULL
        _emit 0xc7
        _emit 0x46
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002442c: c7 46 08 1c bd f5 00  MOV [ESI+0x8],0xf5bd1c ; *(this+8) = sentinel
        _emit 0xc7
        _emit 0x46
        _emit 0x08
        _emit 0x1c
        _emit 0xbd
        _emit 0xf5
        _emit 0x00
        // 00024433: 8b 4c 24 0c     MOV ECX,[ESP+0xc]  ; ECX = saved FS:[0]
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 00024437: 64 89 0d 00 00 00 00  MOV FS:[0x0],ECX ; restore SEH chain
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0002443e: 59              POP ECX            ; pop /GS cookie
        _emit 0x59
        // 0002443f: 5e              POP ESI            ; restore ESI
        _emit 0x5e
        // 00024440: 83 c4 10        ADD ESP,0x10       ; remove remaining frame
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 00024443: c3              RET
        _emit 0xc3
    }
}
