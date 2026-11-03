# Kinja Run offline recreation: initial findings

> Historical snapshot: this file records the initial static-only investigation and predates the working offline JNI/IL2CPP bridge and Android runtime validations. For current architecture, build instructions, implemented hooks, evidence levels and open issues, read [`MAINTENANCE_GUIDE.md`](MAINTENANCE_GUIDE.md). Do not interpret the statements below that no code was executed as current status.

## Target

- APK: `kinja-run-1-9-1.apk`
- SHA-256: `7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e`
- Package name is corroborated by an embedded Play Store URL: `com.habby.kinjarun`.
- REA evidence: artifact graph `ev_e8885fca3509b2e7088eae0dd378a6233577e64d6713f9c63c634793798c9243`; extraction `ev_38dae9d9915fe3bcf76a6e1d89bf1f4a5d2288679f7b2f7e757e9d6fad3645ae`.

## Observed in the supplied APK

- The APK contains 3,594 ZIP entries (about 489 MB expanded). The inventory is complete for the ZIP central directory; the APK signing block is outside that inventory (`ev_e8885fca3509b2e7088eae0dd378a6233577e64d6713f9c63c634793798c9243`).
- It is a Unity 2020.3.34f1 build using IL2CPP: `lib/arm64-v8a/libil2cpp.so`, `global-metadata.dat`, and Unity asset bundles are present. The game bundles use the `.ht` extension but begin with the `UnityFS` header, including `clientdata.ht`, multiple `clientgameobjects*`, texture and audio bundles.
- The APK also contains `Assembly-CSharp.pdb` (1,677,120 bytes), four DEX files, and 3,516 unique file nodes in the artifact graph. The managed game code itself is compiled to native code; the PDB and metadata may help recover names and mapping but are not original C# source.
- Static strings include game service hosts such as `account.kinjarun.com`, `business.kinjarun.com`, `mail.kinjarun.com`, `feedback.kinjarun.com`, and `hu.kinjarun.com/AssetBundlesHotUpdate/`. They also include test endpoints under `kinjiarun.test.hotoongame.com` and a WebSocket endpoint on port 9696. These are embedded references, not proof any endpoint is currently live or was contacted.
- A bounded string scan of `global-metadata.dat` found gameplay identifiers including `_curLevel`, `_curStage`, `_enterFight`, `_endFightResult`, `_heroCfgDic`, `_bossAchievements`, and `_assetBundleRetrievalAttemptCount`. This supports that level, fight, hero, boss, and asset-loading state is represented in the client metadata; it does not recover the methods' control flow or show those systems running offline.
- The packaged client bundles indicate that substantial game content is local. They do not establish that the level catalog, account state, progression, rewards, or game start flow can work without server responses.

## Inferences and unknowns

- An offline solo build appears technically plausible if the client-side gameplay and required levels are in the APK. A faithful port still needs the bootstrap path, local level-selection data, player save model, and gameplay state transitions reconstructed.
- Guilds, bounty challenges, account services, online events, purchases, and cloud content would need to be omitted or replaced with local behavior for the requested no-competition offline edition.
- Static inspection has not established which API responses are mandatory before gameplay starts, which levels are packaged and complete, or whether the content bundle format has additional decryption or integrity requirements.
- No original source project, Unity editor, Android SDK tools, JDK/JADX provider, or Apktool launcher is available in the current workspace session. REA's `inspect_android_package` is blocked because the caller-supplied JADX MCP JAR 0.7.1 is not configured; APK resource decoding is separately unavailable because no Apktool launcher is selected. The native-analysis provider check also found no configured Ghidra installation, and the Windows Ghidra profile does not accept this ARM64 ELF target. These blockers prevent manifest/class decompilation and IL2CPP function analysis here. No APK code was executed.

## Next reconstruction steps

1. Obtain a compatible local JADX MCP JAR and Apktool launcher to decode the Android manifest/resources and inspect Java-side initialization. This will not recover IL2CPP gameplay code.
2. Analyze `libil2cpp.so` together with `global-metadata.dat` and the PDB in a compatible IL2CPP/Ghidra workflow. Trace startup, authentication, remote configuration, asset loading, level selection, and gameplay entry points.
3. Inventory the UnityFS bundles and recover the playable scenes, prefabs, level data, and dependencies needed for a standalone run.
4. Rebuild the smallest playable loop in a local Unity project: local profile/save, local level selection, runner/combat loop, local completion/reward state, and no network or competitive submission path.
5. Build and sign a separate Android APK, then verify installation and fully offline play on the intended Android device.

This is an initial static investigation, not yet a playable recreation. The key technical risk is that the APK contains the assets but only compiled IL2CPP gameplay code, and the required analysis/editor toolchain is absent from this session.
