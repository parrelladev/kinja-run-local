# Original player bootstrap: static findings and adapter points

Source: `analysis/metadata/libil2cpp.stripes.so`, SHA-256
`a2a4510a6650a6bfc9c23949e466392a098f9a1c992e21194b531c1e45babdbe`,
completed Il2CppDumper `analysis/il2cpp/dump.cs` and `script.json`.
This investigation did not launch or modify an APK. The disassemblies in this
directory include instruction bytes, VAs, direct call labels, and metadata-handle
annotations. Generic native bodies are shared; their preceding MethodInfo handle
determines the specialization. Names/signatures in dump.cs do not supply original
C# method bodies.

## 1. Original managers can receive a local profile

`PlayerDataManager.Init` **0x2796920** creates the existing manager singletons
before authentication. Its last section resets the login counters, adds two
tasks, installs login-progress UI, and tail-calls `AccountInit` **0x27972e0**.
The last account transition is **0x27972d8**, opcode `02000014`,
`b 0x27972e0`. This is a useful boundary for replacing account/session bootstrap
with a local initializer while retaining the game's original manager creation.

`AccountInit` clears bool `server_disconnect`, emits an analytics login stage,
creates the original `Action<int,string,LoginError>` callback
`<AccountInit>b__65_0` **0x279a154**, and tail-calls
`HabbyAccountLogin` **0x27976b0** at **0x2797448** (`9a000014`).
When the callback receives `code==0`, its success branch at **0x279a280**
eventually calls `Login(string token)` **0x2797dec** at **0x279a2cc**
(`c8f6ff17`). Therefore fabricating account success alone still enters network
session login.

The current session path is:

```text
AccountInit -> HabbyAccountLogin -> successful account callback
  -> PlayerDataManager.Login(token)
  -> PlayerDataManager.<Login>d__73.MoveNext (0x27ea348)
  -> ProtocolManager.Login(address, token) (0x2a96c5c)
  -> ET.LoginHelper.Login / <Login>d__0.MoveNext (0x2818ef0)
  -> DNS, NetKcp session, C2G_Login, Session.Call
```

The caller awaits a real `ETTask<G2C_Login>`, not a bool. The protocol callsite is
**0x27eac78**, opcode `f9af0a94`. `GetServerConnect` **0x2d0f4fc** selects
primary/fallback strings from the packaged configurations at **0x27eaa50** and
**0x27eabe4**. These are addresses, not an offline toggle.
ET's state machine creates `C2G_Login` at **0x2819394**, then calls
`ET.Session.Call` **0x26c9bd4** at **0x2819424** (`ecc1fa97`). An offline adapter
should bypass this bootstrap path or provide a completed original response task.

## 2. Exact player response and original callback chain

The active value model is `ET.G2C_PlayerData`, type index **16249**. It is
different from ProtocolManager's legacy `PlayerAllData` at manager offset
`0x150`. The current server push handler `ET.G2C_PlayerDataHandler.Run`
**0x26872e4** gets the original PlayerDataManager singleton and tail-calls
`OnPlayerValueDataResp` **0x2796494** at **0x26873b4** (`383c0414`).

`OnPlayerValueDataResp` is the appropriate existing callback for local data:

| Callsite | Opcode | Original effect |
|---|---|---|
| `0x279651c` | `60fcff97` | `OnGetPlayerAllData(response)` at `0x279569c` |
| `0x2796524` | `1a000094` | `SyncValueToShow()` at `0x279658c` |
| `0x2796530` | `54000094` | `set_PlayerDataIsReady(true)` at `0x2796680` |
| `0x2796538` | `79000094` | `CompleteLoginTask()` at `0x279671c` |
| `0x2796584` | `fd21ab17` | `UIManager.NetworkLoading(false)` at `0x125ed78` |

`OnGetPlayerAllData` stores the response in `_allData` (object offset **0x18**)
at **0x27958bc** (`138c01f8`) and uses the native GC write barrier. It rebuilds
the original obscured values, registration date/time, player status dictionary,
guide state, login-day counters, and UI indicators. It calls the original
`ObscuredInt.op_Implicit` **0x2f6220c** rather than storing raw integers over
the protected structs.

The response constructor **0x2687214** creates non-null empty
`List<string> PlayerStatus` and `List<long> PlayerStatusTime` at offsets `0x60`
and `0x68`. It is useful for building a minimal response with the original type.
The profile's native layout is:

