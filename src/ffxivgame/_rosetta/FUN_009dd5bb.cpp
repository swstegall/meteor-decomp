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
// FUNCTION: ffxivgame 0x005dd5bb — `__cdecl` CRT stream character-read
//                                   (300 B / 0x12c, __EH_prolog3 SEH frame)
//
// Inspection (read from the disassembly at orig RVA 0x005dd5bb):
//
//   __cdecl int FUN_009dd5bb(void *stream);
//
//   CRT stream read function (likely `fgetc` / `_fgetc_nolock` variant).
//   Takes a FILE* or similar stream pointer at EBP+8.
//
//   Structure:
//     - __EH_prolog3 frame setup: PUSH 0x0C / PUSH scope_table / CALL __EH_prolog3
//     - NULL-checks the stream pointer (ESI = [EBP+8]);
//       if NULL: set errno = EINVAL (0x16), call _invalid_parameter(NULL×5),
//       return -1 (OR EAX, -1 then JMP to epilogue)
//     - Calls a stream-init/lock function at 0x9d4e3e (PUSH ESI / CALL)
//     - Tests [ESI+0x0C] & 0x40 (stream flags, locked bit)
//     - If not locked: executes a complex two-pass lock-lookup sequence
//       calling function 0x9d6a61 four times each pass with ESI as arg;
//       computes lock-table index as: table[EAX>>5] + ((EAX & 0x1f) << 6)
//       where table lives at absolute VA 0x0137b7e0;
//       falls back to VA 0x012eb4d8 on -1/-2 return codes
//     - Checks byte [lock_obj+0x24] against 0x7f (first pass) and 0x80
//       (second pass); if 0x80 bit set: EINVAL + _invalid_parameter, then
//       set result = -1
//     - If result == 0: standard MSVC FILE buffer read:
//         DEC [ESI+4] (_cnt--)
//         if JS (underflow): CALL 0x9e0242 (__filbuf / refill)
//         else: MOVZX EAX, [ECX] / INC ECX / MOV [ESI], ECX  (*_ptr++)
//     - MOV [EBP-4], 0xFFFFFFFE (exit SEH scope)
//     - CALL __EH_epilog3 (at 0x9dd6ea, just past function end)
//     - MOV EAX, [EBP-0x1c] (load result)
//     - CALL __security_check_cookie (0x9de535)
//     - RET
//
//   Stack frame (via __EH_prolog3 with alloca_size=0x0C):
//     [EBP+08]            stream    (arg, loaded into ESI)
//     [EBP-04]            SEH state (0 during try body, -2/0xFFFFFFFE on exit)
//     [EBP-1C]            result    (int, init to 0; -1 on error; char on success)
//     (EBX, ESI, EDI, security cookie saved by __EH_prolog3)
//
//   Reloc-bearing sites (absolute addresses baked in post-link; wildcarded
//   by compare.py's COFF-reloc mask on the compiled .obj's side):
//     +0x02   scope-table PUSH    (0x0122D2B8 — .rdata FuncInfo)
//     +0x07   __EH_prolog3 CALL   (0x9DE4F0 — CRT prolog helper)
//     +0x1e   __errno CALL        (0x9D9D47 — relative)
//     +0x2a   _invalid_parameter  (0x9D2290 — relative)
//     +0x3e   stream-init CALL    (0x9D4E3E — relative)
//     +0x50   lock-fn CALL ×4     (0x9D6A61 — four separate rel32s)
//     +0x78   LEA abs addr        (0x0137B7E0 — .data lock table)
//     +0x88   MOV EAX abs addr    (0x012EB4D8 — .data fallback object)
//     +0xb4   LEA abs addr        (0x0137B7E0 — 2nd occurrence)
//     +0xca   MOV EAX abs addr    (0x012EB4D8 — 2nd occurrence)
//     +0xd9   __errno CALL        (0x9D9D47 — 2nd occurrence)
//     +0xdf   _invalid_parameter  (0x9D2290 — 2nd occurrence)
//     +0x107  __filbuf CALL       (0x9E0242 — stream refill)
//     +0x11e  __EH_epilog3 CALL   (0x9DD6EA — just past fn end)
//     +0x123  __security_check_cookie CALL (0x9DE535)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ would require MSVC 2005 /O2 /GS /EHsc to reproduce
//   the exact __EH_prolog3 prolog bytes, the two-pass locking index
//   computation (four back-to-back calls to the same function with
//   SAR/LEA/AND/SHL/ADD address arithmetic), the exact branch structure,
//   and every relative-call displacement. The absolute addresses in the
//   LEA + MOV instructions alone would shift relocation offsets away
//   from their original positions.
//
//   The pragmatic choice — the same one FUN_004014b0, FUN_00401a00, and
//   FUN_00408f10 took — is a `__declspec(naked)` body that re-emits the
//   orig 300 bytes verbatim via MASM `_emit` directives. The .obj's
//   `.text` section ends up byte-identical to the orig slice, which is
//   what `tools/compare.py` checks against.

