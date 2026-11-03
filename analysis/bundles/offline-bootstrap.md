# Original client: offline bootstrap and local protocol findings

This report uses the restored native image `analysis/metadata/libil2cpp.stripes.so`, SHA-256 `a2a4510a6650a6bfc9c23949e466392a098f9a1c992e21194b531c1e45babdbe`, and the completed Il2CppDumper outputs `analysis/il2cpp/dump.cs` and `script.json`. Files ending `.annotated.txt` in this directory are static disassemblies of that restored image. Earlier `.txt` without `.annotated` or `.restored` may come from the preliminary, incompletely restored image; use the annotated files for this report.

No APK was changed or executed by this worker. Findings below establish original code behavior statically and do not establish a playable offline build.

## 1. There is a real local protocol implementation, but its bootstrap is disabled

`ProtocolManager.SendProtocol<T>` at **0x32ef5ec** does the following:

1. Maps the request class to an integer protocol ID from dictionaries at manager offsets `0x168` and `0x160`.
2. Serializes the request with `UnityEngine.JsonUtility.ToJson`.
3. Calls **ProtocolManager.HandleProtocol at 0x2a8fac4**, callsite **0x32ef6e8**.
4. When that returns true, logs the literal `Send（fake）`. Otherwise it calls `ProtocolManager.Send` at **0x2a95b54**.

`HandleProtocol` compares sixteen original `C2S_*` protocol IDs. It contains original local calculations and constructs responses such as `S2C_PlayerHeroData`, `S2C_PlayerItemData`, `S2C_Player_Equip_Data`, `S2C_GainRewards`, then dispatches through `TriggerProtocolEvent`. Exact compared IDs and callsites are recorded in `local_handler_protocols.json`:

- Fight result, resurrection, scene progress reward, turntable;
- Hero upgrade/star/badge/purchase/equip;
- Talent upgrade;
- Equipment upgrade/compound/one-key compound/break/wear;
- Achievement reward.

The fight-result branch **0x2a8ffa4–0x2a8ffd4** immediately returns true. That branch by itself records no result or reward. Therefore a successful fake-send does not mean offline progression is saved.

**The local subsystem does not boot a player in this build.** `ProtocolManager.InitPlayerData` at **0x2a94f94** has an IFix patch check followed by a base implementation that simply returns. It creates no `PlayerAllData`, reads no save and grants no birth data. `ProtocolManager.Send` at **0x2a95b54** is also only a debug-log operation in its base implementation. `ResetLocalProtocolData` at **0x2a89684** calls `CodeStage.AntiCheat.Storage.ObscuredPrefs.DeleteAll`; it does not generate a new player.

The local model `PlayerAllData` at manager offset **0x150** uses arrays `ivk/iv`, `fvk/fv`, `svk/sv`, plus `PlayerDataItems` and `PlayerDataHeros`. The active ET network model **G2C_PlayerData is a different class**. Simply placing a `PlayerAllData` object in that field or changing a server flag will not initialize the active managers.

All these functions contain IFix hooks. This report describes the packaged base code; an external IFix payload could replace their behavior. No external payload was fetched.

## 2. Active login loads server push messages into the original game managers

`PlayerDataManager.Init` at **0x2796920** creates the original gameplay manager singletons, resets login task counters, adds **two tasks** at **0x2797194** and **0x279719c**, opens login loading UI and then calls **AccountInit at 0x27972e0**.

The two core completion paths are:

| Path | Original code and effect |
|---|---|
| Account/session login | `PlayerDataManager.Login(string)` runs async state machine `MoveNext` **0x27ea348**. It awaits `ProtocolManager.Login` **0x2a96c5c** at callsite **0x27eac78**, receives `ET.G2C_Login`, and calls `CompleteLoginTask` **0x279671c** at **0x27eab88** on the successful path. |
| Player values push | `ET.G2C_PlayerDataHandler.Run` **0x26872e4** calls `PlayerDataManager.OnPlayerValueDataResp` **0x2796494**. That method calls `OnGetPlayerAllData` **0x279569c**, `SyncValueToShow` **0x279658c**, sets player readiness and completes a login task. |

