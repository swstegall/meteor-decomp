# meteor-decomp

Decompilation of the FINAL FANTASY XIV 1.23b Windows client binaries
(`ffxivgame.exe`, `ffxivboot.exe`, `ffxivlogin.exe`,
`ffxivupdater.exe`, `ffxivconfig.exe`).

See **[PLAN.md](PLAN.md)** for the full strategy, scope, and roadmap.

## Quickstart

```sh
# 1. Symlink original binaries from the workspace install (does NOT copy).
./tools/symlink_orig.sh

# 2. Dump PE structure for every binary; sanity check.
python3 tools/extract_pe.py

# 3. (Phase 1+) Run the static-analysis pipeline. Requires Ghidra 11.x.
make split
```

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
