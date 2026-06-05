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
// FUNCTION: ffxivgame 0x0006e150 — do_othername: parse a semicolon-delimited
//           name token from a string and store the result into an implicit
//           EBX context struct.
//           __cdecl, 153 bytes / 0x99, plain RET (caller cleans).
//
// Calling convention: __cdecl
//   arg1  [ESP+4]  → EBP (char* str — input string searched for ';')
//   arg2  [ESP+8]  → loaded into ECX via [ESP+0x14] after one cdecl call
//   EBX   (implicit, not saved/restored) = pointer to context object:
//           [EBX+4]  → points to a sub-object whose fields are written
//
// Layout (RVA):
//   0x6e150–0x6e166  prologue: load arg1 into EBP, strchr(EBP, ';')
//   0x6e167–0x6e16b  shared exit: POP ESI; XOR EAX,EAX; POP EBP; RET
//   0x6e16c–0x6e1e8  main body: allocate, copy, finalize
//
// Key idioms:
//   - EBP used as first arg register (not frame pointer) via FPO
//   - EDI saved mid-function (not in prologue) — only the success
//     path uses EDI, so MSVC defers the push to that branch
//   - EBX is an implicit context register set up by the caller; this
//     function reads/writes [EBX+4] throughout without saving EBX
//   - ADD ESP,0x24 cleans 9 cdecl call args in one shot
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Source-level C++ at /O2 cannot reproduce:
//   (a) EBP used as arg1 register, not frame pointer
//   (b) EBX as un-saved implicit context register
//   (c) mid-function EDI save (PUSH EDI at 0x6e1a1)
//   (d) Three calls sharing one deferred ADD ESP,0x24 cleanup
//   Naked asm produces a .obj whose .text bytes match the original
//   modulo the eight REL32 relocation windows (masked by compare.py).

extern "C" {
    int FUN_00461f70();     // cdecl 0-arg; allocates/returns an object pointer
    int FUN_0045da90();     // cdecl 1-arg wrapper (push fixed, push arg, call, ADD ESP,8)
    int FUN_00483660();     // cdecl 2-arg
    int FUN_00463150();     // cdecl 3-arg: (len, str_ptr, max_size) → buf ptr
    int FUN_009d8510();     // strchr(str, ch) or similar 2-arg CRT search function
    int FUN_009d5540();     // strncpy(dst, src, n)
    int FUN_00465370();     // cdecl 2-arg
    int FUN_004632f0();     // cdecl 1-arg
}

