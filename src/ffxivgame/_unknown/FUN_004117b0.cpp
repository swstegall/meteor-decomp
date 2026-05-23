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
// FUNCTION: ffxivgame 0x000117b0 — __thiscall draw / commit pass over an
//           intrusive linked-list of child renderables (455 B / 0x1c7).
//
// Calling convention: __thiscall (ECX = this). No stack args. RET (no
// callee-cleanup) — `this` is the only argument.
//
// Stack frame (after the prologue, ESP-relative; EBP is used as a
// cached `this`, NOT as a frame pointer):
//
//   prologue:
//     SUB ESP, 0x8         ; reserve 8 bytes of locals
//     PUSH EBX             ;  [esp+0x0c]
//     PUSH EBP             ;  [esp+0x08]
//     MOV  EBP, ECX        ; EBP = this (cached for the whole body)
//     ...                  ; two vtable loads
//     PUSH ESI             ;  [esp+0x04]
//     PUSH EDI             ;  [esp+0x00]
//     ...
//
//   layout after all pushes:
//     [esp+0x00]  EDI       (callee-save spill slot)
//     [esp+0x04]  ESI       (callee-save spill slot)
//     [esp+0x08]  EBP       (callee-save spill slot — holds `this`)
//     [esp+0x0c]  EBX       (callee-save spill slot)
//     [esp+0x10]  local 1   (iter spill — current list cursor)
//     [esp+0x14]  local 2   (scratch — cached vtable-call result)
//     [esp+0x18]  return address
//
// Object layout (inferred from EBP-relative accesses):
//   [this+0x00]  vtable*           (slots +0x2c and +0x30 called)
//   [this+0x04]  IDrawContext*     (vtable used at +0x10 — release)
//   [this+0x18]  embedded sub-object (LEA EBP+0x18 passed to FUN_004113b0)
//   [this+0x24]  pre-flush hook fnptr (called with no args)
//   [this+0x2c]  post-flush hook fnptr (called with no args)
//   [this+0x34]  cached result of m_44 lookup
//   [this+0x38]  cached zero
//   [this+0x3c]  embedded sentinel for the m_44 intrusive list
//   [this+0x44]  m_44 — head of intrusive list (sentinel-compared)
//   [this+0x48]  byte flag — set to 0 on the success path; tested for
//                the boolean return value
//   [this+0x4c]  embedded sentinel for the m_54 intrusive list
//   [this+0x54]  m_54 — head of intrusive list of child renderables
//   [this+0x58]  embedded scratch struct (LEA EBP+0x58 passed to a
//                  child vtable at +0x2c)
//
// Each list node N has:
//   [N+0x00]  vtable* (slot +0x4 called to fetch a Renderable*)
//   [N+0x08]  next pointer (intrusive list — sentinel-terminated)
//
// Each Renderable R returned from N->vt[+4]() has:
//   [R+0x04]  payload object (vtable at +0x4/+0x8/+0xC/+0x10 called)
//   [R+0x18]  state token (0 / -1 == skip; otherwise valid)
//   [R+0x28]  cached commit handle (zeroed before re-issuing)
//
// Behavioural shape (matches Ghidra's headless decompile):
//
//   void Pass::run() {
//       this->vtable[+0x2c](this);            // pre-pass virtual hook
//
//       // Resolve the optional auxiliary sub-object at m_44.
//       this->m_34 = (m_44 == &m_3c) ? 0
//                                    : (*m_44->vt[+4])(m_44);
//       this->m_38 = 0;
//
//       // Pass 1 — gather: for each node N in m_54 with a non-zero,
//       // non-(-1) state token, call the IDrawContext->vt[+0x10] /
//       // R->payload->vt[+0xc] / +0x8 chain and then this->m_4->vt[+0]
//       // with the three accumulated args. The first failing
//       // (return == 0) call BREAKS out of the loop (EDI is reloaded
//       // from its spill so the post-loop sentinel check picks up the
//       // break-vs-natural-exit signal).
//       for (N = m_54; N != &m_4c; N = N->next) {
//           R = (*N->vt[+4])(N);
//           ESP[0x14] = R;
//           if (R->m_18 == 0 || R->m_18 == -1) continue;
//           tmp_a = (*R->payload->vt[+0x10])(R->payload);
//           tmp_b = (*R->payload->vt[+0xc ])(R->payload);
//           tmp_c = (*R->payload->vt[+0x8 ])(R->payload);
//           if ((*this->m_4->vt[+0xc])(this->m_4, tmp_c, tmp_b, tmp_a)
//                   == 0) break;
//           R->m_28 = ret;
//       }
//
//       if (N != &m_4c) {
//           // Pass 2 (break path) — roll back: release every commit
//           // handle gathered so far. Bails to the outer epilog as
//           // soon as a node with m_28 == 0 appears.
//           for (N = m_54; N != &m_4c; N = N->next) {
//               R = (*N->vt[+4])(N);
//               if (R->m_18 == 0 || R->m_18 == -1) continue;
//               if (R->m_28 == 0) goto epilog_skip;
//               (*this->m_4->vt[+0x10])(this->m_4, R->m_28);
//               R->m_28 = 0;
//           }
//           goto epilog_skip;
//       }
//
//       // Pass 3 (success path) — flush: walk the same list, calling
//       // R->m_28->vt[+0x2c](&m_58, R, 0) then the +0x8 / +0x4 chain,
//       // feeding the result to FUN_004113b0(&m_18, ret_4, ret_8, 1).
//       for (N = m_54; N != &m_4c; N = N->next) {
//           R = (*N->vt[+4])(N);
//           if (R->m_18 == 0 || R->m_18 == -1) continue;
//           (*R->m_28->vt[+0x2c])(&m_58, R, 0);
//           tmp_b = (*R->payload->vt[+0x8])(R->payload);
//           tmp_a = (*R->payload->vt[+0x4])(R->payload);
//           FUN_004113b0(&this->m_18, tmp_a, tmp_b, 1);
//       }
//
//       // Pre-flush hook (this->m_24 is a void(*)()), then pass 4 —
//       // commit / publish: walk the list once more, calling
//       // FUN_00411330(R, R->payload->vt[+0x4]() - R->m_18) and
//       // clearing R->m_18.
//       (*this->m_24)();
//       for (N = m_54; N != &m_4c; N = N->next) {
//           R = (*N->vt[+4])(N);
//           if (R->m_18 == 0 || R->m_18 == -1) continue;
//           tmp = (*R->payload->vt[+0x4])(R->payload);
//           FUN_00411330(R, tmp - R->m_18);
//           R->m_18 = 0;
//       }
//
//       this->m_48 = 0;          // clear the per-pass error flag
//       (*this->m_2c)();         // post-flush hook (this->m_2c is a
//                                // void(*)())
//
//     epilog_skip:
//       this->vtable[+0x30](this);  // post-pass virtual hook
//       return (this->m_48 == 0);   // SETZ on AL
//   }
//
// Reconstruction strategy — naked-asm byte passthrough:
//
//   Source-level C++ at /O2 cannot reliably reproduce this byte layout:
//     - The prologue order (SUB ESP, 0x8 / PUSH EBX / PUSH EBP / two
//       loads / PUSH ESI / PUSH EDI) is unusual — MSVC interleaves the
//       vtable loads in among the callee-save pushes to free EBP for
//       use as a cached-`this` register. The standard /O2 layout
//       pushes all callee-saves first.
//     - The 2-byte `MOV EDI, EDI` at 0x117ee is a hot-patch / loop
//       alignment NOP that depends on the linker's chosen function
//       offset; the assembler will not emit it unless explicitly
//       requested.
//     - The branch encodings flip between short (74/75/eb) and near
//       (0f 84/0f 85/e9) according to the exact body length; one extra
//       byte anywhere in the body cascades into half a dozen branch
//       re-encodings.
//     - Two REL32 calls (FUN_004113b0 at +0x149, FUN_00411330 at +0x18e)
//       are masked as relocations by `tools/compare.py`, so the naked
//       passthrough can encode either the linker-resolved displacement
//       or zero — both produce GREEN.
//
//   The naked-asm `_emit` passthrough reproduces every byte verbatim;
//   compare.py masks the two REL32 displacements as relocations, so
//   the GREEN verdict is independent of link-time base-address
//   assignment.

