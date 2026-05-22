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
// FUNCTION: ffxivgame 0x0000f2d0 — assertion failure handler
//                                   (__cdecl, 166 B / 0xa6)
//
// void __cdecl FUN_0040f2d0(const char *cond, const char *msg,
//                            const char *file, int line, const char *func)
//
// Formats an assertion failure message to a 2048-byte stack buffer, calls
// the debug output function via IAT slot at [0x012651b4], then intentionally
// writes to address 0 to force an access violation (hard assert crash).
//
// Stack layout (caller's perspective, __cdecl):
//   [ESP+0x04]  cond  — condition expression string
//   [ESP+0x08]  msg   — message string
//   [ESP+0x0c]  file  — source file name
//   [ESP+0x10]  line  — source line number
//   [ESP+0x14]  func  — function name (NULL if no function context)
//
// If func != NULL:  "%s(%d):[%s] <assert> (%s) %s\n" (file, line, func, cond, msg)
// If func == NULL:  "%s(%d): <assert> (%s) %s\n"     (file, line, cond, msg)
//
// String literals (pooled by /GF):
//   0xf54d14  "%s(%d):[%s] <assert> (%s) %s\n"
//   0xf54cf8  "%s(%d): <assert> (%s) %s\n"
//
// IAT slot [0x012651b4] — debug output fn, called as fn(buf, 6)
//
// The function pushes no callee-saved registers and accesses all
// parameters via ESP-relative addressing (compiled with /Oy).
// The 2048-byte local buffer would normally trigger /GS cookie
// instrumentation, but the function was compiled without it.
//
// Calling convention: __cdecl; no callee-saves; no /GS cookie.
// Epilogue: ADD ESP, 0x808 (cleans local buffer + 2 IAT call args); RET.
//
// Reconstruction strategy — naked-asm passthrough:
//   The ESP-only addressing and absence of /GS make this function
//   not reproducible from plain C++ source under the default MSVC 2005
//   flags. The __declspec(naked) body emits the original instructions
//   via MASM mnemonics; reloc-bearing positions (CALL rel32 targets
//   and the IAT ff15 address) are declared as extern so the assembler
//   emits proper COFF relocations that compare.py masks.

extern "C" {

// The two assertion format strings live in .rdata (address-taken, /GF pooled).
// Declaring them as extern char[] causes the assembler to emit a DIR32
// relocation for each PUSH offset <sym> that the linker fills from the
// binary's load address. compare.py masks these reloc positions.
extern char g_assert_fmt_func[];   // 0x00f54d14 — "%s(%d):[%s] <assert> (%s) %s\n"
extern char g_assert_fmt_nofunc[]; // 0x00f54cf8 — "%s(%d): <assert> (%s) %s\n"

// _snprintf_s is a direct CALL (rel32) into the CRT.
// The actual target is RVA 0x009d4f9f; the rel32 values in the two
// call sites differ because the sites are at different RVAs, but both
// resolve to this symbol. Declaring it extern causes the assembler
// to emit a REL32 relocation that compare.py masks.
int __cdecl _snprintf_s(char *buf, size_t bufSize, size_t count, const char *fmt, ...);

// The debug output function is called through a function-pointer slot
// in a global data region (IAT-style indirect call: ff 15 [abs-addr]).
// Declaring it as __declspec(dllimport) forces the ff15 encoding.
__declspec(dllimport) void __cdecl g_debugOutputFn(const char *buf, int level);

}

