# Original client offline bridge

## Fresh-profile readiness audit — v109 on audit1

[Readiness audit](../../analysis/native/OFFLINE_READINESS_AUDIT.md). The user authorized testing natural progression in a separate profile. audit1 is a distinct package using normal seeds, no QA defines and the existing lab keystore; lab6/v107/save remain intact. The exact v107 library on a clean profile (v108) reproduced login stuck at67% because menu unlocks required an unreached region3 record. v109 adds `read_local_menu_progress`: an absent region3 with MaxRegion<3 means locked, while missing reached records or malformed existing records still fail. No save/progress/resource fabrication. ARM64 parser regression passed; audit1 updated with adb install -r and reached home level1/1500gold/0gems/0EXP/empty inventory. Gameplay/restart tests are ongoing; do not call progression validated. Main-profile cold start in airplane mode passed but RecoverPower NRE persists. v109 has not been promoted to lab6 yet.

## Current collective fusion — v107

[Authorized offline policy and evidence](../../analysis/native/ONEKEY_OFFLINE_V107.md). The user explicitly authorized local selection: same-ID/quality triples for qualities1–3, cascading through quality4, highest-level root retaining UID, equipped items excluded. Root ties use lowest UID; materials use lowest levels/highest UID. This is not claimed to reproduce unknown ET server selection. Original recipes, models and the independent collective callback remain authoritative for compatibility.

Checked hook `ET.EquipHelper.OneKeyCompoundEquip()` at `0x2C2D50C`, static `ETTask<bool>`, entry bytes `FF0301D1 F31300F9 FD7B03A9 FDC30091`. Uses original `G2C_OneKeyCompoundEquip` (final EquipUids and material CostEquips) and `OnGetOneKeyCompoundEquip` after a single full inventory commit/refresh/readback. No redirection to individual fusion or legacy ProtocolManager. Validates all saved/live fields, count, uniqueness, wear, parts, current recipes and initial original caches; shared fusion guard includes wear. Snapshots/journal/result are synced; rollback keeps recovery evidence until memory restoration succeeds. No currency charge/refund or new QA seeding; shop visuals unchanged.

Installed build `v107-onekey-atomic-durable`, APK SHA256 `5F244C374A26EAA44558E8F61C93C3A88830E1EC403EB7BBD420043E3A0437F0`, .so `3973702C547BE9C709EF62EA0AA1E82CFBFD7C159D1FBEC05C462ADE068CEB2A`. Existing lab6/keystore/firstInstallTime preserved with adb install -r. All original/reference APKs preserved. v105 external diagnostics were rejected by automatic approval review and never installed; v106/v107 keep complete diagnostics only in app-private filesDir (`kinja_onekey_dryrun.json`, `kinja_onekey_preflight.json`, `kinja_onekey_last_commit.json`). These are evidence, not inventory inputs.

Bootstrap dry-run validates manager/config/model APIs without consuming or invoking success. Device projection:195 current items,69 steps,138 materials,31 final result UIDs,57 survivors. ARM64 planner/real JSON/journal fixture tests pass. v106 restart comparison captured all195 UID/ID/quality/level/part/wear records unchanged and balances unchanged; see report for final v107 restart evidence. Collective consuming gameplay/success visuals/result persistence remain unvalidated because the batch includes real save items and the nondestructive restriction remains. Do not call projection gameplay validation or seed pieces to create a test.

Final v107 restart PIDs20496→22535 also passed the internal195-item preflight/model checks, with unchanged balances/projection and observed bag. External comparison in kinja-v107-validation.json has195 rows before/138 after (all captured fields equal;57 post-restart rows suppressed by logd). Installed APK hash matches the artifact. No consuming collective callback/commit was executed; result persistence remains pending. Final fsync/journal fixture evidence is kinja-v107-json-journal-regression.txt.

## Historical collective investigation before policy authorization

See [1.9.1 collective contract investigation](../../analysis/native/ONEKEY_1_9_1_INVESTIGATION.md). The modern ET request has no selected UIDs and awaits server selection; its own success response/callback are confirmed. The legacy ProtocolManager path aggregates all matching materials without enforcing the configured count, allocates another UID and has separate refunds/events. It must not be treated as the ET server implementation. No collective hook or new build was produced in this follow-up; installed v102 is unchanged. Device screenshot/logs were captured without consuming items. Exact server selection needs further evidence, or an explicitly authorized documented offline policy; do not infer permission from silence.

> Current implementation and issue-triage guide: [`../../MAINTENANCE_GUIDE.md`](../../MAINTENANCE_GUIDE.md). The build-by-build notes below are a historical log through the earlier combat/revive work; use the main guide for current hooks, current progression behavior, and validation status.

`offline_bridge.cpp` is an ARM64 JNI adapter around the original compiled Unity game. It changes checked original method entries **in process memory only**, after checking all sixteen expected entry bytes. It does not modify the original APK or `libil2cpp.so` on disk.

## Current equipment fusion — v102

See [exact 1.9.1 findings, recipes and evidence](../../analysis/native/FUSION_1_9_1_V102.md) and [functional audit](../../FUNCTIONAL_AUDIT.md). The saved root quality was discarded by a second lookup overwriting the modified JSON record. Eligibility caches were also calculated before function102 was opened; the original update method returned early. The bridge now retains the changed record, rebuilds original caches after `SetOpeningFunc`, and validates the **current** `EquipQualityCfg.cost` recipe through managed APIs. Higher ranks require one material and may use same part/different ID or lower material quality; UID2=0 is the original UI contract for one material.

All models/UIDs/wear/parts/save-live values are validated before consumption. Journal/snapshots, inventory readback and rollback remain; original inventory callback precedes the original fusion-success callback so result UI sees the applied item. No currency is charged. Upgrades/chest purchases are refused during fusion reentry. Equipped materials remain refused pending their original gameplay contract. No direct managed-object offset writes are added. QA seeding was not enabled in these builds and no new items were created in the save.

The checked `UICompoundEquipTips.UpdateCompoundList` observer at `0x12F8530` has prologue `FF4307D1 EAAB00FD E923166D FC6F17A9`. It calls the original trampoline and reads the original `_selected` list on Unity thread; `Fusion UI selected` identifies root/material UIDs before a consuming test. Do not choose test items from visual position alone.

Final installed BuildId: `v102-fusion-atomic-recipes-selection-evidence`; APK SHA-256 `D4B367A3E54B7A30093F6237AA54ECBD36B3A61DBCDEE25DB121498FEB87E5FC`; ARM64 `.so` `A7DED0DD84EC19309A7E58D3BD8E2537D0655F24E1FC56C0CDDC6492608DAD7D`. Signed with the existing lab keystore; `adb install -r` preserved lab6/version1.9.1/code51/firstInstallTime. Use a **new** BuildId for rebuilding; preserve the original APK and reference builds, including v57:

```powershell
.\offline\build.ps1 -BuildId NEW-ID -PackageName com.habby.kinjarun.offline.lab6 -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-NEW-ID.apk
```

Static evidence: reconstructed exact APK library (`A2A4510A...`) includes original `CheckCompound`, cache builder, UI selection/state machine and callbacks. Simple-XOR `code-analysis.so` alone does not undo stripe permutation. The current recipe table and red+4 rank differ from parts of the player reference; Rare/Excellent describe the same blue quality3 in this copy.

Device bridge evidence: v99/v102 logs show QA UIDs28–32 unchanged while can/relate switches0→1 after original cache update. A standalone ARM64 regression (`build/kinja-v99-json-regression.txt`) reproduced the old quality-loss defect and passed corrected source snippets without accessing game save.

Visual/gameplay/save: one controlled fusion of **existing v84 QA clones**28/29/30 at20:23:48 consumed only29/30 and persisted root28 q2→q3, EquipId3002/part3/Nv.5/unequipped retained. Original result screen: Excellent, limit20→30, ATK31→43, HP restoration attribute. Process restart reloaded root28 q3 and the bag showed the blue Nv.5 item and three remaining green Nv.5 items (six before). Saved gold7490720/gems938307/EXP1000640 unchanged. See `build/kinja-v102-fusion-test-plan.json`, pre-fusion/result/after-restart screenshots and logs listed in the report. JSON baselines have partial log coverage; they are not full private-save exports.

Limits: higher-rank one-material recipes, equipped-root/material cases, failure injection/reentry and actual combat effect were not exercised. Collective fusion is still the independent online `OneKeyCompoundEquip` route; UI prechecks qualities1–3, while server root/group selection is not confirmed. It was not hooked, redirected or tested with real items. Next work: establish its transaction contract. Shop visuals were outside scope. Historical v79/v80 commit logs lacked quality readback and cannot prove persistent rank increases.

