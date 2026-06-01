#!/usr/bin/env bash
# Push local-ahead GREEN _rosetta matches to origin/develop.
#
# Local-mode runs commit each match straight onto develop without pushing; over
# a run that strands a backlog of unpushed commits, which then reads as
# "develop ahead of origin" divergence. This clears it SAFELY.
#
# For EVERY commit on develop not yet on origin/develop it requires:
#   * the commit's changes are limited to ONE added src/<bin>/_rosetta/FUN_<hex>.cpp
#     (a config/<bin>.yaml row edit alongside it is allowed; nothing else is)
#   * that function re-grades GREEN (byte-identical) via the local toolchain
# Only if ALL ahead commits pass does it drop byte-identical untracked _rosetta
# strays (already-on-origin dups), rebase develop onto origin/develop, and push.
# If ANY ahead commit fails the profile or the GREEN re-grade, it pushes NOTHING
# and names the offending commit — a non-match or broken commit can never
# auto-ship. It also refuses to run while a decomp-agents run is active, so it
# can never race a live committer.
#
# Exit: 0 = pushed or nothing to do; 1 = held back for a human / unsafe to run.
set -uo pipefail
export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin${PATH:+:$PATH}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT" || exit 1
say() { printf '%s\n' "$*"; }

# --- preconditions -----------------------------------------------------------
git rev-parse --git-dir >/dev/null 2>&1 || { say "not a git repo"; exit 1; }
git remote get-url origin >/dev/null 2>&1 || { say "no origin remote"; exit 1; }

# Never run while a decomp-agents run is active — rebasing develop under a live
# committer is a git race. Match the ORCHESTRATOR process specifically (the venv
# console script or `python -m decomp_agents`), NOT the decomp-agents directory
# name, so start.sh invoking us pre-run is not mistaken for a live run.
if pgrep -f 'bin/decomp-agents|-m +decomp_agents' >/dev/null 2>&1; then
  say "a decomp-agents run is active — skipping to avoid racing it"
  exit 1
fi

cur="$(git symbolic-ref --short -q HEAD || true)"
[ "$cur" = "develop" ] || { say "not on develop (on '${cur:-detached}'); aborting"; exit 1; }
if ! git diff --quiet || ! git diff --cached --quiet; then
  say "working tree has staged/unstaged changes; commit or stash first"; exit 1
fi

git fetch --quiet origin develop || { say "fetch failed"; exit 1; }

ahead="$(git rev-list --reverse origin/develop..develop)"
[ -n "$ahead" ] || { say "develop has no unpushed commits — nothing to do"; exit 0; }

# --- validate + GREEN-grade EVERY ahead commit before touching anything ------
stage="$REPO_ROOT/.push-matches-stage"
trap 'rm -rf "$stage"' EXIT
rm -rf "$stage"; mkdir -p "$stage"
bins=""   # space-delimited set of binary stems seen

for c in $ahead; do
  short="$(git rev-parse --short "$c")"
  added_cpp=""; bad=""
  # plumbing name-status of just this commit (A/M/D/R<n> + path)
  while IFS=$'\t' read -r st p _; do
    [ -z "${st:-}" ] && continue
    case "$st" in
      A)
        if printf '%s' "$p" | grep -qE '^src/[A-Za-z0-9_]+/_rosetta/FUN_[0-9a-fA-F]+\.cpp$'; then
          if [ -n "$added_cpp" ]; then bad="more than one added _rosetta file"; else added_cpp="$p"; fi
        else bad="adds non-_rosetta file: $p"; fi ;;
      M)
        printf '%s' "$p" | grep -qE '^config/[A-Za-z0-9_]+\.yaml$' || bad="modifies non-yaml file: $p" ;;
      *)
        bad="has a $st change: $p" ;;
    esac
    [ -n "$bad" ] && break
  done <<EOF
$(git diff-tree --no-commit-id --name-status -r "$c")
EOF

  [ -n "$bad" ] && { say "HELD (no push): commit $short — $bad"; say "Push/resolve that commit manually; nothing was pushed."; exit 1; }
  [ -n "$added_cpp" ] || { say "HELD (no push): commit $short adds no _rosetta match file"; exit 1; }

  bin="$(printf '%s' "$added_cpp" | sed -E 's#^src/([^/]+)/_rosetta/.*#\1#')"
  mkdir -p "$stage/$bin"
  git show "$c:$added_cpp" > "$stage/$bin/$(basename "$added_cpp")" 2>/dev/null \
    || { say "HELD: cannot read $added_cpp from $short"; exit 1; }
  case " $bins " in *" $bin "*) : ;; *) bins="$bins $bin" ;; esac
done

# GREEN-grade each binary's staged set (relative path keeps MSVC happy).
for bin in $bins; do
  rel=".push-matches-stage/$bin"
  staged="$(ls "$rel"/*.cpp 2>/dev/null | wc -l | tr -d ' ')"
  out="$(make rosetta-bulk BINARY="$bin.exe" ROSETTA_SRC_DIR="$rel" 2>&1)"
  summ="$(printf '%s' "$out" | grep -E 'rosetta-bulk\[' | tail -1)"
  green="$(printf '%s' "$summ" | grep -oE 'GREEN=[0-9]+' | cut -d= -f2)"
  if ! printf '%s' "$summ" | grep -qE 'PARTIAL=0  MISMATCH=0  cl_failed=0' || [ "${green:-0}" != "$staged" ]; then
    say "HELD (no push): $bin matches did not all re-grade GREEN ($staged staged):"
    say "  ${summ:-<no summary; compile failed>}"
    exit 1
  fi
  say "GREEN re-grade OK ($bin): $summ"
done
rm -rf "$stage"; trap - EXIT

# --- all verified GREEN → drop dup strays, rebase, push ----------------------
# A stray whose path is ALREADY on origin/develop (committed via a PR) is a
# transient duplicate — origin's version is authoritative — so drop it whether
# or not the bytes match (a differing untracked copy would otherwise block the
# rebase checkout, the #1 cause of a manual reconcile). A stray NOT on origin is
# potential unique local work: it can't block the rebase (origin doesn't add
# it), so leave it in place but surface it so it isn't silently lost.
unpushed_strays=""
while IFS= read -r f; do
  [ -z "$f" ] && continue
  printf '%s' "$f" | grep -qE '^src/[A-Za-z0-9_]+/_rosetta/FUN_[0-9a-fA-F]+\.cpp$' || continue
  if git cat-file -e "origin/develop:$f" 2>/dev/null; then
    rm -f "$f"; say "dropped dup stray $f (origin's committed version is authoritative)"
  else
    unpushed_strays="$unpushed_strays $f"
  fi
done < <(git ls-files --others --exclude-standard)
[ -n "$unpushed_strays" ] && say "note: untracked _rosetta file(s) NOT on origin — left in place; commit them if they are real matches:$unpushed_strays"

n="$(printf '%s\n' "$ahead" | grep -c .)"
for try in 1 2 3 4 5; do
  git fetch --quiet origin develop
  if ! git rebase origin/develop >/dev/null 2>&1; then
    git rebase --abort 2>/dev/null || true
    say "HELD: rebase onto origin/develop hit a conflict; resolve manually"
    exit 1
  fi
  if git push origin develop 2>&1 | tail -1; then
    say "pushed $n GREEN match commit(s) to origin/develop — develop in sync"
    exit 0
  fi
  say "origin advanced mid-push; retrying…"; sleep 2
done
say "HELD: could not push after retries (origin contention)"
exit 1
