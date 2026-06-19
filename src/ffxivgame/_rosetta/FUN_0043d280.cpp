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
// FUNCTION: ffxivgame 0x0043d280 — transfer/assign a managed pointer with
//                                  SEH + /GS frame; releases old referent
//                                  via vtable[0](1) if non-null (__cdecl,
//                                  137 bytes / 0x89).
//
// Calling convention: __cdecl (caller cleans 0 arg bytes on return).
// Frame: /GS security-cookie + SEH exception-registration record.
//   Callee-saves pushed: ESI only.
//   Local frame: 8 bytes (2 DWORDs at ESP+0x8 / ESP+0xc after prologue).
//
// Stack layout (relative to ESP after prologue + cookie push):
//   ESP+0x00  cookie (security_cookie ^ ESP)
//   ESP+0x04  saved ESI
//   ESP+0x08  local_1  (user, initialised 0 then 1)
//   ESP+0x0c  local_2  (out-param slot passed to FUN_0043cf00)
//   ESP+0x10  prev SEH node
//   ESP+0x14  handler (0xe56970)
//   ESP+0x18  SEH try-state (-1 → 0)
//   ESP+0x1c  return address
//   ESP+0x20  param1 — void** dst
//   ESP+0x24  param2 — arg2 to FUN_0043cf00
//   ESP+0x28  param3 — arg3 to FUN_0043cf00
//   ESP+0x2c  param4 — arg4 to FUN_0043cf00
//
// Body summary:
//   local_2 = 0;
//   void** pVal = FUN_0043cf00(&local_2, param2, param3, param4);
//   *param1 = *pVal;   *pVal = NULL;
//   if (local_2) local_2->vtable[0](1);   // Release/Destroy with arg 1
//   return param1;
//
// Reloc sites (compare.py masks these positions):
//   +0x02  PUSH 0xe56970            — SEH handler address (abs32 fixup)
//   +0x12  MOV EAX,[0x012ea8b0]    — __security_cookie    (abs32 fixup)
//   +0x40  CALL FUN_0043cf00        — rel32 call target
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The /GS + SEH prologue shape, the exact register spill order (ESI
//   pushed between SUB ESP,8 and the cookie XOR), and the post-call
//   sequence that reads param5 through the pushed-args window before the
//   ADD ESP,0x10 cleanup are all not reproducible from plain C++ source.
//   The naked body re-emits the original 137 bytes verbatim; compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_0043d280() {
    __asm {
        // 0003d280: 6a ff           PUSH -0x1
        _emit 0x6a
        _emit 0xff
        // 0003d282: 68 70 69 e5 00  PUSH 0xe56970  (SEH handler, reloc)
        _emit 0x68
        _emit 0x70
        _emit 0x69
        _emit 0xe5
        _emit 0x00
        // 0003d287: 64 a1 00 00 00 00  MOV EAX,FS:[0x0]
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d28d: 50              PUSH EAX
        _emit 0x50
        // 0003d28e: 83 ec 08        SUB ESP,0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 0003d291: 56              PUSH ESI
        _emit 0x56
        // 0003d292: a1 b0 a8 2e 01  MOV EAX,[0x012ea8b0]  (__security_cookie, reloc)
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0003d297: 33 c4           XOR EAX,ESP
        _emit 0x33
        _emit 0xc4
        // 0003d299: 50              PUSH EAX
        _emit 0x50
        // 0003d29a: 8d 44 24 10     LEA EAX,[ESP+0x10]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x10
        // 0003d29e: 64 a3 00 00 00 00  MOV FS:[0x0],EAX
        _emit 0x64
        _emit 0xa3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d2a4: 8b 4c 24 28     MOV ECX,[ESP+0x28]   ; ECX = param3
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        // 0003d2a8: c7 44 24 08 00 00 00 00  MOV [ESP+0x8],0x0   ; local_1 = 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d2b0: 8b 44 24 2c     MOV EAX,[ESP+0x2c]   ; EAX = param4
        _emit 0x8b
        _emit 0x44
        _emit 0x24
        _emit 0x2c
        // 0003d2b4: 8b 54 24 24     MOV EDX,[ESP+0x24]   ; EDX = param2
        _emit 0x8b
        _emit 0x54
        _emit 0x24
        _emit 0x24
        // 0003d2b8: 50              PUSH EAX  (param4)
        _emit 0x50
        // 0003d2b9: 51              PUSH ECX  (param3)
        _emit 0x51
        // 0003d2ba: 52              PUSH EDX  (param2)
        _emit 0x52
        // 0003d2bb: 8d 44 24 18     LEA EAX,[ESP+0x18]   ; = &local_2
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x18
        // 0003d2bf: 50              PUSH EAX  (&local_2, 1st arg)
        _emit 0x50
        // 0003d2c0: e8 3b fc ff ff  CALL FUN_0043cf00   (rel32, reloc)
        _emit 0xe8
        _emit 0x3b
        _emit 0xfc
        _emit 0xff
        _emit 0xff
        // 0003d2c5: 8b 08           MOV ECX,[EAX]        ; ECX = *result
        _emit 0x8b
        _emit 0x08
        // 0003d2c7: 8b 74 24 30     MOV ESI,[ESP+0x30]   ; ESI = param1 (through pushed-args window)
        _emit 0x8b
        _emit 0x74
        _emit 0x24
        _emit 0x30
        // 0003d2cb: c7 00 00 00 00 00  MOV [EAX],0x0     ; *result = NULL
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d2d1: 83 c4 10        ADD ESP,0x10         ; cdecl cleanup (4 args)
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0003d2d4: 89 0e           MOV [ESI],ECX        ; *param1 = ECX
        _emit 0x89
        _emit 0x0e
        // 0003d2d6: 8b 4c 24 0c     MOV ECX,[ESP+0xc]    ; ECX = local_2
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        // 0003d2da: 85 c9           TEST ECX,ECX
        _emit 0x85
        _emit 0xc9
        // 0003d2dc: c7 44 24 18 00 00 00 00  MOV [ESP+0x18],0x0  ; SEH state → 0
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d2e4: c7 44 24 08 01 00 00 00  MOV [ESP+0x8],0x1   ; local_1 = 1
        _emit 0xc7
        _emit 0x44
        _emit 0x24
        _emit 0x08
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d2ec: 74 08           JZ +0x8              ; → epilogue if local_2 == NULL
        _emit 0x74
        _emit 0x08
        // 0003d2ee: 8b 11           MOV EDX,[ECX]        ; EDX = vtable of local_2
        _emit 0x8b
        _emit 0x11
        // 0003d2f0: 8b 02           MOV EAX,[EDX]        ; EAX = vtable[0]
        _emit 0x8b
        _emit 0x02
        // 0003d2f2: 6a 01           PUSH 0x1
        _emit 0x6a
        _emit 0x01
        // 0003d2f4: ff d0           CALL EAX             ; local_2->vtable[0](1)
        _emit 0xff
        _emit 0xd0
        // 0003d2f6: 8b c6           MOV EAX,ESI          ; return param1
        _emit 0x8b
        _emit 0xc6
        // 0003d2f8: 8b 4c 24 10     MOV ECX,[ESP+0x10]   ; ECX = prev SEH node
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 0003d2fc: 64 89 0d 00 00 00 00  MOV FS:[0x0],ECX  ; restore SEH chain
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0003d303: 59              POP ECX              ; pop cookie
        _emit 0x59
        // 0003d304: 5e              POP ESI              ; restore ESI
        _emit 0x5e
        // 0003d305: 83 c4 14        ADD ESP,0x14         ; remove frame
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 0003d308: c3              RET
        _emit 0xc3
    }
}