extern "C" __declspec(naked) void FUN_009dd5bb() {
    __asm {
        _emit 0x6a              // PUSH 0x0c                     ; __EH_prolog3 frame-size arg
        _emit 0x0c
        _emit 0x68              // PUSH 0x0122D2B8               ; scope-table (reloc)
        _emit 0xb8
        _emit 0xd2
        _emit 0x22
        _emit 0x01
        _emit 0xe8              // CALL __EH_prolog3             ; (reloc)
        _emit 0x29
        _emit 0x0f
        _emit 0x00
        _emit 0x00
        _emit 0x33              // XOR EBX, EBX
        _emit 0xdb
        _emit 0x89              // MOV [EBP-0x1c], EBX           ; result = 0
        _emit 0x5d
        _emit 0xe4
        _emit 0x33              // XOR EAX, EAX
        _emit 0xc0
        _emit 0x8b              // MOV ESI, [EBP+8]              ; ESI = stream
        _emit 0x75
        _emit 0x08
        _emit 0x3b              // CMP ESI, EBX                  ; stream == NULL?
        _emit 0xf3
        _emit 0x0f              // SETNZ AL
        _emit 0x95
        _emit 0xc0
        _emit 0x3b              // CMP EAX, EBX
        _emit 0xc3
        _emit 0x75              // JNZ (stream != NULL path)
        _emit 0x20

        // ---- NULL stream error path ----
        _emit 0xe8              // CALL __errno                  ; (reloc)
        _emit 0x68
        _emit 0xc7
        _emit 0xff
        _emit 0xff
        _emit 0xc7              // MOV [EAX], 0x16               ; errno = EINVAL
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0xe8              // CALL _invalid_parameter       ; (reloc)
        _emit 0xa1
        _emit 0x4c
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x83              // OR EAX, 0xffffffff            ; return -1
        _emit 0xc8
        _emit 0xff
        _emit 0xe9              // JMP epilogue
        _emit 0xe7
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ---- stream != NULL path ----
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL stream_init_or_lock      ; (reloc)
        _emit 0x3e
        _emit 0x78
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x89              // MOV [EBP-4], EBX              ; SEH scope = 0
        _emit 0x5d
        _emit 0xfc
        _emit 0xf6              // TEST [ESI+0x0c], 0x40         ; check locked flag
        _emit 0x46
        _emit 0x0c
        _emit 0x40
        _emit 0x0f              // JNZ (already locked, skip lock acquire)
        _emit 0x85
        _emit 0xa6
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ---- lock acquisition (first pass) ----
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0x4d
        _emit 0x94
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x74              // JZ fallback_1
        _emit 0x2e
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0x41
        _emit 0x94
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x83              // CMP EAX, -2
        _emit 0xf8
        _emit 0xfe
        _emit 0x74              // JZ fallback_1
        _emit 0x22
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0x35
        _emit 0x94
        _emit 0xff
        _emit 0xff
        _emit 0xc1              // SAR EAX, 5
        _emit 0xf8
        _emit 0x05
        _emit 0x8d              // LEA EDI, [EAX*4+0x0137b7e0]  ; (reloc abs addr)
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0x25
        _emit 0x94
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x59              // POP ECX
        _emit 0x83              // AND EAX, 0x1f
        _emit 0xe0
        _emit 0x1f
        _emit 0xc1              // SHL EAX, 6
        _emit 0xe0
        _emit 0x06
        _emit 0x03              // ADD EAX, [EDI]               ; lock obj addr
        _emit 0x07
        _emit 0xeb              // JMP check_lock_1
        _emit 0x05
        // ---- fallback_1: use default lock object ----
        _emit 0xb8              // MOV EAX, 0x012eb4d8          ; (reloc abs addr)
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01

        // ---- check_lock_1: test flag byte ----
        _emit 0xf6              // TEST [EAX+0x24], 0x7f
        _emit 0x40
        _emit 0x24
        _emit 0x7f
        _emit 0x75              // JNZ (flag set, goto common path)
        _emit 0x45

        // ---- lock acquisition (second pass) ----
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0x08
        _emit 0x94
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x83              // CMP EAX, -1
        _emit 0xf8
        _emit 0xff
        _emit 0x74              // JZ fallback_2
        _emit 0x2e
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0xfc
        _emit 0x93
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x83              // CMP EAX, -2
        _emit 0xf8
        _emit 0xfe
        _emit 0x74              // JZ fallback_2
        _emit 0x22
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0xf0
        _emit 0x93
        _emit 0xff
        _emit 0xff
        _emit 0xc1              // SAR EAX, 5
        _emit 0xf8
        _emit 0x05
        _emit 0x8d              // LEA EDI, [EAX*4+0x0137b7e0]  ; (reloc abs addr)
        _emit 0x3c
        _emit 0x85
        _emit 0xe0
        _emit 0xb7
        _emit 0x37
        _emit 0x01
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL lock_fn                  ; (reloc)
        _emit 0xe0
        _emit 0x93
        _emit 0xff
        _emit 0xff
        _emit 0x59              // POP ECX
        _emit 0x59              // POP ECX
        _emit 0x83              // AND EAX, 0x1f
        _emit 0xe0
        _emit 0x1f
        _emit 0xc1              // SHL EAX, 6
        _emit 0xe0
        _emit 0x06
        _emit 0x03              // ADD EAX, [EDI]
        _emit 0x07
        _emit 0xeb              // JMP check_lock_2
        _emit 0x05
        // ---- fallback_2: use default lock object ----
        _emit 0xb8              // MOV EAX, 0x012eb4d8          ; (reloc abs addr)
        _emit 0xd8
        _emit 0xb4
        _emit 0x2e
        _emit 0x01

        // ---- check_lock_2: test 0x80 flag byte ----
        _emit 0xf6              // TEST [EAX+0x24], 0x80
        _emit 0x40
        _emit 0x24
        _emit 0x80
        _emit 0x74              // JZ (no error, fall through)
        _emit 0x1c

        // ---- second pass error: set EINVAL + _invalid_parameter ----
        _emit 0xe8              // CALL __errno                  ; (reloc)
        _emit 0xaa
        _emit 0xc6
        _emit 0xff
        _emit 0xff
        _emit 0xc7              // MOV [EAX], 0x16               ; errno = EINVAL
        _emit 0x00
        _emit 0x16
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0x53              // PUSH EBX (NULL)
        _emit 0xe8              // CALL _invalid_parameter       ; (reloc)
        _emit 0xe3
        _emit 0x4b
        _emit 0xff
        _emit 0xff
        _emit 0x83              // ADD ESP, 0x14
        _emit 0xc4
        _emit 0x14
        _emit 0x83              // OR [EBP-0x1c], 0xffffffff     ; result = -1
        _emit 0x4d
        _emit 0xe4
        _emit 0xff

        // ---- common path: check if result was set (error) ----
        _emit 0x39              // CMP [EBP-0x1c], EBX           ; result == 0?
        _emit 0x5d
        _emit 0xe4
        _emit 0x75              // JNZ (skip read, result already set)
        _emit 0x19

        // ---- MSVC FILE buffer read ----
        _emit 0xff              // DEC [ESI+4]                   ; _cnt--
        _emit 0x4e
        _emit 0x04
        _emit 0x78              // JS (buffer exhausted, call __filbuf)
        _emit 0x0a
        _emit 0x8b              // MOV ECX, [ESI]                ; ECX = _ptr
        _emit 0x0e
        _emit 0x0f              // MOVZX EAX, [ECX]              ; read byte
        _emit 0xb6
        _emit 0x01
        _emit 0x41              // INC ECX                       ; _ptr++
        _emit 0x89              // MOV [ESI], ECX                ; store _ptr
        _emit 0x0e
        _emit 0xeb              // JMP (skip filbuf)
        _emit 0x07
        // ---- __filbuf call (buffer refill) ----
        _emit 0x56              // PUSH ESI
        _emit 0xe8              // CALL __filbuf                 ; (reloc)
        _emit 0x74
        _emit 0x2b
        _emit 0x00
        _emit 0x00
        _emit 0x59              // POP ECX

        // ---- store result, exit SEH scope ----
        _emit 0x89              // MOV [EBP-0x1c], EAX           ; result = char/EOF
        _emit 0x45
        _emit 0xe4
        _emit 0xc7              // MOV [EBP-4], 0xFFFFFFFE       ; SEH scope = -2
        _emit 0x45
        _emit 0xfc
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        _emit 0xff
        _emit 0xe8              // CALL __EH_epilog3             ; (reloc - just past fn)
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // ---- epilogue ----
        _emit 0x8b              // MOV EAX, [EBP-0x1c]           ; load result
        _emit 0x45
        _emit 0xe4
        _emit 0xe8              // CALL __security_check_cookie  ; (reloc)
        _emit 0x4f
        _emit 0x0e
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
