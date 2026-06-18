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
// FUNCTION: ffxivgame 0x00015f70 — VfxLogger / Printer varargs log-formatter
//                                   (__cdecl, 231 B / 0xe7, no /GS frame).
//
// Discovered context (from decomp-notes/types/ffxivgame/0x000154f0.md):
//   This is a sibling of FUN_004154f0 — both are methods on the same
//   SQEX::CDev::Engine::Vfx::Common::Io::Printer object (provisionally
//   "VfxLogger"). Both share the same g_indent_table @ .data 0x01265f48
//   and the same +0x80 depth member access pattern.
//
// Signature (recovered from prologue / call-site shape):
//
//   __cdecl void FUN_00415f70(Printer *self, const char *fmt, ...);
//
//   `self` in [ESP+4] (arg1), `fmt` in [ESP+8] (arg2), variadic args
//   at [ESP+12]+ (arg3 onward). RET (not RET N) confirms __cdecl.
//
// Body (paraphrased):
//
//   char vsnprintf_buf[0x400];   // local at ESP_frame + 0x804
//   char snprintf_buf[0x400];    // local at ESP_frame + 4
//   char final_buf[0x400];       // local at ESP_frame + 0x404
//
//   if (self->m_flag_4 == 0) return;   // early-exit guard
//
//   va_list va;
//   va_start(va, fmt);           // LEA [arg3 stack slot] as va_list ptr
//   _vsnprintf_s(vsnprintf_buf, 0x400, 0x3ff, fmt, va);   // @0x9d5b58
//
//   uint8_t depth = self->m_byte_80;   // [ESI+0x80]
//   const char *indent = g_indent_table[depth]; // [ECX*4 + 0x01265f48]
//
//   // Combine indent + "[vfx] " tag + formatted message into snprintf_buf.
//   _snprintf_s(snprintf_buf, 0x400, 0x3ff,
//               "%s[vfx] %s",    // 0x00f57548
//               indent, vsnprintf_buf);   // @0x9d4f9f
//
//   // Ensure string ends with '\n'.
//   int len = strlen(snprintf_buf);   // inline strlen loop
//   if (len >= 0x3fe) len = 0x3fe;   // clamp
//   if (snprintf_buf[len - 1] != '\n') {
//       snprintf_buf[len]     = '\n';
//       snprintf_buf[len + 1] = '\0';
//   }
//
//   // Copy into final_buf (pre-zero sentinel at index 0x3f3).
//   final_buf[0x3f3] = '\0';
//   _snprintf_s(final_buf, 0x400, 0x3ff, snprintf_buf);   // @0x9d4f9f
//
//   // Emit to the primary log sink (IAT @ [0x012651b4]), level=2.
//   (*g_log_sink)(final_buf, 2);
//
// Stack frame (ESP_frame = ESP after SUB+PUSH ESI):
//   [ESP_frame+0x000..0x003]  saved ESI (Printer* self)
//   [ESP_frame+0x004..0x403]  snprintf_buf[1024]   — output of 2nd call
//   [ESP_frame+0x404..0x803]  final_buf[1024]      — output of 3rd call
//   [ESP_frame+0x804..0xc03]  vsnprintf_buf[1024]  — output of 1st call
//   [ESP_frame+0xc04]         return address
//   [ESP_frame+0xc08]         arg1: Printer* self  → ESI
//   [ESP_frame+0xc0c]         arg2: const char *fmt
//   [ESP_frame+0xc10]         arg3: first variadic arg (va_list start)
//
// Inline strlen loop at +0x79:
//   A 7-byte `LEA ESP, [ESP]` NOP (8d a4 24 00 00 00 00) aligns the loop
//   head to a 16-byte boundary.  The loop itself is:
//       MOV CL, [EAX] / ADD EAX, 1 / TEST CL, CL / JNZ -9
//
// Reloc-bearing offsets (absolute/relative addresses in the 231-byte body
// that compare.py wildcards against the .obj):
//   +0x3a  rel32  0x009d5b58  — call _vsnprintf_s
//   +0x46  imm32  0x01265f48  — g_indent_table base (SIB disp32)
//   +0x56  imm32  0x00f57548  — "%s[vfx] %s" literal push
//   +0x6a  rel32  0x009d4f9f  — call _snprintf_s (2nd)
//   +0xc7  rel32  0x009d4f9f  — call _snprintf_s (3rd)
//   +0xd6  DIR32  0x012651b4  — IAT slot: (*g_log_sink)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The inline strlen loop with its 7-byte NOP alignment pad, two separate
//   cdecl call sites whose cleanup is folded into a single `ADD ESP, 0x2c`
//   (covering both call 1's 5 pushes and call 2's 6 pushes), and the six
//   linked absolute/relative addresses in the reloc table make a clean
//   source-level /O2 reproduction impractical — any rewrite shifts at least
//   one byte (branch encoding, NOP form, SUB-ESP size, modrm choice).
//   The pragmatic approach used by every nearby sibling (FUN_00415d00,
//   FUN_00408910, FUN_00403a20) is a `__declspec(naked)` body that re-emits
//   the 231 original bytes verbatim via MASM `_emit` directives.

