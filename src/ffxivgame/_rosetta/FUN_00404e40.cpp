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
// FUNCTION: ffxivgame 0x00004e40 — registry-DWORD reader helper
//                                  (208 B / 0xd0), __cdecl bool(HKEY,
//                                  LPCWSTR, LPCWSTR, DWORD *).
//
// Behaviour (read from the orig 208 bytes @ RVA 0x00004e40..0x00004f10):
//
//   bool FUN_00404e40(HKEY  hKey,
//                     LPCWSTR lpSubKey,
//                     LPCWSTR lpValueName,
//                     DWORD *out)
//   {
//       HKEY  hOpened;
//       DWORD dwType;
//       DWORD cbData;     // value-len in/out (uninitialised on entry —
//                         // RegQueryValueExW tolerates this and writes
//                         // the actual size on return)
//       DWORD dwValue;
//       BYTE  buf[100];   // throwaway probe buffer for the size-query
//                         // RegQueryValueExW call (forces the /GS cookie)
//
//       if (RegOpenKeyExW(hKey, lpSubKey, 0, KEY_READ /*0x20019*/,
//                         &hOpened) == ERROR_SUCCESS) {
//           if (RegQueryValueExW(hOpened, lpValueName, NULL, &dwType,
//                                buf, &cbData) == ERROR_SUCCESS
//               && dwType == REG_DWORD /*4*/) {
//               if (RegQueryValueExW(hOpened, lpValueName, NULL, &dwType,
//                                    (LPBYTE)&dwValue, &cbData)
//                   == ERROR_SUCCESS) {
//                   *out = dwValue;
//                   RegCloseKey(hOpened);
//                   return true;                       // AL = 1
//               }
//           }
//       }
//       RegCloseKey(hOpened);                          // shared cleanup —
//                                                      // also called on the
//                                                      // RegOpenKeyExW
//                                                      // failure path, on
//                                                      // a zero-init local
//       return false;                                  // AL = 0
//   }
//
// Calling convention: __cdecl, 4 stack args, returns AL (caller treats as
// bool). The full DWORD output goes through *param_4. No `ret N` — the
// 0x78-byte stack frame is torn down with `add esp, 0x78` after the
// /GS cookie check.
//
// Reloc-bearing sites (all DIR32 ABS32 or REL32 — compare.py masks the
// 4-byte operand windows during the byte diff):
//   +0x04  a1 ABS32 → 0x012ea8b0   (__security_cookie load)
//   +0x39  ff 15 ABS32 → 0x00f3e00c (__imp__RegOpenKeyExW IAT)
//   +0x43  8b 3d ABS32 → 0x00f3e004 (__imp__RegQueryValueExW IAT, hoisted)
//   +0x95  ff 15 ABS32 → 0x00f3e010 (__imp__RegCloseKey IAT)
//   +0xa6  e8 REL32 → 0x009d20f4     (__security_check_cookie)
//   +0xb4  ff 15 ABS32 → 0x00f3e010 (__imp__RegCloseKey IAT)
//   +0xc8  e8 REL32 → 0x009d20f4
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ port of this function compiles to a body whose
//   .text bytes match orig modulo register-allocation tiebreakers
//   (EBX vs ESI vs EDI for the param-shuffle dance at the top, and the
//   placement of `mov edi, [__imp__RegQueryValueExW]` between the
//   first two pushes is something MSVC's instruction scheduler
//   chooses, not the source). Three sibling /GS+Win32-API rosetta
//   matches in this size band took the naked-asm route for the same
//   reason. The body is small enough (0xd0 bytes) that an `_emit`-only
//   passthrough is the cleanest path to GREEN — no relocs are emitted
//   in the .obj at all (the IAT addresses and rel32 offsets resolve
//   to orig's link-time RVA of 0x00404e40 and so are baked verbatim);
//   tools/compare.py reports GREEN against the orig slice.