JNI export: `Java_local_kinja_OfflineActivity_configure(String filesDir, AssetManager assets)`. Library filename: `libkinjaoffline.so`. The wrapper calls configure before `UnityPlayerActivity.onCreate`; the worker waits for the original IL2CPP library/domain and attaches before resolving metadata. All game singleton creation, deserialization and callbacks happen on the Unity thread in the original `AccountInit` call.

## Build

Run `offline/native/build-native.ps1` with the portable Android NDK r27d path. Equivalent command:

```powershell
& 'D:\Programacao\kinja-run\tools\android\native-toolchain\android-ndk-r27d\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android23-clang++.cmd' -std=c++17 -O2 -Wall -Wextra -fPIC -shared -static-libstdc++ '-Wl,-z,max-page-size=16384' 'D:\Programacao\kinja-run\offline\native\offline_bridge.cpp' -o 'D:\Programacao\kinja-run\offline\build\arm64-v8a\libkinjaoffline.so' -llog -landroid -ldl
```

The workspace sandbox prevented clang from resolving its own executable; compilation worked with the tool's approved `require_escalated` execution. That is a host tool limitation, not a required Android permission.

Compiled output is `offline/build/arm64-v8a/libkinjaoffline.so`. All ELF load segments have 16 KiB alignment (`p_align=0x4000`), ARM64 Android API 23, static libc++.

Package the `.so` under `lib/arm64-v8a/` and every `offline/assets/seed_*.json` under APK `assets/offline/`.

## Original APIs and data

The exact JSON overload is `Newtonsoft.Json.JsonConvert.DeserializeObject(System.String,System.Type)`. Classes and signatures are checked; ambiguous/missing matches abort with logs. Closed original singleton parent types supply `get_Instance`. Parameters are passed through `il2cpp_runtime_invoke`; native object layouts are not written directly.

Profile deliveries use original ET models/callbacks:

| Seed | Model | Original callback |
|---|---|---|
| seed_items.json | ET.G2C_ItemInfo | ItemManager.OnItemInit |
| seed_equipment.json | ET.G2C_EquipData | EquipManager.OnEquipDataInitResp |
| seed_talents.json | ET.G2C_TalentData | TalentManager.OnTalentDataResp |
| seed_heroes.json | ET.G2C_HeroInfo | HeroManager.HeroDataInit |
| seed_regions.json | ET.G2C_RegionData | SceneDataManager.OnGetSceneProgress |
| seed_guide.json | ET.G2C_PlayerVariable | PlayerGuideManager.OnGetGuideData |
| seed_player.json | ET.G2C_PlayerData | PlayerDataManager.OnPlayerValueDataResp |
| seed_shop_chests.json | ET.G2C_ShopChestState | ShopManager.OnGetChestState |

All required list fields in the seed payloads are non-null. Original Birth data supplies hero 1001 and gold 1500; original Variable `tili_limit` supplies power 30. Hero level 1/star 0, empty inventories, local UID/PID 1 and a fresh tutorial are local initialization choices. The preserved originals are documented in `analysis/bundles/FINDINGS.md` and `offline-bootstrap.md`.

The bridge calls original `ObscuredLong.op_Implicit(long)` then `il2cpp_field_set_value` for `ProtocolManager._playerId`, sets original UID properties and server time, invokes every original response callback, and completes the replacement account/session task. It does not call Habby account login or its optional shop/event batch.

## Combat transport hooks

The original FightRecordManager parent computes attack data **before** calling FightHelper. The bridge substitutes completed **original closed ETTask instances**, created with `Create(false)` and `SetResult`, at:

- `ET.FightHelper.EnterFight(int regionId,int enemyCount)` — VA `0x29c3728`, returns ETTask<bool>(true).
- `ET.FightHelper.KillEnemy(int enemyId,int instanceId,bool boss,bool elite)` — VA `0x29c4abc`, returns original ET.G2C_KillEnemy from `seed_kill_enemy.json`.
- `ET.FightHelper.EndGame(int stage,long score,int buds,int perfect,int skills)` — VA `0x29c3f44`, returns original ET.G2C_EndGame from `seed_end_game.json` and sets its region ID from the local EnterFight call.

The baseline attack overrides are zero to preserve original computed values. Combat reply metadata resolution is optional: a missing exact task method leaves the core bootstrap installed and emits a clear failure log. Runtime prologue mismatch also refuses the affected hook.

## Diagnostics and limits

Logcat tag: `KinjaOffline`. Status: app files directory `offline_bootstrap_status.json`. Persisted initial seed files: `offline_save/seed_*.json`; existing copies are read on the next launch. Last local end-fight request: `offline_save/last_fight.json`.

Combat remains in the original game. The local EndGame hook infers a clear from the original `FightDataManager.CurrentRegion > RegionCount` counters, returns pass/new-score flags, persists inventory items and region pass/best score/progress, then refreshes the original region manager. For region 2, a first clear grants the configured bronze chest (item 100001); the first 100% clear grants two configured talent crystals (item 40001). A region 2 clear also unlocks region 3, advances the selected chapter, and creates its local region record. The v17 bootstrap repairs this unlock for existing saves where region 2 is already passed but `MaxRegion` remained 2. Device evidence: the first clear logged `passed=1`, `firstWin=1`, `milestone100=1` and two `AddLocalItem` calls; after installing v17, bootstrap logged the repair and the home screen displayed `Capítulo 2`. The client configuration supplies no gold for region 2's milestone or first-win reward; server-owned gold/experience/drop formulas remain unknown and are zero. The reward inventory counts have not yet been independently read back from the non-debuggable app's private storage. This implementation is scoped to the normal solo mode and region 2's verified reward configuration.

The status `initialized` means the eight original callbacks completed successfully; compile success alone is not a playable-game claim. Authorized device validation is documented per build below.

The v35 local Shop seed supplies chest state IDs 101, 115 and 108, which `TreasureBoxItem.UpdateItem` requires through `ShopManager.GetShopChestState`. On the authorized Xiaomi, updating v34 to v35 with `adb install -r` preserved the existing profile, the new seed was created once, and the Shop screen opened without the previous `NullReferenceException`. This validates panel opening only: chest opening/rewards remain untested, and the Shop still displays real-money gem products that must be disabled for the offline product.

The v39 Shop/payment policy calls the original `UITreasureBoxPage.UpdateBuyDiamond` (RVA `0x13DFB00`) through a checked trampoline, then finds `obj/ContentPanel/Top/BuyDiamond` on the original page Transform and calls the original `GameObject.SetActive(false)`. The path comes from `UITreasureBoxPage.prefab` inside `clientgameobjectsui48.ht`; the disabled parent contains `imgTitleBg`, six real-money `BuyDiamondItemN` cards, and `diamondShopEffect`. v39 device logs confirm the transform lookup and section deactivation, and its screenshot shows the real-money catalog gone while both chest cards remain. `PurchaseManager.Purchase(int)` (RVA `0x2AA0120`) is also intercepted to return without forwarding to external purchase. The hook is installed, but no paid click was tested; chest/gem flows remain untested and may still depend on online server state. See `FUNCTIONAL_AUDIT.md` for APK SHA, screenshots and limitations.

The v18 quality-of-life bootstrap seeds local `PlayerStatus` IDs `8` (SkipAd) and `9` (InfinitePower), with expiration timestamps at 2100-01-01. On the authorized Android device, both `CheckPlayerStatusValue` calls returned `1`, the home UI rendered the infinite-energy icon, and the app completed local initialization. The battle button still displays its original `x5` label; an actual battle is needed to confirm whether energy is deducted or this is only stale UI text. The long timer currently rendered by the original UI is cosmetically excessive and should be replaced with a clean persistent indicator. The launch log also contains a `HangUpHelper.GetHangUpData` null-reference while offline; it did not prevent the home screen from rendering, but hang-up rewards remain an online-only feature and should be hidden/neutralized in a later pass.

The v19 quality-of-life update additionally seeds `AdditionalResurrection` status ID `10` and hooks the original `AdManager.ShowRewardAdWithCallback` (VA `0xF08120`, exact first 16 bytes checked against the supplied client). It invokes each rewarded-ad callback with `AdFinishType.Complete`, so the corresponding rewards/revive are available immediately without opening an ad. Runtime evidence confirms the hook was installed and status ID `10` returns `1`; the actual death/revive UI path has not yet been exercised on device. Chapter 2 also receives a local test assist at offline fight entry: `HeroInvincible=true` and `LevelSpeedRate=4.0`. The v19 launch test did not enter a fight, so the setters and duration reduction remain to be validated by a manual run. This assist is scoped to region 2; other chapters keep their original difficulty.

