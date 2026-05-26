// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Decompile a specific set of functions (by VA) to C and print them. Targeted
// counterpart to DumpPseudoC.java (which does the whole binary) — for reading
// a handful of named functions during a focused investigation.
//
//   DECOMP_VAS="0x6e32f0,0x89f180,..."  analyzeHeadless ... -process <bin> \
//       -noanalysis -postScript DecompileToText.java
//
//@category meteor-decomp

import ghidra.app.decompiler.DecompInterface;
import ghidra.app.decompiler.DecompileResults;
import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.util.task.TaskMonitor;

public class DecompileToText extends GhidraScript {
	@Override
	public void run() throws Exception {
		String vas = System.getenv("DECOMP_VAS");
		if (vas == null || vas.isEmpty()) {
			println("DecompileToText: set DECOMP_VAS=0x..,0x..");
			return;
		}
		DecompInterface dec = new DecompInterface();
		dec.openProgram(currentProgram);
		dec.setSimplificationStyle("decompile");
		for (String tok : vas.split(",")) {
			tok = tok.trim();
			if (tok.isEmpty()) continue;
			long va = Long.parseLong(tok.replaceFirst("^0x", ""), 16);
			Address addr = toAddr(va);
			Function fn = getFunctionAt(addr);
			println("===== " + tok + " =====");
			if (fn == null) {
				println("(no function at " + tok + ")");
				continue;
			}
			println("// " + fn.getName() + "  @" + addr + "  size="
				+ fn.getBody().getNumAddresses());
			DecompileResults res = dec.decompileFunction(fn, 60, TaskMonitor.DUMMY);
			if (res != null && res.decompileCompleted()) {
				println(res.getDecompiledFunction().getC());
			} else {
				println("// DECOMPILE FAILED: "
					+ (res == null ? "null" : res.getErrorMessage()));
			}
		}
		dec.dispose();
	}
}
