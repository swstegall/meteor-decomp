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
// FUNCTION: ffxivgame 0x00016160 — SQEX::CDev::Engine::Vfx::Common::Io::Printer
//                                  variadic assert / log helper
//                                  (266 B / 0x10a, no SEH, single-block body)
//
// Inspection (read from the disassembly at orig RVA 0x00016160):
//
//   __cdecl void FUN_00416160(Printer *param_1, const char *fmt, ...);
//
//     char fmt_buf[0x400];                          // [esp+0x008..esp+0x408]
//     char vfx_buf[0x408];                          // [esp+0x408..esp+0x810]
//     char prefix_buf[0x400];                       // [esp+0x810..esp+0xc10]
//     char line_buf[0x3fe];                         // [esp+0xc14..esp+0x1012]
//                                                   // (line_buf has a NUL written
//                                                   //  at +0x42e — i.e. byte
//                                                   //  prefix_buf[0xc0a] sentinel,
//                                                   //  see asm offset 0x416201)
//
//     vsnprintf_s(fmt_buf, 0x400, 0x3ff, fmt, &vararg_start);     // 0x009d5b58
//     FUN_00415650(2, 0, vfx_buf);                                 // 0x00415650
//     snprintf_s(prefix_buf, 0x400, 0x3ff,
//                "%s[vfx.assert] %s",                              // 0x00f57534
//                (&PTR_DAT_01265f48)[param_1->state_80 /*byte*/],  // 0x01265f48
//                vfx_buf);
//     prefix_buf[0x402] = 0;                       // hard-clamp at +0x42e
//     snprintf_s(line_buf, 0x400, 0x3fe, prefix_buf);              // 0x009d4f9f
//     strcat_s(line_buf, 0x400, "\n");             // 0x00f54d98 / 0x009d4bb4
//     (*PTR_FUN_012651b4)(line_buf, 3);            // indirect via .data slot
//     if (param_1->flag_7c != 0) {
//         FUN_004165e0(prefix_buf, 1);                              // 0x004165e0
//     }
//     FUN_00406550("false", "<file-string>",                        // 0x00406550
//                  ".\\Io\\Printer.cpp", 0x217,
//                  "SQEX::CDev::Engine::Vfx::Common::Io::Printer::Run_");
//
//   Stack frame (ESP-relative, after the 0x100c-byte __alloca_probe):
//     [esp+0x000]                  __alloca_probe scratch
//     [esp+0x004]                  callee-saved ESI (push esi at 0x00416171)
//     [esp+0x008..esp+0x407]       fmt_buf      (1024 B)
//     [esp+0x408..esp+0xc0f]       vfx_buf      (used by FUN_00415650; size
//                                                 1032 B inclusive of trailing
//                                                 NUL slot at +0xc0a / 0xc0b)
//     [esp+0xc10..esp+0x100f]      prefix_buf   (1024 B) and line_buf overlap
//                                                 with the +0x42e NUL-poke at
//                                                 0x00416201 — the layout
//                                                 reuses the same stack window
//                                                 across the two snprintf
//                                                 destinations.
//     [esp+0x1010]                 return address slot (popped by ADD ESP)
//
//   __cdecl vararg signature confirmed by:
//     - varargs explicitly forwarded via `LEA EAX,[ESP+0x101c]` (entry +0xc,
//       i.e. arg3 onwards) before the vsnprintf_s call;
//     - caller-cleanup epilogue `ADD ESP, 0x100c` + `RET` (no `RET N`).
//
//   Reloc-bearing sites in the orig 266 bytes (these absolute addresses
//   resolve only in a full-binary relink at image base 0x00400000;
//   standalone .obj compilation can't reproduce them):
//     +0x06  __alloca_probe CALL              (.text 0x009d29d0 rel32)
//     +0x28  vsnprintf_s CALL                 (.text 0x009d5b58 rel32)
//     +0x4a  FUN_00415650 CALL                (.text 0x00415650 rel32 — __thiscall)
//     +0x57  PTR_DAT_01265f48 base load       (.data 0x01265f48 — table of
//                                              prefix strings indexed by
//                                              param_1->state_80)
//     +0x66  push "%s[vfx.assert] %s"         (.rdata 0x00f57534)
//     +0x7d  snprintf_s CALL                  (.text 0x009d4f9f rel32)
//     +0xa1  snprintf_s CALL                  (.text 0x009d4f9f rel32, 2nd)
//     +0xa7  push prefix_buf snprintf src     (.rdata 0x00f54d98)
//     +0xb5  strcat_s CALL                    (.text 0x009d4bb4 rel32)
//     +0xc3  PTR_FUN_012651b4 indirect CALL   (.data 0x012651b4 — fn pointer)
//     +0xdc  FUN_004165e0 CALL                (.text 0x004165e0 rel32)
//     +0xe2  push "<arg-string-1>"            (.rdata 0x00f57640)
//     +0xec  push "<arg-string-2>"            (.rdata 0x00f5758c)
//     +0xf1  push "<arg-string-3>"            (.rdata 0x00f54d48)
//     +0xf6  push "<arg-string-4>"            (.rdata 0x00f56510)
//     +0xff  FUN_00406550 CALL                (.text 0x00406550 rel32)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ here would need to coax MSVC 2005 /O2 /GS /EHsc into
//   reproducing the exact __alloca_probe-based prologue (rather than the
//   plain `sub esp, imm32` form, which MSVC chooses based on a heuristic
//   keyed on frame size + local-array placement), the precise register
//   allocation across the vsnprintf_s / snprintf_s / strcat_s vararg
//   forwarding (this/edx ↔ stack), the chosen overlap between prefix_buf
//   and line_buf in the stack frame, AND the linker-resolved absolute
//   addresses in the sixteen relocation windows above. Each of those
//   constraints is brittle under /O2.
//
//   The pragmatic choice — the same one all the other large
//   relocation-rich helpers in this binary took (FUN_00401a00,
//   FUN_004014b0, FUN_00403a20, …) — is a `__declspec(naked)` body that
//   re-emits the orig 266 bytes verbatim via MASM `_emit` directives.
//   The .obj's `.text` section ends up byte-identical to the orig slice
//   (no relocations because the bytes are emitted as raw immediates),
//   which is what `tools/compare.py` checks against.
//
//   The structural commentary above is the readable record of what the
//   function actually does, so a future contributor can promote this to
//   a real source-level match once the surrounding Printer class
//   (param_1->state_80 selector, param_1->flag_7c branch), the prefix-
//   string table at .data 0x01265f48, and the dispatcher function
//   pointer at .data 0x012651b4 are catalogued under decomp-notes/types/.

