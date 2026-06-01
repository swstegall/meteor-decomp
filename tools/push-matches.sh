#!/usr/bin/env bash
# Push local GREEN _rosetta matches to origin/develop.
#
# Local-mode runs commit matches onto develop without pushing (and sometimes
# leave a match as an UNTRACKED file that was never committed at all); over a run
# this strands work that reads as "develop ahead of origin" divergence. This
# clears it SAFELY and hands-off:
#
#   1. UNTRACKED _rosetta strays are resolved first:
#        * already on origin/develop (committed via a PR) -> dropped, whatever the
#          bytes (origin's merged version is authoritative; a differing copy would
#          otherwise block the rebase — the #1 cause of a manual reconcile)
#        * NOT on origin + re-grades GREEN + has the SPDX header -> ADOPTED:
#          git add + commit as a normal match, so it gets pushed
#        * NOT on origin + not GREEN / no header -> left in place, surfaced
#   2. Every COMMIT on develop not yet on origin must be a single added
#      src/<bin>/_rosetta/FUN_<hex>.cpp (a config/<bin>.yaml row edit allowed) AND
#      re-grade GREEN. If ALL pass, develop is rebased onto origin and pushed.
#      If ANY fails the profile or the GREEN re-grade, NOTHING is pushed and the
#      offending commit is named — a non-match or broken commit can never ship.
#
# Refuses to run while a decomp-agents run is active (no git race) or on a dirty
# tracked tree. Exit: 0 = pushed or nothing to do; 1 = held back / unsafe to run.
set -uo pipefail
export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin${PATH:+:$PATH}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT" || exit 1
say() { printf '%s\n' "$*"; }

stage="$REPO_ROOT/.push-matches-stage"
LAST_SUMM=""

# image base (decimal) for a binary stem — from build/pe-layout/<bin>.json,
# defaulting to 0x400000. Only feeds the cosmetic @0x<rva> commit subject.
image_base_of() {
  local bin="$1" f="$REPO_ROOT/build/pe-layout/$1.json" b
  if [ -f "$f" ]; then
    b="$(grep -oE '"image_base"[^,}]*' "$f" | grep -oE '0x[0-9a-fA-F]+' | head -1)"
    [ -n "$b" ] && { printf '%s' "$((b))"; return; }
  fi
  printf '%s' "$((0x400000))"
}

# path -> "decomp: match FUN_<va> @0x<rva8>"  (rva = va - image_base)
commit_subject() {
  local f="$1" sym bin va ib
  sym="$(basename "$f" .cpp)"
  bin="$(printf '%s' "$f" | sed -E 's#^src/([^/]+)/_rosetta/.*#\1#')"
  va=$((16#${sym#FUN_}))
  ib="$(image_base_of "$bin")"
  printf 'decomp: match %s @0x%08x' "$sym" "$((va - ib))"
}

# bin, relative-stage-dir -> 0 iff every staged .cpp re-grades GREEN.
# Sets LAST_SUMM to the rosetta-bulk summary line.
green_grade() {
  local bin="$1" rel="$2" staged out summ green
  staged="$(ls "$rel"/*.cpp 2>/dev/null | wc -l | tr -d ' ')"
  [ "${staged:-0}" -gt 0 ] || { LAST_SUMM="<nothing staged>"; return 1; }
  out="$(make rosetta-bulk BINARY="$bin.exe" ROSETTA_SRC_DIR="$rel" 2>&1)"
  summ="$(printf '%s' "$out" | grep -E 'rosetta-bulk\[' | tail -1)"
  green="$(printf '%s' "$summ" | grep -oE 'GREEN=[0-9]+' | cut -d= -f2)"
  LAST_SUMM="${summ:-<no summary; compile failed>}"
  printf '%s' "$summ" | grep -qE 'PARTIAL=0  MISMATCH=0  cl_failed=0' && [ "${green:-0}" = "$staged" ]
}

# --- preconditions -----------------------------------------------------------
git rev-parse --git-dir >/dev/null 2>&1 || { say "not a git repo"; exit 1; }
git remote get-url origin >/dev/null 2>&1 || { say "no origin remote"; exit 1; }

# Never run while a decomp-agents run is active — rebasing develop under a live
# committer is a git race. Match the ORCHESTRATOR process specifically, NOT the
# decomp-agents directory name, so start.sh invoking us pre-run is not mistaken
# for a live run.
if pgrep -f 'bin/decomp-agents|-m +decomp_agents' >/dev/null 2>&1; then
  say "a decomp-agents run is active — skipping to avoid racing it"; exit 1
fi

cur="$(git symbolic-ref --short -q HEAD || true)"
[ "$cur" = "develop" ] || { say "not on develop (on '${cur:-detached}'); aborting"; exit 1; }
if ! git diff --quiet || ! git diff --cached --quiet; then
  say "working tree has staged/unstaged changes; commit or stash first"; exit 1
fi

git fetch --quiet origin develop || { say "fetch failed"; exit 1; }
trap 'rm -rf "$stage"' EXIT

# --- Phase 0: resolve untracked _rosetta strays ------------------------------
while IFS= read -r f; do
  [ -z "$f" ] && continue
  printf '%s' "$f" | grep -qE '^src/[A-Za-z0-9_]+/_rosetta/FUN_[0-9a-fA-F]+\.cpp$' || continue

  if git cat-file -e "origin/develop:$f" 2>/dev/null; then
    rm -f "$f"; say "dropped dup stray $f (origin's committed version is authoritative)"
    continue
  fi
  # Not on origin — candidate unique local match. Require the SPDX header so we
  # never commit a license-noncompliant file.
  if ! grep -qE 'SPDX-License-Identifier' "$f"; then
    say "left $f (untracked, missing SPDX header — commit manually if it is a real match)"
    continue
  fi
  bin="$(printf '%s' "$f" | sed -E 's#^src/([^/]+)/_rosetta/.*#\1#')"
  rm -rf "$stage"; mkdir -p "$stage/$bin"; cp "$f" "$stage/$bin/"
  if green_grade "$bin" ".push-matches-stage/$bin"; then
    git add "$f"
    git commit -q -m "$(commit_subject "$f")"
    say "adopted GREEN untracked match: $(commit_subject "$f" | sed 's/^decomp: match //')"
  else
    say "left $f (untracked, did NOT re-grade GREEN — needs manual review): $LAST_SUMM"
  fi
done < <(git ls-files --others --exclude-standard)
rm -rf "$stage"

# --- Phase 1: validate + GREEN-grade every ahead commit ----------------------
ahead="$(git rev-list --reverse origin/develop..develop)"
[ -n "$ahead" ] || { say "develop has no unpushed commits — nothing to do"; exit 0; }

mkdir -p "$stage"
bins=""
for c in $ahead; do
  short="$(git rev-parse --short "$c")"
  added_cpp=""; bad=""
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

for bin in $bins; do
  if green_grade "$bin" ".push-matches-stage/$bin"; then
    say "GREEN re-grade OK ($bin): $LAST_SUMM"
  else
    say "HELD (no push): $bin matches did not all re-grade GREEN:"
    say "  $LAST_SUMM"
    exit 1
  fi
done
rm -rf "$stage"; trap - EXIT

# --- Phase 2: rebase + push (strays already resolved in Phase 0) --------------
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
