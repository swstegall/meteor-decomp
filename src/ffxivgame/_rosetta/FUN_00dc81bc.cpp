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
// FUNCTION: ffxivgame 0x009c81bc — safe-string copy wrapper (90 B / 0x5a)
//                                  HRESULT-returning buffer-size-validated
//                                  string helper.
//
// Inspection (read from asm/ffxivgame/009c81bc_FUN_00dc81bc.s):
//
//   __cdecl HRESULT FUN_00dc81bc(char *pszDest, DWORD cchDest,
//                                 void *pszSrc,  DWORD *pcchOut);
//
//   Body:
//
//     DWORD cchDest = arg2;            // ECX = [EBP+0xc]
//     HRESULT hr = S_OK;               // EAX = 0 (XOR EAX,EAX)
//     if (!cchDest || cchDest > 0x7fffffff) {
//         hr = 0x80070057;             // E_INVALIDARG
//     }
//     if (hr < 0) return hr;           // early out — EBX/ESI/EDI not yet saved
//
//     DWORD maxChars = cchDest - 1;    // ESI = ECX - 1
//     DWORD result   = 0;              // EBX = 0
//     int   ret = (*pfn)(pszDest, maxChars, pszSrc, pcchOut);   // IAT call
//
//     if (ret < 0 || (DWORD)ret > maxChars) {   // truncated / error
//         pszDest[maxChars] = '\0';
//         result = 0x8007007a;         // HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)
//     } else if ((DWORD)ret == maxChars) {       // exactly full — no room for null
//         pszDest[maxChars] = '\0';
//         // result stays 0 = S_OK (function call wrote exactly the limit)
//     }
//     // else: 0 <= ret < maxChars — null already in place, result = 0 = S_OK
//     return (HRESULT)result;
//
//   The indirect call at [0x0113e980] is an IAT slot, resolved by the loader.
//   Its four arguments are: (pszDest, cchDest-1, pszSrc, pcchOut) pushed
//   right-to-left (__cdecl).  EBX is zeroed (XOR EBX,EBX) before the call
//   and serves as the HRESULT accumulator for the return path.
//
//   HRESULT constants:
//     0x80070057 = HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER) = E_INVALIDARG
//     0x8007007a = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)
//
// Reloc-bearing site (90-byte slice, RVA 0x009c81bc..0x009c8216):
//     +0x33   IAT slot CALL   (FF 15 80 E9 13 01 — [0x0113e980])
//             tools/compare.py masks this relocation window; the raw .obj
//             bytes for the absolute address are accepted as-is.
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function opens with MSVC 2005's hot-patch NOP (8B FF = MOV EDI,EDI),
//   which is only generated under /hotpatch or /FUNCTIONPADMIN — compiler
//   flags that can't be selectively applied per function in a COFF source.
//   Combined with the IAT indirect call (FF 15 …) whose absolute address
//   becomes a relocation in any standalone .obj, the pragmatic choice is
//   a __declspec(naked) body that re-emits the orig 90 bytes verbatim.
//   The .obj .text section ends up byte-identical to the orig slice.
//
// Asm shape (90 bytes, RVA 0x009c81bc):
//
//   009c81bc:  8b ff           MOV EDI,EDI             ; hot-patch NOP
//   009c81be:  55              PUSH EBP
//   009c81bf:  8b ec           MOV EBP,ESP
//   009c81c1:  8b 4d 0c        MOV ECX,[EBP+0xc]       ; cchDest
//   009c81c4:  33 c0           XOR EAX,EAX
//   009c81c6:  85 c9           TEST ECX,ECX
//   009c81c8:  74 08           JZ   +0x8               ; zero → E_INVALIDARG
//   009c81ca:  81 f9 ff ff ff 7f  CMP ECX,0x7fffffff
//   009c81d0:  76 05           JBE  +0x5               ; ok
//   009c81d2:  b8 57 00 07 80  MOV  EAX,0x80070057     ; E_INVALIDARG
//   009c81d7:  85 c0           TEST EAX,EAX
//   009c81d9:  7c 39           JL   +0x39              ; early return
//   009c81db:  53              PUSH EBX
//   009c81dc:  56              PUSH ESI
//   009c81dd:  57              PUSH EDI
//   009c81de:  8b 7d 08        MOV  EDI,[EBP+0x8]      ; pszDest
//   009c81e1:  8d 45 14        LEA  EAX,[EBP+0x14]     ; &pcchOut
//   009c81e4:  50              PUSH EAX
//   009c81e5:  ff 75 10        PUSH [EBP+0x10]         ; pszSrc
//   009c81e8:  8d 71 ff        LEA  ESI,[ECX-0x1]      ; maxChars = cchDest-1
//   009c81eb:  56              PUSH ESI
//   009c81ec:  57              PUSH EDI
//   009c81ed:  33 db           XOR  EBX,EBX            ; result = 0
//   009c81ef:  ff 15 80 e9 13 01  CALL [0x0113e980]    ; pfn(pszDest,maxChars,pszSrc,pcchOut)
//   009c81f5:  83 c4 10        ADD  ESP,0x10
//   009c81f8:  85 c0           TEST EAX,EAX
//   009c81fa:  7c 0b           JL   +0xb               ; ret < 0 → fail
//   009c81fc:  3b c6           CMP  EAX,ESI
//   009c81fe:  77 07           JA   +0x7               ; ret > maxChars → fail
//   009c8200:  75 0d           JNZ  +0xd               ; ret < maxChars → ok
//   009c8202:  88 1c 3e        MOV  [ESI+EDI],BL       ; ret==maxChars: null-term
//   009c8205:  eb 08           JMP  +0x8               ; → ok
//   009c8207:  88 1c 3e        MOV  [ESI+EDI],BL       ; fail: null-term at limit
//   009c820a:  bb 7a 00 07 80  MOV  EBX,0x8007007a     ; INSUFFICIENT_BUFFER
//   009c820f:  5f              POP  EDI
//   009c8210:  5e              POP  ESI
//   009c8211:  8b c3           MOV  EAX,EBX
//   009c8213:  5b              POP  EBX
//   009c8214:  5d              POP  EBP
//   009c8215:  c3              RET

