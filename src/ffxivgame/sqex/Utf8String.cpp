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
//   #2 [PARTIAL — 109/116 (-7 B), 57 mismatches]
//      do-while strlen `do { c = *p++; } while (c); len = (p-data)-1;`
//      Got mine 3 bytes shorter — the strlen body does match orig's
//      post-test shape, but the new gap is MSVC scheduling MOV ECX, 1
//      AFTER the EDI load + CMP, while orig schedules MOV EDX, 1
//      BEFORE the EDI load. This propagates a register-color shift
//      through the byte stores (MOV [ESI+0x10], CL vs DL etc.) for
//      most of the function body.
//
//   #3 [PARTIAL — 115/116 (-1 B), 2026-05-02]  ← current
//      Pre-saved `start_plus1 = data + 1` matches orig's
//      `LEA EDI, [EAX+1]` setup. Got us within 1 byte of GREEN.
//      Remaining gap: MSVC reuses EDX=1 in the strlen loop's
//      increment (`ADD EAX, EDX` = 2 bytes) where orig emits
//      `ADD EAX, 1` (3 bytes). MSVC's CSE is smart enough to
//      observe EDX still holds the constant 1 from the field-init
//      stores; orig's MSVC didn't make that connection. Tried
//      `volatile` reads to clobber EDX-=1 — didn't help; MSVC
//      still kept the constant tracked.
//
// Why DEFERRED at 115/116: the remaining 1-byte gap is MSVC's
// CSE optimization (reuse EDX=1 across both the field stores and
// the loop increment). No C-source-level way to defeat this
// without inline asm. The structural decode is correct and matches
// orig in every other respect.

// Class layout lives in include/sqex/Utf8String.h so callers (e.g.
// PackRead::ProcessChunk) can stack-allocate Utf8String correctly.
#include "../../../include/sqex/Utf8String.h"

// `extern "C"` to suppress C++ name mangling on the memcpy reloc —
// the orig CALL goes to the CRT memcpy at RVA 0x005d5110.
extern "C" void *memcpy(void *dst, const void *src, unsigned n);

// External cdecl helper at RVA 0x0004d350 — frees m_data when the
// string is in heap mode. Signature `(void *data, int capacity, int
// flag=0xb)` per the args pushed by the destructor below.
extern "C" void Utf8StringFree(void *data, int capacity, int flag);

// External cdecl helper at RVA 0x0004d500 — counterpart for
// allocation. Args: (alloc_class=0xb, zero_fill=0, size). Returns
// pointer to new buffer.
// Utf8StringAlloc actually only reads its first arg (size); the other
// two are caller-side cdecl extras that the callee ignores.
extern "C" void *Utf8StringAlloc(unsigned size, int /*unused*/, int /*unused*/);

// FUNCTION: ffxivgame 0x00046f50 — Sqex::Misc::Utf8String::~Utf8String (24 B)
//
// Original bytes:
//   00046f50: 80 79 11 00 75 11 8b 41 04 8b 09 6a 0b 50 51 e8
//   00046f60: ?? ?? ?? ?? 83 c4 0c c3
//
// Decoded:
//   CMP byte ptr [ECX+0x11], 0     ; m_flag_11 == 0?
//   JNZ +0x11 (= 0x00046f67)        ; if (m_flag_11 != 0) → return
//   MOV EAX, [ECX+4]                ; EAX = m_capacity
//   MOV ECX, [ECX]                  ; ECX = m_data
//   PUSH 0xb                         ; arg3: allocator-class flag
//   PUSH EAX                         ; arg2: capacity
//   PUSH ECX                         ; arg1: data
//   CALL Utf8StringFree              ; cdecl (orig RVA 0x0004d350)
//   ADD ESP, 0xc                     ; clean 3 args
//   RET
//
// Semantics: when m_flag_11 == 1 (SSO, set by ctor), do nothing —
// data lives in m_inline_buf and there's no heap to free. When
// m_flag_11 == 0 (after Reserve grew the string past 0x40 bytes),
// call the heap-class-aware free helper. The MSVC clever bit:
// uses two PUSH-reg ops (EAX, ECX) instead of the equivalent stack
// `[ECX+...]` reads — saves ~6 bytes vs a direct PUSH `[ECX+4]`.
Utf8String::~Utf8String() {
    if (m_flag_11 == 0) {
        Utf8StringFree(m_data, m_capacity, 0xb);
    }
}

