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
// FUNCTION: ffxivgame 0x000123b0 — __thiscall, 3 stack args, 117 bytes
//           Lock → factory-create → node-insert → associate → Unlock.
//
// ESI = this (cached throughout)
// EBX = result of FUN_00412360 (node ptr, or 0 if factory returned null)
// EBP = param_3 (cached across two call sites)
// EDI = piVar1 (return value of field_4->vtable[3])
//
// Returns: result ? result + 4 : 0
//
// Asm (117 bytes @ orig RVA 0x000123b0):
//   53                   PUSH EBX
//   55                   PUSH EBP
//   56                   PUSH ESI
//   8b f1                MOV ESI, ECX
//   8b 06                MOV EAX, [ESI]
//   8b 50 2c             MOV EDX, [EAX+0x2c]
//   57                   PUSH EDI
//   ff d2                CALL EDX              ; this->vtable[11]() — lock
//   8b 6c 24 1c          MOV EBP, [ESP+0x1c]  ; EBP = param_3
//   8b 54 24 18          MOV EDX, [ESP+0x18]  ; EDX = param_2
//   8b 4e 04             MOV ECX, [ESI+4]     ; ECX = this->field_4
//   8b 01                MOV EAX, [ECX]       ; EAX = field_4->vtable
//   8b 40 0c             MOV EAX, [EAX+0xc]   ; EAX = vtable[3]
//   55                   PUSH EBP             ; arg3 = param_3
//   52                   PUSH EDX             ; arg2 = param_2
//   8b 54 24 1c          MOV EDX, [ESP+0x1c]  ; EDX = param_1
//   52                   PUSH EDX             ; arg1 = param_1
//   33 db                XOR EBX, EBX         ; result = 0
//   ff d0                CALL EAX             ; field_4->vtable[3](p1,p2,p3)
//   8b f8                MOV EDI, EAX         ; piVar1 = result
//   85 ff                TEST EDI, EDI
//   74 27                JE  +0x27            ; if piVar1==null → skip block
//   8b 4c 24 18          MOV ECX, [ESP+0x18]  ; reload param_2
//   8b 54 24 14          MOV EDX, [ESP+0x14]  ; reload param_1
//   55                   PUSH EBP             ; arg3 = param_3
//   51                   PUSH ECX             ; arg2 = param_2
//   52                   PUSH EDX             ; arg1 = param_1
//   8b ce                MOV ECX, ESI         ; ECX = this
//   e8 70 ff ff ff       CALL FUN_00412360    ; this->FUN_00412360(p1,p2,p3)
//   8b d8                MOV EBX, EAX         ; result = EAX (node ptr)
//   6a 00                PUSH 0               ; arg3 = 0
//   89 7b 28             MOV [EBX+0x28], EDI  ; [result+0x28] = piVar1
//   8b 07                MOV EAX, [EDI]       ; piVar1->vtable
//   8b 50 2c             MOV EDX, [EAX+0x2c]  ; vtable[11]
//   53                   PUSH EBX             ; arg2 = result
//   8d 4e 58             LEA ECX, [ESI+0x58]  ; arg1 = &this->field_0x58
//   51                   PUSH ECX
//   8b cf                MOV ECX, EDI         ; ECX = piVar1
//   ff d2                CALL EDX             ; piVar1->vtable[11](&field_0x58,result,0)
//   8b 06                MOV EAX, [ESI]
//   8b 50 30             MOV EDX, [EAX+0x30]  ; vtable[12]
//   8b ce                MOV ECX, ESI
//   ff d2                CALL EDX             ; this->vtable[12]() — unlock
//   85 db                TEST EBX, EBX
//   74 0a                JE  +0x0a            ; if result==0 → return 0
//   5f                   POP EDI
//   5e                   POP ESI
//   5d                   POP EBP
//   8d 43 04             LEA EAX, [EBX+4]     ; return result + 4
//   5b                   POP EBX
//   c2 0c 00             RET 0xc
//   5f                   POP EDI
//   5e                   POP ESI
//   5d                   POP EBP
//   33 c0                XOR EAX, EAX         ; return 0
//   5b                   POP EBX
//   c2 0c 00             RET 0xc

// IResult_123b0: object returned by factory; has vtable[11] at +0x2c
struct IResult_123b0 {
    virtual void vfunc_00() = 0;
    virtual void vfunc_01() = 0;
    virtual void vfunc_02() = 0;
    virtual void vfunc_03() = 0;
    virtual void vfunc_04() = 0;
    virtual void vfunc_05() = 0;
    virtual void vfunc_06() = 0;
    virtual void vfunc_07() = 0;
    virtual void vfunc_08() = 0;
    virtual void vfunc_09() = 0;
    virtual void vfunc_0a() = 0;
    virtual void vfunc_0b(void *a, int b, int c) = 0;  // vtable +0x2c
};

// IFactory_123b0: factory object; vtable[3] at +0x0c returns IResult_123b0*
struct IFactory_123b0 {
    virtual void vfunc_00() = 0;
    virtual void vfunc_01() = 0;
    virtual void vfunc_02() = 0;
    virtual IResult_123b0 *vfunc_03(int a, int b, int c) = 0;  // vtable +0x0c
};

// IBase_123b0: base vtable for `this`; lock at +0x2c, unlock at +0x30
struct IBase_123b0 {
    virtual void vfunc_00() = 0;
    virtual void vfunc_01() = 0;
    virtual void vfunc_02() = 0;
    virtual void vfunc_03() = 0;
    virtual void vfunc_04() = 0;
    virtual void vfunc_05() = 0;
    virtual void vfunc_06() = 0;
    virtual void vfunc_07() = 0;
    virtual void vfunc_08() = 0;
    virtual void vfunc_09() = 0;
    virtual void vfunc_0a() = 0;
    virtual void vfunc_0b() = 0;   // vtable +0x2c — lock
    virtual void vfunc_0c() = 0;   // vtable +0x30 — unlock
};

// The outer class: vtable at +0, field_4 at +4, field_0x58 at +0x58
struct FUN_004123b0_C : public IBase_123b0 {
    IFactory_123b0 *field_4;   // offset 4 (after vtable ptr)
    char _pad[0x50];           // padding bytes to reach offset 0x58
    int field_0x58;            // embedded field at offset 0x58

    int FUN_004123b0(int p1, int p2, int p3);
    int FUN_00412360(int p1, int p2, int p3);
};

int FUN_004123b0_C::FUN_004123b0(int p1, int p2, int p3)
{
    vfunc_0b();  // this->vtable[11]() — lock
    int result = 0;
    IResult_123b0 *piVar1 = field_4->vfunc_03(p1, p2, p3);
    if (piVar1 != 0) {
        result = FUN_00412360(p1, p2, p3);
        *(IResult_123b0 **)(result + 0x28) = piVar1;
        piVar1->vfunc_0b(&field_0x58, result, 0);
    }
    vfunc_0c();  // this->vtable[12]() — unlock
    if (result != 0) {
        return result + 4;
    }
    return 0;
}