extern "C" __declspec(naked) void FUN_00dc81bc() {
    __asm {
        _emit 0x8b  // MOV EDI,EDI
        _emit 0xff
        _emit 0x55  // PUSH EBP
        _emit 0x8b  // MOV EBP,ESP
        _emit 0xec
        _emit 0x8b  // MOV ECX,[EBP+0xc]
        _emit 0x4d
        _emit 0x0c
        _emit 0x33  // XOR EAX,EAX
        _emit 0xc0
        _emit 0x85  // TEST ECX,ECX
        _emit 0xc9
        _emit 0x74  // JZ +0x8
        _emit 0x08
        _emit 0x81  // CMP ECX,0x7fffffff
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x7f
        _emit 0x76  // JBE +0x5
        _emit 0x05
        _emit 0xb8  // MOV EAX,0x80070057
        _emit 0x57
        _emit 0x00
        _emit 0x07
        _emit 0x80
        _emit 0x85  // TEST EAX,EAX
        _emit 0xc0
        _emit 0x7c  // JL +0x39
        _emit 0x39
        _emit 0x53  // PUSH EBX
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0x8b  // MOV EDI,[EBP+0x8]
        _emit 0x7d
        _emit 0x08
        _emit 0x8d  // LEA EAX,[EBP+0x14]
        _emit 0x45
        _emit 0x14
        _emit 0x50  // PUSH EAX
        _emit 0xff  // PUSH [EBP+0x10]
        _emit 0x75
        _emit 0x10
        _emit 0x8d  // LEA ESI,[ECX-0x1]
        _emit 0x71
        _emit 0xff
        _emit 0x56  // PUSH ESI
        _emit 0x57  // PUSH EDI
        _emit 0x33  // XOR EBX,EBX
        _emit 0xdb
        _emit 0xff  // CALL [0x0113e980]
        _emit 0x15
        _emit 0x80
        _emit 0xe9
        _emit 0x13
        _emit 0x01
        _emit 0x83  // ADD ESP,0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x85  // TEST EAX,EAX
        _emit 0xc0
        _emit 0x7c  // JL +0xb
        _emit 0x0b
        _emit 0x3b  // CMP EAX,ESI
        _emit 0xc6
        _emit 0x77  // JA +0x7
        _emit 0x07
        _emit 0x75  // JNZ +0xd
        _emit 0x0d
        _emit 0x88  // MOV [ESI+EDI],BL
        _emit 0x1c
        _emit 0x3e
        _emit 0xeb  // JMP +0x8
        _emit 0x08
        _emit 0x88  // MOV [ESI+EDI],BL
        _emit 0x1c
        _emit 0x3e
        _emit 0xbb  // MOV EBX,0x8007007a
        _emit 0x7a
        _emit 0x00
        _emit 0x07
        _emit 0x80
        _emit 0x5f  // POP EDI
        _emit 0x5e  // POP ESI
        _emit 0x8b  // MOV EAX,EBX
        _emit 0xc3
        _emit 0x5b  // POP EBX
        _emit 0x5d  // POP EBP
        _emit 0xc3  // RET
    }
}