Build v20 replaces the ineffective early fight-entry assist with a checked hook on `FightDataManager.ResetProperty` (RVA `0x27DB454`). It calls the original method through an ARM64 trampoline, then sets the region-2 test run's `RunningHeroAtk` and `RunningHeroRealAtk` to `9999`, after the original stat reset. Build v20 also replaces `UIResurgenceTips.OnClickAd` with a local full-HP response passed to the original `GameSceneManager.OnResurgenceSucceed`, then closes the revive panel; this avoids the failed online resurgence request that v19 still reached after completing an ad callback. Device startup logs confirm both hooks installed on the Xiaomi Android 16 device and the app initialized. Damage impact and successful in-game revive have not yet been exercised; v20 is installed and awaiting the user's gameplay check. APK: `build/kinja-run-offline-lab-v20-testdamage-localrevive.apk` (SHA-256 `2c9c02a7ef6df4d86378fecb017a94bf6ce72601d489fe341f5515a68bea3c6a`).

Build v24 restores the original revive UI flow: `UIResurgenceTips.OnClickAd` and its ad-completion callback are no longer replaced, so the existing offline `AdManager.ShowRewardAdWithCallback` hook immediately reports `AdFinishType.Complete`. A checked hook on `FightHelper.Resurgence(bool)` at RVA `0x29C54C0` supplies the local `G2C_Resurgence` response (`Error=0`, `Type=0`, `Cost=0`, `ResurgenceHp=1.0`) through the original closed `ETTask<G2C_Resurgence>` factory. This keeps the original `DoAdResurgence` coroutine responsible for the scene/UI cleanup and avoids direct `OnResurgenceSucceed` plus a synthetic `CloseTween`. Startup logs on the authorized Xiaomi device confirm the typed task factory and `FightHelper.Resurgence` hook installed. The in-game panel dismissal and input restoration still need the user's validation. APK: `build/kinja-run-offline-lab-v24.apk`.

Build v25 addresses the remaining frozen gameplay state from the user's screenshot. Static analysis of `ProtocolManager.HandleResurgence` showed that the online response is routed to `GameSceneManager.OnResurgenceSucceed` before the awaiting revive coroutine completes. The v24 local task skipped that gameplay callback, explaining how the coroutine could dismiss the menu while leaving the player/game state inactive. The local hook now invokes the same success handler with the seeded full-health response, then completes the original task so the original coroutine still closes the panel. The first v25c runtime test exposed a namespace lookup error: the APK defines `GameSceneManager` in the global namespace, not `ET`. Build v26 corrects the class lookup. The v26 APK was packaged, signature-verified, installed, and launched on the authorized Xiaomi device. Startup logs confirm bridge initialization; the in-game callback and resumed gameplay still need validation during a revive. APK SHA-256: `66AA377D0050CC10DF2417FBC0EF6BE713C8ED2848BFB493A64C5EC5E55F4C4B`.

Build v27 adds a Chapter 2-only difficulty assist after `FightDataManager.ResetData`: it sets the original `HeroInvincible` flag for region 3, preventing enemy attacks from draining player HP; the existing 9999 hero attack assist remains active for regions 2 and 3. The v27 APK was packaged, signature-verified, installed, and launched on the device. Runtime confirmation that the Chapter 2 setter succeeds and the full run is easier remains pending the next play session. APK SHA-256: `8B18F61B8B1BDC3F32CF89735F1A28D716995E679F24FD8BF0CCF0643C172A34`.

Build v28 responds to the user's report that the 9999 attack value did not increase real damage. v27 logs showed successful writes after `ResetData`, but this did not prove the values survived the later `FightDataManager.ResetProperty` combat-stat recalculation. v28 calls the original `ResetProperty` through a checked trampoline, reapplies the Chapter 2 assists afterward, and reads both obscured attack properties back for runtime verification. The v28 APK was packaged, signature-verified, installed, and launched; in-fight readback and actual damage effect await the next gameplay check. APK SHA-256: `A11D495E9949501F57F0347DC2861C5F6AB0BFC85505B6B786031F2F5C26AC0C`.

ABI check: in this supplied client, `il2cpp_format_exception` at `0xd71ba4` and `il2cpp_format_stack_trace` at `0xd71c10` consume `x1=buffer,w2=size`; this bridge uses their buffer-writing three-argument signatures.

Correction after the user's v28 gameplay report: the global-stat edits did not change actual combat. Device logs show `set_HeroInvincible(System.Boolean)` could not be resolved, and the attempted ObscuredInt readbacks were ambiguous. Do not treat the v27/v28 attack or invulnerability assist as working.

Build v29a moves the Chapter 2 assist into the actual `FightDataManager.DeductEnemyHp` and `DeductPlayerHp` application paths. For local region ID 3 (displayed Chapter 2), player hits are raised to at least 9999 damage and non-forced incoming hits are reduced to 10%, rounded up; forced kills are preserved. The hook targets were matched to the original ARM64 function prologues, and startup logs on the connected Xiaomi confirm both hooks installed. That proves interception setup only; a combat run is still required to confirm damage behavior and that the full by-value IL2CPP arguments survive native forwarding. APK: `build/kinja-run-offline-lab-v29a-ch2-damage.apk` (SHA-256 `F5C6AA5E6EA9592A1EBF3AE44DC689F22F99DEE69C2301EA1C8C556DF486D2D7`).

HangUp v41/v42 replaces `ET.HangUpHelper.GetHangUpData()` at RVA `0x2964EB0` with a completed original `ETTask<G2C_HangUpData>` built from `seed_hangup.json` (zero counters, empty `Items`/`equips`). The target prologue is runtime `ff0301d1 f31300f9 fd7b03a9 fdc30091`; v40's incorrect expectation was refused by `patch_entry`. Device logs confirm the v41/v42 hook and deserialization, and opening the panel shows “Nenhum item...” without the previous NRE. Build v42 also replaces `RewardHangUp(short)` at RVA `0x2964F98` using `seed_hangup_reward.json` with empty rewards and zero costs. Hook installation is confirmed, but ADB taps on the green Receive button did not log a method call. Click behavior, quick return, persistence and counters remain unvalidated; panel opening alone does not make HangUp a complete offline feature.

## Atualização v44 — HangUp e energia

A disassembly 1.9.1 de `UITimeGainPanel.<OnClickReceive>d__45.MoveNext` explica o `RewardHangUp(0)` ausente: a rotina retorna cedo se `_items` estiver vazia e só chama a recompensa quando há pelo menos um item. Como `seed_hangup.json` usa `Items:[]`, o botão verde permanece sem efeito percebido. A opção grátis de Volta Rápida foi exercitada e chamou `RewardHangUp(1)`, mas a resposta seed é vazia; nenhuma moeda/item é concedida. Implementar recompensa ociosa local coerente com dados 1.9.1 ou comunicar indisponibilidade, sem esconder exceções. Opções com custo não foram tocadas.

Na v44, `UIMainPanel.DoOpen` chama o método original e desativa o nó estático `imgPower` em `Content/Page/Main/Bottom/LevelHandle/Unclear/btnUnclearFight/imgPower` via APIs Unity/IL2CPP originais e na thread Unity. Captura de dispositivo confirma `x5` oculto e o ícone `∞` preservado. Hook diagnóstico de `OnClickFight` lê o Power antes/depois por getter original e `ObscuredInt.GetDecrypted`: 0→0, entrada permitida em Capítulo 3/Fase 1/10. Isso não mede débito com Power positivo. O save existente é privado/não-debuggable e não foi alterado para criar saldo de teste. Hash, passos e capturas estão em `FUNCTIONAL_AUDIT.md`.

## Atualização v51 — HangUp local persistente

`GetHangUpData` calcula ouro/EXP usando `TimeGainCfg` do capítulo local, com limite de acúmulo de 6 horas. A coleta normal type 0 retorna IDs 101/104 e só avança `LastClaim` depois de confirmar o crédito. `OnClickReceive` não despacha quando a lista de itens está vazia; com acumulação visível, a v51 confirmou o toque e a resposta local.

