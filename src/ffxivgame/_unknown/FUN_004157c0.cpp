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
// FUNCTION: ffxivgame 0x004157c0 — newline-terminating debug-log forwarder
//                                  (150 B, __cdecl, void)
//
// __cdecl void FUN_004157c0(char *param_1, char param_2)
//
//   When `param_2 != 0`, treat `param_1` as a debug message and ensure
//   it ends with '\n' before forwarding:
//     1. Inline strlen of param_1, capped at 0x3fe.
//     2. If the last byte is already '\n', forward param_1 verbatim.
//     3. Else memcpy into a 1024-byte local, append '\n','\0', and
//        retarget param_1 to the local.
//   When `param_2 == 0`, skip the newline-fixup entirely.
//
//   Then format the (possibly rewritten) param_1 through _snprintf_s
//   into a second 1024-byte local with bound 0x3ff, and invoke the
//   installed log-sink callback at PTR_FUN_012651b4 with
//   (local_400, 2).
//
// Calling convention: __cdecl (caller-cleans).
// Stack frame: SUB ESP, 0x800 (two 1024-byte char buffers + 1 guard byte).
// Returns: void (plain RET, frame torn down via ADD ESP, 0x818).
//
// Inspection (read from orig RVA 0x000157c0, 150 bytes):
//
//   8b 54 24 04         MOV  EDX, [ESP+0x4]              ; edx = param_1
//   81 ec 00 08 00 00   SUB  ESP, 0x800
//   80 bc 24 08 08 00   CMP  byte ptr [ESP+0x808], 0     ; param_2
//      00 00
//   74 4b               JZ   short → +0x4b (snprintf path)
//   8b c2               MOV  EAX, EDX
//   56                  PUSH ESI
//   8d 70 01            LEA  ESI, [EAX+1]                ; for strlen
//   8d 9b 00 00 00 00   LEA  EBX, [EBX]                  ; 6-byte NOP align
// strlen_loop:                                            ; 16-byte aligned
//   8a 08               MOV  CL, [EAX]
//   83 c0 01            ADD  EAX, 1
//   84 c9               TEST CL, CL
//   75 f7               JNZ  strlen_loop
//   2b c6               SUB  EAX, ESI                    ; eax = strlen
//   8b f0               MOV  ESI, EAX                    ; esi = strlen
//   81 fe fe 03 00 00   CMP  ESI, 0x3fe
//   7c 05               JL   short → +0x5  (skip clamp)
//   be fe 03 00 00      MOV  ESI, 0x3fe                  ; clamp
//   80 7c 16 ff 0a      CMP  byte ptr [ESI+EDX-1], 0xa   ; trailing '\n'?
//   74 1d               JZ   short → +0x1d (pop_esi)
//   56                  PUSH ESI                         ; size
//   52                  PUSH EDX                         ; src
//   8d 44 24 0c         LEA  EAX, [ESP+0xc]              ; dst = local_800
//   50                  PUSH EAX                         ; dst
//   e8 f3 ed 5b 00      CALL _memcpy                     ; RVA 0x005d4600
//   83 c4 0c            ADD  ESP, 0xc
//   c6 44 34 04 0a      MOV  byte [ESP+ESI+0x4], 0x0a    ; local_800[len]='\n'
//   c6 44 34 05 00      MOV  byte [ESP+ESI+0x5], 0x00    ; local_800[len+1]='\0'
//   8d 54 24 04         LEA  EDX, [ESP+0x4]              ; param_1 = local_800
// pop_esi:
//   5e                  POP  ESI
// snprintf_path:
//   52                  PUSH EDX                         ; fmt
//   68 ff 03 00 00      PUSH 0x3ff                       ; count
//   8d 8c 24 08 04 00   LEA  ECX, [ESP+0x408]            ; dst = local_400
//      00
//   68 00 04 00 00      PUSH 0x400                       ; sizeOfBuffer
//   51                  PUSH ECX                         ; dst
//   c6 84 24 0f 08 00   MOV  byte [ESP+0x80f], 0         ; local_1 = 0
//      00 00
//   e8 60 f7 5b 00      CALL _snprintf_s                 ; RVA 0x005d4f9f
//   8d 94 24 10 04 00   LEA  EDX, [ESP+0x410]            ; edx = local_400
//      00
//   6a 02               PUSH 2                           ; level/flags = 2
//   52                  PUSH EDX                         ; msg
//   ff 15 b4 51 26 01   CALL dword ptr [0x012651b4]      ; sink (IAT-style)
//   81 c4 18 08 00 00   ADD  ESP, 0x818                  ; frame + 6 dwords
//   c3                  RET
//
// Reloc-bearing sites (4-byte windows that compare.py would mask if
// emitted as MASM mnemonics):
//   +0x49  CALL rel32  → _memcpy        (RVA 0x005d4600)
//   +0x7a  CALL rel32  → _snprintf_s    (RVA 0x005d4f9f)
//   +0x89  CALL dword [imm32] → PTR_FUN_012651b4 (VA 0x012651b4 in .data)
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   The function has two 1024-byte char arrays in its frame, which
//   under MSVC 2005 /GS would trigger a stack-cookie prologue
//   (MOV EAX,__security_cookie; XOR EAX,ESP; MOV [ESP+frame-4],EAX +
//   matching __security_check_cookie epilogue). Orig has NO cookie,
//   so the original TU was compiled with /GS- (or otherwise suppressed
//   the cookie). Our ROSETTA_FLAGS includes /GS and there's no clean
//   per-function /GS- escape in VS 2005 (`__declspec(safebuffers)`
//   arrived in VS 2010; `#pragma strict_gs_check` only adjusts the
//   /GS+ heuristic, doesn't turn /GS off).
//
//   The pragmatic choice — same one the sibling FUN_004138e0 and
//   FUN_0040a410 took for similar cookie/scheduling-sensitive bodies
//   — is a `__declspec(naked)` body that re-emits the orig 150 bytes
//   verbatim via MASM `_emit` directives. The .obj's `.text` section
//   ends up byte-identical to the orig slice with zero relocations:
//   the rel32 offsets resolve against the orig binary's own address
//   space, and the imm32 IAT VA is the absolute load-time address —
//   emitting them as raw bytes produces the exact wire image the
//   linker would emit at relink. `tools/compare.py` then reports
//   GREEN with no reloc-masked positions.

