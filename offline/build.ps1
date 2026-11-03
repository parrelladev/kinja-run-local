param(
    [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._-]*$')][string]$BuildId = 'v3',
    [ValidatePattern('^[A-Za-z][A-Za-z0-9_]*(\.[A-Za-z][A-Za-z0-9_]*)+$')][string]$PackageName = 'com.habby.kinjarun.offline.lab',
    [string]$NativeLibrary,
    [ValidateSet('None','Migration','RewardBoundary')][string]$AccountProgressionFixture = 'None',
    [switch]$PackageApk
)

$ErrorActionPreference = 'Stop'
$kinjaProject = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
if ($AccountProgressionFixture -ne 'None') {
    $kinjaFixturePackage = if ($AccountProgressionFixture -eq 'Migration') {
        'com.habby.kinjarun.offline.qa.expmigration'
    } else { 'com.habby.kinjarun.offline.qa.expreward' }
    if ($PackageName -ne $kinjaFixturePackage) {
        throw "Account EXP fixture requires isolated package $kinjaFixturePackage; existing player profiles are forbidden"
    }
}
$kinjaBuildRoot = Join-Path $kinjaProject 'build'
. (Join-Path $kinjaProject 'tools/workspace/paths.ps1')
$kinjaLocalPaths = Get-KinjaWorkspacePaths -ProjectRoot $kinjaProject
$kinjaWork = Join-Path $kinjaBuildRoot ('offline-bridge-' + $BuildId)
# BuildIds remain unique across the configured historical archive and this checkout.
if ($kinjaLocalPaths.archiveRoot) {
    $kinjaArchiveBuild = Join-Path $kinjaLocalPaths.archiveRoot 'build'
    foreach ($kinjaPriorName in @(('offline-bridge-' + $BuildId), ('kinja-run-offline-lab-' + $BuildId + '.apk'), ('kinja-run-offline-lab-' + $BuildId + '.unsigned.apk'))) {
        if (Test-Path -LiteralPath (Join-Path $kinjaArchiveBuild $kinjaPriorName)) { throw 'BuildId already exists in configured archive; choose a new BuildId' }
    }
}

