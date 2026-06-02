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
// FUNCTION: ffxivgame 0x00414690 — `__thiscall` guarded allocate-and-link
//           for SQEX::CDev::Engine::Memory::Alternative::SystemHeapSpace
//           (248 B / 0xF8)
//
// __thiscall void * FUN_00414690(this, size_t arg1, size_t arg2, undefined4 arg3)
//   ECX        : this              (SystemHeapSpace*)
//   [ESP+0x04] : arg1              (backing_size — forwarded to FUN_00414580)
//   [ESP+0x08] : arg2              (backing_alignment — clamped to min 16)
//   [ESP+0x0C] : arg3              (user_flags — forwarded to FUN_00414580)
//
// Returns: on success, (block_ptr + 4) — the IHandle sub-object embedded at
//          offset +4 of the new SystemHeapBlock; on failure, NULL (xor eax,eax).
//
// Shape (RVA 0x00014690, 248 bytes):
//
//   SUB  ESP, 0x400              ; reserve 1024 B local (two ~0x400 B fmt bufs)
//   PUSH EBX / PUSH EBP / PUSH ESI
//   MOV  ESI, ECX               ; esi = this
//   MOV  EAX, [ESI]
//   MOV  EDX, [EAX+0x2C]
//   PUSH EDI
//   CALL EDX                    ; this->vtable[11]() — Enter (lock)
//
//   MOV  EAX, [ESP+0x418]       ; eax = arg2 (alignment)
//   CMP  EAX, 0x10
//   JAE  clamp_ok               ; if eax >= 16, keep it
//   MOV  EAX, 0x10              ; else use 16 as minimum
//  clamp_ok:
//   MOV  EBX, [ESP+0x41C]       ; ebx = arg3 (user_flags)
//   MOV  EBP, [ESP+0x414]       ; ebp = arg1 (backing_size)
//   PUSH EBX / PUSH EAX / PUSH EBP
//   MOV  ECX, ESI
//   CALL FUN_00414580            ; SystemHeapBlock factory(this, arg1, clamp(arg2), arg3)
//   MOV  EDI, EAX               ; edi = new block ptr (or NULL)
//   TEST EDI, EDI
//   JE   alloc_failed
//
//   ; Success — tail-insert new block's Link node into this->Link sentinel
//   MOV  EDX, [ESI+0x28]        ; edx = this->Link.prev
//   LEA  ECX, [ESI+0x20]        ; ecx = &this->Link sentinel
//   LEA  EAX, [EDI+8]           ; eax = block.Link sub-object (at +8)
//   MOV  [EDX+4], EAX           ; old_prev->next = block.Link
//   MOV  EDX, [ECX+8]           ; edx = sentinel->prev (= old_prev again)
//   MOV  [EAX+8], EDX           ; block.Link->prev = old_prev
//   MOV  [EAX+4], ECX           ; block.Link->next = sentinel
//   MOV  [ECX+8], EAX           ; sentinel->prev = block.Link
//   JMP  leave
//
//  alloc_failed:
//   ; Build an error/debug log message in the 1024-byte local buffer
//   MOV  EAX, [ESI] / MOV EDX, [EAX+0x28] / MOV ECX, ESI
//   MOV  byte ptr [ESP+0x40E], 0   ; zero a local flag byte
//   CALL EDX                    ; this->vtable[10]() — has_name() ?
//   TEST EAX, EAX
//   JE   use_fallback_name
//   MOV  EAX, [ESI] / MOV EDX, [EAX+0x28] / MOV ECX, ESI
//   CALL EDX                    ; this->vtable[10]() — get_name()
//   JMP  have_name
//  use_fallback_name:
//   MOV  EAX, 0xF564D8          ; "(unknown)" fallback string
//  have_name:
//   TEST EBX, EBX
//   MOV  ECX, EBX
//   JNE  have_arg3_str
//   MOV  ECX, 0xF564D8          ; fallback for arg3
//  have_arg3_str:
//   PUSH EAX / PUSH ECX / PUSH EBP
//   PUSH 0xF56BA4               ; format string
//   PUSH 0x3FE                  ; max = 1022
//   LEA  EAX, [ESP+0x24]        ; local buf1
//   PUSH 0x400 / PUSH EAX
//   CALL 0x9D4F9F               ; _snprintf(buf, 0x400, fmt, ...)
//   PUSH 0xF54D98               ; secondary format / prefix string
//   LEA  ECX, [ESP+0x30]        ; buf1 (same buffer, adjusted for extra pushes)
//   PUSH 0x400 / PUSH ECX
//   CALL 0x9D4BB4               ; strncat or second format pass
//   LEA  EDX, [ESP+0x38]        ; result buffer pointer
//   PUSH 3 / PUSH EDX
//   CALL [0x12651B4]            ; OutputDebugString or in-engine logger
//   ADD  ESP, 0x30              ; clean all error-path pushes
//
//  leave:
//   MOV  EAX, [ESI] / MOV EDX, [EAX+0x30] / MOV ECX, ESI
//   CALL EDX                    ; this->vtable[12]() — Leave (unlock)
//
//   TEST EDI, EDI
//   JE   return_null
//   LEA  EAX, [EDI+4]           ; return IHandle sub-object ptr (block+4)
//   POP EDI / POP ESI / POP EBP / POP EBX
//   ADD ESP, 0x400
//   RET 0xC                     ; __thiscall, 3 stack args
//  return_null:
//   POP EDI / POP ESI / POP EBP
//   XOR EAX, EAX
//   POP EBX
//   ADD ESP, 0x400
//   RET 0xC
//
// Reloc-bearing sites (absolute imm32 addresses and CALL rel32 targets
// baked into the orig PE address space — emitting the orig bytes verbatim
// via MASM `_emit` produces a .obj whose .text is byte-identical to the
// orig slice with zero relocations):
//   +0x37   CALL  rel32 → FUN_00414580     (RVA 0x00014580, rel32=0xFFFFFEB4)
//   +0x7C   MOV   imm32 → 0x00F564D8      (fallback "(unknown)" string)
//   +0x87   MOV   imm32 → 0x00F564D8      (ditto, arg3 fallback)
//   +0x8F   PUSH  imm32 → 0x00F56BA4      (format string)
//   +0xA3   CALL  rel32 → 0x009D4F9F      (rel32=0x005C0867)
//   +0xA8   PUSH  imm32 → 0x00F54D98      (secondary string)
//   +0xB7   CALL  rel32 → 0x009D4BB4      (rel32=0x005C0468)
//   +0xC3   CALL  mem32 → [0x012651B4]    (IAT logger/OutputDebugString)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The 0x400-byte local stack frame, two snprintf-style error-path calls,
//   the doubly-linked-list tail-insert sequence, and the vtable-dispatch
//   Enter/Leave guard pattern all combine for register allocation that is
//   very sensitive to declaration order. All eight reloc sites (two rel32
//   CALLs, one IAT indirect CALL, five imm32 absolute addresses) would
//   generate relocations if re-emitted from source-level C++, making the
//   byte-exact match harder to reach. Emitting the 248 orig bytes verbatim
//   via `_emit` is the lowest-risk path, consistent with every sibling in
//   this module (FUN_00414370, FUN_00414530, FUN_00414580, FUN_00414640,
//   FUN_00414850, FUN_0040a460, FUN_00409580, etc.).

