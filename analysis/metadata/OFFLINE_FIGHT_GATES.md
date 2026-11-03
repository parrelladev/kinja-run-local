# Original fight gates — Kinja Run 1.9.1 arm64

Static evidence from `analysis/metadata/libil2cpp.stripes.so`, SHA256
`a2a4510a6650a6bfc9c23949e466392a098f9a1c992e21194b531c1e45babdbe`,
with recovered `analysis/il2cpp/script.json` and `dump.cs`. All addresses below are
ELF virtual addresses in the restored analysis copy, not raw encrypted bytes to
write into the APK. This report makes no APK/device changes and does not claim a
completed runtime validation. IFix can override these base bodies.

## 1. Normal entry can use a completed local task

Observed normal chain:

```
FightRecordManager.ReqEnterFight(configBase = null, isReStart)
  wrapper 0x29ce8a8 / MoveNext 0xf4dd70
  -> calculate hero/equipment properties locally
  -> ET.FightHelper.EnterFight(regionId, enemyCount) 0x29c3728
  -> await ETTask<bool>
  -> if true, OnEnterFightSucceedResp(int regionId) 0x29c7844
  -> GameSceneManager.Enter(regionId, heroIds) 0x2698c10
```

The parent state machine reads `SceneDataManager.CurSelectRegion +0x18` at
`0xf4ed04`, saves it at state-machine `+0x68`, reads
`SceneWidgetCreator +0x158` for enemy count, and calls the helper at
`0xf4ed14` (`85d26994`). `GetResult()` at `0xf4e5d8` is followed by
`0xf4e5dc: tbz w0,#0,0xf4ebe8`; true calls the original normal callback at
`0xf4e5f0` (`95e46994`).

Local property calculation is **in the parent before the helper**:

| Original operation | Call / store VA |
|---|---|
| `HeroManager.GetCurrentHeroProperty` | `0xf4e67c`, `0xf4e6b0`, `0xf4e7c8` |
| `EquipManager.GetWearEquipUidByPart` | `0xf4e700` |
| `EquipManager.GetOwnEquipProperty` | `0xf4e740`, `0xf4e760` |
| `BaseAtk` protected struct at FRM `+0x124` | `0xf4e7e0..0xf4e7f8` |
| `EquipAtk` protected struct at FRM `+0x138` | `0xf4e814..0xf4e82c` |
| `FightBuffManager.CalcPropertyValueWithBuff` | `0xf4e870`, `0xf4e890`, `0xf4e8b4`, `0xf4e8d8` |

Therefore replacing **only** `FightHelper.EnterFight` with a completed original
`ETTask<bool>(true)` preserves those parent calculations, timers, function-state
updates, success callback and original scene code. Returning a raw `bool` from
this method is ABI-incompatible: its return value is a managed task object.

### The network helper and exact entry response

`FightHelper.<EnterFight>d__0.MoveNext` is `0xf462e4`. It gets ET scene / session,
constructs `C2G_EnterGame` (`.ctor 0x108e990`), sets request `RegionId +0x18`
and `EnemyCount +0x14`, calls `Session.Call 0x26c9bd4` at
`0xf46440` (`e50d5e94`), and casts the result to `G2C_EnterGame`.

`G2C_EnterGame` TypeDefIndex 16252, `.ctor 0x27716fc`, layout:

| Offset | Type | Field |
|---|---|---|
| `+0x10` | int | RpcId |
| `+0x14` | int | Error |
| `+0x18` | string | Message |
| `+0x20` | int | CalcBaseAtkWithoutBuff |
| `+0x24` | int | CalcEquipAtkWithoutBuff |
| `+0x28` | int | CalcEquipBaseAtkWithoutBuff |
| `+0x30` | List\<RegionMonsterInfo\> | regionMonsterInfos |

`Error != 0` logs and returns false (`0xf46560..0xf4659c`). Success condition is
`Error == 0`. Each attack override applies only when its value is >=1
(`0xf465a0`, `0xf46618`, `0xf46690`); it writes protected structs at FRM
`+0x124`, `+0x138`, `+0x14c`. Zero preserves their existing values.
`0xf46708` sets true, and `0xf46724` completes the bool builder.
The entry helper does not read or store `regionMonsterInfos`; its original
constructor creates a non-null empty list at `0x2771760`. No `FightId`, random
seed or monster list is copied in this helper. The parent did not show a local
write to FRM `EquipBaseAtk +0x14c`; do not invent one from this trace.