$kinjaOriginal = $kinjaLocalPaths.originalApk
$kinjaTools = Join-Path $kinjaProject 'tools\android'
$kinjaNdk = Join-Path $kinjaLocalPaths.ndkDirectory 'toolchains/llvm/prebuilt/windows-x86_64'
$kinjaClang = Join-Path $kinjaNdk 'bin\clang++.exe'
$kinjaNm = Join-Path $kinjaNdk 'bin\llvm-nm.exe'
$kinjaReadelf = Join-Path $kinjaNdk 'bin\llvm-readelf.exe'
$kinjaNativeSource = Join-Path $PSScriptRoot 'native\offline_bridge.cpp'
$kinjaSeeds = Join-Path $PSScriptRoot 'assets'
$kinjaRequiredSeeds = @('seed_items.json','seed_equipment.json','seed_talents.json','seed_heroes.json','seed_regions.json','seed_guide.json','seed_player.json','seed_shop_chests.json','seed_hangup.json','seed_hangup_reward.json','seed_kill_enemy.json','seed_end_game.json')
if ((Get-FileHash -LiteralPath $kinjaOriginal -Algorithm SHA256).Hash.ToLowerInvariant() -ne '7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e') { throw 'Original APK digest guard failed' }
if (Test-Path -LiteralPath $kinjaWork) { throw 'Build work directory already exists; choose a new BuildId' }
foreach ($kinjaSeed in $kinjaRequiredSeeds) {
    $kinjaSeedPath = Join-Path $kinjaSeeds $kinjaSeed
    if (-not (Test-Path -LiteralPath $kinjaSeedPath)) { throw ('Missing seed asset: ' + $kinjaSeedPath) }
}
$kinjaAllSeeds = @(Get-ChildItem -LiteralPath $kinjaSeeds -Filter 'seed_*.json' -File | Sort-Object Name | ForEach-Object { $_.Name })
foreach ($kinjaSeed in $kinjaAllSeeds) { Get-Content -LiteralPath (Join-Path $kinjaSeeds $kinjaSeed) -Raw | ConvertFrom-Json | Out-Null }
if (-not $NativeLibrary -and -not (Test-Path -LiteralPath $kinjaNativeSource)) { throw 'Native bridge source has not landed' }
[IO.Directory]::CreateDirectory($kinjaWork) | Out-Null
$kinjaLabel = 'Kinja Run Offline Lab'
if ($AccountProgressionFixture -ne 'None') {
    # Authorized synthetic seeds, only in the new build directory and package.
    # No runtime seeding/replenishment and no modification of normal assets.
    $kinjaFixtureSeeds = Join-Path $kinjaWork 'qa-seeds'
    New-Item -ItemType Directory -Path $kinjaFixtureSeeds | Out-Null
    foreach ($kinjaSeed in $kinjaAllSeeds) { Copy-Item -LiteralPath (Join-Path $kinjaSeeds $kinjaSeed) -Destination $kinjaFixtureSeeds }
    $kinjaSeeds = $kinjaFixtureSeeds
    $kinjaFixturePlayer = Get-Content -LiteralPath (Join-Path $kinjaSeeds 'seed_player.json') -Raw | ConvertFrom-Json
    $kinjaFixturePlayer.Level = 2
    $kinjaFixturePlayer.Exp = if ($AccountProgressionFixture -eq 'Migration') { 5838 } else { 1990 }
    $kinjaFixturePlayer.PlayerName = 'QA EXP ' + $AccountProgressionFixture
    $kinjaFixturePlayer | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $kinjaSeeds 'seed_player.json') -Encoding utf8NoBOM
    if ($AccountProgressionFixture -eq 'RewardBoundary') {
        # Chapter 2 completion exposes HangUp for one ordinary claim via its UI.
        $kinjaFixtureRegions = Get-Content -LiteralPath (Join-Path $kinjaSeeds 'seed_regions.json') -Raw | ConvertFrom-Json
        $kinjaFixtureRegions.SelectRegion = 3
        $kinjaFixtureRegions.MaxRegion = 3
        $kinjaFixtureRegions.MaxRegionProgress = 100
        $kinjaFixtureRegions.RegionsInfo[1].IsPass = $true
        $kinjaFixtureRegions.RegionsInfo += [pscustomobject]@{RegionId=3;IsPass=$true;MaxScore=0;ConsecutiveLostCount=0;DifficultPass=@($false,$false)}
        $kinjaFixtureRegions | ConvertTo-Json -Depth 10 | Set-Content -LiteralPath (Join-Path $kinjaSeeds 'seed_regions.json') -Encoding utf8NoBOM
    }
    $kinjaLabel = 'Kinja QA EXP ' + $AccountProgressionFixture
}
$kinjaNativeOut = Join-Path $kinjaWork 'libkinjaoffline.so'
if ($NativeLibrary) {
    Copy-Item -LiteralPath (Resolve-Path -LiteralPath $NativeLibrary).Path -Destination $kinjaNativeOut
} else {
    & $kinjaClang --target=aarch64-linux-android23 -std=c++17 -shared -fPIC -O2 -g -static-libstdc++ -pthread '-Wl,-soname,libkinjaoffline.so' '-Wl,-z,max-page-size=16384' $kinjaNativeSource -o $kinjaNativeOut -llog -landroid -ldl
    if ($LASTEXITCODE -ne 0) { throw 'Native bridge compilation failed' }
}
$kinjaExports = & $kinjaNm --dynamic --defined-only $kinjaNativeOut
if ($LASTEXITCODE -ne 0) { throw 'Native bridge export inventory failed' }
foreach ($kinjaRequiredExport in @('JNI_OnLoad','Java_local_kinja_OfflineActivity_configure')) {
    if (-not ($kinjaExports -match ('\b' + [regex]::Escape($kinjaRequiredExport) + '$'))) { throw ('Missing native export: ' + $kinjaRequiredExport) }
}
$kinjaExports | Set-Content -LiteralPath (Join-Path $kinjaWork 'native-exports.txt') -Encoding utf8
& $kinjaReadelf -h -d $kinjaNativeOut *> (Join-Path $kinjaWork 'native-layout.txt')
if ($LASTEXITCODE -ne 0) { throw 'Native bridge ELF validation failed' }
if (-not (Select-String -LiteralPath (Join-Path $kinjaWork 'native-layout.txt') -Pattern 'AArch64' -Quiet)) { throw 'Native bridge must target ARM64' }
if (Select-String -LiteralPath (Join-Path $kinjaWork 'native-layout.txt') -Pattern 'libc\+\+_shared' -Quiet) { throw 'Bridge requires an unpackaged libc++ shared library; use static libc++' }
& (Join-Path $PSScriptRoot 'prepare_wrapper.ps1') -OutputDirectory (Join-Path $kinjaWork 'wrapper')
if ($LASTEXITCODE -ne 0) { throw 'Wrapper preparation failed' }
$kinjaPatch = Join-Path $kinjaWork 'patch'
& python (Join-Path $kinjaTools 'prepare_offline_lab.py') --input $kinjaOriginal --output-dir $kinjaPatch --launcher-activity local.kinja.OfflineActivity --package $PackageName --label $kinjaLabel
if ($LASTEXITCODE -ne 0) { throw 'Wrapper manifest preparation failed' }
& python (Join-Path $kinjaTools 'patch_resource_package.py') --input $kinjaOriginal --output (Join-Path $kinjaPatch 'resources.arsc') --new-package $PackageName
if ($LASTEXITCODE -ne 0) { throw 'Resource namespace preparation failed' }
$kinjaRepackArgs = @((Join-Path $kinjaTools 'repack_apk.py'),'--input',$kinjaOriginal,'--align','4',
    '--replace',('AndroidManifest.xml=' + (Join-Path $kinjaPatch 'AndroidManifest.xml')),
    '--replace',('assets/bin/Data/level0=' + (Join-Path $kinjaPatch 'level0')),
    '--replace',('resources.arsc=' + (Join-Path $kinjaPatch 'resources.arsc')),
    '--add',('classes6.dex=' + (Join-Path $kinjaWork 'wrapper\dex\classes.dex')),
    '--add',('lib/arm64-v8a/libkinjaoffline.so=' + $kinjaNativeOut))
