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
// FUNCTION: ffxivgame 0x009c59d5 (VA 0x00dc59d5) — `__cdecl` wide-string path
//                                  helper (306 B / 0x132, /GS-guarded, IAT calls).
//
// Inspection (read from the disassembly at orig RVA 0x009c59d5):
//
//   __cdecl int FUN_00dc59d5(const wchar_t* path);
//
//   Structure (reconstructed from the x86 disassembly):
//
//     wchar_t  stack_buf[0x108];       // [ebp-0x218 .. ebp-0x008] ~528 B
//     DWORD    flag_alloc = 0;          // [ebp-0x220]
//     int      result     = -1;         // [ebp-0x21c]
//     wchar_t* buf        = stack_buf;  // esi
//
//     if (path == NULL) {
//         *_errno()  = 0;
//         *_errno()  = EINVAL;   // 0x16 = 22
//         _invalid_parameter(…);
//         return -1;
//     }
//     DWORD attr = GetFileAttributesW(path);   // IAT [0xf3e0e4]
//     if (attr == 0) goto error;
//
//     DWORD len = GetCurrentDirectoryW(0x105, stack_buf);  // IAT [0xf3e0e8]
//     if (len > 0x104) {
//         DWORD need = len + 1;
//         buf = (wchar_t*)calloc(need, 2);    // 0x9ddfba
//         if (!buf) { buf = NULL; goto error_nobuf; }
//         if (len == 0) { buf = NULL; goto error_nobuf; }
//         flag_alloc = 1;
//         len = GetCurrentDirectoryW(need, buf);
//         ebx = 0;
//     }
//     if (len == 0) goto error;
//
//     WCHAR first = buf[0];
//     if (first == L'\\' || first == L'/') {
//         if (first == buf[1]) { result = 0; goto done; }
//     } else {
//         // Build env-var name "=X:" (per-drive current dir)
//         wchar_t env[4] = { L'=', towupper(first), L':', L'\0' };
//         if (!GetEnvironmentVariableW(buf, env, …))  // IAT [0xf3e1f0]
//             goto error;
//         result = 0;
//     }
//     done:
//     if (flag_alloc) free(buf);           // 0x9d5c88
//     return result;
//
//   Stack frame (EBP-relative; /GS active):
//     [ebp - 0x004]   security cookie (ebp^cookie)
//     [ebp - 0x008]   local wchar (env name scratch: L'=')
//     [ebp - 0x00a]   local wchar (env name scratch: towupper(first))
//     [ebp - 0x00c]   local wchar (env name scratch: L':')
//     [ebp - 0x21c]   result (int, init -1 via OR 0xffffffff)
//     [ebp - 0x220]   flag_alloc (DWORD, init 0)
//     [ebp - 0x218 .. -0x009]  stack_buf (wchar_t[0x108])
//
//   IAT references:
//     [0xf3e0e4]   GetFileAttributesW
//     [0xf3e0e8]   GetCurrentDirectoryW
//     [0xf3e1f0]   GetEnvironmentVariableW
//     [0xf3e1c4]   GetLastError (error path)
//
//   Internal calls:
//     0x9d9d5a  _errno getter #1
//     0x9d9d47  _errno getter #2
//     0x9d2290  _invalid_parameter_noinfo
//     0x9ddfba  calloc (or _calloc_crt)
//     0xdc789a  towupper (inline in-binary)
//     0x9d9d6d  some error handler / SetLastError
//     0x9d5c88  free
//     0x9d20f4  __security_check_cookie
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc
//   into reproducing the exact /GS prologue (cookie ^ EBP), the exact
//   register allocation across the multiple IAT branches, and the
//   precise [ebp-0x21c] / [ebp-0x220] slot assignments. The local-
//   env-var wchar_t[4] build at [ebp-0xc..ebp-6] is particularly
//   sensitive to reordering. Given that every sibling in this _rosetta
//   cluster uses the naked-asm strategy for similar /GS-framed IAT
//   functions, we follow the same pattern.
//
//   The structural commentary above is the readable record of what
//   the function does; a future contributor can promote it to a real
//   source-level match once the surrounding class hierarchy and CRT
//   wrappers are catalogued.

