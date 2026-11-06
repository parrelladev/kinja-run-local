param(
    [Parameter(Mandatory=$true)][string]$Serial,
    [string]$OutputDirectory
)

$ErrorActionPreference = 'Stop'
$kinjaProject = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
. (Join-Path $kinjaProject 'tools/workspace/paths.ps1')
$kinjaToolchain = (Get-KinjaWorkspacePaths -ProjectRoot $kinjaProject).toolchainDirectory
$kinjaToolPaths = Get-Content -LiteralPath (Join-Path $kinjaToolchain 'paths.json') -Raw | ConvertFrom-Json
$kinjaJava = Join-Path $kinjaToolchain $kinjaToolPaths.java
$kinjaJavac = Join-Path (Split-Path -Parent $kinjaJava) 'javac.exe'
$kinjaJar = Join-Path (Split-Path -Parent $kinjaJava) 'jar.exe'
$kinjaAdb = Join-Path $kinjaToolchain 'platform-tools/adb.exe'
$kinjaAndroidJar = Join-Path $kinjaToolchain 'android-api33/android.jar'
$kinjaD8 = Join-Path $kinjaToolchain 'android-build-tools/android-37.0/lib/d8.jar'
foreach ($kinjaDependency in @($kinjaJava,$kinjaJavac,$kinjaJar,$kinjaAdb,$kinjaAndroidJar,$kinjaD8)) {
    if (-not (Test-Path -LiteralPath $kinjaDependency -PathType Leaf)) { throw ('Missing configured dependency: '+$kinjaDependency) }
}
if ($Serial -notmatch '^[A-Za-z0-9_.:-]+$') { throw 'Invalid ADB serial' }
$kinjaId = [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')+'-'+[Guid]::NewGuid().ToString('N')
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $kinjaProject ('build/tests/backup-art-'+$kinjaId) }
$kinjaOutput = [IO.Path]::GetFullPath($OutputDirectory)
$kinjaAllowed = [IO.Path]::GetFullPath((Join-Path $kinjaProject 'build/tests'))+[IO.Path]::DirectorySeparatorChar
if (-not $kinjaOutput.StartsWith($kinjaAllowed,[StringComparison]::OrdinalIgnoreCase)) { throw 'Output must be inside workspace build/tests' }
if (Test-Path -LiteralPath $kinjaOutput) { throw 'ART test output must be new' }
$kinjaRemote = '/data/local/tmp/kinja-backup-qa-'+$kinjaId
if ($kinjaRemote -notmatch '^/data/local/tmp/kinja-backup-qa-[a-zA-Z0-9-]+$') { throw 'Invalid synthetic Android sandbox path' }
foreach ($kinjaPart in @('src','classes','dex','assets')) {
    [IO.Directory]::CreateDirectory((Join-Path $kinjaOutput $kinjaPart)) | Out-Null
}
$kinjaSourcePaths = @(
    (Join-Path $kinjaProject 'offline/java/local/kinja/BackupStore.java'),
    (Join-Path $kinjaProject 'offline/java/local/kinja/BackupCatalog.java'),
    (Join-Path $PSScriptRoot 'BackupStoreTest.java')
)
$kinjaHashes = @{}
foreach ($kinjaSource in $kinjaSourcePaths) {
    $kinjaName = [IO.Path]::GetFileName($kinjaSource)
    $kinjaCopy = Join-Path $kinjaOutput ('src/'+$kinjaName)
    Copy-Item -LiteralPath $kinjaSource -Destination $kinjaCopy
    $kinjaHashes[$kinjaName] = (Get-FileHash -LiteralPath $kinjaCopy -Algorithm SHA256).Hash
}
$kinjaSeedNames = @('seed_player.json','seed_regions.json','seed_items.json','seed_equipment.json','seed_talents.json','seed_heroes.json','seed_guide.json','seed_shop_chests.json')
foreach ($kinjaSeed in $kinjaSeedNames) {
    $kinjaCopy = Join-Path $kinjaOutput ('assets/'+$kinjaSeed)
    Copy-Item -LiteralPath (Join-Path $kinjaProject ('offline/assets/'+$kinjaSeed)) -Destination $kinjaCopy
    $kinjaHashes[$kinjaSeed] = (Get-FileHash -LiteralPath $kinjaCopy -Algorithm SHA256).Hash
}
$kinjaClasses = Join-Path $kinjaOutput 'classes'
$kinjaSources = @(Get-ChildItem -LiteralPath (Join-Path $kinjaOutput 'src') -Filter '*.java' -File | ForEach-Object { $_.FullName })
& $kinjaJavac --release 8 -encoding UTF-8 -d $kinjaClasses @kinjaSources
if ($LASTEXITCODE -ne 0) { throw 'ART fixture Java compilation failed' }
$kinjaClassJar = Join-Path $kinjaOutput 'fixture-classes.jar'
& $kinjaJar --create --file $kinjaClassJar -C $kinjaClasses .
if ($LASTEXITCODE -ne 0) { throw 'ART class packaging failed' }
& $kinjaJava -cp $kinjaD8 com.android.tools.r8.D8 --release --min-api 26 --lib $kinjaAndroidJar --output (Join-Path $kinjaOutput 'dex') $kinjaClassJar
if ($LASTEXITCODE -ne 0) { throw 'ART DEX compilation failed' }
$kinjaRunner = Join-Path $kinjaOutput 'runner.jar'
& $kinjaJar --create --file $kinjaRunner -C (Join-Path $kinjaOutput 'dex') classes.dex
if ($LASTEXITCODE -ne 0) { throw 'ART runner packaging failed' }
& $kinjaAdb -s $Serial get-state
if ($LASTEXITCODE -ne 0) { throw 'Selected Android device unavailable' }
$kinjaSdk = (& $kinjaAdb -s $Serial shell getprop ro.build.version.sdk).Trim()
if ($LASTEXITCODE -ne 0 -or [int]$kinjaSdk -lt 26) { throw 'Synthetic ART fixture runner requires Android API26+' }
# Unique mkdir refuses reuse. No app installation, private save access, shell delete,
# system setting change or archive write occurs anywhere in this runner.
& $kinjaAdb -s $Serial shell mkdir -m 700 $kinjaRemote
if ($LASTEXITCODE -ne 0) { throw 'Cannot create unique Android synthetic sandbox' }
& $kinjaAdb -s $Serial shell mkdir ($kinjaRemote+'/assets') ($kinjaRemote+'/fixtures')
if ($LASTEXITCODE -ne 0) { throw 'Cannot prepare synthetic Android fixture directories' }
& $kinjaAdb -s $Serial push $kinjaRunner ($kinjaRemote+'/runner.jar')
if ($LASTEXITCODE -ne 0) { throw 'Cannot transfer synthetic ART runner' }
foreach ($kinjaSeed in $kinjaSeedNames) {
    & $kinjaAdb -s $Serial push (Join-Path $kinjaOutput ('assets/'+$kinjaSeed)) ($kinjaRemote+'/assets/'+$kinjaSeed)
    if ($LASTEXITCODE -ne 0) { throw ('Cannot transfer bundled synthetic seed '+$kinjaSeed) }
}
$kinjaReport = [ordered]@{
    runtime='Android ART app_process'; serial=$Serial; androidApi=$kinjaSdk; remoteSandbox=$kinjaRemote
    productionSources=$kinjaHashes; runnerSha256=(Get-FileHash -LiteralPath $kinjaRunner -Algorithm SHA256).Hash
    scope='Synthetic /data/local/tmp fixtures only; no APK install, private save access, Unity, SAF, OS kill or power-loss validation.'
    artifactsRetained=$true
}
$kinjaReport | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath (Join-Path $kinjaOutput 'run.json') -Encoding UTF8
Write-Output ('Synthetic ART sandbox retained: '+$kinjaRemote)
& $kinjaAdb -s $Serial shell env ('CLASSPATH='+$kinjaRemote+'/runner.jar') app_process /system/bin local.kinja.BackupStoreTest ($kinjaRemote+'/assets') ($kinjaRemote+'/fixtures') android 2>&1 | Tee-Object -FilePath (Join-Path $kinjaOutput 'result.txt')
if ($LASTEXITCODE -ne 0) { throw 'Synthetic ART codec/transaction regressions failed; inspect retained result and sandbox' }
if (-not (Select-String -LiteralPath (Join-Path $kinjaOutput 'result.txt') -Pattern '^PASS assertions=' -Quiet)) { throw 'ART runner did not report completion' }
Write-Output ('ART evidence: '+$kinjaOutput)
