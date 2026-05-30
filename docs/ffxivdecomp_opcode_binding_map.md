# ffxivDecomp opcode + Lua-binding map (imported)

> **Imported from ffxivDecomp** (github.com/Yokimitsuro/ffxivDecomp), an
> independent docs-only RE of the same 1.23b `ffxivgame.exe`. Names are
> **cross-referenced, not byte-verified** here; confirm against the asm
> when matching. Regenerate with `tools/import_ffxivdecomp_symbols.py`.

All VAs are absolute (image base 0x00400000); RVA = VA - 0x400000.
Machine form: `config/ffxivgame.ffxivdecomp_symbols.json`.

| RVA | name | kind | opcode | Lua binding | current symbol |
|-----|------|------|--------|-------------|----------------|
| `0x000b3c50` | `NetworkModule_topLevelTick` | named |  |  | `FUN_004b3c50` |
| `0x000d6d30` | `Application_dispatchToZoneClient` | dispatcher |  |  | `FUN_004d6d30` |
| `0x000d9970` | `ChunkRegistry_LookupAt_4AC` | named |  |  | `FUN_004d9970` |
| `0x000da680` | `Application_mainTick_perFrame_eventLoopAndSubsystems` | named |  |  | `FUN_004da680` |
| `0x000dc690` | `Zone_MAIN_inbound_opcode_dispatcher_50plus_handlers` | dispatcher |  |  | `FUN_004dc690` |
| `0x000de190` | `HandleMap_Find` | named |  |  | `FUN_004de190` |
| `0x000df0a0` | `LobbyClient_ctor` | named |  |  | `FUN_004df0a0` |
| `0x000df3d0` | `LobbyClient_teardownAndRelease` | named |  |  | `FUN_004df3d0` |
| `0x000df6d0` | `ChatOut_send_opcode_0xc9_chatMessage_536B` | chat | 0xc9 |  | `FUN_004df6d0` |
| `0x000df810` | `ChatOut_send_opcode_0xc8_tell_560B` | chat | 0xc8 |  | `FUN_004df810` |
| `0x000df9a0` | `ChatOut_send_handshake_or_loginAck_opcode_0x02_or_0x12c` | chat | 0x02 |  | `FUN_004df9a0` |
| `0x000e0240` | `ZoneClient_dispatchOutbound` | dispatcher |  |  | `FUN_004e0240` |
| `0x000e0290` | `ZoneClient_sendKeepalive` | named |  |  | `FUN_004e0290` |
| `0x000e0320` | `ZoneClient_sendLargeStatePush` | named |  |  | `FUN_004e0320` |
| `0x000e0690` | `LobbyClient_proxyStartA` | named |  |  | `FUN_004e0690` |
| `0x000e0790` | `LobbyClient_proxyStartB` | named |  |  | `FUN_004e0790` |
| `0x000e11f0` | `LobbyClient_getReadinessField` | named |  |  | `FUN_004e11f0` |
| `0x000e20a0` | `ZoneClient_mainLoopTick` | named |  |  | `FUN_004e20a0` |
| `0x000e30a0` | `NetworkClientModule_tick` | named |  |  | `FUN_004e30a0` |
| `0x000e3750` | `ChatTellDescriptor_construct_opcode_0xc8` | chat | 0xc8 |  | `FUN_004e3750` |
| `0x000e3b90` | `ScalarTypeLoginParam_Utf8_flag2_ctor` | named |  |  | `FUN_004e3b90` |
| `0x000e5ca0` | `ZoneClient_packetTree_rbLookupOrInsert_bySequence` | named |  |  | `FUN_004e5ca0` |
| `0x000e5ff0` | `ZoneClient_packetDispatch_treeOrDestroy_threshold_0x1c11` | dispatcher |  |  | `FUN_004e5ff0` |
| `0x000e7750` | `IpcPacket_acquireSendSlot` | named |  |  | `FUN_004e7750` |
| `0x000e80b0` | `IpcPacket_finalizeAndSend` | named |  |  | `FUN_004e80b0` |
| `0x0015cf70` | `SessionBatch_processMultiRecord_count_in_payload_0xa4_40Bperrecord` | named |  |  | `FUN_0055cf70` |
| `0x0016d3c0` | `ChatParser_lookupCommandIdByName` | chat |  |  | `FUN_0056d3c0` |
| `0x0016e040` | `ChatParser_parseTypedArg` | chat |  |  | `FUN_0056e040` |
| `0x0016e380` | `ChatParser_tokenizeAndParse` | chat |  |  | `FUN_0056e380` |
| `0x00173460` | `Actor_isBazaarDealer` | named |  |  | `FUN_00573460` |
| `0x001735e0` | `Actor_getStateMainSkill_at_idx` | named |  |  | `FUN_005735e0` |
| `0x00173610` | `Actor_getConstanceCommandSlot_at_idx` | named |  |  | `FUN_00573610` |
| `0x00173640` | `Actor_getGiftCommandSlot_at_idx` | named |  |  | `FUN_00573640` |
| `0x00173670` | `Actor_getSkillLevel_at_idx` | named |  |  | `FUN_00573670` |
| `0x001736a0` | `Actor_getAbilityCostPointUsed` | named |  |  | `FUN_005736a0` |
| `0x001736c0` | `Actor_getAbilityCostPointMax` | named |  |  | `FUN_005736c0` |
| `0x001736e0` | `Actor_getConstanceCostPointUsed` | named |  |  | `FUN_005736e0` |
| `0x00173700` | `Actor_getConstanceCostPointMax` | named |  |  | `FUN_00573700` |
| `0x00173720` | `Actor_getGiftCostPointUsed` | named |  |  | `FUN_00573720` |
| `0x00173740` | `Actor_getGiftCostPointMax` | named |  |  | `FUN_00573740` |
| `0x00173760` | `Actor_isCommandAcquired` | named |  |  | `FUN_00573760` |
| `0x001737a0` | `Actor_getCommandAt_complex` | named |  |  | `FUN_005737a0` |
| `0x001738b0` | `Player_getVariableCommandConfirmRaise` | named |  |  | `FUN_005738b0` |
| `0x001738d0` | `Player_getVariableCommandConfirmWarp` | named |  |  | `FUN_005738d0` |
| `0x001738f0` | `Player_getVariableCommandContent` | named |  |  | `FUN_005738f0` |
| `0x00173910` | `Player_getVariableCommandPlaceDriven_at_idx` | named |  |  | `FUN_00573910` |
| `0x00173940` | `Player_getVariableCommandEmoteSit_default10001` | named |  |  | `FUN_00573940` |
| `0x00175550` | `ZoneIn_opcode_0x18d_sessionGated_bufferOrDirect_dispatch` | dispatcher | 0x18d |  | `FUN_00575550` |
| `0x00176050` | `ZoneIn_opcode_0x196_MULTI_FIELD_BIT_PACKED_8flags_8ushorts` | named | 0x196 |  | `FUN_00576050` |
| `0x00176140` | `ZoneIn_opcode_0x1a3_UI_MSGPOOL_push_uint` | named | 0x1a3 |  | `FUN_00576140` |
| `0x00176150` | `ZoneIn_opcode_0x198_STRING_UPDATE_pushToSubsystem_0xc` | named | 0x198 |  | `FUN_00576150` |
| `0x00176240` | `ZoneIn_opcode_0x143_DESPAWN_extractAndForwardToDespawnHandler` | named | 0x143 |  | `FUN_00576240` |
| `0x00176250` | `ZoneIn_opcode_0x17c_SPAWN_extractAndForwardToFactory` | named | 0x17c |  | `FUN_00576250` |
| `0x001762c0` | `ZoneIn_opcode_0x17d_thinBridge_with8B_to_subsystem_0x18` | named | 0x17d |  | `FUN_005762c0` |
| `0x001762d0` | `ZoneIn_opcode_0x17e_STATE_EVENT_uint32_session_gated` | named | 0x17e |  | `FUN_005762d0` |
| `0x001762e0` | `ZoneIn_opcode_0x17f_STATE_EVENT_uint64_session_gated_typeA` | named | 0x17f |  | `FUN_005762e0` |
| `0x001762f0` | `ZoneIn_opcode_0x180_STATE_EVENT_uint64_session_gated_typeB` | named | 0x180 |  | `FUN_005762f0` |
| `0x00176300` | `ZoneIn_opcode_0x181_STATE_EVENT_uint64_session_gated_typeC` | named | 0x181 |  | `FUN_00576300` |
| `0x00176310` | `ZoneIn_opcode_0x182_STATE_EVENT_uint64_session_gated_typeD` | named | 0x182 |  | `FUN_00576310` |
| `0x00176320` | `ZoneIn_opcode_0x183_STATE_EVENT_uint_to_subsystem_0x18_variantA` | named | 0x183 |  | `FUN_00576320` |
| `0x00176330` | `ZoneIn_opcode_0x184_STATE_EVENT_uint_to_subsystem_0x18_variantB` | named | 0x184 |  | `FUN_00576330` |
| `0x00176340` | `ZoneIn_opcode_0x185_STATE_EVENT_uint_to_subsystem_0x18_variantC` | named | 0x185 |  | `FUN_00576340` |
| `0x00176350` | `ZoneIn_opcode_0x186_MULTI_ACTOR_STATE_SET_12Bperrecord` | named | 0x186 |  | `FUN_00576350` |
| `0x00176360` | `ZoneIn_opcode_0x188_LINKSHELL_ENTRY_single_to_factory` | named | 0x188 |  | `FUN_00576360` |
| `0x00176370` | `ZoneIn_opcode_0x189_LINKSHELL_ENTRY_BATCH_to_factory` | named | 0x189 |  | `FUN_00576370` |
| `0x00176380` | `ZoneIn_opcode_0x18a_BULK_PAIR_SET_8Bperentry_countAt_0x60` | named | 0x18a |  | `FUN_00576380` |
| `0x00176390` | `ZoneIn_opcode_0x187_WORKSYNC_typedPacket_extractForwarder` | named | 0x187 |  | `FUN_00576390` |
| `0x001763a0` | `ZoneIn_opcode_0x18b_MEMBERINFO_typedPacket_extractForwarder` | named | 0x18b |  | `FUN_005763a0` |
| `0x001763b0` | `ZoneIn_opcode_0x17a_STATE_EVENT_uint_to_subsystem_0x18` | named | 0x17a |  | `FUN_005763b0` |
| `0x001763c0` | `ZoneIn_opcode_0x16d_ACTOR_EVENT_byte_payload` | named | 0x16d |  | `FUN_005763c0` |
| `0x00176430` | `ZoneIn_opcode_0x16e_ACTOR_EVENT_with_context_lookup` | named | 0x16e |  | `FUN_00576430` |
| `0x001764c0` | `ZoneIn_opcode_0x146_ACTOR_EVENT_with_context_lookup` | named | 0x146 |  | `FUN_005764c0` |
| `0x00176560` | `ZoneIn_opcode_0x148_actorLookup_thenDispatch_subsystem_0x24` | dispatcher | 0x148 |  | `FUN_00576560` |
| `0x00176bf0` | `ZoneIn_opcode_0x176_ACTOR_EVENT_simple_payload` | named | 0x176 |  | `FUN_00576bf0` |
| `0x00176c60` | `ZoneIn_opcode_0x18f_ACTOR_TRIGGER_no_payload` | named | 0x18f |  | `FUN_00576c60` |
| `0x00176cd0` | `ZoneIn_opcode_0x190_ACTOR_EVENT_with_payload` | named | 0x190 |  | `FUN_00576cd0` |
| `0x00176d40` | `ZoneIn_opcode_0x191_ACTOR_PING_lookupDispatchNoPayload` | dispatcher | 0x191 |  | `FUN_00576d40` |
| `0x00178970` | `PerFrameTick_Subsystems_widgets_zone_spawn_etc` | named |  |  | `FUN_00578970` |
| `0x00178c90` | `ZoneIn_opcode_0x193_SYSTEM_ERROR_dispatcher_22codes_with_localized_strings` | dispatcher | 0x193 |  | `FUN_00578c90` |
| `0x001794c0` | `DebugConsole_achievementListCommand` | named |  |  | `FUN_005794c0` |
| `0x0017ab60` | `GetMemberAt_0x10` | named |  |  | `FUN_0057ab60` |
| `0x00180e70` | `ZoneIn_0x148_SINGLE_ACTION_RESULT_to_actor_queue` | named |  |  | `FUN_00580e70` |
| `0x00180ef0` | `ZoneIn_0x149_BATCH_ACTION_RESULT_multiRecord_to_actor_queue` | named |  |  | `FUN_00580ef0` |
| `0x00180f70` | `ZoneIn_0x14a_ACTION_BATCH_16fixed_to_actor_queue` | named |  |  | `FUN_00580f70` |
| `0x00180ff0` | `ZoneIn_0x14b_ACTION_BATCH_32fixed_to_actor_queue` | named |  |  | `FUN_00580ff0` |
| `0x00181070` | `ZoneIn_0x14c_LARGE_ACTION_BATCH_64records_to_actor_queue` | named |  |  | `FUN_00581070` |
| `0x001810f0` | `ZoneIn_0x14d_STATUS_SINGLE_to_actor_queue` | named |  |  | `FUN_005810f0` |
| `0x00181170` | `ZoneIn_0x14e_STATUS_VARIABLE_countByte_to_actor_queue` | named |  |  | `FUN_00581170` |
| `0x001811f0` | `ZoneIn_0x14f_STATUS_EFFECT_LIST_16slots_to_actor_queue` | named |  |  | `FUN_005811f0` |
| `0x00181270` | `ZoneIn_0x150_EXTENDED_STATUS_LIST_32slots_to_actor_queue` | named |  |  | `FUN_00581270` |
| `0x001812f0` | `ZoneIn_0x151_STATUS_LIST_64fixed_to_actor_queue` | named |  |  | `FUN_005812f0` |
| `0x00181370` | `ZoneIn_0x152_IDS_SINGLE_ushort_to_actor_queue` | named |  |  | `FUN_00581370` |
| `0x001813f0` | `ZoneIn_0x153_IDS_VARIABLE_countByte_to_actor_queue` | named |  |  | `FUN_005813f0` |
| `0x00181470` | `ZoneIn_0x154_IDS_LIST_16fixed_to_actor_queue` | named |  |  | `FUN_00581470` |
| `0x001814f0` | `ZoneIn_0x155_IDS_LIST_32fixed_to_actor_queue` | named |  |  | `FUN_005814f0` |
| `0x00181570` | `ZoneIn_0x156_IDS_LIST_64fixed_to_actor_queue` | named |  |  | `FUN_00581570` |
| `0x00183440` | `PerFrameSubsystem_slot7_INBOUND_WORKSYNC_PUMP_complex_32pertick` | named |  |  | `FUN_00583440` |
| `0x001836d0` | `PerFrameSubsystem_slot8_INBOUND_WORKSYNC_PUMP_simple_32pertick` | named |  |  | `FUN_005836d0` |
| `0x00185f60` | `MsgQueue_WriteSlot` | named |  |  | `FUN_00585f60` |
| `0x00187f80` | `MsgQueue_FillTwoSlots` | named |  |  | `FUN_00587f80` |
| `0x00189260` | `MsgQueue_EnqueueWithPriority200` | named |  |  | `FUN_00589260` |
| `0x001c5c80` | `ZoneIn_handler_default_noop` | opcode_handler |  |  | `FUN_005c5c80` |
| `0x002b0220` | `Bootup_stateMachineTick` | named |  |  | `Application::Main::Menu::Bootup::BootupManager::vfunc2` |
| `0x002c1c00` | `MultiActorStateSet_loopRecords_lookupAndApply_12Bstride` | named |  |  | `FUN_006c1c00` |
| `0x002c31f0` | `MultiActorStateSet_processPayload_vtableDispatch` | dispatcher |  |  | `FUN_006c31f0` |
| `0x002c5020` | `DespawnPipeline_scanPendingList_conditionalBreakup` | named |  |  | `FUN_006c5020` |
| `0x002c5150` | `DespawnPipeline_constructBreakupBuilder_enqueueToRingBuffer` | named |  |  | `FUN_006c5150` |
| `0x002c5240` | `MemberInfoUpdater_FACTORY_constructAndEnqueue` | named |  |  | `FUN_006c5240` |
| `0x002c5750` | `PropertyUpdater_FACTORY_constructAndEnqueue_vtableCallback` | named |  |  | `FUN_006c5750` |
| `0x002c5de0` | `DespawnPipeline_forwarder_toBreakupConstructor` | named |  |  | `FUN_006c5de0` |
| `0x002c5df0` | `MemberInfoUpdater_forwarder_toFactory` | named |  |  | `FUN_006c5df0` |
| `0x002c5f40` | `SpawnPipeline_outerRing_packetTypeDispatcher_with2711tag` | dispatcher |  |  | `FUN_006c5f40` |
| `0x002c6a70` | `BulkPairSet_loopEntries_insertToMap_8Bstride` | named |  |  | `FUN_006c6a70` |
| `0x002c6b20` | `WorkSyncUpdater_FACTORY_constructAndEnqueue` | named |  |  | `FUN_006c6b20` |
| `0x002c72e0` | `WorkSyncAlt_serializePayloadAndSend_opcode_0x133` | lua_impl | 0x133 | _updateWorkAlt | `WorkSyncAlt_serializePayloadAndSend_opcode_0x133` |
| `0x002c82a0` | `BulkPairSet_initOnceAndForward` | named |  |  | `FUN_006c82a0` |
| `0x002c8340` | `WorkSyncUpdater_forwarder_toFactory` | named |  |  | `FUN_006c8340` |
| `0x002c8cf0` | `SpawnPipeline_T5_allocateActor_84B_invokeOnInit_ackVia_0x133` | named |  |  | `FUN_006c8cf0` |
| `0x002cb860` | `EntryLinkShellBuilder_dtor_revertsToParentVftable` | named |  |  | `FUN_006cb860` |
| `0x002cbc90` | `SpawnPipeline_T4_buildAndDispatchToAllocator` | dispatcher |  |  | `FUN_006cbc90` |
| `0x002cbfb0` | `EntryLinkShellBuilder_ctor_setsVftable_allocates0xf8Child` | named |  |  | `FUN_006cbfb0` |
| `0x002cc070` | `SpawnPipeline_FACTORY_dispatchByTypeTag_enqueueToRingBuffer` | dispatcher |  |  | `FUN_006cc070` |
| `0x002cc390` | `EntryLinkShellBuilder_FACTORY_constructAndEnqueue` | named |  |  | `FUN_006cc390` |
| `0x002cc5b0` | `SpawnPipeline_helper_copyPacket120B_setDiscriminator` | named |  |  | `FUN_006cc5b0` |
| `0x002cc620` | `SpawnPipeline_dispatcher_check2711tag_routeToFactory` | dispatcher |  |  | `FUN_006cc620` |
| `0x002cc720` | `LinkShellEntry_batchLoop_countAt_packet_0x200_stride_0x40` | named |  |  | `FUN_006cc720` |
| `0x002cd790` | `LinkShellEntry_forwarder_single_toFactory` | named |  |  | `FUN_006cd790` |
| `0x002cd7b0` | `LinkShellEntry_forwarder_batch_toFactory` | named |  |  | `FUN_006cd7b0` |
| `0x002cd8e0` | `SpawnPipeline_T2_orchestrate_listObject_emits_0x130_pair` | named |  |  | `FUN_006cd8e0` |
| `0x002cda80` | `SpawnPipeline_T1_ringBufferConsumer_castEntryBuilderBase` | named |  |  | `FUN_006cda80` |
| `0x002cdd20` | `SpawnPipeline_T0_perTickPump_processQueue` | named |  |  | `FUN_006cdd20` |
| `0x002cdf20` | `SpawnPipeline_perFrameWrapper_dispatchesT0` | dispatcher |  |  | `FUN_006cdf20` |
| `0x002cea20` | `WorkPath_joinAsString` | named |  |  | `FUN_006cea20` |
| `0x002ced30` | `GenericQueue_peekHead_sharedHelper` | named |  |  | `FUN_006ced30` |
| `0x002dacd0` | `Lua_listObjectDelete_sends_0x130_variantA` | lua_impl | 0x130 | _sendListObjectDelete | `Lua_listObjectDelete_sends_0x130_variantA` |
| `0x002dae90` | `Lua_listObjectQueueAdd_sends_0x130_variantA` | lua_impl | 0x130 | _sendListObjectQueueAdd | `Lua_listObjectQueueAdd_sends_0x130_variantA` |
| `0x002db9a0` | `SpawnPipeline_T3_dispatch2plusN_actorsList` | dispatcher |  |  | `FUN_006db9a0` |
| `0x002dbcb0` | `ActorBase_cpp_wait_thunk` | named |  |  | `FUN_006dbcb0` |
| `0x002dc040` | `WaitForTurningResumeChecker_ctor` | named |  |  | `FUN_006dc040` |
| `0x002dc0e0` | `WaitForCharaSchedulerFinishedResumeChecker_ctor` | named |  |  | `FUN_006dc0e0` |
| `0x002dc8b0` | `Debug_executeScriptFromCutSceneTable` | named |  |  | `Debug_executeScriptFromCutSceneTable` |
| `0x002dcc30` | `global_cpp_defineClass_thunk` | named |  |  | `FUN_006dcc30` |
| `0x002de510` | `PlayerBase_check_charaWork_state_via_binding_ids` | named |  |  | `FUN_006de510` |
| `0x002e1700` | `CharaBase_cpp_waitForTurning_thunk` | named |  |  | `FUN_006e1700` |
| `0x002e1b90` | `DesktopWidget_cpp_waitForItemSearchWidget_thunk` | named |  |  | `FUN_006e1b90` |
| `0x002e1c50` | `DesktopWidget_cpp_waitForCameraTutorial_thunk` | named |  |  | `FUN_006e1c50` |
| `0x002e2130` | `Lua_send8byteStateAt0x68_via_0x130_variantB` | lua_impl | 0x130 | _sendMovementState | `Lua_send8byteStateAt0x68_via_0x130_variantB` |
| `0x002e2af0` | `Lua_sendByteUshortAt0x68_via_0x132` | lua_impl | 0x132 | _sendCompoundState | `Lua_sendByteUshortAt0x68_via_0x132` |
| `0x002e2fb0` | `ServerNotify_send_via_0x12d_simple_128Bpayload` | named |  |  | `FUN_006e2fb0` |
| `0x002e42e0` | `Lua_listIndexSend_via_0x130_variantA` | lua_impl | 0x130 | _sendListIndex | `Lua_listIndexSend_via_0x130_variantA` |
| `0x002e4b40` | `CharaBase_cpp_waitForCharaSchedulerFinished_thunk` | named |  |  | `FUN_006e4b40` |
| `0x002e5710` | `DesktopWidget_cpp_waitForTargetTutorial_thunk` | named |  |  | `FUN_006e5710` |
| `0x002e5ad0` | `Lua_sendByteToggle_via_opcode_0x131` | lua_impl | 0x131 | _sendByteToggle | `Lua_sendByteToggle_via_opcode_0x131` |
| `0x002e6360` | `Lua_sendChallenge_via_opcode_0x134` | lua_impl | 0x134 | _sendChallenge | `Lua_sendChallenge_via_opcode_0x134` |
| `0x002e6c20` | `WorldMaster_cpp_waitForCharaSchedulerTutorialFinished_thunk` | named |  |  | `FUN_006e6c20` |
| `0x002e6d90` | `Lua_worldMaster__lookAtPlayerTutorial` | lua_impl |  | _lookAtPlayerTutorial | `Lua_worldMaster__lookAtPlayerTutorial` |
| `0x002e7670` | `CharaBase_cpp_updateWork_thunk` | named |  |  | `FUN_006e7670` |
| `0x002e8360` | `CommandUpdater_dispatchByTargetType` | dispatcher |  |  | `FUN_006e8360` |
| `0x002e85e0` | `lua_updateWork_impl` | lua_impl | 0x12f | _updateWork | `lua_updateWork_impl` |
| `0x002e8890` | `GroupBase_cpp_updateWork_thunk_customDispatch` | dispatcher |  |  | `FUN_006e8890` |
| `0x002e8b30` | `MyPlayer_callServerOnCommand_impl_vtable0xb4_send_suspend` | named |  |  | `FUN_006e8b30` |
| `0x002eced0` | `DesktopWidget_cpp_appendMessagePool_thunk` | named |  |  | `FUN_006eced0` |
| `0x002f0840` | `SpreadSheet_cpp_loadKeyTemporarily_thunk` | named |  |  | `FUN_006f0840` |
| `0x002f6900` | `Actor_eventHandler_onFinalize` | named |  |  | `FUN_006f6900` |
| `0x002f6a80` | `CharaBase_invokeLua_onInit` | inbound_invoker |  |  | `FUN_006f6a80` |
| `0x002f6d60` | `Actor_invokeLua_onInit` | inbound_invoker |  |  | `FUN_006f6d60` |
| `0x002f6ed0` | `Actor_invokeLua_onFinalize_v2` | inbound_invoker |  |  | `FUN_006f6ed0` |
| `0x002f7000` | `ItemBase_invokeLua_onInit` | inbound_invoker |  |  | `FUN_006f7000` |
| `0x002f71a0` | `ItemBase_invokeLua_onFinalize` | inbound_invoker |  |  | `FUN_006f71a0` |
| `0x002f73b0` | `Actor_invokeLua_onUpdateWork` | inbound_invoker |  |  | `FUN_006f73b0` |
| `0x002f7cd0` | `MyPlayer_dispatchCancelJobQuestComplete_3stage` | dispatcher |  |  | `FUN_006f7cd0` |
| `0x002f7fa0` | `MyPlayer_invokeLua_onJobQuestCompleteFirst_timeGated` | inbound_invoker |  |  | `FUN_006f7fa0` |
| `0x002f80d0` | `MyPlayer_invokeLua_onJobQuestCompleteSecond_timeGated` | inbound_invoker |  |  | `FUN_006f80d0` |
| `0x002f8200` | `MyPlayer_onMoveAtSit_eventHandler` | named |  |  | `FUN_006f8200` |
| `0x002f8350` | `Player_invokeLua_onGetGoobbue` | inbound_invoker |  |  | `FUN_006f8350` |
| `0x002f8470` | `Actor_queryLua_getBattalion` | named |  |  | `FUN_006f8470` |
| `0x002f8660` | `Actor_queryLua_isRetainer` | named |  |  | `FUN_006f8660` |
| `0x002fa840` | `Actor_dispatchLuaHook_onChangeActorMainStat` | dispatcher |  |  | `FUN_006fa840` |
| `0x002fa980` | `Actor_dispatchLuaHook_onChangeNetStatSystem` | dispatcher |  |  | `FUN_006fa980` |
| `0x002fae70` | `CharaBase_invokeLua_onChangeSystemFlag` | inbound_invoker |  |  | `CharaBase_invokeLua_onChangeSystemFlag` |
| `0x002faff0` | `CharaBase_invokeLua_onUpdateDisplayName_idChange` | inbound_invoker |  |  | `FUN_006faff0` |
| `0x002fb280` | `CharaBase_invokeLua_onUpdateDisplayName_nameChange` | inbound_invoker |  |  | `FUN_006fb280` |
| `0x002fb430` | `CharaBase_invokeLua_onChangeAccessibleInServer` | inbound_invoker |  |  | `FUN_006fb430` |
| `0x002fb9c0` | `CutScene_invokeLua_onFinalizeClip` | inbound_invoker |  |  | `CutScene_invokeLua_onFinalizeClip` |
| `0x002fbc50` | `CutScene_method_setActiveAndFinalize` | named |  |  | `CutScene_method_setActiveAndFinalize` |
| `0x002fbcc0` | `CutScene_invokeLua_onInitializationClip_PreviewSetupClip` | inbound_invoker |  |  | `CutScene_invokeLua_onInitializationClip_PreviewSetupClip` |
| `0x002fbe80` | `CutScene_invokeLua_onInitializationClip` | inbound_invoker |  |  | `CutScene_invokeLua_onInitializationClip` |
| `0x002fc080` | `CutScene_invokeLua_onShowUIClip` | inbound_invoker |  |  | `CutScene_invokeLua_onShowUIClip` |
| `0x002fc260` | `CutScene_invokeLua_onHideUIClip` | inbound_invoker |  |  | `CutScene_invokeLua_onHideUIClip` |
| `0x002fc3a0` | `CutScene_invokeLua_onShowWidgetClip` | inbound_invoker |  |  | `CutScene_invokeLua_onShowWidgetClip` |
| `0x002fc4d0` | `CutScene_invokeLua_onHideWidgetClip` | inbound_invoker |  |  | `CutScene_invokeLua_onHideWidgetClip` |
| `0x002fc5f0` | `CutScene_invokeLua_onOpenUIClip` | inbound_invoker |  |  | `CutScene_invokeLua_onOpenUIClip` |
| `0x002fe2a0` | `DesktopWidget_cpp_parseTextCommand_thunk` | named |  |  | `FUN_006fe2a0` |
| `0x002fe720` | `DesktopWidget_queryLua_checkTargetable_globalFn` | named |  |  | `FUN_006fe720` |
| `0x002fe960` | `DesktopWidget_invokeLua_onTargetChanged` | inbound_invoker |  |  | `DesktopWidget_invokeLua_onTargetChanged` |
| `0x002febb0` | `DesktopWidget_invokeLua_onTargetDecided` | inbound_invoker |  |  | `DesktopWidget_invokeLua_onTargetDecided` |
| `0x002fede0` | `DesktopWidget_invokeLua_onPreWarp` | inbound_invoker |  |  | `DesktopWidget_invokeLua_onPreWarp` |
| `0x002fef10` | `DesktopWidget_invokeLua_onPostWarp` | inbound_invoker |  |  | `DesktopWidget_invokeLua_onPostWarp` |
| `0x002ff040` | `DesktopWidget_invokeLua_onCreatedWidgetInWidgetContainer` | inbound_invoker |  |  | `FUN_006ff040` |
| `0x002ff1a0` | `global_canCreateActorByName_thunk_creatabilityCheck` | named |  |  | `FUN_006ff1a0` |
| `0x002ff210` | `global_isInstanceOf_thunk_dualDispatch_7rtti_plus_luaChain` | dispatcher |  |  | `FUN_006ff210` |
| `0x00300760` | `Group_invokeLua_onUpdateMemberInformation` | inbound_invoker |  |  | `FUN_00700760` |
| `0x003008b0` | `Group_invokeLua_onUpdateMember_idAndBool` | inbound_invoker |  |  | `FUN_007008b0` |
| `0x00300a10` | `Group_invokeLua_onUpdateMember_refAndBool` | inbound_invoker |  |  | `FUN_00700a10` |
| `0x00300b70` | `Group_invokeLua_onUpdateMember_nullAndBool` | inbound_invoker |  |  | `FUN_00700b70` |
| `0x00300cc0` | `Actor_invokeLua_onUpdateWork_withWorkRecord` | inbound_invoker |  |  | `FUN_00700cc0` |
| `0x00300e70` | `Group_invokeLua_onUpdateGroupCurrent` | inbound_invoker |  |  | `FUN_00700e70` |
| `0x00300ff0` | `Group_invokeLua_onUpdateGroupCurrent_simple` | inbound_invoker |  |  | `FUN_00700ff0` |
| `0x00302e30` | `Trade_invokeLua_onUpdateItemPackage` | inbound_invoker |  |  | `FUN_00702e30` |
| `0x003030d0` | `Trade_invokeLua_onUpdateTradingItem` | inbound_invoker |  |  | `FUN_007030d0` |
| `0x00303280` | `Trade_invokeLua_onUpdateTradingItem_withResolve` | inbound_invoker |  |  | `FUN_00703280` |
| `0x003037e0` | `MyPlayer_invokeLua_onJobQuestCompleteThird_timeGated` | inbound_invoker |  |  | `FUN_007037e0` |
| `0x00303970` | `Achievement_invokeLua_onReceiveAchievementId_loop` | inbound_invoker |  |  | `FUN_00703970` |
| `0x00303f60` | `MyPlayer_invokeLua_onChocoboRideEvents` | inbound_invoker |  |  | `FUN_00703f60` |
| `0x00304230` | `Player_handleLimitAddictedNotice` | named |  |  | `Player_handleLimitAddictedNotice` |
| `0x00304430` | `Achievement_invokeLua_onReceiveAchievementId_single` | inbound_invoker |  |  | `FUN_00704430` |
| `0x00304690` | `Achievement_invokeLua_onReceiveAchievementRate` | inbound_invoker |  |  | `FUN_00704690` |
| `0x00304820` | `Actor_handleActorMainStatChange` | named |  |  | `FUN_00704820` |
| `0x00305eb0` | `Lua_queryBinding_dispatchType_sends_0x135` | lua_impl | 0x135 | _queryBinding | `Lua_queryBinding_dispatchType_sends_0x135` |
| `0x00306dc0` | `MyPlayer_invokeLua_onChangeJob` | inbound_invoker |  |  | `FUN_00706dc0` |
| `0x00306f60` | `Actor_invokeLua_onReaction` | inbound_invoker |  |  | `FUN_00706f60` |
| `0x00307300` | `invokeLua_onLoadKeyAsync` | named |  |  | `FUN_00707300` |
| `0x00307610` | `invokeLua_onHoverHelp` | named |  |  | `FUN_00707610` |
| `0x00307d60` | `MyPlayer_invokeLua_onChangeSubStatStatus` | inbound_invoker |  |  | `FUN_00707d60` |
| `0x003084e0` | `MyPlayer_invokeLua_onChangeSubStatMode` | inbound_invoker |  |  | `FUN_007084e0` |
| `0x00308fc0` | `Command_invokeLua_onCommand_5strDispatch` | inbound_invoker |  |  | `FUN_00708fc0` |
| `0x00309640` | `global_cpp_createActor_thunk` | named |  |  | `FUN_00709640` |
| `0x00309ca0` | `Group_invokeLua_onUpdateGroupInformation` | inbound_invoker |  |  | `FUN_00709ca0` |
| `0x00309f00` | `Actor_callLua_executeTalk_directInvoke` | named |  |  | `FUN_00709f00` |
| `0x0030a010` | `MyPlayer_executeCommand_impl_vtable0xa8_validateTargets` | named |  |  | `FUN_0070a010` |
| `0x0030a350` | `MyPlayer_invokeLua_onLoginEvent_timeGated` | inbound_invoker |  |  | `FUN_0070a350` |
| `0x0030a580` | `invokeLua_onLoadMultiKeyAsync` | named |  |  | `FUN_0070a580` |
| `0x0030a720` | `SpreadSheet_cpp_getData_thunk` | named |  |  | `FUN_0070a720` |
| `0x0030aa10` | `WorkPath_construct_base` | named |  |  | `FUN_0070aa10` |
| `0x0030aaa0` | `WorkPath_construct_withFields` | named |  |  | `FUN_0070aaa0` |
| `0x003139c0` | `AppendMessageResumeChecker_ctor` | named |  |  | `FUN_007139c0` |
| `0x00313a50` | `TargetTutorialResumeChecker_ctor` | named |  |  | `FUN_00713a50` |
| `0x00313d50` | `ItemSearchWidgetResumeChecker_ctor` | named |  |  | `FUN_00713d50` |
| `0x00313e70` | `CameraTutorialResumeChecker_ctor` | named |  |  | `FUN_00713e70` |
| `0x00313f80` | `ScriptCoroutineKey_construct` | named |  |  | `FUN_00713f80` |
| `0x00313fe0` | `OnInitResumeChecker_ctor` | named |  |  | `FUN_00713fe0` |
| `0x003176c0` | `CharaIterate_ctor` | named |  |  | `FUN_007176c0` |
| `0x003177b0` | `WaitForCharaSchedulerTutorialFinishedResumeChecker_ctor` | named |  |  | `FUN_007177b0` |
| `0x0031d420` | `WorkPathTree_lowerBound` | named |  |  | `FUN_0071d420` |
| `0x0031db70` | `SpreadSheet_LoadDataFunctionEndCallback_ctor` | named |  |  | `FUN_0071db70` |
| `0x0031e760` | `Functor_MemberFunctionHolder_ctor` | functor |  |  | `FUN_0071e760` |
| `0x003238b0` | `ringBuffer_enqueue_4bytes` | named |  |  | `FUN_007238b0` |
| `0x00324800` | `Functor_entry_ctor` | functor |  |  | `FUN_00724800` |
| `0x00325a50` | `SpreadSheet_LoadDataResumeChecker_ctor` | named |  |  | `FUN_00725a50` |
| `0x00326720` | `Functor_pool_alloc` | functor |  |  | `FUN_00726720` |
| `0x003290e0` | `PlayerBase_registerLua_isTouching` | registrar |  |  | `FUN_007290e0` |
| `0x00329230` | `PlayerBase_registerLua_isAchievedTrophy` | registrar |  |  | `FUN_00729230` |
| `0x00329380` | `PlayerBase_registerLua_getOccupancyContentsTime` | registrar |  |  | `FUN_00729380` |
| `0x003294d0` | `PlayerBase_registerLua_getAchievementCategoryId` | registrar |  |  | `FUN_007294d0` |
| `0x00329620` | `PlayerBase_registerLua_countAchievementItem` | registrar |  |  | `FUN_00729620` |
| `0x00329770` | `PlayerBase_registerLua_getAchievementItemId` | registrar |  |  | `FUN_00729770` |
| `0x003298c0` | `PlayerBase_registerLua_isDoneAchievement` | registrar |  |  | `FUN_007298c0` |
| `0x00329a10` | `PlayerBase_registerLua_getAchievementPoint` | registrar |  |  | `FUN_00729a10` |
| `0x00329b60` | `PlayerBase_registerLua_getEnableAchievementTitle` | registrar |  |  | `FUN_00729b60` |
| `0x00329cb0` | `PlayerBase_registerLua_hasAchievementTitle` | registrar |  |  | `FUN_00729cb0` |
| `0x00329e00` | `PlayerBase_registerLua_hasAchievementItem` | registrar |  |  | `FUN_00729e00` |
| `0x00329f50` | `PlayerBase_registerLua_getAchievementSheetDataPoint` | registrar |  |  | `FUN_00729f50` |
| `0x0032a0a0` | `PlayerBase_registerLua_getAchievementSheetDataIcon` | registrar |  |  | `FUN_0072a0a0` |
| `0x0032a1f0` | `PlayerBase_registerLua_getAchievementSheetDataTitle` | registrar |  |  | `FUN_0072a1f0` |
| `0x0032a340` | `PlayerBase_registerLua_getAchievementSheetDataItem` | registrar |  |  | `FUN_0072a340` |
| `0x0032a490` | `PlayerBase_registerLua_getAchievementRate` | registrar |  |  | `FUN_0072a490` |
| `0x0032a5e0` | `PlayerBase_registerLua_countAchievementRateList` | registrar |  |  | `FUN_0072a5e0` |
| `0x0032a730` | `PlayerBase_registerLua_isDoneAchievementRateList` | registrar |  |  | `FUN_0072a730` |
| `0x0032a880` | `PlayerBase_registerLua_isCompletedCutSceneReplayQuest` | registrar |  |  | `FUN_0072a880` |
| `0x0032a9d0` | `PlayerBase_registerLua_canStoreItem` | registrar |  |  | `FUN_0072a9d0` |
| `0x0032ab20` | `PlayerBase_registerLua_countStoredItem` | registrar |  |  | `FUN_0072ab20` |
| `0x0032ac70` | `PlayerBase_registerLua_getStoredItem` | registrar |  |  | `FUN_0072ac70` |
| `0x0032adc0` | `PlayerBase_registerLua_getEnableEntrustItem` | registrar |  |  | `FUN_0072adc0` |
| `0x0032af10` | `PlayerBase_registerLua_getEntrustItem` | registrar |  |  | `FUN_0072af10` |
| `0x0032b0c0` | `Math_registerLua_randomInteger` | registrar |  |  | `FUN_0072b0c0` |
| `0x0032b440` | `Functor_NpcBaseClass_buildOutputStackOperatorVector` | functor |  |  | `Functor_NpcBaseClass_buildOutputStackOperatorVector` |
| `0x0032c7e0` | `Functor_NpcBaseClass_buildInputStackOperatorVector` | functor |  |  | `Functor_NpcBaseClass_buildInputStackOperatorVector` |
| `0x0032df60` | `ActorBaseClass_registerLua_callSuperClassFunction` | registrar |  |  | `FUN_0072df60` |
| `0x0032e0b0` | `ActorBaseClass_registerLua_wait` | registrar |  |  | `FUN_0072e0b0` |
| `0x0032e200` | `ActorBaseClass_registerLua_restrictYieldFunction_internal` | registrar |  |  | `FUN_0072e200` |
| `0x0032e350` | `ActorBaseClass_registerLua_loadTextDataPermanently` | registrar |  |  | `FUN_0072e350` |
| `0x0032e4a0` | `ActorBaseClass_registerLua_delete` | registrar |  |  | `FUN_0072e4a0` |
| `0x0032e5f0` | `AreaMaster_registerLua_setInstanceRaid` | registrar |  |  | `FUN_0072e5f0` |
| `0x0032e740` | `CharaBaseClass_registerLua_setDirection` | registrar |  |  | `FUN_0072e740` |
| `0x0032e890` | `CharaBaseClass_registerLua_setNameplateIcon` | registrar |  |  | `FUN_0072e890` |
| `0x0032e9e0` | `CharaBaseClass_registerLua_setNameplateGauge` | registrar |  |  | `FUN_0072e9e0` |
| `0x0032eb30` | `CharaBaseClass_registerLua_setNameplateVisible` | registrar |  |  | `FUN_0072eb30` |
| `0x0032ec80` | `CharaBaseClass_registerLua_setFloatingOffset` | registrar |  |  | `FUN_0072ec80` |
| `0x0032edd0` | `CharaBaseClass_registerLua_setVisible` | registrar |  |  | `FUN_0072edd0` |
| `0x0032ef20` | `CharaBaseClass_registerLua_setGroundOn` | registrar |  |  | `FUN_0072ef20` |
| `0x0032f070` | `CharaBaseClass_registerLua_setMapMarker` | registrar |  |  | `FUN_0072f070` |
| `0x0032f1c0` | `CharaBaseClass_registerLua_updateGroup` | registrar |  |  | `FUN_0072f1c0` |
| `0x0032f310` | `CharaBaseClass_registerLua_getGroupCurrent` | registrar |  |  | `FUN_0072f310` |
| `0x0032f460` | `CharaBaseClass_registerLua_getExtendedTemporaryGroupCurrent` | registrar |  |  | `FUN_0072f460` |
| `0x0032f5b0` | `CharaBaseClass_registerLua_lookAtCharacter` | registrar |  |  | `FUN_0072f5b0` |
| `0x0032f700` | `CharaBaseClass_registerLua_cancelLookAt` | registrar |  |  | `FUN_0072f700` |
| `0x0032f850` | `CharaBaseClass_registerLua_runCharaScheduler` | registrar |  |  | `FUN_0072f850` |
| `0x0032f9a0` | `CharaBaseClass_registerLua_runCharaSchedulerFromMidstream_internal` | registrar |  |  | `FUN_0072f9a0` |
| `0x0032faf0` | `CharaBaseClass_registerLua_runCharaSchedulerAgainstTarget` | registrar |  |  | `FUN_0072faf0` |
| `0x0032fc40` | `CharaBaseClass_registerLua_waitForCharaSchedulerFinished` | registrar |  |  | `FUN_0072fc40` |
| `0x0032fd90` | `CharaBaseClass_registerLua_turnDir_internal` | registrar |  |  | `FUN_0072fd90` |
| `0x0032fee0` | `CharaBaseClass_registerLua_turnClientDir` | registrar |  |  | `FUN_0072fee0` |
| `0x00330030` | `CharaBaseClass_registerLua_turnBack` | registrar |  |  | `FUN_00730030` |
| `0x00330180` | `CharaBaseClass_registerLua_waitForTurning` | registrar |  |  | `FUN_00730180` |
| `0x003302d0` | `CharaBaseClass_registerLua_updateItemPackage` | registrar |  |  | `FUN_007302d0` |
| `0x00330420` | `CharaBaseClass_registerLua_transformIntoChocobo_internal` | registrar |  |  | `FUN_00730420` |
| `0x00330570` | `CharaBaseClass_registerLua_getJob` | registrar |  |  | `FUN_00730570` |
| `0x003306c0` | `GroupBaseClass_registerLua_updateMemberAndInformation` | registrar |  |  | `FUN_007306c0` |
| `0x00330810` | `ItemBaseClass_registerLua_updateWork` | registrar |  |  | `FUN_00730810` |
| `0x00330960` | `PlayerBase_registerLua_executeTalk` | registrar |  | _executeTalk | `PlayerBase_registerLua_executeTalk` |
| `0x00330ab0` | `PlayerBase_registerLua_executeEmote` | registrar |  | _executeEmote | `PlayerBase_registerLua_executeEmote` |
| `0x00330c00` | `PlayerBase_registerLua_canExecuteCommand` | registrar |  | _canExecuteCommand | `PlayerBase_registerLua_canExecuteCommand` |
| `0x00330d50` | `PlayerBase_registerLua_canExecuteTalk` | registrar |  | _canExecuteTalk | `PlayerBase_registerLua_canExecuteTalk` |
| `0x00330ea0` | `PlayerBase_registerLua_canExecuteEmote` | registrar |  | _canExecuteEmote | `PlayerBase_registerLua_canExecuteEmote` |
| `0x00330ff0` | `PlayerBase_registerLua_cancelCommand` | registrar |  | _cancelCommand | `PlayerBase_registerLua_cancelCommand` |
| `0x00331140` | `PlayerBase_registerLua_cancelTalk` | registrar |  | _cancelTalk | `PlayerBase_registerLua_cancelTalk` |
| `0x00331290` | `PlayerBase_registerLua_cancelNotice` | registrar |  | _cancelNotice | `PlayerBase_registerLua_cancelNotice` |
| `0x003313e0` | `PlayerBase_registerLua_cancelEmote` | registrar |  | _cancelEmote | `PlayerBase_registerLua_cancelEmote` |
| `0x00331530` | `PlayerBase_registerLua_cancelPush` | registrar |  | _cancelPush | `PlayerBase_registerLua_cancelPush` |
| `0x00331680` | `PlayerBase_registerLua_breakCommand` | registrar |  | _breakCommand | `PlayerBase_registerLua_breakCommand` |
| `0x003317d0` | `PlayerBase_registerLua_isEventPlaying` | registrar |  | _isEventPlaying | `PlayerBase_registerLua_isEventPlaying` |
| `0x00331920` | `PlayerBase_registerLua_isCommandPlaying` | registrar |  | _isCommandPlaying | `PlayerBase_registerLua_isCommandPlaying` |
| `0x00331a70` | `PlayerBase_registerLua_countCommandPlaying` | registrar |  | _countCommandPlaying | `PlayerBase_registerLua_countCommandPlaying` |
| `0x00331bc0` | `PlayerBase_registerLua_fadeIn` | registrar |  | _fadeIn | `PlayerBase_registerLua_fadeIn` |
| `0x00331d10` | `PlayerBase_registerLua_fadeOut` | registrar |  | _fadeOut | `PlayerBase_registerLua_fadeOut` |
| `0x00331e60` | `PlayerBase_registerLua_waitForFading` | registrar |  | _waitForFading | `PlayerBase_registerLua_waitForFading` |
| `0x00331fb0` | `PlayerBase_registerLua_isFading` | registrar |  | _isFading | `PlayerBase_registerLua_isFading` |
| `0x00332100` | `PlayerBase_registerLua_cancelFading` | registrar |  | _cancelFading | `PlayerBase_registerLua_cancelFading` |
| `0x00332250` | `PlayerBase_registerLua_fadeInAfterWarp` | registrar |  | _fadeInAfterWarp | `PlayerBase_registerLua_fadeInAfterWarp` |
| `0x003323a0` | `PlayerBase_registerLua_resetFade` | registrar |  | _resetFade | `PlayerBase_registerLua_resetFade` |
| `0x003324f0` | `PlayerBase_registerLua_fadeInNowLoadingForNoticeEventJustInArea` | registrar |  | _fadeInNowLoadingForNoticeEventJustInArea | `PlayerBase_registerLua_fadeInNowLoadingForNoticeEventJustInArea` |
| `0x00332640` | `PlayerBase_registerLua_lockPlayerControl` | registrar |  | _lockPlayerControl | `PlayerBase_registerLua_lockPlayerControl` |
| `0x00332790` | `PlayerBase_registerLua_unlockPlayerControl` | registrar |  | _unlockPlayerControl | `PlayerBase_registerLua_unlockPlayerControl` |
| `0x003328e0` | `PlayerBase_registerLua_isPlayerControlEnabled` | registrar |  | _isPlayerControlEnabled | `PlayerBase_registerLua_isPlayerControlEnabled` |
| `0x00332a30` | `PlayerBase_registerLua_lockLockonControl` | registrar |  | _lockLockonControl | `PlayerBase_registerLua_lockLockonControl` |
| `0x00332b80` | `PlayerBase_registerLua_unlockLockonControl` | registrar |  | _unlockLockonControl | `PlayerBase_registerLua_unlockLockonControl` |
| `0x00332cd0` | `PlayerBase_registerLua_isLockonControlEnabled` | registrar |  | _isLockonControlEnabled | `PlayerBase_registerLua_isLockonControlEnabled` |
| `0x00332e20` | `PlayerBase_registerLua_lockCameraControl` | registrar |  | _lockCameraControl | `PlayerBase_registerLua_lockCameraControl` |
| `0x00332f70` | `PlayerBase_registerLua_unlockCameraControl` | registrar |  | _unlockCameraControl | `PlayerBase_registerLua_unlockCameraControl` |
| `0x003330c0` | `PlayerBase_registerLua_isCameraControlEnabled` | registrar |  | _isCameraControlEnabled | `PlayerBase_registerLua_isCameraControlEnabled` |
| `0x00333210` | `PlayerBase_registerLua_setLockonTarget` | registrar |  | _setLockonTarget | `PlayerBase_registerLua_setLockonTarget` |
| `0x00333360` | `PlayerBase_registerLua_getLockonTarget` | registrar |  | _getLockonTarget | `PlayerBase_registerLua_getLockonTarget` |
| `0x003334b0` | `PlayerBase_registerLua_waitForMapLoaded` | registrar |  | _waitForMapLoaded | `PlayerBase_registerLua_waitForMapLoaded` |
| `0x00333600` | `PlayerBase_registerLua_setMusic` | registrar |  | _setMusic | `PlayerBase_registerLua_setMusic` |
| `0x00333750` | `PlayerBase_registerLua_setWeather` | registrar |  | _setWeather | `PlayerBase_registerLua_setWeather` |
| `0x003338a0` | `PlayerBase_registerLua_setTouchAttribute` | registrar |  |  | `FUN_007338a0` |
| `0x003339f0` | `PlayerBase_registerLua_canGetTrophy` | registrar |  |  | `FUN_007339f0` |
| `0x00333b40` | `PlayerBase_registerLua_achieveTrophy` | registrar |  |  | `FUN_00733b40` |
| `0x00333c90` | `PlayerBase_registerLua_turn` | registrar |  |  | `FUN_00733c90` |
| `0x00333de0` | `PlayerBase_registerLua_getNormalBehestTime` | registrar |  |  | `FUN_00733de0` |
| `0x00333f30` | `PlayerBase_registerLua_getCompanyBehestTime` | registrar |  |  | `FUN_00733f30` |
| `0x00334080` | `PlayerBase_registerLua_getWarpRecastTime` | registrar |  |  | `FUN_00734080` |
| `0x003341d0` | `PlayerBase_registerLua_getChocoboGrade` | registrar |  |  | `FUN_007341d0` |
| `0x00334320` | `PlayerBase_registerLua_getChocoboRidingGrade` | registrar |  |  | `FUN_00734320` |
| `0x00334470` | `PlayerBase_registerLua_isEnabledGoobbue` | registrar |  |  | `FUN_00734470` |
| `0x003345c0` | `PlayerBase_registerLua_isPushingOut` | registrar |  |  | `FUN_007345c0` |
| `0x00334710` | `PlayerBase_registerLua_haveEnmityCharacters` | registrar |  |  | `FUN_00734710` |
| `0x00334860` | `PlayerBase_registerLua_countAchievementCategory` | registrar |  |  | `FUN_00734860` |
| `0x003349b0` | `PlayerBase_registerLua_setAchievementTitle` | registrar |  |  | `FUN_007349b0` |
| `0x00334b00` | `PlayerBase_registerLua_countEnableAchievementTitle` | registrar |  |  | `FUN_00734b00` |
| `0x00334c50` | `PlayerBase_registerLua_clearAchievementRateCache` | registrar |  |  | `FUN_00734c50` |
| `0x00334da0` | `PlayerBase_registerLua_getAchievementRateList` | registrar |  |  | `FUN_00734da0` |
| `0x00334ef0` | `PlayerBase_registerLua_getCutSceneReplaySnpcNickname` | registrar |  |  | `FUN_00734ef0` |
| `0x00335040` | `PlayerBase_registerLua_getCutSceneReplaySnpcCoordinate` | registrar |  |  | `FUN_00735040` |
| `0x00335190` | `PlayerBase_registerLua_getCutSceneReplaySnpcSkin` | registrar |  |  | `FUN_00735190` |
| `0x003352e0` | `PlayerBase_registerLua_getCutSceneReplaySnpcPersonality` | registrar |  |  | `FUN_007352e0` |
| `0x00335430` | `PlayerBase_registerLua_cancelJobQuestCompleteTriple` | registrar |  |  | `FUN_00735430` |
| `0x00335580` | `PlayerBase_registerLua_countEnableEntrustItem` | registrar |  |  | `FUN_00735580` |
| `0x003356d0` | `PlayerBase_registerLua_countEntrustItem` | registrar |  |  | `FUN_007356d0` |
| `0x00335820` | `PlayerBase_registerLua_readyInnBed` | registrar |  |  | `FUN_00735820` |
| `0x00335970` | `PlayerBase_registerLua_forceCameraTPSMode` | registrar |  |  | `FUN_00735970` |
| `0x00335ac0` | `PlayerBase_registerLua_countHamletDefenseScore` | registrar |  |  | `FUN_00735ac0` |
| `0x00335c10` | `PlayerBase_registerLua_getHamletDefenseScore` | registrar |  |  | `FUN_00735c10` |
| `0x00335d60` | `PlayerBase_registerLua_getNMRushUpdateTime` | registrar |  |  | `FUN_00735d60` |
| `0x00335eb0` | `Sequence_registerLua_setFilename` | registrar |  |  | `FUN_00735eb0` |
| `0x00336000` | `Sequence_registerLua_loadCutScene_internal` | registrar |  |  | `FUN_00736000` |
| `0x00336150` | `Sequence_registerLua_replay` | registrar |  |  | `FUN_00736150` |
| `0x003362a0` | `Sequence_registerLua_skip` | registrar |  |  | `FUN_007362a0` |
| `0x003363f0` | `Debug_registerLua_printLog_internal` | registrar |  |  | `FUN_007363f0` |
| `0x00336540` | `Debug_registerLua_printWarning_internal` | registrar |  |  | `FUN_00736540` |
| `0x00336690` | `Debug_registerLua_getClassName` | registrar |  |  | `FUN_00736690` |
| `0x003367e0` | `Debug_registerLua_getInstanceName_internal` | registrar |  |  | `FUN_007367e0` |
| `0x00336930` | `Debug_registerLua_getAllCharacter` | registrar |  |  | `FUN_00736930` |
| `0x00336a80` | `Debug_registerLua_getLowResolutionTime_internal` | registrar |  |  | `FUN_00736a80` |
| `0x00336bd0` | `Debug_registerLua_getTimeCost_internal` | registrar |  |  | `FUN_00736bd0` |
| `0x00336d20` | `Debug_registerLua_deleteQuestActorForPreview_internal` | registrar |  |  | `FUN_00736d20` |
| `0x00336e70` | `Debug_registerLua_copyClipboard_internal` | registrar |  |  | `FUN_00736e70` |
| `0x00336fc0` | `NpcBaseClass_registerLua_callServerOnTalk` | registrar |  | _callServerOnTalk | `NpcBaseClass_registerLua_callServerOnTalk` |
| `0x00337110` | `NpcBaseClass_registerLua_callServerOnEmote` | registrar |  | _callServerOnEmote | `NpcBaseClass_registerLua_callServerOnEmote` |
| `0x00337260` | `NpcBaseClass_registerLua_callServerOnPush` | registrar |  | _callServerOnPush | `NpcBaseClass_registerLua_callServerOnPush` |
| `0x003373b0` | `NpcBaseClass_registerLua_doServerOnTalk` | registrar |  |  | `FUN_007373b0` |
| `0x00337500` | `NpcBaseClass_registerLua_doServerOnEmote` | registrar |  |  | `FUN_00737500` |
| `0x00337650` | `NpcBaseClass_registerLua_doServerOnPush` | registrar |  |  | `FUN_00737650` |
| `0x003377a0` | `NpcBaseClass_registerLua_breakTalk` | registrar |  |  | `FUN_007377a0` |
| `0x003378f0` | `NpcBaseClass_registerLua_breakEmote` | registrar |  |  | `FUN_007378f0` |
| `0x00337a40` | `NpcBaseClass_registerLua_breakPush` | registrar |  |  | `FUN_00737a40` |
| `0x00337b90` | `NpcBaseClass_registerLua_breakNotice` | registrar |  |  | `FUN_00737b90` |
| `0x00337ce0` | `NpcBaseClass_registerLua_initAsMapObj` | registrar |  |  | `FUN_00737ce0` |
| `0x00337e30` | `NpcBaseClass_registerLua_runBgScheduler` | registrar |  |  | `FUN_00737e30` |
| `0x00337f80` | `NpcBaseClass_registerLua_runBgSchedulerFromMidstream` | registrar |  |  | `FUN_00737f80` |
| `0x003380d0` | `NpcBaseClass_registerLua_waitForBgSchedulerFinished` | registrar |  |  | `FUN_007380d0` |
| `0x00338220` | `WidgetBaseClass_registerLua_setFilename` | registrar |  |  | `FUN_00738220` |
| `0x00338370` | `WidgetBaseClass_registerLua_loadForm` | registrar |  |  | `FUN_00738370` |
| `0x003384c0` | `WidgetBaseClass_registerLua_removeItem` | registrar |  |  | `FUN_007384c0` |
| `0x00338610` | `WidgetBaseClass_registerLua_isKeyboardFocused` | registrar |  |  | `FUN_00738610` |
| `0x00338760` | `WidgetBaseClass_registerLua_getKeyboardFocusedControl` | registrar |  |  | `FUN_00738760` |
| `0x003388b0` | `WidgetBaseClass_registerLua_setKeyboardFocusedControl` | registrar |  |  | `FUN_007388b0` |
| `0x00338a00` | `WidgetBaseClass_registerLua_setParentWidget` | registrar |  |  | `FUN_00738a00` |
| `0x00338b50` | `WidgetBaseClass_registerLua_addList` | registrar |  |  | `FUN_00738b50` |
| `0x00338ca0` | `WidgetBaseClass_registerLua_removeList` | registrar |  |  | `FUN_00738ca0` |
| `0x00338df0` | `WidgetBaseClass_registerLua_clearAllList` | registrar |  |  | `FUN_00738df0` |
| `0x00338f40` | `WidgetBaseClass_registerLua_updateList` | registrar |  |  | `FUN_00738f40` |
| `0x00339090` | `DesktopWidget_registerLua_clearLogPool` | registrar |  |  | `FUN_00739090` |
| `0x003391e0` | `DesktopWidget_registerLua_initTargetCursors` | registrar |  |  | `FUN_007391e0` |
| `0x00339330` | `DesktopWidget_registerLua_setTargetCursorImage` | registrar |  |  | `FUN_00739330` |
| `0x00339480` | `DesktopWidget_registerLua_setTargetableDistance` | registrar |  |  | `FUN_00739480` |
| `0x003395d0` | `DesktopWidget_registerLua_setCurrentTargetCursor` | registrar |  |  | `FUN_007395d0` |
| `0x00339720` | `DesktopWidget_registerLua_setAllTargetCursorMask` | registrar |  |  | `FUN_00739720` |
| `0x00339870` | `DesktopWidget_registerLua_setTargetCharacter` | registrar |  |  | `FUN_00739870` |
| `0x003399c0` | `DesktopWidget_registerLua_setTargetCharacterByDisplayName` | registrar |  |  | `FUN_007399c0` |
| `0x00339b10` | `DesktopWidget_registerLua_setTargetNearestCharacter` | registrar |  |  | `FUN_00739b10` |
| `0x00339c60` | `DesktopWidget_registerLua_lockTargetCursorControl` | registrar |  |  | `FUN_00739c60` |
| `0x00339db0` | `DesktopWidget_registerLua_unlockTargetCursorControl` | registrar |  |  | `FUN_00739db0` |
| `0x00339f00` | `DesktopWidget_registerLua_setLockonCursorImage` | registrar |  |  | `FUN_00739f00` |
| `0x0033a050` | `DesktopWidget_registerLua_resetUserConfig` | registrar |  |  | `FUN_0073a050` |
| `0x0033a1a0` | `DesktopWidget_registerLua_saveUserConfig` | registrar |  |  | `FUN_0073a1a0` |
| `0x0033a2f0` | `DesktopWidget_registerLua_setUserMacroIcon` | registrar |  |  | `FUN_0073a2f0` |
| `0x0033a440` | `DesktopWidget_registerLua_saveUserMacro` | registrar |  |  | `FUN_0073a440` |
| `0x0033a590` | `DesktopWidget_registerLua_waitForItemSearchWidget` | registrar |  |  | `FUN_0073a590` |
| `0x0033a6e0` | `DesktopWidget_registerLua_waitForTargetTutorial` | registrar |  |  | `FUN_0073a6e0` |
| `0x0033a830` | `DesktopWidget_registerLua_waitForCameraTutorial` | registrar |  |  | `FUN_0073a830` |
| `0x0033a980` | `DesktopWidget_registerLua_reserveWidgetContainer` | registrar |  |  | `FUN_0073a980` |
| `0x0033aad0` | `DesktopWidget_registerLua_deleteCreatingWidgetInWidgetContainer` | registrar |  |  | `FUN_0073aad0` |
| `0x0033ac20` | `DesktopWidget_registerLua_sendCountDown` | registrar |  |  | `FUN_0073ac20` |
| `0x0033ad70` | `WorldMaster_registerLua_printLog` | registrar |  |  | `FUN_0073ad70` |
| `0x0033aec0` | `WorldMaster_registerLua_printDebugLog` | registrar |  |  | `FUN_0073aec0` |
| `0x0033b010` | `WorldMaster_registerLua_loadWord` | registrar |  |  | `FUN_0073b010` |
| `0x0033b160` | `WorldMaster_registerLua_unloadWord` | registrar |  |  | `FUN_0073b160` |
| `0x0033b2b0` | `WorldMaster_registerLua_runCharaSchedulerTutorial` | registrar |  |  | `FUN_0073b2b0` |
| `0x0033b400` | `WorldMaster_registerLua_waitForCharaSchedulerTutorialFinished` | registrar |  |  | `FUN_0073b400` |
| `0x0033b550` | `Register_lookAtPlayerTutorial_LuaBinding` | named |  |  | `FUN_0073b550` |
| `0x0033b6a0` | `WorldMaster_registerLua_cancelLookAtPlayerTutorial` | registrar |  |  | `FUN_0073b6a0` |
| `0x0033b7f0` | `WorldMaster_registerLua_aimCameraTutorial` | registrar |  |  | `FUN_0073b7f0` |
| `0x0033b940` | `WorldMaster_registerLua_cancelAimCameraTutorial` | registrar |  |  | `FUN_0073b940` |
| `0x0033ba90` | `WorldMaster_registerLua_transformIntoChocobo` | registrar |  |  | `FUN_0073ba90` |
| `0x0033bbe0` | `WorldMaster_registerLua_cancelTransformIntoChocobo` | registrar |  |  | `FUN_0073bbe0` |
| `0x0033bd30` | `WorldMaster_registerLua_aimCameraChocobo` | registrar |  |  | `FUN_0073bd30` |
| `0x0033be80` | `WorldMaster_registerLua_cancelAimCameraChocobo` | registrar |  |  | `FUN_0073be80` |
| `0x0033bfd0` | `DirectorBaseClass_registerLua_breakNotice` | registrar |  |  | `FUN_0073bfd0` |
| `0x0033c120` | `DirectorBaseClass_registerLua_waitForHamletDefenseScore` | registrar |  |  | `FUN_0073c120` |
| `0x0033c270` | `global_registerLua_defineClass` | registrar |  |  | `FUN_0073c270` |
| `0x0033c3c0` | `global_registerLua_defineBaseClass` | registrar |  |  | `FUN_0073c3c0` |
| `0x0033c510` | `global_registerLua_getActorByName` | registrar |  |  | `FUN_0073c510` |
| `0x0033c660` | `global_registerLua_isExistActor` | registrar |  |  | `FUN_0073c660` |
| `0x0033c7b0` | `global_registerLua_prepareAllCommandStaticActor` | registrar |  |  | `FUN_0073c7b0` |
| `0x0033c900` | `global_registerLua_getUTF8StringLength` | registrar |  |  | `FUN_0073c900` |
| `0x0033ca50` | `global_registerLua_getUTF8StringByteLength` | registrar |  |  | `FUN_0073ca50` |
| `0x0033cba0` | `Math_registerLua_randomFloat` | registrar |  |  | `FUN_0073cba0` |
| `0x0033ccf0` | `Math_registerLua_randomIntegerWithSeed` | registrar |  |  | `FUN_0073ccf0` |
| `0x0033e9f0` | `CharaBaseClass_registerLua_setPosition` | registrar |  |  | `FUN_0073e9f0` |
| `0x0033eb40` | `CharaBaseClass_registerLua_updateWork` | registrar |  |  | `FUN_0073eb40` |
| `0x0033ec90` | `CharaBaseClass_registerLua_lookAtCharacterEid` | registrar |  |  | `FUN_0073ec90` |
| `0x0033ede0` | `CharaBaseClass_registerLua_lookAtDirection` | registrar |  |  | `FUN_0073ede0` |
| `0x0033ef30` | `GroupBaseClass_registerLua_updateWork` | registrar |  |  | `FUN_0073ef30` |
| `0x0033f080` | `PlayerBase_registerLua_executeCommand` | registrar |  | _executeCommand | `PlayerBase_registerLua_executeCommand` |
| `0x0033f1d0` | `PlayerBase_registerLua_callServerOnCommand` | registrar |  | _callServerOnCommand | `PlayerBase_registerLua_callServerOnCommand` |
| `0x0033f320` | `PlayerBase_registerLua_doServerOnCommand` | registrar |  | _doServerOnCommand | `PlayerBase_registerLua_doServerOnCommand` |
| `0x0033f470` | `Debug_registerLua_getText_internal` | registrar |  |  | `FUN_0073f470` |
| `0x0033f5c0` | `Debug_registerLua_printText_internal` | registrar |  |  | `FUN_0073f5c0` |
| `0x0033f710` | `NpcBaseClass_registerLua_setMapObjScale` | registrar |  |  | `FUN_0073f710` |
| `0x0033f860` | `NpcBaseClass_registerLua_preloadItemSpreadSheetContainer` | registrar |  |  | `FUN_0073f860` |
| `0x0033f9b0` | `WidgetBaseClass_registerLua_setUICommandCondition` | registrar |  |  | `FUN_0073f9b0` |
| `0x0033fb00` | `DesktopWidget_registerLua_setUserMacroTitle` | registrar |  |  | `FUN_0073fb00` |
| `0x0033fc50` | `DirectorBaseClass_registerLua_updateWork` | registrar |  |  | `FUN_0073fc50` |
| `0x0033fda0` | `Math_registerLua_randomFloatWithSeed` | registrar |  |  | `FUN_0073fda0` |
| `0x00340ec0` | `Math_registerAllLuaBindings` | registrar |  |  | `FUN_00740ec0` |
| `0x00340fb0` | `CharaBaseClass_registerLua_setNameplateColor` | registrar |  |  | `FUN_00740fb0` |
| `0x00341100` | `CharaBaseClass_registerLua_lookAtPosition` | registrar |  |  | `FUN_00741100` |
| `0x00341250` | `PlayerBase_registerLua_setPositionDirectionInn` | registrar |  |  | `FUN_00741250` |
| `0x003413a0` | `PlayerBase_registerLua_getHamletDefenseScoreAll` | registrar |  |  | `FUN_007413a0` |
| `0x003414f0` | `WidgetBaseClass_registerLua_setUICommandTemplateCondition` | registrar |  |  | `FUN_007414f0` |
| `0x00341640` | `WidgetBaseClass_registerLua_setProperty` | registrar |  |  | `FUN_00741640` |
| `0x00341790` | `WidgetBaseClass_registerLua_addItem` | registrar |  |  | `FUN_00741790` |
| `0x003418e0` | `DesktopWidget_registerLua_setUserMacroData` | registrar |  |  | `FUN_007418e0` |
| `0x00341a30` | `global_registerLua_getStaticActor` | registrar |  |  | `FUN_00741a30` |
| `0x00341b80` | `global_registerLua_isExistStaticActor` | registrar |  |  | `FUN_00741b80` |
| `0x00342760` | `ActorBaseClass_registerLua_setLoopInterval` | registrar |  |  | `FUN_00742760` |
| `0x003428b0` | `NpcBaseClass_registerLua_setReactionTriggerBox` | registrar |  |  | `FUN_007428b0` |
| `0x00342e50` | `DesktopWidget_registerLua_setUserConfig` | registrar |  |  | `FUN_00742e50` |
| `0x00343550` | `WidgetBaseClass_registerLua_sendStoryboardCommand` | registrar |  |  | `FUN_00743550` |
| `0x003436a0` | `WidgetBaseClass_registerLua_getListProperty` | registrar |  |  | `FUN_007436a0` |
| `0x00343c30` | `ItemBaseClass_registerLua_bindSpreadSheetData` | registrar |  |  | `FUN_00743c30` |
| `0x00343d80` | `PlayerBase_registerLua_chat` | registrar |  |  | `FUN_00743d80` |
| `0x00343ed0` | `DesktopWidget_registerLua_appendMessagePool` | registrar |  |  | `FUN_00743ed0` |
| `0x00344020` | `DesktopWidget_registerLua_appendLogPool` | registrar |  |  | `FUN_00744020` |
| `0x00344460` | `WidgetBaseClass_registerLua_setListProperty` | registrar |  |  | `FUN_00744460` |
| `0x00344990` | `WidgetBaseClass_registerLua_getProperty` | registrar |  |  | `FUN_00744990` |
| `0x00345230` | `CharaBaseClass_registerLua_setNameplate` | registrar |  |  | `FUN_00745230` |
| `0x00345ff0` | `WidgetBaseClass_registerLua_setTextProperty` | registrar |  |  | `FUN_00745ff0` |
| `0x00346140` | `WidgetBaseClass_registerLua_setListTextProperty` | registrar |  |  | `FUN_00746140` |
| `0x00346510` | `NpcBaseClass_registerLua_releaseItemSpreadSheetContainer` | registrar |  |  | `FUN_00746510` |
| `0x00346e70` | `SpreadSheet_registerLua_setFilename` | registrar |  |  | `FUN_00746e70` |
| `0x00346fc0` | `SpreadSheet_registerLua_loadKeyTemporarily` | registrar |  |  | `FUN_00746fc0` |
| `0x00347110` | `SpreadSheet_registerLua_loadKeySemipermanently` | registrar |  |  | `FUN_00747110` |
| `0x00347260` | `SpreadSheet_registerLua_loadAllKeyPermanently` | registrar |  |  | `FUN_00747260` |
| `0x003473b0` | `SpreadSheet_registerLua_loadKeyAsync` | registrar |  |  | `FUN_007473b0` |
| `0x00347600` | `SpreadSheet_registerLua_loadMultiKeyAsync` | registrar |  |  | `FUN_00747600` |
| `0x00347990` | `SpreadSheet_registerLua_unloadKey` | registrar |  |  | `FUN_00747990` |
| `0x003497a0` | `ActorBaseClass_registerLua_getCurrentAreaMaster` | registrar |  |  | `FUN_007497a0` |
| `0x003498f0` | `ActorBaseClass_registerLua_getStaticActorID` | registrar |  |  | `FUN_007498f0` |
| `0x00349a40` | `AreaMaster_registerLua_getRegion` | registrar |  |  | `FUN_00749a40` |
| `0x00349b90` | `AreaMaster_registerLua_getZoneName` | registrar |  |  | `FUN_00749b90` |
| `0x00349ce0` | `AreaMaster_registerLua_canRideChocobo` | registrar |  |  | `FUN_00749ce0` |
| `0x00349e30` | `AreaMaster_registerLua_isWarpRideChocobo` | registrar |  |  | `FUN_00749e30` |
| `0x00349f80` | `AreaMaster_registerLua_canStealth` | registrar |  |  | `FUN_00749f80` |
| `0x0034a0d0` | `AreaMaster_registerLua_isInn` | registrar |  |  | `FUN_0074a0d0` |
| `0x0034a220` | `AreaMaster_registerLua_countHamletSupplyRanking` | registrar |  |  | `FUN_0074a220` |
| `0x0034a370` | `AreaMaster_registerLua_getHamletSupplyRanking` | registrar |  |  | `FUN_0074a370` |
| `0x0034a4c0` | `CharaBaseClass_registerLua_getPosition` | registrar |  |  | `FUN_0074a4c0` |
| `0x0034a610` | `CharaBaseClass_registerLua_getDirection` | registrar |  |  | `FUN_0074a610` |
| `0x0034a760` | `CharaBaseClass_registerLua_getOrientation` | registrar |  |  | `FUN_0074a760` |
| `0x0034a8b0` | `CharaBaseClass_registerLua_getGear` | registrar |  |  | `FUN_0074a8b0` |
| `0x0034aa00` | `CharaBaseClass_registerLua_getActorMainStat` | registrar |  |  | `FUN_0074aa00` |
| `0x0034ab50` | `CharaBaseClass_registerLua_getSubStatWaste` | registrar |  |  | `FUN_0074ab50` |
| `0x0034aca0` | `CharaBaseClass_registerLua_getSubStatGuard` | registrar |  |  | `FUN_0074aca0` |
| `0x0034adf0` | `CharaBaseClass_registerLua_getSubStatChant` | registrar |  |  | `FUN_0074adf0` |
| `0x0034af40` | `CharaBaseClass_registerLua_getSubStatObject` | registrar |  |  | `FUN_0074af40` |
| `0x0034b090` | `CharaBaseClass_registerLua_getSubStatBreakage` | registrar |  |  | `FUN_0074b090` |
| `0x0034b1e0` | `CharaBaseClass_registerLua_getSubStatMotionPack` | registrar |  |  | `FUN_0074b1e0` |
| `0x0034b330` | `CharaBaseClass_registerLua_getSubStatStatus` | registrar |  |  | `FUN_0074b330` |
| `0x0034b480` | `CharaBaseClass_registerLua_getActorExtraStat_internal` | registrar |  |  | `FUN_0074b480` |
| `0x0034b5d0` | `CharaBaseClass_registerLua_getGrandOnExtraStat` | registrar |  |  | `FUN_0074b5d0` |
| `0x0034b720` | `CharaBaseClass_registerLua_getNetStatUser` | registrar |  |  | `FUN_0074b720` |
| `0x0034b870` | `CharaBaseClass_registerLua_getNetStatSystem` | registrar |  |  | `FUN_0074b870` |
| `0x0034b9c0` | `CharaBaseClass_registerLua_getSystemFlag` | registrar |  |  | `FUN_0074b9c0` |
| `0x0034bb10` | `CharaBaseClass_registerLua_isAccessibleInServer` | registrar |  |  | `FUN_0074bb10` |
| `0x0034bc60` | `CharaBaseClass_registerLua_getLocalizedDisplayName` | registrar |  |  | `FUN_0074bc60` |
| `0x0034bdb0` | `CharaBaseClass_registerLua_getLocalizedDisplayNameForChat` | registrar |  |  | `FUN_0074bdb0` |
| `0x0034bf00` | `CharaBaseClass_registerLua_getDisplayName` | registrar |  |  | `FUN_0074bf00` |
| `0x0034c050` | `CharaBaseClass_registerLua_isNameplateVisible` | registrar |  |  | `FUN_0074c050` |
| `0x0034c1a0` | `CharaBaseClass_registerLua_getFloatingOffset` | registrar |  |  | `FUN_0074c1a0` |
| `0x0034c2f0` | `CharaBaseClass_registerLua_isActorMainStatMode` | registrar |  |  | `FUN_0074c2f0` |
| `0x0034c440` | `CharaBaseClass_registerLua_getLocation` | registrar |  |  | `FUN_0074c440` |
| `0x0034c590` | `CharaBaseClass_registerLua_getLookAtCharacter` | registrar |  |  | `FUN_0074c590` |
| `0x0034c6e0` | `CharaBaseClass_registerLua_containsDamageAttribute_internal` | registrar |  |  | `FUN_0074c6e0` |
| `0x0034c830` | `CharaBaseClass_registerLua_encodeBonusPoint` | registrar |  |  | `FUN_0074c830` |
| `0x0034c980` | `CharaBaseClass_registerLua_decodeBonusPoint` | registrar |  |  | `FUN_0074c980` |
| `0x0034cad0` | `GroupBaseClass_registerLua_getMember` | registrar |  |  | `FUN_0074cad0` |
| `0x0034cc20` | `GroupBaseClass_registerLua_countMember` | registrar |  |  | `FUN_0074cc20` |
| `0x0034cd70` | `GroupBaseClass_registerLua_getKind` | registrar |  |  | `FUN_0074cd70` |
| `0x0034cec0` | `GroupBaseClass_registerLua_isExistInClientMember` | registrar |  |  | `FUN_0074cec0` |
| `0x0034d010` | `GroupBaseClass_registerLua_isExistInWorldMember` | registrar |  |  | `FUN_0074d010` |
| `0x0034d160` | `GroupBaseClass_registerLua_isMember` | registrar |  |  | `FUN_0074d160` |
| `0x0034d2b0` | `GroupBaseClass_registerLua_getMemberLocation` | registrar |  |  | `FUN_0074d2b0` |
| `0x0034d400` | `GroupBaseClass_registerLua_getMemberLocalizedDisplayName` | registrar |  |  | `FUN_0074d400` |
| `0x0034d550` | `GroupBaseClass_registerLua_getLocalizedDisplayName_internal` | registrar |  |  | `FUN_0074d550` |
| `0x0034d6a0` | `GroupBaseClass_registerLua_getMemberDisplayName` | registrar |  |  | `FUN_0074d6a0` |
| `0x0034d7f0` | `GroupBaseClass_registerLua_getDisplayName` | registrar |  |  | `FUN_0074d7f0` |
| `0x0034d940` | `GroupBaseClass_registerLua_getProperty` | registrar |  |  | `FUN_0074d940` |
| `0x0034da90` | `ItemBaseClass_registerLua_getCatalogID` | registrar |  |  | `FUN_0074da90` |
| `0x0034dbe0` | `ItemBaseClass_registerLua_getMaxStack` | registrar |  |  | `FUN_0074dbe0` |
| `0x0034dd30` | `ItemBaseClass_registerLua_isStackable` | registrar |  |  | `FUN_0074dd30` |
| `0x0034de80` | `ItemBaseClass_registerLua_countStack` | registrar |  |  | `FUN_0074de80` |
| `0x0034dfd0` | `ItemBaseClass_registerLua_isRare` | registrar |  |  | `FUN_0074dfd0` |
| `0x0034e120` | `ItemBaseClass_registerLua_getKind_internal` | registrar |  |  | `FUN_0074e120` |
| `0x0034e270` | `ItemBaseClass_registerLua_getPackage` | registrar |  |  | `FUN_0074e270` |
| `0x0034e3c0` | `ItemBaseClass_registerLua_getOwner` | registrar |  |  | `FUN_0074e3c0` |
| `0x0034e510` | `ItemBaseClass_registerLua_getNameIndex` | registrar |  |  | `FUN_0074e510` |
| `0x0034e660` | `ItemBaseClass_registerLua_isLocking` | registrar |  |  | `FUN_0074e660` |
| `0x0034e7b0` | `ItemBaseClass_registerLua_getLockingInfo` | registrar |  |  | `FUN_0074e7b0` |
| `0x0034e900` | `ItemBaseClass_registerLua_isDealing` | registrar |  |  | `FUN_0074e900` |
| `0x0034ea50` | `ItemBaseClass_registerLua_isAttached` | registrar |  |  | `FUN_0074ea50` |
| `0x0034eba0` | `ItemBaseClass_registerLua_getDealingInfo` | registrar |  |  | `FUN_0074eba0` |
| `0x0034ecf0` | `ItemBaseClass_registerLua_getDealingAttached` | registrar |  |  | `FUN_0074ecf0` |
| `0x0034ee40` | `ItemBaseClass_registerLua_isTrading` | registrar |  |  | `FUN_0074ee40` |
| `0x0034ef90` | `ItemBaseClass_registerLua_isEquipping` | registrar |  |  | `FUN_0074ef90` |
| `0x0034f0e0` | `ItemBaseClass_registerLua_getEquippingSlot` | registrar |  |  | `FUN_0074f0e0` |
| `0x0034f230` | `PlayerBase_registerLua_getGMRank` | registrar |  |  | `FUN_0074f230` |
| `0x0034f380` | `PlayerBase_registerLua_getBelongGrandCompany` | registrar |  |  | `FUN_0074f380` |
| `0x0034f4d0` | `PlayerBase_registerLua_getGrandCompanyRank` | registrar |  |  | `FUN_0074f4d0` |
| `0x0034f620` | `PlayerBase_registerLua_getAchievementTitle` | registrar |  |  | `FUN_0074f620` |
| `0x0034f810` | `Sequence_registerLua_play` | registrar |  |  | `FUN_0074f810` |
| `0x0034f960` | `Debug_registerLua_getLocalizedDisplayName_internal` | registrar |  |  | `FUN_0074f960` |
| `0x0034fab0` | `Debug_registerLua_getEventPriority_internal` | registrar |  |  | `FUN_0074fab0` |
| `0x0034fc00` | `Debug_registerLua_getCharacterLocation_internal` | registrar |  |  | `FUN_0074fc00` |
| `0x0034fd50` | `Debug_registerLua_getPlayingCutSceneActor_internal` | registrar |  |  | `FUN_0074fd50` |
| `0x0034fea0` | `Debug_registerLua_getSpreadSheetAllAttribute_internal` | registrar |  |  | `FUN_0074fea0` |
| `0x0034fff0` | `Debug_registerLua_isSpreadSheetExistAttribute_internal` | registrar |  |  | `FUN_0074fff0` |
| `0x00350140` | `Debug_registerLua_getItem_internal` | registrar |  |  | `FUN_00750140` |
| `0x00350290` | `Debug_registerLua_getAllItem` | registrar |  |  | `FUN_00750290` |
| `0x003503e0` | `NpcBaseClass_registerLua_isTalkable` | registrar |  |  | `FUN_007503e0` |
| `0x00350530` | `NpcBaseClass_registerLua_isEmotable` | registrar |  |  | `FUN_00750530` |
| `0x00350680` | `NpcBaseClass_registerLua_isPushable` | registrar |  |  | `FUN_00750680` |
| `0x003507d0` | `NpcBaseClass_registerLua_isMapObj` | registrar |  |  | `FUN_007507d0` |
| `0x00350920` | `NpcBaseClass_registerLua_isPushing` | registrar |  |  | `FUN_00750920` |
| `0x00350a70` | `NpcBaseClass_registerLua_isEnmity` | registrar |  |  | `FUN_00750a70` |
| `0x00350bc0` | `SpreadSheet_registerLua_isExistKey` | registrar |  |  | `FUN_00750bc0` |
| `0x00350d10` | `SpreadSheet_registerLua_getAllKey` | registrar |  |  | `FUN_00750d10` |
| `0x00350e60` | `WidgetBaseClass_registerLua_getParentWidget` | registrar |  |  | `FUN_00750e60` |
| `0x00350fb0` | `WidgetBaseClass_registerLua_countChildWidgets` | registrar |  |  | `FUN_00750fb0` |
| `0x00351100` | `WidgetBaseClass_registerLua_getChildWidget` | registrar |  |  | `FUN_00751100` |
| `0x00351250` | `DesktopWidget_registerLua_getCurrentTargetCursor` | registrar |  |  | `FUN_00751250` |
| `0x003513a0` | `DesktopWidget_registerLua_getTargetCharacter` | registrar |  |  | `FUN_007513a0` |
| `0x003514f0` | `DesktopWidget_registerLua_getCharacterByDisplayNameForTextCommand` | registrar |  |  | `FUN_007514f0` |
| `0x00351640` | `DesktopWidget_registerLua_getKeyboardFocusedWidget` | registrar |  |  | `FUN_00751640` |
| `0x00351790` | `DesktopWidget_registerLua_setKeyboardFocusedWidget` | registrar |  |  | `FUN_00751790` |
| `0x003518e0` | `DesktopWidget_registerLua_isTargetCursorControlEnabled` | registrar |  |  | `FUN_007518e0` |
| `0x00351a30` | `DesktopWidget_registerLua_getUserConfig` | registrar |  |  | `FUN_00751a30` |
| `0x00351b80` | `DesktopWidget_registerLua_getUserMacroTitle` | registrar |  |  | `FUN_00751b80` |
| `0x00351cd0` | `DesktopWidget_registerLua_getUserMacroIcon` | registrar |  |  | `FUN_00751cd0` |
| `0x00351e20` | `DesktopWidget_registerLua_getUserMacroData` | registrar |  |  | `FUN_00751e20` |
| `0x00351f70` | `DesktopWidget_registerLua_parseTextCommand` | registrar |  |  | `FUN_00751f70` |
| `0x003520c0` | `DesktopWidget_registerLua_getLastAttacker` | registrar |  |  | `FUN_007520c0` |
| `0x00352210` | `DesktopWidget_registerLua_getWidgetContainerSize` | registrar |  |  | `FUN_00752210` |
| `0x00352360` | `DesktopWidget_registerLua_isExistWidgetInWidgetContainer` | registrar |  |  | `FUN_00752360` |
| `0x003524b0` | `DesktopWidget_registerLua_isCreatingWidgetInWidgetContainer` | registrar |  |  | `FUN_007524b0` |
| `0x00352600` | `DesktopWidget_registerLua_getWidgetFromWidgetContainer` | registrar |  |  | `FUN_00752600` |
| `0x00352750` | `WorldMaster_registerLua_getMyPlayer` | registrar |  |  | `FUN_00752750` |
| `0x003528a0` | `WorldMaster_registerLua_getServerTime` | registrar |  |  | `FUN_007528a0` |
| `0x003529f0` | `WorldMaster_registerLua_getPendingCutSceneActor` | registrar |  |  | `FUN_007529f0` |
| `0x00352b40` | `WorldMaster_registerLua_getHydaelynHour` | registrar |  |  | `FUN_00752b40` |
| `0x00352c90` | `WorldMaster_registerLua_getHydaelynDay` | registrar |  |  | `FUN_00752c90` |
| `0x00352de0` | `WorldMaster_registerLua_getHydaelynTime` | registrar |  |  | `FUN_00752de0` |
| `0x00352f30` | `WorldMaster_registerLua_getHydaelynMoon` | registrar |  |  | `FUN_00752f30` |
| `0x00353080` | `WorldMaster_registerLua_isKeyboardOnlyTutorial` | registrar |  |  | `FUN_00753080` |
| `0x003531d0` | `WorldMaster_registerLua_getSpecialEventWork` | registrar |  |  | `FUN_007531d0` |
| `0x00353320` | `global_registerLua_canCreateActorByName` | registrar |  |  | `FUN_00753320` |
| `0x00353470` | `global_registerLua_isInstanceOf` | registrar |  |  | `FUN_00753470` |
| `0x003535c0` | `global_registerLua_getQuestActorForCutSceneReplay` | registrar |  |  | `FUN_007535c0` |
| `0x00353710` | `global_registerLua_normalizeDisplayName` | registrar |  |  | `FUN_00753710` |
| `0x00353860` | `global_registerLua_replaceMacroCodeString` | registrar |  |  | `FUN_00753860` |
| `0x00353a40` | `AreaBaseClass_registerLua_getAreaType_internal` | registrar |  |  | `FUN_00753a40` |
| `0x00353c30` | `ActorBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_00753c30` |
| `0x00353cf0` | `AreaMaster_registerAllLuaBindings` | registrar |  |  | `FUN_00753cf0` |
| `0x00353dd0` | `ItemBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_00753dd0` |
| `0x00353f90` | `PlayerBase_registerAllLuaBindings` | master_block |  |  | `PlayerBase_registerAllLuaBindings` |
| `0x003547d0` | `Sequence_registerAllLuaBindings` | registrar |  |  | `FUN_007547d0` |
| `0x00354850` | `NpcBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_00754850` |
| `0x00354a60` | `WidgetBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_00754a60` |
| `0x00354c70` | `WorldMaster_registerAllLuaBindings` | registrar |  |  | `FUN_00754c70` |
| `0x00354e70` | `AreaBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_00754e70` |
| `0x00354e90` | `CharaBaseClass_registerLua_getSubStatMode` | registrar |  |  | `FUN_00754e90` |
| `0x00354fe0` | `CharaBaseClass_registerLua_setActorExtraStat_internal` | registrar |  |  | `FUN_00754fe0` |
| `0x00355130` | `CharaBaseClass_registerLua_setDisplayName` | registrar |  |  | `FUN_00755130` |
| `0x00355280` | `CharaBaseClass_registerLua_getGroup` | registrar |  |  | `FUN_00755280` |
| `0x003553d0` | `CharaBaseClass_registerLua_getExtendedTemporaryGroup` | registrar |  |  | `FUN_007553d0` |
| `0x00355520` | `CharaBaseClass_registerLua_getAllGroup` | registrar |  |  | `FUN_00755520` |
| `0x00355670` | `CharaBaseClass_registerLua_getExtendedTemporaryAllGroup` | registrar |  |  | `FUN_00755670` |
| `0x003557c0` | `CharaBaseClass_registerLua_getGroupByDisplayName` | registrar |  |  | `FUN_007557c0` |
| `0x00355910` | `CharaBaseClass_registerLua_getExtendedTemporaryGroupByDisplayName` | registrar |  |  | `FUN_00755910` |
| `0x00355a60` | `CharaBaseClass_registerLua_getItem` | registrar |  |  | `FUN_00755a60` |
| `0x00355bb0` | `CharaBaseClass_registerLua_getExtendedTemporaryItem` | registrar |  |  | `FUN_00755bb0` |
| `0x00355d00` | `CharaBaseClass_registerLua_getEquippingItem` | registrar |  |  | `FUN_00755d00` |
| `0x00355e50` | `CharaBaseClass_registerLua_getExtendedTemporaryEquippingItem` | registrar |  |  | `FUN_00755e50` |
| `0x00355fa0` | `CharaBaseClass_registerLua_getItemPackageCapacity` | registrar |  |  | `FUN_00755fa0` |
| `0x003560f0` | `CharaBaseClass_registerLua_getItemPackageFreeSpace` | registrar |  |  | `FUN_007560f0` |
| `0x00356240` | `CharaBaseClass_registerLua_hasItemPackage` | registrar |  |  | `FUN_00756240` |
| `0x00356390` | `CharaBaseClass_registerLua_isLockingItem` | registrar |  |  | `FUN_00756390` |
| `0x003564e0` | `CharaBaseClass_registerLua_isItemDealing` | registrar |  |  | `FUN_007564e0` |
| `0x00356630` | `CharaBaseClass_registerLua_getTradingItem` | registrar |  |  | `FUN_00756630` |
| `0x00356780` | `CharaBaseClass_registerLua_getExtendedTemporaryTradingItem` | registrar |  |  | `FUN_00756780` |
| `0x003568d0` | `CharaBaseClass_registerLua_createVirtualItem` | registrar |  |  | `FUN_007568d0` |
| `0x00356a20` | `CharaBaseClass_registerLua_createExtendedTemporaryVirtualItem` | registrar |  |  | `FUN_00756a20` |
| `0x00356b70` | `GroupBaseClass_registerLua_getOccupancyGroup` | registrar |  |  | `FUN_00756b70` |
| `0x00356cc0` | `GroupBaseClass_registerLua_getExtendedTemporaryOccupancyGroup` | registrar |  |  | `FUN_00756cc0` |
| `0x00356e10` | `Debug_registerLua_commandDebug` | registrar |  |  | `FUN_00756e10` |
| `0x00356f60` | `DesktopWidget_registerLua_createWidgetInWidgetContainer` | registrar |  |  | `FUN_00756f60` |
| `0x003570b0` | `DirectorBaseClass_registerLua_getGroupByDisplayName` | registrar |  |  | `FUN_007570b0` |
| `0x00357200` | `DirectorBaseClass_registerLua_getExtendedTemporaryGroupByDisplayName` | registrar |  |  | `FUN_00757200` |
| `0x00357350` | `global_registerLua_createActor` | registrar |  |  | `FUN_00757350` |
| `0x003574a0` | `CharaBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_007574a0` |
| `0x00357b70` | `GroupBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_00757b70` |
| `0x00357ce0` | `Debug_registerAllLuaBindings` | registrar |  |  | `FUN_00757ce0` |
| `0x00357ea0` | `DesktopWidget_registerAllLuaBindings` | registrar |  |  | `FUN_00757ea0` |
| `0x00358260` | `DirectorBaseClass_registerAllLuaBindings` | registrar |  |  | `FUN_00758260` |
| `0x003582e0` | `global_registerAllLuaBindings` | registrar |  |  | `FUN_007582e0` |
| `0x00358520` | `SpreadSheet_registerLua_getData` | registrar |  |  | `FUN_00758520` |
| `0x00358670` | `SpreadSheet_registerAllLuaBindings` | registrar |  |  | `FUN_00758670` |
| `0x00359940` | `ZoneIn_handler_opcode_11_CutScene_onShowWidgetClip` | opcode_handler |  |  | `ZoneIn_handler_opcode_11_CutScene_onShowWidgetClip` |
| `0x003599e0` | `ZoneIn_handler_opcode_12_CutScene_onHideWidgetClip` | opcode_handler |  |  | `ZoneIn_handler_opcode_12_CutScene_onHideWidgetClip` |
| `0x00359a50` | `ZoneIn_handler_opcode_15_NOP` | opcode_handler |  |  | `FUN_00759a50` |
| `0x00359a60` | `ZoneIn_handler_opcode_16_Debug_scriptExec` | opcode_handler |  |  | `ZoneIn_handler_opcode_16_Debug_scriptExec` |
| `0x00359ad0` | `ZoneIn_handler_opcode_17_onPreCutSceneCancel` | opcode_handler |  |  | `ZoneIn_handler_opcode_17_onPreCutSceneCancel` |
| `0x00359b40` | `ZoneIn_handler_opcode_18_onPostCutSceneCancel` | opcode_handler |  |  | `ZoneIn_handler_opcode_18_onPostCutSceneCancel` |
| `0x00359bb0` | `ZoneIn_handler_opcode_20_DesktopWidget_onPreWarp` | opcode_handler |  |  | `ZoneIn_handler_opcode_20_DesktopWidget_onPreWarp` |
| `0x00359c20` | `ZoneIn_handler_opcode_21_DesktopWidget_onPostWarp` | opcode_handler |  |  | `ZoneIn_handler_opcode_21_DesktopWidget_onPostWarp` |
| `0x00359c90` | `ZoneIn_handler_vtable_dispatch_slot21` | opcode_handler |  |  | `ZoneIn_handler_vtable_dispatch_slot21` |
| `0x00359cb0` | `ZoneIn_handler_vtable_dispatch_slot22` | opcode_handler |  |  | `ZoneIn_handler_vtable_dispatch_slot22` |
| `0x00359cd0` | `ZoneIn_handler_vtable_dispatch_slot23` | opcode_handler |  |  | `FUN_00759cd0` |
| `0x00359cf0` | `ZoneIn_handler_vtable_dispatch_slot24` | opcode_handler |  |  | `ZoneIn_handler_vtable_dispatch_slot24` |
| `0x00359d10` | `ZoneIn_handler_vtable_dispatch_slot25` | opcode_handler |  |  | `ZoneIn_handler_vtable_dispatch_slot25` |
| `0x00359d20` | `ZoneIn_handler_opcode_27_SetEventStatusReceiver` | opcode_handler |  |  | `FUN_00759d20` |
| `0x00359de0` | `ZoneIn_handler_opcode_28_NOP` | opcode_handler |  |  | `FUN_00759de0` |
| `0x00359df0` | `ZoneIn_handler_opcode_29_NOP` | opcode_handler |  |  | `FUN_00759df0` |
| `0x00359e00` | `ZoneIn_handler_opcode_30_NOP` | opcode_handler |  |  | `FUN_00759e00` |
| `0x00359e10` | `ZoneIn_handler_opcode_31_NOP` | opcode_handler |  |  | `FUN_00759e10` |
| `0x00359e20` | `ZoneIn_handler_opcode_32_NOP` | opcode_handler |  |  | `FUN_00759e20` |
| `0x00359e30` | `ZoneIn_handler_opcode_33_NOP` | opcode_handler |  |  | `FUN_00759e30` |
| `0x00359e40` | `ZoneIn_handler_opcode_34_NOP` | opcode_handler |  |  | `FUN_00759e40` |
| `0x00359e50` | `ZoneIn_handler_dataPacket_calls_onReceiveDataPacket` | opcode_handler |  |  | `FUN_00759e50` |
| `0x00359ed0` | `ZoneIn_handler_opcode_39_internal_map_insert` | opcode_handler |  |  | `ZoneIn_handler_opcode_39_internal_map_insert` |
| `0x00359f50` | `ZoneIn_handler_opcode_40_timed_or_immediate_exec` | opcode_handler |  |  | `ZoneIn_handler_opcode_40_timed_or_immediate_exec` |
| `0x0035a060` | `ZoneIn_handler_opcode_43_CharaBase_onChangeSystemFlag` | opcode_handler |  |  | `ZoneIn_handler_opcode_43_CharaBase_onChangeSystemFlag` |
| `0x0035a0e0` | `ZoneIn_handler_opcode_44_MyPlayer_onReceiveLimitAddicted` | opcode_handler |  |  | `ZoneIn_handler_opcode_44_MyPlayer_onReceiveLimitAddicted` |
| `0x0035a160` | `ZoneIn_handler_opcode_45_HateStatusReceiver` | opcode_handler |  |  | `FUN_0075a160` |
| `0x0035a200` | `ZoneIn_handler_opcode_46_ChocoboReceiver` | opcode_handler |  |  | `FUN_0075a200` |
| `0x0035a280` | `ZoneIn_handler_opcode_47_ChocoboGradeReceiver` | opcode_handler |  |  | `FUN_0075a280` |
| `0x0035a300` | `ZoneIn_handler_opcode_48_GoobbueReceiver` | opcode_handler |  |  | `FUN_0075a300` |
| `0x0035a380` | `ZoneIn_handler_opcode_49_VehicleGradeReceiver` | opcode_handler |  |  | `FUN_0075a380` |
| `0x0035a400` | `ZoneIn_handler_opcode_50_GrandCompanyReceiver` | opcode_handler |  |  | `FUN_0075a400` |
| `0x0035a490` | `ZoneIn_handler_opcode_51_NOP` | opcode_handler |  |  | `FUN_0075a490` |
| `0x0035a4a0` | `ZoneIn_handler_opcode_52_NOP` | opcode_handler |  |  | `FUN_0075a4a0` |
| `0x0035a4b0` | `ZoneIn_handler_opcode_53_AchievementPointReceiver` | opcode_handler |  |  | `FUN_0075a4b0` |
| `0x0035a530` | `ZoneIn_handler_opcode_54_AchievementTitleReceiver` | opcode_handler |  |  | `FUN_0075a530` |
| `0x0035a5b0` | `ZoneIn_handler_opcode_55_AchievementIdReceiver` | opcode_handler |  |  | `FUN_0075a5b0` |
| `0x0035a630` | `ZoneIn_handler_opcode_56_AchievementAchievedCountReceiver` | opcode_handler |  |  | `FUN_0075a630` |
| `0x0035a6b0` | `ZoneIn_handler_opcode_58_JobChangeReceiver` | opcode_handler |  |  | `FUN_0075a6b0` |
| `0x0035a730` | `ZoneIn_handler_opcode_59_EntrustItemReceiver` | opcode_handler |  |  | `FUN_0075a730` |
| `0x0035b300` | `System_broadcastSubsystem_preCancelHooks` | named |  |  | `FUN_0075b300` |
| `0x0035b330` | `System_broadcastSubsystem_postCancelHooks` | named |  |  | `FUN_0075b330` |
| `0x0035c580` | `MsgDispatch_LookupAndEnqueue` | dispatcher |  |  | `FUN_0075c580` |
| `0x0035ccf0` | `ChatParser_entry` | chat |  |  | `FUN_0075ccf0` |
| `0x0035d120` | `PerFrameSubsystem_slot1_0x110_PLAYER_MODE_TICKER_3bindings` | named |  |  | `FUN_0075d120` |
| `0x0035d750` | `ZoneIn_handler_opcode_4_DesktopWidget_onTargetChanged` | opcode_handler |  |  | `ZoneIn_handler_opcode_4_DesktopWidget_onTargetChanged` |
| `0x0035d780` | `ZoneIn_handler_opcode_5_DesktopWidget_onTargetDecided` | opcode_handler |  |  | `ZoneIn_handler_opcode_5_DesktopWidget_onTargetDecided` |
| `0x0035d7b0` | `ZoneIn_handler_opcode_6_GetCurrentTarget_query` | opcode_handler |  |  | `ZoneIn_handler_opcode_6_GetCurrentTarget_query` |
| `0x0035d830` | `ZoneIn_handler_opcode_7_CutScene_onInitializationClip_Preview` | opcode_handler |  |  | `ZoneIn_handler_opcode_7_CutScene_onInitializationClip_Preview` |
| `0x0035d860` | `ZoneIn_handler_opcode_8_CutScene_onInitializationClip` | opcode_handler |  |  | `ZoneIn_handler_opcode_8_CutScene_onInitializationClip` |
| `0x0035d890` | `ZoneIn_handler_opcode_9_CutScene_onShowUIClip` | opcode_handler |  |  | `ZoneIn_handler_opcode_9_CutScene_onShowUIClip` |
| `0x0035d8d0` | `ZoneIn_handler_opcode_10_CutScene_onHideUIClip` | opcode_handler |  |  | `ZoneIn_handler_opcode_10_CutScene_onHideUIClip` |
| `0x0035d900` | `ZoneIn_handler_opcode_13_CutScene_onOpenUIClip` | opcode_handler |  |  | `ZoneIn_handler_opcode_13_CutScene_onOpenUIClip` |
| `0x0035d950` | `ZoneIn_handler_opcode_14_CutScene_setActiveAndFinalize` | opcode_handler |  |  | `ZoneIn_handler_opcode_14_CutScene_setActiveAndFinalize` |
| `0x0035e1c0` | `ZoneOut_send_large_simple` | opcode_sender | 0x12d |  | `ZoneOut_send_large_simple` |
| `0x0035e230` | `ZoneOut_send_large_checksummed_v3` | opcode_sender | 0x12d |  | `ZoneOut_send_large_checksummed_v3` |
| `0x0035e3a0` | `ZoneOut_send_large_checksummed_v1` | opcode_sender | 0x12d |  | `ZoneOut_send_large_checksummed_v1` |
| `0x0035e510` | `ZoneOut_send_large_checksummed_v2` | opcode_sender | 0x12d |  | `ZoneOut_send_large_checksummed_v2` |
| `0x0035e670` | `ZoneOut_send_opcode_0x12e_104B` | opcode_sender | 0x12e |  | `ZoneOut_send_opcode_0x12e_104B` |
| `0x0035e770` | `WorkSync_buildAndSendPacket_opcode_0x12f` | named | 0x12f |  | `FUN_0075e770` |
| `0x0035e860` | `ZoneOut_send_opcode_0x130_32B_variantA` | opcode_sender | 0x130 |  | `ZoneOut_send_opcode_0x130_32B_variantA` |
| `0x0035e8d0` | `ZoneOut_send_opcode_0x130_32B_variantB` | opcode_sender | 0x130 |  | `ZoneOut_send_opcode_0x130_32B_variantB` |
| `0x0035e950` | `ZoneOut_send_opcode_0x133_56B` | opcode_sender | 0x133 |  | `ZoneOut_send_opcode_0x133_56B` |
| `0x0035ea50` | `ZoneOut_send_opcode_0x131_24B_byte` | opcode_sender | 0x131 |  | `ZoneOut_send_opcode_0x131_24B_byte` |
| `0x0035eac0` | `ZoneOut_send_opcode_0x132_24B_byteUshort` | opcode_sender | 0x132 |  | `ZoneOut_send_opcode_0x132_24B_byteUshort` |
| `0x0035eba0` | `ZoneOut_send_opcode_0x134_40B_withNonce` | opcode_sender | 0x134 |  | `ZoneOut_send_opcode_0x134_40B_withNonce` |
| `0x0035ecd0` | `ZoneOut_send_opcode_0x135_24B_dword` | opcode_sender | 0x135 |  | `ZoneOut_send_opcode_0x135_24B_dword` |
| `0x00364150` | `CharaList_ForEach` | named |  |  | `FUN_00764150` |
| `0x00364fd0` | `PerFrameSubsystem_slot1_0x114_WIDGET_CONTAINER_CHILD_NOTIFIER` | named |  |  | `FUN_00764fd0` |
| `0x00365340` | `PerFrameSubsystem_slot12_DEAD_SESSION_CLEANUP_TICK` | named |  |  | `FUN_00765340` |
| `0x00366f00` | `PerFrameSubsystem_slot2_widgetLifecyclePump_stateMachine` | named |  |  | `FUN_00766f00` |
| `0x00367c00` | `WorkSync_serializePayloadAndSend` | named |  |  | `FUN_00767c00` |
| `0x00367fc0` | `WorkSync_dispatchOrEnqueue` | dispatcher |  |  | `FUN_00767fc0` |
| `0x00368e40` | `StatusEffectList_construct_16slots_6Bperentry` | named |  |  | `FUN_00768e40` |
| `0x00368ef0` | `ExtendedStatusList_construct_32slots_6B_hardcoded` | named |  |  | `FUN_00768ef0` |
| `0x0036a9c0` | `PerFrameSubsystem_slot5_spreadsheetCSVPreloader_4categories` | named |  |  | `FUN_0076a9c0` |
| `0x0036b3d0` | `CommandUpdater_allocAndEnqueueRecord` | named |  |  | `FUN_0076b3d0` |
| `0x0036b760` | `ActionResult_parseSingleRecord_112B_complex` | named |  |  | `FUN_0076b760` |
| `0x0036b950` | `ActorMessageQueue_lookupOrCreate_perActorId_WorkPathTree` | named |  |  | `FUN_0076b950` |
| `0x0036c0d0` | `ZoneIn_handler_chat_say_substitution_entry35` | opcode_handler |  |  | `FUN_0076c0d0` |
| `0x0036c220` | `ZoneIn_handler_chat_variant_C_tell` | opcode_handler |  |  | `ZoneIn_handler_chat_variant_C_tell` |
| `0x0036c3b0` | `ZoneIn_handler_chat_simple_entry37` | opcode_handler |  |  | `FUN_0076c3b0` |
| `0x0036c690` | `ZoneIn_handler_chat_variant_D` | opcode_handler |  |  | `FUN_0076c690` |
| `0x0036dab0` | `PerFrameSubsystem_slot11_compound_widget_tick_2subdispatchers` | dispatcher |  |  | `?DoPropExchange@COleControl@@UAEXPAVCPropExchange@@@Z` |
| `0x0036e270` | `ZoneOut_sendScriptError_opcode_0x12d` | opcode_sender | 0x12d |  | `ZoneOut_sendScriptError_opcode_0x12d` |
| `0x0036f040` | `TimeoutMonitor_perEntry_900frameThreshold_triggerAction` | named |  |  | `FUN_0076f040` |
| `0x0036f6f0` | `PerFrameSubsystem_slot3_widgetAnimationStateTick` | named |  |  | `FUN_0076f6f0` |
| `0x003700b0` | `PerFrameSubsystem_slot4_widgetLoadManager_msg0xde` | named |  |  | `FUN_007700b0` |
| `0x00370c00` | `PerFrameSubsystem_slot10_timeoutMonitor_900frames_15sec` | named |  |  | `FUN_00770c00` |
| `0x003713e0` | `ActionResultBatch_construct_multiRecord_countAt_0x380_byte` | named |  |  | `FUN_007713e0` |
| `0x003715f0` | `LargeActionBatch_construct_64records_112B_hardcoded` | named |  |  | `FUN_007715f0` |
| `0x00371f50` | `CommandUpdater_send_toCharaBase` | named |  |  | `FUN_00771f50` |
| `0x00372050` | `CommandUpdater_send_toCharaBase_WMSelf` | named |  |  | `FUN_00772050` |
| `0x003721b0` | `CommandUpdater_send_toActorId` | named |  |  | `FUN_007721b0` |
| `0x00372560` | `CommandUpdater_send_toActorName` | named |  |  | `FUN_00772560` |
| `0x00372650` | `CommandUpdater_send_broadcast` | named |  |  | `FUN_00772650` |
| `0x00373d90` | `CommandUpdater_invokeLua_onUpdateWork_clipObj` | inbound_invoker |  |  | `FUN_00773d90` |
| `0x00373f10` | `CommandUpdater_invokeLua_onUpdateWork_complex` | inbound_invoker |  |  | `FUN_00773f10` |
| `0x00376340` | `UserDataReceiver_vtable_noop_inherited` | receiver_slot |  |  | `UserDataReceiver_vtable_noop_inherited` |
| `0x00376690` | `CommandUpdate_constructor_init` | named |  |  | `FUN_00776690` |
| `0x00376760` | `PacketBuilder_opcode_0x12d_200B_tagged` | named | 0x12d |  | `FUN_00776760` |
| `0x003838c0` | `UpdateQueue_pushEntry` | named |  |  | `FUN_007838c0` |
| `0x00389b90` | `ActionResult_pushToActorList_88Bstride` | named |  |  | `FUN_00789b90` |
| `0x00389cd0` | `BehaviorLogger_SourceDisplayNameResolverListener_ctor` | named |  |  | `FUN_00789cd0` |
| `0x0038b850` | `WaitResumeChecker_ctor` | named |  |  | `FUN_0078b850` |
| `0x0038bbb0` | `typed_invokeLua_onInit_helper` | inbound_invoker |  |  | `FUN_0078bbb0` |
| `0x0038bed0` | `wait_extractAndPushChecker` | named |  |  | `FUN_0078bed0` |
| `0x0038c2a0` | `defineClass_extractAndRegister` | named |  |  | `FUN_0078c2a0` |
| `0x00493380` | `ServerNotify_createResumeChecker_atCmdSubsystem_0xf8` | named |  |  | `FUN_00893380` |
| `0x00494090` | `Lua_send6argRpc_via_opcode_0x12e` | lua_impl | 0x12e | _send6argRpc | `Lua_send6argRpc_via_opcode_0x12e` |
| `0x00496510` | `Command_queued_enqueueRecord_to_list_0x14` | named |  |  | `FUN_00896510` |
| `0x00497310` | `Command_immediate_sendVia_0x12d_checksummed_v1v2` | named |  |  | `FUN_00897310` |
| `0x00497660` | `Player_invokeLua_onPreEvent` | inbound_invoker |  |  | `FUN_00897660` |
| `0x004977b0` | `Player_invokeLua_onPostEvent` | inbound_invoker |  |  | `FUN_008977b0` |
| `0x00497a20` | `Player_invokeLua_onCommandRejected` | inbound_invoker |  |  | `FUN_00897a20` |
| `0x00497b40` | `Player_invokeLua_onPreCommand` | inbound_invoker |  |  | `FUN_00897b40` |
| `0x00497c60` | `Player_invokeLua_onPostCommand` | inbound_invoker |  |  | `FUN_00897c60` |
| `0x00497d90` | `Player_invokeLua_onCommandCancel_v1` | inbound_invoker |  |  | `FUN_00897d90` |
| `0x00497ee0` | `Player_invokeLua_onCommandCancel_v2` | inbound_invoker |  |  | `FUN_00897ee0` |
| `0x00498030` | `Player_invokeLua_onEventCancel_v1` | inbound_invoker |  |  | `FUN_00898030` |
| `0x004981a0` | `Player_invokeLua_onEventCancel_v2` | inbound_invoker |  |  | `FUN_008981a0` |
| `0x00498310` | `Player_invokeLua_onEventCancel_v3` | inbound_invoker |  |  | `FUN_00898310` |
| `0x00498480` | `Command_dispatch_immediateVsQueued_byVtable0x1c` | dispatcher |  |  | `FUN_00898480` |
| `0x00498760` | `Player_invokeLua_onEventCancel_v4` | inbound_invoker |  |  | `FUN_00898760` |
| `0x004988d0` | `Player_invokeLua_onEventCancel_v5` | inbound_invoker |  |  | `FUN_008988d0` |
| `0x00498a40` | `Player_invokeLua_onEventCancel_v6` | inbound_invoker |  |  | `FUN_00898a40` |
| `0x00498bb0` | `Player_invokeLua_onEventCancel_v7` | inbound_invoker |  |  | `FUN_00898bb0` |
| `0x00498d20` | `Player_invokeLua_onTouch_proximityBegin` | inbound_invoker |  |  | `Player_invokeLua_onTouch_proximityBegin` |
| `0x00498eb0` | `Player_invokeLua_onTouch_proximityEnd` | inbound_invoker |  |  | `Player_invokeLua_onTouch_proximityEnd` |
| `0x0049c460` | `AchievementPointReceiver_construct` | named |  |  | `FUN_0089c460` |
| `0x0049c510` | `AchievementPointReceiver_applyToMyPlayer` | named |  |  | `FUN_0089c510` |
| `0x0049c540` | `AchievementTitleReceiver_construct` | named |  |  | `FUN_0089c540` |
| `0x0049c5f0` | `AchievementTitleReceiver_applyToPlayerBase_field_0xe8` | named |  |  | `FUN_0089c5f0` |
| `0x0049c620` | `AchievementIdReceiver_construct` | named |  |  | `FUN_0089c620` |
| `0x0049c6d0` | `AchievementIdReceiver_applyToMyPlayer` | named |  |  | `FUN_0089c6d0` |
| `0x0049c700` | `AchievementAchievedCountReceiver_construct` | named |  |  | `FUN_0089c700` |
| `0x0049c7c0` | `AchievementAchievedCountReceiver_applyToMyPlayer` | named |  |  | `FUN_0089c7c0` |
| `0x0049c8b0` | `Router_dispatch_to_MyPlayer_handleLimitAddictedNotice` | router |  |  | `Router_dispatch_to_MyPlayer_handleLimitAddictedNotice` |
| `0x0049ca80` | `Router_dispatch_to_CharaBase_onChangeSystemFlag` | router |  |  | `Router_dispatch_to_CharaBase_onChangeSystemFlag` |
| `0x0049cab0` | `JobChangeReceiver_construct` | named |  |  | `FUN_0089cab0` |
| `0x0049cb60` | `JobChangeReceiver_applyToPlayerBase` | named |  |  | `FUN_0089cb60` |
| `0x0049cb90` | `EntrustItemReceiver_applyToMyPlayer` | named |  |  | `FUN_0089cb90` |
| `0x0049cc90` | `GrandCompanyReceiver_construct` | named |  |  | `FUN_0089cc90` |
| `0x0049cd60` | `GrandCompanyReceiver_applyToPlayerBase_polymorphic` | named |  |  | `FUN_0089cd60` |
| `0x0049cf60` | `HateStatusReceiver_construct` | named |  |  | `FUN_0089cf60` |
| `0x0049d030` | `HateStatusReceiver_applyToNpcBase` | named |  |  | `FUN_0089d030` |
| `0x0049d170` | `ChatBuilder_singleTarget_writeCommandTag_B` | chat |  |  | `ChatBuilder_singleTarget_writeCommandTag_B` |
| `0x0049d220` | `ChatBuilder_singleTarget_writeCommandTag_A` | chat |  |  | `ChatBuilder_singleTarget_writeCommandTag_A` |
| `0x0049d340` | `ChatBuilder_singleTarget_writeCommandTag_D` | chat |  |  | `ChatBuilder_singleTarget_writeCommandTag_D` |
| `0x0049d770` | `SetEventStatusReceiver_construct` | named |  |  | `FUN_0089d770` |
| `0x0049d860` | `SetEventStatusReceiver_applyToNpcBase` | named |  |  | `FUN_0089d860` |
| `0x0049e320` | `ChatBuilder_tell_readRecipientName` | chat |  |  | `ChatBuilder_tell_readRecipientName` |
| `0x0049e3f0` | `ChatBuilder_tell_thunk` | chat |  |  | `ChatBuilder_tell_thunk` |
| `0x0049eed0` | `Network_UserDataReceiver_ctor` | receiver |  |  | `Network_UserDataReceiver_ctor` |
| `0x0049f110` | `EntrustItemReceiver_construct_32byte_struct` | named |  |  | `FUN_0089f110` |
| `0x0049fbf0` | `Network_UserDataReceiver_multiModeDispatcher` | dispatcher |  |  | `Network_UserDataReceiver_multiModeDispatcher` |
| `0x004a0190` | `UserDataReceiver_invokeLua_onReceiveDataPacket` | inbound_invoker |  |  | `FUN_008a0190` |
| `0x004a0370` | `UserDataReceiver_invokeLua_onReceiveTimingPacket` | inbound_invoker |  |  | `FUN_008a0370` |
| `0x004a1510` | `UserDataReceiver_extractActorId` | named |  |  | `FUN_008a1510` |
| `0x004a15b0` | `UserDataReceiver_extractActorName` | named |  |  | `FUN_008a15b0` |
| `0x004a2b70` | `UserDataReceiver_vtable_slot23_resolveActorIntoField0x18` | receiver_slot |  |  | `UserDataReceiver_vtable_slot23_resolveActorIntoField0x18` |
| `0x004a2d50` | `UserDataReceiver_vtable_slot22_appendPayloadToContainer` | receiver_slot |  |  | `UserDataReceiver_vtable_slot22_appendPayloadToContainer` |
| `0x004a2e70` | `ChocoboReceiver_construct` | named |  |  | `FUN_008a2e70` |
| `0x004a2f30` | `ChocoboReceiver_applyToMyPlayer` | named |  |  | `FUN_008a2f30` |
| `0x004a2f70` | `ChocoboGradeReceiver_construct` | named |  |  | `FUN_008a2f70` |
| `0x004a3020` | `ChocoboGradeReceiver_applyToMyPlayer` | named |  |  | `FUN_008a3020` |
| `0x004a3050` | `GoobbueReceiver_construct` | named |  |  | `FUN_008a3050` |
| `0x004a3100` | `GoobbueReceiver_applyToMyPlayer` | named |  |  | `FUN_008a3100` |
| `0x004a3130` | `VehicleGradeReceiver_construct` | named |  |  | `FUN_008a3130` |
| `0x004a31e0` | `VehicleGradeReceiver_applyToMyPlayer` | named |  |  | `FUN_008a31e0` |
| `0x004a3990` | `Router_dispatch_to_CutScene_onInitializationClip` | router |  |  | `Router_dispatch_to_CutScene_onInitializationClip` |
| `0x004a39e0` | `Router_dispatch_to_CutScene_onShowUIClip` | router |  |  | `Router_dispatch_to_CutScene_onShowUIClip` |
| `0x004a3a40` | `Router_dispatch_to_CutScene_onHideUIClip` | router |  |  | `Router_dispatch_to_CutScene_onHideUIClip` |
| `0x004a3aa0` | `Router_dispatch_to_CutScene_method_FUN_006fc3a0` | router |  |  | `Router_dispatch_to_CutScene_method_FUN_006fc3a0` |
| `0x004a3b00` | `Router_dispatch_to_CutScene_onHideWidgetClip` | router |  |  | `Router_dispatch_to_CutScene_onHideWidgetClip` |
| `0x004a3b50` | `Router_dispatch_to_CutScene_onOpenUIClip` | router |  |  | `Router_dispatch_to_CutScene_onOpenUIClip` |
| `0x004a3ea0` | `Router_dispatch_to_DesktopWidget_onTargetChanged` | router |  |  | `Router_dispatch_to_DesktopWidget_onTargetChanged` |
| `0x004a3ed0` | `Router_dispatch_to_DesktopWidget_onTargetDecided` | router |  |  | `Router_dispatch_to_DesktopWidget_onTargetDecided` |
| `0x004a4410` | `Router_dispatch_to_DesktopWidget_onPreWarp` | router |  |  | `Router_dispatch_to_DesktopWidget_onPreWarp` |
| `0x004a44d0` | `Router_dispatch_to_DesktopWidget_onPostWarp` | router |  |  | `Router_dispatch_to_DesktopWidget_onPostWarp` |
| `0x004a45d0` | `Router_dispatch_to_System_onPreCutSceneCancel` | router |  |  | `Router_dispatch_to_System_onPreCutSceneCancel` |
| `0x004a4720` | `Router_dispatch_to_System_onPostCutSceneCancel` | router |  |  | `Router_dispatch_to_System_onPostCutSceneCancel` |
| `0x004a4880` | `DebugConsole_invokeLua_onDebugInput` | inbound_invoker |  |  | `FUN_008a4880` |
| `0x008c7050` | `LuaEngine_registerClassDerivedFrom` | named |  |  | `FUN_00cc7050` |
| `0x008c71f0` | `LuaEngine_finalizeClassDef` | named |  |  | `FUN_00cc71f0` |
| `0x008c7200` | `ClassRegistry_lookupAndCheckCategoryTag_wrapper` | named |  |  | `FUN_00cc7200` |
| `0x008c7210` | `IsInstanceOf_dynamic_dispatch_luaClassChainWrapper` | dispatcher |  |  | `FUN_00cc7210` |
| `0x008c7b90` | `Actor_readBindingUInt` | named |  |  | `FUN_00cc7b90` |
| `0x008c7be0` | `Actor_readBindingBool` | named |  |  | `FUN_00cc7be0` |
| `0x008c7de0` | `Actor_readBindingFloat` | named |  |  | `FUN_00cc7de0` |
| `0x008c98a0` | `Widget_perFrameTick_externalEntryPoint` | named |  |  | `FUN_00cc98a0` |
| `0x008ceac0` | `Lua_invokeFunctor_withTimeProgress` | named |  |  | `FUN_00cceac0` |
| `0x008d25c0` | `Lua_callMethod_storedCtx` | named |  |  | `FUN_00cd25c0` |
| `0x008d2630` | `CoroutineContext_findPendingCallback` | named |  |  | `FUN_00cd2630` |
| `0x008d27d0` | `CoroutineContext_isTrackingEnabled` | named |  |  | `FUN_00cd27d0` |
| `0x008d2860` | `CoroutineContext_pushResumeChecker` | named |  |  | `FUN_00cd2860` |
| `0x008d28c0` | `CoroutineContext_pushEndCallback` | named |  |  | `FUN_00cd28c0` |
| `0x008d6e60` | `DispatcherC_tickStackBased` | dispatcher |  |  | `FUN_00cd6e60` |
| `0x008d7a30` | `LuaClass_resolveTypeChainStart` | named |  |  | `FUN_00cd7a30` |
| `0x008d7fe0` | `LpbLoader_getLpbVersion` | named |  |  | `FUN_00cd7fe0` |
| `0x008d8100` | `LuaClass_walkParentChain_checkClassId` | named |  |  | `FUN_00cd8100` |
| `0x008d8870` | `ClassRegistry_lookupOrErrorPending` | named |  |  | `FUN_00cd8870` |
| `0x008d8990` | `LuaGameEngine_installBootBindings` | named |  |  | `FUN_00cd8990` |
| `0x008d91e0` | `ClassRegistry_addDerivedClass` | named |  |  | `FUN_00cd91e0` |
| `0x008d9c10` | `ClassRegistry_clearPendingFlag` | named |  |  | `FUN_00cd9c10` |
| `0x008d9c60` | `ClassRegistry_lookupAndCheckCategoryTag` | named |  |  | `FUN_00cd9c60` |
| `0x008da330` | `Widget_perFrameUpdate_ticksAll3Dispatchers` | dispatcher |  |  | `FUN_00cda330` |
| `0x008e16c0` | `ClassEntry_isInNoncreatableCategorySet_3tags` | named |  |  | `FUN_00ce16c0` |
| `0x008e1840` | `DispatcherA_retryEventViaCachedFunctor` | dispatcher |  |  | `FUN_00ce1840` |
| `0x008e44d0` | `BindingStorage_writeField_lowLevel_byBindingId` | named |  |  | `FUN_00ce44d0` |
| `0x008e4550` | `BindingStorage_readField_lowLevel` | named |  |  | `FUN_00ce4550` |
| `0x008e45d0` | `BindingStorage_writeField_lowLevel` | named |  |  | `FUN_00ce45d0` |
| `0x008e5290` | `BindingStorage_readField_dispatchByType` | dispatcher |  |  | `FUN_00ce5290` |
| `0x008ede40` | `LuaClass_resolveOrRegisterClassByName` | named |  |  | `FUN_00cede40` |
| `0x008ef510` | `DispatcherA_tickWithTimeoutBudget` | dispatcher |  |  | `FUN_00cef510` |
| `0x008f1060` | `Lua_createCallableFunctor_cached` | named |  |  | `FUN_00cf1060` |
| `0x008f4680` | `luaL_loadbuffer` | named |  |  | `FUN_00cf4680` |
| `0x008f5ec0` | `UIEventDispatcher_fireOneEvent_5argMax` | dispatcher |  |  | `FUN_00cf5ec0` |
| `0x008f6080` | `UIEventDispatcher_processQueuedEvents_timed` | dispatcher |  |  | `FUN_00cf6080` |
| `0x00908180` | `lua_luaGameEngineLoad` | named |  |  | `FUN_00d08180` |
| `0x009082a0` | `lua_lge_time_or_clock` | named |  |  | `FUN_00d082a0` |
| `0x009083b0` | `lua_lge_setLoopInterval` | named |  |  | `FUN_00d083b0` |
| `0x00908a10` | `lua_luaGameEngineRequire` | named |  |  | `FUN_00d08a10` |
| `0x00908e50` | `lua_luaGameEngineRequireEnd` | named |  |  | `FUN_00d08e50` |
| `0x00908ed0` | `lua_lge_assert` | named |  |  | `FUN_00d08ed0` |
| `0x009090c0` | `lua_lge_error` | named |  |  | `FUN_00d090c0` |
| `0x009093b0` | `lua_lge_pcall` | named |  |  | `FUN_00d093b0` |
| `0x009094a0` | `lua_lge_getWork` | named |  |  | `FUN_00d094a0` |
| `0x00909810` | `lua_lge_syncById` | named |  |  | `FUN_00d09810` |
| `0x009098d0` | `lua_lge_getIndividualIndex` | named |  |  | `FUN_00d098d0` |
| `0x009099b0` | `lua_metatable_newindex_handler` | named |  |  | `FUN_00d099b0` |
| `0x0090bb10` | `LpbLoader_ResumeChecker_ctor` | named |  |  | `FUN_00d0bb10` |
| `0x0090ca50` | `LpbLoader_loadIntoLuaState` | named |  |  | `FUN_00d0ca50` |
| `0x0090cfb0` | `LpbLoader_resolveAndFetch` | named |  |  | `FUN_00d0cfb0` |
| `0x0090d470` | `LpbLoader_popRingSlotAndLoad` | named |  |  | `FUN_00d0d470` |
| `0x00911c70` | `BitPacked_readByte_type1` | named |  |  | `FUN_00d11c70` |
| `0x00911cf0` | `BitPacked_readShort_type2` | named |  |  | `FUN_00d11cf0` |
| `0x00911d30` | `BitPacked_writeByte_type1` | named |  |  | `FUN_00d11d30` |
| `0x00911db0` | `BitPacked_readUint24_type3` | named |  |  | `FUN_00d11db0` |
| `0x00911e50` | `BitPacked_readUint32_type4` | named |  |  | `FUN_00d11e50` |
| `0x00911e90` | `BitPacked_writeShort_type2` | named |  |  | `FUN_00d11e90` |
| `0x00911fd0` | `BitPacked_writeUint24_type3` | named |  |  | `FUN_00d11fd0` |
| `0x00912080` | `BitPacked_writeUint32_type4` | named |  |  | `FUN_00d12080` |
| `0x0093a380` | `Crc32_standard_sliceBy8_poly_0xEDB88320` | named |  |  | `FUN_00d3a380` |
| `0x0093aae0` | `SqexCrypt_Crc32_finalize_returnValue` | named |  |  | `FUN_00d3aae0` |
| `0x0093ab60` | `SqexCrypt_Crc32_init_computeOverBuffer` | named |  |  | `FUN_00d3ab60` |
| `0x009430d0` | `Socket_send_thin` | named |  |  | `FUN_00d430d0` |
| `0x00943140` | `Socket_recv_thin` | named |  |  | `FUN_00d43140` |
| `0x00944610` | `Socket_send_vtable_inner` | named |  |  | `Sqex::Socket::SocketImpl::vfunc8` |
| `0x00944690` | `Socket_recv_vtable_inner` | named |  |  | `Sqex::Socket::SocketImpl::vfunc9` |
| `0x009447e0` | `Socket_RecvTCP_worker` | named |  |  | `FUN_00d447e0` |
| `0x00944950` | `Socket_RecvFrom_worker` | named |  |  | `FUN_00d44950` |
| `0x00944ae0` | `Socket_StateMachineTick` | named |  |  | `FUN_00d44ae0` |
| `0x0094dc60` | `NetIo_PollStep_outer` | named |  |  | `FUN_00d4dc60` |
| `0x009511c0` | `NetIo_HandleReadySockets` | named |  |  | `FUN_00d511c0` |
| `0x009514f0` | `NetIo_PollStep` | named |  |  | `FUN_00d514f0` |
| `0x00957530` | `NetIo_SelectWait` | named |  |  | `FUN_00d57530` |
| `0x009a2be0` | `IpcPacket_buildHeader` | named |  |  | `FUN_00da2be0` |
| `0x009a45b0` | `LobbyClient_ensureConnection` | named |  |  | `FUN_00da45b0` |
| `0x009a4790` | `InitOperationStep_kick` | named |  |  | `FUN_00da4790` |
| `0x009a4950` | `LobbyClient_setupAndConnect` | named |  |  | `FUN_00da4950` |
| `0x009a4b80` | `LobbyClient_decode_LobbyLoginPayload` | named |  |  | `FUN_00da4b80` |
| `0x009a4c20` | `LobbyClient_decode_CharacterList` | named |  |  | `FUN_00da4c20` |
| `0x009a4d80` | `LobbyClient_decode_RetainerList` | named |  |  | `FUN_00da4d80` |
| `0x009a4f30` | `LobbyClient_CloseLobbyConnection` | named |  |  | `FUN_00da4f30` |
| `0x009a4f80` | `LobbyClient_onSuccessfulLobbyLogin` | named |  |  | `FUN_00da4f80` |
| `0x009a5030` | `LobbyClient_onSuccessfulServiceLogin` | named |  |  | `FUN_00da5030` |
| `0x009a5110` | `LobbyClient_onSuccessfulGameLogin` | named |  |  | `FUN_00da5110` |
| `0x009a5190` | `LobbyClient_onSuccessfulCharaMake` | named |  |  | `FUN_00da5190` |
| `0x009a5300` | `LobbyClient_gcCompletedOpsAndCheckActive` | named |  |  | `FUN_00da5300` |
| `0x009a5410` | `LobbyLoginOperationStep_onLobbyLogin` | named |  |  | `FUN_00da5410` |
| `0x009a54d0` | `LobbyClient_queueLoginOperation` | named |  |  | `FUN_00da54d0` |
| `0x009a55c0` | `LobbyClient_doStartLobbyLogin` | named |  |  | `FUN_00da55c0` |
| `0x009a6320` | `LobbyClient_decode_WorldList` | named |  |  | `FUN_00da6320` |
| `0x009a64b0` | `LobbyClient_decode_GameLoginPayload` | named |  |  | `FUN_00da64b0` |
| `0x009a7040` | `LobbyLoginOperationStep_kick` | named |  |  | `FUN_00da7040` |
| `0x009a76b0` | `LobbyClient_decode_ServiceLoginPayload` | named |  |  | `FUN_00da76b0` |
| `0x009a79d0` | `LobbyClient_decode_CharaMakePayload` | named |  |  | `FUN_00da79d0` |
| `0x009a84c0` | `LobbyLoginOperation_ctor` | named |  |  | `FUN_00da84c0` |
| `0x009a88e0` | `InitOperationStep_ctor` | named |  |  | `FUN_00da88e0` |
| `0x009a89f0` | `LobbyLoginOperationStep_ctor` | named |  |  | `FUN_00da89f0` |
| `0x009a9880` | `LobbyLoginOperation_buildAndSendPacket` | named |  |  | `FUN_00da9880` |
| `0x009a9ec0` | `LobbyClient_dispatchInbound_LobbyLogin` | dispatcher |  |  | `FUN_00da9ec0` |
| `0x009aa070` | `LobbyLoginOperation_sendAck32` | named |  |  | `FUN_00daa070` |
| `0x009aa190` | `LobbyLoginOperation_sendCharaMakeOrSubOp` | named |  |  | `FUN_00daa190` |
| `0x009aa740` | `LobbyLoginOperation_sendAck40` | named |  |  | `FUN_00daa740` |
| `0x009aa950` | `LobbyClient_dispatchInbound_GameLogin` | dispatcher |  |  | `FUN_00daa950` |
| `0x009aa9f0` | `LobbyClient_dispatchInbound_ServiceLogin` | dispatcher |  |  | `FUN_00daa9f0` |
| `0x009aac30` | `LobbyClient_dispatchInbound_CharaMake` | dispatcher |  |  | `FUN_00daac30` |
| `0x009ab290` | `LobbyConnection_ctor` | named |  |  | `FUN_00dab290` |
| `0x009ad750` | `LobbyRequestCallback_ctor` | named |  |  | `FUN_00dad750` |
| `0x009ad770` | `LobbyOperation_ctor` | named |  |  | `FUN_00dad770` |
| `0x009ae010` | `ZoneClient_forwardOutbound` | named |  |  | `FUN_00dae010` |
| `0x009ae1e0` | `ZoneClient_sendInitialHandshake` | named |  |  | `FUN_00dae1e0` |
| `0x009ae3b0` | `ZoneClient_pumpConnectionState` | named |  |  | `FUN_00dae3b0` |
| `0x009af850` | `ZoneIpcPacket_acquireSendSlot` | named |  |  | `FUN_00daf850` |
| `0x009b06a0` | `ZoneIpcPacket_finalizeAndSend` | named |  |  | `FUN_00db06a0` |
| `0x009b35e0` | `PacketBufferBase_parseSegment` | named |  |  | `FUN_00db35e0` |
| `0x009b3880` | `PacketBufferBase_parseChunk` | named |  |  | `FUN_00db3880` |
| `0x009b3e30` | `ChatClient_dispatchOutbound_generic` | chat |  |  | `FUN_00db3e30` |
| `0x009b4020` | `ChatClient_sendInitialHandshake` | chat |  |  | `FUN_00db4020` |
| `0x009b4300` | `PacketBufferTmpl_debugRecvLog` | named |  |  | `FUN_00db4300` |
| `0x009b5010` | `ChatIpcPacket_acquireSendSlot` | chat |  |  | `FUN_00db5010` |
| `0x009b5300` | `ProtoChannel_dispatchPacketById` | dispatcher |  |  | `ProtoChannel_dispatchPacketById` |
| `0x009b5a90` | `ChatIpcPacket_finalizeAndSend` | chat |  |  | `FUN_00db5a90` |
| `0x009b6140` | `PacketBufferBase_tryGetNextPacket` | named |  |  | `FUN_00db6140` |
| `0x009b6590` | `PacketBufferTmpl_processOnePacket` | named |  |  | `FUN_00db6590` |
| `0x009b67e0` | `PacketBufferTmpl_processAllPackets` | named |  |  | `FUN_00db67e0` |
| `0x009b6d20` | `PacketBufferTmpl_tryGetNextTyped` | named |  |  | `FUN_00db6d20` |
| `0x009c1490` | `ZoneIpcPacket_bodyPtr` | named |  |  | `FUN_00dc1490` |
| `0x009c1cf0` | `ZoneIpcPacket_buildHeader` | named |  |  | `FUN_00dc1cf0` |
| `0x00a40820` | `ChatIpcPacket_bodyPtr` | chat |  |  | `FUN_00e40820` |
| `0x00a40a60` | `ChatIpcPacket_buildHeader` | chat |  |  | `FUN_00e40a60` |

