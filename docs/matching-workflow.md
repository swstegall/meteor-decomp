# Per-function matching workflow

This is the loop a contributor (human or agent) runs when picking up
one row of `config/ffxivgame.yaml`.

## 0. Prerequisites

- `make split` has run (asm + symbol map generated, Phase 1).
- `tools/cl-wine.sh` is configured and the Rosetta Stone function
  matches (Phase 2).
- `objdiff` is installed and configured.

## 1. Claim the function

`config/ffxivgame.yaml` rows look like:

```yaml
- rva: 0x004a1230
  end: 0x004a12a0
  size: 0x70
  module: net/blowfish
  symbol: Blowfish::Init
  type: matching                # matching | functional | middleware-crt | ...
  status: unmatched              # unmatched | wip | matched | functional
  owner: null
```

Set `status: wip` and `owner: <your-handle>` and commit before
starting (one-line commit, no source changes — this is the "claim"
that prevents two contributors picking the same function).

## 2. Read the disassembly

`asm/ffxivgame/004a1230_Blowfish__Init.s` is the per-function dump
written by `tools/ghidra_scripts/dump_functions.py`. It's already
RVA-rebased and labeled with whatever symbols the seed sources have
provided.

Quick reads:
- Calling convention: `__cdecl` (caller cleans), `__stdcall` (callee
  cleans, `ret 0xN`), `__fastcall` (ecx/edx pre-loaded), or member
  function `__thiscall` (`this` in ecx).
- Local stack frame size: from the `sub esp, 0xN` in the prologue.
- Return type guess: from how `eax` is used at `ret`.

## 3. Pull Ghidra's pseudo-C

Open the Ghidra project (`build/ghidra/ffxivgame.gpr`) and copy the
function's decompiled view into a scratch buffer. Ghidra's output is
a strong starting point but never byte-correct; you rewrite it.

## 4. Write the C/C++

`src/ffxivgame/<module>/<symbol>.cpp` — one function per file in the
early days for clean per-function diffing. Add the AGPL header. Add
`#include`s. Replace `iVar1` / `local_4` with meaningful names.
Replace integer constants with named enums where possible (e.g. opcodes
defined in `include/net/opcodes.h`).

For matching modules, pick the simplest plausible C — extra `if (x)
{ y; }` vs `if (x) y;` can affect codegen, but more importantly the
*structure* (loop type, guard pattern, return-style) maps to specific
codegen idioms. When in doubt, mirror the Ghidra structure
literally; refactor for readability *after* it matches.

## 5. Build the function

```sh
make src/ffxivgame/net/blowfish/Init.obj
```

The Makefile invokes `tools/cl-wine.sh` with the locked
`MSVC_FLAGS=` and produces a single `.obj` per `.cpp`.

## 6. Diff

For matching functions:

```sh
make diff FUNC=Blowfish::Init
```

`tools/compare.py` invokes `objdiff` with the original `.text`
slice for that RVA range against the new `.obj`. Output: per-line
diff + a one-line OK/PARTIAL/MISMATCH verdict.

For functional functions:

```sh
make test FUNC=Blowfish::Init
```

Runs `tests/net/blowfish/init_test.cpp` (a small main that loads a
known input and asserts the output bytes). The behavioural fixture
either comes from a packet capture (`captures/`) or is hand-written
from a Project Meteor reference.

## 7. Iterate

If matching fails, the canonical bag of tricks (in rough order of
how often they're the cause):

| Symptom                                   | Fix                                                   |
|-------------------------------------------|-------------------------------------------------------|
| Wrong register allocation                 | Reorder local declarations; MSVC allocates in source order. |
| Off-by-one stack frame                    | Add a dead local of the right type; sometimes a temp the optimiser leaves materialised. |
| Branch direction flipped                  | Negate the condition: `if (x) A; else B;` ↔ `if (!x) B; else A;`. MSVC emits the first arm's branch unconditionally and the second arm with a forward jump. |
| Missing `__stdcall` / `__cdecl` mismatch  | Check the calling convention against the prologue's `ret N`. |
| Member fn looks `__cdecl`                 | Should be `__thiscall`. Use a class member declaration. |
| FP code mismatched                        | MSVC 2005 uses x87, not SSE2 by default. Don't `/arch:SSE2`. |
| `if (a && b)` vs `if (a) if (b)`          | Both are valid; MSVC's order-of-evaluation lowering can pick either. Try the alternative. |
| `for` vs `while`                          | Same loop body, different prologue. Try both. |
| Switch jump table                         | MSVC builds a jump table at `>=4` cases, dense by default. Add cases / reorder until the table layout matches. |
| String literal positions                  | If `.rdata` strings are coming out at different offsets, pool them with `__declspec(selectany)` or check `/GF` (string pooling). |
| `__security_cookie` dropouts              | Function had `/GS` enabled but you didn't add a buffer big enough. Add a `char buf[5]` local; `/GS` triggers cookies for any local array of size 5+ bytes. |
| Tail call missing                         | MSVC 2005 doesn't tail-call by default; use `__forceinline` on the callee or `if(...) return f();` form. |

## 8. Commit

One commit per function in the early phases. Subject:

```
decomp: match Blowfish::Init @0x004a1230
```

Body: brief notes on which compiler flag combo / refactor was
needed, any unusual idioms (helps the next contributor recognise the
same shape).

For functional decomps:

```
decomp: functional ComputeDamage @0x008c5a40

Behavioural fixture: tests/battle/compute_damage_test.cpp asserts
against three damage samples drawn from ffxiv_youtube_atlas_context.md
(Plumage 47-51, Cure +210-+240, Chaos Thrust 89-104).
```

## 9. Update YAML and ship

Update `config/ffxivgame.yaml` row:
- `status: matched` (or `functional`).
- `owner: null` (releases the claim).
- Optionally add `notes:` with one-line rationale.

`tools/progress.py` re-runs and updates the project's headline number
in `README.md` (matched / total).
