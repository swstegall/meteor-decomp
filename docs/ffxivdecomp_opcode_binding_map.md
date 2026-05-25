# ffxivDecomp opcode + Lua-binding map (imported)

> **Imported from ffxivDecomp** (github.com/Yokimitsuro/ffxivDecomp), an
> independent docs-only RE of the same 1.23b `ffxivgame.exe`. Names are
> **cross-referenced, not byte-verified** here; confirm against the asm
> when matching. Regenerate with `tools/import_ffxivdecomp_symbols.py`.

All VAs are absolute (image base 0x00400000); RVA = VA - 0x400000.
Machine form: `config/ffxivgame.ffxivdecomp_symbols.json`.

| RVA | name | kind | opcode | Lua binding | current symbol |
|-----|------|------|--------|-------------|----------------|
| `0x002c72e0` | `WorkSyncAlt_serializePayloadAndSend_opcode_0x133` | lua_impl | 0x133 | _updateWorkAlt | `FUN_006c72e0` |
| `0x002dacd0` | `Lua_listObjectDelete_sends_0x130_variantA` | lua_impl | 0x130 | _sendListObjectDelete | `FUN_006dacd0` |
| `0x002dae90` | `Lua_listObjectQueueAdd_sends_0x130_variantA` | lua_impl | 0x130 | _sendListObjectQueueAdd | `FUN_006dae90` |
| `0x002e2130` | `Lua_send8byteStateAt0x68_via_0x130_variantB` | lua_impl | 0x130 | _sendMovementState | `FUN_006e2130` |
| `0x002e2af0` | `Lua_sendByteUshortAt0x68_via_0x132` | lua_impl | 0x132 | _sendCompoundState | `FUN_006e2af0` |
| `0x002e42e0` | `Lua_listIndexSend_via_0x130_variantA` | lua_impl | 0x130 | _sendListIndex | `FUN_006e42e0` |
| `0x002e5ad0` | `Lua_sendByteToggle_via_opcode_0x131` | lua_impl | 0x131 | _sendByteToggle | `FUN_006e5ad0` |
| `0x002e6360` | `Lua_sendChallenge_via_opcode_0x134` | lua_impl | 0x134 | _sendChallenge | `FUN_006e6360` |
| `0x002e6d90` | `Lua_worldMaster__lookAtPlayerTutorial` | lua_impl |  | _lookAtPlayerTutorial | `FUN_006e6d90` |
| `0x002e85e0` | `lua_updateWork_impl` | lua_impl | 0x12f | _updateWork | `FUN_006e85e0` |
| `0x00305eb0` | `Lua_queryBinding_dispatchType_sends_0x135` | lua_impl | 0x135 | _queryBinding | `FUN_00705eb0` |
| `0x00330960` | `PlayerBase_registerLua_executeTalk` | registrar |  | _executeTalk | `FUN_00730960` |
| `0x00330ab0` | `PlayerBase_registerLua_executeEmote` | registrar |  | _executeEmote | `FUN_00730ab0` |
| `0x00330c00` | `PlayerBase_registerLua_canExecuteCommand` | registrar |  | _canExecuteCommand | `FUN_00730c00` |
| `0x00330d50` | `PlayerBase_registerLua_canExecuteTalk` | registrar |  | _canExecuteTalk | `FUN_00730d50` |
| `0x00330ea0` | `PlayerBase_registerLua_canExecuteEmote` | registrar |  | _canExecuteEmote | `FUN_00730ea0` |
| `0x00330ff0` | `PlayerBase_registerLua_cancelCommand` | registrar |  | _cancelCommand | `FUN_00730ff0` |
| `0x00331140` | `PlayerBase_registerLua_cancelTalk` | registrar |  | _cancelTalk | `FUN_00731140` |
| `0x00331290` | `PlayerBase_registerLua_cancelNotice` | registrar |  | _cancelNotice | `FUN_00731290` |
| `0x003313e0` | `PlayerBase_registerLua_cancelEmote` | registrar |  | _cancelEmote | `FUN_007313e0` |
| `0x00331530` | `PlayerBase_registerLua_cancelPush` | registrar |  | _cancelPush | `FUN_00731530` |
| `0x00331680` | `PlayerBase_registerLua_breakCommand` | registrar |  | _breakCommand | `FUN_00731680` |
| `0x003317d0` | `PlayerBase_registerLua_isEventPlaying` | registrar |  | _isEventPlaying | `FUN_007317d0` |
| `0x00331920` | `PlayerBase_registerLua_isCommandPlaying` | registrar |  | _isCommandPlaying | `FUN_00731920` |
| `0x00331a70` | `PlayerBase_registerLua_countCommandPlaying` | registrar |  | _countCommandPlaying | `FUN_00731a70` |
| `0x00331bc0` | `PlayerBase_registerLua_fadeIn` | registrar |  | _fadeIn | `FUN_00731bc0` |
| `0x00331d10` | `PlayerBase_registerLua_fadeOut` | registrar |  | _fadeOut | `FUN_00731d10` |
| `0x00331e60` | `PlayerBase_registerLua_waitForFading` | registrar |  | _waitForFading | `FUN_00731e60` |
| `0x00331fb0` | `PlayerBase_registerLua_isFading` | registrar |  | _isFading | `FUN_00731fb0` |
| `0x00332100` | `PlayerBase_registerLua_cancelFading` | registrar |  | _cancelFading | `FUN_00732100` |
| `0x00332250` | `PlayerBase_registerLua_fadeInAfterWarp` | registrar |  | _fadeInAfterWarp | `FUN_00732250` |
| `0x003323a0` | `PlayerBase_registerLua_resetFade` | registrar |  | _resetFade | `FUN_007323a0` |
| `0x003324f0` | `PlayerBase_registerLua_fadeInNowLoadingForNoticeEventJustInArea` | registrar |  | _fadeInNowLoadingForNoticeEventJustInArea | `FUN_007324f0` |
| `0x00332640` | `PlayerBase_registerLua_lockPlayerControl` | registrar |  | _lockPlayerControl | `FUN_00732640` |
| `0x00332790` | `PlayerBase_registerLua_unlockPlayerControl` | registrar |  | _unlockPlayerControl | `FUN_00732790` |
| `0x003328e0` | `PlayerBase_registerLua_isPlayerControlEnabled` | registrar |  | _isPlayerControlEnabled | `FUN_007328e0` |
| `0x00332a30` | `PlayerBase_registerLua_lockLockonControl` | registrar |  | _lockLockonControl | `FUN_00732a30` |
| `0x00332b80` | `PlayerBase_registerLua_unlockLockonControl` | registrar |  | _unlockLockonControl | `FUN_00732b80` |
| `0x00332cd0` | `PlayerBase_registerLua_isLockonControlEnabled` | registrar |  | _isLockonControlEnabled | `FUN_00732cd0` |
| `0x00332e20` | `PlayerBase_registerLua_lockCameraControl` | registrar |  | _lockCameraControl | `FUN_00732e20` |
| `0x00332f70` | `PlayerBase_registerLua_unlockCameraControl` | registrar |  | _unlockCameraControl | `FUN_00732f70` |
| `0x003330c0` | `PlayerBase_registerLua_isCameraControlEnabled` | registrar |  | _isCameraControlEnabled | `FUN_007330c0` |
| `0x00333210` | `PlayerBase_registerLua_setLockonTarget` | registrar |  | _setLockonTarget | `FUN_00733210` |
| `0x00333360` | `PlayerBase_registerLua_getLockonTarget` | registrar |  | _getLockonTarget | `FUN_00733360` |
| `0x003334b0` | `PlayerBase_registerLua_waitForMapLoaded` | registrar |  | _waitForMapLoaded | `FUN_007334b0` |
| `0x00333600` | `PlayerBase_registerLua_setMusic` | registrar |  | _setMusic | `FUN_00733600` |
| `0x00333750` | `PlayerBase_registerLua_setWeather` | registrar |  | _setWeather | `FUN_00733750` |
| `0x00336fc0` | `NpcBaseClass_registerLua_callServerOnTalk` | registrar |  | _callServerOnTalk | `FUN_00736fc0` |
| `0x00337110` | `NpcBaseClass_registerLua_callServerOnEmote` | registrar |  | _callServerOnEmote | `FUN_00737110` |
| `0x00337260` | `NpcBaseClass_registerLua_callServerOnPush` | registrar |  | _callServerOnPush | `FUN_00737260` |
| `0x0033f080` | `PlayerBase_registerLua_executeCommand` | registrar |  | _executeCommand | `FUN_0073f080` |
| `0x0033f1d0` | `PlayerBase_registerLua_callServerOnCommand` | registrar |  | _callServerOnCommand | `FUN_0073f1d0` |
| `0x0033f320` | `PlayerBase_registerLua_doServerOnCommand` | registrar |  | _doServerOnCommand | `FUN_0073f320` |
| `0x00353f90` | `PlayerBase_registerAllLuaBindings` | master_block |  |  | `FUN_00753f90` |
| `0x0035e1c0` | `ZoneOut_send_large_simple` | opcode_sender | 0x12d |  | `FUN_0075e1c0` |
| `0x0035e230` | `ZoneOut_send_large_checksummed_v3` | opcode_sender | 0x12d |  | `FUN_0075e230` |
| `0x0035e3a0` | `ZoneOut_send_large_checksummed_v1` | opcode_sender | 0x12d |  | `FUN_0075e3a0` |
| `0x0035e510` | `ZoneOut_send_large_checksummed_v2` | opcode_sender | 0x12d |  | `FUN_0075e510` |
| `0x0035e670` | `ZoneOut_send_opcode_0x12e_104B` | opcode_sender | 0x12e |  | `FUN_0075e670` |
| `0x0035e860` | `ZoneOut_send_opcode_0x130_32B_variantA` | opcode_sender | 0x130 |  | `FUN_0075e860` |
| `0x0035e8d0` | `ZoneOut_send_opcode_0x130_32B_variantB` | opcode_sender | 0x130 |  | `FUN_0075e8d0` |
| `0x0035e950` | `ZoneOut_send_opcode_0x133_56B` | opcode_sender | 0x133 |  | `FUN_0075e950` |
| `0x0035ea50` | `ZoneOut_send_opcode_0x131_24B_byte` | opcode_sender | 0x131 |  | `FUN_0075ea50` |
| `0x0035eac0` | `ZoneOut_send_opcode_0x132_24B_byteUshort` | opcode_sender | 0x132 |  | `FUN_0075eac0` |
| `0x0035eba0` | `ZoneOut_send_opcode_0x134_40B_withNonce` | opcode_sender | 0x134 |  | `FUN_0075eba0` |
| `0x0035ecd0` | `ZoneOut_send_opcode_0x135_24B_dword` | opcode_sender | 0x135 |  | `FUN_0075ecd0` |
| `0x0036e270` | `ZoneOut_sendScriptError_opcode_0x12d` | opcode_sender | 0x12d |  | `FUN_0076e270` |
| `0x00494090` | `Lua_send6argRpc_via_opcode_0x12e` | lua_impl | 0x12e | _send6argRpc | `FUN_00894090` |
| `0x009b5300` | `ProtoChannel_dispatchPacketById` | dispatcher |  |  | `FUN_00db5300` |

## SEQ-005 cutscene-hang relevance

Two registrars here name functions the SEQ-005 kick-dispatcher work
has been circling (see the SEQ-005 memory chain):

- `_cancelNotice` registrar — the Notice stream's cancel path (1.x has 5 command streams: Command/Talk/Notice/Emote/Push).
- `_fadeInNowLoadingForNoticeEventJustInArea` — previously located via MyPlayer vtable slot 66 as the kick-dispatcher clearer; ffxivDecomp
  independently names its registrar, corroborating that anchor and
  giving the registrar→thunk (LAB_0071e3f0) link to force-disassemble next.