`PlayerDataIsReady` is represented by the original in-memory bool key **`player_data_get_ready`**, accessed through `PlayerDataManager.GetBool/SetBool`. `LoginIsComplete` **0x27967dc** separately checks `_curLoginTaskCount >= _loginTaskCount` at object offsets **0x78** and **0x7c**; a target count below 1 also returns true. Making either of those predicates true without loading data bypasses waiting while leaving the game state incomplete.

The primary value response `ET.G2C_PlayerData` contains `PlayerName`, `RegTime`, `LastTime`, `Lang`, `Level`, `Coin`, `Diamond`, `Power`, `Exp`, `IsNewDay`, `CumulativeLoginDays`, **non-null PlayerStatus/PlayerStatusTime lists**, rename/icon/frame values and union values. The original callback copies those values into the original `ObscuredInt` properties, sets registration date/time, rebuilds player statuses, initializes guide state and updates manager/UI indicators. New allocations should use the original constructors and property APIs rather than writing plain integers over `ObscuredInt` structures.

After login the async state machine calls `PurchaseManager.PullShopInfo` and `PlayerDataManager.PullActivityData` **0x2797f1c**. The latter requests piggy bank, promo shop, daily promo shop, growth, battle passes, equipment investment, purchase events and first-charge data. Some requests add more login tasks. A local solo adaptation should avoid starting those optional online requests or provide their original completion callbacks.

## 3. Values alone do not supply the hero, items or playable chapter

Original ET push handlers populate the needed managers independently:

| Original response | Original handler | Manager callback |
|---|---:|---:|
| G2C_PlayerData | 0x26872e4 | `PlayerDataManager.OnPlayerValueDataResp` **0x2796494** |
| G2C_HeroInfo | 0x2772fc0 | `HeroManager.HeroDataInit` **0x296d944** |
| G2C_RegionData | 0x2688d2c | `SceneDataManager.OnGetSceneProgress` **0x277e3bc** |
| G2C_EquipData | 0x27718cc | `EquipManager.OnEquipDataInitResp` **0x2c35544** |
| G2C_ItemInfo | 0x2773414 | `ItemManager.OnItemInit` **0x280a82c** |
| G2C_TalentData | 0x268ab4c | `TalentManager.OnTalentDataResp` **0x28ffcd0** |
| G2C_PlayerVariable | 0x26885a8 | `PlayerGuideManager.OnGetGuideData` **0x279ae9c** |

Their field structures are in `offline_model_fields.txt`. Relevant shapes:

- `G2C_HeroInfo`: equipped hero ID and `List<PlayerHero>`. Each hero has ID, level, star and non-null talent list.
- `G2C_RegionData`: selected/max region and progress, `List<PlayerRegion>`, support gifts list. Each region has ID, pass status, max score, consecutive losses and non-null difficulty-pass list.
- Equipment/items responses hold non-null lists, even when initially empty.
- Talent response contains two integer levels. Guide variable response holds a non-null guide IDs list.

The APK's original `Birth` table supplies **hero 1001, gold 1500, jewels 0, no equipment/items**. Original tutorial content is **region 1, stage 101**, with complete packaged bricks/prefabs as documented in `tutorial_content_seed.json`. Hero level/star, initial power, exact clock units and optional data defaults still require explicit setup and validation; the birth table does not specify all of them.

`Init.OnPlayerDataInit` **0x2805174** adds another task at **0x280534c** and loads original **Main** through `ResourceManager.LoadSceneAsync` **0x289aff8**. Main's `OnPlayerDataInit` **0x29e17f8** reads `SceneDataManager` and `HeroManager` and may call **GameSceneManager.PreEnter at 0x2697e8c** with the original selected region and equipped hero. This route preserves the original gameplay and assets.

## 4. Development servers are still remote; no packaged offline switch was found

