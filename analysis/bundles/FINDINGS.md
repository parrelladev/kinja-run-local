# Original APK: Unity content and bootstrap evidence

Target: `D:\Programacao\kinja-run\kinja-run-1-9-1.apk`, SHA-256 `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`.

Source: prior REA extraction `C:\Users\parrella\AppData\Local\Temp\rea-extracted-df1badd0-2245-4966-b856-d921c70678f9`, tied to REA extraction Evidence `ev_38dae9d9915fe3bcf76a6e1d89bf1f4a5d2288679f7b2f7e757e9d6fad3645ae` and APK inventory Evidence `ev_e8885fca3509b2e7088eae0dd378a6233577e64d6713f9c63c634793798c9243`.

This pass parses shipped resources. It does not execute the APK or prove offline gameplay.

## Observed resources

- `inventory.json` records APK-relative path, SHA-256, object type, Unity path ID, containing serialized file, and object position in the uncompressed serialized file for 174 selected source files. Positions inside compressed UnityFS are **not APK byte positions**.
- `clientdata.ht` SHA-256 and all individual TextAsset hashes are in the inventory. It contains 143 data tables, recovered in `tables/clientdata.ht/`. Their payloads use NRBF/.NET binary serialization and are readable without running .NET or instantiating its classes.
- 4,995 TextAssets were extracted from the selected original bundles. 4,994 parse as NRBF; all consumed their full payload and no parser errors occurred. The remaining asset is the plain-text weapon behavior list `_all_behaviour` in `clientleveldata3.ht`, 608 bytes, a list separated by `|`.
- Every JSON object retains `$class`, `$id`, `$offset` and `$member_offsets`. Those offsets address the **extracted TextAsset NRBF payload**. `table_index.json` links each JSON to its payload and reports row counts, libraries, classes, record types and exact consumption.
- Recovered gameplay configuration includes `GameRegion` (700 records), `GameStage` (741), `Monster` (11,336), `BasicSkill` (74), `Skill` (72), `BuffCfg` (1,095), `BrickRandomGroupCfg` (733), `BlockMatrixCfg` (875), hero configuration (3), player operation parameters and rewards.
- Level geometry is original serialized `SceneWidgetRTData` with `Uid`, `Data`, fog settings, camera settings and widget collections. Each widget contains its original source prefab path, position, rotation, scale, collision damage, pickup effects, HP, skills, movement and trigger parameters. Boss and weapon behavior data include `BossBD`, `BossBOD`, `WeaponBehaviorData` and associated groups.
- `clientresourcesdatabase.ht` contains a `ClientResourcesDatabase` MonoBehaviour, path ID `6070231976214901267`, in `CAB-3553962663b5f98698fff0f5c50da0cf`, object position `5624`, object size `2031040`. Its 7,631 resource entries map original asset paths and aliases to bundles and dependencies. Saved separately as `asset_database.json`.
- That database references 819 distinct bundle names. All 819 are present in the extracted APK directory. This confirms **file presence**, not that every reference loads successfully during gameplay.
- No file named `clientconfig*.ht` is present in the extracted `assets/AssetBundles/Android` directory. Most table configurations are in `clientdata.ht`; additional character, pillar, reward and level configurations are MonoBehaviours in `clientgameobjects*.ht`.

## Tutorial content closure

`tutorial_content_seed.json` contains observed data and dependencies for original region `1`, stage `101`, brick group `4001`, the stage's direct bricks and their prefab paths. It is an evidence artifact, **not an executable offline player profile**.

