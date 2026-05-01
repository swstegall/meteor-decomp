# meteor-decomp

Decompilation of the FINAL FANTASY XIV 1.23b Windows client binaries
(`ffxivgame.exe`, `ffxivboot.exe`, `ffxivlogin.exe`,
`ffxivupdater.exe`, `ffxivconfig.exe`).

See **[PLAN.md](PLAN.md)** for the full strategy, scope, and roadmap.

## Quickstart

```sh
# 0. Symlink original binaries from the workspace install (does NOT copy)
#    + dump PE structure (sanity check):
make bootstrap

# 1. Static-analysis pipeline. Requires Ghidra 12 + JDK 21
#    (`brew install ghidra` pulls openjdk@21).
make split BINARY=ffxivlogin.exe   # ~30s — sanity check
make split BINARY=ffxivgame.exe    # ~30-60 min on Apple Silicon

# 2. Inspect the work pool:
make progress
```

After `make split`:
- `asm/<binary>/<rva>_<symbol>.s` — one file per function
- `config/<binary>.symbols.json` — function list with sizes / sections
- `config/<binary>.strings.json` — strings + seed-hint flags (`__FILE__`,
  `__FUNCTION__`, Lua callbacks)
- `config/<binary>.rtti.json` — recovered class names + vtable RVAs
- `config/<binary>.yaml` — work pool, one row per function

## Why "meteor-decomp"?

Project Meteor is the long-running effort to revive FFXIV 1.x — the
C# server (`project-meteor-server`), the launchers, the dataminers.
This subproject is the missing piece: a first-party reading of the
client itself, so the rest of the workspace stops reverse-engineering
through capture-and-guess and starts working from the source.

## Original binaries are NOT in this repo

Square-Enix-copyright `.exe` files belong only in
`ffxiv-install-environment/target/prefix/.../FINAL FANTASY XIV/`.
`tools/symlink_orig.sh` makes them visible under `orig/` for the
build pipeline. Never `git add` them.

## License

AGPL-3.0-or-later (matches `garlemald-server` / `garlemald-client`).
See [LICENSE.md](LICENSE.md) and [NOTICE.md](NOTICE.md).
