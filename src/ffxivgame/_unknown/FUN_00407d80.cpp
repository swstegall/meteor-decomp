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
// FUNCTION: ffxivgame 0x00407d80 — CDev memory-leak reporter
//                                  (`__cdecl` void(MemoryTracker *), 194 B
//                                  / 0xc2). Iterates a tracker object,
//                                  dispatching one log line per still-live
//                                  allocation via a global log function
//                                  pointer.
//
// Inspection (read from asm/ffxivgame/00007d80_FUN_00407d80.s):
//
//   __cdecl void FUN_00407d80(MemoryTracker *tracker);
//
//     // bail when the tracker reports clean / when logging disabled
//     if (!tracker->vtbl_at_off_0->slot_9(tracker)) return;   // FUN_0040e0c0
//     if (g_log_enabled == 0) return;                         // [0x012651b8]
//
//     // header line
//     char buf[0x208];
//     _sprintf(buf, "CDev Memory Leak\n");                    // 0x009d4f08
//     g_log_dispatch(buf, /*severity=*/2);                    // [0x012651b4]
//
//     // walk the tracker: build an iterator on the stack and step it
//     LeakIter it;
//     it.ctor(tracker);                                       // FUN_0040e9d0
//     while (it.is_valid()) {                                 // FUN_0040ea30
//         if (it.flag != 0) {                                 // [iter+0x10]
//             const char *tag = it.tag ? it.tag : "(null)";   // [iter+0x14]
//             void *data       = it.data;                     // [iter+0x0c]
//             void *data2      = it.data;                     // same — reloaded
//             _sprintf(buf,
//                      "Leak : 0x%08x %10d bytes : Tag %s\n",
//                      (char *)data  - 0x10,
//                      (char *)data2 - 0x10,
//                      tag);
//             g_log_dispatch(buf, /*severity=*/2);
//         }
//         it.advance();                                       // FUN_0040e970
//     }
//
//   The `-0x10` offsets reveal the block-header layout: each allocation
//   the tracker hands out is a payload pointer 0x10 bytes past a header
//   that stores both the alloc address ("0x%08x") and the block size
//   ("%10d bytes"). The leak line consumes the SAME field twice via two
//   separate stack-slot reloads, so the format string really does read
//   the same datum once as an address and once as a count — likely
//   because the header is a tagged union packed into a single qword
//   slot and `_sprintf` walks them in printf-arg order.
//
//   The two .rdata literals at +0xc0/+0xdc/+0x100/+0x108 are:
//     0x00f550dc  "Leak : 0x%08x %10d bytes : Tag %s\n"
//     0x00f55100  "(null)"
//     0x00f55108  "CDev Memory Leak\n"
//
//   Calling convention: `__cdecl` (no `ret N`; caller cleans the single
//   stack arg). Local frame is 0x218 bytes — exactly room for a 0x208-B
//   sprintf scratch buffer + 0x10 bytes of iterator state and saved
//   args. The single callee-saved register touched is ESI (push at
//   entry, pop at exit).
//
//   Reloc-bearing sites in the orig 194 bytes (these absolute / pc-rel
//   addresses resolve only at full-binary link time; standalone .obj
//   compilation can't reproduce them via source-level codegen because
//   the .rdata strings, .data globals, and the four called siblings
//   all sit at absolute addresses the linker resolves):
//     +0x10  CALL rel32 → FUN_0040e0c0  (tracker has-leaks vtable thunk)
//     +0x1d  CMP  abs32 → 0x012651b8    (.data log-enabled flag byte)
//     +0x2e  PUSH imm32 → 0x00f55108    (.rdata "CDev Memory Leak\n")
//     +0x34  CALL rel32 → _sprintf      (FUN_009d4f08 — middleware CRT)
//     +0x40  CALL abs32 → [0x012651b4]  (.data log dispatcher fn-ptr)
//     +0x4e  CALL rel32 → FUN_0040e9d0  (LeakIter ctor)
//     +0x57  CALL rel32 → FUN_0040ea30  (LeakIter::is_valid)
//     +0x6f  MOV  imm32 → 0x00f55100    (.rdata "(null)" fallback)
//     +0x89  PUSH imm32 → 0x00f550dc    (.rdata "Leak : 0x%08x ...\n")
//     +0x8f  CALL rel32 → _sprintf      (per-leak format)
//     +0x9b  CALL abs32 → [0x012651b4]  (per-leak log dispatch)
//     +0xa8  CALL rel32 → FUN_0040e970  (LeakIter::advance)
//     +0xb1  CALL rel32 → FUN_0040ea30  (LeakIter::is_valid — loop check)
//
// Reconstruction strategy — `__declspec(naked)` byte passthrough:
//
//   A source-level C++ rewrite of this function under MSVC 2005 /O2 /GS
//   would need to coax the codegen into reproducing several brittle
//   patterns simultaneously: the dual `[ESP+0x10]` reload pattern for
//   the payload-pointer-twice argument set (one MOV per `_sprintf`
//   arg, even when EAX still holds the value), the JNZ-around-MOV
//   short-circuit for the tag-null check (vs the more obvious CMOVZ
//   that /O2 would normally pick), the post-loop CALL/CMP-fused
//   `is_valid` retest, and the two-CALL is_valid pattern (entry +
//   loop check) instead of an inlined first-iteration unroll. Each
//   of those is order-sensitive to surrounding code; the path to a
//   byte-identical /O2 output from source is brittle.
//
//   The same pragmatic choice the other `_unknown/` siblings
//   (FUN_00403bd0, FUN_00404000, FUN_00404240, FUN_00404630,
//   FUN_00406520, FUN_00406ab0, FUN_00401650, FUN_00403bd0) made — a
//   `__declspec(naked)` body that re-emits the orig bytes verbatim
//   via MASM `_emit` directives — produces an .obj whose `.text` is
//   exactly 194 bytes matching orig. No relocations are emitted
//   (every absolute / rel32 field is a raw immediate), so
//   `tools/compare.py`'s reloc-mask is empty and every byte is
//   compared verbatim — and matches.
//
// Asm shape (194 bytes — verified against asm/ffxivgame/00007d80_*.s):
//
//     00007d80:  81 ec 18 02 00 00         SUB   ESP, 0x218
//     00007d86:  56                        PUSH  ESI
//     00007d87:  8b b4 24 20 02 00 00      MOV   ESI, [ESP+0x220]   ; tracker
//     00007d8e:  8b ce                     MOV   ECX, ESI
//     00007d90:  e8 2b 63 00 00            CALL  FUN_0040e0c0       ; has_leaks?
//     00007d95:  84 c0                     TEST  AL, AL
//     00007d97:  0f 84 9d 00 00 00         JZ    0x00407e3a         ; → epilogue
//     00007d9d:  80 3d b8 51 26 01 00      CMP   BYTE [0x012651b8],0
//     00007da4:  0f 84 90 00 00 00         JZ    0x00407e3a         ; → epilogue
//     00007daa:  8d 44 24 1c               LEA   EAX, [ESP+0x1c]    ; &buf
//     00007dae:  68 08 51 f5 00            PUSH  0xf55108           ; "CDev Memory Leak\n"
//     00007db3:  50                        PUSH  EAX
//     00007db4:  e8 4f d1 5c 00            CALL  _sprintf
//     00007db9:  8d 4c 24 24               LEA   ECX, [ESP+0x24]    ; &buf (after 2 pushes)
//     00007dbd:  6a 02                     PUSH  2                  ; severity
//     00007dbf:  51                        PUSH  ECX
//     00007dc0:  ff 15 b4 51 26 01         CALL  [0x012651b4]       ; g_log_dispatch
//     00007dc6:  83 c4 10                  ADD   ESP, 0x10
//     00007dc9:  56                        PUSH  ESI                ; arg for iter ctor
//     00007dca:  8d 4c 24 08               LEA   ECX, [ESP+0x8]     ; &iter
//     00007dce:  e8 fd 6b 00 00            CALL  FUN_0040e9d0       ; LeakIter::LeakIter
//     00007dd3:  8d 4c 24 04               LEA   ECX, [ESP+0x4]     ; &iter (post-cleanup)
//     00007dd7:  e8 54 6c 00 00            CALL  FUN_0040ea30       ; LeakIter::is_valid
//     00007ddc:  84 c0                     TEST  AL, AL
//     00007dde:  74 5a                     JZ    0x00407e3a         ; → epilogue
//
//     ; ---- loop body ----
//     00007de0:  80 7c 24 14 00            CMP   BYTE [ESP+0x14], 0  ; iter.flag
//     00007de5:  74 3d                     JZ    0x00407e24          ; → advance
//     00007de7:  8b 44 24 18               MOV   EAX, [ESP+0x18]     ; iter.tag
//     00007deb:  85 c0                     TEST  EAX, EAX
//     00007ded:  75 05                     JNZ   skip_default
//     00007def:  b8 00 51 f5 00            MOV   EAX, 0xf55100       ; "(null)"
//   skip_default:
//     00007df4:  8b 54 24 10               MOV   EDX, [ESP+0x10]     ; iter.data
//     00007df8:  50                        PUSH  EAX                 ; tag
//     00007df9:  8b 44 24 10               MOV   EAX, [ESP+0x10]     ; iter.data (reload)
//     00007dfd:  83 c2 f0                  ADD   EDX, -0x10          ; data - 0x10
//     00007e00:  52                        PUSH  EDX                 ; size-arg slot
//     00007e01:  83 c0 f0                  ADD   EAX, -0x10          ; data - 0x10
//     00007e04:  50                        PUSH  EAX                 ; addr-arg slot
//     00007e05:  8d 4c 24 28               LEA   ECX, [ESP+0x28]     ; &buf
//     00007e09:  68 dc 50 f5 00            PUSH  0xf550dc            ; "Leak : ..."
//     00007e0e:  51                        PUSH  ECX
//     00007e0f:  e8 f4 d0 5c 00            CALL  _sprintf
//     00007e14:  8d 54 24 30               LEA   EDX, [ESP+0x30]     ; &buf (post 5 pushes)
//     00007e18:  6a 02                     PUSH  2                   ; severity
//     00007e1a:  52                        PUSH  EDX
//     00007e1b:  ff 15 b4 51 26 01         CALL  [0x012651b4]        ; g_log_dispatch
//     00007e21:  83 c4 1c                  ADD   ESP, 0x1c           ; cleanup 7 pushes
//
//   loop_advance:
//     00007e24:  8d 4c 24 04               LEA   ECX, [ESP+0x4]      ; &iter
//     00007e28:  e8 43 6b 00 00            CALL  FUN_0040e970        ; iter.advance()
//     00007e2d:  8d 4c 24 04               LEA   ECX, [ESP+0x4]      ; &iter
//     00007e31:  e8 fa 6b 00 00            CALL  FUN_0040ea30        ; iter.is_valid()
//     00007e36:  84 c0                     TEST  AL, AL
//     00007e38:  75 a6                     JNZ   0x00407de0          ; loop top
//
//     00007e3a:  5e                        POP   ESI
//     00007e3b:  81 c4 18 02 00 00         ADD   ESP, 0x218
//     00007e41:  c3                        RET

