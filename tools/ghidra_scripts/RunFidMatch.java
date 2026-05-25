// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Force the Ghidra "Function ID" analyzer to run against an already-analyzed
// program, so a freshly-attached FidDb actually matches. A plain
// `-process` won't re-run an analyzer that already ran during import (when no
// FidDb was attached → 0 matches). We temporarily disable every other
// analyzer, run analyzeAll() (only FID fires), then restore the originals so
// the project's analysis options aren't left corrupted.
//
// Run headless after AttachFidDatabase.java (which attaches the FidDb):
//   analyzeHeadless ... -preScript AttachFidDatabase.java \
//                       -postScript RunFidMatch.java -postScript DumpFunctions.java
//
//@category meteor-decomp

import java.util.HashMap;
import java.util.Map;

import ghidra.app.script.GhidraScript;
import ghidra.framework.options.OptionType;
import ghidra.framework.options.Options;
import ghidra.program.model.listing.Program;

public class RunFidMatch extends GhidraScript {

	private static final String FID = "Function ID";

	@Override
	public void run() throws Exception {
		Options opts = currentProgram.getOptions(Program.ANALYSIS_PROPERTIES);

		// Save + disable every analyzer enable-toggle except Function ID.
		// The enable-toggle is the bare analyzer name (a top-level BOOLEAN
		// option with no '.'); sub-options look like "Analyzer.SubOption".
		Map<String, Boolean> saved = new HashMap<>();
		int disabled = 0;
		for (String name : opts.getOptionNames()) {
			if (name.indexOf('.') >= 0) {
				continue;
			}
			if (opts.getType(name) != OptionType.BOOLEAN_TYPE) {
				continue;
			}
			boolean cur = opts.getBoolean(name, false);
			saved.put(name, cur);
			if (!name.equals(FID) && cur) {
				opts.setBoolean(name, false);
				disabled++;
			}
		}
		if (opts.contains(FID)) {
			opts.setBoolean(FID, true);
		}
		println("RunFidMatch: disabled " + disabled + " analyzers; running Function ID over "
			+ currentProgram.getFunctionManager().getFunctionCount() + " functions");

		analyzeAll(currentProgram);

		// Restore original enable-states so the .gpr keeps its analysis config.
		for (Map.Entry<String, Boolean> e : saved.entrySet()) {
			opts.setBoolean(e.getKey(), e.getValue());
		}
		println("RunFidMatch: Function ID pass complete; restored " + saved.size()
			+ " analyzer options");
	}
}
