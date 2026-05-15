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
	@echo "  make bootstrap            symlink orig/ + dump PE structure (Phase 0)"
	@echo "  make pe-info              re-run tools/extract_pe.py (Phase 0)"
	@echo "  make split BINARY=X.exe   Phase 1: Ghidra import + dump + work-pool YAML"
	@echo "  make split-all            Phase 1: split every binary in orig/"
	@echo "  make setup-msvc           Phase 2: verify VS 2005 SP1 + Wine setup"
	@echo "  make find-rosetta         Phase 2: pick best Rosetta-Stone candidate"
	@echo "  make rosetta              Phase 2: compile + diff staged Rosetta function"
	@echo "  make rosetta-bulk         Phase 2: like rosetta, but never bails — for stamped clusters"
	@echo "  make emit-passthrough     Phase 2.6: byte-passthrough .cpp for unmatched fns (BINARY=, FUNC=, MAX=)"
	@echo "  make compile-passthrough  Phase 2.6: compile every src/<bin>/_passthrough/*.cpp (BINARY=)"
	@echo "  make mark-passthrough     Phase 2.6: flip YAML status to passthrough for GREEN .objs (BINARY=)"
	@echo "  make mark-passthrough-all Phase 2.6: mark-passthrough across all five binaries"
	@echo "  make passthrough          Phase 2.6: emit + compile + mark (universal byte-fallback)"
	@echo "  make emit-text-gaps       Phase 2.6: fill inter-function .text gap bytes (BINARY=)"
	@echo "  make emit-data-sections   Phase 2.6: byte-blob .cpp per non-code section (BINARY=)"
	@echo "  make recompile-coverage   Phase 2.6: emit + gaps + data-sections + compile + mark"
	@echo "  make emit-text-blob       Phase 2.7: orig .text as one naked-asm function (BINARY=)"
	@echo "  make link                 Phase 2.7: link.exe + postlink patcher (BINARY=)"
	@echo "  make relink               Phase 2.7: emit-text-blob + emit-data-sections + compile + link"
	@echo "  make diff-pe              Phase 2.7: byte-level diff vs orig PE (BINARY=)"
	@echo "  make swap-rosetta         Phase 2.8: splice _rosetta/<sym>.cpp into relink (BINARY=, FUNC=)"
	@echo "  make list-swaps           Phase 2.8: list active rosetta swaps (BINARY=)"
	@echo "  make swap-source-file     Phase 2.9: splice multi-fn hand-written .cpp into relink (BINARY=, SRC=)"
	@echo "  make swap-source-all      Phase 2.9: splice every crt/sqex/sqpack/install .cpp (BINARY=)"
	@echo "  make list-source-swaps    Phase 2.9: list active source-file swaps (BINARY=)"
	@echo "  make extract-net          Phase 3: net-class vtable → fn_rva map"
	@echo "  make extract-gam          Phase 3: GAM property registry (id → type)"
	@echo "  make emit-gam-header      Phase 3: include/net/gam_registry.h from GAM"
	@echo "  make extract-paramnames   Phase 3: dereference per-class PARAMNAME dispatchers"
	@echo "  make extract-gam-types-rtti Phase 3: enrich gam_params.json with RTTI types"
	@echo "  make validate-chara-make  Phase 3: garlemald parse_new_char_request ↔ GAM CharaMakeData"
	@echo "  make validate-chara-list  Phase 3: garlemald build_for_chara_list ↔ GAM ClientSelectData"
	@echo "  make validate-murmur2     Phase 3: MurmurHash2 vectors (FUN_00d31490 ↔ garlemald)"
	@echo "  make extract-opcodes      Phase 3: opcode → vtable-slot map (zone/lobby/chat Down)"
	@echo "  make extract-up-opcodes   Phase 3: Up-direction CPB ctor inventory + RX-opcode validation"
	@echo "  make extract-crypt-engine Phase 3: decode LobbyCryptEngine 9 slots + validate Blowfish init"
	@echo "  make decode-lpb           Phase 6: decode shipped client/script/*.le.lpb wrapper"
	@echo "  make decompile-lpb        Phase 6: unluac-decompile build/lpb/*.luac → build/lua/*.lua"
	@echo "  make extract-cpp-bindings Phase 6: enumerate engine C++-bound Lua API from _u files"
	@echo "  make garlemald-lua-coverage Phase 6: garlemald userdata.rs ↔ scripts/lua/ coverage report"
	@echo "  make extract-work-fields  Phase 6: enumerate per-class state-field inventory (playerWork etc.)"
	@echo "  make work-field-coverage  Phase 6: garlemald SetActorProperty paths ↔ inventory coverage"
	@echo "  make lpb-corpus           Phase 6: decode-lpb + decompile-lpb + extract-cpp-bindings"
	@echo "  make diff FUNC=X          objdiff-cli on one matched function"
	@echo "  make progress             print matched/total across all *.yaml"
	@echo "  make clean                wipe build/"

