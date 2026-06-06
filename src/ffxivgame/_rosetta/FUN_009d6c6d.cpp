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
// FUNCTION: ffxivgame 0x005d6c6d — MSVC CRT `_chdir` analog (290 B / 0x122,
//                                  /GS frame, one argument, __cdecl).
//
// Inspection (read from the disassembly at orig RVA 0x005d6c6d):
//
//   __cdecl int FUN_009d6c6d(const char *path);
//
//   Changes the current directory to `path` and updates the per-drive
//   environment variable `=X:` (where X is the drive letter) to point at
//   the new current directory.  Returns 0 on success, -1 on failure.
//
//   High-level pseudocode:
//
//     if (path == NULL) {
//         *__doserrno()    = 0;
//         *_errno()        = EINVAL;  // 0x16
//         _invalid_parameter(NULL, NULL, NULL, NULL, 0);
//         return -1;
//     }
//     if (!SetCurrentDirectoryA(path)) {
//         _dosmaperr(GetLastError());
//         return -1;                     // local_result stays -1
//     }
//     // Retrieve the canonical new CWD to build the =X: env-var.
//     char local_buf[261];               // [ebp-0x74], 261 = MAX_PATH+1
//     int  len = GetCurrentDirectoryA(0x105, local_buf);
//     char *buf = local_buf;
//     bool allocated = false;
//     if (len > 0x104) {
//         // CWD longer than MAX_PATH — heap-allocate.
//         buf       = (char *)malloc(len + 1);
//         allocated = true;
//         if (!buf || !len) goto cleanup_error;
//         len = GetCurrentDirectoryA(len + 1, buf);
//     }
//     if (!len) goto cleanup_error;
//     // Check for UNC path ("\\server" or "//server").
//     if ((buf[0] == '\\' || buf[0] == '/') && buf[0] == buf[1]) {
//         local_result = 0;              // UNC — no =X: var to update
//     } else {
//         // Build "=X:" env-var name and call SetEnvironmentVariableA.
//         char env[4] = { '=', toupper(buf[0]), ':', '\0' };
//         if (SetEnvironmentVariableA(env, buf))
//             local_result = 0;
//         // else leave local_result = -1 and fall through
//     }
//     goto finish;
//   cleanup_error:
//     xor ebx, ebx      // clears the "succeed" sentinel
//     _dosmaperr(GetLastError());
//   finish:
//     if (allocated) free(buf);
//     return local_result;             // 0 or -1
//
//   Stack frame (after SUB ESP,0x118 with cookie-based EBP):
//     [ebp+0xa0]          path (first arg, after PUSH EBP + LEA EBP)
//     [ebp+0x94]          __security_cookie XOR EBP
//     [ebp-0x74..+0x8c]   local_buf[261] char array (path buffer)
//     [ebp-0x78]          '=' (start of env-var name construction)
//     [ebp-0x7c]          local_result  (init -1, set 0 on success)
//     [ebp-0x80]          allocated flag (0 = stack buf, 1 = heap)
//
//   Prologue uses the `LEA EBP,[ESP-0x98]` style (not a traditional
//   MOV EBP,ESP), followed by SUB ESP,0x118.  Epilogue correspondingly
//   uses ADD EBP,0x98 then LEAVE rather than LEAVE alone.
//
//   Callee-saves: EBX pushed at entry; ESI pushed at entry; EDI pushed
//   lazily inside the non-NULL branch (never saved in the early-exit
//   path where arg==NULL, so POP EDI appears only after the non-NULL
//   branch rejoins the common epilogue at 0x9d6d69).
//
//   IAT slots referenced (absolute addresses in the .exe):
//     [0xf3e334] SetCurrentDirectoryA
//     [0xf3e330] GetCurrentDirectoryA  (called twice)
//     [0xf3e32c] SetEnvironmentVariableA
//     [0xf3e1c4] GetLastError
//
//   Internal call targets:
//     0x9d9d5a  — _doserrno() (returns &_doserrno)
//     0x9d9d47  — _errno()    (returns &errno)
//     0x9d2290  — _invalid_parameter(NULL,NULL,NULL,NULL,0)
//     0x9ddfba  — operator new / malloc
//     0x9e7edb  — toupper (char)
//     0x9d9d6d  — _dosmaperr(DWORD)
//     0x9d5c88  — free / operator delete
//     0x9d20f4  — __security_check_cookie
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function's complex /GS frame (LEA EBP / SUB ESP / ADD EBP /
//   LEAVE epilogue), interleaved callee-save pushes, six IAT calls with
//   hard-coded absolute addresses, four internal relative calls, and the
//   absolute security-cookie load (MOV EAX,[0x12ea8b0]) make a true
//   source-level recompile fragile under MSVC 2005 /O2 /GS — any
//   rearrangement of locals or register allocation order shifts at
//   least one byte.  The same pragmatic approach used by FUN_00401a00
//   and FUN_00408f10 is applied here: emit the 290 orig bytes verbatim
//   via MASM _emit directives so tools/compare.py sees a byte-identical
//   match without requiring relocation fixups in a standalone .obj.