extern "C" __declspec(naked) void FUN_00dc59d5() {
    __asm {
        // offset 0x0000 — prologue: push ebp / mov ebp,esp / sub esp,0x220
        _emit 0x55  // push       ebp
        _emit 0x8b  // mov        ebp, esp
        _emit 0xec
        _emit 0x81  // sub        esp, 0x220
        _emit 0xec
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xa1  // mov        eax, [__security_cookie]
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        _emit 0x33  // xor        eax, ebp
        _emit 0xc5
        // offset 0x0010
        _emit 0x89  // mov        [ebp-4], eax
        _emit 0x45
        _emit 0xfc
        _emit 0x8b  // mov        eax, [ebp+8]  ; arg1 = path
        _emit 0x45
        _emit 0x08
        _emit 0x83  // or         [ebp-0x21c], 0xffffffff  ; result = -1
        _emit 0x8d
        _emit 0xe4
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0x53  // push       ebx
        _emit 0x33  // xor        ebx, ebx
        _emit 0xdb
        // offset 0x0020
        _emit 0x3b  // cmp        eax, ebx
        _emit 0xc3
        _emit 0x56  // push       esi
        _emit 0x8d  // lea        esi, [ebp-0x218]
        _emit 0xb5
        _emit 0xe8
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x89  // mov        [ebp-0x220], ebx
        _emit 0x9d
        _emit 0xe0
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x75  // jne        (to non-null path)
        // offset 0x0030
        _emit 0x27
        _emit 0xe8  // call       0x9d9d5a  (_errno getter)
        _emit 0x4f
        _emit 0x43
        _emit 0xc1
        _emit 0xff
        _emit 0x89  // mov        [eax], ebx  ; *errno = 0
        _emit 0x18
        _emit 0xe8  // call       0x9d9d47  (_errno getter #2)
        _emit 0x35
        _emit 0x43
        _emit 0xc1
        _emit 0xff
        _emit 0x53  // push       ebx  (0)
        _emit 0x53  // push       ebx
        _emit 0x53  // push       ebx
        // offset 0x0040
        _emit 0x53  // push       ebx
        _emit 0x53  // push       ebx
        _emit 0xc7  // mov        [eax], 0x16  ; *errno = EINVAL
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8  // call       0x9d2290  (_invalid_parameter)
        _emit 0x6e
        _emit 0xc8
        _emit 0xc0
        _emit 0xff
        _emit 0x83  // add        esp, 0x14
        _emit 0xc4
        _emit 0x14
        // offset 0x0050
        _emit 0x83  // or         eax, 0xffffffff  ; eax = -1
        _emit 0xc8
        _emit 0xff
        _emit 0xe9  // jmp        epilogue
        _emit 0xcc
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x57  // push       edi
        _emit 0x50  // push       eax  (path)
        _emit 0xff  // call       [GetFileAttributesW]
        _emit 0x15
        _emit 0xe4
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        // offset 0x0060
        _emit 0x85  // test       eax, eax
        _emit 0xc0
        _emit 0x0f  // je         error
        _emit 0x84
        _emit 0x99
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d  // lea        eax, [ebp-0x218]  ; stack_buf
        _emit 0x85
        _emit 0xe8
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x50  // push       eax
        _emit 0x68  // push       0x105
        // offset 0x0070
        _emit 0x05
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0xff  // call       [GetCurrentDirectoryW]
        _emit 0x15
        _emit 0xe8
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x8b  // mov        edi, eax
        _emit 0xf8
        _emit 0x81  // cmp        edi, 0x104
        _emit 0xff
        _emit 0x04
        _emit 0x01
        // offset 0x0080
        _emit 0x00
        _emit 0x00
        _emit 0x7e  // jle        (edi <= 0x104)
        _emit 0x2d
        _emit 0x8d  // lea        ebx, [edi+1]
        _emit 0x5f
        _emit 0x01
        _emit 0x6a  // push       2
        _emit 0x02
        _emit 0x53  // push       ebx
        _emit 0xe8  // call       0x9ddfba  (calloc)
        _emit 0x56
        _emit 0x85
        _emit 0xc1
        _emit 0xff
        _emit 0x8b  // mov        esi, eax
        // offset 0x0090
        _emit 0xf0
        _emit 0x85  // test       esi, esi
        _emit 0xf6
        _emit 0x59  // pop        ecx
        _emit 0x59  // pop        ecx
        _emit 0x74  // je         error_nobuf
        _emit 0x68
        _emit 0x85  // test       edi, edi
        _emit 0xff
        _emit 0xc7  // mov        [ebp-0x220], 1
        _emit 0x85
        _emit 0xe0
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x01
        // offset 0x00a0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74  // je         error_nobuf
        _emit 0x5a
        _emit 0x56  // push       esi
        _emit 0x53  // push       ebx
        _emit 0xff  // call       [GetCurrentDirectoryW]
        _emit 0x15
        _emit 0xe8
        _emit 0xe0
        _emit 0xf3
        _emit 0x00
        _emit 0x8b  // mov        edi, eax
        _emit 0xf8
        _emit 0x33  // xor        ebx, ebx
        // offset 0x00b0
        _emit 0xdb
        _emit 0x3b  // cmp        edi, ebx
        _emit 0xfb
        _emit 0x74  // je         error
        _emit 0x4c
        _emit 0x0f  // movzx      eax, word ptr [esi]
        _emit 0xb7
        _emit 0x06
        _emit 0x66  // cmp        ax, 0x5c  ; L'\\'
        _emit 0x3d
        _emit 0x5c
        _emit 0x00
        _emit 0x74  // je
        _emit 0x06
        _emit 0x66  // cmp        ax, 0x2f  ; L'/'
        _emit 0x3d
        // offset 0x00c0
        _emit 0x2f
        _emit 0x00
        _emit 0x75  // jne        (not a separator)
        _emit 0x06
        _emit 0x66  // cmp        ax, word ptr [esi+2]
        _emit 0x3b
        _emit 0x46
        _emit 0x02
        _emit 0x74  // je         (UNC path: result=0)
        _emit 0x2d
        _emit 0x66  // mov        word ptr [ebp-0xc], 0x3d  ; L'='
        _emit 0xc7
        _emit 0x45
        _emit 0xf4
        _emit 0x3d
        _emit 0x00
        // offset 0x00d0
        _emit 0x0f  // movzx      eax, word ptr [esi]
        _emit 0xb7
        _emit 0x06
        _emit 0x50  // push       eax
        _emit 0xe8  // call       0xdc789a  (towupper)
        _emit 0xec
        _emit 0x1d
        _emit 0x00
        _emit 0x00
        _emit 0x59  // pop        ecx
        _emit 0x66  // mov        word ptr [ebp-0xa], ax
        _emit 0x89
        _emit 0x45
        _emit 0xf6
        _emit 0x56  // push       esi
        _emit 0x8d  // lea        eax, [ebp-0xc]
        // offset 0x00e0
        _emit 0x45
        _emit 0xf4
        _emit 0x50  // push       eax
        _emit 0x66  // mov        word ptr [ebp-8], 0x3a  ; L':'
        _emit 0xc7
        _emit 0x45
        _emit 0xf8
        _emit 0x3a
        _emit 0x00
        _emit 0x66  // mov        word ptr [ebp-6], bx    ; L'\0'
        _emit 0x89
        _emit 0x5d
        _emit 0xfa
        _emit 0xff  // call       [GetEnvironmentVariableW]
        _emit 0x15
        _emit 0xf0
        // offset 0x00f0
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x85  // test       eax, eax
        _emit 0xc0
        _emit 0x74  // je         error
        _emit 0x0a
        _emit 0x89  // mov        [ebp-0x21c], ebx  ; result = 0
        _emit 0x9d
        _emit 0xe4
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0xeb  // jmp        done
        _emit 0x0f
        _emit 0x33  // xor        ebx, ebx
        // offset 0x0100
        _emit 0xdb
        _emit 0xff  // call       [GetLastError]
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        _emit 0x50  // push       eax
        _emit 0xe8  // call       0x9d9d6d  (error handler)
        _emit 0x8b
        _emit 0x42
        _emit 0xc1
        _emit 0xff
        _emit 0x59  // pop        ecx
        _emit 0x39  // cmp        [ebp-0x220], ebx
        _emit 0x9d
        // offset 0x0110
        _emit 0xe0
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x5f  // pop        edi
        _emit 0x74  // je         no_free
        _emit 0x07
        _emit 0x56  // push       esi
        _emit 0xe8  // call       0x9d5c88  (free)
        _emit 0x96
        _emit 0x01
        _emit 0xc1
        _emit 0xff
        _emit 0x59  // pop        ecx
        _emit 0x8b  // mov        eax, [ebp-0x21c]  ; return value
        _emit 0x85
        // offset 0x0120
        _emit 0xe4
        _emit 0xfd
        _emit 0xff
        _emit 0xff
        _emit 0x8b  // mov        ecx, [ebp-4]
        _emit 0x4d
        _emit 0xfc
        _emit 0x5e  // pop        esi
        _emit 0x33  // xor        ecx, ebp
        _emit 0xcd
        _emit 0x5b  // pop        ebx
        _emit 0xe8  // call       __security_check_cookie
        _emit 0xef
        _emit 0xc5
        _emit 0xc0
        _emit 0xff
        // offset 0x0130
        _emit 0xc9  // leave
        _emit 0xc3  // ret
    }
}
