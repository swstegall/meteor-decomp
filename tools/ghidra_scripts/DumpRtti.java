// meteor-decomp — clean-room decompilation of FINAL FANTASY XIV 1.x client binaries
// Copyright (C) 2026  Samuel Stegall
// SPDX-License-Identifier: AGPL-3.0-or-later
//
// Walk MSVC RTTI symbols recovered by Ghidra's RTTI Analyzer and emit:
//   config/<binary>.rtti.json   — class name + vtable RVA + base-class chain
//
// MSVC RTTI mangling reminders (so the JSON is interpretable downstream):
//   ??_R0    type_info record  (class name + null-terminated descriptor)
//   ??_R1    base class descriptor
//   ??_R2    base class array
//   ??_R3    class hierarchy descriptor
//   ??_R4    complete object locator (the thing immediately above each vtable)
//   ??_7     vtable
//
// Ghidra's "Microsoft RTTI" analyzer creates symbols with these names when
// auto-analysis runs against an MSVC PE binary. We pick out ??_R4 records,
// follow their references back to the vtable, and emit one JSON entry per
// class.
//
//@category meteor-decomp

import java.io.File;
import java.io.FileWriter;
import java.io.PrintWriter;
import java.util.Iterator;

import ghidra.app.script.GhidraScript;
import ghidra.program.model.address.Address;
import ghidra.program.model.listing.Function;
import ghidra.program.model.listing.FunctionManager;
import ghidra.program.model.listing.Program;
import ghidra.program.model.mem.Memory;
import ghidra.program.model.mem.MemoryBlock;
import ghidra.program.model.symbol.Symbol;
import ghidra.program.model.symbol.SymbolTable;

public class DumpRtti extends GhidraScript {

    @Override
    public void run() throws Exception {
        Program prog = currentProgram;
        long imageBase = prog.getImageBase().getOffset();

        String repoRoot = System.getenv("METEOR_DECOMP_ROOT");
        if (repoRoot == null) repoRoot = ".";
        String binary = prog.getName().toLowerCase().replace(".exe", "");
        File configDir = new File(repoRoot, "config");
        configDir.mkdirs();
        File out = new File(configDir, binary + ".rtti.json");

        SymbolTable tbl = prog.getSymbolTable();
        FunctionManager fnmgr = prog.getFunctionManager();
        Memory mem = prog.getMemory();
        PrintWriter w = new PrintWriter(new FileWriter(out));

        // ALSO emit a parallel JSONL slot map: one line per
        // (class, slot, fn_rva) tuple. build_split_yaml.py reads this to
        // override module classification — every function pointed to by a
        // class's vtable gets that class name as its module guess.
        File slotsFile = new File(configDir, binary + ".vtable_slots.jsonl");
        PrintWriter slots = new PrintWriter(new FileWriter(slotsFile));

        w.println("[");
        boolean first = true;
        int count = 0;
        int slotCount = 0;

        Iterator<Symbol> iter = tbl.getAllSymbols(true).iterator();
        while (iter.hasNext() && !monitor.isCancelled()) {
            Symbol s = iter.next();
            String name = s.getName(true);
            if (!isVtableSymbol(name)) continue;

            Address addr = s.getAddress();
            long rva = addr.getOffset() - imageBase;
            String cls = stripVtableSuffix(name);

            // Walk the vtable: read consecutive 32-bit pointers until we
            // hit something that isn't a function in the .text section.
            // We follow each pointer, look up the function, and emit a
            // (class, slot, fn_rva, fn_name) tuple per slot.
            int slotIdx = 0;
            int slotsForThisClass = 0;
            try {
                while (slotIdx < 1024) {  // sanity cap; no real vtable is this big
                    Address slotAddr = addr.add(slotIdx * 4L);
                    int ptr = mem.getInt(slotAddr);
                    if (ptr == 0) break;
                    long ptrU = ((long) ptr) & 0xFFFFFFFFL;
                    Address target = prog.getAddressFactory().getDefaultAddressSpace()
                        .getAddress(ptrU);
                    MemoryBlock blk = mem.getBlock(target);
                    if (blk == null || !blk.isExecute()) break;
                    Function fn = fnmgr.getFunctionAt(target);
                    if (fn == null) break;
                    long fnRva = ptrU - imageBase;
                    slots.printf("{\"class\": %s, \"slot\": %d, \"fn_rva\": %d, \"fn_name\": %s}%n",
                        jsonStr(cls), slotIdx, fnRva, jsonStr(fn.getName()));
                    slotCount++;
                    slotsForThisClass++;
                    slotIdx++;
                }
            } catch (Exception e) {
                // Reading past the end of memory or hitting a bad pointer is
                // the natural terminator; ignore.
            }

            if (!first) w.println(",");
            first = false;
            w.printf("  {\"rva\": %d, \"rva_hex\": \"0x%x\", \"class\": %s, \"vtable_symbol\": %s, \"slot_count\": %d}",
                rva, rva, jsonStr(cls), jsonStr(name), slotsForThisClass);
            count++;
        }

        w.println();
        w.println("]");
        w.close();
        slots.close();
        println(String.format("DumpRtti: %s — %d vtable records, %d total slots → %s + %s",
            binary, count, slotCount, out, slotsFile));
    }

    private static boolean isVtableSymbol(String name) {
        if (name == null) return false;
        return name.endsWith("_vftable")
            || name.endsWith("`vftable'")
            || name.endsWith("::vftable")
            || name.endsWith("_rtti_complete_object_locator");
    }

    private static String stripVtableSuffix(String name) {
        for (String suf : new String[] {
                "_vftable", "::`vftable'", "::vftable",
                "_rtti_complete_object_locator"}) {
            if (name.endsWith(suf)) return name.substring(0, name.length() - suf.length());
        }
        return name;
    }

    private static String jsonStr(String s) {
        StringBuilder sb = new StringBuilder("\"");
        for (int i = 0; i < s.length(); i++) {
            char c = s.charAt(i);
            switch (c) {
                case '"':  sb.append("\\\""); break;
                case '\\': sb.append("\\\\"); break;
                case '\n': sb.append("\\n"); break;
                case '\r': sb.append("\\r"); break;
                case '\t': sb.append("\\t"); break;
                default:
                    if (c < 0x20) sb.append(String.format("\\u%04x", (int) c));
                    else sb.append(c);
            }
        }
        sb.append('"');
        return sb.toString();
    }
}