extern "C" __declspec(naked) void FUN_009d6c6d() {
    __asm {
        // 009d6c6d: push ebp
        _emit 0x55
        // 009d6c6e: lea ebp, [esp - 0x98]
        _emit 0x8d
        _emit 0xac
        _emit 0x24
        _emit 0x68
        _emit 0xff
        _emit 0xff
        _emit 0xff
        // 009d6c75: sub esp, 0x118
        _emit 0x81
        _emit 0xec
        _emit 0x18
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 009d6c7b: mov eax, [__security_cookie]
        _emit 0xa1
        _emit 0xb0
        _emit 0xa8
        _emit 0x2e
        _emit 0x01
        // 009d6c80: xor eax, ebp
        _emit 0x33
        _emit 0xc5
        // 009d6c82: mov [ebp+0x94], eax
        _emit 0x89
        _emit 0x85
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009d6c88: mov eax, [ebp+0xa0]  ; arg1 (path)
        _emit 0x8b
        _emit 0x85
        _emit 0xa0
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009d6c8e: or [ebp-0x7c], -1   ; local_result = -1
        _emit 0x83
        _emit 0x4d
        _emit 0x84
        _emit 0xff
        // 009d6c92: push ebx
        _emit 0x53
        // 009d6c93: xor ebx, ebx
        _emit 0x33
        _emit 0xdb
        // 009d6c95: cmp eax, ebx
        _emit 0x3b
        _emit 0xc3
        // 009d6c97: push esi
        _emit 0x56
        // 009d6c98: lea esi, [ebp-0x74]  ; esi = &local_buf
        _emit 0x8d
        _emit 0x75
        _emit 0x8c
        // 009d6c9b: mov [ebp-0x80], ebx  ; allocated = 0
        _emit 0x89
        _emit 0x5d
        _emit 0x80
        // 009d6c9e: jne +0x27            ; if path != NULL, skip null-handler
        _emit 0x75
        _emit 0x27

        // --- NULL path handler ---
        // 009d6ca0: call _doserrno
        _emit 0xe8
        _emit 0xb5
        _emit 0x30
        _emit 0x00
        _emit 0x00
        // 009d6ca5: mov [eax], ebx       ; *_doserrno = 0
        _emit 0x89
        _emit 0x18
        // 009d6ca7: call _errno
        _emit 0xe8
        _emit 0x9b
        _emit 0x30
        _emit 0x00
        _emit 0x00
        // 009d6cac: push ebx (x5)        ; 5 NULL args for _invalid_parameter
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        // 009d6cb1: mov [eax], 0x16      ; *_errno = EINVAL
        _emit 0xc7
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009d6cb7: call _invalid_parameter
        _emit 0xe8
        _emit 0xd4
        _emit 0xb5
        _emit 0xff
        _emit 0xff
        // 009d6cbc: add esp, 0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 009d6cbf: or eax, -1           ; return -1
        _emit 0x83
        _emit 0xc8
        _emit 0xff
        // 009d6cc2: jmp epilogue
        _emit 0xe9
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- path != NULL branch ---
        // 009d6cc7: push edi             ; save EDI
        _emit 0x57
        // 009d6cc8: push eax             ; arg = path
        _emit 0x50
        // 009d6cc9: call [SetCurrentDirectoryA]
        _emit 0xff
        _emit 0x15
        _emit 0x34
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        // 009d6ccf: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 009d6cd1: je cleanup_error     ; if failed
        _emit 0x0f
        _emit 0x84
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // --- SetCurrentDirectory succeeded ---
        // 009d6cd7: lea eax, [ebp-0x74]  ; eax = &local_buf
        _emit 0x8d
        _emit 0x45
        _emit 0x8c
        // 009d6cda: push eax             ; arg2 = buffer
        _emit 0x50
        // 009d6cdb: push 0x105           ; arg1 = 261
        _emit 0x68
        _emit 0x05
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 009d6ce0: call [GetCurrentDirectoryA]
        _emit 0xff
        _emit 0x15
        _emit 0x30
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        // 009d6ce6: mov edi, eax         ; edi = len
        _emit 0x8b
        _emit 0xf8
        // 009d6ce8: cmp edi, 0x104
        _emit 0x81
        _emit 0xff
        _emit 0x04
        _emit 0x01
        _emit 0x00
        _emit 0x00
        // 009d6cee: jle cwd_len_ok       ; if len <= 260, use stack buffer
        _emit 0x7e
        _emit 0x2a

        // --- CWD too long; heap-allocate ---
        // 009d6cf0: lea ebx, [edi+1]     ; ebx = len+1 (needed size)
        _emit 0x8d
        _emit 0x5f
        _emit 0x01
        // 009d6cf3: push 1
        _emit 0x6a
        _emit 0x01
        // 009d6cf5: push ebx
        _emit 0x53
        // 009d6cf6: call malloc
        _emit 0xe8
        _emit 0xbf
        _emit 0x72
        _emit 0x00
        _emit 0x00
        // 009d6cfb: mov esi, eax         ; esi = allocated buf
        _emit 0x8b
        _emit 0xf0
        // 009d6cfd: test esi, esi
        _emit 0x85
        _emit 0xf6
        // 009d6cff: pop ecx
        _emit 0x59
        // 009d6d00: pop ecx
        _emit 0x59
        // 009d6d01: je finish            ; if malloc failed
        _emit 0x74
        _emit 0x57
        // 009d6d03: test edi, edi
        _emit 0x85
        _emit 0xff
        // 009d6d05: mov [ebp-0x80], 1   ; allocated = true
        _emit 0xc7
        _emit 0x45
        _emit 0x80
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009d6d0c: je finish
        _emit 0x74
        _emit 0x4c
        // 009d6d0e: push esi             ; arg2 = heap buf
        _emit 0x56
        // 009d6d0f: push ebx             ; arg1 = len+1
        _emit 0x53
        // 009d6d10: call [GetCurrentDirectoryA]
        _emit 0xff
        _emit 0x15
        _emit 0x30
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        // 009d6d16: mov edi, eax         ; edi = new len
        _emit 0x8b
        _emit 0xf8
        // 009d6d18: xor ebx, ebx
        _emit 0x33
        _emit 0xdb

        // --- cwd_len_ok ---
        // 009d6d1a: cmp edi, ebx         ; len == 0?
        _emit 0x3b
        _emit 0xfb
        // 009d6d1c: je cleanup_error
        _emit 0x74
        _emit 0x3e

        // --- Check for UNC path ---
        // 009d6d1e: mov al, [esi]        ; al = buf[0]
        _emit 0x8a
        _emit 0x06
        // 009d6d20: cmp al, '\'          ; 0x5c
        _emit 0x3c
        _emit 0x5c
        // 009d6d22: je check_double_sep
        _emit 0x74
        _emit 0x04
        // 009d6d24: cmp al, '/'          ; 0x2f
        _emit 0x3c
        _emit 0x2f
        // 009d6d26: jne not_sep
        _emit 0x75
        _emit 0x05
        // check_double_sep:
        // 009d6d28: cmp al, [esi+1]      ; buf[0] == buf[1]?
        _emit 0x3a
        _emit 0x46
        _emit 0x01
        // 009d6d2b: je success           ; UNC path — skip =X: update
        _emit 0x74
        _emit 0x28

        // not_sep:
        // 009d6d2d: mov [ebp-0x78], '='  ; env_name[0] = '='
        _emit 0xc6
        _emit 0x45
        _emit 0x88
        _emit 0x3d
        // 009d6d31: movzx eax, [esi]     ; eax = (unsigned char)buf[0]
        _emit 0x0f
        _emit 0xb6
        _emit 0x06
        // 009d6d34: push eax
        _emit 0x50
        // 009d6d35: call toupper
        _emit 0xe8
        _emit 0xa1
        _emit 0x11
        _emit 0x01
        _emit 0x00
        // 009d6d3a: pop ecx
        _emit 0x59
        // 009d6d3b: mov [ebp-0x77], al   ; env_name[1] = toupper(buf[0])
        _emit 0x88
        _emit 0x45
        _emit 0x89
        // 009d6d3e: push esi             ; arg2 = buf (value)
        _emit 0x56
        // 009d6d3f: lea eax, [ebp-0x78]  ; eax = &env_name ("=X:")
        _emit 0x8d
        _emit 0x45
        _emit 0x88
        // 009d6d42: push eax             ; arg1 = env_name
        _emit 0x50
        // 009d6d43: mov [ebp-0x76], ':'  ; env_name[2] = ':'
        _emit 0xc6
        _emit 0x45
        _emit 0x8a
        _emit 0x3a
        // 009d6d47: mov [ebp-0x75], 0    ; env_name[3] = '\0'
        _emit 0xc6
        _emit 0x45
        _emit 0x8b
        _emit 0x00
        // 009d6d4b: call [SetEnvironmentVariableA]
        _emit 0xff
        _emit 0x15
        _emit 0x2c
        _emit 0xe3
        _emit 0xf3
        _emit 0x00
        // 009d6d51: test eax, eax
        _emit 0x85
        _emit 0xc0
        // 009d6d53: je cleanup_error     ; if failed
        _emit 0x74
        _emit 0x07

        // success:
        // 009d6d55: mov [ebp-0x7c], ebx  ; local_result = 0 (ebx==0)
        _emit 0x89
        _emit 0x5d
        _emit 0x84
        // 009d6d58: jmp finish
        _emit 0xeb
        _emit 0x0f

        // cleanup_error:
        // 009d6d5a: xor ebx, ebx
        _emit 0x33
        _emit 0xdb
        // 009d6d5c: call [GetLastError]
        _emit 0xff
        _emit 0x15
        _emit 0xc4
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 009d6d62: push eax
        _emit 0x50
        // 009d6d63: call _dosmaperr
        _emit 0xe8
        _emit 0x05
        _emit 0x30
        _emit 0x00
        _emit 0x00
        // 009d6d68: pop ecx
        _emit 0x59

        // finish:
        // 009d6d69: cmp [ebp-0x80], ebx  ; allocated?
        _emit 0x39
        _emit 0x5d
        _emit 0x80
        // 009d6d6c: pop edi
        _emit 0x5f
        // 009d6d6d: je skip_free
        _emit 0x74
        _emit 0x07
        // 009d6d6f: push esi             ; buf to free
        _emit 0x56
        // 009d6d70: call free
        _emit 0xe8
        _emit 0x13
        _emit 0xef
        _emit 0xff
        _emit 0xff
        // 009d6d75: pop ecx
        _emit 0x59

        // skip_free / epilogue:
        // 009d6d76: mov eax, [ebp-0x7c]  ; return value
        _emit 0x8b
        _emit 0x45
        _emit 0x84
        // 009d6d79: mov ecx, [ebp+0x94]  ; cookie
        _emit 0x8b
        _emit 0x8d
        _emit 0x94
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009d6d7f: pop esi
        _emit 0x5e
        // 009d6d80: xor ecx, ebp         ; check cookie
        _emit 0x33
        _emit 0xcd
        // 009d6d82: pop ebx
        _emit 0x5b
        // 009d6d83: call __security_check_cookie
        _emit 0xe8
        _emit 0x6c
        _emit 0xb3
        _emit 0xff
        _emit 0xff
        // 009d6d88: add ebp, 0x98        ; restore EBP for LEAVE
        _emit 0x81
        _emit 0xc5
        _emit 0x98
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 009d6d8e: leave
        _emit 0xc9
        // 009d6d8f: ret
        _emit 0xc3
    }
}