Para creditar 101/104, o bridge usa `ObscuredInt.op_Implicit`, os setters originais `PlayerDataManager.set_PlayerGold/set_PlayerExp`, `SyncValueToShow` e leitura de retorno via `ItemManager.GetOwnItemNum`. Depois altera apenas `Coin` e `Exp` no `offline_save/seed_player.json` já existente, preservando os outros campos. v50 foi apenas runtime e o perfil voltou ao seed após reinício; v51 foi testada em dispositivo: 1500/0 → 1507/7 e os dois valores persistiram após reiniciar o processo. Não limpar o perfil para validar isso.

Volta Rápida type 1 usa a mesma taxa por seis horas, até 3 vezes/dia. A rota de anúncio grátis foi vista alcançar a callback em build anterior sem abrir anúncio, mas não foi repetida após a correção de persistência; o diálogo v51 observado só mostrou a opção de 50 gemas, que não foi tocada. Tipos pagos retornam erro local, mensagem de indisponibilidade em português e custo zero; a exibição desse erro pela UI não foi verificada. Consulte `FUNCTIONAL_AUDIT.md` antes de afirmar que Volta Rápida está validada.

Consumo de energia com Power positivo continua sem teste: save atual preservado tinha Power zero. A entrada de fase observada anteriormente foi 0→0. Não fabricar saldo alterando o save para medir consumo.

## Combat currency settlement (v52–v54)

Local gold is returned per original `G2C_KillEnemy` response and accumulated only for the active run. `EndGame` determines success from the original region counters. Losses and abandoned runs receive no persistent gold. On a successful run, the bridge applies only the not-yet-credited delta through the original `PlayerDataManager` setters and persists only `Coin`/`Exp` in the existing local player JSON. This handles a final KillEnemy reply that arrives after EndGame without duplicate credit.

The result UI reads `G2C_EndGame.Gold`. v54 retains that managed response with an IL2CPP GC handle and updates it through the original `set_Gold` API if a late kill arrives. It releases the handle on the next EnterFight. On-device v53 exercised the late balance delta (495 + 5) and v54 exercised a complete 500-gold victory whose kill replies all preceded EndGame; result, home balance (3002 after starting from 2502) and process-relaunch persistence agreed. The v54 late result-object update branch has not yet been directly observed. EXP from combat remains zero until a client-local formula is established.

## Local talent spending (v56)

`ET.TalentHelper.UpgradeTalent(int)` has a checked 1.9.1 hook at RVA `0x28FD814`. The original async method reaches a missing `CurrentScenesComponent.Session` and throws before returning a result. The hook queries `TalentCfg.GetCfg`, checks `ItemManager.GetOwnItemNum(101)` and the live/saved talent level, debits gold through the original `PlayerDataManager` setters, sends `G2C_TalentUpgrade` through the original `TalentManager.OnTalentDataResp`, and persists only `Coin` and `TalentLevel` in their existing seed files. `talent_purchase_pending.json` records the before/after values to reconcile an interruption between those two writes; crash recovery has not been fault tested.

On the Xiaomi, v56 bought nodes 1001 and 1002 for 500 and 1000 gold. Balance fell 3502→3002→2002, the purchased node no longer offered a buy button, equipment stats changed from ATK 10/HP 90 to 15/110, and the final balance/stats survived a process relaunch. The Shop still prices its chests in gems; zero-gem bronze purchase showed an insufficient-gems message. See `FUNCTIONAL_AUDIT.md` for captures, APK hash, and remaining limitations.

## Combat gem reward (v57)

`kill_enemy_hook` preserves the existing per-event gold payout (common 5, elite 25, boss 50) and adds integer division `gold / 2` to the run's gem accumulator. Division happens independently for each kill: 5→2, 25→12, 50→25; fractional remainders are discarded, not carried. The live `G2C_KillEnemy` remains the source for combat gold HUD. Persistent gold/gems are granted only when the original region-counter inference yields `passed=1` at EndGame, under the same condition as gold. Loss and exit before EndGame leave both balances unchanged.

The separate gem counters (`run_diamond_payout`, `run_diamond_credited`) let settlement apply only uncredited deltas if kill callbacks arrive after a successful EndGame. The EndGame response carries both `Gold` and `Diamond`; its GC-handle-retained original object is updated with IL2CPP setters `set_Gold` and `set_Diamond` if a kill arrives late. The v57 two-win device test confirmed the normal order and result UI (500 gold/211 gems), plus process-relaunch persistence. Direct runtime observation of the late callback/UI branch remains pending; the v53 earlier run observed only a late gold delta.

For gem ID 102, `grant_local_currency_reward` reads with original `ItemManager.GetOwnItemNum`, converts through `ObscuredInt.op_Implicit`, calls `PlayerDataManager.set_PlayerDiamond`, and refreshes with `SyncValueToShow`, on Unity thread. It verifies IDs 101/102/104 by readback, then changes only `Coin`, `Diamond`, and `Exp` in the existing `seed_player.json`; existing fields are compared to live balances before the write. It does not replace the profile or write managed-object offsets. The Xiaomi profile went from 2345 gold/0 gems to 3345/422 after two wins and remained so after restarting. Build details/screenshots/logs: `FUNCTIONAL_AUDIT.md`, v57 section.

### Loja de baús: fluxo 1.9.1 e estado atual

Baseline de análise anterior à v63 (atualizado pelo fluxo implementado abaixo): na configuração 1.9.1, `TreasureBoxCfg` oferece o baú 101 por 60 gemas e o 115 por 250. As recompensas passam pela seleção `TBContentCfg`. O fluxo preservado é `ShopManager.BuyTreasureChest` → resposta local → `ShopManager.OnBuyTreasureChestSucceed(G2C_BuyChestMsg)` → `AnalysisChest`/`DoOpenChest` e callbacks originais de inventário/equipamento. `OpenBoxResp` retorna o modelo antigo `S2C_Buy_Treasure_Chest_Succeed_Data`; não tratá-lo sozinho como concessão. A v63 passou a validar preço e saldo live/save, debitar por setters originais, validar readback, persistir itens/equipamentos nos seeds existentes e atualizar managers na thread Unity. Guard concorrente instalado; a v66 adiciona janela contra double-tap de 650 ms, ainda sem teste de compra dupla; journal contra encerramento abrupto segue pendente.

Na build v57 instalada, a reprodução ADB de 14/10/2026 mostrou 422 gemas e 1845 ouro. Os botões bronze/diamante não alteraram tela ou saldo; nenhum hook local de compra estava instalado e nenhum item foi concedido. **Registro histórico supersedido pelas compras v63 e bloqueios v65/v66 abaixo.** Os cartões pagos são ocultos e `PurchaseManager.Purchase` bloqueia pagamentos externos. A oferta de 4000 ouro é de anúncio; o callback genérico de anúncio offline não deve ser confundido com uma compra local segura. Evidências históricas e atuais estão em `FUNCTIONAL_AUDIT.md`.

## Compra local de baús e política da Loja v63/v66

Em `offline_bridge.cpp`, `ShopManager.BuyTreasureChest` só aceita os baús 1.9.1 ID 101 (60 gemas) e ID 115 (250 gemas). Os pools são escolhidos pelos `TBContentCfg.base_weight`: 1001–1036 (`box_id=1011`) e 1175–1240 (`box_id=1021`); a quantidade de sorteios vem de `TreasureBoxCfg.item_nums`. Saldo live e `seed_player.json` precisam concordar. O bridge gera a resposta `G2C_BuyChestMsg`, debita com setters originais, inclui os itens/equipamentos nos seeds existentes e chama `ShopManager.OnBuyTreasureChestSucceed`; então recarrega `ItemManager.OnItemInit` e `EquipManager.OnEquipDataInitResp` na thread Unity. Preserva a apresentação/abertura original. Não usar `OpenBoxResp`, que depende de caches ausentes no bootstrap offline. Erros de write/callback tentam rollback; reentrada simultânea é recusada. A v66 também ignora o mesmo baú por 650 ms para reduzir cobrança por double-tap. Esse debounce foi instalado, mas não exercitado por compra dupla; não há journal de crash entre os três arquivos.

v65 bloqueia `UITreasureBoxPage.OnClickAdCoin`, `OnClickBuyLotCoin` e `OnClickBuyHugeCoin` com prólogos conferidos do APK 1.9.1; a primeira rota não abre anúncio nem dá prêmio, as duas últimas não chamam compra online nem alteram moeda. `PurchaseManager.Purchase` continua bloqueado. As três ofertas de OURO continuam visíveis porque o caminho visual do prefab não foi resolvido; os handlers foram testados por ADB e registraram bloqueio sem alterar o saldo. Saldo insuficiente do baú diamante foi tentado com 112/250 e não debitou, porém a UI ficou vazia e a mensagem não foi observada, portanto não classificar o aviso como validado.