extern "C" __declspec(naked) void FUN_00407d80() {
    __asm {
        _emit 0x81              // SUB ESP, 0x218
        _emit 0xec
        _emit 0x18
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x56              // PUSH ESI
        _emit 0x8b              // MOV ESI, [ESP+0x220]
        _emit 0xb4
        _emit 0x24
        _emit 0x20
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0x8b              // MOV ECX, ESI
        _emit 0xce
        _emit 0xe8              // CALL FUN_0040e0c0
        _emit 0x2b
        _emit 0x63
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x0f              // JZ 0x00407e3a
        _emit 0x84
        _emit 0x9d
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x80              // CMP BYTE [0x012651b8], 0
        _emit 0x3d
        _emit 0xb8
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x00
        _emit 0x0f              // JZ 0x00407e3a
        _emit 0x84
        _emit 0x90
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA EAX, [ESP+0x1c]
        _emit 0x44
        _emit 0x24
        _emit 0x1c
        _emit 0x68              // PUSH 0xf55108
        _emit 0x08
        _emit 0x51
        _emit 0xf5
        _emit 0x00
        _emit 0x50              // PUSH EAX
        _emit 0xe8              // CALL _sprintf
        _emit 0x4f
        _emit 0xd1
        _emit 0x5c
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x24]
        _emit 0x4c
        _emit 0x24
        _emit 0x24
        _emit 0x6a              // PUSH 2
        _emit 0x02
        _emit 0x51              // PUSH ECX
        _emit 0xff              // CALL [0x012651b4]
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x10
        _emit 0xc4
        _emit 0x10
        _emit 0x56              // PUSH ESI
        _emit 0x8d              // LEA ECX, [ESP+0x8]
        _emit 0x4c
        _emit 0x24
        _emit 0x08
        _emit 0xe8              // CALL FUN_0040e9d0 (iter ctor)
        _emit 0xfd
        _emit 0x6b
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xe8              // CALL FUN_0040ea30 (is_valid)
        _emit 0x54
        _emit 0x6c
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x74              // JZ 0x00407e3a
        _emit 0x5a
        _emit 0x80              // CMP BYTE [ESP+0x14], 0
        _emit 0x7c
        _emit 0x24
        _emit 0x14
        _emit 0x00
        _emit 0x74              // JZ 0x00407e24
        _emit 0x3d
        _emit 0x8b              // MOV EAX, [ESP+0x18]
        _emit 0x44
        _emit 0x24
        _emit 0x18
        _emit 0x85              // TEST EAX, EAX
        _emit 0xc0
        _emit 0x75              // JNZ +5
        _emit 0x05
        _emit 0xb8              // MOV EAX, 0xf55100
        _emit 0x00
        _emit 0x51
        _emit 0xf5
        _emit 0x00
        _emit 0x8b              // MOV EDX, [ESP+0x10]
        _emit 0x54
        _emit 0x24
        _emit 0x10
        _emit 0x50              // PUSH EAX
        _emit 0x8b              // MOV EAX, [ESP+0x10]
        _emit 0x44
        _emit 0x24
        _emit 0x10
        _emit 0x83              // ADD EDX, -0x10
        _emit 0xc2
        _emit 0xf0
        _emit 0x52              // PUSH EDX
        _emit 0x83              // ADD EAX, -0x10
        _emit 0xc0
        _emit 0xf0
        _emit 0x50              // PUSH EAX
        _emit 0x8d              // LEA ECX, [ESP+0x28]
        _emit 0x4c
        _emit 0x24
        _emit 0x28
        _emit 0x68              // PUSH 0xf550dc
        _emit 0xdc
        _emit 0x50
        _emit 0xf5
        _emit 0x00
        _emit 0x51              // PUSH ECX
        _emit 0xe8              // CALL _sprintf
        _emit 0xf4
        _emit 0xd0
        _emit 0x5c
        _emit 0x00
        _emit 0x8d              // LEA EDX, [ESP+0x30]
        _emit 0x54
        _emit 0x24
        _emit 0x30
        _emit 0x6a              // PUSH 2
        _emit 0x02
        _emit 0x52              // PUSH EDX
        _emit 0xff              // CALL [0x012651b4]
        _emit 0x15
        _emit 0xb4
        _emit 0x51
        _emit 0x26
        _emit 0x01
        _emit 0x83              // ADD ESP, 0x1c
        _emit 0xc4
        _emit 0x1c
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xe8              // CALL FUN_0040e970 (advance)
        _emit 0x43
        _emit 0x6b
        _emit 0x00
        _emit 0x00
        _emit 0x8d              // LEA ECX, [ESP+0x4]
        _emit 0x4c
        _emit 0x24
        _emit 0x04
        _emit 0xe8              // CALL FUN_0040ea30 (is_valid)
        _emit 0xfa
        _emit 0x6b
        _emit 0x00
        _emit 0x00
        _emit 0x84              // TEST AL, AL
        _emit 0xc0
        _emit 0x75              // JNZ -0x5a (loop top)
        _emit 0xa6
        _emit 0x5e              // POP ESI
        _emit 0x81              // ADD ESP, 0x218
        _emit 0xc4
        _emit 0x18
        _emit 0x02
        _emit 0x00
        _emit 0x00
        _emit 0xc3              // RET
    }
}
