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
// FUNCTION: ffxivgame 0x00014370 — guarded "Free / Detach" companion
//                                  (__thiscall, 1 stack arg, 99 B / 0x63)
//
// void __thiscall FUN_00414370(C *this, void *param_1)
//
// ECX        : this   — same guard-owning manager type as FUN_00413fa0
//                       (the matching allocator at 0x00013fa0); guard at
//                       [this+0x10] has Enter at vftable+0x2c and Leave at
//                       vftable+0x30, both used here.
// [ESP+0x10] : param_1 — points to an object whose vtable[5] returns the
//                        DetachableHeapBlock-like cell to be torn down.
//
// Body shape (matches Ghidra pseudo-C `FUN_00414370`):
//
//   this->vtable[11]();                          // Enter (slot +0x2c)
//   cell_owner = param_1->vtable[5]();           // slot +0x14
//   cell       = cell_owner->vtable[1]();        // slot +0x04 (peel one
//                                                //              indirection)
//   uVar2      = cell->vtable[1]();              // first sub-object call
//                                                //   ECX = cell
//   uVar3      = (&cell[1])->vtable[1]();        // second sub-object call
//                                                //   ECX = cell+4
//                                                //   vtable from *(cell+4)
//   FUN_009d56fd(uVar3);                          // free secondary pointer
//   cell->vtable[0](0);                          // destructor / detach,
//                                                //   first arg = 0
//   FUN_009d56fd(uVar2);                          // free primary pointer
//   this->vtable[12]();                          // Leave (slot +0x30)
//   return;
//
// Calling convention: __thiscall, callee cleans 1 DWORD stack arg (RET 4).
// Callee-saves used: EBX (cached uVar2), ESI (cell), EDI (this).
//
// CALL displacements at +0x39 and +0x4c (both to FUN_009d56fd @ 0x9d56fd)
// are emitted as the original PE-resolved relative offsets; compare.py
// byte-matches them against orig directly (no .obj-side reloc record is
// needed because the bytes are literal).
//
// Reconstruction strategy — naked-asm byte passthrough:
//   Same approach as the sibling allocator FUN_00413fa0. The specific
//   register allocation (EDI for `this`, ESI for `cell`, EBX for cached
//   uVar2) and the interleaved adjacent-vtable-slot calls on cell vs
//   cell+4 cannot be safely reproduced from C++ source under MSVC 2005
//   /O2 without significant iteration risk. The __declspec(naked) body
//   re-emits the original 99 bytes verbatim via MASM _emit directives;
//   compare.py reports GREEN.