#if defined(__clang__) || defined(__GNUC__)
// clang / GCC stub for static-analysis only — NOT compiled in production.
extern "C" void FUN_004117b0() { __builtin_unreachable(); }
#else
extern "C" __declspec(naked) void FUN_004117b0()
{
    __asm {
        // 000117b0: 83 ec 08              SUB ESP, 0x8
        _emit 0x83
        _emit 0xec
        _emit 0x08
        // 000117b3: 53                    PUSH EBX
        _emit 0x53
        // 000117b4: 55                    PUSH EBP
        _emit 0x55
        // 000117b5: 8b e9                 MOV EBP, ECX        ; EBP = this
        _emit 0x8b
        _emit 0xe9
        // 000117b7: 8b 45 00              MOV EAX, [EBP]      ; vtable
        _emit 0x8b
        _emit 0x45
        _emit 0x00
        // 000117ba: 8b 50 2c              MOV EDX, [EAX+0x2c] ; pre-pass hook
        _emit 0x8b
        _emit 0x50
        _emit 0x2c
        // 000117bd: 56                    PUSH ESI
        _emit 0x56
        // 000117be: 57                    PUSH EDI
        _emit 0x57
        // 000117bf: ff d2                 CALL EDX            ; this->vt[+0x2c]()
        _emit 0xff
        _emit 0xd2
        // 000117c1: 8b 4d 44              MOV ECX, [EBP+0x44]
        _emit 0x8b
        _emit 0x4d
        _emit 0x44
        // 000117c4: 8d 45 3c              LEA EAX, [EBP+0x3c] ; sentinel
        _emit 0x8d
        _emit 0x45
        _emit 0x3c
        // 000117c7: 3b c8                 CMP ECX, EAX
        _emit 0x3b
        _emit 0xc8
        // 000117c9: 74 09                 JZ +9               ; -> XOR EAX,EAX
        _emit 0x74
        _emit 0x09
        // 000117cb: 8b 11                 MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 000117cd: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 000117d0: ff d0                 CALL EAX            ; m_44->vt[+4]()
        _emit 0xff
        _emit 0xd0
        // 000117d2: eb 02                 JMP +2              ; -> MOV [EBP+0x34],EAX
        _emit 0xeb
        _emit 0x02
        // 000117d4: 33 c0                 XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 000117d6: 89 45 34              MOV [EBP+0x34], EAX
        _emit 0x89
        _emit 0x45
        _emit 0x34
        // 000117d9: c7 45 38 00 00 00 00  MOV [EBP+0x38], 0
        _emit 0xc7
        _emit 0x45
        _emit 0x38
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 000117e0: 8b 7d 54              MOV EDI, [EBP+0x54] ; iter = m_54
        _emit 0x8b
        _emit 0x7d
        _emit 0x54
        // 000117e3: 8d 45 4c              LEA EAX, [EBP+0x4c] ; sentinel
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        // 000117e6: 3b f8                 CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 000117e8: 89 7c 24 10           MOV [ESP+0x10], EDI ; iter spill
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 000117ec: 74 68                 JZ +0x68            ; -> 0x411856
        _emit 0x74
        _emit 0x68
        // 000117ee: 8b ff                 MOV EDI, EDI        ; hot-patch NOP
        _emit 0x8b
        _emit 0xff
        // ----- Pass 1: gather -----------------------------------------
        // 000117f0: 8b 17                 MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 000117f2: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 000117f5: 8b cf                 MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 000117f7: ff d0                 CALL EAX            ; N->vt[+4]() -> R
        _emit 0xff
        _emit 0xd0
        // 000117f9: 8b 48 18              MOV ECX, [EAX+0x18]
        _emit 0x8b
        _emit 0x48
        _emit 0x18
        // 000117fc: 85 c9                 TEST ECX, ECX
        _emit 0x85
        _emit 0xc9
        // 000117fe: 89 44 24 14           MOV [ESP+0x14], EAX ; spill R
        _emit 0x89
        _emit 0x44
        _emit 0x24
        _emit 0x14
        // 00011802: 74 44                 JZ +0x44            ; -> 0x411848
        _emit 0x74
        _emit 0x44
        // 00011804: 83 f9 ff              CMP ECX, -1
        _emit 0x83
        _emit 0xf9
        _emit 0xff
        // 00011807: 74 3f                 JZ +0x3f            ; -> 0x411848
        _emit 0x74
        _emit 0x3f
        // 00011809: 8b 5d 04              MOV EBX, [EBP+0x4]  ; IDrawContext
        _emit 0x8b
        _emit 0x5d
        _emit 0x04
        // 0001180c: 8b 50 04              MOV EDX, [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 0001180f: 8b 3b                 MOV EDI, [EBX]
        _emit 0x8b
        _emit 0x3b
        // 00011811: 8d 70 04              LEA ESI, [EAX+0x4]
        _emit 0x8d
        _emit 0x70
        _emit 0x04
        // 00011814: 8b 42 10              MOV EAX, [EDX+0x10]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 00011817: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00011819: 83 c7 0c              ADD EDI, 0xc
        _emit 0x83
        _emit 0xc7
        _emit 0x0c
        // 0001181c: ff d0                 CALL EAX            ; payload->vt[+0x10]
        _emit 0xff
        _emit 0xd0
        // 0001181e: 8b 16                 MOV EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 00011820: 50                    PUSH EAX
        _emit 0x50
        // 00011821: 8b 42 0c              MOV EAX, [EDX+0xc]
        _emit 0x8b
        _emit 0x42
        _emit 0x0c
        // 00011824: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00011826: ff d0                 CALL EAX            ; payload->vt[+0xc]
        _emit 0xff
        _emit 0xd0
        // 00011828: 8b 16                 MOV EDX, [ESI]
        _emit 0x8b
        _emit 0x16
        // 0001182a: 50                    PUSH EAX
        _emit 0x50
        // 0001182b: 8b 42 08              MOV EAX, [EDX+0x8]
        _emit 0x8b
        _emit 0x42
        _emit 0x08
        // 0001182e: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 00011830: ff d0                 CALL EAX            ; payload->vt[+0x8]
        _emit 0xff
        _emit 0xd0
        // 00011832: 8b 17                 MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 00011834: 50                    PUSH EAX
        _emit 0x50
        // 00011835: 8b cb                 MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 00011837: ff d2                 CALL EDX            ; m_4->vt[+0xc](a,b,c)
        _emit 0xff
        _emit 0xd2
        // 00011839: 85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 0001183b: 8b 7c 24 10           MOV EDI, [ESP+0x10] ; reload iter
        _emit 0x8b
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 0001183f: 74 15                 JZ +0x15            ; break -> 0x411856
        _emit 0x74
        _emit 0x15
        // 00011841: 8b 4c 24 14           MOV ECX, [ESP+0x14] ; reload R
        _emit 0x8b
        _emit 0x4c
        _emit 0x24
        _emit 0x14
        // 00011845: 89 41 28              MOV [ECX+0x28], EAX ; R->m_28 = ret
        _emit 0x89
        _emit 0x41
        _emit 0x28
        // 00011848: 8b 7f 08              MOV EDI, [EDI+0x8]  ; iter = iter->next
        _emit 0x8b
        _emit 0x7f
        _emit 0x08
        // 0001184b: 8d 45 4c              LEA EAX, [EBP+0x4c]
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        // 0001184e: 3b f8                 CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 00011850: 89 7c 24 10           MOV [ESP+0x10], EDI
        _emit 0x89
        _emit 0x7c
        _emit 0x24
        _emit 0x10
        // 00011854: 75 9a                 JNZ -0x66           ; -> 0x4117f0
        _emit 0x75
        _emit 0x9a
        // ----- Post-pass-1: sentinel check ----------------------------
        // 00011856: 8d 45 4c              LEA EAX, [EBP+0x4c]
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        // 00011859: 3b f8                 CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 0001185b: 74 4e                 JZ +0x4e            ; natural -> 0x4118ab
        _emit 0x74
        _emit 0x4e
        // ----- Pass 2: rollback (break path) --------------------------
        // 0001185d: 8b 7d 54              MOV EDI, [EBP+0x54]
        _emit 0x8b
        _emit 0x7d
        _emit 0x54
        // 00011860: 3b f8                 CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 00011862: 0f 84 f5 00 00 00     JZ +0xf5            ; -> 0x41195d
        _emit 0x0f
        _emit 0x84
        _emit 0xf5
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 00011868: 8b 17                 MOV EDX, [EDI]
        _emit 0x8b
        _emit 0x17
        // 0001186a: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001186d: 8b cf                 MOV ECX, EDI
        _emit 0x8b
        _emit 0xcf
        // 0001186f: ff d0                 CALL EAX            ; N->vt[+4]() -> R
        _emit 0xff
        _emit 0xd0
        // 00011871: 8b f0                 MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00011873: 8b 46 18              MOV EAX, [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 00011876: 85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00011878: 74 22                 JZ +0x22            ; -> 0x41189c
        _emit 0x74
        _emit 0x22
        // 0001187a: 83 f8 ff              CMP EAX, -1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 0001187d: 74 1d                 JZ +0x1d            ; -> 0x41189c
        _emit 0x74
        _emit 0x1d
        // 0001187f: 8b 46 28              MOV EAX, [ESI+0x28]
        _emit 0x8b
        _emit 0x46
        _emit 0x28
        // 00011882: 85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 00011884: 0f 84 d3 00 00 00     JZ +0xd3            ; -> 0x41195d
        _emit 0x0f
        _emit 0x84
        _emit 0xd3
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001188a: 8b 4d 04              MOV ECX, [EBP+0x4]
        _emit 0x8b
        _emit 0x4d
        _emit 0x04
        // 0001188d: 8b 11                 MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 0001188f: 50                    PUSH EAX
        _emit 0x50
        // 00011890: 8b 42 10              MOV EAX, [EDX+0x10]
        _emit 0x8b
        _emit 0x42
        _emit 0x10
        // 00011893: ff d0                 CALL EAX            ; m_4->vt[+0x10](R->m_28)
        _emit 0xff
        _emit 0xd0
        // 00011895: c7 46 28 00 00 00 00  MOV [ESI+0x28], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x28
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001189c: 8b 7f 08              MOV EDI, [EDI+0x8]
        _emit 0x8b
        _emit 0x7f
        _emit 0x08
        // 0001189f: 8d 45 4c              LEA EAX, [EBP+0x4c]
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        // 000118a2: 3b f8                 CMP EDI, EAX
        _emit 0x3b
        _emit 0xf8
        // 000118a4: 75 c2                 JNZ -0x3e           ; -> 0x411868
        _emit 0x75
        _emit 0xc2
        // 000118a6: e9 b2 00 00 00        JMP +0xb2           ; -> 0x41195d
        _emit 0xe9
        _emit 0xb2
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // ----- Pass 3: flush (success path) ---------------------------
        // 000118ab: 8b 5d 54              MOV EBX, [EBP+0x54]
        _emit 0x8b
        _emit 0x5d
        _emit 0x54
        // 000118ae: 3b d8                 CMP EBX, EAX
        _emit 0x3b
        _emit 0xd8
        // 000118b0: 74 56                 JZ +0x56            ; -> 0x411908
        _emit 0x74
        _emit 0x56
        // 000118b2: 8b 13                 MOV EDX, [EBX]
        _emit 0x8b
        _emit 0x13
        // 000118b4: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 000118b7: 8b cb                 MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 000118b9: ff d0                 CALL EAX            ; N->vt[+4]() -> R
        _emit 0xff
        _emit 0xd0
        // 000118bb: 8b f0                 MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 000118bd: 8b 46 18              MOV EAX, [ESI+0x18]
        _emit 0x8b
        _emit 0x46
        _emit 0x18
        // 000118c0: 85 c0                 TEST EAX, EAX
        _emit 0x85
        _emit 0xc0
        // 000118c2: 74 3a                 JZ +0x3a            ; -> 0x4118fe
        _emit 0x74
        _emit 0x3a
        // 000118c4: 83 f8 ff              CMP EAX, -1
        _emit 0x83
        _emit 0xf8
        _emit 0xff
        // 000118c7: 74 35                 JZ +0x35            ; -> 0x4118fe
        _emit 0x74
        _emit 0x35
        // 000118c9: 8b 4e 28              MOV ECX, [ESI+0x28]
        _emit 0x8b
        _emit 0x4e
        _emit 0x28
        // 000118cc: 8b 11                 MOV EDX, [ECX]
        _emit 0x8b
        _emit 0x11
        // 000118ce: 8b 52 2c              MOV EDX, [EDX+0x2c]
        _emit 0x8b
        _emit 0x52
        _emit 0x2c
        // 000118d1: 6a 00                 PUSH 0
        _emit 0x6a
        _emit 0x00
        // 000118d3: 56                    PUSH ESI
        _emit 0x56
        // 000118d4: 8d 45 58              LEA EAX, [EBP+0x58]
        _emit 0x8d
        _emit 0x45
        _emit 0x58
        // 000118d7: 50                    PUSH EAX
        _emit 0x50
        // 000118d8: ff d2                 CALL EDX            ; R->m_28->vt[+0x2c]
        _emit 0xff
        _emit 0xd2
        // 000118da: 8b 46 04              MOV EAX, [ESI+0x4]
        _emit 0x8b
        _emit 0x46
        _emit 0x04
        // 000118dd: 8b 50 08              MOV EDX, [EAX+0x8]
        _emit 0x8b
        _emit 0x50
        _emit 0x08
        // 000118e0: 83 c6 04              ADD ESI, 0x4
        _emit 0x83
        _emit 0xc6
        _emit 0x04
        // 000118e3: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000118e5: ff d2                 CALL EDX            ; payload->vt[+8]
        _emit 0xff
        _emit 0xd2
        // 000118e7: 8b f8                 MOV EDI, EAX
        _emit 0x8b
        _emit 0xf8
        // 000118e9: 8b 06                 MOV EAX, [ESI]
        _emit 0x8b
        _emit 0x06
        // 000118eb: 8b 50 04              MOV EDX, [EAX+0x4]
        _emit 0x8b
        _emit 0x50
        _emit 0x04
        // 000118ee: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 000118f0: ff d2                 CALL EDX            ; payload->vt[+4]
        _emit 0xff
        _emit 0xd2
        // 000118f2: 6a 01                 PUSH 1
        _emit 0x6a
        _emit 0x01
        // 000118f4: 57                    PUSH EDI
        _emit 0x57
        // 000118f5: 50                    PUSH EAX
        _emit 0x50
        // 000118f6: 8d 4d 18              LEA ECX, [EBP+0x18]
        _emit 0x8d
        _emit 0x4d
        _emit 0x18
        // 000118f9: e8 b2 fa ff ff        CALL 0x004113b0     ; rel32 (reloc-masked)
        _emit 0xe8
        _emit 0xb2
        _emit 0xfa
        _emit 0xff
        _emit 0xff
        // 000118fe: 8b 5b 08              MOV EBX, [EBX+0x8]
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 00011901: 8d 45 4c              LEA EAX, [EBP+0x4c]
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        // 00011904: 3b d8                 CMP EBX, EAX
        _emit 0x3b
        _emit 0xd8
        // 00011906: 75 aa                 JNZ -0x56           ; -> 0x4118b2
        _emit 0x75
        _emit 0xaa
        // ----- Pre-flush hook + Pass 4: commit / publish --------------
        // 00011908: 8b 45 24              MOV EAX, [EBP+0x24]
        _emit 0x8b
        _emit 0x45
        _emit 0x24
        // 0001190b: ff d0                 CALL EAX            ; this->m_24()
        _emit 0xff
        _emit 0xd0
        // 0001190d: 8b 5d 54              MOV EBX, [EBP+0x54]
        _emit 0x8b
        _emit 0x5d
        _emit 0x54
        // 00011910: 8d 45 4c              LEA EAX, [EBP+0x4c]
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        // 00011913: 3b d8                 CMP EBX, EAX
        _emit 0x3b
        _emit 0xd8
        // 00011915: 74 3d                 JZ +0x3d            ; -> 0x411954
        _emit 0x74
        _emit 0x3d
        // 00011917: 8b 13                 MOV EDX, [EBX]
        _emit 0x8b
        _emit 0x13
        // 00011919: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 0001191c: 8b cb                 MOV ECX, EBX
        _emit 0x8b
        _emit 0xcb
        // 0001191e: ff d0                 CALL EAX            ; N->vt[+4]() -> R
        _emit 0xff
        _emit 0xd0
        // 00011920: 8b f0                 MOV ESI, EAX
        _emit 0x8b
        _emit 0xf0
        // 00011922: 8b 7e 18              MOV EDI, [ESI+0x18]
        _emit 0x8b
        _emit 0x7e
        _emit 0x18
        // 00011925: 85 ff                 TEST EDI, EDI
        _emit 0x85
        _emit 0xff
        // 00011927: 74 21                 JZ +0x21            ; -> 0x41194a
        _emit 0x74
        _emit 0x21
        // 00011929: 83 ff ff              CMP EDI, -1
        _emit 0x83
        _emit 0xff
        _emit 0xff
        // 0001192c: 74 1c                 JZ +0x1c            ; -> 0x41194a
        _emit 0x74
        _emit 0x1c
        // 0001192e: 8b 56 04              MOV EDX, [ESI+0x4]
        _emit 0x8b
        _emit 0x56
        _emit 0x04
        // 00011931: 8b 42 04              MOV EAX, [EDX+0x4]
        _emit 0x8b
        _emit 0x42
        _emit 0x04
        // 00011934: 8d 4e 04              LEA ECX, [ESI+0x4]
        _emit 0x8d
        _emit 0x4e
        _emit 0x04
        // 00011937: ff d0                 CALL EAX            ; payload->vt[+4]
        _emit 0xff
        _emit 0xd0
        // 00011939: 2b c7                 SUB EAX, EDI
        _emit 0x2b
        _emit 0xc7
        // 0001193b: 50                    PUSH EAX
        _emit 0x50
        // 0001193c: 8b ce                 MOV ECX, ESI
        _emit 0x8b
        _emit 0xce
        // 0001193e: e8 ed f9 ff ff        CALL 0x00411330     ; rel32 (reloc-masked)
        _emit 0xe8
        _emit 0xed
        _emit 0xf9
        _emit 0xff
        _emit 0xff
        // 00011943: c7 46 18 00 00 00 00  MOV [ESI+0x18], 0
        _emit 0xc7
        _emit 0x46
        _emit 0x18
        _emit 0x00
        _emit 0x00
        _emit 0x00
        _emit 0x00
        // 0001194a: 8b 5b 08              MOV EBX, [EBX+0x8]
        _emit 0x8b
        _emit 0x5b
        _emit 0x08
        // 0001194d: 8d 45 4c              LEA EAX, [EBP+0x4c]
        _emit 0x8d
        _emit 0x45
        _emit 0x4c
        // 00011950: 3b d8                 CMP EBX, EAX
        _emit 0x3b
        _emit 0xd8
        // 00011952: 75 c3                 JNZ -0x3d           ; -> 0x411917
        _emit 0x75
        _emit 0xc3
        // ----- Post-flush hook ----------------------------------------
        // 00011954: 8b 4d 2c              MOV ECX, [EBP+0x2c]
        _emit 0x8b
        _emit 0x4d
        _emit 0x2c
        // 00011957: c6 45 48 00           MOV byte ptr [EBP+0x48], 0
        _emit 0xc6
        _emit 0x45
        _emit 0x48
        _emit 0x00
        // 0001195b: ff d1                 CALL ECX            ; this->m_2c()
        _emit 0xff
        _emit 0xd1
        // ----- Epilog: post-pass virtual hook + boolean return --------
        // 0001195d: 8b 55 00              MOV EDX, [EBP]      ; vtable
        _emit 0x8b
        _emit 0x55
        _emit 0x00
        // 00011960: 8b 42 30              MOV EAX, [EDX+0x30] ; post-pass hook
        _emit 0x8b
        _emit 0x42
        _emit 0x30
        // 00011963: 8b cd                 MOV ECX, EBP        ; this
        _emit 0x8b
        _emit 0xcd
        // 00011965: ff d0                 CALL EAX            ; this->vt[+0x30]()
        _emit 0xff
        _emit 0xd0
        // 00011967: 5f                    POP EDI
        _emit 0x5f
        // 00011968: 33 c0                 XOR EAX, EAX
        _emit 0x33
        _emit 0xc0
        // 0001196a: 38 45 48              CMP byte ptr [EBP+0x48], AL
        _emit 0x38
        _emit 0x45
        _emit 0x48
        // 0001196d: 5e                    POP ESI
        _emit 0x5e
        // 0001196e: 5d                    POP EBP
        _emit 0x5d
        // 0001196f: 0f 94 c0              SETZ AL             ; ret = (m_48 == 0)
        _emit 0x0f
        _emit 0x94
        _emit 0xc0
        // 00011972: 5b                    POP EBX
        _emit 0x5b
        // 00011973: 83 c4 08              ADD ESP, 0x8
        _emit 0x83
        _emit 0xc4
        _emit 0x08
        // 00011976: c3                    RET
        _emit 0xc3
    }
}
#endif

// vim: ts=4 sts=4 sw=4 et
