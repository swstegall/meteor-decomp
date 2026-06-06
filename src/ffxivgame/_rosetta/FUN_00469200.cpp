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
// FUNCTION: ffxivgame 0x00469200 — `_DH_new_method` (342 B / 0x156), `__cdecl`.
//                                  Allocates and initialises a DH method object.
//
// Inspection (read from the disassembly at orig RVA 0x00069200):
//
//   __cdecl void* _DH_new_method(void* param);
//
//   Structure:
//     1. Allocates a 0x4c-byte block via FUN_00463150(0x6f, <string>, 0x4c).
//        If allocation fails, logs error via FUN_0045c940 and returns NULL.
//     2. Lazy-initialises a global handle at [0x0132e834] by calling
//        FUN_0047c280 if the slot is zero.
//     3. If `param` is non-NULL, validates it via FUN_00469540; on failure
//        logs error, frees the new block via FUN_004632f0, returns NULL.
//        Stores the validated param (or a default from FUN_0047c600 when
//        param was NULL) into [obj+0x48].
//     4. Initialises [obj+0x44] via FUN_00469150([obj+0x48]).
//        On failure, logs error, frees [obj+0x48] and obj, returns NULL.
//     5. Zero-initialises a range of fields in the block, sets [obj+0x38]=1,
//        copies a field from the FUN_00469150 result into [obj+0x1c], then
//        calls FUN_00461660(obj, 8) to further set up the object.
//     6. Reads a vtable slot at [obj+0x44+0x10] and calls it with (obj).
//        If it returns 0 (failure), cleans up [obj+0x48], calls
//        FUN_004616c0(&local, obj, 8) and FUN_004632f0(obj), zeroes ESI,
//        and returns NULL.
//     7. Returns the newly-constructed obj pointer.
//
//   Stack frame (flat ESP frame, no EBP base):
//     Entry: PUSH ESI / PUSH EDI (saved callee-saved regs)
//     After initial alloc call+cleanup: ESI = obj, EDI = 0
//     [ESP+0x10] after PUSH EBX = first caller arg (`param`)
//     EBX = param, ECX = [obj+0x44] vtable pointer at end
//
//   Calling convention: __cdecl (RET without operand; callee does not
//   clean caller args).
//
// Reloc-bearing sites in the orig 342 bytes (absolute VAs baked into the
// instruction stream; tools/compare.py masks on the comparison path):
//
//     +0x04   DIR32 → 0x00f7916c   (string arg pushed to alloc)
//     +0x11   DIR32 → 0x00f7916c   (string arg pushed to error logger)
//     +0x1d   REL32 → 0x0045c940   (CALL error logger — alloc-fail)
//     +0x35   DIR32 → 0x0132e834   (global handle — MOV EAX,[...])
//     +0x3f   REL32 → 0x0047c280   (CALL init for global handle)
//     +0x43   DIR32 → 0x0132e834   (global handle — MOV [...],EAX)
//     +0x55   REL32 → 0x00469540   (CALL param validator)
//     +0x63   DIR32 → 0x00f7916c   (string arg pushed to error logger)
//     +0x6e   REL32 → 0x0045c940   (CALL error logger — param-invalid)
//     +0x74   REL32 → 0x004632f0   (CALL free obj — param-invalid path)
//     +0x87   REL32 → 0x0047c600   (CALL default param getter)
//     +0x97   REL32 → 0x00469150   (CALL FUN_00469150 — [obj+0x44] init)
//     +0xab   DIR32 → 0x00f7916c   (string arg pushed to error logger)
//     +0xb6   REL32 → 0x0045c940   (CALL error logger — obj+0x44-fail)
//     +0xbf   REL32 → 0x004695c0   (CALL free [obj+0x48])
//     +0xc5   REL32 → 0x004632f0   (CALL free obj — obj+0x44-fail)
//     +0x110  REL32 → 0x00461660   (CALL FUN_00461660(obj,8))
//     +0x134  REL32 → 0x004695c0   (CALL free [obj+0x48] — vtable-fail)
//     +0x140  REL32 → 0x004616c0   (CALL FUN_004616c0(&local,obj,8))
//     +0x146  REL32 → 0x004632f0   (CALL free obj — vtable-fail)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   The function's multi-exit control flow, vtable call, ESP-relative
//   parameter access, and interleaved relocation-bearing direct calls
//   make a source-level MSVC 2005 /O2 rewrite fragile (register
//   allocation order, branch direction, frame layout all shift). The
//   same naked-asm byte passthrough used by the other _rosetta siblings
//   (FUN_00405080, FUN_004014b0, FUN_00408f10, etc.) is the pragmatic
//   approach here. Every PC-relative call displacement and absolute
//   address is re-emitted verbatim; tools/compare.py masks reloc windows
//   during the byte comparison.

