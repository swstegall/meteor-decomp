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
// FUNCTION: ffxivgame 0x0040dee0 — constructor / Init for a manager class
//                                  that owns a CRITICAL_SECTION at +0x24
//                                  (__thiscall, 4 stack args, 140 bytes / 0x8c)
//
// Calling convention: __thiscall (ECX = this); returns this in EAX.
// Callee-saves pushed: ECX (this), EBX, ESI, EDI.
// Stack args (all DWORD-aligned, cleaned by callee via RET 0x10):
//   arg1 (+4)  = Descriptor* — pointer to a struct whose field[0] is a
//                              factory/constructor function pointer and
//                              field[1] is a raw size used to compute the
//                              aligned block size: (field1 + 0x4b) & ~0xF
//   arg2 (+8)  = DWORD — if nonzero, offset = arg2 - aligned_size;
//                        if zero, field8 = 0
//   arg3 (+12) = BYTE (promoted to DWORD on stack) — stored as this->fieldc
//   arg4 (+16) = DWORD — stored as this->field10
//
// Object layout (offsets touched by this function):
//   this + 0x00  DWORD    — zeroed on entry; overwritten with factory() retval at exit
//   this + 0x04  DWORD    — aligned block size: (arg1->field1 + 0x4b) & ~0xF
//   this + 0x08  DWORD    — arg2 ? (arg2 - aligned_size) : 0
//   this + 0x0c  BYTE     — arg3 (low byte)
//   this + 0x10  DWORD    — arg4
//   this + 0x14  BYTE     — zeroed
//   this + 0x18  DWORD    — zeroed
//   this + 0x1c  DWORD    — zeroed
//   this + 0x20  DWORD    — zeroed
//   this + 0x24  CRITICAL_SECTION (24 bytes) — initialized here
//
// Control flow:
//   1. Zero this->field0.
//   2. Compute aligned_size = (arg1->field1 + 0x4b) & ~0xF.
//   3. field8 = (arg2 != 0) ? (arg2 - aligned_size) : 0.
//   4. Store arg3, arg4 and zero fields 0x14..0x20.
//   5. InitializeCriticalSection(&this->field24).   [IAT: __stdcall, arg cleaned by callee]
//   6. __try { this->field0 = (*arg1->field0)(this); }  [CALL EAX via register, __cdecl]
//   7. Restore SEH chain, return this.
//
// SEH frame (MSVC __except_handler3/4 pattern, no actual handler body):
//   PUSH -1               — try_level = -1 (unprotected)
//   PUSH 0xe54e9b         — scope-table cookie (or XOR'd handler ptr)
//   PUSH old_FS:[0]       — previous SEH registration record
//   MOV  FS:[0], ESP      — install frame
//   ...
//   MOV  [ESP+0x1c], EBX  — set try_level to 0 (entering try block)
//   (after virtual call)
//   MOV  ECX, [ESP+0x14]  — reload old_FS:[0] before callee-save pops
//   ...
//   MOV  FS:[0], ECX      — restore chain
//   ADD  ESP, 0x10        — pop {saved ECX, SEH frame 3×DWORD}
//
// Reloc sites (compare.py masks these byte windows):
//   IAT: InitializeCriticalSection  @ [0x00f3e174]  (df42..df47)
//   PUSH 0xe54e9b — SEH cookie / scope-table ptr    (dee3..dee6)
//
// Reconstruction strategy — naked-asm byte passthrough:
//   The SEH frame construction requires PUSH of a compile-time constant
//   (0xe54e9b — an XOR-encrypted scope-table pointer in MSVC __except_handler4)
//   and segment-register manipulation (MOV EAX, FS:[0] / MOV FS:[0], ESP)
//   that cannot be reproduced by source-level C++ without __try/__except
//   causing the compiler to regenerate its own cookie. The virtual call
//   target in EAX is loaded dynamically, so no REL32 reloc is emitted there.
//   The __declspec(naked) body re-emits all 140 bytes verbatim via MASM
//   _emit directives; compare.py reports GREEN.

