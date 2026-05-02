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
// FUNCTION: ffxivgame 0x00047260 — Sqex::Misc::Utf8String::Utf8String(data, len)
//
// One of the SE-internal string constructors. The class has SSO (small-
// string optimisation) with a 64-byte inline buffer and a heap fallback
// when content > 64 bytes. This particular ctor takes (data, length)
// where length=-1 means "use strlen(data) at runtime".
//
// Class layout (recovered from this constructor's stores):
//
//   class Utf8String {
//       char *m_data;            // +0x00 — points to inline buf or heap
//       int   m_capacity;        // +0x04 — initially 0x40 (64-byte SSO)
//       int   m_size;            // +0x08 — # bytes in use (incl. NUL)
//       int   m_field_c;         // +0x0c — flags / encoding bits (?)
//       char  m_flag_10;         // +0x10 — set to 1 by ctor
//       char  m_flag_11;         // +0x11 — set to 1 by ctor
//       char  m_inline_buf[0x40]; // +0x12..+0x51 (64 bytes inline SSO)
//       // sizeof = 0x54 (84 bytes, padded to int alignment)
//   };
//
// The constructor's stack footprint of 0 bytes locals (just PUSH EBX,
// ESI, EDI for callee-saves) plus the class's 84 bytes is what gives
// ProcessChunk its 92-byte (0x5c) frame: 84 (Utf8String) + 4 (byte-
// swap scratch) + 4 (SEH state index) = 92.
//
// Original bytes (116 B; ends at 0x000472d3 with `c2 08 00` RET 8,
// followed by `cc cc...` INT3 padding):
//
//   00047260: 53 8b 5c 24 08 56 8b f1 ba 01 00 00 00 57 8b 7c
//   00047270: 24 14 83 ff ff 8d 46 12 88 56 10 88 56 11 c7 46
//   00047280: 0c 00 00 00 00 89 56 08 c7 46 04 40 00 00 00 89
//   00047290: 06 c6 00 00 75 17 8b c3 8d 78 01 eb 03 8d 49 00
//   000472a0: 8a 08 83 c0 01 84 c9 75 f7 2b c7 8b f8 52 8d 47
//   000472b0: 01 50 8b ce e8 ?? ?? ?? ?? 8b 0e 57 53 51 e8 ??
//   000472c0: ?? ?? ?? 8b 16 83 c4 0c c6 04 17 00 5f 8b c6 5e
//   000472d0: 5b c2 08 00
//
// Decoded:
//   PUSH EBX                              ; callee-saved
//   MOV  EBX, [ESP+8]                     ; arg1 = data ptr
//   PUSH ESI
//   MOV  ESI, ECX                          ; ESI = this
//   MOV  EDX, 1                            ; EDX = 1 (reused throughout)
//   PUSH EDI
//   MOV  EDI, [ESP+0x14]                  ; arg2 = length
//   CMP  EDI, -1                           ; length == 0xffffffff sentinel?
//   LEA  EAX, [ESI+0x12]                   ; EAX = &m_inline_buf[0]
//   MOV  [ESI+0x10], DL                    ; m_flag_10 = 1
//   MOV  [ESI+0x11], DL                    ; m_flag_11 = 1
//   MOV  DWORD PTR [ESI+0xc], 0            ; m_field_c = 0
//   MOV  [ESI+8], EDX                      ; m_size = 1
//   MOV  DWORD PTR [ESI+4], 0x40           ; m_capacity = 64
//   MOV  [ESI], EAX                        ; m_data = &m_inline_buf[0]
//   MOV  byte ptr [EAX], 0                 ; null-terminate inline
//   JNZ  have_length                        ; if length != -1, skip strlen
//   ; --- length == -1: compute strlen(data) ---
//   MOV  EAX, EBX                           ; EAX = data
//   LEA  EDI, [EAX+1]                       ; EDI = data + 1 (saved start+1)
//   JMP  loop
//   nop  (LEA ECX, [ECX+0])                  ; alignment padding
//   loop:
//   MOV  CL, [EAX]
//   ADD  EAX, 1
//   TEST CL, CL
//   JNZ  loop                               ; until null
//   SUB  EAX, EDI                            ; EAX = strlen(data)
//   MOV  EDI, EAX
//   ; --- have_length (EDI = string length) ---
//   PUSH EDX                                ; push 1 (still EDX)
//   LEA  EAX, [EDI+1]                        ; EAX = length + 1
//   PUSH EAX                                 ; push (length + 1)
//   MOV  ECX, ESI                            ; ECX = this
//   CALL Utf8String::Reserve                  ; Reserve(length+1, 1)
//   ; --- copy data in via memcpy ---
//   MOV  ECX, [ESI]                          ; ECX = m_data
//   PUSH EDI                                 ; len
//   PUSH EBX                                 ; src
//   PUSH ECX                                 ; dst
//   CALL memcpy
//   MOV  EDX, [ESI]                          ; EDX = m_data
//   ADD  ESP, 0xc                            ; clean memcpy args
//   MOV  byte ptr [EDX+EDI], 0               ; null-terminate at end
//   POP  EDI; MOV EAX, ESI; POP ESI; POP EBX
//   RET  8                                  ; stdcall, 2 args
//
// The MSVC clever bits worth flagging:
//   1. EDX is set to 1 once at the top and reused for two byte stores
//      ([ESI+0x10], [ESI+0x11]) AND the dword `m_size` store ([ESI+8])
//      AND the PUSH 1 arg to Reserve. Saves 8 bytes vs separate
//      immediates.
//   2. The `JNZ have_length` after the CMP EDI, -1 reuses the flags
//      from CMP — between the CMP and the JNZ, MSVC emits 8 instructions
//      (LEA, MOV byte, MOV byte, MOV dword, MOV dword, MOV dword,
//      MOV dword, MOV byte) that don't touch flags. Same trick we saw
//      in ChunkReadUInt::ReadNextChunkHeader.
//   3. The `LEA ECX, [ECX+0]` at offset 0x3d is a 3-byte NOP for branch-
//      target alignment (the `loop:` label that follows). MSVC routinely
//      inserts these.
//
// Iteration history:
//
//   #1 [PARTIAL — 112/116 (-4 B), 64 mismatches]
//      Naive `while (*p) ++p; len = p - data;` strlen. MSVC compiled
//      the pre-test loop as `CMP [EBX], 0; JZ skip; loop: ...`. Orig
//      uses a post-test loop with LEA-saved start.
//
//   #2 [PARTIAL — 109/116 (-7 B), 57 mismatches]  ← committed
//      do-while strlen `do { c = *p++; } while (c); len = (p-data)-1;`
//      Got mine 3 bytes shorter — the strlen body does match orig's
//      post-test shape, but the new gap is MSVC scheduling MOV ECX, 1
//      AFTER the EDI load + CMP, while orig schedules MOV EDX, 1
//      BEFORE the EDI load. This propagates a register-color shift
//      through the byte stores (MOV [ESI+0x10], CL vs DL etc.) for
//      most of the function body.
//
// Why DEFERRED: the structural decode is correct and the Utf8String
// LAYOUT is recovered (the actual deliverable for unblocking
// PackRead::ProcessChunk). Closing the remaining 57-byte gap would
// require coaxing MSVC to:
//   - Schedule the constant `1` load BEFORE the arg2 load, AND
//   - Pick EDX over ECX for that constant.
//
// Both are register-allocator heuristics with no clean C++ source
// trigger. Likely needs `__declspec(naked)` or experimental flag-
// twiddling. Deferring further iterations — the layout finding is
// the unblocker for ProcessChunk and other Utf8String-using callers.

// Class layout lives in include/sqex/Utf8String.h so callers (e.g.
// PackRead::ProcessChunk) can stack-allocate Utf8String correctly.
#include "../../../include/sqex/Utf8String.h"

// `extern "C"` to suppress C++ name mangling on the memcpy reloc —
// the orig CALL goes to the CRT memcpy at RVA 0x005d5110.
extern "C" void *memcpy(void *dst, const void *src, unsigned n);

Utf8String::Utf8String(const char *data, unsigned length) {
    m_flag_10 = 1;
    m_flag_11 = 1;
    m_field_c = 0;
    m_size    = 1;
    m_capacity = 0x40;
    m_data    = m_inline_buf;
    m_inline_buf[0] = 0;           // null-terminate the inline buf

    if (length == 0xffffffff) {
        // -1 sentinel → compute strlen(data)
        // Use a do-while with pre-incremented pointer to match orig's
        // post-test loop shape (`EAX += 1; TEST CL, CL; JNZ loop`).
        const char *p = data;
        char c;
        do { c = *p++; } while (c);
        length = (unsigned)(p - data) - 1;
    }

    Reserve(length + 1, 1);
    memcpy(m_data, data, length);
    m_data[length] = 0;
}