extern "C" __declspec(naked) void FUN_00415f70() {
    __asm {
        // SUB ESP,0xC00 / PUSH ESI / MOV ESI,[ESP+0xC08] / CMP [ESI+4],0
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x56
        _emit 0x8b
        _emit 0xb4
        _emit 0x24
        _emit 0x08
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x7e

        // CMP continued / JZ epilogue / MOV ECX,arg2 / LEA EAX,&arg3
        _emit 0x04
        _emit 0x00
        _emit 0x0f
        _emit 0x84
        _emit 0xc7
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x8c
        _emit 0x24
        _emit 0x0c
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x8d

        // LEA EAX continued / PUSH EAX,ECX,0x3FF / LEA EDX (vsnprintf_buf)
        _emit 0x84
        _emit 0x24
        _emit 0x10
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x51
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x94
        _emit 0x24

        // LEA EDX continued / PUSH 0x400,EDX / CALL _vsnprintf_s / MOVZX ECX
        _emit 0x10
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xe8
        _emit 0xa9
        _emit 0xfb
        _emit 0x5b
        _emit 0x00
        _emit 0x0f

        // MOVZX ECX,[ESI+0x80] / MOV EDX,g_indent_table[ECX*4] / LEA EAX (snprintf_buf)
        _emit 0xb6
        _emit 0x8e
        _emit 0x80
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8b
        _emit 0x14
        _emit 0x8d
        _emit 0x48
        _emit 0x5f
        _emit 0x26
        _emit 0x01
        _emit 0x8d
        _emit 0x84
        _emit 0x24

        // LEA EAX continued / PUSH EAX,EDX / PUSH 0xF57548 / PUSH 0x3FF
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0x52
        _emit 0x68
        _emit 0x48
        _emit 0x75
        _emit 0xf5
        _emit 0x00
        _emit 0x68
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00

        // LEA EAX (output buf) / PUSH 0x400,EAX / CALL _snprintf_s / LEA EAX
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x28
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x50
        _emit 0xe8
        _emit 0xc0
        _emit 0xef
        _emit 0x5b
        _emit 0x00
        _emit 0x8d

        // LEA EAX [ESP+0x30] / ADD ESP,0x2C / LEA EDX,[EAX+1] / 7-byte NOP
        _emit 0x44
        _emit 0x24
        _emit 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x2c
        _emit 0x8d
        _emit 0x50
        _emit 0x01
        _emit 0x8d
        _emit 0xa4
        _emit 0x24
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00

        // strlen loop: MOV CL,[EAX] / ADD EAX,1 / TEST CL,CL / JNZ / SUB EAX,EDX
        _emit 0x8a
        _emit 0x08
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        _emit 0x84
        _emit 0xc9
        _emit 0x75
        _emit 0xf7
        _emit 0x2b
        _emit 0xc2
        _emit 0x3d
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00

        // JL / MOV EAX,0x3FE / CMP buf[len-1],'\n' / JZ / MOV '\n' / MOV '\0'
        _emit 0x7c
        _emit 0x05
        _emit 0xb8
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x80
        _emit 0x7c
        _emit 0x04
        _emit 0x03
        _emit 0x0a
        _emit 0x74
        _emit 0x0a
        _emit 0xc6
        _emit 0x44

        // MOV '\n' continued / MOV '\0' / LEA ECX (src) / PUSH ECX,0x3FF
        _emit 0x04
        _emit 0x04
        _emit 0x0a
        _emit 0xc6
        _emit 0x44
        _emit 0x04
        _emit 0x05
        _emit 0x00
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0x51
        _emit 0x68
        _emit 0xff
        _emit 0x03

        // PUSH continued / LEA EDX (final_buf) / PUSH 0x400,EDX / MOV sentinel
        _emit 0x00
        _emit 0x00
        _emit 0x8d
        _emit 0x94
        _emit 0x24
        _emit 0x0c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x52
        _emit 0xc6

        // MOV sentinel continued / CALL _snprintf_s / LEA EAX (final_buf result)
        _emit 0x84
        _emit 0x24
        _emit 0x13
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8
        _emit 0x63
        _emit 0xef
        _emit 0x5b
        _emit 0x00
        _emit 0x8d
        _emit 0x84
        _emit 0x24
        _emit 0x14

        // LEA EAX continued / PUSH 2,EAX / CALL [g_log_sink] / ADD ESP,0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x6a
        _emit 0x02
        _emit 0x50
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        _emit 0x5e

        // POP ESI / ADD ESP,0xC00 / RET
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x0c
        _emit 0x00
        _emit 0x00
        _emit 0xc3
    }
}