// FUNCTION: ffxivgame 0x00047010 — Sqex::Misc::Utf8String::Reserve (153 B)
//
// Ensures m_data has capacity >= `size` bytes. If `size` exceeds the
// current m_capacity, allocates a new buffer (rounded up to a 32-byte
// alignment), copies the existing m_size bytes if old data was non-
// null, frees the old heap buffer if `m_flag_11 == 0` (heap mode),
// updates the fields, and marks the string as heap-owned.
//
// Original bytes (153 B):
//   00047010: 83 ec 08 80 7c 24 10 00 56 57 8b f1 74 04 c6 46
//   00047020: 10 00 8b 46 04 8b 7c 24 14 3b f8 89 44 24 0c 76
//   00047030: 6d 8b 46 08 53 55 8b 2e 83 c7 1f 6a 0b 83 e7 e0
//   00047040: 80 7e 11 00 6a 00 57 0f 94 44 24 2c 89 44 24 1c
//   00047050: e8 ?? ?? ?? ?? 83 c4 0c 85 ed 8b d8 74 26 8b 4c
//   00047060: 24 10 51 55 53 e8 ?? ?? ?? ?? 83 c4 0c 80 7c 24
//   00047070: 20 00 74 10 8b 54 24 14 6a 0b 52 55 e8 ?? ?? ??
//   00047080: ?? 83 c4 0c 8b 44 24 1c 5d 89 1e 5b 89 7e 04 5f
//   00047090: c6 46 11 00 89 46 08 5e 83 c4 08 c2 08 00 89 7e
//   000470a0: 08 5f 5e 83 c4 08 c2 08 00
//
// Decoded:
//   SUB  ESP, 8                          ; locals
//   CMP  byte ptr [ESP+0x10], 0          ; arg2 (small_ok) == 0?
//   PUSH ESI; PUSH EDI
//   MOV  ESI, ECX                         ; ESI = this
//   JZ   skip_clear_flag10                  ; (flags from CMP above)
//     MOV  byte ptr [ESI+0x10], 0          ; m_flag_10 = 0
//   skip_clear_flag10:
//   MOV  EAX, [ESI+4]                    ; EAX = m_capacity
//   MOV  EDI, [ESP+0x14]                 ; EDI = arg1 (size)
//   CMP  EDI, EAX
//   MOV  [ESP+0xc], EAX                   ; save capacity in local
//   JBE  fast_path                         ; if size <= capacity → skip grow
//   ; --- grow path ---
//   MOV  EAX, [ESI+8]                    ; EAX = m_size
//   PUSH EBX; PUSH EBP
//   MOV  EBP, [ESI]                       ; EBP = m_data (old)
//   ADD  EDI, 0x1f                        ; round size up to 32 alignment
//   PUSH 0xb                              ; arg1 to alloc: alloc class
//   AND  EDI, ~0x1f
//   CMP  byte ptr [ESI+0x11], 0           ; was SSO (m_flag_11 == 1)?
//   PUSH 0                                ; arg2 to alloc: zero_fill = 0
//   PUSH EDI                              ; arg3 to alloc: size
//   SETZ byte ptr [ESP+0x2c]              ; was_heap = (m_flag_11 == 0)
//   MOV  [ESP+0x1c], EAX                  ; save m_size
//   CALL Utf8StringAlloc                   ; cdecl → returns new buf in EAX
//   ADD  ESP, 0xc                         ; clean 3 args
//   TEST EBP, EBP                         ; old data null?
//   MOV  EBX, EAX                         ; EBX = new buffer
//   JZ   skip_copy_free                    ; if no old data, skip
//   ; --- copy old data into new buffer ---
//   MOV  ECX, [ESP+0x10]                  ; ECX = saved m_size
//   PUSH ECX                              ; bytes
//   PUSH EBP                              ; src (old data)
//   PUSH EBX                              ; dst (new buffer)
//   CALL memcpy
//   ADD  ESP, 0xc
//   CMP  byte ptr [ESP+0x20], 0           ; was_heap flag (saved by SETZ)
//   JZ   skip_copy_free                    ; if was SSO, no free needed
//   ; --- free old heap buffer ---
//   MOV  EDX, [ESP+0x14]                  ; old capacity
//   PUSH 0xb
//   PUSH EDX
//   PUSH EBP                              ; old data
//   CALL Utf8StringFree
//   ADD  ESP, 0xc
//   skip_copy_free:
//   MOV  EAX, [ESP+0x1c]                  ; reload m_size
//   POP  EBP
//   MOV  [ESI], EBX                       ; m_data = new buffer
//   POP  EBX
//   MOV  [ESI+4], EDI                     ; m_capacity = (rounded) new size
//   POP  EDI
//   MOV  byte ptr [ESI+0x11], 0           ; m_flag_11 = 0 (heap-owned)
//   MOV  [ESI+8], EAX                     ; m_size = saved
//   POP  ESI
//   ADD  ESP, 8
//   RET  8
//   ; --- fast path: size <= capacity ---
//   fast_path:
//   MOV  [ESI+8], EDI                     ; m_size = new size (in-place)
//   POP  EDI; POP ESI
//   ADD  ESP, 8
//   RET  8
//
// Iteration #1: best-effort C++ candidate. Closing this match
// to GREEN is ambitious (153 B with multi-branch + several CALLs
// + register/scheduling sensitivity).
// Iteration #2 (2026-05-02): tried do/while + goto-forward to flip
// fast/grow path layout, no change. The 9-byte gap is purely a
// MSVC reg-allocator choice — orig spills `saved_size` to a
// dedicated stack slot (`SUB ESP, 8`, 2 locals); mine keeps it in
// EBX (`PUSH ECX`, 1 local). Both are valid optimizations of
// equivalent C++; without inline asm there's no portable way to
// override MSVC's spill decision.
// Iteration #3 (2026-05-02): tried `volatile` on saved_size to
// force a stack spill — got 144→150 bytes (3 short of orig's 153)
// but the byte-match rate at the same offsets DROPPED from ~94 %
// (size-match metric) to ~17 % (strict-position byte-match metric)
// because the volatile forced extra MOV-through-memory writes
// throughout the function, shifting all downstream bytes. Net
// regression on real byte-match. Reverted.
// Iteration #4: also volatile on `was_heap` — went 8 bytes UNDER
// orig (worse on size). Reverted.
// Conclusion: the 9-byte SUB-ESP-vs-PUSH-ECX gap can't be closed
// via volatile alone — it triggers downstream spills that
// regress the body's byte match. Real GREEN would need either
// inline asm or a deeper structural rewrite that creates two
// "natural" stack-resident locals MSVC chooses to spill without
// being forced.
// Status: 144/153 size-match (~94 % size); body is structurally
// equivalent but reg-allocation divergent throughout.
void Utf8String::Reserve(unsigned size, int small_ok) {
    if (small_ok) {
        m_flag_10 = 0;
    }
    unsigned old_capacity = m_capacity;
    if (size <= old_capacity) {
        m_size = size;
        return;
    }
    int saved_size = m_size;
    char *old_data = m_data;
    size = (size + 0x1f) & ~0x1fu;          // round up to 32-byte alignment
    int was_heap = (m_flag_11 == 0);
    // Note: Utf8StringAlloc only reads its first arg (size). The other
    // two are caller-side leftovers — orig still pushes them as cdecl
    // expects 3, but the order is (size, 0, 0xb) NOT the inverse. This
    // matters for matching because cdecl pushes right-to-left, so the
    // PUSH order is "0xb, 0, size" in the asm.
    char *new_data = (char *)Utf8StringAlloc(size, 0, 0xb);
    if (old_data) {
        memcpy(new_data, old_data, saved_size);
        if (was_heap) {
            Utf8StringFree(old_data, old_capacity, 0xb);
        }
    }
    m_data     = new_data;
    m_capacity = (int)size;
    m_flag_11  = 0;
    m_size     = saved_size;
}

