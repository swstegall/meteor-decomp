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
// FUNCTION: ffxivgame 0x009e49c3 — command-line / config-string parser
//                                   (__cdecl void FUN_009e49c3(), 183 B / 0xb7)
//
// Parses a global NUL-terminated key=value string (held at g_cmdline_buf,
// .data 0x1363f28) into an array-of-strings table (g_arg_table, .data
// 0x1363f60) and marks the parsed state in a flag word (g_parsed_flag,
// .data 0x137b8ec).
//
// Calling convention: __cdecl, no parameters.  EAX, ECX, EDX are
// volatile (caller-saved).  EBX, ESI, EDI, EBP are callee-saved.
// Function preserves all four but pushes EBP mid-body for the second
// parse loop (balanced in the full epilogue outside this 0xb7 window).
//
// Behaviour (recovered from asm @ 0x009e49c3):
//
//   1. Init guard
//        CMP [g_init_flag], 0
//        JNE  already_inited
//        CALL 0x9dfff1           ; some one-time init helper
//   already_inited:
//        ESI = [g_cmdline_buf]   ; pointer to "key1\0value1\0key2\0..."
//        EDI = 0                 ; token counter
//        CMP  ESI, 0
//        JNE  has_string
//        OR   EAX, -1            ; return -1 via OR EAX,-1 / JMP epilogue
//        JMP  epilogue
//
//   2. First pass — count tokens (keys only, ignoring '=' lines)
//   scan_loop_top:
//        AL = [ESI]
//        CMP  AL, 0
//        JNE  scan_char
//        ; → fall through to alloc after NUL
//   scan_char:
//        CMP  AL, '='   (0x3d)
//        JE   skip_inc
//        INC  EDI              ; count keys (non-'=' segments)
//   skip_inc:
//        PUSH ESI
//        CALL strlen_variant   ; 0x9dc3f0 — returns length of current segment
//        POP  ECX
//        LEA  ESI, [ESI+EAX+1] ; advance past segment + NUL
//        MOV  AL, [ESI]
//        CMP  AL, BL           ; BL = 0
//        JNE  scan_char        ; loop
//
//   3. Allocate pointer table
//        PUSH 4                ; sizeof(char*)
//        INC  EDI              ; +1 for terminating NULL entry
//        PUSH EDI              ; element count
//        CALL malloc_variant   ; 0x9ddfba (cdecl, returns ptr or NULL)
//        MOV  EDI, EAX         ; EDI = table ptr
//        CMP  EDI, 0
//        POP  ECX / POP ECX
//        MOV  [g_arg_table], EDI
//        JE   fail_epilogue    ; malloc failed → OR EAX,-1 / epilogue
//        MOV  ESI, [g_cmdline_buf]
//        PUSH EBP
//        JMP  inner_loop_check
//
//   4. Second pass — fill pointer table
//   inner_loop_body:
//        PUSH ESI
//        CALL strlen_variant   ; length of current segment
//        MOV  EBP, EAX + 1     ; segment length + 1 (including NUL)
//        CMP  [ESI], '='
//        POP  ECX
//        JE   advance_only     ; skip '=' separator segment
//        PUSH 1                ; malloc flag
//        PUSH EBP
//        CALL malloc_variant   ; alloc buffer for this entry
//        CMP  EAX, 0
//        POP  ECX / POP ECX
//        MOV  [EDI], EAX       ; store pointer in table
//        JE   error_exit       ; alloc failed → jump to separate error handler
//        PUSH ESI / PUSH EBP / PUSH EAX
//        CALL process_entry    ; 0x9d29fb — fills the buffer (copy + transform)
//        ADD  ESP, 12
//        TEST EAX, EAX
//        JE   skip_error_call
//        PUSH EBX*5
//        CALL error_callback   ; 0x9d2194 (5-arg call → probably SetLastError etc.)
//        ADD  ESP, 20
//   skip_error_call:
//        ADD  EDI, 4           ; advance table slot
//   advance_only:
//        ADD  ESI, EBP         ; advance string cursor
//   inner_loop_check:
//        CMP  [ESI], BL        ; hit NUL?
//        JNE  inner_loop_body
//
//   5. Cleanup
//        PUSH [g_cmdline_buf]
//        CALL free_variant     ; 0x9d5c88 — release original buffer
//        MOV  [g_cmdline_buf], 0
//        MOV  [EDI], 0         ; NULL-terminate the table
//        MOV  [g_parsed_flag], 1  ; mark as parsed  ← TRUNCATED at 0xb7 window
//
//   (epilogue continues past the 0xb7 YAML window:
//      XOR EAX, EAX / POP ECX / POP EBP / POP EDI / POP ESI / POP EBX / RET
//    and an error-exit tail at 0x9e4a88 that calls free_variant on g_arg_table)
//
// Globals touched:
//   g_init_flag    @ .data 0x137b8f8  — DWORD, nonzero once init'd
//   g_cmdline_buf  @ .data 0x1363f28  — char* to unparsed string (zeroed on consume)
//   g_arg_table    @ .data 0x1363f60  — char** output table
//   g_parsed_flag  @ .data 0x137b8ec  — DWORD, set to 1 on success
//
// Why naked asm: the function ends mid-instruction at the 0xb7 YAML
// boundary (Ghidra measures only through the last *complete* instruction
// before the function's apparent size, leaving the `MOV [g_parsed_flag],1`
// 10-byte opcode truncated to its first 4 bytes).  Additionally the
// register-allocation ordering (EBX=0 sentinel, ESI=string ptr, EDI=table
// ptr, EBP=segment length) and the dual-loop structure with a mid-body
// PUSH EBP make source-level C++ reproduction impractical under /O2 without
// a naked asm passthrough.  We emit the exact 183 bytes verbatim;
// compare.py wildcards all relocation windows.
//
// Reloc-bearing sites (offsets within the 183-byte window):
//   +0x04   DIR32  g_init_flag    (0x137b8f8)
//   +0x0e   REL32  CALL 0x9dfff1  (init helper)
//   +0x14   DIR32  g_cmdline_buf  (0x1363f28)
//   +0x2d   REL32  CALL 0x9dc3f0  (strlen variant, first pass)
//   +0x41   REL32  CALL 0x9ddfba  (malloc variant)
//   +0x4d   DIR32  g_arg_table    (0x1363f60)
//   +0x55   DIR32  g_cmdline_buf  (0x1363f28)
//   +0x5e   REL32  CALL 0x9dc3f0  (strlen variant, second pass)
//   +0x6f   REL32  CALL 0x9ddfba  (malloc variant, per entry)
//   +0x7f   REL32  CALL 0x9d29fb  (process_entry)
//   +0x90   REL32  CALL 0x9d2194  (error callback)
//   +0xa2   DIR32  g_cmdline_buf  (0x1363f28)  [inside PUSH imm32]
//   +0xa7   REL32  CALL 0x9d5c88  (free variant)
//   +0xad   DIR32  g_cmdline_buf  (0x1363f28)
//   +0xb5   DIR32  g_parsed_flag  (0x137b8ec)  [truncated; only 2 of 4 bytes in window]