No aparelho, v63 comprou ID 101 com 422→362 gemas e exibiu Calças de Couro (`ContentIds=[1016]`); após reiniciar, a compra v63 de ID 115 debitou 362→112 e exibiu Colar de Cura Santa (`ContentIds=[1182]`). Seeds de jogador/itens/equipamento foram carregados pelos callbacks originais após reinícios, e a tela de equipamento mostrou ambos os itens. v66 é o APK final de laboratório instalado: `build/kinja-run-offline-lab-v66-local-gem-chests-debounce.apk`, SHA-256 `5038CB596BD3043E208F17DDB0EA7A86CF0923F988E6AC6BF98628C77E7B0902`. `adb install -r` preservou o package e `firstInstallTime`; capturas, logs e passos exatos estão no audit funcional.

## Atualização v67/v68: recompensas e ações sem servidor

A oferta de 4000 ouro na Loja é um resgate gratuito diário local. `UITreasureBoxPage.OnClickAdCoin` não inicia anúncio; valida a data/estado, credita 4000 uma única vez, persiste no jogador com journal e chama a atualização original do contador. Testado no Xiaomi em v67: 1845→5845 ouro, 52 gemas sem mudança, contador zerado, toques repetidos bloqueados e saldo mantido após reinício. Troca manual do relógio não foi testada.

Reviver usa a callback local do rewarded ad e `FightHelper.Resurgence` entrega `G2C_Resurgence` à `GameSceneManager.OnResurgenceSucceed`. Teste v67 acionou duas revives gratuitas numa run: sem anúncio e sem gastar gemas; o fim da run foi derrota na fase 6.

`EquipHelper.WearEquip` foi substituído por escrita segura via JSON do seed de equipamento, seguida por `EquipManager.OnEquipDataInitResp` original e tarefa ET concluída. A confirmação visual do item vestido pós-restart faltava na rodada v67; a captura v69 abaixo mostra o colar equipado após novo reinício. O NRE em `UIHeroPage.HeroProperty` continua e está pendente de rastreamento.

Os drops usam `Monster.GetCfg.reward_id` e `RewardManager.RandomRewardPack` na thread Unity; filas são únicas por instância e só chegam ao inventário se `EndGame` confirmar vitória. Na tabela 1.9.1, reward 100030 do Capítulo 3 lista livros 20001–20006 com peso 48 cada (peso total 9000, cerca de 0,53% por livro); chefe 100031 usa peso 100 por entrada. A derrota observada na v67 não concedeu itens e não provou uma rolagem positiva. Limitação histórica resolvida na v69 abaixo: após dez rolagens vazias elegíveis, um livro é protegido contra azar; vitória, bolsa e persistência foram validadas.

A compra local de baús descrita em “Compra local de baús e política da Loja v63/v66” permanece válida; tabela e IDs: 101/60 gemas/1001–1036 (`box_id=1011`) e 115/250 gemas/1175–1240 (`box_id=1021`). A compra concede o conteúdo pela callback original e persiste seeds. Pagamento real, compra do ouro por anúncio e trocas ouro/gemas seguem bloqueados. Com 52 gemas frente a 60, os toques ADB v68 não chegaram comprovadamente a `BuyTreasureChest`, então alerta e recusa visual continuam sem validação.



### Regra de drop offline de livros — v68/v69

A tabela 1.9.1 torna livros muito raros em mobs comuns. Reward 100020: peso 8700 para `ItemId=0`, 45 para cada livro 20001–20006, 30 para item 300101; peso total 9000. Reward 100030: peso 8682 sem item e 48 por livro, total 9000. Isso equivale a cerca de 0,5% por livro em cada mob; o callback v68 confirmou várias rolagens `item=0 count=1`. Boss reward 100021/100031 contém somente livros de IDs 20001–20006, peso 100 cada.

v69 preserva essas rolagens e IDs. Para progressão offline, a cada dez resultados vazios de reward IDs comuns de capítulo, enfileira um livro 20001–20006, count 1. O contador reinicia no começo de cada run e quando um livro natural sai. A entrega ainda depende de vitória: `pending_run_items` passa ao resultado e `apply_local_reward_items` usa a API original e grava `seed_items.json`; derrotas e abandono descartam a fila. Evita offset de objeto e callbacks fora da Unity thread.

Validação runtime: run Capítulo 2/Região 3 terminou em vitória. O resultado mostrou 500 ouro, 211 gemas e itens; logs confirmaram livros 20001–20005 persistidos. Após reinício, saldo 6345/263/350, bolsa mostrou `Livro de Forja de C` x4 e o colar continuou equipado. Ver `build/kinja-v69-result.png`, `build/kinja-v69-bag.png` e `build/kinja-v69-test-logcat.txt`; APK/hash na seção v69 do audit.

## Triagem de melhorias, Correio e menus — v69 (14/10/2026)

ADB confirmou a v69 pelo APK extraído e seu SHA-256. No save preservado: 1845 ouro, 13 gemas, 350 EXP, Lv.2/Capítulo 3, 2 cristais 40001; Anel do Oráculo Lv.1/20 com livro A 1/1. O toque no botão Melhorar disponível falha no `ET.EquipHelper.<UpgradeEquip>d__2.MoveNext`; o toque autorizado em Reforço de um clique falha separadamente em `OneKeyUpgradeEquip>d__3.MoveNext`, sem consumo. O primeiro caminho custa 100 ouro + um livro no nível 1, de acordo com a UI e `EquipUpgradeCfg`. A exceção de sessão/backend é a causa imediata observada, mas os helpers ainda precisam de substituição local transacional antes da build seguinte.

O Correio inicializa requisições remotas de configuração/lista; o logcat salvo mostra quatro tentativas e `SocketError=11001`. Não há seed local de mensagens no diretório de seeds. A configuração 1.9.1 exige nível/capítulo para Atividade e HangUp; não incluir recursos gated sem atender as condições. Ver capturas, linhas filtradas e limitações em `FUNCTIONAL_AUDIT.md`.

## Build v72 — melhoria local e diagnóstico de tela (14/10/2026)