The restored `ET.ConstValue..cctor` at **0x2d13ea0** builds five `ServerConnect` objects. `ConfigTool.GetServerConnect` **0x2d0f4fc** selects one using enum `DevelopIn`, `DevelopOut`, `Release`, `DevelopOutRaw`, `PreRelease`.

| Configuration | Login/fallback | TLS | Production |
|---|---|---:|---:|
| DevelopIn | `192.168.50.71:5688` for both | false | false |
| DevelopOut | `test-core.kinjarun.com:443`, `test-secondary-core.kinjarun.com:443` | true | false |
| DevelopOutRaw | Packaged AWS ELB test host, port 443, for both | true | false |
| Release | `core.kinjarun.com:443`, `secondary-core.kinjarun.com:443` | true | true |
| PreRelease | `pre-core.kinjarun.com:443`, `pre-secondary-core.kinjarun.com:443` | true | true |

The `ServerConnect` model also holds resource, mail, business, account, feedback and hotfix addresses. The release account endpoint is `https://account.kinjarun.com`; the resource address is `hu.kinjarun.com`. Switching development or disabling TLS selects a different transport/host; it does not provide local account state.

A bounded scan of **385 bundles** covered all `clientgameobjects*.ht` and every scene bundle referenced by the resource database, with zero read errors. It found original **LittleLevelTest** components in `clientgameobjects2.ht` and `clientgameobjects8.ht`, but none for `LittleLevelTestIniter`, `BulletShooterTest` or `WeaponTest`, and no serialized `Offline`, `Mock`, `TestMode` or `LocalMode` field. `bootstrap_asset_scan.json` records the boundary and all matches.

`LittleLevelTest` has only a constructor. `LittleLevelTestIniter.Awake` **0x13aa154** only gets ResourceManager and fetches `Prefabs/Scene/LittleLevelTest/LittleLevelTestCamera`; it does not create a player or enter a fight. That is a camera/editor helper, not a usable offline bootstrap.

Search of all 143 decoded config tables yielded twelve related string hits in `offline_resource_hits.json`: six `divineTreasure.status="offline"` product entries and six localization/idle-gain strings. None defines an offline gameplay flag or localhost service address.

## 5. Persistence and concrete adaptation points

No `.db`, `.sqlite` or `.sqlite3` file was found by extension in the prior complete APK extraction. SQLite library metadata exists, so this negative filename search does not prove SQLite is unused at runtime.

`PlayerDataManager.SaveLocalInt` **0x2799a24** concatenates original `ProtocolManager.PlayerId` and the supplied key through `HTString.LinkString`, then calls **UnityEngine.PlayerPrefs.SetInt at 0x157653c**. `GetLocalInt` **0x2799c38** builds the same key and calls **PlayerPrefs.GetInt at 0x157669c**. Their disassembly and matching icall strings corroborate local scalar preferences. Neither routine restores the complete account/hero/chapter profile.

The smallest justified implementation direction is an **offline bootstrap/transport adapter inside the original client**:

1. Retain the already identified content update bypass (`level0` byte 4980 / `IsCheckUpdate=0`).
2. Redirect `PlayerDataManager.AccountInit` **0x27972e0** or the session login callback to a local initializer **after the original manager setup**. Allocate original ET response types, fill the essential local profile and invoke the existing manager callbacks above in an order that initializes hero/chapter/items before ready events reach Main.
3. Complete the two initial tasks through the original API and let Init load original Main. Avoid starting the optional account/shop/event request batch. Do not use a predicate-only readiness patch as the complete solution.
4. Preserve original `Main`, `GameSceneManager.PreEnter`, movement/combat methods and packaged assets. Intercept future ET requests requiring progression state and supply local original response messages, then persist that state. Existing local `HandleProtocol` routines can be reused where their original models are compatible; account bootstrap and fight rewards need explicit implementation.

That is a small transport/state replacement around the original engine, not a game clone or a full recompilation from recovered source. The native IL2CPP methods and assets can remain original. This report does not contain the adapter or a validated patch: the remaining concrete work is callback order/state completeness, ET request coverage, persistence and actual device execution.
