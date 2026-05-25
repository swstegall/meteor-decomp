#!/usr/bin/env bash
# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later
#
# On-demand struct/vtable analysis of OUR OWN ffxivgame.exe via the
# FFXIVLegacyClientStructs CLI (github.com/Yokimitsuro/FFXIVLegacyClientStructs,
# MIT). The CLI is third-party tooling; running it against our own
# legitimately-installed binary (orig/ffxivgame.exe) yields facts from
# *our* binary — the provenance-clean way to recover a struct layout the
# committed FFXIVLegacyClientStructs *.cs left as a shell.
#
# Builds the CLI on first use (net8.0 target; we roll forward to the
# installed net10 runtime). Usage:
#
#   tools/analyze_legacy_struct.sh --analyze CharaActor
#   tools/analyze_legacy_struct.sh --vtfuncs Client::Control::PlayerBase
#   tools/analyze_legacy_struct.sh --hierarchy GameManagerActor
#   tools/analyze_legacy_struct.sh --search Director
#
# NOTE on yield: the --analyze command walks a *standard* constructor to
# recover field offsets. Classes built by template instantiation (e.g.
# the GAM Network::*::Data::* CompileTimeParameterCollection family) or
# by non-standard init (much of the Lua Control hierarchy:
# NpcBase/CharaBase/MyPlayer/DirectorBase) report "Constructor not found".
# Use --vtable / --vtfuncs / --hierarchy for those instead.
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LCS_ROOT="${LCS_ROOT:-$REPO_ROOT/../FFXIVLegacyClientStructs}"
EXE="${EXE:-$REPO_ROOT/orig/ffxivgame.exe}"
DLL="$LCS_ROOT/FFXIVClientStructs.Tools.CLI/bin/Release/net8.0/FFXIVClientStructs.Tools.CLI.dll"

if [ ! -e "$EXE" ]; then
  echo "ERROR: binary not found at $EXE (symlink orig/ffxivgame.exe missing?)" >&2
  exit 1
fi
if [ ! -f "$DLL" ]; then
  echo "Building FFXIVLegacyClientStructs CLI (first run)..." >&2
  dotnet build "$LCS_ROOT/FFXIVClientStructs.Tools.CLI/FFXIVClientStructs.Tools.CLI.csproj" \
    -c Release >&2
fi

DOTNET_ROLL_FORWARD=Major exec dotnet "$DLL" --exe "$EXE" "$@"
