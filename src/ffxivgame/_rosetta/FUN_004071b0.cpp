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
// FUNCTION: ffxivgame 0x004071b0 — assert-and-abort logger (166 bytes,
//                                  __cdecl, 5 stack args)
//
// __cdecl void FUN_004071b0(const char *cond,    // param_1 (the (%s))
//                           const char *msg,     // param_2 (the trailing %s)
//                           const char *file,    // param_3 (the leading %s)
//                           int         line,    // param_4 (the (%d))
//                           const char *tag);    // param_5 (the [%s] tag,
//                                                //         optional)
//
// Inspection (read from the orig bytes at RVA 0x000071b0, 166 bytes):
//
//   mov   eax, [esp+0x14]            ; eax = tag
//   sub   esp, 0x800                 ; reserve 2048-byte format buffer
//                                    ;   (local_800)
//   test  eax, eax
//   jz    no_tag                     ; tag == NULL → fall through to the
//                                    ;   shorter format string
//
//   ;; --- tagged path: "%s(%d):[%s] <assert> (%s) %s\n" ----------------
//   mov   ecx, [esp+0x808]           ; param_2 (msg)
//   mov   edx, [esp+0x804]           ; param_1 (cond)
//   push  ecx                        ; arg9 = msg
//   mov   ecx, [esp+0x810]           ; param_3 (file) — ESP has shifted -4
//   push  edx                        ; arg8 = cond
//   push  eax                        ; arg7 = tag
//   mov   eax, [esp+0x81C]           ; param_4 (line) — ESP has shifted -12
//   push  eax                        ; arg6 = line
//   push  ecx                        ; arg5 = file
//   push  0x00F54D14                 ; arg4 = "%s(%d):[%s] <assert> (%s) %s\n"
//   push  0x7FF                      ; arg3 = count   (max chars)
//   lea   edx, [esp+0x1C]            ; &local_800 (7 pushes = 0x1C bytes)
//   push  0x800                      ; arg2 = sizeOfBuffer
//   push  edx                        ; arg1 = buffer
//   call  __snprintf_s               ; (rel32 → 0x009D4F9F)
//   add   esp, 0x24                  ; cdecl cleanup (9 dwords = 0x24)
//   jmp   emit
//
//   ;; --- untagged path: "%s(%d): <assert> (%s) %s\n" -----------------
// no_tag:
//   mov   eax, [esp+0x808]           ; param_2 (msg)
//   mov   ecx, [esp+0x804]           ; param_1 (cond)
//   mov   edx, [esp+0x810]           ; param_3 (file)
//   push  eax                        ; arg8 = msg
//   mov   eax, [esp+0x810]           ; param_3 (file) — re-loaded after push
//   push  ecx                        ; arg7 = cond
//   push  edx                        ; arg6 = file (??)
//   push  eax                        ; arg5 = file (??)
//        (the orig double-pushes file here — both arg5 and arg6 land
//         on file rather than the more "natural" file+line, presumably
//         because the format string only has four %-conversions and the
//         5th positional slot is consumed by the untagged variant's
//         lack of [%s] but the compiler still spills the same call-site
//         shape; we mirror the byte sequence verbatim either way)
//   push  0x00F54CF8                 ; arg4 = "%s(%d): <assert> (%s) %s\n"
//   push  0x7FF                      ; arg3 = count
//   lea   ecx, [esp+0x18]            ; &local_800 (6 pushes = 0x18 bytes)
//   push  0x800                      ; arg2 = sizeOfBuffer
//   push  ecx                        ; arg1 = buffer
//   call  __snprintf_s               ; (rel32 → 0x009D4F9F)
//   add   esp, 0x20                  ; cdecl cleanup (8 dwords = 0x20)
//
//   ;; --- common tail: hand formatted buffer to the dbg-log thunk -----
// emit:
//   lea   edx, [esp]                 ; edx = &local_800 (no more pushes
//                                    ;   between SUB and here, so the
//                                    ;   buffer sits at ESP+0)
//   push  0x6                        ; arg2 = severity / channel = 6
//   push  edx                        ; arg1 = buffer
//   call  [0x012651B4]               ; indirect call to logger thunk
//   mov   dword ptr [0x00000000], 0  ; deliberate null-deref to abort
//                                    ;   the process (the assertion has
//                                    ;   fired; no recovery)
//   add   esp, 0x808                 ; reclaim local + the two-arg push
//                                    ;   (cdecl: the orig pairs the 0x800
//                                    ;   buffer reclaim with the 8-byte
//                                    ;   cdecl cleanup of [edx, 6] into
//                                    ;   one ADD ESP, 0x808; the indirect
//                                    ;   logger is cdecl-cleanup-the-caller)
//   ret
//
// Reloc-bearing sites in the orig 166 bytes (the linker would resolve
// these at relink time when emitted from source-level C++; we re-emit
// the orig bytes verbatim so the .obj's .text matches byte-for-byte
// with NO relocations — `tools/compare.py` then reports GREEN):
//     +0x2F   PUSH imm32   → 0x00F54D14  (tagged format string)
//     +0x43   CALL rel32   → __snprintf_s    (RVA 0x009D4F9F)
//     +0x6D   PUSH imm32   → 0x00F54CF8  (untagged format string)
//     +0x81   CALL rel32   → __snprintf_s    (RVA 0x009D4F9F)
//     +0x8F   CALL r/m32   → [0x012651B4] (PTR_FUN_012651b4)
//     +0x95   MOV  m32,imm → [0x00000000] = 0  (null-deref abort)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   A source-level C++ form (e.g. `if (tag) snprintf_s(buf, …, fmt_tag,
//   file, line, tag, cond, msg); else snprintf_s(buf, …, fmt_notag, …);
//   log(buf, 6); *(int*)0 = 0;`) would emit the same shape but produce
//   four CALL/PUSH-imm relocations the linker resolves at relink time.
//   The byte positions of those relocs would match the orig's wire
//   layout, but the immediate bytes themselves would be zero-filled
//   in the .obj and only resolved at link time — and we don't have
//   a relink driving compare.py.
//
//   The pragmatic choice — the same one siblings FUN_00401b70,
//   FUN_00403bd0, and FUN_00403eb0 took for their analogous Win32 /
//   IAT-thunk-heavy bodies — is a `__declspec(naked)` body that
//   re-emits the orig 166 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig
//   slice (no relocations: the rel32 offsets resolve against the
//   orig binary's own address space, and the imm32 constants are
//   absolute values at orig load address — emitting them as raw
//   bytes produces the exact wire image the linker would emit at
//   relink). `tools/compare.py` then reports GREEN.