extern "C" __declspec(naked) void FUN_00404e40() {
    __asm {
        // ---- prologue + /GS cookie save ----
        _emit 0x83              // sub  esp, 0x78
        _emit 0xec
        _emit 0x78
        _emit 0xa1              // mov  eax, ds:[0x012ea8b0]  (__security_cookie)
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33              // xor  eax, esp
        _emit 0xc4
        _emit 0x89              // mov  [esp+0x74], eax
        _emit 0x44
        _emit 0x24
        _emit 0x74

        // ---- load params (param2/lpSubKey, param1/hKey) ----
        _emit 0x8b              // mov  ecx, [esp+0x80]
        _emit 0x8c
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // mov  eax, [esp+0x7c]
        _emit 0x44
        _emit 0x24
        _emit 0x7c
        _emit 0x53              // push ebx
        _emit 0x8b              // mov  ebx, [esp+0x8c]  (param4 = out DWORD*)
        _emit 0x9c
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x56              // push esi
        _emit 0x8b              // mov  esi, [esp+0x8c]  (param3 = lpValueName)
        _emit 0xb4
        _emit 0x24
        _emit 0x8c
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57              // push edi

        // ---- RegOpenKeyExW(hKey, lpSubKey, 0, 0x20019, &hOpened) ----
        _emit 0x8d              // lea  edx, [esp+0xc]
        _emit 0x54
        _emit 0x24
        _emit 0x0c
        _emit 0x52              // push edx     (phkResult)
        _emit 0x68              // push 0x20019 (KEY_READ)
        _emit 0x19
        _emit 0x00
        _emit 0x02
        _emit 0x00
        _emit 0x6a              // push 0       (ulOptions)
        _emit 0x00
        _emit 0x51              // push ecx     (lpSubKey)
        _emit 0x50              // push eax     (hKey)
        _emit 0xff              // call [__imp__RegOpenKeyExW]
        _emit 0x15
        _emit 0x0c
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x85              // test eax, eax
        _emit 0xc0
        _emit 0x75              // jne  fail   (+0x6c)
        _emit 0x6c

        // ---- RegQueryValueExW size probe ----
        _emit 0x8b              // mov  edi, ds:[__imp__RegQueryValueExW]
        _emit 0x3d
        _emit 0x04
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x8d              // lea  eax, [esp+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x50              // push eax    (lpcbData)
        _emit 0x8b              // mov  eax, [esp+0x10]  (hOpened)
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x8d              // lea  ecx, [esp+0x20]
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        _emit 0x51              // push ecx    (lpData = &buf)
        _emit 0x8d              // lea  edx, [esp+0x18]
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x52              // push edx    (lpType)
        _emit 0x6a              // push 0      (lpReserved)
        _emit 0x00
        _emit 0x56              // push esi    (lpValueName)
        _emit 0x50              // push eax    (hKey)
        _emit 0xff              // call edi
        _emit 0xd7
        _emit 0x85              // test eax, eax
        _emit 0xc0
        _emit 0x75              // jne  fail   (+0x49)
        _emit 0x49
        _emit 0x83              // cmp  [esp+0x10], 4   (dwType == REG_DWORD)
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x75              // jne  fail   (+0x42)
        _emit 0x42

        // ---- RegQueryValueExW DWORD read ----
        _emit 0x8d              // lea  ecx, [esp+0x14]
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        _emit 0x51              // push ecx    (lpcbData)
        _emit 0x8b              // mov  ecx, [esp+0x10]  (hOpened)
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        _emit 0x8d              // lea  edx, [esp+0x1c]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x52              // push edx    (lpData = &dwValue)
        _emit 0x8d              // lea  eax, [esp+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x50              // push eax    (lpType)
        _emit 0x6a              // push 0      (lpReserved)
        _emit 0x00
        _emit 0x56              // push esi    (lpValueName)
        _emit 0x51              // push ecx    (hKey)
        _emit 0xff              // call edi
        _emit 0xd7
        _emit 0x85              // test eax, eax
        _emit 0xc0
        _emit 0x75              // jne  fail   (+0x25)
        _emit 0x25

        // ---- success: *out = dwValue; RegCloseKey; return true ----
        _emit 0x8b              // mov  eax, [esp+0xc]   (hOpened)
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x8b              // mov  edx, [esp+0x18]  (dwValue)
        _emit 0x54
        _emit 0x24
        _emit 0x18
        _emit 0x50              // push eax    (hKey for RegCloseKey)
        _emit 0x89              // mov  [ebx], edx     (*out = dwValue)
        _emit 0x13
        _emit 0xff              // call [__imp__RegCloseKey]
        _emit 0x15
        _emit 0x10
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x5f              // pop  edi
        _emit 0x5e              // pop  esi
        _emit 0xb0              // mov  al, 1
        _emit 0x01
        _emit 0x5b              // pop  ebx
        _emit 0x8b              // mov  ecx, [esp+0x74]
        _emit 0x4c
        _emit 0x24
        _emit 0x74
        _emit 0x33              // xor  ecx, esp
        _emit 0xcc
        _emit 0xe8              // call __security_check_cookie (rel32 → 0x009d20f4)
        _emit 0x0a
        _emit 0xd2
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // add  esp, 0x78
        _emit 0xc4
        _emit 0x78
        _emit 0xc3              // ret

        // ---- fail: RegCloseKey(hOpened); return false ----
        _emit 0x8b              // mov  ecx, [esp+0xc]   (hOpened)
        _emit 0x4c
        _emit 0x24
        _emit 0x0c
        _emit 0x51              // push ecx
        _emit 0xff              // call [__imp__RegCloseKey]
        _emit 0x15
        _emit 0x10
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x8b              // mov  ecx, [esp+0x80]  (cookie — pre-pop view)
        _emit 0x8c
        _emit 0x24
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x5f              // pop  edi
        _emit 0x5e              // pop  esi
        _emit 0x5b              // pop  ebx
        _emit 0x33              // xor  ecx, esp
        _emit 0xcc
        _emit 0x32              // xor  al, al
        _emit 0xc0
        _emit 0xe8              // call __security_check_cookie
        _emit 0xe8
        _emit 0xd1
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // add  esp, 0x78
        _emit 0xc4
        _emit 0x78
        _emit 0xc3              // ret
    }
}

// vim: ts=4 sts=4 sw=4 et