bootstrap:
	@$(TOOLS)/symlink_orig.sh
	@$(PY) $(TOOLS)/extract_pe.py

pe-info:
	@$(PY) $(TOOLS)/extract_pe.py

clean:
	rm -rf $(BUILD)

# --- Phase 1 (TODO once Ghidra is wired) -------------------------------

.PHONY: split split-all

# Single-binary split. Usage: make split BINARY=ffxivgame.exe
split:
	@if [ -z "$(BINARY)" ]; then echo "usage: make split BINARY=<binary>.exe"; exit 1; fi
	mkdir -p $(BUILD)/logs
	$(PY) $(TOOLS)/import_to_ghidra.py $(BINARY) 2>&1 | tee $(BUILD)/logs/$(basename $(BINARY)).import.log
	$(PY) $(TOOLS)/build_split_yaml.py $(basename $(BINARY))

split-all:
	@for exe in $(ORIG)/*.exe; do \
	    bn=$$(basename $$exe); \
	    $(MAKE) split BINARY=$$bn || exit $$?; \
	done

# --- Phase 2 (TODO once MSVC is wired) ---------------------------------

.PHONY: setup-msvc find-rosetta rosetta diff progress extract-net extract-gam extract-gam-types-rtti emit-gam-header extract-paramnames validate-chara-make validate-chara-list validate-murmur2 extract-opcodes extract-up-opcodes extract-crypt-engine

# Walk the RTTI dump for net-relevant classes; emit class→slot→fn_rva map.
extract-net:
	$(PY) $(TOOLS)/extract_net_vtables.py $(or $(BINARY),ffxivgame)

# Parse Component::GAM::CompileTimeParameter mangled types from .rdata
# strings; emit the structured (id, namespace, type, decorator) registry.
extract-gam:
	$(PY) $(TOOLS)/extract_gam_params.py $(or $(BINARY),ffxivgame)

# Emit include/net/gam_registry.h from the GAM extraction. Pulls
# extract-paramnames first so the header carries property names.
emit-gam-header: extract-gam extract-paramnames
	$(PY) $(TOOLS)/emit_gam_header.py $(or $(BINARY),ffxivgame)

# Resolve PARAMNAME_<id> string pointers from each Data class's
# MetadataProvider dispatcher (currently CharaMakeData; extend the
# DISPATCHERS dict in the script to cover Player / etc. as their
# dispatchers are identified). Enriches config/<bin>.gam_params.json
# in-place with a `paramname` field per entry.
extract-paramnames:
	$(PY) $(TOOLS)/extract_paramnames_dispatch.py $(or $(BINARY),ffxivgame)

# Augment gam_params.json with `rtti_type` parsed from the
# CompileTimeParameter template-parameter signature in the binary's
# RTTI. This is ground truth and corrects the legacy extractor's
# off-by-one Array sizes (e.g. Array<int,4> vs reported int[3]).
extract-gam-types-rtti: extract-gam extract-net
	$(PY) $(TOOLS)/extract_gam_types_rtti.py $(or $(BINARY),ffxivgame)

# Cross-validate garlemald-server's chara_info.rs parser flow against
# the GAM CharaMakeData schema.
validate-chara-make: extract-gam extract-paramnames extract-gam-types-rtti
	$(PY) $(TOOLS)/validate_chara_make.py $(or $(BINARY),ffxivgame)

# Cross-validate garlemald's build_for_chara_list against GAM
# ClientSelectData (schema-level — not byte-layout).
validate-chara-list: extract-gam extract-paramnames extract-gam-types-rtti
	$(PY) $(TOOLS)/validate_chara_list.py $(or $(BINARY),ffxivgame)

# Walk the binary's Down dispatchers (zone/lobby/chat) and emit
# opcode → vtable-slot map cross-referenced with garlemald opcodes.rs.
extract-opcodes:
	$(PY) $(TOOLS)/extract_opcode_dispatch.py $(or $(BINARY),ffxivgame)

# Up-direction (client → server) opcode reconnaissance — locates CPB
# constructors and validates that all garlemald OP_RX_* values appear in
# .text as PUSH immediates.
extract-up-opcodes:
	$(PY) $(TOOLS)/extract_up_opcodes.py $(or $(BINARY),ffxivgame)

# Decode LobbyCryptEngine's 9 vtable slots, verify the Blowfish P/S
# init tables are canonical OpenSSL pi-derived, and cross-check
# garlemald-server/common/src/blowfish_tables.rs byte-for-byte.
extract-crypt-engine:
	$(PY) $(TOOLS)/extract_crypt_engine.py $(or $(BINARY),ffxivgame)

# Compute MurmurHash2 test vectors (Python port of FUN_00d31490).
# Cross-check against garlemald's murmur_hash2; see docs/murmur2.md.
validate-murmur2:
	$(PY) $(TOOLS)/validate_murmur2.py


# Run the setup checks: wine + MSVC_TOOLCHAIN_DIR + cl.exe + objdiff.
setup-msvc:
	$(TOOLS)/setup-msvc.sh

# Pick the best Rosetta-Stone candidate function. Output:
#   build/rosetta/ffxivgame.candidates.json
#   build/rosetta/ffxivgame.top.txt
find-rosetta:
	$(PY) $(TOOLS)/find_rosetta.py $(or $(BINARY),ffxivgame)

# Compile + diff the staged Rosetta source against the original binary.
# The candidate's .cpp lives at src/<binary>/_rosetta/<sym>.cpp.
# Pass BINARY=ffxivboot.exe (or any other split binary) to retarget.
ROSETTA_FLAGS ?= /c /O2 /Oy /GR /EHsc /Gy /GS /MT /Zc:wchar_t /Zc:forScope /TP
ROSETTA_BIN_STEM = $(basename $(or $(BINARY),ffxivgame.exe))
ROSETTA_SRC_DIR = src/$(ROSETTA_BIN_STEM)/_rosetta
ROSETTA_OBJ_DIR = $(BUILD)/obj/_rosetta/$(ROSETTA_BIN_STEM)
rosetta: setup-msvc
	@if ! ls $(ROSETTA_SRC_DIR)/*.cpp >/dev/null 2>&1; then \
	    echo "no rosetta source staged in $(ROSETTA_SRC_DIR)/"; \
	    echo "run 'make find-rosetta BINARY=$(ROSETTA_BIN_STEM).exe' and hand-translate the top candidate,"; \
	    echo "or use tools/seed_templates.py to bootstrap canonical stub templates from another binary."; \
	    exit 1; \
	fi
	mkdir -p $(ROSETTA_OBJ_DIR)
	@for cpp in $(ROSETTA_SRC_DIR)/*.cpp; do \
	    name=$$(basename $$cpp .cpp); \
	    obj=$(ROSETTA_OBJ_DIR)/$$name.obj; \
	    echo ">>> cl $$cpp -> $$obj"; \
	    $(TOOLS)/cl-wine.sh $(ROSETTA_FLAGS) /Fo$$obj $$cpp || exit $$?; \
	    echo ">>> objdiff $$obj vs orig"; \
	    $(PY) $(TOOLS)/compare.py BINARY=$(ROSETTA_BIN_STEM).exe FUNC=$$name || exit $$?; \
	done

# Like `rosetta` but never bails on a non-GREEN verdict — useful for
# bulk validation across hundreds of stamped cluster siblings, where a
# single PARTIAL or MISMATCH shouldn't stop the whole sweep. The
# verdict per file is still printed; aggregate counts come from the
# log via `grep -c GREEN/PARTIAL/MISMATCH`. The tally is also written
# to build/logs/rosetta_bulk_summary_<binary>.txt for quick review.
.PHONY: rosetta-bulk
rosetta-bulk: setup-msvc
	@if ! ls $(ROSETTA_SRC_DIR)/*.cpp >/dev/null 2>&1; then \
	    echo "no rosetta source staged in $(ROSETTA_SRC_DIR)/"; \
	    exit 1; \
	fi
	mkdir -p $(ROSETTA_OBJ_DIR) $(BUILD)/logs
	@green=0; partial=0; mismatch=0; cl_fail=0; \
	for cpp in $(ROSETTA_SRC_DIR)/*.cpp; do \
	    name=$$(basename $$cpp .cpp); \
	    obj=$(ROSETTA_OBJ_DIR)/$$name.obj; \
	    echo ">>> cl $$cpp -> $$obj"; \
	    if ! $(TOOLS)/cl-wine.sh $(ROSETTA_FLAGS) /Fo$$obj $$cpp; then \
	        cl_fail=$$((cl_fail + 1)); echo "  ⚠️  cl failed (continuing)"; continue; \
	    fi; \
	    echo ">>> objdiff $$obj vs orig"; \
	    if $(PY) $(TOOLS)/compare.py BINARY=$(ROSETTA_BIN_STEM).exe FUNC=$$name >/tmp/.rb.$$$$ 2>&1; then \
	        green=$$((green + 1)); grep -E "GREEN" /tmp/.rb.$$$$ | head -1; \
	    else \
	        rc=$$?; \
	        if grep -q PARTIAL /tmp/.rb.$$$$; then partial=$$((partial + 1)); \
	        else mismatch=$$((mismatch + 1)); fi; \
	        grep -E "PARTIAL|MISMATCH" /tmp/.rb.$$$$ | head -1; \
	    fi; \
	    rm -f /tmp/.rb.$$$$; \
	done; \
	echo; echo "rosetta-bulk[$(ROSETTA_BIN_STEM)] summary  GREEN=$$green  PARTIAL=$$partial  MISMATCH=$$mismatch  cl_failed=$$cl_fail"; \
	{ echo "GREEN=$$green"; echo "PARTIAL=$$partial"; echo "MISMATCH=$$mismatch"; echo "cl_failed=$$cl_fail"; } > $(BUILD)/logs/rosetta_bulk_summary_$(ROSETTA_BIN_STEM).txt

diff:
	@if [ -z "$(FUNC)" ]; then echo "usage: make diff FUNC=Symbol::Name"; exit 1; fi
	$(PY) $(TOOLS)/compare.py FUNC=$(FUNC) $(if $(BINARY),BINARY=$(BINARY),)

progress:
	$(PY) $(TOOLS)/progress.py

# --- Phase 2.6 — byte-passthrough fallback ----------------------------
#
# For functions still without a hand-written source under _rosetta/,
# emit a __declspec(naked) `_emit`-only `.cpp` whose compiled .obj's
# .text matches orig byte-for-byte. This is the universal fallback that
# lets us produce a complete .obj inventory across the binary even
# before every function has source-level decomp; future linker work
# can then weave matched + passthrough .objs into a relinkable PE.
#
# Usage:
#   make emit-passthrough BINARY=ffxivlogin.exe         # all unmatched
#   make emit-passthrough BINARY=ffxivlogin.exe MAX=50  # cap output
#   make emit-passthrough BINARY=ffxivlogin.exe FUNC=FUN_00489290
#   make compile-passthrough BINARY=ffxivlogin.exe
#   make passthrough BINARY=ffxivlogin.exe              # emit + compile
#
# Notes on the section pragma each passthrough .cpp emits:
#   #pragma code_seg(".text$X<rva>")
# The `.text$<key>` subsection naming is a long-standing MSVC linker
# convention — link.exe sorts subsections alphabetically and packs them
# into the merged `.text` output. By keying on the orig RVA we make
# the linker land each function at the right offset within `.text`
# (modulo gap padding, which a future tool will provide).

.PHONY: emit-passthrough compile-passthrough passthrough

PASSTHROUGH_BIN_STEM = $(basename $(or $(BINARY),ffxivgame.exe))
PASSTHROUGH_SRC_DIR  = src/$(PASSTHROUGH_BIN_STEM)/_passthrough
PASSTHROUGH_OBJ_DIR  = $(BUILD)/obj/_passthrough/$(PASSTHROUGH_BIN_STEM)
# Naked-asm passthroughs don't trigger /GS — drop /GS to keep the .obj
# free of __security_cookie references that would force us to also
# bring in the cookie init runtime when linking.
PASSTHROUGH_FLAGS    ?= /c /O2 /Oy /GR- /EHs- /Gy /MT /Zc:wchar_t /Zc:forScope /TP
# `_text_gaps.cpp` on ffxivgame has ~70k subsections, exceeding COFF's
# 65535 limit — that file needs /bigobj. Smaller files don't, and
# /bigobj forces the alternate ANON object header format that breaks
# our `tools/patch_obj_alignment.py` COFF parser. So apply /bigobj
# only to files that need it (recipe: per-file detection in the loop).
PASSTHROUGH_FLAGS_BIGOBJ = $(PASSTHROUGH_FLAGS) /bigobj

emit-passthrough:
	@if [ -n "$(FUNC)" ]; then \
	    $(PY) $(TOOLS)/emit_passthrough_cpp.py $(FUNC) --binary $(PASSTHROUGH_BIN_STEM) $(if $(FORCE),--force,); \
	else \
	    $(PY) $(TOOLS)/emit_passthrough_cpp.py --all --binary $(PASSTHROUGH_BIN_STEM) $(if $(MAX),--max $(MAX),) $(if $(FORCE),--force,); \
	fi

compile-passthrough:
	@if ! ls $(PASSTHROUGH_SRC_DIR)/*.cpp >/dev/null 2>&1; then \
	    echo "no passthrough source in $(PASSTHROUGH_SRC_DIR)/ — run 'make emit-passthrough BINARY=$(PASSTHROUGH_BIN_STEM).exe' first"; \
	    exit 1; \
	fi
	mkdir -p $(PASSTHROUGH_OBJ_DIR) $(BUILD)/logs
	@total=$$(ls $(PASSTHROUGH_SRC_DIR)/*.cpp | wc -l | tr -d ' '); \
	echo ">>> compile-passthrough[$(PASSTHROUGH_BIN_STEM)] $$total .cpp files (P=$(PARALLEL_JOBS))"
	@find $(PASSTHROUGH_SRC_DIR) -maxdepth 1 -name '*.cpp' -print0 \
	    | xargs -0 -n 1 -P $(PARALLEL_JOBS) -I '{}' bash -c '\
	        cpp="$$1"; \
	        name=$$(basename "$$cpp" .cpp); \
	        obj="$(PASSTHROUGH_OBJ_DIR)/$$name.obj"; \
	        if [ -f "$$obj" ] && [ "$$obj" -nt "$$cpp" ] && [ -z "$(FORCE)" ]; then exit 0; fi; \
	        if [ "$$name" = "_text_gaps" ]; then \
	            flags="$(PASSTHROUGH_FLAGS_BIGOBJ)"; \
	        else \
	            flags="$(PASSTHROUGH_FLAGS)"; \
	        fi; \
	        if ! $(TOOLS)/cl-wine.sh $$flags /Fo"$$obj" "$$cpp" >/dev/null 2>&1; then \
	            echo "  CL FAIL: $$name" >&2; \
	        fi \
	    ' _ '{}'
	@expected=$$(ls $(PASSTHROUGH_SRC_DIR)/*.cpp | wc -l | tr -d ' '); \
	got=$$(ls $(PASSTHROUGH_OBJ_DIR)/*.obj 2>/dev/null | wc -l | tr -d ' '); \
	echo; echo "compile-passthrough[$(PASSTHROUGH_BIN_STEM)] expected=$$expected obj_count=$$got"; \
	{ echo "EXPECTED=$$expected"; echo "OBJ_COUNT=$$got"; } > $(BUILD)/logs/passthrough_$(PASSTHROUGH_BIN_STEM).txt

passthrough: emit-passthrough compile-passthrough mark-passthrough

# Sync YAML `status: unmatched → passthrough` for every function whose
# _passthrough/ .obj is byte-identical to orig (cheap COFF .text vs
# orig slice diff). Idempotent. Never overwrites `matched`.
.PHONY: mark-passthrough mark-passthrough-all
mark-passthrough:
	$(PY) $(TOOLS)/mark_passthrough_yaml.py $(PASSTHROUGH_BIN_STEM)

mark-passthrough-all:
	$(PY) $(TOOLS)/mark_passthrough_yaml.py

# Phase 2.6 — full-binary coverage (text gaps + non-code sections).
#
# After running `make passthrough` (function-level), these two
# additional emitters cover:
#   - emit-text-gaps: byte-fills the inter-function gap regions of
#     `.text` so every byte of `.text` has a contributing .obj.
#   - emit-data-sections: emits one .cpp per non-code PE section
#     (`.rdata`, `.data`, `.rsrc`, `.tls`, etc.) as a single
#     `__declspec(allocate(...))` byte array.
#
# Together with the function-level passthroughs they constitute
# everything the linker needs to produce a byte-identical PE.

.PHONY: emit-text-gaps emit-data-sections recompile-coverage
emit-text-gaps:
	$(PY) $(TOOLS)/emit_text_gaps.py $(or $(PASSTHROUGH_BIN_STEM),)

emit-data-sections:
	$(PY) $(TOOLS)/emit_data_sections.py $(or $(PASSTHROUGH_BIN_STEM),)

# Land everything needed for a re-link in one go (per-binary).
# Order: emit-passthrough (per-fn) → emit-text-gaps → emit-data-sections
# → compile-passthrough (compiles every .cpp in _passthrough/) → mark.
recompile-coverage: emit-passthrough emit-text-gaps emit-data-sections compile-passthrough mark-passthrough

# --- Phase 2.7 — full PE re-link path (Stage D + E from docs/recompilable-strategy.md)
#
# `make relink BINARY=…` builds a byte-identical PE from scratch:
#   1. emit-text-blob       — orig .text as one naked-asm function
#   2. emit-data-sections   — orig .rdata/.data/.rsrc/.tls as byte blobs
#   3. compile-passthrough  — produce one .obj per .cpp under _passthrough/
#   4. link.exe             — link them via tools/link_pe.sh
#   5. postlink_patch.py    — copy PE-header fields from orig + cert
#
# Output:    build/link/<bin>.exe
# Verify:    `make diff-pe BINARY=…` runs tools/diff_pe.py
.PHONY: emit-text-blob link relink diff-pe

emit-text-blob:
	$(PY) $(TOOLS)/emit_text_blob.py $(PASSTHROUGH_BIN_STEM)

.PHONY: emit-globals-stub
emit-globals-stub:
	$(PY) $(TOOLS)/emit_globals_stub.py $(PASSTHROUGH_BIN_STEM)

link: patch-align
	$(TOOLS)/link_pe.sh $(PASSTHROUGH_BIN_STEM)
	$(PY) $(TOOLS)/postlink_patch.py $(PASSTHROUGH_BIN_STEM)

# Patch every `_passthrough/<bin>/*.obj`'s `.text$X<rva>` subsection
# alignment to 1 byte. Idempotent. Always run before linking — fresh
# .objs default to align=16 (cl.exe / `code_seg`) which causes link.exe
# to pad up at every obj-file boundary in the merged `.text`.
.PHONY: patch-align
patch-align:
	$(PY) $(TOOLS)/patch_obj_alignment.py $(PASSTHROUGH_BIN_STEM)

relink: emit-text-blob emit-data-sections emit-globals-stub compile-passthrough link

diff-pe:
	$(PY) $(TOOLS)/diff_pe.py $(PASSTHROUGH_BIN_STEM)

# Splice a hand-written `_rosetta/<sym>.cpp` into the relink at the
# function's RVA, replacing the byte-blob's coverage there.
#   make swap-rosetta BINARY=ffxivlogin.exe FUNC=FUN_00401350
#   make relink       BINARY=ffxivlogin.exe                  # rebuild
.PHONY: swap-rosetta list-swaps swap-source-file swap-source-all list-source-swaps
swap-rosetta:
	@if [ -z "$(FUNC)" ]; then echo "usage: make swap-rosetta BINARY=<bin>.exe FUNC=FUN_<va>"; exit 1; fi
	$(PY) $(TOOLS)/swap_rosetta.py $(PASSTHROUGH_BIN_STEM) $(FUNC)

list-swaps:
	$(PY) $(TOOLS)/swap_rosetta.py $(PASSTHROUGH_BIN_STEM) --list

# Swap a hand-written multi-function source file (crt/, sqex/, etc.).
# Auto-discovers each function's RVA via reloc-aware byte-pattern
# search against orig `.text`. Strict-mode rejects files where any
# function fails to match.
#   make swap-source-file BINARY=ffxivgame.exe SRC=src/ffxivgame/sqex/Utf8String.cpp
#   make swap-source-all  BINARY=ffxivgame.exe         # all crt/sqex/sqpack/install
swap-source-file:
	@if [ -z "$(SRC)" ]; then echo "usage: make swap-source-file BINARY=<bin>.exe SRC=<path>"; exit 1; fi
	$(PY) $(TOOLS)/swap_source_file.py $(PASSTHROUGH_BIN_STEM) $(SRC)

swap-source-all:
	$(PY) $(TOOLS)/swap_source_file.py $(PASSTHROUGH_BIN_STEM) --all

list-source-swaps:
	$(PY) $(TOOLS)/swap_source_file.py $(PASSTHROUGH_BIN_STEM) --list

# --- Phase 6 follow-ups: shipped Lua script extraction ----------------

# Decode all 2671 shipped client/script/*.le.lpb files from a 1.x install
# to standard Lua 5.1 bytecode (build/lpb/*.luac), then optionally run
# unluac on each to get readable Lua source (build/lua/*.lua). Together
# this turns the install's obfuscated client-side script tree into a
# grep-able corpus — see docs/lpb_corpus_survey.md for what's in it.
#
# Required:
#   FFXIV_INSTALL  path to the 1.x install root (the dir that contains
#                  client/script/, e.g. .../FINAL FANTASY XIV).
#                  Defaults to ../ffxiv-install-environment/.../FINAL_FANTASY_XIV
# Optional:
#   UNLUAC_JAR     path to unluac.jar (Java; for the decompile pass).
#                  Defaults to /tmp/unluac/unluac.jar; if absent, the
#                  decompile pass will print install instructions.
#   PARALLEL_JOBS  worker count for the unluac pass (default: 8).
.PHONY: decode-lpb decompile-lpb lpb-corpus

REPO_ROOT      := $(shell pwd)
FFXIV_INSTALL  ?= $(REPO_ROOT)/../ffxiv-install-environment/target/prefix/drive_c/Program Files (x86)/SquareEnix/FINAL FANTASY XIV
UNLUAC_JAR     ?= /tmp/unluac/unluac.jar
PARALLEL_JOBS  ?= 8

decode-lpb:
	@if [ ! -d "$(FFXIV_INSTALL)/client/script" ]; then \
	    echo "error: \$$FFXIV_INSTALL/client/script not found." >&2; \
	    echo "       set FFXIV_INSTALL=<path-to-FINAL_FANTASY_XIV-dir>" >&2; \
	    echo "       (the dir that contains client/script/, ffxivgame.exe, etc.)" >&2; \
	    exit 1; \
	fi
	$(PY) $(TOOLS)/decode_lpb.py "$(FFXIV_INSTALL)" --out $(BUILD)/lpb

decompile-lpb:
	@if [ ! -d "$(BUILD)/lpb" ]; then \
	    echo "error: $(BUILD)/lpb missing — run 'make decode-lpb' first" >&2; \
	    exit 1; \
	fi
	@if [ ! -f "$(UNLUAC_JAR)" ]; then \
	    echo "error: $(UNLUAC_JAR) not found." >&2; \
	    echo "       Download unluac from:" >&2; \
	    echo "         https://sourceforge.net/projects/unluac/files/latest/download" >&2; \
	    echo "       Then place the .jar at /tmp/unluac/unluac.jar" >&2; \
	    echo "       (or set UNLUAC_JAR=<path>) and re-run." >&2; \
	    exit 1; \
	fi
	@if ! command -v java >/dev/null 2>&1; then \
	    echo "error: 'java' not on PATH (unluac requires JDK 8+)" >&2; \
	    exit 1; \
	fi
	@mkdir -p $(BUILD)/lua
	@n=$$(find $(BUILD)/lpb -name '*.luac' | wc -l | tr -d ' '); \
	echo ">>> Decompiling $$n .luac files via unluac (P=$(PARALLEL_JOBS))…"
	@find $(BUILD)/lpb -name '*.luac' -print0 \
	    | xargs -0 -n 1 -P $(PARALLEL_JOBS) -I '{}' bash -c '\
	        src="$$1"; rel="$${src#$(BUILD)/lpb/}"; out="$(BUILD)/lua/$${rel%.luac}.lua"; \
	        mkdir -p "$$(dirname "$$out")"; \
	        java -jar "$(UNLUAC_JAR)" "$$src" > "$$out" 2>/dev/null \
	    ' _ '{}'
	@n=$$(find $(BUILD)/lua -name '*.lua' 2>/dev/null | wc -l | tr -d ' '); \
	echo ">>> Done: $$n .lua files in $(BUILD)/lua/"

.PHONY: extract-cpp-bindings
extract-cpp-bindings:
	@if [ ! -d "$(BUILD)/lpb" ]; then \
	    echo "error: $(BUILD)/lpb missing — run 'make decode-lpb' first" >&2; \
	    exit 1; \
	fi
	$(PY) $(TOOLS)/extract_cpp_bindings.py

# Cross-reference garlemald's Lua bindings (userdata.rs) against
# methods CALLED by garlemald's own server-side scripts. Surfaces
# coverage gaps (unbound methods that scripts call) and dead bindings.
# See docs/garlemald_lua_coverage_index.md for what to do with the output.
.PHONY: garlemald-lua-coverage
garlemald-lua-coverage:
	$(PY) $(TOOLS)/garlemald_lua_coverage.py

# Walk the decompiled Lua corpus for `<work_table>.<field>` access
# patterns and emit the per-table field inventory garlemald must
# populate via SetActorProperty packets. See
# docs/work_field_inventory_index.md for the analysis.
.PHONY: extract-work-fields
extract-work-fields:
	@if [ ! -d "$(BUILD)/lua" ]; then \
	    echo "error: $(BUILD)/lua missing — run 'make decompile-lpb' first" >&2; \
	    exit 1; \
	fi
	$(PY) $(TOOLS)/extract_work_fields.py

# Cross-reference the work-field inventory against garlemald's
# SetActorProperty path table — surfaces fields the client reads
# but garlemald has no writer for. See
# docs/work_field_coverage_index.md for the analysis.
.PHONY: work-field-coverage
work-field-coverage:
	@if [ ! -f "$(BUILD)/wire/work_field_inventory.json" ]; then \
	    echo "error: $(BUILD)/wire/work_field_inventory.json missing" >&2; \
	    echo "       run 'make extract-work-fields' first" >&2; \
	    exit 1; \
	fi
	$(PY) $(TOOLS)/work_field_coverage.py

lpb-corpus: decode-lpb decompile-lpb extract-cpp-bindings
	@echo
	@echo "Corpus ready. Try:"
	@echo "  grep -r 'processEvent020_3' $(BUILD)/lua --include='*.lua' -l"
	@echo "  grep -r '_defineBaseClass'  $(BUILD)/lua --include='*.lua'"
	@echo "  grep -r '/Director/Quest/'  $(BUILD)/lua --include='*.lua'"
	@echo
	@echo "Single-script lookup by source name (e.g. Man0g0):"
	@echo "  $(PY) tools/decode_lpb.py '$(FFXIV_INSTALL)' Man0g0"
	@echo "  java -jar $(UNLUAC_JAR) build/lpb/Man0g0.luac"
	@echo
	@echo "See docs/lpb_corpus_survey.md for what's in the corpus."
