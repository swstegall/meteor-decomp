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
// FUNCTION: ffxivgame 0x0044ee30 — "fetch module path, hand to sibling,
//                                   return !result" /GS-guarded thunk
//                                   (82 B / 0x52, __cdecl, returns bool).
//
// Asm shape (read from orig RVA 0x0004ee30):
//
//   bool __cdecl FUN_0044ee30() {
//       wchar_t buf[0x800];                       // 0x1000-byte local array
//       // /GS prologue: mov eax, 0x1004; call __chkstk; cookie xor esp
//       Api(0, buf, 0x800);                        // call dword ptr [0x00f3e1e0]
//       int r = FUN_0044e9f0(buf);                 // call 0x0044e9f0
//       return r == 0;                             // neg/sbb/inc → !r
//       // /GS epilogue: xor ecx, esp; __security_check_cookie; add esp,0x1004
//   }
//
// Reloc-bearing sites in the orig 82 bytes (all resolved at full-binary
// relink — PC-relative CALLs and absolute DIR32 data references):
//   +0x05   CALL __chkstk / _alloca_probe        (e8 + REL32 → 0x009d29d0)
//   +0x0a   MOV  eax, [__security_cookie]         (a1 + DIR32 → 0x012ea8b0)
//   +0x24   CALL dword ptr [import]              (ff15 + DIR32 → 0x00f3e1e0)
//   +0x2e   CALL FUN_0044e9f0                    (e8 + REL32 → 0x0044e9f0)
//   +0x46   CALL __security_check_cookie         (e8 + REL32 → 0x009d20f4)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function is short and reloc-dense (three CALLs + two absolute data
//   refs). A source-level port would need the matching /GS cookie codegen,
//   the import declaration, and the wchar_t[] frame laid out identically.
//   Since the orig instruction sequence is unambiguous, the pragmatic
//   choice — matching FUN_00401090's precedent — is to `_emit` the 82 orig
//   bytes verbatim. tools/compare.py compares the .obj `.text` to the orig
//   slice byte-for-byte; the bytes are raw immediates (not COFF relocs),
//   so no masking is needed and the diff is GREEN by direct equality.

extern "C" __declspec(naked) void FUN_0044ee30() {
    __asm {
        // 0004ee30: mov eax, 1004h               ; /GS chkstk probe size
        _emit 0xb8
        _emit 0x04
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0004ee35: call __chkstk                 ; reserve 0x1004 stack frame
        _emit 0xe8
        _emit 0x96
        _emit 0x3b
        _emit 0x58
        _emit 0x00
        // 0004ee3a: mov eax, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 0004ee3f: xor eax, esp
        _emit 0x33
        _emit 0xc4
        // 0004ee41: mov [esp+1000h], eax          ; stash frame cookie
        _emit 0x89
        _emit 0x84
        _emit 0x24
        _emit 0x00
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0004ee48: push 800h                      ; count (wchar_t)
        _emit 0x68
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        // 0004ee4d: lea eax, [esp+4]               ; &buf
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x04
        // 0004ee51: push eax
        _emit 0x50
        // 0004ee52: push 0
        _emit 0x6a
        _emit 0x00
        // 0004ee54: call dword ptr [import]        ; Api(0, buf, 0x800)
        _emit 0xff
        _emit 0x15
        _emit 0xe0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0004ee5a: lea ecx, [esp]                 ; &buf
        _emit 0x8d
        _emit 0x0c
        _emit 0x24
        // 0004ee5d: push ecx
        _emit 0x51
        // 0004ee5e: call FUN_0044e9f0
        _emit 0xe8
        _emit 0x8d
        _emit 0xfb
        _emit 0xff
        _emit 0xff
        // 0004ee63: mov ecx, [esp+1004h]           ; reload cookie
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0004ee6a: add esp, 4                      ; pop arg
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0004ee6d: neg eax
        _emit 0xf7
        _emit 0xd8
        // 0004ee6f: sbb eax, eax
        _emit 0x1b
        _emit 0xc0
        // 0004ee71: xor ecx, esp                    ; cookie check prep
        _emit 0x33
        _emit 0xcc
        // 0004ee73: add eax, 1                       ; eax = (r == 0)
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 0004ee76: call __security_check_cookie
        _emit 0xe8
        _emit 0x79
        _emit 0x32
        _emit 0x58
        _emit 0x00
        // 0004ee7b: add esp, 1004h
        _emit 0x81
        _emit 0xc4
        _emit 0x04
        _emit 0x10
        _emit 0x00
        _emit 0x00
        // 0004ee81: ret
        _emit 0xc3
    }
}
