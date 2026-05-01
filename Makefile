# meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
# Copyright (C) 2026  Samuel Stegall
# SPDX-License-Identifier: AGPL-3.0-or-later

PY      ?= python3
ORIG    := orig
BUILD   := build
TOOLS   := tools

# --- Phase 0 (works today) ---------------------------------------------

.PHONY: help bootstrap pe-info clean

help:
	@echo "meteor-decomp — top-level targets"
	@echo "  make bootstrap     symlink orig/ + dump PE structure (Phase 0)"
	@echo "  make pe-info       re-run tools/extract_pe.py (Phase 0)"
	@echo "  make split         Phase 1: Ghidra split + work-pool YAML (TODO)"
	@echo "  make rosetta       Phase 2: build the Rosetta-Stone function (TODO)"
	@echo "  make diff FUNC=X   Phase 2: objdiff-cli on one matched function (TODO)"
	@echo "  make progress      print matched/total per binary (TODO)"
	@echo "  make clean         wipe build/"

bootstrap:
	@$(TOOLS)/symlink_orig.sh
	@$(PY) $(TOOLS)/extract_pe.py

pe-info:
	@$(PY) $(TOOLS)/extract_pe.py

clean:
	rm -rf $(BUILD)

# --- Phase 1 (TODO once Ghidra is wired) -------------------------------

.PHONY: split

split:
	@if [ -z "$$GHIDRA_HOME" ]; then \
	    echo "error: \$$GHIDRA_HOME not set; run tools/setup.sh --with-ghidra"; \
	    exit 1; \
	fi
	@for exe in $(ORIG)/*.exe; do \
	    bn=$$(basename $$exe); \
	    echo ">>> ghidra import $$bn"; \
	    $(PY) $(TOOLS)/import_to_ghidra.py $$bn; \
	done
	@$(PY) $(TOOLS)/build_split_yaml.py

# --- Phase 2 (TODO once MSVC is wired) ---------------------------------

.PHONY: rosetta diff progress

rosetta:
	@echo "rosetta: not yet wired — see PLAN.md §6 Phase 2 + docs/compiler-detection.md"
	@false

diff:
	@if [ -z "$(FUNC)" ]; then echo "usage: make diff FUNC=Symbol::Name"; exit 1; fi
	@$(PY) $(TOOLS)/compare.py FUNC=$(FUNC)

progress:
	@$(PY) $(TOOLS)/progress.py
