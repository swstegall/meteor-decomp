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
// Headless, env-driven Function ID library population — a no-prompt
// replacement for the stock CreateMultipleLibraries.java (whose ask*/.properties
// flow is headless-hostile). Creates (or opens) a FidDb and ingests all
// programs under a project folder as one library, using the same FidService
// API CreateMultipleLibraries uses.
//
// Env vars:
//   FID_DB        absolute path to the .fidb (created if missing)
//   FID_FAMILY    library family name (e.g. "zlib")
//   FID_VERSION   library version      (e.g. "1.2.3")
//   FID_VARIANT   library variant      (e.g. "x86")
//   FID_LANG      LanguageID           (e.g. "x86:LE:32:default")
//   FID_ROOT      project folder to ingest (e.g. "/zlib/1.2.3/x86"); default "/"
//
// Run once via: analyzeHeadless <proj> <name>[/<folder>] -process <anyProg>
//   -readOnly -noanalysis -postScript PopulateFidLibrary.java
// (the -process target only triggers the script; it ingests FID_ROOT itself.)
//
//@category meteor-decomp

import java.io.File;
import java.util.ArrayList;
import java.util.List;

import ghidra.app.script.GhidraScript;
import ghidra.feature.fid.db.FidDB;
import ghidra.feature.fid.db.FidFile;
import ghidra.feature.fid.db.FidFileManager;
import ghidra.feature.fid.service.FidPopulateResult;
import ghidra.feature.fid.service.FidService;
import ghidra.framework.model.DomainFile;
import ghidra.framework.model.DomainFolder;
import ghidra.program.database.ProgramContentHandler;
import ghidra.program.model.lang.LanguageID;
import ghidra.util.task.TaskMonitor;

public class PopulateFidLibrary extends GhidraScript {

	@Override
	public void run() throws Exception {
		String dbPath = req("FID_DB");
		String family = req("FID_FAMILY");
		String version = req("FID_VERSION");
		String variant = req("FID_VARIANT");
		String lang = req("FID_LANG");
		String rootPath = System.getenv("FID_ROOT");
		if (rootPath == null || rootPath.isEmpty()) {
			rootPath = "/";
		}

		File dbFile = new File(dbPath);
		FidFileManager mgr = FidFileManager.getInstance();
		if (!dbFile.exists()) {
			mgr.createNewFidDatabase(dbFile);
			println("PopulateFidLibrary: created " + dbFile);
		}
		mgr.addUserFidFile(dbFile);

		FidFile fidFile = null;
		for (FidFile f : mgr.getUserAddedFiles()) {
			if (f.getName().equals(dbFile.getName())) {
				fidFile = f;
				break;
			}
		}
		if (fidFile == null) {
			printerr("PopulateFidLibrary: could not find attached FidFile for " + dbFile.getName());
			return;
		}

		DomainFolder root = rootPath.equals("/")
			? state.getProject().getProjectData().getRootFolder()
			: state.getProject().getProjectData().getFolder(rootPath);
		if (root == null) {
			printerr("PopulateFidLibrary: project folder not found: " + rootPath);
			return;
		}

		ArrayList<DomainFile> programs = new ArrayList<>();
		findPrograms(programs, root);
		println("PopulateFidLibrary: " + programs.size() + " programs under " + root.getPathname()
			+ " -> library " + family + " " + version + " " + variant);
		if (programs.isEmpty()) {
			printerr("PopulateFidLibrary: no programs found; nothing to populate");
			return;
		}

		FidService service = new FidService();
		FidDB fidDb = fidFile.getFidDB(true);
		try {
			FidPopulateResult result = service.createNewLibraryFromPrograms(fidDb, family, version,
				variant, programs, null, new LanguageID(lang), null, null, TaskMonitor.DUMMY);
			if (result != null) {
				println("PopulateFidLibrary: result = " + result);
			}
			fidDb.saveDatabase("populate " + family + " " + version, monitor);
			println("PopulateFidLibrary: saved " + dbFile);
		}
		finally {
			fidDb.close();
		}
	}

	private String req(String name) {
		String v = System.getenv(name);
		if (v == null || v.isEmpty()) {
			throw new IllegalStateException("missing required env var " + name);
		}
		return v;
	}

	private void findPrograms(List<DomainFile> programs, DomainFolder folder) {
		if (folder == null) {
			return;
		}
		for (DomainFile df : folder.getFiles()) {
			if (df.getContentType().equals(ProgramContentHandler.PROGRAM_CONTENT_TYPE)) {
				programs.add(df);
			}
		}
		for (DomainFolder sub : folder.getFolders()) {
			findPrograms(programs, sub);
		}
	}
}
