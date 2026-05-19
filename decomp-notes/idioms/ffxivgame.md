# ffxivgame.exe — MSVC 2005 idioms

Curator-maintained. Add new entries here when a pattern reappears across
multiple match attempts. Agents read this file but never modify it.

## Toolchain

- MSVC 2005 SP1 (linker version 8.0)
- x87 FP, NOT SSE2 (`/arch:` defaults)
- `/GS` enabled (security cookies on functions with local arrays ≥5 bytes)
- `/GF` enabled (string pooling — duplicate string literals coalesce)

## Patterns observed so far

(empty — populate as patterns emerge from the blocked/ post-mortems)
