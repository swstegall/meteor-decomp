// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// For each VA in CALLER_VAS (comma-separated), print every reference TO it and
// the function that contains each reference — i.e. "who calls / points at this
// function". Distinguishes direct call sites from data references (vtable slots
// are data refs from a vftable, so this also surfaces which vtable a function
// sits in).
//
//   CALLER_VAS="0x6e32f0,..."  analyzeHeadless ... -process <bin> -noanalysis \
//       -postScript FindCallers.java
//
//@category meteor-decomp

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.symbol.Reference;
import ghidra.program.model.symbol.ReferenceIterator;

public class FindCallers extends GhidraScript {
	@Override
	public void run() throws Exception {
		String vas = System.getenv("CALLER_VAS");
		if (vas == null || vas.isEmpty()) {
			println("FindCallers: set CALLER_VAS=0x..,0x..");
			return;
		}
		for (String tok : vas.split(",")) {
			tok = tok.trim();
			if (tok.isEmpty()) continue;
			long va = Long.parseLong(tok.replaceFirst("^0x", ""), 16);
			Address target = toAddr(va);
			println("===== refs to " + tok + " =====");
			ReferenceIterator it = currentProgram.getReferenceManager().getReferencesTo(target);
			int n = 0;
			while (it.hasNext()) {
				Reference r = it.next();
				Address from = r.getFromAddress();
				Function host = getFunctionContaining(from);
				String hostName = (host == null) ? "(not in a function — data/vtable)"
					: host.getName() + " @" + host.getEntryPoint();
				println("  " + r.getReferenceType() + " from " + from + "  in " + hostName);
				if (++n > 60) { println("  ... (truncated)"); break; }
			}
			if (n == 0) println("  (no references found)");
		}
	}
}
