#!/usr/bin/env bash
# Install (or refresh) a macOS launchd timer that runs tools/sync-develop.sh
# every SYNC_INTERVAL_SECONDS (default 600s) so local `develop` stays
# fast-forwarded to origin without you remembering to pull. Idempotent — safe to
# re-run. The generated plist holds machine-specific absolute paths and lives in
# ~/Library/LaunchAgents (never committed); only this generator is tracked.
#
# macOS only. On Linux, add `tools/sync-develop.sh` to cron instead, e.g.:
#   */10 * * * * /abs/path/to/meteor-decomp/tools/sync-develop.sh
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
SYNC="$REPO_ROOT/tools/sync-develop.sh"
LABEL="com.swstegall.meteor-decomp.sync-develop"
PLIST="$HOME/Library/LaunchAgents/$LABEL.plist"
INTERVAL="${SYNC_INTERVAL_SECONDS:-600}"

if [ "$(uname)" != "Darwin" ]; then
  echo "Not macOS — add this to cron instead:"
  echo "  */10 * * * * $SYNC"
  exit 0
fi

[ -f "$SYNC" ] || { echo "error: $SYNC not found" >&2; exit 1; }
chmod +x "$SYNC"
mkdir -p "$HOME/Library/LaunchAgents" "$REPO_ROOT/build/logs"

cat > "$PLIST" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key><string>$LABEL</string>
  <key>ProgramArguments</key>
  <array>
    <string>/bin/bash</string>
    <string>$SYNC</string>
  </array>
  <key>WorkingDirectory</key><string>$REPO_ROOT</string>
  <key>StartInterval</key><integer>$INTERVAL</integer>
  <key>RunAtLoad</key><true/>
  <key>StandardOutPath</key><string>$REPO_ROOT/build/logs/sync-develop.launchd.out</string>
  <key>StandardErrorPath</key><string>$REPO_ROOT/build/logs/sync-develop.launchd.err</string>
  <key>ProcessType</key><string>Background</string>
</dict>
</plist>
PLIST

# Reload (unload first so a changed interval/path takes effect).
launchctl unload "$PLIST" 2>/dev/null || true
launchctl load -w "$PLIST"

echo "Loaded $LABEL — runs every ${INTERVAL}s."
echo "  sync log:    $REPO_ROOT/build/logs/sync-develop.log"
echo "  launchd err: $REPO_ROOT/build/logs/sync-develop.launchd.err"
echo "Disable with:  launchctl unload '$PLIST' && rm '$PLIST'"