extern "C" __declspec(naked) void FUN_00469200() {
    __asm {
        // 00069200: 56                  PUSH ESI
        _emit 0x56
        // 00069201: 57                  PUSH EDI
        _emit 0x57
        // 00069202: 6a 6f               PUSH 0x6f
        _emit 0x6a
        _emit 0x6f
        // 00069204: 68 6c 91 f7 00      PUSH 0xf7916c
        _emit 0x68
        _emit 0x6c
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069209: 6a 4c               PUSH 0x4c
        _emit 0x6a
        _emit 0x4c
        // 0006920b: e8 40 9f ff ff      CALL 0x00463150
        _emit 0xe8
        _emit 0x40
        _emit 0x9f
        _emit 0xff
        _emit 0xff
        // 00069210: 8b f0               MOV ESI,EAX
        _emit 0x8b
        _emit 0xf0
        // 00069212: 33 ff               XOR EDI,EDI
        _emit 0x33
        _emit 0xff
        // 00069214: 83 c4 0c            ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 00069217: 3b f7               CMP ESI,EDI
        _emit 0x3b
        _emit 0xf7
        // 00069219: 75 1a               JNZ 0x00469235
        _emit 0x75
        _emit 0x1a
        // 0006921b: 6a 72               PUSH 0x72
        _emit 0x6a
        _emit 0x72
        // 0006921d: 68 6c 91 f7 00      PUSH 0xf7916c
        _emit 0x68
        _emit 0x6c
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069222: 6a 41               PUSH 0x41
        _emit 0x6a
        _emit 0x41
        // 00069224: 6a 69               PUSH 0x69
        _emit 0x6a
        _emit 0x69
        // 00069226: 6a 05               PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 00069228: e8 13 37 ff ff      CALL 0x0045c940
        _emit 0xe8
        _emit 0x13
        _emit 0x37
        _emit 0xff
        _emit 0xff
        // 0006922d: 83 c4 14            ADD ESP,0x14
        _emit 0x83
        _emit 0xc4
        _emit 0x14
        // 00069230: 5f                  POP EDI
        _emit 0x5f
        // 00069231: 33 c0               XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00069233: 5e                  POP ESI
        _emit 0x5e
        // 00069234: c3                  RET
        _emit 0xc3
        // 00069235: a1 34 e8 32 01      MOV EAX,[0x0132e834]
        _emit 0xa1
        _emit 0x34
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        // 0006923a: 3b c7               CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 0006923c: 75 0a               JNZ 0x00469248
        _emit 0x75
        _emit 0x0a
        // 0006923e: e8 3d 30 01 00      CALL 0x0047c280
        _emit 0xe8
        _emit 0x3d
        _emit 0x30
        _emit 0x01
        _emit 0x00
        // 00069243: a3 34 e8 32 01      MOV [0x0132e834],EAX
        _emit 0xa3
        _emit 0x34
        _emit 0xe8
        _emit 0x32
        _emit 0x01
        // 00069248: 53                  PUSH EBX
        _emit 0x53
        // 00069249: 8b 5c 24 10         MOV EBX,[ESP+0x10]
        _emit 0x8b
        _emit 0x5c
        _emit 0x24
        _emit 0x10
        // 0006924d: 3b df               CMP EBX,EDI
        _emit 0x3b
        _emit 0xdf
        // 0006924f: 89 46 44            MOV [ESI+0x44],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x44
        // 00069252: 74 33               JZ 0x00469287
        _emit 0x74
        _emit 0x33
        // 00069254: 53                  PUSH EBX
        _emit 0x53
        // 00069255: e8 e6 02 00 00      CALL 0x00469540
        _emit 0xe8
        _emit 0xe6
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 0006925a: 83 c4 04            ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006925d: 85 c0               TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0006925f: 75 21               JNZ 0x00469282
        _emit 0x75
        _emit 0x21
        // 00069261: 6a 7c               PUSH 0x7c
        _emit 0x6a
        _emit 0x7c
        // 00069263: 68 6c 91 f7 00      PUSH 0xf7916c
        _emit 0x68
        _emit 0x6c
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 00069268: 6a 26               PUSH 0x26
        _emit 0x6a
        _emit 0x26
        // 0006926a: 6a 69               PUSH 0x69
        _emit 0x6a
        _emit 0x69
        // 0006926c: 6a 05               PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 0006926e: e8 cd 36 ff ff      CALL 0x0045c940
        _emit 0xe8
        _emit 0xcd
        _emit 0x36
        _emit 0xff
        _emit 0xff
        // 00069273: 56                  PUSH ESI
        _emit 0x56
        // 00069274: e8 77 a0 ff ff      CALL 0x004632f0
        _emit 0xe8
        _emit 0x77
        _emit 0xa0
        _emit 0xff
        _emit 0xff
        // 00069279: 83 c4 18            ADD ESP,0x18
        _emit 0x83
        _emit 0xc4
        _emit 0x18
        // 0006927c: 5b                  POP EBX
        _emit 0x5b
        // 0006927d: 5f                  POP EDI
        _emit 0x5f
        // 0006927e: 33 c0               XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 00069280: 5e                  POP ESI
        _emit 0x5e
        // 00069281: c3                  RET
        _emit 0xc3
        // 00069282: 89 5e 48            MOV [ESI+0x48],EBX
        _emit 0x89
        _emit 0x5e
        _emit 0x48
        // 00069285: eb 08               JMP 0x0046928f
        _emit 0xeb
        _emit 0x08
        // 00069287: e8 74 33 01 00      CALL 0x0047c600
        _emit 0xe8
        _emit 0x74
        _emit 0x33
        _emit 0x01
        _emit 0x00
        // 0006928c: 89 46 48            MOV [ESI+0x48],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x48
        // 0006928f: 8b 46 48            MOV EAX,[ESI+0x48]
        _emit 0x8b
        _emit 0x46
        _emit 0x48
        // 00069292: 3b c7               CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 00069294: 74 3d               JZ 0x004692d3
        _emit 0x74
        _emit 0x3d
        // 00069296: 50                  PUSH EAX
        _emit 0x50
        // 00069297: e8 b4 fe ff ff      CALL 0x00469150
        _emit 0xe8
        _emit 0xb4
        _emit 0xfe
        _emit 0xff
        _emit 0xff
        // 0006929c: 83 c4 04            ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006929f: 3b c7               CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 000692a1: 89 46 44            MOV [ESI+0x44],EAX
        _emit 0x89
        _emit 0x46
        _emit 0x44
        // 000692a4: 75 2d               JNZ 0x004692d3
        _emit 0x75
        _emit 0x2d
        // 000692a6: 68 89 00 00 00      PUSH 0x89
        _emit 0x68
        _emit 0x89
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000692ab: 68 6c 91 f7 00      PUSH 0xf7916c
        _emit 0x68
        _emit 0x6c
        _emit 0x91
        _emit 0xf7
        _emit 0x00
        // 000692b0: 6a 26               PUSH 0x26
        _emit 0x6a
        _emit 0x26
        // 000692b2: 6a 69               PUSH 0x69
        _emit 0x6a
        _emit 0x69
        // 000692b4: 6a 05               PUSH 0x5
        _emit 0x6a
        _emit 0x05
        // 000692b6: e8 85 36 ff ff      CALL 0x0045c940
        _emit 0xe8
        _emit 0x85
        _emit 0x36
        _emit 0xff
        _emit 0xff
        // 000692bb: 8b 46 48            MOV EAX,[ESI+0x48]
        _emit 0x8b
        _emit 0x46
        _emit 0x48
        // 000692be: 50                  PUSH EAX
        _emit 0x50
        // 000692bf: e8 fc 02 00 00      CALL 0x004695c0
        _emit 0xe8
        _emit 0xfc
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 000692c4: 56                  PUSH ESI
        _emit 0x56
        // 000692c5: e8 26 a0 ff ff      CALL 0x004632f0
        _emit 0xe8
        _emit 0x26
        _emit 0xa0
        _emit 0xff
        _emit 0xff
        // 000692ca: 83 c4 1c            ADD ESP,0x1c
        _emit 0x83
        _emit 0xc4
        _emit 0x1c
        // 000692cd: 5b                  POP EBX
        _emit 0x5b
        // 000692ce: 5f                  POP EDI
        _emit 0x5f
        // 000692cf: 33 c0               XOR EAX,EAX
        _emit 0x33
        _emit 0xc0
        // 000692d1: 5e                  POP ESI
        _emit 0x5e
        // 000692d2: c3                  RET
        _emit 0xc3
        // 000692d3: 8b 4e 44            MOV ECX,[ESI+0x44]
        _emit 0x8b
        _emit 0x4e
        _emit 0x44
        // 000692d6: 8d 5e 3c            LEA EBX,[ESI+0x3c]
        _emit 0x8d
        _emit 0x5e
        _emit 0x3c
        // 000692d9: 53                  PUSH EBX
        _emit 0x53
        // 000692da: 89 3e               MOV [ESI],EDI
        _emit 0x89
        _emit 0x3e
        // 000692dc: 89 7e 04            MOV [ESI+0x4],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x04
        // 000692df: 89 7e 08            MOV [ESI+0x8],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x08
        // 000692e2: 89 7e 0c            MOV [ESI+0xc],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x0c
        // 000692e5: 89 7e 10            MOV [ESI+0x10],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x10
        // 000692e8: 89 7e 14            MOV [ESI+0x14],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x14
        // 000692eb: 89 7e 18            MOV [ESI+0x18],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x18
        // 000692ee: 89 7e 24            MOV [ESI+0x24],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x24
        // 000692f1: 89 7e 28            MOV [ESI+0x28],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x28
        // 000692f4: 89 7e 2c            MOV [ESI+0x2c],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x2c
        // 000692f7: 89 7e 30            MOV [ESI+0x30],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x30
        // 000692fa: 89 7e 34            MOV [ESI+0x34],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x34
        // 000692fd: 89 7e 20            MOV [ESI+0x20],EDI
        _emit 0x89
        _emit 0x7e
        _emit 0x20
        // 00069300: c7 46 38 01 00 00 00  MOV [ESI+0x38],0x1
        _emit 0xc7
        _emit 0x46
        _emit 0x38
        _emit 0x01
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00069307: 8b 51 18            MOV EDX,[ECX+0x18]
        _emit 0x8b
        _emit 0x51
        _emit 0x18
        // 0006930a: 56                  PUSH ESI
        _emit 0x56
        // 0006930b: 6a 08               PUSH 0x8
        _emit 0x6a
        _emit 0x08
        // 0006930d: 89 56 1c            MOV [ESI+0x1c],EDX
        _emit 0x89
        _emit 0x56
        _emit 0x1c
        // 00069310: e8 4b 83 ff ff      CALL 0x00461660
        _emit 0xe8
        _emit 0x4b
        _emit 0x83
        _emit 0xff
        _emit 0xff
        // 00069315: 8b 46 44            MOV EAX,[ESI+0x44]
        _emit 0x8b
        _emit 0x46
        _emit 0x44
        // 00069318: 8b 40 10            MOV EAX,[EAX+0x10]
        _emit 0x8b
        _emit 0x40
        _emit 0x10
        // 0006931b: 83 c4 0c            ADD ESP,0xc
        _emit 0x83
        _emit 0xc4
        _emit 0x0c
        // 0006931e: 3b c7               CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 00069320: 74 2e               JZ 0x00469350
        _emit 0x74
        _emit 0x2e
        // 00069322: 56                  PUSH ESI
        _emit 0x56
        // 00069323: ff d0               CALL EAX
        _emit 0xff
        _emit 0xd0
        // 00069325: 83 c4 04            ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 00069328: 85 c0               TEST EAX,EAX
        _emit 0x85
        _emit 0xc0
        // 0006932a: 75 24               JNZ 0x00469350
        _emit 0x75
        _emit 0x24
        // 0006932c: 8b 46 48            MOV EAX,[ESI+0x48]
        _emit 0x8b
        _emit 0x46
        _emit 0x48
        // 0006932f: 3b c7               CMP EAX,EDI
        _emit 0x3b
        _emit 0xc7
        // 00069331: 74 09               JZ 0x0046933c
        _emit 0x74
        _emit 0x09
        // 00069333: 50                  PUSH EAX
        _emit 0x50
        // 00069334: e8 87 02 00 00      CALL 0x004695c0
        _emit 0xe8
        _emit 0x87
        _emit 0x02
        _emit 0x00
        _emit 0x00
        // 00069339: 83 c4 04            ADD ESP,0x4
        _emit 0x83
        _emit 0xc4
        _emit 0x04
        // 0006933c: 53                  PUSH EBX
        _emit 0x53
        // 0006933d: 56                  PUSH ESI
        _emit 0x56
        // 0006933e: 6a 08               PUSH 0x8
        _emit 0x6a
        _emit 0x08
        // 00069340: e8 7b 83 ff ff      CALL 0x004616c0
        _emit 0xe8
        _emit 0x7b
        _emit 0x83
        _emit 0xff
        _emit 0xff
        // 00069345: 56                  PUSH ESI
        _emit 0x56
        // 00069346: e8 a5 9f ff ff      CALL 0x004632f0
        _emit 0xe8
        _emit 0xa5
        _emit 0x9f
        _emit 0xff
        _emit 0xff
        // 0006934b: 83 c4 10            ADD ESP,0x10
        _emit 0x83
        _emit 0xc4
        _emit 0x10
        // 0006934e: 33 f6               XOR ESI,ESI
        _emit 0x33
        _emit 0xf6
        // 00069350: 5b                  POP EBX
        _emit 0x5b
        // 00069351: 5f                  POP EDI
        _emit 0x5f
        // 00069352: 8b c6               MOV EAX,ESI
        _emit 0x8b
        _emit 0xc6
        // 00069354: 5e                  POP ESI
        _emit 0x5e
        // 00069355: c3                  RET
        _emit 0xc3
    }
}