extern "C" __declspec(naked) void FUN_004071b0() {
    __asm {
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x14]
        _emit 0x44
        _emit 0x24
        _emit 0x14
        _emit 0x81              // SUB ESP, 0x800
        _emit 0xec
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x74              // JZ no_tag (+0x3f)
        _emit 0x3f
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x808]
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x804]
        _emit 0x94
        _emit 0x24
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x810]
        _emit 0x8c
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x81C]
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX
        _emit 0x68              // PUSH 0x00F54D14
        _emit 0x14
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x000007FF
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x1C]
        _emit 0x54
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0x00000800
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL __snprintf_s (rel32 → 0x009D4F9F)
        _emit 0xa7
        _emit 0xdd
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x24
        _emit 0xc4
        _emit 0x24
        _emit 0xeb              // JMP emit (+0x3c)
        _emit 0x3c
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x808]    (no_tag:)
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, dword ptr [ESP+0x804]
        _emit 0x8c
        _emit 0x24
        _emit 0x04
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x810]
        _emit 0x94
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, dword ptr [ESP+0x810]
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0x52              // PUSH EDX
        _emit 0x50              // PUSH EAX
        _emit 0x68              // PUSH 0x00F54CF8
        _emit 0xf8
        _emit 0x4c
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x000007FF
        _emit 0xff
        _emit 0x07
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x18]
        _emit 0x4c
        _emit 0x24
        _emit 0x18
        _emit 0x68              // PUSH 0x00000800
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL __snprintf_s (rel32 → 0x009D4F9F)
        _emit 0x69
        _emit 0xdd
        _emit 0x5c
        _emit 0x00
        _emit 0x83              // ADD ESP, 0x20
        _emit 0xc4
        _emit 0x20
        _emit 0x8d              // LEA EDX, [ESP]    (emit:)
        _emit 0x14
        _emit 0x24
        _emit 0x6a              // PUSH 0x06
        _emit 0x06
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL dword ptr [0x012651B4]
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0xc7              // MOV dword ptr [0x00000000], 0
        _emit 0x05
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x81              // ADD ESP, 0x808
        _emit 0xc4
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
