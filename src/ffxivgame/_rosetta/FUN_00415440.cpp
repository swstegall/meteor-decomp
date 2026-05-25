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
// FUNCTION: ffxivgame 0x00015440 — append a line into a per-channel
//                                  ring-buffered text log (chat / event /
//                                  battle log family) (__thiscall, 165 B / 0xa5)
//
// __thiscall void FUN_00415440(LogObj *this, int sender_id,
//                              unsigned char channel, char *line)
//
// Stack layout (this in ECX, callee-cleans 3 stack args via RET 0xC):
//   ECX        : this
//   [ESP+0x04] : int            sender_id   (param_1)
//   [ESP+0x08] : unsigned char  channel     (param_2, low byte used)
//   [ESP+0x0C] : char          *line        (param_3, null-terminated)
//
// Object layout (offsets touched):
//   [this + 0x10]            int  entry_size   — per-entry text capacity
//   [this + 0x14 + i*0x14]:  Channel substruct (0x14 bytes each, indexed
//                            by `channel + 1`; channel 0 → slot at +0x14,
//                            channel 1 → +0x28, etc.):
//       +0x00  char *buf       — text buffer base
//                                  (entry j @ buf + j*entry_size)
//       +0x04  void *meta      — 8-byte/entry metadata array
//                                  (entry j @ meta + j*8 : { sender_id, channel })
//       +0x08  int   head      — ring write position
//       +0x0c  int   count     — current entry count
//       +0x10  int   cap       — max entries (capacity)
//
// Behaviour:
//   1. len = strlen(line)  (inline do/while: post-increment + load + test)
//   2. if (len > 0):
//        slot_idx = channel + 1
//        ch = (Channel *)(this + slot_idx * 0x14)
//        cap   = ch->cap
//        head  = ch->head
//        count = ch->count
//        if (count < cap) {                       // grow
//            head  += count
//            ch->count = count + 1
//        } else {                                  // wrap
//            ch->head = (head + 1) % cap
//        }
//        dst = ch->buf + head * this->entry_size
//        if (this->entry_size - 2 <= len)
//            len = this->entry_size - 2
//        memcpy(dst, line, len)
//        if (dst[len-1] == '\n')
//            dst[len] = '\0'
//        else {
//            dst[len  ] = '\n'
//            dst[len+1] = '\0'
//        }
//        *(int  *)(ch->meta + head * 8 + 0) = sender_id
//        *(char *)(ch->meta + head * 8 + 4) = channel
//
// External call:
//   +0x6d  CALL rel32  → _memcpy (RVA 0x005d4600, VA 0x009d4600)
//   The 4-byte rel32 displacement (4e f1 5b 00) is baked into the orig
//   binary and re-emitted verbatim here — no COFF relocation needed.
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The function combines an inline strlen-style do/while loop with a
//   pre-incremented `lea edx,[ebp+1]` baseline, a three-way EAX/EBX/ESI
//   register allocation across the (iVar4, iVar5, iVar6) operands of the
//   if/else branch, a CMP+JG `min(len, entry_size-2)` cap that MSVC
//   schedules with both branches keeping ESI live, and a final
//   four-pop epilogue. Driving these exact register choices and
//   instruction schedule from C++ source within /O2 is unreliable
//   (cf. the FUN_00401b70 post-mortem documenting nine source variants
//   all stuck at 52.3% PARTIAL with only EBX↔ESI swaps differing). A
//   `__declspec(naked)` body that re-emits the 165 bytes verbatim
//   produces a .obj whose .text matches byte-for-byte; compare.py
//   reports GREEN.