### Original entry callback

`OnEnterFightSucceedResp(int)` clears `RequestEnterFight +0x1f4`, resets current
fight record/rewards/queue, sets current fight region fields `+0x1f8/+0x1fc`,
and enters the original scene. With selected region matching the response
region, `0x29c7f7c` (`2543f397`) calls `GameSceneManager.Enter 0x2698c10`.
With a differing selection it changes `CurSelectRegion` and calls
`PreEnter 0x2697e8c` at `0x29c8030`, whose callback
`0xf4bdbc` tailcalls `Enter` at `0xf4be40` (`74335d14`). An unseen region manga
is shown by `UIManager.ShowManga 0x1262d14` at `0x29c80b4`; its other callback
`0xf4bd30` also reaches the same original entry.

`GameSceneManager.Enter` executes the original scene systems:
`SceneDataManager.EnterFight` (`0x269909c`), equipment/halidom/buff reset,
`CameraFollow.TweenToFight` (`0x26993fc`), `FightDataManager.ResetProperty`
(`0x2699920`), events / `ResetData`, and `InitRegionBrickData` (`0x2699ccc`).
**Inference:** a local completed task can pass the network gate and enter the
original game, provided profile/hero/region/inventory managers and pre-entered
scene assets are valid. This static chain alone does not prove every subsequent
monster or result path works offline.

## 2. Killing a monster is another mandatory await

`FightRecordManager.RecordKillMonster` wrapper is `0x29cde70`; state-machine
MoveNext is `0xf4c4a0`. Normal mode calls `FightHelper.KillEnemy 0x29c4abc`
at `0xf4d084` (`8ede6994`), waits `ETTask<G2C_KillEnemy>`, then accumulates
FRM gains (`+0x70/+0x74/+0x78`), uses response item/equipment lists, executes
original drop creation, `FightRecordItem.AddKillMonster` (`0xf4d774`), events,
and may call `ReqFightResult` (`0xf4d83c`). Skipping the whole recorder would
remove these original updates.

`FightHelper.<KillEnemy>d__19.MoveNext 0xf48b68` calls `Session.Call` at
`0xf48cd0` (`c1035e94`), checks `Error`, conditionally updates the same attack
fields (>0), and completes the response task at `0xf48fb0`.

`G2C_KillEnemy` TypeDefIndex 16254, `.ctor 0x2773b60`:
`+10 int RpcId`, `+14 int Error`, `+18 string Message`,
`+20 int Gold`, `+24 int Diamond`, `+28 int Exp`,
`+30 List<ItemInfo> Items`, `+38 List<int> EquipsUid`,
`+40/+44/+48 int CalcBaseAtkWithoutBuff/CalcEquipAtkWithoutBuff/CalcEquipBaseAtkWithoutBuff`.
For a local response keep both lists non-null. Zero rewards permit the response
processing path but do not recreate the server reward rules. `ItemInfo` is
`+10 int ItemId`, `+14 int ItemCount`, `.ctor 0x2809b44`.

## 3. Normal finish needs a non-null typed result

`ReqFightResult(forceQuit=false)` wrapper `0x29cf6b8`, MoveNext `0x26a629c`,
waits for `PlayerDataManager.PlayerDataIsReady` (`0x26a79bc`) and calls
`FightHelper.EndGame(stage,score,budCount,perfectSkillCount,skillCount)`
`0x29c3f44` at `0x26a7b10` (`0d710c94`). Its state machine
`0xf43d30` constructs `C2G_EndGame` (`.ctor 0x108e78c`) and calls
`Session.Call` at `0xf43ea8` (`4b175e94`). The request fields are
`+10 RpcId`, `+18 long Score`, `+20 Stage`, `+24 BudCount`,
`+28 PerfectSkillCount`, `+2c SkillCount` (all other fields int).

`G2C_EndGame` TypeDefIndex 16256, `.ctor 0x2771058`, exact layout:

| Offset | Type | Field |
|---|---|---|
| `+0x10`, `+0x14`, `+0x18` | int, int, string | RpcId, Error, Message |
| `+0x20` | int | RegionId |
| `+0x24`, `+0x25`, `+0x26`, `+0x27`, `+0x28` | bool each | IsNewStage, IsNewScore, IsNewChapter, IsPass, IsReplay |
| `+0x2c`, `+0x30`, `+0x34`, `+0x38` | int each | LostCount, Gold, Diamond, Exp |
| `+0x40` | List\<ItemInfo\> | Items |
| `+0x48` | List\<int\> | EquipsUid |
| `+0x50`, `+0x54` | int each | PiggyDiamond, BpExp |

The original constructor creates the two lists (`0x27710d4`, `0x277110c`).
`RegionId` must name an existing `GameRegion`: result UI looks it up at
`0x10b03f4`. Make success/passed/new-progress flags reflect local gameplay;
setting them all true does not update the saved region record.

After task result, normal parent checks `Error +0x14` at `0x26a7b64`:
zero saves the result into FRM `_endFightResult +0xd8` at `0x26a84b4`
(`018c0df8` plus write barrier); null or error `0x30d44` retries after 1 second;
other errors call `ProtocolManager.ForceDisconnect` (`0x26a84f8`). Thus returning
null or an uncompleted task is not an offline finish.

After response, `forceQuit=true` immediately calls `TryFightEnd` at `0x26a87c4`.
Otherwise the response remains stored and the original end-of-brick flow will
consume it; if `_waitingForFightResult +0xe0` is set, `TryFightToResult` is
called at `0x26a89fc` and sends `SceneWidgetCreator` toward the brick end.
`TryFightEnd 0x29cf7e8` dispatches by actual response class; normal G2C_EndGame
tailcalls `OnFightResultSucceedResp(G2C_EndGame) 0x29ca650` at
`0x29cfa30` (`08ebff17`). If no recognized response is stored it re-requests.

The normal callback copies `IsNewChapter` into GameSceneManager `+0x1c`
(`0x29cab5c`), copies `IsPass` into FightDataManager protected bool `+0x33c`
(`0x29cab94..0x29cabb4`), calls the original result handling
`GameSceneManager.OnGetFightResult(G2C_EndGame) 0x269604c` at
`0x29cabcc` (`202df397`), and records guide/analytics data.
That scene method opens original `UIFightFail` or `UINewFightResult`; success
closure `0x26b1388` calls `UINewFightResult.ShowResult 0x10b0144` at
`0x26b13d0` (`5dfba797`).

Hooking only `FightHelper.EndGame` to a completed original typed task preserves
these parent/callback/result-UI paths. Its original helper also increments
`ActivityManager.EndGameCount +0xe0` on success (`0xf44038..0xf44040`); a helper
replacement should preserve this small side effect if matching behavior matters.
Currency/inventory/region persistence is separate: neither this helper nor the
normal callback calls `SceneDataManager.OnGetSceneProgress` or the full player
push initialization callback. A local authority must update/persist those models
in addition to displaying rewards.

## 4. All finish callback targets and model coverage

| Response class | Native original callback VA |
|---|---|
| G2C_EndGame | `0x29ca650` |
| G2C_EndGlobefish | `0x29cbb50` |
| G2C_EndWorldCup | `0x29cbd70` |
| G2C_EndActivity | `0x29cbe40` |
| G2C_EndSecret | `0x29cc7b0` |
| G2C_EndBigSecret | `0x29cce9c` |
| G2C_EndDifficultGame | `0x29cd3a0` |
| G2C_EndBountyOrder | `0x29cd8c8` |

Entry callback overloads are: normal int `0x29c7844`; ActivityCfg `0x29c8304`;
SecretCfg `0x29c893c`; DifficultChapterCfg `0x29c8fac`; BountyServerCfg
`0x29ce9f0`. The parent branches call different ET helpers for each mode.
This report establishes the normal solo mode; activity/secret/bounty modes have
additional rank/server state and are not implied to work by a normal task hook.