extern "C" __declspec(naked) void FUN_0040f2d0() {
    __asm {
        // Offset 0x00:  8b 44 24 14    MOV EAX, dword ptr [ESP+0x14]
        //   Load param_5 (func) from stack before SUB ESP widens the frame.
        mov     eax, dword ptr [esp + 0x14]
        // Offset 0x04:  81 ec 00 08 00 00  SUB ESP, 0x800
        //   Reserve 2048 bytes for the format buffer (local_800).
        sub     esp, 0x800
        // Offset 0x0a:  85 c0              TEST EAX, EAX
        test    eax, eax
        // Offset 0x0c:  74 3f              JZ zero_branch
        jz      zero_branch

        // --- non-zero branch (func != NULL) ---
        // Offset 0x0e:  8b 8c 24 08 08 00 00  MOV ECX, [ESP+0x808]  (= param_2 / msg)
        mov     ecx, dword ptr [esp + 0x808]
        // Offset 0x15:  8b 94 24 04 08 00 00  MOV EDX, [ESP+0x804]  (= param_1 / cond)
        mov     edx, dword ptr [esp + 0x804]
        // Offset 0x1c:  51                    PUSH ECX              (arg: msg)
        push    ecx
        // Offset 0x1d:  8b 8c 24 10 08 00 00  MOV ECX, [ESP+0x810]  (= param_3 / file, after PUSH)
        mov     ecx, dword ptr [esp + 0x810]
        // Offset 0x24:  52                    PUSH EDX              (arg: cond)
        push    edx
        // Offset 0x25:  50                    PUSH EAX              (arg: func)
        push    eax
        // Offset 0x26:  8b 84 24 1c 08 00 00  MOV EAX, [ESP+0x81c]  (= param_4 / line, after 3x PUSH)
        mov     eax, dword ptr [esp + 0x81c]
        // Offset 0x2d:  50                    PUSH EAX              (arg: line)
        push    eax
        // Offset 0x2e:  51                    PUSH ECX              (arg: file)
        push    ecx
        // Offset 0x2f:  68 14 4d f5 00        PUSH offset g_assert_fmt_func
        push    offset g_assert_fmt_func
        // Offset 0x34:  68 ff 07 00 00        PUSH 0x7ff            (count)
        push    0x7ff
        // Offset 0x39:  8d 54 24 1c           LEA EDX, [ESP+0x1c]   (= &local_800)
        lea     edx, [esp + 0x1c]
        // Offset 0x3d:  68 00 08 00 00        PUSH 0x800            (bufSize)
        push    0x800
        // Offset 0x42:  52                    PUSH EDX              (buf = local_800)
        push    edx
        // Offset 0x43:  e8 xx xx xx xx        CALL _snprintf_s
        call    _snprintf_s
        // Offset 0x48:  83 c4 24              ADD ESP, 0x24         (clean 9 args: buf,bufSize,count,fmt,file,line,func,cond,msg)
        add     esp, 0x24
        // Offset 0x4b:  eb 3c                 JMP after_branch
        jmp     after_branch

    zero_branch:
        // Offset 0x4d:  8b 84 24 08 08 00 00  MOV EAX, [ESP+0x808]  (= param_2 / msg)
        mov     eax, dword ptr [esp + 0x808]
        // Offset 0x54:  8b 8c 24 04 08 00 00  MOV ECX, [ESP+0x804]  (= param_1 / cond)
        mov     ecx, dword ptr [esp + 0x804]
        // Offset 0x5b:  8b 94 24 10 08 00 00  MOV EDX, [ESP+0x810]  (= param_4 / line)
        mov     edx, dword ptr [esp + 0x810]
        // Offset 0x62:  50                    PUSH EAX              (arg: msg)
        push    eax
        // Offset 0x63:  8b 84 24 10 08 00 00  MOV EAX, [ESP+0x810]  (= param_3 / file, after PUSH)
        mov     eax, dword ptr [esp + 0x810]
        // Offset 0x6a:  51                    PUSH ECX              (arg: cond)
        push    ecx
        // Offset 0x6b:  52                    PUSH EDX              (arg: line)
        push    edx
        // Offset 0x6c:  50                    PUSH EAX              (arg: file)
        push    eax
        // Offset 0x6d:  68 f8 4c f5 00        PUSH offset g_assert_fmt_nofunc
        push    offset g_assert_fmt_nofunc
        // Offset 0x72:  68 ff 07 00 00        PUSH 0x7ff            (count)
        push    0x7ff
        // Offset 0x77:  8d 4c 24 18           LEA ECX, [ESP+0x18]   (= &local_800)
        lea     ecx, [esp + 0x18]
        // Offset 0x7b:  68 00 08 00 00        PUSH 0x800            (bufSize)
        push    0x800
        // Offset 0x80:  51                    PUSH ECX              (buf = local_800)
        push    ecx
        // Offset 0x81:  e8 xx xx xx xx        CALL _snprintf_s
        call    _snprintf_s
        // Offset 0x86:  83 c4 20              ADD ESP, 0x20         (clean 8 args: buf,bufSize,count,fmt,file,line,cond,msg)
        add     esp, 0x20

    after_branch:
        // Offset 0x89:  8d 14 24              LEA EDX, [ESP]        (= &local_800)
        lea     edx, [esp]
        // Offset 0x8c:  6a 06                 PUSH 0x6              (level = 6)
        push    0x6
        // Offset 0x8e:  52                    PUSH EDX              (buf = local_800)
        push    edx
        // Offset 0x8f:  ff 15 b4 51 26 01     CALL [g_debugOutputFn] (IAT-style indirect)
        call    dword ptr [g_debugOutputFn]
        // Offset 0x95:  c7 05 00 00 00 00 00 00 00 00  MOV [0x00000000], 0  (null-deref crash)
        _emit 0xc7
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // Offset 0x9f:  81 c4 08 08 00 00     ADD ESP, 0x808
        add     esp, 0x808
        // Offset 0xa5:  c3                    RET
        ret
    }
}
