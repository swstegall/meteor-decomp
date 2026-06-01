#!/usr/bin/env bash
# Safe, fast-forward-ONLY sync of local `develop` to origin/develop.
#
# It NEVER merges, rebases, resets, or force-updates. It refuses to act —
# loudly, in the log — whenever doing so could lose work:
#   * working tree has staged/unstaged changes -> skip
#   * local develop is ahead (unpushed commits) -> skip
#   * histories have diverged (non-fast-forward) -> skip
# The worst case is therefore "did nothing": it can never clobber local commits
# or uncommitted edits. Safe to run on a launchd timer AND from
# decomp-agents/start.sh; a mkdir lock serialises concurrent invocations.
#
# Exit status is always 0 — a no-op must not break a caller such as start.sh.

set -uo pipefail

# launchd hands processes a minimal PATH — make sure git resolves there.
export PATH="/opt/homebrew/bin:/usr/local/bin:/usr/bin:/bin:/usr/sbin:/sbin${PATH:+:$PATH}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
cd "$REPO_ROOT" 2>/dev/null || exit 0

LOG_DIR="$REPO_ROOT/build/logs"
mkdir -p "$LOG_DIR" 2>/dev/null || true
LOG="$LOG_DIR/sync-develop.log"
log() { printf '%s  %s\n' "$(date '+%Y-%m-%dT%H:%M:%S')" "$*" >> "$LOG" 2>/dev/null; }

# --- portable lock (macOS ships no flock): atomic mkdir, with staleness reap --
# NB: BSD `find -mmin` is unreliable for sub-file age tests, so staleness is
# measured with a stat-epoch delta. The lock records its owner PID so the EXIT
# trap only ever removes a lock THIS process still holds (a lock that was
# reaped-as-stale and re-taken by another run must not be deleted on our exit).
LOCKDIR="$LOG_DIR/.sync-develop.lock.d"
lock_mtime() { stat -f %m "$LOCKDIR" 2>/dev/null || stat -c %Y "$LOCKDIR" 2>/dev/null || echo 0; }
acquire()    { mkdir "$LOCKDIR" 2>/dev/null && printf '%s' "$$" > "$LOCKDIR/pid" 2>/dev/null; }

if ! acquire; then
  now="$(date +%s)"; mt="$(lock_mtime)"
  if [ -d "$LOCKDIR" ] && [ "${mt:-0}" -gt 0 ] && [ "$((now - mt))" -gt 1800 ]; then
    log "reaping a stale lock ($((now - mt))s old)"
    rm -rf "$LOCKDIR" 2>/dev/null || true
  fi
  if ! acquire; then
    log "another sync holds the lock; skipping"
    exit 0
  fi
fi
trap '[ "$(cat "$LOCKDIR/pid" 2>/dev/null)" = "$$" ] && rm -rf "$LOCKDIR" 2>/dev/null || true' EXIT

# --- preconditions -----------------------------------------------------------
git rev-parse --git-dir >/dev/null 2>&1 || { log "not a git repo; skip"; exit 0; }
git remote get-url origin >/dev/null 2>&1 || { log "no origin remote; skip"; exit 0; }
git show-ref --verify --quiet refs/heads/develop || { log "no local develop branch; skip"; exit 0; }

# Never touch a tree with staged/unstaged changes. (Untracked files are allowed;
# if an incoming commit would overwrite one, the ff-merge below fails safely.)
if ! git diff --quiet --ignore-submodules || ! git diff --cached --quiet --ignore-submodules; then
  log "working tree has uncommitted changes; skipping sync"
  exit 0
fi

git fetch --quiet origin develop 2>>"$LOG" || { log "git fetch failed; skip"; exit 0; }

# ahead = local-only commits, behind = remote-only commits.
counts="$(git rev-list --left-right --count develop...origin/develop 2>/dev/null || printf '0\t0')"
ahead="$(printf '%s' "$counts" | awk '{print $1+0}')"
behind="$(printf '%s' "$counts" | awk '{print $2+0}')"

if [ "${ahead:-0}" -gt 0 ]; then
  log "develop is ahead by $ahead unpushed commit(s); leaving it alone (push or PR them first)"
  exit 0
fi
if [ "${behind:-0}" -eq 0 ]; then
  log "develop already up to date"
  exit 0
fi

cur="$(git symbolic-ref --short -q HEAD || true)"
if [ "$cur" = "develop" ]; then
  if git merge --ff-only origin/develop >>"$LOG" 2>&1; then
    log "fast-forwarded develop by $behind commit(s) (checked out)"
  else
    log "ff-only merge refused (untracked-file collision or divergence); left untouched"
  fi
else
  # develop is not checked out: ff-update its ref directly. `fetch B:B` rejects
  # a non-fast-forward, so this can never rewrite local history.
  if git fetch --quiet origin develop:develop 2>>"$LOG"; then
    log "fast-forwarded develop ref by $behind commit(s) (currently on '${cur:-detached HEAD}')"
  else
    log "could not fast-forward develop ref (divergence?); left untouched"
  fi
fi
exit 0