`fight-response-models.json` contains every exact field/offset/type and constructor
VA for all 8 finish classes, normal entry and kill requests/responses,
`RegionMonsterInfo` and `ItemInfo`. Generated reproducibly by
`tools/metadata/extract_fight_models.py`. Annotated assembly is in corresponding
`FightRecordManager.*.txt`, `ET.FightHelper.*.txt`, `GameSceneManager.*.txt` and
`UINewFightResult.ShowResult.txt`, generated by `trace_offline_player.py`.

Remaining unknowns: reward/progression formulas formerly owned by the server;
full state persistence; subsequent pause/resurgence/update-enemy-count requests;
IFix override behavior; runtime scene/resource validity with the adapter.
`regionMonsterInfos` is unused in the traced normal entry helper, but this is not
a whole-program proof that every serialized field is unused elsewhere.

## 5. Post-clear validation and progression (2026-10-13)

The user completed a full normal-mode match to 100% on the lab APK. The device
log recorded `EndGame stage=11 score=25511`,
`CurrentRegion=11`, `RegionCount=10`, and `passed=1`. The local result returned
two configured item stacks and invoked `ItemManager.AddLocalItem` twice. The
user did not see additional gold or chapter advancement on that build.

The shipped `GameRegionCfgData` entry for region 2 defines a 100% progress
milestone (`progress:[100]`, `progress_reward:[10013]`) and a first-win reward
(`first_win_reward_type:1`, `first_win_reward:100001`). `RewardCfgData` 10013
maps to `item_ids:["40001_2"]` (two items 40001, talent crystals). Item 100001
is a bronze chest (`ItemCfgData`, `param:1001`); reward config 1001 lists 50
jewels for that chest. There is no gold amount in either region 2 reward
configuration, so the local result intentionally grants no coins. The full
server-owned score, gold, experience, and drop formulas remain unknown.

The v15 save marked region 2 passed and set progress to 100, but left
`MaxRegion=2`, `SelectRegion=2`, and no record for region 3. That prevented the
next chapter from appearing. v17 now unlocks the next region on a successful
clear and migrates this prior saved clear during bootstrap. On-device runtime
logs confirmed the migration (`MaxRegion=3`, `SelectRegion=3`), and a read-only
screenshot after relaunch displayed `Capítulo 2`. Item additions were observed
in the runtime log and persisted by the hook; the non-debuggable app's private
inventory file has not been independently read back, so inventory UI
visibility/count remains unverified.

The original `G2C_EndGame` success callback presents the result screen, but
does not invoke `SceneDataManager.OnGetSceneProgress` or the player bootstrap
push. Therefore, returning non-zero result fields and `IsPass:true` is enough to
display a result but is not by itself enough to persist progression/economy.
The local result authority is now implemented in bridge build v15: it marks
the region passed and updates best score/progress, applies configured region 2
items through `ItemManager.AddLocalItem`, and persists region and item JSON for
the next startup. First-win chest and 100% milestone rewards are gated by the
previously saved region record, making those rewards idempotent. It infers a
clear from `FightDataManager.CurrentRegion > RegionCount`; the prior `FightClear`
field is set from the server response and is not an independent win signal.
Gold and experience remain zero because their server formulas have not been
recovered. Device startup and bootstrap hooks are confirmed on the installed
build. The next required validation is a fresh full clear, then checking the
result UI, inventory/progression, and their persistence after restarting the
app; this runtime check is still pending.

Current scope is validated only for the normal solo mode. Other activity,
secret, bounty, ranking, daily, purchase, and ad-backed systems have separate
server-owned state and still need individual offline treatment or an explicit
disabled/unsupported policy.

## 6. Quality-of-life and offline economy (2026-10-13)

Build v18 seeds PlayerStatus IDs `8` (SkipAd) and `9` (InfinitePower), each
with an expiration timestamp at 2100-01-01. On-device bootstrap invoked the
original `PlayerDataManager.CheckPlayerStatusValue` method and observed `1`
for both statuses; the home screen rendered the infinite-energy icon. The
original stage button still displays `x5`, so actual energy deduction has not
yet been verified by an in-game run on v18. The long expiration countdown is
cosmetically noisy and needs a permanent/clean presentation. An unrelated
`HangUpHelper.GetHangUpData` null-reference is logged during home-panel setup;
the home screen still renders, but this online-only feature needs local fallback
or hiding.