| Offset | Field | Type |
|---|---|---|
| `0x10` | PlayerName | string |
| `0x18`, `0x20` | RegTime, LastTime | long |
| `0x28` | Lang | string |
| `0x30` | Level | int |
| `0x38`, `0x40`, `0x48`, `0x50` | Coin, Diamond, Power, Exp | long |
| `0x58`, `0x5c` | IsNewDay, CumulativeLoginDays | bool, int |
| `0x60`, `0x68` | PlayerStatus, PlayerStatusTime | matching lists |
| `0x70`, `0x74`, `0x78` | RenameCount, Icon, IconFrame | int |
| `0x80`, `0x88` | Union, UnionHelpLastTime | long |

Empty matching status lists are accepted by the observed loop; null lists cause
the original null-reference path. Constructors and setters should supply managed
objects and GC barriers. `RegTime` is treated as seconds added to
`DateTime(1970,1,1,1,1,1)` in the observed callback: the constructor is at
**0x27959f4**, and `DateTime.AddSeconds` is called at **0x2795a10**. The use of
01:01:01 is directly in the instructions, so a consumer should preserve the
original convention rather than assume a different epoch without device checks.

The public response has no player ID. Stable local identity also requires the
original ProtocolManager player ID and related account/guide/region state to be
initialized; writing the value response alone does not do that.

## 3. Event order and exact login task accounting

`PlayerDataIsReady` is an in-memory bool named `player_data_get_ready`, accessed
through PlayerDataManager's `GetBool/SetBool`; it is not the task counter.

| Method | VA | Base body |
|---|---:|---|
| ResetLoginTask | `0x2796850` | `str xzr,[this,#0x78]` at `0x2796894`; resets both consecutive int counters |
| AddLoginTaskCount | `0x279677c` | increments int at `this+0x7c`, instructions `0x27967c0..0x27967c8` |
| CompleteLoginTask | `0x279671c` | increments int at `this+0x78`, instructions `0x2796760..0x2796768` |
| LoginIsComplete | `0x27967dc` | true if target `<1`, otherwise completed `>=target`, `0x2796820..0x2796840` |

Initial `Init()` calls `ResetLoginTask` at **0x279718c** and registers **two**
tasks at **0x2797194** and **0x279719c**. Their normal completions are the
successful session-login branch **0x27eab88** and the value-response callback
**0x2796538**.

Inside `OnGetPlayerAllData`, **before** setting ready and completing the value
task, the original code triggers **GameEventName.PlayerDataInit**. It reads the
static field at offset **0x3a8** and calls `EventManager.TriggerEvent`
**0x2adef04** at **0x2795d34** (`74240d94`). This event can synchronously reach
Init/Main and their required hero/chapter state. A local adapter should populate
the other essential managers before delivering this response.

`Init.OnPlayerDataInit` **0x2805174** responds by closing loading UI, clearing
`login_process`, adding **one more** task at **0x280534c** (`0c45fe97`), and
loading original scene **Main** through `ResourceManager.LoadSceneAsync`
**0x289aff8** at **0x2805448** (`ec560214`). The scene callback
`Init.<>c.<OnPlayerDataInit>b__20_0` **0x281133c** unloads Init and completes
that task at **0x2811458** (`b114fe17`). With this listener active, the core
bootstrap therefore has **three** tasks: account/session, values, Main scene.

Normal `Login(token)` additionally calls `PullActivityData` **0x2797f1c** at
**0x27eb1c0**. That batch has five additional task-registration sites:

| Registration site | Request | Completion callback site |
|---|---|---|
| `0x279816c` | Piggy bank info | `0x27e939c` |
| `0x27983ac` | Battle pass data | `0x27e9790` |
| `0x2798434` | Little battle pass data | `0x27e9888` |
| `0x279855c` | Activity info | `0x27e9aac` |
| `0x279867c` | Accumulated purchase data | `0x27e9d0c` |

Other promotional calls in that batch do not register a task at their direct
callsites. Conditions can change which requests are reached. Thus a normal
login can grow from the core count of three to as many as eight in the traced
base path. The bounded direct-call scan is saved in `player-callers.json` and
covers the restored `il2cpp` executable section; virtual/hotfix calls are a
separate coverage limit.

An adapter that bypasses the normal network `Login(token)` must complete the
account/session task itself once, let `OnPlayerValueDataResp` complete the value
task, and let the existing Main scene callback complete the scene task. It
should skip the optional request batch or feed each actual completion callback.
Simply returning true from readiness checks leaves uninitialized data and can
hide pending work.