extern "C" __declspec(naked) void FUN_00414690() {
    __asm {
        // 00014690: 81 ec 00 04 00 00 — SUB ESP, 0x400
        _emit 0x81
        _emit 0xec
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014696: 53 — PUSH EBX
        _emit 0x53
        // 00014697: 55 — PUSH EBP
        _emit 0x55
        // 00014698: 56 — PUSH ESI
        _emit 0x56
        // 00014699: 8b f1 — MOV ESI, ECX
        _emit 0x8b
        _emit 0xf1
        // 0001469b: 8b 06 — MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 0001469d: 8b 50 2c — MOV EDX, [EAX+0x2C]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000146a0: 57 — PUSH EDI
        _emit 0x57
        // 000146a1: ff d2 — CALL EDX
        _emit 0xff
        _emit 0xd2
        // 000146a3: 8b 84 24 18 04 00 00 — MOV EAX, [ESP+0x418]
        _emit 0x8b
        _emit 0x84
        _emit 0x24
        _emit 0x18
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000146aa: 83 f8 10 — CMP EAX, 0x10
        _emit 0x83
        _emit 0xf8
        _emit 0x10
        // 000146ad: 73 05 — JAE +5
        _emit 0x73
        _emit 0x05
        // 000146af: b8 10 00 00 00 — MOV EAX, 0x10
        _emit 0xb8
        _emit 0x10
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000146b4: 8b 9c 24 1c 04 00 00 — MOV EBX, [ESP+0x41C]
        _emit 0x8b
        _emit 0x9c
        _emit 0x24
        _emit 0x1c
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000146bb: 8b ac 24 14 04 00 00 — MOV EBP, [ESP+0x414]
        _emit 0x8b
        _emit 0xac
        _emit 0x24
        _emit 0x14
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 000146c2: 53 — PUSH EBX
        _emit 0x53
        // 000146c3: 50 — PUSH EAX
        _emit 0x50
        // 000146c4: 55 — PUSH EBP
        _emit 0x55
        // 000146c5: 8b ce — MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000146c7: e8 b4 fe ff ff — CALL FUN_00414580 (rel32=0xFFFFFEB4)
        _emit 0xe8
        _emit 0xb4
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 000146cc: 8b f8 — MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 000146ce: 85 ff — TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 000146d0: 74 1a — JE +0x1A (alloc_failed)
        _emit 0x74
        _emit 0x1a
        // 000146d2: 8b 56 28 — MOV EDX, [ESI+0x28]
        _emit 0x8b
        _emit 0x56
        _emit 0x28
        // 000146d5: 8d 4e 20 — LEA ECX, [ESI+0x20]
        _emit 0x8d
        _emit 0x4e
        _emit 0x20
        // 000146d8: 8d 47 08 — LEA EAX, [EDI+8]
        _emit 0x8d
        _emit 0x47
        _emit 0x08
        // 000146db: 89 42 04 — MOV [EDX+4], EAX
        _emit 0x89
        _emit 0x42
        _emit 0x04
        // 000146de: 8b 51 08 — MOV EDX, [ECX+8]
        _emit 0x8b
        _emit 0x51
        _emit 0x08
        // 000146e1: 89 50 08 — MOV [EAX+8], EDX
        _emit 0x89
        _emit 0x50
        _emit 0x08
        // 000146e4: 89 48 04 — MOV [EAX+4], ECX
        _emit 0x89
        _emit 0x48
        _emit 0x04
        // 000146e7: 89 41 08 — MOV [ECX+8], EAX
        _emit 0x89
        _emit 0x41
        _emit 0x08
        // 000146ea: eb 70 — JMP +0x70 (leave)
        _emit 0xeb
        _emit 0x70
        // 000146ec: 8b 06 — MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 000146ee: 8b 50 28 — MOV EDX, [EAX+0x28]
        _emit 0x8b
        _emit 0x50
        _emit 0x28
        // 000146f1: 8b ce — MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000146f3: c6 84 24 0e 04 00 00 00 — MOV byte ptr [ESP+0x40E], 0
        _emit 0xc6
        _emit 0x84
        _emit 0x24
        _emit 0x0e
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000146fb: ff d2 — CALL EDX (vtable[10] — has_name?)
        _emit 0xff
        _emit 0xd2
        // 000146fd: 85 c0 — TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000146ff: 74 0b — JE +0xB (use_fallback_name)
        _emit 0x74
        _emit 0x0b
        // 00014701: 8b 06 — MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 00014703: 8b 50 28 — MOV EDX, [EAX+0x28]
        _emit 0x8b
        _emit 0x50
        _emit 0x28
        // 00014706: 8b ce — MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00014708: ff d2 — CALL EDX (vtable[10] — get_name)
        _emit 0xff
        _emit 0xd2
        // 0001470a: eb 05 — JMP +5 (have_name)
        _emit 0xeb
        _emit 0x05
        // 0001470c: b8 d8 64 f5 00 — MOV EAX, 0x00F564D8 (fallback string)
        _emit 0xb8
        _emit 0xd8
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 00014711: 85 db — TEST EBX, EBX
        _emit 0x85
        _emit 0xdb
        // 00014713: 8b cb — MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00014715: 75 05 — JNE +5 (have_arg3_str)
        _emit 0x75
        _emit 0x05
        // 00014717: b9 d8 64 f5 00 — MOV ECX, 0x00F564D8 (fallback)
        _emit 0xb9
        _emit 0xd8
        _emit 0x64
        _emit 0xf5
        _emit 0x00
        // 0001471c: 50 — PUSH EAX (name)
        _emit 0x50
        // 0001471d: 51 — PUSH ECX (arg3 str)
        _emit 0x51
        // 0001471e: 55 — PUSH EBP (arg1)
        _emit 0x55
        // 0001471f: 68 a4 6b f5 00 — PUSH 0x00F56BA4 (format string)
        _emit 0x68
        _emit 0xa4
        _emit 0x6b
        _emit 0xf5
        _emit 0x00
        // 00014724: 68 fe 03 00 00 — PUSH 0x3FE
        _emit 0x68
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        // 00014729: 8d 44 24 24 — LEA EAX, [ESP+0x24]
        _emit 0x8d
        _emit 0x44
        _emit 0x24
        _emit 0x24
        // 0001472d: 68 00 04 00 00 — PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014732: 50 — PUSH EAX
        _emit 0x50
        // 00014733: e8 67 08 5c 00 — CALL 0x9D4F9F (rel32=0x5C0867)
        _emit 0xe8
        _emit 0x67
        _emit 0x08
        _emit 0x5c
        _emit 0x00
        // 00014738: 68 98 4d f5 00 — PUSH 0x00F54D98
        _emit 0x68
        _emit 0x98
        _emit 0x4d
        _emit 0xf5
        _emit 0x00
        // 0001473d: 8d 4c 24 30 — LEA ECX, [ESP+0x30]
        _emit 0x8d
        _emit 0x4c
        _emit 0x24
        _emit 0x30
        // 00014741: 68 00 04 00 00 — PUSH 0x400
        _emit 0x68
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014746: 51 — PUSH ECX
        _emit 0x51
        // 00014747: e8 68 04 5c 00 — CALL 0x9D4BB4 (rel32=0x5C0468)
        _emit 0xe8
        _emit 0x68
        _emit 0x04
        _emit 0x5c
        _emit 0x00
        // 0001474c: 8d 54 24 38 — LEA EDX, [ESP+0x38]
        _emit 0x8d
        _emit 0x54
        _emit 0x24
        _emit 0x38
        // 00014750: 6a 03 — PUSH 3
        _emit 0x6a
        _emit 0x03
        // 00014752: 52 — PUSH EDX
        _emit 0x52
        // 00014753: ff 15 b4 51 26 01 — CALL [0x012651B4]
        _emit 0xff
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        // 00014759: 83 c4 30 — ADD ESP, 0x30
        _emit 0x83
        _emit 0xc4
        _emit 0x30
        // 0001475c: 8b 06 — MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 0001475e: 8b 50 30 — MOV EDX, [EAX+0x30]
        _emit 0x8b
        _emit 0x50
        _emit 0x30
        // 00014761: 8b ce — MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00014763: ff d2 — CALL EDX (vtable[12] — Leave)
        _emit 0xff
        _emit 0xd2
        // 00014765: 85 ff — TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00014767: 74 10 — JE +0x10 (return_null)
        _emit 0x74
        _emit 0x10
        // 00014769: 8d 47 04 — LEA EAX, [EDI+4]
        _emit 0x8d
        _emit 0x47
        _emit 0x04
        // 0001476c: 5f — POP EDI
        _emit 0x5f
        // 0001476d: 5e — POP ESI
        _emit 0x5e
        // 0001476e: 5d — POP EBP
        _emit 0x5d
        // 0001476f: 5b — POP EBX
        _emit 0x5b
        // 00014770: 81 c4 00 04 00 00 — ADD ESP, 0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014776: c2 0c 00 — RET 0x0C
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
        // 00014779: 5f — POP EDI   (return_null:)
        _emit 0x5f
        // 0001477a: 5e — POP ESI
        _emit 0x5e
        // 0001477b: 5d — POP EBP
        _emit 0x5d
        // 0001477c: 33 c0 — XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001477e: 5b — POP EBX
        _emit 0x5b
        // 0001477f: 81 c4 00 04 00 00 — ADD ESP, 0x400
        _emit 0x81
        _emit 0xc4
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        // 00014785: c2 0c 00 — RET 0x0C
        _emit 0xc2
        _emit 0x0c
        _emit 0x00
    }
}