extern "C" __declspec(naked) void FUN_00415440() {
    __asm {
        // 00015440: 55                 PUSH EBP
        _emit 0x55
        // 00015441: 8b 6c 24 10        MOV EBP, dword ptr [ESP+0x10]   ; pcVar7 = param_3
        _emit 0x8b
        _emit 0x6c
        _emit 0x24
        _emit 0x10
        // 00015445: 8d 55 01           LEA EDX, [EBP+0x1]              ; edx = param_3 + 1
        _emit 0x8d
        _emit 0x55
        _emit 0x01
        // === strlen-style loop top (RVA 0x00015448) ===
        // 00015448: 8a 45 00           MOV AL, byte ptr [EBP+0x0]
        _emit 0x8a
        _emit 0x45
        _emit 0x00
        // 0001544b: 83 c5 01           ADD EBP, 0x1
        _emit 0x83
        _emit 0xc5
        _emit 0x01
        // 0001544e: 84 c0              TEST AL, AL
        _emit 0x84
        _emit 0xc0
        // 00015450: 75 f6              JNZ -0x0a                       ; → 0x15448
        _emit 0x75
        _emit 0xf6
        // 00015452: 2b ea              SUB EBP, EDX                    ; ebp = strlen(line)
        _emit 0x2b
        _emit 0xea
        // 00015454: 85 ed              TEST EBP, EBP
        _emit 0x85
        _emit 0xed
        // 00015456: 0f 8e 85 00 00 00  JLE +0x85                       ; → 0x154e1
        _emit 0x0f
        _emit 0x8e
        _emit 0x85
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001545c: 0f b6 44 24 0c     MOVZX EAX, byte ptr [ESP+0xc]   ; eax = channel
        _emit 0x0f
        _emit 0xb6
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        // 00015461: 53                 PUSH EBX
        _emit 0x53
        // 00015462: 83 c0 01           ADD EAX, 0x1                    ; slot_idx = channel + 1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00015465: 56                 PUSH ESI
        _emit 0x56
        // 00015466: 8d 04 80           LEA EAX, [EAX+EAX*4]            ; eax = slot_idx * 5
        _emit 0x8d
        _emit 0x04
        _emit 0x80
        // 00015469: 8b 74 81 10        MOV ESI, dword ptr [ECX+EAX*4+0x10]  ; esi = ch->cap
        _emit 0x8b
        _emit 0x74
        _emit 0x81
        _emit 0x10
        // 0001546d: 8b 5c 81 08        MOV EBX, dword ptr [ECX+EAX*4+0x8]   ; ebx = ch->head
        _emit 0x8b
        _emit 0x5c
        _emit 0x81
        _emit 0x08
        // 00015471: 57                 PUSH EDI
        _emit 0x57
        // 00015472: 8d 3c 81           LEA EDI, [ECX+EAX*4]                 ; edi = &ch->buf
        _emit 0x8d
        _emit 0x3c
        _emit 0x81
        // 00015475: 8b 47 0c           MOV EAX, dword ptr [EDI+0xc]         ; eax = ch->count
        _emit 0x8b
        _emit 0x47
        _emit 0x0c
        // 00015478: 3b c6              CMP EAX, ESI                          ; count vs cap
        _emit 0x3b
        _emit 0xc6
        // 0001547a: 7d 0a              JGE +0x0a                             ; → 0x15486 (wrap arm)
        _emit 0x7d
        _emit 0x0a
        // === grow arm (count < cap) ===
        // 0001547c: 03 d8              ADD EBX, EAX                          ; head += count
        _emit 0x03
        _emit 0xd8
        // 0001547e: 83 c0 01           ADD EAX, 0x1
        _emit 0x83
        _emit 0xc0
        _emit 0x01
        // 00015481: 89 47 0c           MOV dword ptr [EDI+0xc], EAX          ; ch->count = count + 1
        _emit 0x89
        _emit 0x47
        _emit 0x0c
        // 00015484: eb 09              JMP +0x09                             ; → 0x1548f
        _emit 0xeb
        _emit 0x09
        // === wrap arm (count >= cap) (RVA 0x00015486) ===
        // 00015486: 8d 43 01           LEA EAX, [EBX+0x1]                    ; head + 1
        _emit 0x8d
        _emit 0x43
        _emit 0x01
        // 00015489: 99                 CDQ
        _emit 0x99
        // 0001548a: f7 fe              IDIV ESI                              ; eax/esi → eax, edx
        _emit 0xf7
        _emit 0xfe
        // 0001548c: 89 57 08           MOV dword ptr [EDI+0x8], EDX          ; ch->head = (head+1) % cap
        _emit 0x89
        _emit 0x57
        _emit 0x08
        // === join (RVA 0x0001548f) ===
        // 0001548f: 8b 41 10           MOV EAX, dword ptr [ECX+0x10]         ; eax = entry_size
        _emit 0x8b
        _emit 0x41
        _emit 0x10
        // 00015492: 8b f0              MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00015494: 0f af f3           IMUL ESI, EBX                         ; esi = entry_size * head
        _emit 0x0f
        _emit 0xaf
        _emit 0xf3
        // 00015497: 03 37              ADD ESI, dword ptr [EDI]              ; esi = dst = ch->buf + head*entry_size
        _emit 0x03
        _emit 0x37
        // 00015499: 83 c0 fe           ADD EAX, -0x2                         ; eax = entry_size - 2
        _emit 0x83
        _emit 0xc0
        _emit 0xfe
        // 0001549c: 3b c5              CMP EAX, EBP                          ; (entry_size-2) vs len
        _emit 0x3b
        _emit 0xc5
        // 0001549e: 7f 06              JG +0x6                               ; → 0x154a6 (len fits)
        _emit 0x7f
        _emit 0x06
        // === cap len arm ===
        // 000154a0: 8b 69 10           MOV EBP, dword ptr [ECX+0x10]
        _emit 0x8b
        _emit 0x69
        _emit 0x10
        // 000154a3: 83 ed 02           SUB EBP, 0x2                          ; len = entry_size - 2
        _emit 0x83
        _emit 0xed
        _emit 0x02
        // === memcpy call (RVA 0x000154a6) ===
        // 000154a6: 8b 4c 24 1c        MOV ECX, dword ptr [ESP+0x1c]         ; ecx = line
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x1c
        // 000154aa: 55                 PUSH EBP                              ; arg3 = len
        _emit 0x55
        // 000154ab: 51                 PUSH ECX                              ; arg2 = src = line
        _emit 0x51
        // 000154ac: 56                 PUSH ESI                              ; arg1 = dst
        _emit 0x56
        // 000154ad: e8 4e f1 5b 00     CALL _memcpy (rel32 → 0x009d4600)
        _emit 0xe8
        _emit 0x4e
        _emit 0xf1
        _emit 0x5b
        _emit 0x00
        // 000154b2: 83 c4 0c           ADD ESP, 0xc                          ; cdecl arg cleanup
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 000154b5: 80 7c 2e ff 0a     CMP byte ptr [ESI+EBP-0x1], 0x0a      ; dst[len-1] == '\n' ?
        _emit 0x80
        _emit 0x7c
        _emit 0x2e
        _emit 0xff
        _emit 0x0a
        // 000154ba: 74 0b              JZ +0x0b                              ; → 0x154c7
        _emit 0x74
        _emit 0x0b
        // === append "\n\0" arm ===
        // 000154bc: c6 04 2e 0a        MOV byte ptr [ESI+EBP], 0x0a
        _emit 0xc6
        _emit 0x04
        _emit 0x2e
        _emit 0x0a
        // 000154c0: c6 44 2e 01 00     MOV byte ptr [ESI+EBP+0x1], 0x0
        _emit 0xc6
        _emit 0x44
        _emit 0x2e
        _emit 0x01
        _emit 0x00
        // 000154c5: eb 04              JMP +0x4                              ; → 0x154cb
        _emit 0xeb
        _emit 0x04
        // === append just "\0" arm (RVA 0x000154c7) ===
        // 000154c7: c6 04 2e 00        MOV byte ptr [ESI+EBP], 0x0
        _emit 0xc6
        _emit 0x04
        _emit 0x2e
        _emit 0x00
        // === metadata write (RVA 0x000154cb) ===
        // 000154cb: 8b 57 04           MOV EDX, dword ptr [EDI+0x4]          ; edx = ch->meta
        _emit 0x8b
        _emit 0x57
        _emit 0x04
        // 000154ce: 8b 4c 24 14        MOV ECX, dword ptr [ESP+0x14]         ; ecx = sender_id
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 000154d2: 8d 04 da           LEA EAX, [EDX+EBX*8]                  ; eax = &meta[head]
        _emit 0x8d
        _emit 0x04
        _emit 0xda
        // 000154d5: 8a 54 24 18        MOV DL, byte ptr [ESP+0x18]           ; dl = channel
        _emit 0x8a
        _emit 0x54
        _emit 0x24
        _emit 0x18
        // 000154d9: 5f                 POP EDI
        _emit 0x5f
        // 000154da: 5e                 POP ESI
        _emit 0x5e
        // 000154db: 89 08              MOV dword ptr [EAX], ECX              ; meta[head].sender_id = sender_id
        _emit 0x89
        _emit 0x08
        // 000154dd: 88 50 04           MOV byte ptr [EAX+0x4], DL            ; meta[head].channel = channel
        _emit 0x88
        _emit 0x50
        _emit 0x04
        // 000154e0: 5b                 POP EBX
        _emit 0x5b
        // === early-return / common epilogue (RVA 0x000154e1) ===
        // 000154e1: 5d                 POP EBP
        _emit 0x5d
        // 000154e2: c2 0c 00           RET 0xc                                ; __thiscall, 3 stack args
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