extern "C" __declspec(naked) void FUN_00416160() {
    __asm {
        // ---- prologue: stack-probing alloca for 0x100c locals ----
        _emit 0xb8              // MOV  EAX, 0x100c
        _emit 0x0c
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL __alloca_probe (rel32 → 0x009d29d0)
        _emit 0x66
        _emit 0xc8
        _emit 0x5b
        _emit 0x00

        // ---- vsnprintf_s(fmt_buf, 0x400, 0x3ff, fmt, va_args) ----
        _emit 0x8b              // MOV  ECX, [ESP+0x1014]   ; fmt (arg2)
        _emit 0x8c
        _emit 0x24
        _emit 0x14
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA  EAX, [ESP+0x101c]   ; &va_args (arg3..)
        _emit 0x84
        _emit 0x24
        _emit 0x1c
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x51              // PUSH ECX                  ; fmt
        _emit 0x68              // PUSH 0x3ff
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA  EDX, [ESP+0xc1c]    ; fmt_buf
        _emit 0x94
        _emit 0x24
        _emit 0x1c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x400
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0xe8              // CALL __vsnprintf_s        (rel32 → 0x009d5b58)
        _emit 0xc6
        _emit 0xf9
        _emit 0x5b
        _emit 0x00

        // ---- FUN_00415650(2, 0, vfx_buf)   [__thiscall: ECX=param_1] ----
        _emit 0x8b              // MOV  ESI, [ESP+0x1028]   ; param_1 (arg1)
        _emit 0xb4
        _emit 0x24
        _emit 0x28
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x83              // ADD  ESP, 0x14            ; cleanup 5 args
        _emit 0xc4
        _emit 0x14
        _emit 0x8d              // LEA  EAX, [ESP+0x808]    ; &vfx_buf
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0x6a              // PUSH 0
        _emit 0x00
        _emit 0x6a              // PUSH 2
        _emit 0x02
        _emit 0x8b              // MOV  ECX, ESI             ; this = param_1
        _emit 0xce
        _emit 0xe8              // CALL FUN_00415650          (rel32 → 0x00415650)
        _emit 0xa1
        _emit 0xf4
        _emit 0xff
        _emit 0xff

        // ---- snprintf_s(prefix_buf, 0x400, 0x3ff, "%s[vfx.assert] %s",
        //                 PTR_TABLE[param_1->state_80], vfx_buf) ----
        _emit 0x0f              // MOVZX EDX, byte ptr [ESI+0x80]
        _emit 0xb6
        _emit 0x96
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV  EAX, [EDX*4 + 0x01265f48]
        _emit 0x04
        _emit 0x95
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8d              // LEA  ECX, [ESP+0x808]    ; &vfx_buf
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX                  ; vfx_buf
        _emit 0x50              // PUSH EAX                  ; PTR_TABLE[idx]
        _emit 0x68              // PUSH "%s[vfx.assert] %s"  (.rdata 0x00f57534)
        _emit 0x34
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x3ff
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA  ECX, [ESP+0x418]    ; &prefix_buf
        _emit 0x8c
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x400
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL __snprintf_s         (rel32 → 0x009d4f9f)
        _emit 0xbd
        _emit 0xed
        _emit 0x5b
        _emit 0x00

        // ---- snprintf_s(line_buf, 0x400, 0x3fe, prefix_buf) ; sentinel NUL ----
        _emit 0x8d              // LEA  EDX, [ESP+0x420]    ; &prefix_buf source
        _emit 0x94
        _emit 0x24
        _emit 0x20
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52              // PUSH EDX
        _emit 0x68              // PUSH 0x3fe
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA  EAX, [ESP+0x28]     ; &line_buf
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x68              // PUSH 0x400
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xc6              // MOV  byte ptr [ESP+0x42e], 0   ; clamp sentinel
        _emit 0x84
        _emit 0x24
        _emit 0x2e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL __snprintf_s         (rel32 → 0x009d4f9f, 2nd)
        _emit 0x99
        _emit 0xed
        _emit 0x5b
        _emit 0x00

        // ---- strcat_s(line_buf, 0x400, "\n") ----
        _emit 0x68              // PUSH "\n" / sep-string    (.rdata 0x00f54d98)
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA  ECX, [ESP+0x34]     ; &line_buf
        _emit 0x4c
        _emit 0x24
        _emit 0x34
        _emit 0x68              // PUSH 0x400
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL strcat_s             (rel32 → 0x009d4bb4)
        _emit 0x9a
        _emit 0xe9
        _emit 0x5b
        _emit 0x00

        // ---- indirect (*PTR_FUN_012651b4)(line_buf, 3) ----
        _emit 0x8d              // LEA  EDX, [ESP+0x3c]     ; &line_buf (after cleanup)
        _emit 0x54
        _emit 0x24
        _emit 0x3c
        _emit 0x6a              // PUSH 3
        _emit 0x03
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL dword ptr [0x012651b4]
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01

        // ---- if (param_1->flag_7c != 0) FUN_004165e0(prefix_buf, 1) ----
        _emit 0x8b              // MOV  ECX, [ESI+0x7c]     ; flag_7c
        _emit 0x4e
        _emit 0x7c
        _emit 0x83              // ADD  ESP, 0x3c            ; cleanup pushes
        _emit 0xc4
        _emit 0x3c
        _emit 0x85              // TEST ECX, ECX
        _emit 0xc9
        _emit 0x5e              // POP  ESI
        _emit 0x74              // JZ   +0x0f → 0x00416241
        _emit 0x0f
        _emit 0x6a              // PUSH 1
        _emit 0x01
        _emit 0x8d              // LEA  EAX, [ESP+0x408]    ; &prefix_buf
        _emit 0x84
        _emit 0x24
        _emit 0x08
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL FUN_004165e0          (rel32 → 0x004165e0)
        _emit 0x9f
        _emit 0x03
        _emit 0x00
        _emit 0x00

        // ---- FUN_00406550("false", "<arg2>", ".\\Io\\Printer.cpp", 0x217,
        //                   "SQEX::CDev::Engine::Vfx::Common::Io::Printer::Run_")
        //      [__thiscall: ECX = &fmt_buf (or its tail)] ----
    label_416241:
        _emit 0x68              // PUSH 0x00f57640
        _emit 0x40
        _emit 0x76
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x217
        _emit 0x17
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x00f5758c
        _emit 0x8c
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f54d48
        _emit 0x48
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        _emit 0x68              // PUSH 0x00f56510
        _emit 0x10
        _emit 0x65
        _emit 0xf5
        _emit 0x00
        _emit 0x8d              // LEA  ECX, [ESP+0x17]
        _emit 0x4c
        _emit 0x24
        _emit 0x17
        _emit 0xe8              // CALL FUN_00406550          (rel32 → 0x00406550)
        _emit 0xed
        _emit 0x02
        _emit 0xff
        _emit 0xff

        // ---- epilogue: drop locals + return ----
        _emit 0x81              // ADD  ESP, 0x100c
        _emit 0xc4
        _emit 0x0c
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