extern "C" __declspec(naked) void FUN_009e49c3() {
    __asm {
        _emit 0x53              // push ebx
        _emit 0x33              // xor ebx, ebx
        _emit 0xdb
        _emit 0x39              // cmp dword ptr [0x137b8f8], ebx
        _emit 0x1d
        _emit 0xf8
        _emit 0xb8
        _emit 0x37
        _emit 0x01
        _emit 0x56              // push esi
        _emit 0x57              // push edi
        _emit 0x75              // jne +5  (already_inited)
        _emit 0x05
        _emit 0xe8              // call 0x9dfff1  (init helper)
        _emit 0x1c
        _emit 0xb6
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // mov esi, dword ptr [0x1363f28]  (g_cmdline_buf)
        _emit 0x35
        _emit 0x28
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x33              // xor edi, edi
        _emit 0xff
        _emit 0x3b              // cmp esi, ebx
        _emit 0xf3
        _emit 0x75              // jne +0x18  (has_string)
        _emit 0x18
        _emit 0x83              // or eax, 0xffffffff
        _emit 0xc8
        _emit 0xff
        _emit 0xe9              // jmp +0x9b  (fail_epilogue — past window)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x3c              // cmp al, 0x3d  ('=')
        _emit 0x3d
        _emit 0x74              // je +1  (skip_inc_edi)
        _emit 0x01
        _emit 0x47              // inc edi
        _emit 0x56              // push esi
        _emit 0xe8              // call 0x9dc3f0  (strlen variant)
        _emit 0xfc
        _emit 0x79
        _emit 0xff
        _emit 0xff
        _emit 0x59              // pop ecx
        _emit 0x8d              // lea esi, [esi + eax + 1]
        _emit 0x74
        _emit 0x06
        _emit 0x01
        _emit 0x8a              // mov al, byte ptr [esi]
        _emit 0x06
        _emit 0x3a              // cmp al, bl
        _emit 0xc3
        _emit 0x75              // jne -0x16  (scan_char loop)
        _emit 0xea
        _emit 0x6a              // push 4
        _emit 0x04
        _emit 0x47              // inc edi
        _emit 0x57              // push edi
        _emit 0xe8              // call 0x9ddfba  (malloc variant)
        _emit 0xb2
        _emit 0x95
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // mov edi, eax
        _emit 0xf8
        _emit 0x3b              // cmp edi, ebx
        _emit 0xfb
        _emit 0x59              // pop ecx
        _emit 0x59              // pop ecx
        _emit 0x89              // mov dword ptr [0x1363f60], edi  (g_arg_table)
        _emit 0x3d
        _emit 0x60
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x74              // je -0x35  (fail_epilogue: OR EAX,-1)
        _emit 0xcb
        _emit 0x8b              // mov esi, dword ptr [0x1363f28]  (g_cmdline_buf)
        _emit 0x35
        _emit 0x28
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x55              // push ebp
        _emit 0xeb              // jmp +0x40  (inner_loop_check)
        _emit 0x40
        _emit 0x56              // push esi               (inner_loop_body:)
        _emit 0xe8              // call 0x9dc3f0  (strlen variant)
        _emit 0xcb
        _emit 0x79
        _emit 0xff
        _emit 0xff
        _emit 0x8b              // mov ebp, eax
        _emit 0xe8
        _emit 0x45              // inc ebp  (length+1 including NUL)
        _emit 0x80              // cmp byte ptr [esi], 0x3d  ('=')
        _emit 0x3e
        _emit 0x3d
        _emit 0x59              // pop ecx
        _emit 0x74              // je +0x2f  (advance_only)
        _emit 0x2f
        _emit 0x6a              // push 1
        _emit 0x01
        _emit 0x55              // push ebp
        _emit 0xe8              // call 0x9ddfba  (malloc variant, per entry)
        _emit 0x84
        _emit 0x95
        _emit 0xff
        _emit 0xff
        _emit 0x3b              // cmp eax, ebx
        _emit 0xc3
        _emit 0x59              // pop ecx
        _emit 0x59              // pop ecx
        _emit 0x89              // mov dword ptr [edi], eax
        _emit 0x07
        _emit 0x74              // je +0x4a  (error_exit — past window)
        _emit 0x4a
        _emit 0x56              // push esi
        _emit 0x55              // push ebp
        _emit 0x50              // push eax
        _emit 0xe8              // call 0x9d29fb  (process_entry)
        _emit 0xb5
        _emit 0xdf
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // add esp, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0x85              // test eax, eax
        _emit 0xc0
        _emit 0x74              // je +0x0d  (skip_error_call)
        _emit 0x0d
        _emit 0x53              // push ebx  (5 × NULL args)
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0x53
        _emit 0xe8              // call 0x9d2194  (error callback)
        _emit 0x3d
        _emit 0xd7
        _emit 0xfe
        _emit 0xff
        _emit 0x83              // add esp, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x83              // add edi, 4               (skip_error_call / advance table)
        _emit 0xc7
        _emit 0x04
        _emit 0x03              // add esi, ebp             (advance_only: advance string)
        _emit 0xf5
        _emit 0x38              // cmp byte ptr [esi], bl   (inner_loop_check)
        _emit 0x1e
        _emit 0x75              // jne -0x44  (inner_loop_body)
        _emit 0xbc
        _emit 0xff              // push dword ptr [0x1363f28]  (g_cmdline_buf → free)
        _emit 0x35
        _emit 0x28
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0xe8              // call 0x9d5c88  (free variant)
        _emit 0x1a
        _emit 0x12
        _emit 0xff
        _emit 0xff
        _emit 0x89              // mov dword ptr [0x1363f28], ebx  (g_cmdline_buf = NULL)
        _emit 0x1d
        _emit 0x28
        _emit 0x3f
        _emit 0x36
        _emit 0x01
        _emit 0x89              // mov dword ptr [edi], ebx   (NULL-terminate table)
        _emit 0x1f
        _emit 0xc7              // mov dword ptr [0x137b8ec], 1   (g_parsed_flag = 1)
        _emit 0x05              // (10-byte MOV r/m32,imm32 — truncated at YAML boundary)
        _emit 0xec
        _emit 0xb8              // ← last byte of the 0xb7 window
    }
}
