// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Read-only sanity check: count how many functions in the .gpr carry
// FID library names vs. are still FUN_xxx. Confirms whether a FID apply
// actually persisted names into the project (run with -readOnly).
//
//@category meteor-decomp

import java.util.regex.Pattern;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionIterator;

public class CountLibNames extends GhidraScript {
	@Override
	public void run() throws Exception {
		Pattern lib = Pattern.compile(
			"^_?(lua[A-Z_]|luaL_|luaopen_|EVP_|BN_|RSA_|DSA_|DH_|EC_|SHA|MD5|MD4|BF_|DES_|AES_|"
			+ "RC4|RC2|HMAC|ASN1|X509|PEM_|CRYPTO_|OPENSSL_|inflate|deflate|crc32|adler32|"
			+ "_tr_|memcpy|memset|strlen|strcmp|strncpy|malloc|free)");
		int libn = 0, fun = 0, total = 0;
		FunctionIterator it = currentProgram.getFunctionManager().getFunctions(true);
		while (it.hasNext()) {
			Function f = it.next();
			total++;
			String n = f.getName();
			if (n.startsWith("FUN_") || n.startsWith("thunk_FUN_") || n.startsWith("LAB_")
				|| n.startsWith("SUB_")) {
				fun++;
			}
			else if (lib.matcher(n).find()) {
				libn++;
			}
		}
		println("CountLibNames: total=" + total + " lib-named=" + libn + " FUN_xxx=" + fun);
	}
}