O APK original exato usado é `kinja-run-1-9-1.apk`, SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`. Para reproduzir a build final de laboratório desta etapa:

```powershell
.\offline\native\build-native.ps1
.\offline\build.ps1 -BuildId v72-local-equipment-transaction -PackageName com.habby.kinjarun.offline.lab6 -NativeLibrary .\offline\build\arm64-v8a\libkinjaoffline.so -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v72-local-equipment-transaction.apk
```

A v72 foi instalada no Xiaomi sem limpar dados: package `com.habby.kinjarun.offline.lab6`, versão original 1.9.1/versionCode 51, `firstInstallTime` permaneceu `2026-10-13 19:43:33`. APK SHA-256 `CC722CC4D664EF93245D778960E4CF0FECA1F935A1F48B42C41FBAC114E80E42`. Não substituir os APKs de referência original/v57/v69.

### Equipamento

Os handlers 1.9.1 chamam `EquipManager.UpgradeEquip(uid, oneKey)`, que usa helpers online que lançavam NRE sem sessão. Hooks separados só instalam depois de validar os 16 bytes de prólogo do APK alvo. A rota local consulta APIs originais de `EquipManager`, `EquipUpgradeCfg`, `ItemManager` e `GetEquipCfgByIdAndLvAndQuality`; não altera campos gerenciados por offset. A transação existente prepara mudanças, guarda os três seeds (`seed_equipment`, `seed_items`, `seed_player`) com marcador, grava os saves, debita por setters originais, entrega callbacks `EquipManager.OnEquipDataUpdateResp` e `ItemManager.OnItemInit`, compara readback e remove o marcador no commit. Reentrada é recusada; bootstrap restaura os snapshots se o marcador ficou após interrupção.

Resultado observado da melhoria unitária: UID4 Anel do Oráculo Lv.1/ATQ15 → Lv.2/ATQ19, Livro A 1→0, ouro 2565→2465; reinício manteve Lv.2/ATQ19 e 2465 ouro/13 gemas. Capturas/logs: `build/kinja-v71-after-upgrade.png`, `build/kinja-v71-after-upgrade-logcat.txt`, `build/kinja-v72-ring-after-restart.png`. O saldo anterior à build era 1845/13/350; HangUp local acumulou 720 ouro e 720 EXP durante a passagem de tempo, resultando em 2565/13/1070 antes de gastar os 100 ouro. Nenhum cristal azul foi alterado.

OneKey recebe UID selecionado e não opera sobre todos os itens. A validação positiva ficou pendente por falta de livro após a melhoria unitária; não gaste mais recursos nem declare sucesso. O código atual planeja melhoria sequencial daquele UID e recusa material de IDs mistos sem gasto parcial. A semântica exata de parada ainda precisa de corroboração no helper nativo 1.9.1 e validação natural.

### Correio e capítulo

`UIMainPanel.DoOpen` oculta `btnMail` e notificações de mail usando Transform/GetChild, Object.get_name e GameObject.SetActive, todos pelo runtime IL2CPP Unity thread. A captura v71 `build/kinja-v71-hamburger-menu.png` mostra Configurações sem Correio; log v72 confirma o nó ocultado. O cliente ainda tenta config/lista em background: DNS `SocketError=11001`, quatro tentativas no log completo `build/kinja-mail-startup-logcat-full.txt`. Não há seed de mensagens/recompensas no bridge.

Status histórico da v72: encontra a rota exata `Content/Page/Main/Bottom/LevelHandle/Unclear/btnUnclearFight/txtRegionNameInBtn` com `Transform.Find`, obtém UILabel por GetComponent e chama pivot/alinhamento centrais; a captura mostrava texto alto. A v73 adiciona ancoragem ao retângulo do botão e confirma centralização vertical e horizontal visualmente; consulte “Continuação v73” ao final. A tentativa recursiva v70 causou SIGSEGV; não reintroduzir.

### Limitações e evidência cruzada

O fluxo de cristal azul 40001 tem custos/talento em tabelas; o save tinha 2 cristais. Só está implementada a recompensa local do marco inicial do Capítulo 1; recompensas de capítulo seguintes e atividades rankeadas não foram ligadas. A Atividade 104 requer nível8/Capítulo7; HangUp 116 exige passar Capítulo2; ID125 exige nível5. Não liberar menus bloqueados por esses requisitos nem substituir adição de moeda por configuração visual.

Os arquivos de captura e logs acima separam análise estática, chamada do bridge, resultado visual e persistência. Consulte `FUNCTIONAL_AUDIT.md`, “Resultado v72”, antes de afirmar validação de OneKey, crash recovery, cristais azuis ou vertical de capítulo.

Registro de estado final: após a melhoria validada do Anel, um toque ADB de navegação acionou involuntariamente `UpgradeEquip` para UID2, consumindo 100 ouro e 1 Livro C. Log `build/kinja-v72-final-action-logcat.txt`; saldos após reinício `build/kinja-v72-final-after-extra-transaction-restart.png`: 2365 ouro/13 gemas/1070 EXP. Não ajustar manualmente os dados para reverter essa transação.

## Continuação v73 — layout e OneKey pendente (14/10/2026)

BuildId: `v73-chapter-label-anchor`. Comandos usados:

```powershell
./offline/native/build-native.ps1
./offline/build.ps1 -BuildId v73-chapter-label-anchor -PackageName com.habby.kinjarun.offline.lab6 -NativeLibrary ./offline/build/arm64-v8a/libkinjaoffline.so -PackageApk
./tools/android/toolchain/platform-tools/adb.exe install -r ./build/kinja-run-offline-lab-v73-chapter-label-anchor.apk
```

APK SHA-256: `7F36012506293B13C964EC9474FC001CE6AFF5FD5D302DC5963F9C1AD54127E5`. O package manteve `versionCode=51`, `versionName=1.9.1`, `firstInstallTime=2026-10-13 19:43:33` e foi atualizado sem limpar dados. Build e certificado foram conferidos. O alvo 1.9.1 ainda corresponde ao SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`.

### Alinhamento do texto de capítulo

Static/hierarquia: `txtRegionNameInBtn` é filho de `btnUnclearFight`. Bridge: encontra o UILabel por caminho exato, resolve seu pai e ancora ao retângulo do botão; usa pivot/alinhamento centrais por APIs Unity/IL2CPP na thread Unity, sem offsets gerenciados. `build/kinja-v73-home.png` e `build/kinja-v73-after-restart.png` confirmam centralização vertical/horizontal antes e depois do reinício. `build/kinja-v73-after-restart-logcat.txt` registra sucesso de `UIRect.SetAnchor`, `UIWidget.set_pivot` e `UILabel.set_alignment` (`anchor=1 pivot=1 alignment=1`).

### Reforço de um clique

O handler do APK 1.9.1 encaminha o UID escolhido para `EquipManager.UpgradeEquip(uid, true)` e daí para `ET.EquipHelper.OneKeyUpgradeEquip(int)`. Assim, é um item selecionado; o número de níveis e a condição de parada não estão confirmados. O state machine tem `MoveNext` em `0xF3934C`, mas a análise REA falhou porque o provedor Ghidra não está configurado neste Windows. A implementação local que acumula níveis sequenciais até o limite ou insuficiência continua sendo hipótese. Não tocar até confirmar a semântica 1.9.1 e possuir livros naturalmente suficientes.

Estado antes da atualização: `build/kinja-current-before-v73.png` registra 2365 ouro, 13 gemas e 1070/2000 EXP; `build/kinja-equip-detail-before-v73.png` registra Anel do Oráculo Lv.2 e Livro A `0/1`. Após relançar a v73, `build/kinja-v73-after-restart.png` confirma 2365 ouro, 13 gemas e 1070/2000 EXP. Não houve toque de OneKey nem consumo nesta sessão. Consulte `FUNCTIONAL_AUDIT.md`, continuação v73, para separar evidência estática, bridge, visual e gameplay/save.

## v74/v75 — perfil QA e talentos azuis

O save atual é perfil de QA por decisão do usuário. `build-native.ps1 -QaMaxProfile` prepara, uma única vez, moedas, EXP, nível (100) e todos os 56 IDs de item de bolsa configurados no `ItemCfg` a pelo menos 1.000.000. O marcador `qa_max_profile_applied.json` impede que gastos sejam repostos após reiniciar. Builds normais não usam essa opção. Depois da lapidação, a conta real deverá partir de perfil separado e limpo.

O handler azul usa `TalentCfg.type=1`, item 40001, custo/nível mínimo da tabela 1.9.1, `TalentManager.CurExtraUpgradeNum` e o campo `ExtraTalentLevel` de `G2C_TalentUpgrade`. Confere a sequência parent e o saldo live/save; debita o seed de itens e chama `ItemManager.OnItemInit` e `TalentManager.OnTalentDataResp` na Unity thread. A transação usa journal compartilhado com as compras por ouro e mantém leitura compatível de journals legados.

APK v75 instalado: `build/kinja-run-offline-lab-v75-qa-profile-once-blue-talent-persist.apk`, SHA-256 `2171B6026F31DD6E391ABD9031B6E0376D57D2BD00A2B24BFE64B3CF9DD5802D`, package lab6, instalado por `adb install -r` preservando firstInstallTime. IDs 2001 (custo 1) e 2002 (custo 3) foram comprados; os logs confirmam mudança `ExtraTalentLevel 0→1→2`, callbacks e commits. O ramo do Escolhido Divino mudou visualmente para estado adquirido e sobreviveu ao restart. v74 tinha defeito de reabastecer no restart, corrigido na v75 com marcador one-shot. Efeito em combate, falta de saldo, double-tap e recuperação por interrupção continuam pendentes; consulte `FUNCTIONAL_AUDIT.md` para evidência por categoria.

### Retificação QA v75 — interações e leitura de saldo

Navegação ADB por coordenadas acionou involuntariamente talentos comuns 1006–1012 (31.500 ouro), cinco compras do baú local 115 (250 gemas cada; itens 1175, 1200, 1191, 1205 e 1178) e OneKey do UID3 (nível 2→10, 2.500 ouro, 17 livros 20002). Os logs confirmam callbacks e commits locais, mas não validam o algoritmo OneKey de 1.9.1 porque a ação não foi planejada. Não houve reversão nem edição manual.

O readback/persistência do bridge às 17:30:27 indicou 966.000 ouro, 998.750 gemas e 1.000.000 EXP. Após force-stop/relaunch, sem novos toques, `build/kinja-v75-unexpected-after-restart.png` exibiu cerca de 235 mil ouro e 998 mil gemas. O log `build/kinja-v75-unexpected-after-restart-logcat.txt` só mostra bootstrap e marker QA já ativo. Sem acesso de leitura aos arquivos privados via `run-as`, o saldo visual/persistido de ouro fica inconclusivo. Parar testes que gastem recursos até investigar essa divergência; manter as capturas e logs como evidência, sem afirmar persistência visual.