// FUNCTION: ffxivgame 0x00045cf0 — Utf8String::Utf8String() default (39 B)
//
// Recovered from byte pattern 2026-05-02. Initializes a Utf8String to
// SSO-empty state — same field values as the (data, length) ctor's
// initial setup, minus the data copy:
//   m_flag_10 = 1, m_flag_11 = 1, m_size = 1, m_field_c = 0,
//   m_capacity = 0x40, m_data = &m_inline_buf, m_inline_buf[0] = 0.
//
// MSVC emits a tight 39-byte sequence that piggybacks ECX=1 across the
// three fields that hold value 1 (flag_10, flag_11, m_size) before
// re-using ECX to hold &m_inline_buf for the data + null terminator.
//
// Original bytes (39 B):
//   00045cf0: 8b c1 b9 01 00 00 00 88 48 10 88 48 11 89 48 08
//   00045d00: 8d 48 12 c7 40 0c 00 00 00 00 c7 40 04 40 00 00
//   00045d10: 00 89 08 c6 01 00 c3
//
// Field-store order matters for the byte match. m_size must be set
// AFTER the flags (so CL=1 is reused) but BEFORE m_field_c (so MSVC
// can clobber ECX with the LEA).

Utf8String::Utf8String() {
    m_flag_10 = 1;
    m_flag_11 = 1;
    m_size    = 1;
    m_field_c = 0;
    m_capacity = 0x40;
    m_data    = (char *)m_inline_buf;
    ((char *)m_inline_buf)[0] = 0;
}

Utf8String::Utf8String(const char *data, unsigned length) {
    // Field-init stores. Order matters: the four "1" stores
    // (m_flag_10/_11/_size + the PUSH 1 to Reserve later) reuse
    // a single EDX=1 load across all four sites — orig saves bytes
    // via this trick.
    m_flag_10 = 1;
    m_flag_11 = 1;
    m_field_c = 0;
    m_size    = 1;
    m_capacity = 0x40;
    m_data    = (char *)m_inline_buf;
    ((char *)m_inline_buf)[0] = 0;  // null-terminate the inline buf

    if (length == 0xffffffff) {
        // -1 sentinel → compute strlen(data)
        // Match orig's exact pattern: pre-save `data + 1` so the
        // final subtraction gives strlen directly (orig does
        // `LEA EDI, [EAX+1]; ... SUB EAX, EDI`).
        const char *start_plus1 = data + 1;
        const char *p = data;
        char c;
        do {
            c = *p;
            ++p;
        } while (c);
        length = (unsigned)(p - start_plus1);
    }

    Reserve(length + 1, 1);
    memcpy(m_data, data, length);
    m_data[length] = 0;
}