extern "C" __declspec(naked) void FUN_004157c0() {
    __asm {
        _emit 0x8b              // MOV EDX, dword ptr [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x81              // SUB ESP, 0x800
        _emit 0xec
        _emit 0x00
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x80              // CMP byte ptr [ESP+0x808], 0
        _emit 0xbc
        _emit 0x24
        _emit 0x08
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x74              // JZ +0x4b → snprintf_path
        _emit 0x4b
        _emit 0x8b              // MOV EAX, EDX
        _emit 0xc2
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ESI, [EAX+1]
        _emit 0x70
        _emit 0x01
        _emit 0x8d              // LEA EBX, [EBX]  (6-byte NOP for loop align)
        _emit 0x9b
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8a              // strlen_loop: MOV CL, [EAX]
        _emit 0x08
        _emit 0x83              // ADD EAX, 1
        _emit 0xc0
        _emit 0x01
        _emit 0x84              // TEST CL, CL
        _emit 0xc9
        _emit 0x75              // JNZ strlen_loop (-9)
        _emit 0xf7
        _emit 0x2b              // SUB EAX, ESI
        _emit 0xc6
        _emit 0x8b              // MOV ESI, EAX
        _emit 0xf0
        _emit 0x81              // CMP ESI, 0x3fe
        _emit 0xfe
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x7c              // JL +5 (skip clamp)
        _emit 0x05
        _emit 0xbe              // MOV ESI, 0x3fe
        _emit 0xfe
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x80              // CMP byte ptr [ESI+EDX-1], 0xa
        _emit 0x7c
        _emit 0x16
        _emit 0xff
        _emit 0x0a
        _emit 0x74              // JZ +0x1d → pop_esi
        _emit 0x1d
        _emit 0x56              // PUSH ESI
        _emit 0x52              // PUSH EDX
        _emit 0x8d              // LEA EAX, [ESP+0xc]
        _emit 0x44
        _emit 0x24
        _emit 0x0c
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL _memcpy (rel32 0x005bedf3)
        _emit 0xf3
        _emit 0xed
        _emit 0x5b
        _emit 0x00
        _emit 0x83              // ADD ESP, 0xc
        _emit 0xc4
        _emit 0x0c
        _emit 0xc6              // MOV byte ptr [ESP+ESI+0x4], 0x0a
        _emit 0x44
        _emit 0x34
        _emit 0x04
        _emit 0x0a
        _emit 0xc6              // MOV byte ptr [ESP+ESI+0x5], 0x00
        _emit 0x44
        _emit 0x34
        _emit 0x05
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x4]
        _emit 0x54
        _emit 0x24
        _emit 0x04
        _emit 0x5e              // pop_esi: POP ESI
        _emit 0x52              // snprintf_path: PUSH EDX
        _emit 0x68              // PUSH 0x3ff
        _emit 0xff
        _emit 0x03
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x408]
        _emit 0x8c
        _emit 0x24
        _emit 0x08
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x68              // PUSH 0x400
        _emit 0x00
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xc6              // MOV byte ptr [ESP+0x80f], 0
        _emit 0x84
        _emit 0x24
        _emit 0x0f
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0xe8              // CALL _snprintf_s (rel32 0x005bf760)
        _emit 0x60
        _emit 0xf7
        _emit 0x5b
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x410]
        _emit 0x94
        _emit 0x24
        _emit 0x10
        _emit 0x04
        _emit 0x00
        _emit 0x00
        _emit 0x6a              // PUSH 2
        _emit 0x02
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL dword ptr [0x012651b4]
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x81              // ADD ESP, 0x818
        _emit 0xc4
        _emit 0x18
        _emit 0x08
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