## Wire semantics (net-new — ffxivDecomp 2026-05-28)

Cross-referenced from the 2026-05-27/28 ffxivDecomp session, not byte-verified. Full context: `docs/ffxivdecomp_2026-05-28_session_integration.md`.

- **0x12d** has a discriminator byte at **+0x28** (immediate-vs-queued; command vs SIMPLE noticeEvent). Integrity is a **uint32 standard CRC32 at +0x24** (poly 0xEDB88320 = `Sqex::Crypt::Crc32` @ `FUN_00d3a380`) over the 128B payload at +0x49 — NOT the 32 bytes at +0x29 (a command-specific hash/id). CRC = transport integrity, not anti-cheat. 8 commandName flags: commandRequest / commandJudgeMode / commandDefault / commandWeak / commandForced / commandContent / widgetCreate / macroRequest.
- **Per-class `_updateWork` divergence:** CharaBase + Director -> **0x12f** (56B string-path WorkSync, predictive UpdateQueue); Item -> **0x132** (24B, NO WorkPath, no predictive enqueue); GroupBase -> **0x133** (56B, byte-identical to 0x12f, per-instance @ instance+0x68, server-authoritative). The 0x12f/0x133 split is a server-side routing hint (actor-table vs group-table).
- **WorkSync is SUBSCRIBE-based** (not broadcast-all): client requests binding-ids via **0x135**; server pushes only subscribed bindings. 0x3f2/0x3f3/0x3f4 (hp/hpMax/level) always force the server query. Inbound chain: `docs/worksync_inbound_chain.md`. Outbound two-queue split: WorkSync vtable[0xec] vs CommandUpdater 280B records (`docs/group_system_decomp.md`).
- **0x18a is NOT a linkshell variant** (it is BULK_PAIR_SET — existing pin correct); PropertyUpdater has no dedicated opcode (it is EntryLinkShellBuilder vftable[12]).

## SEQ-005 cutscene-hang relevance

Two registrars here name functions the SEQ-005 kick-dispatcher work
has been circling (see the SEQ-005 memory chain):

- `_cancelNotice` registrar — the Notice stream's cancel path (1.x has 4 event types — Command/Talk/Emote/Push — each with a call*/do* pair, plus an orthogonal Notice/cancel mode).
- `_fadeInNowLoadingForNoticeEventJustInArea` — previously located via MyPlayer vtable slot 66 as the kick-dispatcher clearer; ffxivDecomp
  independently names its registrar, corroborating that anchor and
  giving the registrar→thunk (LAB_0071e3f0) link to force-disassemble next.