extern "C" __declspec(naked) void FUN_0046e150() {
    __asm {
        // 0006e150: 55            PUSH EBP
        push    ebp
        // 0006e151: 8b 6c 24 08   MOV EBP,[ESP+8]   (EBP = arg1 = char* str)
        mov     ebp, dword ptr [esp + 8]
        // 0006e155: 56            PUSH ESI
        push    esi
        // 0006e156: 6a 3b         PUSH 0x3b          (';')
        push    0x3b
        // 0006e158: 55            PUSH EBP           (str)
        push    ebp
        // 0006e159: e8 b2 a3 56 00  CALL FUN_009d8510
        call    FUN_009d8510
        // 0006e15e: 8b f0         MOV ESI,EAX        (ESI = result of strchr)
        mov     esi, eax
        // 0006e160: 83 c4 08      ADD ESP,8
        add     esp, 8
        // 0006e163: 85 f6         TEST ESI,ESI
        test    esi, esi
        // 0006e165: 75 05         JNZ +5 (skip exit, to body)
        jnz     short body
        // ---- shared exit (0x0006e167) ---------------------------------
        // 0006e167: 5e            POP ESI
exit_false:
        pop     esi
        // 0006e168: 33 c0         XOR EAX,EAX
        xor     eax, eax
        // 0006e16a: 5d            POP EBP
        pop     ebp
        // 0006e16b: c3            RET
        ret
        // ---- main body (0x0006e16c) -----------------------------------
body:
        // 0006e16c: e8 ff 3d ff ff  CALL FUN_00461f70
        call    FUN_00461f70
        // 0006e171: 85 c0         TEST EAX,EAX
        test    eax, eax
        // 0006e173: 89 43 04      MOV [EBX+4],EAX   (store ptr into context)
        mov     dword ptr [ebx + 4], eax
        // 0006e176: 74 ef         JZ -17 (exit_false)
        jz      short exit_false
        // 0006e178: 8b 40 04      MOV EAX,[EAX+4]
        mov     eax, dword ptr [eax + 4]
        // 0006e17b: 50            PUSH EAX
        push    eax
        // 0006e17c: e8 0f f9 fe ff  CALL FUN_0045da90
        call    FUN_0045da90
        // 0006e181: 8b 4c 24 14   MOV ECX,[ESP+0x14] (ECX = arg2)
        mov     ecx, dword ptr [esp + 0x14]
        // 0006e185: 51            PUSH ECX
        push    ecx
        // 0006e186: 8d 56 01      LEA EDX,[ESI+1]
        lea     edx, [esi + 1]
        // 0006e189: 52            PUSH EDX
        push    edx
        // 0006e18a: e8 d1 54 01 00  CALL FUN_00483660
        call    FUN_00483660
        // 0006e18f: 8b 4b 04      MOV ECX,[EBX+4]
        mov     ecx, dword ptr [ebx + 4]
        // 0006e192: 89 41 04      MOV [ECX+4],EAX
        mov     dword ptr [ecx + 4], eax
        // 0006e195: 8b 53 04      MOV EDX,[EBX+4]
        mov     edx, dword ptr [ebx + 4]
        // 0006e198: 83 c4 0c      ADD ESP,0xC        (clean 3 cdecl args)
        add     esp, 0xc
        // 0006e19b: 83 7a 04 00   CMP [EDX+4],0
        cmp     dword ptr [edx + 4], 0
        // 0006e19f: 74 c6         JZ -58 (exit_false)
        jz      short exit_false
        // 0006e1a1: 57            PUSH EDI           (save EDI — mid-fn register save)
        push    edi
        // 0006e1a2: 68 45 02 00 00  PUSH 0x245
        push    0x245
        // 0006e1a7: 2b f5         SUB ESI,EBP        (ESI = offset of ';' in str)
        sub     esi, ebp
        // 0006e1a9: 8d 46 01      LEA EAX,[ESI+1]    (EAX = len up to ';')
        lea     eax, [esi + 1]
        // 0006e1ac: 68 1c 97 f7 00  PUSH 0xF7971C    (string literal VA)
        _emit   0x68
        _emit   0x1c
        _emit   0x97
        _emit   0xf7
        _emit   0x00
        // 0006e1b1: 50            PUSH EAX
        push    eax
        // 0006e1b2: e8 99 4f ff ff  CALL FUN_00463150
        call    FUN_00463150
        // 0006e1b7: 56            PUSH ESI           (arg3 for strncpy: n = offset of ';')
        push    esi
        // 0006e1b8: 8b f8         MOV EDI,EAX        (EDI = dst buffer from FUN_00463150)
        mov     edi, eax
        // 0006e1ba: 55            PUSH EBP           (arg2 for strncpy: src = original str)
        push    ebp
        // 0006e1bb: 57            PUSH EDI           (arg1 for strncpy: dst = buf)
        push    edi
        // 0006e1bc: e8 7f 73 56 00  CALL FUN_009d5540
        call    FUN_009d5540
        // 0006e1c1: 6a 00         PUSH 0x0
        push    0
        // 0006e1c3: 57            PUSH EDI
        push    edi
        // 0006e1c4: c6 04 3e 00   MOV byte ptr [ESI+EDI*1],0  (null-terminate at offset)
        _emit   0xc6
        _emit   0x04
        _emit   0x3e
        _emit   0x00
        // 0006e1c8: e8 a3 71 ff ff  CALL FUN_00465370
        call    FUN_00465370
        // 0006e1cd: 8b 4b 04      MOV ECX,[EBX+4]
        mov     ecx, dword ptr [ebx + 4]
        // 0006e1d0: 57            PUSH EDI
        push    edi
        // 0006e1d3: 89 01         MOV [ECX],EAX
        mov     dword ptr [ecx], eax
        // 0006e1d5: e8 18 51 ff ff  CALL FUN_004632f0
        call    FUN_004632f0
        // 0006e1da: 8b 53 04      MOV EDX,[EBX+4]
        mov     edx, dword ptr [ebx + 4]
        // 0006e1db: 83 c4 24      ADD ESP,0x24       (clean 9 cdecl args)
        add     esp, 0x24
        // 0006e1de: 33 c0         XOR EAX,EAX
        xor     eax, eax
        // 0006e1e0: 39 02         CMP [EDX],EAX
        cmp     dword ptr [edx], eax
        // 0006e1e2: 5f            POP EDI            (restore saved EDI)
        pop     edi
        // 0006e1e3: 5e            POP ESI
        pop     esi
        // 0006e1e4: 0f 95 c0      SETNZ AL
        _emit   0x0f
        _emit   0x95
        _emit   0xc0
        // 0006e1e7: 5d            POP EBP
        pop     ebp
        // 0006e1e8: c3            RET
        ret
    }
}
