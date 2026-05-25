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
| `0x002dc8b0` | `Debug_executeScriptFromCutSceneTable` | named |  |  | `FUN_006dc8b0` |
| `0x002e2130` | `Lua_send8byteStateAt0x68_via_0x130_variantB` | lua_impl | 0x130 | _sendMovementState | `FUN_006e2130` |
| `0x002e2af0` | `Lua_sendByteUshortAt0x68_via_0x132` | lua_impl | 0x132 | _sendCompoundState | `FUN_006e2af0` |
| `0x002e32f0` | `MyPlayer_fadeInNowLoadingForNoticeEventJustInArea_impl` | lua_impl |  | _fadeInNowLoadingForNoticeEventJustInArea | `FUN_006e32f0` |
| `0x002e42e0` | `Lua_listIndexSend_via_0x130_variantA` | lua_impl | 0x130 | _sendListIndex | `FUN_006e42e0` |
| `0x002e5ad0` | `Lua_sendByteToggle_via_opcode_0x131` | lua_impl | 0x131 | _sendByteToggle | `FUN_006e5ad0` |
| `0x002e6360` | `Lua_sendChallenge_via_opcode_0x134` | lua_impl | 0x134 | _sendChallenge | `FUN_006e6360` |
| `0x002e6d90` | `Lua_worldMaster__lookAtPlayerTutorial` | lua_impl |  | _lookAtPlayerTutorial | `FUN_006e6d90` |
| `0x002e85e0` | `lua_updateWork_impl` | lua_impl | 0x12f | _updateWork | `FUN_006e85e0` |
| `0x002e8f50` | `MyPlayer_cancelNotice_impl` | lua_impl |  | _cancelNotice | `FUN_006e8f50` |
| `0x002fae70` | `CharaBase_invokeLua_onChangeSystemFlag` | inbound_invoker |  |  | `FUN_006fae70` |
| `0x002fb9c0` | `CutScene_invokeLua_onFinalizeClip` | inbound_invoker |  |  | `FUN_006fb9c0` |
| `0x002fbc50` | `CutScene_method_setActiveAndFinalize` | named |  |  | `FUN_006fbc50` |
| `0x002fbcc0` | `CutScene_invokeLua_onInitializationClip_PreviewSetupClip` | inbound_invoker |  |  | `FUN_006fbcc0` |
| `0x002fbe80` | `CutScene_invokeLua_onInitializationClip` | inbound_invoker |  |  | `FUN_006fbe80` |
| `0x002fc080` | `CutScene_invokeLua_onShowUIClip` | inbound_invoker |  |  | `FUN_006fc080` |
| `0x002fc260` | `CutScene_invokeLua_onHideUIClip` | inbound_invoker |  |  | `FUN_006fc260` |
| `0x002fc3a0` | `CutScene_invokeLua_onShowWidgetClip` | inbound_invoker |  |  | `FUN_006fc3a0` |
| `0x002fc4d0` | `CutScene_invokeLua_onHideWidgetClip` | inbound_invoker |  |  | `FUN_006fc4d0` |
| `0x002fc5f0` | `CutScene_invokeLua_onOpenUIClip` | inbound_invoker |  |  | `FUN_006fc5f0` |
| `0x002fe960` | `DesktopWidget_invokeLua_onTargetChanged` | inbound_invoker |  |  | `FUN_006fe960` |
| `0x002febb0` | `DesktopWidget_invokeLua_onTargetDecided` | inbound_invoker |  |  | `FUN_006febb0` |
| `0x002fede0` | `DesktopWidget_invokeLua_onPreWarp` | inbound_invoker |  |  | `FUN_006fede0` |
| `0x002fef10` | `DesktopWidget_invokeLua_onPostWarp` | inbound_invoker |  |  | `FUN_006fef10` |
| `0x00304230` | `Player_handleLimitAddictedNotice` | named |  |  | `FUN_00704230` |
| `0x00305eb0` | `Lua_queryBinding_dispatchType_sends_0x135` | lua_impl | 0x135 | _queryBinding | `FUN_00705eb0` |
| `0x0032b440` | `Functor_NpcBaseClass_buildOutputStackOperatorVector` | functor |  |  | `FUN_0072b440` |
| `0x0032c7e0` | `Functor_NpcBaseClass_buildInputStackOperatorVector` | functor |  |  | `FUN_0072c7e0` |
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
| `0x00359940` | `ZoneIn_handler_opcode_11_CutScene_onShowWidgetClip` | opcode_handler |  |  | `FUN_00759940` |
| `0x003599e0` | `ZoneIn_handler_opcode_12_CutScene_onHideWidgetClip` | opcode_handler |  |  | `FUN_007599e0` |
| `0x00359a60` | `ZoneIn_handler_opcode_16_Debug_scriptExec` | opcode_handler |  |  | `FUN_00759a60` |
| `0x00359ad0` | `ZoneIn_handler_opcode_17_onPreCutSceneCancel` | opcode_handler |  |  | `FUN_00759ad0` |
| `0x00359b40` | `ZoneIn_handler_opcode_18_onPostCutSceneCancel` | opcode_handler |  |  | `FUN_00759b40` |
| `0x00359bb0` | `ZoneIn_handler_opcode_20_DesktopWidget_onPreWarp` | opcode_handler |  |  | `FUN_00759bb0` |
| `0x00359c20` | `ZoneIn_handler_opcode_21_DesktopWidget_onPostWarp` | opcode_handler |  |  | `FUN_00759c20` |
| `0x00359c90` | `ZoneIn_handler_vtable_dispatch_slot21` | opcode_handler |  |  | `FUN_00759c90` |
| `0x00359cb0` | `ZoneIn_handler_vtable_dispatch_slot22` | opcode_handler |  |  | `FUN_00759cb0` |
| `0x00359cf0` | `ZoneIn_handler_vtable_dispatch_slot24` | opcode_handler |  |  | `FUN_00759cf0` |
| `0x00359d10` | `ZoneIn_handler_vtable_dispatch_slot25` | opcode_handler |  |  | `FUN_00759d10` |
| `0x00359ed0` | `ZoneIn_handler_opcode_39_internal_map_insert` | opcode_handler |  |  | `FUN_00759ed0` |
| `0x00359f50` | `ZoneIn_handler_opcode_40_timed_or_immediate_exec` | opcode_handler |  |  | `FUN_00759f50` |
| `0x0035a060` | `ZoneIn_handler_opcode_43_CharaBase_onChangeSystemFlag` | opcode_handler |  |  | `FUN_0075a060` |
| `0x0035a0e0` | `ZoneIn_handler_opcode_44_MyPlayer_onReceiveLimitAddicted` | opcode_handler |  |  | `FUN_0075a0e0` |
| `0x0035d750` | `ZoneIn_handler_opcode_4_DesktopWidget_onTargetChanged` | opcode_handler |  |  | `FUN_0075d750` |
| `0x0035d780` | `ZoneIn_handler_opcode_5_DesktopWidget_onTargetDecided` | opcode_handler |  |  | `FUN_0075d780` |
| `0x0035d7b0` | `ZoneIn_handler_opcode_6_GetCurrentTarget_query` | opcode_handler |  |  | `FUN_0075d7b0` |
| `0x0035d830` | `ZoneIn_handler_opcode_7_CutScene_onInitializationClip_Preview` | opcode_handler |  |  | `FUN_0075d830` |
| `0x0035d860` | `ZoneIn_handler_opcode_8_CutScene_onInitializationClip` | opcode_handler |  |  | `FUN_0075d860` |
| `0x0035d890` | `ZoneIn_handler_opcode_9_CutScene_onShowUIClip` | opcode_handler |  |  | `FUN_0075d890` |
| `0x0035d8d0` | `ZoneIn_handler_opcode_10_CutScene_onHideUIClip` | opcode_handler |  |  | `FUN_0075d8d0` |
| `0x0035d900` | `ZoneIn_handler_opcode_13_CutScene_onOpenUIClip` | opcode_handler |  |  | `FUN_0075d900` |
| `0x0035d950` | `ZoneIn_handler_opcode_14_CutScene_setActiveAndFinalize` | opcode_handler |  |  | `FUN_0075d950` |
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
| `0x0036c220` | `ZoneIn_handler_chat_variant_C_tell` | opcode_handler |  |  | `FUN_0076c220` |
| `0x0036e270` | `ZoneOut_sendScriptError_opcode_0x12d` | opcode_sender | 0x12d |  | `FUN_0076e270` |
| `0x00376340` | `UserDataReceiver_vtable_noop_inherited` | receiver_slot |  |  | `FUN_00776340` |
| `0x00494090` | `Lua_send6argRpc_via_opcode_0x12e` | lua_impl | 0x12e | _send6argRpc | `FUN_00894090` |
| `0x00498d20` | `Player_invokeLua_onTouch_proximityBegin` | inbound_invoker |  |  | `FUN_00898d20` |
| `0x00498eb0` | `Player_invokeLua_onTouch_proximityEnd` | inbound_invoker |  |  | `FUN_00898eb0` |
| `0x0049c8b0` | `Router_dispatch_to_MyPlayer_handleLimitAddictedNotice` | router |  |  | `FUN_0089c8b0` |
| `0x0049ca80` | `Router_dispatch_to_CharaBase_onChangeSystemFlag` | router |  |  | `FUN_0089ca80` |
| `0x0049d170` | `ChatBuilder_singleTarget_writeCommandTag_B` | chat |  |  | `FUN_0089d170` |
| `0x0049d220` | `ChatBuilder_singleTarget_writeCommandTag_A` | chat |  |  | `FUN_0089d220` |
| `0x0049d340` | `ChatBuilder_singleTarget_writeCommandTag_D` | chat |  |  | `FUN_0089d340` |
| `0x0049e320` | `ChatBuilder_tell_readRecipientName` | chat |  |  | `FUN_0089e320` |
| `0x0049e3f0` | `ChatBuilder_tell_thunk` | chat |  |  | `FUN_0089e3f0` |
| `0x0049eed0` | `Network_UserDataReceiver_ctor` | receiver |  |  | `FUN_0089eed0` |
| `0x0049fbf0` | `Network_UserDataReceiver_multiModeDispatcher` | dispatcher |  |  | `FUN_0089fbf0` |
| `0x004a2b70` | `UserDataReceiver_vtable_slot23_resolveActorIntoField0x18` | receiver_slot |  |  | `FUN_008a2b70` |
| `0x004a2d50` | `UserDataReceiver_vtable_slot22_appendPayloadToContainer` | receiver_slot |  |  | `FUN_008a2d50` |
| `0x004a3990` | `Router_dispatch_to_CutScene_onInitializationClip` | router |  |  | `FUN_008a3990` |
| `0x004a39e0` | `Router_dispatch_to_CutScene_onShowUIClip` | router |  |  | `FUN_008a39e0` |
| `0x004a3a40` | `Router_dispatch_to_CutScene_onHideUIClip` | router |  |  | `FUN_008a3a40` |
| `0x004a3aa0` | `Router_dispatch_to_CutScene_method_FUN_006fc3a0` | router |  |  | `FUN_008a3aa0` |
| `0x004a3b00` | `Router_dispatch_to_CutScene_onHideWidgetClip` | router |  |  | `FUN_008a3b00` |
| `0x004a3b50` | `Router_dispatch_to_CutScene_onOpenUIClip` | router |  |  | `FUN_008a3b50` |
| `0x004a3ea0` | `Router_dispatch_to_DesktopWidget_onTargetChanged` | router |  |  | `FUN_008a3ea0` |
| `0x004a3ed0` | `Router_dispatch_to_DesktopWidget_onTargetDecided` | router |  |  | `FUN_008a3ed0` |
| `0x004a4410` | `Router_dispatch_to_DesktopWidget_onPreWarp` | router |  |  | `FUN_008a4410` |
| `0x004a44d0` | `Router_dispatch_to_DesktopWidget_onPostWarp` | router |  |  | `FUN_008a44d0` |
| `0x004a45d0` | `Router_dispatch_to_System_onPreCutSceneCancel` | router |  |  | `FUN_008a45d0` |
| `0x004a4720` | `Router_dispatch_to_System_onPostCutSceneCancel` | router |  |  | `FUN_008a4720` |
| `0x009b5300` | `ProtoChannel_dispatchPacketById` | dispatcher |  |  | `FUN_00db5300` |

## SEQ-005 cutscene-hang relevance

Two registrars here name functions the SEQ-005 kick-dispatcher work
has been circling (see the SEQ-005 memory chain):

- `_cancelNotice` registrar — the Notice stream's cancel path (1.x has 5 command streams: Command/Talk/Notice/Emote/Push).
- `_fadeInNowLoadingForNoticeEventJustInArea` — previously located via MyPlayer vtable slot 66 as the kick-dispatcher clearer; ffxivDecomp
  independently names its registrar, corroborating that anchor and
  giving the registrar→thunk (LAB_0071e3f0) link to force-disassemble next.