Static client metadata identifies gold as `PlayerDataManager.PlayerGold`
(`seed_player.json` field `Coin`), while `ItemManager.AddLocalItem` is the
existing inventory-grant path. The original HeroShop button is a level-7
`btnHeroShop` offer panel and uses online gift purchase state; the separate new
hero menu FunctionCfg 107 is `enable:1` but `show:0` and has an unavailable
condition. Shipped HeroCfg contains two extra playable characters (4002/4003),
each requiring 80 configured fragments through the original unlock scheme.
These facts support the need for an offline-owned local unlock/purchase flow;
they do not prove that simply enabling either original shop would sell heroes
for gameplay-earned gold. User preference is to earn coins in matches and spend
them on the additional heroes. Match coin amounts, purchase prices, UI wiring,
and persistent owned-hero state remain to be implemented and validated.

Build v19 adds `PlayerStatus` 10 (`AdditionalResurrection`) and a checked
ARM64 hook at `AdManager.ShowRewardAdWithCallback` RVA `0xF08120`. Its shipped
prologue bytes matched the original APK, and the device log confirmed the hook
was installed. The replacement invokes the supplied managed ad callback as
`AdFinishType.Complete`; the actual failure/revive screen has not yet been
tested, so only hook installation—not the complete UI flow—is observed. For
the user's Chapter 2 test run, bridge code also sets `FightDataManager` hero
invulnerability and `LevelSpeedRate=4.0` when offline `EnterFight` is called
for region 2. The v19 launch confirmed status 10 is active, but no run has yet
entered that hook; applied difficulty and shorter clear time are pending.

## 7. Chapter 2 test damage and local revive (build v20)

The user tested v19 and reported that the run was not shorter/easier, and that
the revive button still appeared to offer a video; selecting it led to a
pause-like screen without restoring the character. The two v19 assists were
applied before `GameSceneManager.Enter` called the original
`FightDataManager.ResetProperty`, so that reset could overwrite them. The
ineffective invulnerability/speed writes have been removed.

Build v20 installs a checked trampoline hook on `FightDataManager.ResetProperty`
at RVA `0x27DB454`. The original method runs first; while the active region is
2, its `RunningHeroAtk` and `RunningHeroRealAtk` are then set to `9999` through
the original `ObscuredInt` conversion/setters. The trampoline copies only the
verified 16-byte ARM64 stack prologue and branches back to the untouched body.
The device log confirmed this hook was installed, but attack values and combat
effect have not yet been observed during a run.

Build v20 also hooked the verified `UIResurgenceTips.OnClickAd` entry at RVA
`0x145C154`. It bypassed the ad and network request, created a local
`G2C_Resurgence` (`Error=0`, `Type=0`, `Cost=0`, `ResurgenceHp=1.0`), invoked
`GameSceneManager.OnResurgenceSucceed`, and initially attempted to close the
panel through its exit path. Later builds established that direct UI close
behavior was incorrect for the revive flow.

## 8. Preserve original revive coroutine (build v24)

The v23 test showed that `CloseTween` with a null completion callback hides the
revive panel but leaves the scene dark and input blocked. Static disassembly
shows the original `OnClickAd` uses `AdEndCallback` to start
`DoAdResurgence`; that state machine calls `FightHelper.Resurgence(true)`,
awaits its typed result, and continues the original completion path. The bridge
previously skipped this coroutine by replacing `OnClickAd` and manually
invoking `OnResurgenceSucceed`.

Build v24 therefore leaves `UIResurgenceTips.OnClickAd`, `AdEndCallback`,
`DoAdResurgence`, and `GameSceneManager.OnResurgenceSucceed` untouched. The
existing offline rewarded-ad hook immediately invokes the ad callback with
`AdFinishType.Complete`. A checked hook on `FightHelper.Resurgence(bool)` at
RVA `0x29C54C0` supplies the local `G2C_Resurgence` response through the
original `ETTask<G2C_Resurgence>` factory, so no server request is made. Device
startup confirmed the task factory and hook installed. The next device run
must verify that the revive modal disappears and scene brightness/input return
to normal.