- `Birth` record 1 grants hero `[1001]`, gold `1500`, jewels `0`, no equipment/items.
- `GameRegion` record 1 references stage `[101]` and resources `platform_tex_res=100000`, `platform_res=80000`, `boss_res=90001`.
- `GameStage` record 101 references group `[4001]`; foreground `f_chutan`, background `b_chutan`; entry `330001`, finish `320009`, start `320008`, loop/merge `320011`, boss brick `330076`.
- Group 4001 has `use_matrix=0`, `not_random=1`, `select_count=2`, bricks `[280006,280007]`.
- All seven distinct direct/selected brick UIDs have original TextAssets. Every prefab path from those seven bricks resolves in the local resource database. No missing brick or unresolved prefab path was found in this bounded closure.
- `HeroOperationCfg` 10000 contains original parameters, including `RunSpeed=100`, `JumpPower=600`, `DoubleJumpPower=600`, `MaxForwardSpeed=110`, drag, acceleration and jump timing. Units and runtime interpretation must be established from code.

## Bootstrap settings and exact patch positions

Original `assets/bin/Data/level0` has Init GameObject path ID `2` and a MonoBehaviour path ID `10` starting at byte **4944**, length **92**. Its MonoScript pointer selects global script path ID **1755**, the `Init` class in `Assembly-CSharp.dll`. The script catalogue was recovered by concatenating original `globalgamemanagers.assets.split0` and `.split1`; joined copy is in `joined/globalgamemanagers.assets`.

The same Init component appears with a complete type tree in `clientinit4a1687ee0539795d596daafb355f8d81.ht`, `BuildPlayer-Init` path ID `10`, object position **15872**, length **92**. The original scene bundle exposes `Assets/Resources/Scenes/Init.unity`.

| Field | Type | Component-relative offset | level0 absolute offset | Original value |
|---|---|---:|---:|---|
| IsRes | UInt8 + align4 | 32 | 4976 | 0 |
| IsCheckUpdate | UInt8 + align4 | 36 | **4980** | **1** |
| IsDebug | UInt8 + align4 | 40 | 4984 | 0 |
| IsCombine | UInt8 + align4 | 44 | 4988 | 1 |
| IsUWA | UInt8 + align4 | 48 | 4992 | 0 |
| LanguageType | int32 | 52 | 4996 | 0 |
| AssetServer | length + UTF8 + align4 | 56 | 5000 | Android_release_v1_9_1/ |
| ServerAddress | int32 | 84 | 5028 | 2 |
| DownloadThreadCount | int32 | 88 | 5032 | 6 |

`IsCheckUpdate` occupies one byte; there are three alignment bytes after it. Setting original level0 byte 4980 from `01` to `00` preserves its size and structural layout. In the **uncompressed** bundle scene `BuildPlayer-Init`, the corresponding byte is **15908**. A compressed bundle cannot be patched by using that number as a physical file offset.

Observation: the settings enable update checks. Inference: `IsCheckUpdate=0` is a likely way to disable the initial content update operation. Unknown: whether that affects login, response validation, gameplay initialization or any other network gate. The flag name alone does not prove its native method control flow.

Loading uses `LoadingSceneHandle` (MonoScript path ID 7 in `BuildPlayer-Loading.sharedAssets`); Main uses `Main` (script path ID 3 in `BuildPlayer-Main.sharedAssets`), with serialized fields `HeroAnim`, `WeaponAnim`, `FirstOpenGame=0`; relink uses `Relink` (script path ID 3 in its bundle). `level1` MonoBehaviour points to script catalogue path ID 2846, also `Relink`.

## Reproduction and limitations

Tools are isolated under `tools/bundles`, including UnityPy 1.25.4 installed into `tools/bundles/python-libs`. Scripts `extract_bundles.py`, `nrbf_reader.py`, and `summarize_content.py` only read game bytes and write analysis artifacts. The NRBF parser uses the Microsoft record grammar: https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-nrbf/954a0657-b901-4813-9398-4ec732fe8b32 . It does not invoke BinaryFormatter, load the referenced assemblies or execute constructors.

All byte consumption and dependency checks are static. There is no second independent NRBF parser verification, runtime observation, restored login state, method-body reconstruction or recompilation in this scope. The extracted content and tables do establish a substantial local original content set, improving the feasibility of modifying the actual client. To make it playable, the native startup/login path and original player-state model still have to be traced and adapted.

The evidence artifacts occupy approximately 827 MiB, largely JSON with per-field positions. The original APK was not modified by this worker.
