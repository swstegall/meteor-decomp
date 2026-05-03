# `DesktopWidget` decomp — the engine's UI control plane

> Last updated: 2026-05-03 — focused decomp of `DesktopWidget.lpb`,
> the second-most-referenced engine global (3,154 hits across the
> corpus, even more than WorldMaster's 3,100). DesktopWidget is the
> top-level UI controller — every quest/director/widget call that
> manipulates targeting, log/message pools, user config, user macros,
> or widget containers reaches into here.

## File inventory

| File | LOC | Purpose |
|---|---:|---|
| `DesktopWidget.lpb` (main) | 402 | Top-level: lifecycle hooks + mode control + macro/widget command dispatch |
| `DesktopWidget_connector.lpb` | **26,564** | The "connector" — cross-class API access surface (1,135 distinct method references) |
| `DesktopWidget_itemDetail.lpb` | 10,687 | Item-detail UI rendering |
| `DesktopWidget_materia.lpb` | 410 | Materia attachment / detachment UI |
| `DesktopWidget_u.lpb` | 472 | C++ binding declarations (43 `_method_cpp/_inl` pairs) |
| `DesktopUtil.lpb` | 39 | Utility helpers (separate class) |

**Total: ~38KB of Lua source** — about 10× larger than WorldMaster.
The size lives almost entirely in `_connector` (26.5K LOC) and
`_itemDetail` (10.7K LOC). Main + materia are slim wrappers.

`WidgetBaseClass.lpb` (the parent) adds another ~4KB:

| File | LOC | Purpose |
|---|---:|---|
| `WidgetBaseClass.lpb` (main) | 421 | Widget lifecycle + UI command dispatch |
| `WidgetBaseClass_common.lpb` | 3,361 | Shared widget primitives |
| `WidgetBaseClass_u.lpb` | 241 | C++ bindings (24 pairs) |

## C++-bound API (43 methods, from `_u`)

Per the corpus-wide `_cpp/_inl` enumeration in
`docs/cpp_bindings_index.md`, DesktopWidget's 43 engine-bound
methods organize into clear domains:

### Widget container ops (8)

| Method | Role |
|---|---|
| `_createWidgetInWidgetContainer(self, ...)` | Spawn a widget into a named container |
| `_deleteCreatingWidgetInWidgetContainer(self, ...)` | Cancel an in-progress widget creation |
| `_isCreatingWidgetInWidgetContainer(self, name)` | Predicate: creation in flight? |
| `_isExistWidgetInWidgetContainer(self, name)` | Predicate: widget present? |
| `_getWidgetFromWidgetContainer(self, name)` | Lookup by name |
| `_getWidgetContainerSize(self, ...)` | Container item count |
| `_reserveWidgetContainer(self, ...)` | Pre-allocate slots |
| `_setKeyboardFocusedWidget(self, w)` / `_getKeyboardFocusedWidget(self)` | Keyboard-focus management |

### Target cursor / lock-on (12)

| Method | Role |
|---|---|
| `_getTargetCharacter(self)` | Current target |
| `_setTargetCharacter(self, char)` | Set hard target |
| `_setTargetCharacterByDisplayName(self, name)` | Set target by name |
| `_getCharacterByDisplayNameForTextCommand(self, name)` | Resolve name → char (for `/target Name`) |
| `_getCurrentTargetCursor(self)` / `_setCurrentTargetCursor(self, c)` | Active cursor slot |
| `_initTargetCursors(self)` | Reset all cursors |
| `_setTargetCursorImage(self, ...)` | Cursor sprite |
| `_setLockonCursorImage(self, ...)` | Lock-on sprite |
| `_setAllTargetCursorMask(self, mask)` | Bulk visibility mask |
| `_lockTargetCursorControl(self)` / `_unlockTargetCursorControl(self)` | Lock cursor control to a widget |
| `_isTargetCursorControlEnabled(self)` | Predicate |
| `_setTargetableDistance(self, dist)` | Max-targeting range |

### User config (4)

| Method | Role |
|---|---|
| `_getUserConfig(self, key)` | Read config field |
| `_setUserConfig(self, key, val)` | Write config field |
| `_resetUserConfig(self)` | Reset to defaults |
| `_saveUserConfig(self)` | Persist to disk |

### User macros (8)

| Method | Role |
|---|---|
| `_getUserMacroData(self, idx)` / `_setUserMacroData(self, idx, data)` | Macro body (the command sequence) |
| `_getUserMacroIcon(self, idx)` / `_setUserMacroIcon(self, idx, icon)` | Macro icon |
| `_getUserMacroTitle(self, idx)` / `_setUserMacroTitle(self, idx, title)` | Macro title |
| `_saveUserMacro(self)` | Persist all macros |

### Log / message pool (3)

| Method | Role |
|---|---|
| `_appendLogPool(self, ...)` | Append to log pool (chat / system messages) |
| `_appendMessagePool(self, ...)` | Append to message pool (popups) |
| `_clearLogPool(self)` | Wipe log |

### Misc UI control (8)

| Method | Role |
|---|---|
| `_parseTextCommand(self, text)` | Parse `/command arg1 arg2` text input → action |
| `_sendCountDown(self, ...)` | Trigger countdown UI element |
| `_getLastAttacker(self)` | Most-recent damager (combat target tracking) |
| `_waitForCameraTutorial(self)` | Block until camera-tutorial completes |
| `_waitForItemSearchWidget(self)` | Block until item-search widget closes |
| `_waitForTargetTutorial(self)` | Block until target-tutorial completes |

## Lua-side API (DesktopWidget main, 402 LOC)

The main file's overrides + Lua-defined methods include:

### Lifecycle hooks

- `_onInit(self)` — calls `_callSuperClassFunc("_onInit")`, sets up
  the work table
- `_onLoop(self)` — per-frame tick (UI update loop)
- `_onPreWarp(self)` / `_onPostWarp(self)` — fired before / after a
  zone change
- `_onPreCutSceneCancel(self)` / `_onPostCutSceneCancel(self)` —
  fired around cutscene-cancel events
- `_onCreatedWidgetInWidgetContainer(self, widget)` — observer for
  widget-creation events

### Mode control

| Method | Purpose |
|---|---|
| `desktopMode` | Field: current desktop mode |
| `getModeLevel(self)` | Read mode level |
| `orderDesktopWidgetMode(self, mode)` | Request mode change |
| `cancelDesktopWidgetMode(self, mode)` | Cancel pending mode (used in man0g0.lua line 244-ish for cutscene cleanup) |
| `initDesktopInitialParameter(self, ...)` | One-time init |

### Macro / widget commands

| Method | Purpose |
|---|---|
| `commandMacro(self, macro_data)` | Execute a user macro |
| `cancelMacroCommand(self)` | Cancel running macro |
| `isMacroCommandPlaying(self)` | Predicate |
| `commandCreateWidget(self, ...)` | Create a widget by command |
| `cancelWidgetCommand(self)` | Cancel widget creation |
| `isCreateWidgetCommandPlaying(self)` | Predicate |
| `commandAboutWidget(self, ...)` | "About" command on a widget |
| `cancelCommandAboutWidget(self)` | Cancel |
| `isCommandAboutWidgetPlaying(self)` | Predicate |
| `getSystemCommand(self, ...)` | System-command lookup |
| `createWidget(self, name, ...)` | Direct widget creation entry |

### Forwarding / wrappers

| Method | Forwards to |
|---|---|
| `showMessage(self, ...)` | Wraps `_appendMessagePool` |
| `showLog(self, target, kind, ...)` | Wraps `_appendLogPool` (called by `worldMaster:say` with kind=32, `worldMaster:notify` with kind=33) |
| `notify(self, ...)` | Convenience wrapper |
| `cancelAllTarget(self)` | Clears all targeting via `_setTargetCharacter(nil)` etc. |
| `cueAttentionOnClient(self, ...)` | Trigger attention cue |
| `openPublicInformDialogWidget(self, ...)` | Open the public-info popup |
| `recordRequestInformation(self, ...)` | Record info-request action |
| `updateBazaarPackage(self, ...)` | Bazaar inventory refresh |

## DesktopWidget_connector (26,564 LOC, 1,135 method refs)

The connector is the **cross-class API integration surface**. It
references methods from many other classes — `_chat`, `_countMember`,
`_countStack`, `_getEquippingItem`, `_getMember`, `_getNetStatUser`,
`_haveEnmityCharacters`, `_isAttached`, `_isDealing`, `_isEquipping`,
plus dozens of widget-container ops, target ops, etc.

This file is the implementation of how DesktopWidget composes the
underlying engine bindings into higher-level UI behaviours. It's
the LARGEST single Lua file in the entire shipped script corpus
by a wide margin.

For garlemald, the connector's method names are useful as a
reference for **what the UI expects of a player/actor's runtime
state** — every method called via `_chara:_getEquippingItem(slot)`
etc. needs to be backed by a real value or the UI rendering will
fail.

## DesktopWidget_itemDetail (10,687 LOC)

Item-detail rendering — the UI that pops up when you mouse-over
or right-click an item. References:
- `_getCatalogID(self, item)` — item's catalog ID
- `_getMaxStack(self, item)` — stack-size cap
- `_isEquipping(self, item)` — predicate
- `getAttachedMateriaCount(self, item)` — materia count
- `_format(self, ...)` — string formatting
- (plus ~hundreds more)

The decomp here would be valuable IF garlemald ever needs to
serve item-detail UI properly — but most of this is client-internal
rendering that garlemald just needs to provide DATA for via
SetActorProperty / item-package packets.

## WidgetBaseClass — the parent (425 LOC main + 3,361 _common)

WidgetBaseClass is the abstract widget base. Methods include:

### Lifecycle

- `_onInit(self, ...)`, `_onFinalize(self)`, `_onLoop(self)`,
  `_onTimer(self, dt)`, `_onHoverHelp(self, ...)`
- `_setLoopInterval(self, ms)` — per-widget tick rate
- `_setParentWidget(self, parent)` — parent in hierarchy
- `processFinalize(self)` — cleanup

### UI command dispatch (the talk-flow's UI side)

- `_onUICommandEvent(self, evt)` / `_onUICommandRequest(self, req)` —
  paired event/request hooks
- `processUICommandEvent(self, ...)` / `processUICommandRequest(self, ...)`
- `processWidgetCreated(self, ...)` / `processWidgetDeleted(self, ...)`

### Form / sheet loading

- `_loadForm(self, ...)`, `loadFormData(self, ...)`
- `_loadKeyTemporarily(self, key)` / `_loadMultiKeyAsync(self, ...)`
- `_onLoadMultiKeyAsync(self, ...)`
- `loadSpreadSheetDataAsync(self, ...)`
- `processSpreadSheetDataAsync(self, ...)` /
  `processSpreadSheetDataLoaded(self, ...)`
- `requestLoadSpreadSheetData(self, ...)`
- `requestSsdLoadSheet(self, ...)` /
  `requestSsdLoadKeyMin/Max(self, ...)`

### Sub-targets

- `executeSubTarget(self, ...)`, `requestSelectSubTarget(self, ...)`
- `processSubTargetDecided(self, ...)`

### Work table

- `desktopWidgetWork`, `commonTimer`
- type tags: `actor`, `boolean`, `integer32`, `nesting`, `select`

## Implications for garlemald

### Garlemald's coverage report flagged 0 desktopWidget gaps

Per `docs/garlemald_lua_coverage_index.md`, garlemald's scripts make
0 calls into `desktopWidget` directly — the variable name isn't in
the CONVENTIONS table because garlemald scripts don't go through
the desktopWidget global. **This is correct** — desktopWidget is
purely client-side; garlemald drives UI via packets, not by
calling the global.

### Existing garlemald UI work hits this surface indirectly

Garlemald's text-sheet packets, target-update packets, and
content-widget packets ALL terminate in DesktopWidget methods on
the client side:
- `SetTextSheet` (kind=32) → `worldMaster:say` → `desktopWidget:showLog(target, 32, ...)` → `_appendLogPool`
- Target updates → `desktopWidget:_setTargetCharacter` (per Phase 6 item #4 SyncWriter mechanics)
- Content widget create → `_createWidgetInWidgetContainer`
- Cutscene-cancel cleanup → `_onPostCutSceneCancel` (the
  `desktopWidget:cancelDesktopWidgetMode(16)` call in
  Man0g0.lpb's `processEvent000_4`)

So this decomp doesn't surface new garlemald work; it confirms the
client-side terminations of garlemald's existing UI packet flows.

### User config / macro persistence is client-side

The `_saveUserConfig` / `_saveUserMacro` family writes to local
disk on the client, not back to the server. Garlemald **doesn't**
need to persist user-config / macro state — that's the launcher's
job. Garlemald only needs to receive the user's chosen settings if
they affect server-relevant decisions (auto-target preferences,
keybind-triggered abilities), which they generally don't.

### `_parseTextCommand` is the slash-command entry

When the user types `/target Name` or `/macro 5`, the engine calls
`desktopWidget:_parseTextCommand(text)` on the client. The parsed
action then becomes a Lua call into the appropriate handler. Garlemald
sees this only as the resulting wire message (e.g. a target update
packet) — it doesn't need to parse text commands itself.

### `_waitFor*Tutorial` blocks are tutorial-flow gates

Three tutorials are surfaced as blocking primitives:
- `_waitForCameraTutorial` (camera control onboarding)
- `_waitForTargetTutorial` (targeting onboarding)
- `_waitForItemSearchWidget` (item-search UI demo)

These are called from tutorial scripts (the SimpleQuestBattleBaseClass
hierarchy decomped in `docs/director_quest_decomp.md`) to pause the
script until the player completes the corresponding tutorial. Garlemald
should be aware that these are CLIENT-side blocks — the server doesn't
need to drive them, but should expect the corresponding "tutorial
complete" packet to arrive when the user finishes.

## Cross-references

- `docs/world_master_decomp.md` — companion (the other major Lua
  global; WorldMaster wraps DesktopWidget for `say`/`notify`)
- `docs/cpp_bindings_index.md` — the 43 desktopWidget bindings are
  the third-largest C++ surface in the corpus
- `docs/director_quest_decomp.md` — DirectorBaseClass uses
  desktopWidget.closeAllOwnedContentWidget for cleanup
- `docs/scenario_monster_decomp.md` — QuestBaseClass's cinematic
  primitives all wrap desktopWidget calls
- `docs/garlemald_lua_coverage_index.md` — confirms garlemald's
  scripts don't directly use desktopWidget (correct: it's
  client-only)
- `project_garlemald_text_sheet_no_source.md` (memory) —
  garlemald's text-sheet packet builders that ultimately
  feed into `_appendLogPool` / `_appendMessagePool`