## 4. Original Main and gameplay still have required entry points

`Main.Awake` **0x29e1460** subscribes `Main.OnPlayerDataInit`
**0x29e17f8** to `GameEventName.PlayerDataInit`, then calls
`PlayerDataManager.Init` at **0x29e1680**. Each Init call resets counters; do not
deliver profile initialization twice or attach a local bootstrap indiscriminately
to every scene without checking the original lifecycle.

`Main.OnPlayerDataInit` reads `SceneDataManager+0x20`. When it equals 1, it sets
the selected region (`SceneDataManager+0x18`) to 2 and calls
`GameSceneManager.PreEnter` **0x2697e8c** at **0x29e1a90** with the selected
region and the original equipped hero. Otherwise it starts the original
`OpenMainPanel` coroutine **0x29e1c44** at **0x29e1ab0**.
The original field layouts identify those SceneDataManager fields; raw region
integers must be chosen from the packaged catalog rather than inferred from the
method names alone.

The main-panel coroutine **0x281bc0c** opens `UIMainPanel` through original
`UIManager.Open<UIMainPanel>` at **0x281bf8c**, waits for the instantiated UI,
and later checks `LoginIsComplete` at **0x281c374**. It also requests chapter
gifts/cloud mail config/idle data on some branches. These services need local
defaults or bypasses to avoid further waits.

The first-region `PreEnter` completion callback
`Main.<>c.<OnPlayerDataInit>b__9_0` **0x281b53c** calls
`FightRecordManager.ReqEnterFight` **0x29ce8a8** at **0x281b5cc**. A local value
profile and scene load alone therefore do not prove fight entry will succeed;
the fight-entry response path must be adapted as well.

## 5. Smallest supported adaptation, and what is still missing

The smallest supported direction is a local bootstrap/transport adapter around
the original client:

1. Intercept AccountInit **0x27972e0**, after original manager creation, and
   initialize a stable local identity plus the original hero, region, equipment,
   item, talent, and guide response models. Their original handlers and shapes
   are documented in `analysis/bundles/offline-bootstrap.md`.
2. Construct the original `ET.G2C_PlayerData` and invoke
   `OnPlayerValueDataResp` **0x2796494** on the Unity main thread after those
   managers are ready. Preserve its event, obscured-value, readiness and task
   operations. Complete the account task through original `CompleteLoginTask`.
3. Preserve original Main/scene loading and gameplay. Avoid launching the normal
   SDK/network login and optional shop/activity batch from this local bootstrap.
4. Adapt subsequent original ET requests, including fight entry/results/rewards,
   into local responses and persist the resulting profile. The legacy local
   HandleProtocol subset does not supply current ET login/profile state.

This report identifies the original integration points. It does not contain a
working adapter or a verified offline gameplay patch. Remaining barriers are:
full identity/hero/region consistency; main-thread invocation and callback order;
conditional UI requests; fight entry and progression response coverage; complete
local persistence; native/APK integrity checks; and device validation.

All relevant methods also have IFix checks. AccountInit uses IFix ID **0xcea**;
OnGetPlayerAllData uses **0x1707**. A compatible packaged IFix patch can supersede
the base body, so device behavior and loaded IFix payloads must be reconciled.
No external payload was fetched in this worker's scope.

## 6. Protected native patch planning

Native changes must be mapped back into the original XOR/permuted stripes.
`tools/metadata/map_native_patch.py` prints the byte plan and refuses opaque
AES ranges. It also supplies `integrity_deltas(raw, edits)` for the original
section CRC descriptors.

`libanort.so` routine **0x10fabc** uses standard CRC32 polynomial `0xedb88320`,
initial `0xffffffff`, final inversion. All 256 entries at table **0x14adc0**
match that algorithm. The section decoder calls it at **0x6e7b8** and compares
the descriptor CRC at **0x6e7dc**. Original descriptor CRC file offsets are
`.text=0x4a83030`, `il2cpp=0x4a8305c`, `.rodata=0x4a83088`.

Because CRC32 is affine over fixed-length messages, the old declared CRC plus
the plaintext byte XOR delta yields the new CRC without recovering unrelated
opaque AES bytes. The helper plans this update without modifying input files.
Other integrity/signature checks are outside this helper's confirmed coverage.