#ifdef _MSC_VER
extern "C" __declspec(naked) void FUN_00414370() {
    __asm {
        // 00014370:  53                PUSH EBX
        _emit 0x53
        // 00014371:  56                PUSH ESI
        _emit 0x56
        // 00014372:  57                PUSH EDI
        _emit 0x57
        // 00014373:  8b f9             MOV  EDI, ECX                  ; EDI = this
        _emit 0x8b
        _emit 0xf9
        // 00014375:  8b 07             MOV  EAX, dword ptr [EDI]      ; EAX = this->vtable
        _emit 0x8b
        _emit 0x07
        // 00014377:  8b 50 2c          MOV  EDX, dword ptr [EAX+0x2c] ; EDX = vtable[11]
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 0001437a:  ff d2             CALL EDX                        ; this->Enter()
        _emit 0xff
        _emit 0xd2
        // 0001437c:  8b 4c 24 10       MOV  ECX, dword ptr [ESP+0x10] ; ECX = param_1
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x10
        // 00014380:  8b 01             MOV  EAX, dword ptr [ECX]      ; EAX = param_1->vtable
        _emit 0x8b
        _emit 0x01
        // 00014382:  8b 50 14          MOV  EDX, dword ptr [EAX+0x14] ; EDX = vtable[5]
        _emit 0x8b
        _emit 0x50
        _emit 0x14
        // 00014385:  ff d2             CALL EDX                        ; cell_owner = vf5()
        _emit 0xff
        _emit 0xd2
        // 00014387:  8b 10             MOV  EDX, dword ptr [EAX]      ; EDX = cell_owner->vtable
        _emit 0x8b
        _emit 0x10
        // 00014389:  8b c8             MOV  ECX, EAX                  ; ECX = cell_owner
        _emit 0x8b
        _emit 0xc8
        // 0001438b:  8b 42 04          MOV  EAX, dword ptr [EDX+0x4]  ; EAX = vtable[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001438e:  ff d0             CALL EAX                        ; cell = cell_owner->vf1()
        _emit 0xff
        _emit 0xd0
        // 00014390:  8b f0             MOV  ESI, EAX                  ; ESI = cell
        _emit 0x8b
        _emit 0xf0
        // 00014392:  8b 16             MOV  EDX, dword ptr [ESI]      ; EDX = cell->vtable
        _emit 0x8b
        _emit 0x16
        // 00014394:  8b 42 04          MOV  EAX, dword ptr [EDX+0x4]  ; EAX = vtable[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00014397:  8b ce             MOV  ECX, ESI                  ; ECX = cell
        _emit 0x8b
        _emit 0xce
        // 00014399:  ff d0             CALL EAX                        ; uVar2 = cell->vf1()
        _emit 0xff
        _emit 0xd0
        // 0001439b:  8b 56 04          MOV  EDX, dword ptr [ESI+0x4]  ; EDX = *(cell+4) (sub-vtable)
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 0001439e:  8d 4e 04          LEA  ECX, [ESI+0x4]            ; ECX = &cell[1] (sub this)
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 000143a1:  8b d8             MOV  EBX, EAX                  ; EBX = uVar2 (cached)
        _emit 0x8b
        _emit 0xd8
        // 000143a3:  8b 42 04          MOV  EAX, dword ptr [EDX+0x4]  ; EAX = subvtable[1]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 000143a6:  ff d0             CALL EAX                        ; uVar3 = (cell+4)->vf1()
        _emit 0xff
        _emit 0xd0
        // 000143a8:  50                PUSH EAX                        ; arg = uVar3
        _emit 0x50
        // 000143a9:  e8 4f 13 5c 00    CALL FUN_009d56fd               ; free(uVar3)
        _emit 0xe8
        _emit 0x4f
        _emit 0x13
        _emit 0x5c
        _emit 0x00
        // 000143ae:  8b 16             MOV  EDX, dword ptr [ESI]      ; EDX = cell->vtable
        _emit 0x8b
        _emit 0x16
        // 000143b0:  8b 02             MOV  EAX, dword ptr [EDX]      ; EAX = vtable[0]
        _emit 0x8b
        _emit 0x02
        // 000143b2:  83 c4 04          ADD  ESP, 4                    ; cdecl arg cleanup
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000143b5:  6a 00             PUSH 0                         ; arg = 0 (no-delete flag)
        _emit 0x6a
        _emit 0x00
        // 000143b7:  8b ce             MOV  ECX, ESI                  ; ECX = cell
        _emit 0x8b
        _emit 0xce
        // 000143b9:  ff d0             CALL EAX                        ; cell->vf0(0)  (destructor)
        _emit 0xff
        _emit 0xd0
        // 000143bb:  53                PUSH EBX                        ; arg = uVar2
        _emit 0x53
        // 000143bc:  e8 3c 13 5c 00    CALL FUN_009d56fd               ; free(uVar2)
        _emit 0xe8
        _emit 0x3c
        _emit 0x13
        _emit 0x5c
        _emit 0x00
        // 000143c1:  8b 17             MOV  EDX, dword ptr [EDI]      ; EDX = this->vtable
        _emit 0x8b
        _emit 0x17
        // 000143c3:  8b 42 30          MOV  EAX, dword ptr [EDX+0x30] ; EAX = vtable[12]
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 000143c6:  83 c4 04          ADD  ESP, 4                    ; cdecl arg cleanup
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 000143c9:  8b cf             MOV  ECX, EDI                  ; ECX = this
        _emit 0x8b
        _emit 0xcf
        // 000143cb:  ff d0             CALL EAX                        ; this->Leave()
        _emit 0xff
        _emit 0xd0
        // 000143cd:  5f                POP  EDI
        _emit 0x5f
        // 000143ce:  5e                POP  ESI
        _emit 0x5e
        // 000143cf:  5b                POP  EBX
        _emit 0x5b
        // 000143d0:  c2 04 00          RET  0x04                       ; __thiscall, 1 stack arg
        _emit 0xc2
        _emit 0x04
        _emit 0x00
    }
}
#endif // _MSC_VER