foreach ($kinjaSeed in $kinjaAllSeeds) { $kinjaRepackArgs += @('--add',('assets/offline/' + $kinjaSeed + '=' + (Join-Path $kinjaSeeds $kinjaSeed))) }
$kinjaUnsigned = Join-Path $kinjaBuildRoot ('kinja-run-offline-lab-' + $BuildId + '.unsigned.apk')
$kinjaSigned = Join-Path $kinjaBuildRoot ('kinja-run-offline-lab-' + $BuildId + '.apk')
$kinjaPlan = [ordered]@{originalApk=$kinjaOriginal;originalSha256='7953eb369ba9b8c814c4736dac4a7e3aa116a5583837f6f12d795ea70c20595e';
    launcher='local.kinja.OfflineActivity';package=$PackageName;dexMember='classes6.dex';
    nativeMember='lib/arm64-v8a/libkinjaoffline.so';nativeSha256=(Get-FileHash -LiteralPath $kinjaNativeOut -Algorithm SHA256).Hash.ToLowerInvariant();
    wrapperDexSha256=(Get-FileHash -LiteralPath (Join-Path $kinjaWork 'wrapper\dex\classes.dex') -Algorithm SHA256).Hash.ToLowerInvariant();
    seedAssets=$kinjaAllSeeds;accountProgressionFixture=$AccountProgressionFixture;unsignedApk=$kinjaUnsigned;signedApk=$kinjaSigned;packageRequested=[bool]$PackageApk}
$kinjaPlan | ConvertTo-Json -Depth 6 | Set-Content -LiteralPath (Join-Path $kinjaWork 'build-plan.json') -Encoding utf8
if (-not $PackageApk) {
    Write-Output ('Build setup prepared. No APK was written. Plan: ' + (Join-Path $kinjaWork 'build-plan.json'))
    return
}
$kinjaRepackArgs += @('--output',$kinjaUnsigned)
& python @kinjaRepackArgs
if ($LASTEXITCODE -ne 0) { throw 'Offline bridge APK injection failed' }
& (Join-Path $kinjaTools 'sign_apk.ps1') -InputApk $kinjaUnsigned -OutputApk $kinjaSigned -Keystore $kinjaLocalPaths.keystore -Alias offline -StorePasswordFile $kinjaLocalPaths.storePasswordFile
if ($LASTEXITCODE -ne 0) { throw 'Offline bridge APK signing failed' }
Write-Output ('Signed offline bridge build: ' + $kinjaSigned)
# Installation and execution remain a separate action by the coordinating agent.