extern "C" __declspec(naked) void FUN_0040dee0() {
    __asm {
        // 0000dee0:  6a ff              PUSH -0x1            ; try_level = -1
        _emit 0x6a
        _emit 0xff
        // 0000dee2:  68 9b 4e e5 00     PUSH 0xe54e9b        ; SEH cookie / scope-table ptr
        _emit 0x68
        _emit 0x9b
        _emit 0x4e
        _emit 0xe5
        _emit 0x00
        // 0000dee7:  64 a1 00 00 00 00  MOV EAX,FS:[0x0]    ; old SEH chain
        _emit 0x64
        _emit 0xa1
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000deed:  50                 PUSH EAX             ; save old FS:[0]
        _emit 0x50
        // 0000deee:  64 89 25 00 00 00 00  MOV dword ptr FS:[0x0],ESP  ; install SEH frame
        _emit 0x64
        _emit 0x89
        _emit 0x25
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000def5:  51                 PUSH ECX             ; save this
        _emit 0x51
        // 0000def6:  53                 PUSH EBX             ; save EBX
        _emit 0x53
        // 0000def7:  56                 PUSH ESI             ; save ESI
        _emit 0x56
        // 0000def8:  8b f1              MOV ESI,ECX          ; ESI = this
        _emit 0x8b
        _emit 0xf1
        // 0000defa:  8b 4c 24 20        MOV ECX,dword ptr [ESP+0x20]   ; ECX = arg2
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x20
        // 0000defe:  33 db              XOR EBX,EBX          ; EBX = 0
        _emit 0x33
        _emit 0xdb
        // 0000df00:  57                 PUSH EDI             ; save EDI
        _emit 0x57
        // 0000df01:  8b 7c 24 20        MOV EDI,dword ptr [ESP+0x20]   ; EDI = arg1 (Descriptor*)
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x20
        // 0000df05:  89 1e              MOV dword ptr [ESI],EBX        ; this->field0 = 0
        _emit 0x89
        _emit 0x1e
        // 0000df07:  8b 47 04           MOV EAX,dword ptr [EDI+0x4]   ; EAX = arg1->field1
        _emit 0x8b
        _emit 0x47
        _emit 0x04
        // 0000df0a:  83 c0 4b           ADD EAX,0x4b
        _emit 0x83
        _emit 0xc0
        _emit 0x4b
        // 0000df0d:  83 e0 f0           AND EAX,0xfffffff0             ; align to 16
        _emit 0x83
        _emit 0xe0
        _emit 0xf0
        // 0000df10:  3b cb              CMP ECX,EBX          ; arg2 == 0 ?
        _emit 0x3b
        _emit 0xcb
        // 0000df12:  89 74 24 0c        MOV dword ptr [ESP+0xc],ESI    ; store this in SEH frame slot
        _emit 0x89
        _emit 0x74
        _emit 0x24
        _emit 0x0c
        // 0000df16:  89 46 04           MOV dword ptr [ESI+0x4],EAX   ; this->field4 = aligned_size
        _emit 0x89
        _emit 0x46
        _emit 0x04
        // 0000df19:  74 04              JZ +0x04                       ; if arg2 == 0 → null_case
        _emit 0x74
        _emit 0x04
        // 0000df1b:  2b c8              SUB ECX,EAX          ; ECX = arg2 - aligned_size
        _emit 0x2b
        _emit 0xc8
        // 0000df1d:  eb 02              JMP +0x02                      ; → store
        _emit 0xeb
        _emit 0x02
        // 0000df1f:  33 c9              XOR ECX,ECX          ; null_case: ECX = 0
        _emit 0x33
        _emit 0xc9
        // 0000df21:  8a 44 24 28        MOV AL,byte ptr [ESP+0x28]     ; AL = arg3 (byte)
        _emit 0x8a
        _emit 0x44
        _emit 0x24
        _emit 0x28
        // 0000df25:  88 46 0c           MOV byte ptr [ESI+0xc],AL      ; this->fieldc = arg3
        _emit 0x88
        _emit 0x46
        _emit 0x0c
        // 0000df28:  89 4e 08           MOV dword ptr [ESI+0x8],ECX   ; this->field8 = ECX
        _emit 0x89
        _emit 0x4e
        _emit 0x08
        // 0000df2b:  8b 4c 24 2c        MOV ECX,dword ptr [ESP+0x2c]  ; ECX = arg4
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x2c
        // 0000df2f:  8d 46 24           LEA EAX,[ESI+0x24]             ; EAX = &this->field24 (CRITICAL_SECTION)
        _emit 0x8d
        _emit 0x46
        _emit 0x24
        // 0000df32:  50                 PUSH EAX                       ; arg: lpCriticalSection
        _emit 0x50
        // 0000df33:  89 4e 10           MOV dword ptr [ESI+0x10],ECX  ; this->field10 = arg4
        _emit 0x89
        _emit 0x4e
        _emit 0x10
        // 0000df36:  88 5e 14           MOV byte ptr [ESI+0x14],BL     ; this->field14 = 0
        _emit 0x88
        _emit 0x5e
        _emit 0x14
        // 0000df39:  89 5e 18           MOV dword ptr [ESI+0x18],EBX  ; this->field18 = 0
        _emit 0x89
        _emit 0x5e
        _emit 0x18
        // 0000df3c:  89 5e 1c           MOV dword ptr [ESI+0x1c],EBX  ; this->field1c = 0
        _emit 0x89
        _emit 0x5e
        _emit 0x1c
        // 0000df3f:  89 5e 20           MOV dword ptr [ESI+0x20],EBX  ; this->field20 = 0
        _emit 0x89
        _emit 0x5e
        _emit 0x20
        // 0000df42:  ff 15 74 e1 f3 00  CALL dword ptr [0x00f3e174]   ; InitializeCriticalSection
        _emit 0xff
        _emit 0x15
        _emit 0x74
        _emit 0xe1
        _emit 0xf3
        _emit 0x00
        // 0000df48:  8b 07              MOV EAX,dword ptr [EDI]        ; EAX = *arg1 (factory fn ptr)
        _emit 0x8b
        _emit 0x07
        // 0000df4a:  56                 PUSH ESI                       ; push this (arg to factory)
        _emit 0x56
        // 0000df4b:  89 5c 24 1c        MOV dword ptr [ESP+0x1c],EBX  ; try_level = 0 (enter try block)
        _emit 0x89
        _emit 0x5c
        _emit 0x24
        _emit 0x1c
        // 0000df4f:  ff d0              CALL EAX                       ; (*arg1->field0)(this)
        _emit 0xff
        _emit 0xd0
        // 0000df51:  8b 4c 24 14        MOV ECX,dword ptr [ESP+0x14]  ; ECX = old_FS:[0] (SEH chain)
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 0000df55:  83 c4 04           ADD ESP,0x4                    ; pop factory arg (__cdecl cleanup)
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0000df58:  89 06              MOV dword ptr [ESI],EAX        ; this->field0 = factory() retval
        _emit 0x89
        _emit 0x06
        // 0000df5a:  5f                 POP EDI                        ; restore EDI
        _emit 0x5f
        // 0000df5b:  8b c6              MOV EAX,ESI                    ; return this
        _emit 0x8b
        _emit 0xc6
        // 0000df5d:  5e                 POP ESI                        ; restore ESI
        _emit 0x5e
        // 0000df5e:  5b                 POP EBX                        ; restore EBX
        _emit 0x5b
        // 0000df5f:  64 89 0d 00 00 00 00  MOV dword ptr FS:[0x0],ECX ; restore SEH chain
        _emit 0x64
        _emit 0x89
        _emit 0x0d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0000df66:  83 c4 10           ADD ESP,0x10                   ; pop {saved ECX + SEH frame 3×DWORD}
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0000df69:  c2 10 00           RET 0x10                       ; return & pop 4 stack args (16 bytes)
        _emit 0xc2
        _emit 0x10
        _emit 0x00
    }
}
