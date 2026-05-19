# decomp-notes/

Project-local memory layer for autonomous matching agents and humans.
Knowledge accumulates here across sessions — the next agent that touches
a function gets to read what previous attempts learned.

## Layout

```
decomp-notes/
├── README.md                            (this file)
├── idioms/<binary>.md                   (CURATOR-written; agent READ-only)
├── blocked/<binary>/<rva>_<symbol>.md   (agent-APPEND-only; one file per blocked claim)
└── types/<binary>/<rva>.md              (agent-WRITE on discovery; one file per discovered type)
```

## idioms/  — curated patterns

`idioms/<binary>.md` captures patterns specific to one binary's codegen —
e.g. "ffxivgame uses `/GS` extensively, so any function with a local
array ≥5 bytes needs a `__security_cookie` prologue" or "net/blowfish
functions all use `__fastcall` with the s-box passed in ecx."

**Curator-only.** Agents must read these before starting a function but
must not modify them. This keeps low-confidence agent guesses out of
the binary-level guidance that drives every match attempt.

## blocked/  — per-function post-mortems

When an agent bails on a function (`outcome=blocked`), the orchestrator
appends a post-mortem to `blocked/<binary>/<rva>_<symbol>.md` describing
what was tried and what the diff looked like. The NEXT agent that claims
the same RVA reads this first so it doesn't re-try the same dead ends.

**Format** (one block per attempt, append-only):

```markdown
## Attempt 2026-05-18T19:42:00Z by agent-2

**Iterations**: 7

**Tried** (in order):
- branch flip on inner loop bound
- register reorder (moved `n` declaration above `buf`)
- swap `__cdecl` → `__thiscall`

**Diff snapshot**: PARTIAL — 42 of 70 bytes match. First mismatch at
offset 0x18: orig `8B 45 0C` (mov eax, [ebp+0xc]); ours `8B 75 0C` (mov esi, [ebp+0xc]).

**Suspect**: parameter order in source declaration is wrong; MSVC is
binding the second arg into esi instead of eax. Try declaring `len`
before `buf`.

**Bail reason**: out of iteration budget; haven't tried the parameter
reorder yet.
```

These files are checked in. Reviewers can spot recurring stumbling
blocks (e.g. "every fn in this module bails on a switch jump table")
and promote them into `idioms/<binary>.md`.

## types/  — type / class / vtable discoveries

When an agent identifies a class, struct, enum, or vtable membership
during a match, it writes a structured note to `types/<binary>/<rva>.md`.
One file per discovery. These accumulate into a type catalog that future
matches can grep before reinventing names.

**Format**:

```markdown
# CResourceException

**Discovered**: 2026-05-18 by agent-3 while matching FUN_004A1230
**Vtable**: 0x00501234 (sibling RVAs: ctor=0x004A1230, dtor=0x004A1280, what=0x004A12A0)
**Fields** (inferred from offsets):
- +0x00: vtable ptr
- +0x04: int code
- +0x08: char message[64]

**Sources**: config/ffxivgame.rtti.json line 12, sibling FUN_004A12A0 disasm.
```

These files are checked in. Curator periodically rolls discoveries up
into the relevant `idioms/<binary>.md` once they've been validated by
multiple matches.

## What NOT to write here

- Free-form blog posts about what the agent thinks (use the SQLite
  tool-events log if you want a turn-by-turn record).
- Stuff the build system already encodes (RVA, size, section — those
  live in `config/<binary>.yaml` and `config/<binary>.symbols.json`).
- Memory-aid notes scoped to a single session (use the SQLite DB).
