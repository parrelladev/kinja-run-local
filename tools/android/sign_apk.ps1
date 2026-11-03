param(
    [Parameter(Mandatory=$true)][string]$InputApk,
    [Parameter(Mandatory=$true)][string]$OutputApk,
    [Parameter(Mandatory=$true)][string]$Keystore,
    [Parameter(Mandatory=$true)][string]$Alias,
    [string]$StorePasswordEnv,
    [string]$KeyPasswordEnv,
    [string]$StorePasswordFile
)

$ErrorActionPreference = 'Stop'
$kinjaProject = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
. (Join-Path $kinjaProject 'tools/workspace/paths.ps1')
$kinjaToolchain = (Get-KinjaWorkspacePaths -ProjectRoot $kinjaProject).toolchainDirectory
$kinjaPaths = Get-Content -LiteralPath (Join-Path $kinjaToolchain 'paths.json') -Raw | ConvertFrom-Json
$kinjaJava = Join-Path $kinjaToolchain $kinjaPaths.java
$kinjaApksigner = Join-Path $kinjaToolchain $kinjaPaths.apksignerJar
$kinjaZipalign = Join-Path $kinjaToolchain $kinjaPaths.zipalign
$kinjaInput = (Resolve-Path -LiteralPath $InputApk).Path
$kinjaKeystore = (Resolve-Path -LiteralPath $Keystore).Path
$kinjaOutput = [System.IO.Path]::GetFullPath($OutputApk)
$kinjaAligned = $kinjaOutput + '.aligned.unsigned.apk'
if ($kinjaOutput -eq $kinjaInput -or (Test-Path -LiteralPath $kinjaOutput)) {
    throw 'Output must be a new file, distinct from the input APK'
}
[System.IO.Directory]::CreateDirectory([System.IO.Path]::GetDirectoryName($kinjaOutput)) | Out-Null
# Prefer an APK already aligned by the format-aware Python repacker. Native
# zipalign write mode currently crashes on this host; its read-only check works.
& $kinjaZipalign -c -P 16 4 $kinjaInput
if ($LASTEXITCODE -eq 0) {
    $kinjaSignInput = $kinjaInput
} else {
    if (Test-Path -LiteralPath $kinjaAligned) { throw 'Alignment output already exists' }
    & $kinjaZipalign -P 16 4 $kinjaInput $kinjaAligned
    if ($LASTEXITCODE -ne 0) { throw 'zipalign write failed; prepare unsigned APK with repack_apk.py --align 4' }
    $kinjaSignInput = $kinjaAligned
}
$kinjaSignArgs = @('-jar', $kinjaApksigner, 'sign', '--ks', $kinjaKeystore, '--ks-key-alias', $Alias,
    '--min-sdk-version', '23', '--v1-signing-enabled', 'true', '--v2-signing-enabled', 'true',
    '--v3-signing-enabled', 'true', '--v4-signing-enabled', 'false', '--out', $kinjaOutput)
if ($StorePasswordEnv) {
    if ($StorePasswordFile) { throw 'Select either a password file or environment variable' }
    if (-not [Environment]::GetEnvironmentVariable($StorePasswordEnv)) { throw 'Store password environment variable is empty' }
    $kinjaSignArgs += @('--ks-pass', ('env:' + $StorePasswordEnv))
}
if ($StorePasswordFile) {
    $kinjaStorePasswordPath = (Resolve-Path -LiteralPath $StorePasswordFile).Path
    $kinjaSignArgs += @('--ks-pass', ('file:' + $kinjaStorePasswordPath))
}
if ($KeyPasswordEnv) {
    if (-not [Environment]::GetEnvironmentVariable($KeyPasswordEnv)) { throw 'Key password environment variable is empty' }
    $kinjaSignArgs += @('--key-pass', ('env:' + $KeyPasswordEnv))
}
$kinjaSignArgs += $kinjaSignInput
& $kinjaJava @kinjaSignArgs
if ($LASTEXITCODE -ne 0) { throw 'apksigner sign failed' }
& $kinjaJava -jar $kinjaApksigner verify --verbose --print-certs $kinjaOutput 2>&1 |
    Tee-Object -FilePath ($kinjaOutput + '.signature-verification.txt')
if ($LASTEXITCODE -ne 0) { throw 'Signed APK signature verification failed' }
& $kinjaZipalign -c -P 16 4 $kinjaOutput
if ($LASTEXITCODE -ne 0) { throw 'Signed APK alignment verification failed' }
Get-FileHash -LiteralPath $kinjaOutput -Algorithm SHA256