## v77 — troca de gemas por ouro

Nos recursos 1.9.1, `VariableCfg` contém `diamond_buy_lot_coin=[20000 ouro,100 gemas]` e `diamond_buy_huge_coin=[118888 ouro,500 gemas]`. Os handlers `OnClickBuyLotCoin` (`0x13E145C`) e `OnClickBuyHugeCoin` (`0x13E1730`) estavam sendo bloqueados com retorno sem efeito. A v77 instala hooks com prólogos conferidos, consulta `Variable.GetCfg` e `System.Array.GetValue(Int32)` pelas APIs gerenciadas originais, valida os valores 1.9.1 e realiza transação local: valida live/save, saldo e overflow; seta moedas pelo PlayerDataManager, sincroniza/relê pelo ItemManager e grava os dois campos no save em substituição atômica. Guard comum evita reentrada. A seção OURO fica visível; as ofertas monetárias seguem ocultas.

A v76 foi tentativa intermediária: o aparelho não exportou os símbolos C opcionais para arrays; o teste abortou sem consumo. A v77 corrigiu pela API gerenciada e foi instalada no package lab6 com `adb install -r`; firstInstallTime preservado. BuildId `v77-local-gem-gold-exchange-array-api`, APK SHA-256 `C8008999C37F6539716FC501BB6301F526203746A6F754378A47E0453B9B8614`, biblioteca SHA-256 `7DD4A152CB58A7BF8F5FF2633434703CAD93F00E5718391F7A56E4D60DD7EB01`.

Teste em dispositivo: 198.200 ouro/996.940 gemas passaram a 218.200/996.840 após um clique de 100 gemas; EXP permaneceu 1.000.000. Cards e saldos arredondados foram observados antes/depois e o saldo permaneceu após reinício do processo. Capturas e logs exatos constam na seção v77 do `FUNCTIONAL_AUDIT.md`. O pacote de 500 gemas foi validado em complemento abaixo. Saldo insuficiente e repetição concorrente/double-tap continuam sem teste.

Complemento de validação v77: o pacote grande também foi comprado uma vez: ouro 218.200→337.088, gemas 996.840→996.340. Após reinício, a Loja mostrou 337K/996K. Capturas/logs `build/kinja-v77-shop-huge-purchased.png`, `build/kinja-v77-shop-huge-purchase-logcat.txt`, `build/kinja-v77-after-huge-restart-shop.png`, `build/kinja-v77-after-huge-restart-logcat.txt`. As duas ofertas têm resultado e persistência após reinício; saldo insuficiente e double tap/reentrada continuam não exercitados.


## Fusão individual de equipamento — v79

No APK 1.9.1, `UICompoundEquipTips.OnClickCompound` chama `EquipManager.CompoundEquip` e `ET.EquipHelper.CompoundEquip(int,int,int)`. O helper online falha com `NullReferenceException` porque `ET.Game.Scene` é nula antes de `ET.Session.Call`; o antigo dispatcher C2S não é essa rota. A resposta 1.9.1 é `ET.G2C_CompoundEquip` com `EquipUid` e dois `ET.EquipInfo` em `CostEquips`.

v79 adiciona o hook individual validado pelos bytes de prólogo do APK original (SHA-256 `7953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E`; VA `0x2c2d3f4`). Consulta UIDs e atributos no save e `EquipManager`, verifica ID/qualidade iguais, não equipados e a qualidade seguinte pela `EquipQualityCfg`. Persiste a raiz com qualidade incrementada e remove os dois materiais selecionados, sem moeda; então usa `EquipManager.OnGetCompoundEquip` e atualiza o inventário com `OnEquipDataInitResp`. Guard de reentrada, validação de UID e journal com snapshot protegem contra repetição/falha. Callbacks e singletons executam na Unity thread. OneKey permanece independente e não foi alterado.

### Build reproduzível e evidência

```powershell
.\offline\build.ps1 -BuildId v79-local-equipment-fusion-rollback-refresh -PackageName com.habby.kinjarun.offline.lab6 -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v79-local-equipment-fusion-rollback-refresh.apk
```

- APK v79 SHA-256: `5AD160E8F0590B7E5E9766B1BCAB4D3D7E213AE26114B74E27261974F9668E02`.
- Biblioteca ARM64 SHA-256: `6B48154C57CB0E865039322CBCAB4D3D7E213AE26114B74E27261974F9668E02`.
- Package: `com.habby.kinjarun.offline.lab6`, `versionCode=51`, `versionName=1.9.1`; assinado pelo keystore de laboratório. `adb install -r` manteve `firstInstallTime=2026-10-13 19:43:33`.
- `build/kinja-v79-startup-filtered-logcat.txt`: hook instalado após relançar; sem `FATAL EXCEPTION` nessa captura filtrada.
- `build/kinja-v78-before-install.png` e `build/kinja-v79-after-restart.png`: save QA visualmente em ~992K gemas, ~925K ouro, 1.000.000 EXP antes/depois.
- `build/kinja-v78-root-select6.png`: UI de fusão atual informa “Nenhum outro equipamento disponível”; nenhuma confirmação foi enviada e nenhuma peça/recurso foi gasto.

Validação estática, hook carregado e instalação estão confirmados. Resultado visual de fusão e gameplay/save positivo seguem pendentes até que a UI mostre materiais compatíveis naturalmente disponíveis. Os arquivos privados não puderam ser lidos por `run-as`; saldos exatos além dos valores visuais arredondados permanecem desconhecidos. Consulte `FUNCTIONAL_AUDIT.md` antes de tratar esse caminho como validado.

### v80 — validação final da seleção

v80 acrescenta `EquipManager.EquipCanCompound(rootUid)` e `EquipRelateCompound(costUid)` para os dois materiais antes de gravar qualquer alteração. APK SHA-256 `82B74E8048B53B3D959D264EFE2D33F7B2075FF37C9D2C867C2714BFD641810B`, `.so` SHA-256 `FF5130D24B3B812C059686F8353379161CEC95B4214EE330A6D76F4F677A011F`; instalado por `adb install -r`, mesmo package e certificado, firstInstallTime preservado em `2026-10-13 19:43:33`. Log de inicialização `build/kinja-v80-startup-filtered-logcat.txt` mostra o hook instalado. Capturas antes/depois `build/kinja-v80-before-install.png`, `build/kinja-v80-after-restart.png` mostram cerca de 632K ouro, 992K gemas e 1.000.000 EXP.

O log `build/kinja-v80-runtime-logcat.txt` documenta sete operações OneKey registradas em paralelo enquanto o jogo era usado: cinco Nv.1→20 por 57.600 ouro/85 livros cada e duas Nv.1→10 por 2.600 ouro/18 livros cada. Os readbacks fecharam em 632.228 ouro, 992.190 gemas e 1.000.000 EXP. Elas não vieram do bridge de fusão nem da instalação; deixamos o save como estava. A UI não apresentou materiais elegíveis para fusão (`build/kinja-v78-root-select6.png`), então o botão de confirmar não foi acionado e ainda não há validação de resultado/persistência da fusão.

Reprodução de build/instalação v80 (use um BuildId novo em nova execução neste checkout; o diretório v80 existente é preservado):

```powershell
.\offline\build.ps1 -BuildId v80-local-equipment-fusion-original-eligibility -PackageName com.habby.kinjarun.offline.lab6 -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v80-local-equipment-fusion-original-eligibility.apk
```

### v83/v84 — duplicatas locais somente para QA

O define `KINJA_OFFLINE_QA_FUSION_REPEATS` (ativado por `-QaFusionRepeats`) adiciona uma única leva de cópias de uma peça já presente no save, somente para builds de laboratório. O código roda depois que `EquipManager` carregou o modelo, valida a próxima qualidade via `EquipQualityCfg`, clona o JSON preservando campos, calcula UIDs únicos, define `IsWear=false`, grava `seed_equipment.json` atomicamente e chama `EquipManager.OnEquipDataInitResp` na Unity thread. Markers `qa_fusion_repeats.json`/`qa_fusion_repeats_v2.json` impedem repor itens em reinício/continuação. Esse define não pertence a releases. A lógica de fusão sempre mantém as APIs originais `EquipCanCompound`/`EquipRelateCompound` como requisitos.

```powershell
.\offline\native\build-native.ps1 -QaMaxProfile -QaFusionRepeats
.\offline\build.ps1 -BuildId v84-qa-fusion-proven-quality-pool -PackageName com.habby.kinjarun.offline.lab6 -NativeLibrary .\offline\build\arm64-v8a\libkinjaoffline.so -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v84-qa-fusion-proven-quality-pool.apk
```

v83: sete peças totais de `EquipId=6001`, qualidade 1. v84: sete totais de `EquipId=3002`, qualidade 2. Ambas as levas aparecem após force-stop/relaunch; o log v84 diz que os markers já estavam aplicados e não refez o seed. As APIs de elegibilidade continuaram false e a UI exibiu cadeados; nenhuma fusão pós-seed foi confirmada. Hash v84 APK `C585F0AC89884A3ABBF06FD788EF630CFF3392DFF26174D3C2D6A9BB18181B70`, `.so` `C9D7C7440B54C29E887093D59143E42205A6C3F39CD4537050FD8CF5B7B87C58`. Package lab6 e keystore de laboratório; `adb install -r` preservou `firstInstallTime=2026-10-13 19:43:33`. Capturas `build/kinja-v84-equipment-bag.png` e `build/kinja-v84-after-restart-equipment-bag.png`; logs detalhados no `FUNCTIONAL_AUDIT.md`.

Gameplay confirmado anteriormente: a fusão altera `EquipQuality` e o teto, não `EquipLevel` corrente; logs registraram q2→q3 mantendo os níveis 20 e 1. Para esclarecer a inelegibilidade dos lotes QA, ainda é necessário descompilar os corpos ARM64 originais `EquipManager.EquipCanCompound`/`EquipRelateCompound` do APK exato 1.9.1 antes de mudar qualquer regra.

### Baú de diamante 10x — status v87 (14/10/2026)

A tabela 1.9.1 contém o ID108 (10 sorteios, 2200 gemas, pool `box_id=1021`, configs 1175–1240). O hook local `ShopManager.BuyTreasureChest` agora aceita ID108; preço e `item_nums` são lidos do `TreasureBoxCfg`, dez recompensas são sorteadas segundo pesos da `TBContentCfg`, e o bridge debita via setter original, grava seeds de item/equipamento e usa `OnBuyTreasureChestSucceed`/callbacks de inventário na Unity thread.

Limitação importante: a captura v87 mostra apenas o cartão unitário ID115; compra ID108, quantidade de dez resultados, saldo de 2200 gemas e persistência pós-reinício ainda não foram exercitados no aparelho. O hook experimental v86 em `TreasureBoxItem.UpdateItem` fez o Unity cair (SIGBUS); foi retirado. Não acionar o botão unitário como se fosse 10x. Veja `FUNCTIONAL_AUDIT.md` para reprodução, saldos, logs, capturas, hashes e próximo passo de resolver a hierarquia visual antes de autorizar uma compra de 2200 gemas.

### Baú de diamante x10 — cartão exposto na v94

No prefab exato 1.9.1, o GameObject `UITreasureBoxPage/obj/ContentPanel/Top/ChestBox/TenDiamondBox` existe inativo. O hook validado `UITreasureBoxPage.UpdateTreasurePage` ativa o cartão original; o hook `UITreasureBoxPage.UpdatePage` aplica o espaçamento depois do layout do pai usando `Transform.get_localPosition`/`set_localPosition` e `UIScrollView.UpdateScrollbars(true)`. A v94 confirma o prólogo de `UpdatePage` em little-endian e o log `build/kinja-v94-shop.log` confirma hook, ativação, deslocamento de 420 Unity units e atualização de bounds. Use APIs Unity/IL2CPP gerenciadas; não escreva offsets internos de objetos.

A captura `build/kinja-v94-shop.png` mostra o cartão “Baú de Diamante x10” sem cobrir a seção Ouro. Build instalada: `v94-local-ten-draw-after-page-update`, APK SHA-256 `6E5A93CAF051AFF33F468D39D2F8BBDD9FDD8820B0C7344AF2B0ADEA7936F3C1`, `.so` SHA-256 `00677F02CECB81CC46FE7EA9654B0B50E9735D5EFE4E6ED4E496688661BB79FC`. Build e atualização reproduzíveis:

```powershell
.\offline\native\build-native.ps1 -QaMaxProfile -QaFusionRepeats
.\offline\build.ps1 -BuildId v94-local-ten-draw-after-page-update -PackageName com.habby.kinjarun.offline.lab6 -NativeLibrary .\offline\build\arm64-v8a\libkinjaoffline.so -PackageApk
.\tools\android\toolchain\platform-tools\adb.exe install -r .\build\kinja-run-offline-lab-v94-local-ten-draw-after-page-update.apk
```

Não houve compra ID108; a transação de dez itens, custo de 2200 gemas e persistência ainda exigem validação de gameplay autorizada. Consulta ADB depois da instalação: versão 1.9.1/code 51, `firstInstallTime=2026-10-13 19:43:33`, `lastUpdateTime=2026-10-14 19:15:00`. Saldos visuais arredondados: ~990K gemas/~632K ouro; último readback exato conhecido anteriormente: 990690 gemas, 632228 ouro, 1000000 EXP. Consulte também `FUNCTIONAL_AUDIT.md` para o histórico v88–v94 e evidências separadas.

### Auditoria UX da Loja — 14/10/2026 (v94, investigação sem novo APK)

Capturas `build/kinja-shop-ux-current.png` e `build/kinja-shop-ux-scroll-bottom.png` mostram que, durante a rolagem, o conteúdo do Shop entra atrás do HUD fixo e o cabeçalho do cartão x10 fica parcialmente encoberto. Revisar o clip/viewport do `UIPanel` e a região do `UIScrollView` e ajustar por APIs NGUI/Unity gerenciadas. Não há build nova nesta auditoria.

A interface do botão ID108 mostra o item `4001` e `1000000/10`; a tabela 1.9.1 associa ao ID108 também `cost_diamond=2200`, `coin_id=4001`, `coin_num=10`, `cost_item=[10]`. `ItemCfg` identifica 4001 como Soul Stone. O log capturou uma compra local ID108 com débito de 2200 gemas (990440→988240), mas não confirmou decremento de 4001 nem exibiu recibo. A origem do evento é indeterminada: o script não tocou no botão de compra e só fez navegação/swipes; pode ter havido entrada concorrente ou ativação não intencional. Não tocar novamente no botão. Desmontar `TreasureBoxItem.SetButtonInfo`, `OnOpenByItem` e `OnClickBuy` do binário 1.9.1 antes de corrigir a apresentação ou o fluxo de pagamento. Validação de recibo e persistência depois do evento ainda não realizada. Consulte o complemento no `FUNCTIONAL_AUDIT.md`.

## Loja — complemento v97 (14/10/2026)

O ID108 da configuração 1.9.1 usa dez sorteios, chave `open_item_id=1002` × 10 (`cost_item=[10]`) e alternativa de 2200 gemas. `coin_id=4001` é campo separado e não representa o ícone do botão. Hook checked em `TreasureBoxItem.OnOpenByItem` propaga o estado Item/Currency síncrono até `BuyTreasureChest`; débito por chave/gemas compara ItemManager com save, recompensa segue os callbacks originais e journal permite recuperação de transação interrompida. Estado por chave ainda não foi acionado em gameplay.

A v97 também ajusta `UIPanel.baseClipRegion` via API gerenciada, invalida o clipping e recalcula scrollbars. Log confirma top 624→464 com bottom -660 preservado, mas a captura `build/kinja-v97-now.png` ainda mostra o título do x10 sob o HUD em scroll baixo. A correção visual falhou; mapear o painel, as âncoras e `finalClipRegion` antes de alterar novamente.

BuildId `v97-shop-clip-invalidate-readback`; APK SHA-256 `1EFF10D7099EC975DD3EA6C2520C0FACE393EB7F7DEA463B0F9B341FED811358`, biblioteca `CD5DFCEA8377643DB96E042012069BFCA95D0525D75E5012D77901209B7A2AE9`. Package lab6 atualizado por `adb install -r`, firstInstallTime preservado. Estado exato pré-instalação: 632228 ouro, 986040 gemas, 1000000 EXP; logs em `build/kinja-v95-preinstall-device-logcat.txt`. Aquele readback já refletia duas transações ID108 não planejadas, documentadas na auditoria funcional. Nenhuma compra foi iniciada durante v97; validação de pagamento e persistência permanece pendente.
