param(
    [string]$NdkDirectory,
    [switch]$DiagnosticNoWorker,
    [switch]$QaMaxProfile,
    [switch]$QaFusionRepeats
)
$ErrorActionPreference = 'Stop'
$offlineNativeRoot = $PSScriptRoot
$offlineProjectRoot = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
. (Join-Path $offlineProjectRoot 'tools/workspace/paths.ps1')
if (-not $NdkDirectory) { $NdkDirectory = (Get-KinjaWorkspacePaths -ProjectRoot $offlineProjectRoot).ndkDirectory }
$offlineBuildRoot = Join-Path (Split-Path -Parent $offlineNativeRoot) 'build\arm64-v8a'
$offlineCompiler = Join-Path $NdkDirectory 'toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android23-clang++.cmd'
if (-not (Test-Path -LiteralPath $offlineCompiler -PathType Leaf)) {
    throw "Android ARM64 compiler unavailable: $offlineCompiler"
}
New-Item -ItemType Directory -Path $offlineBuildRoot -Force | Out-Null
$offlineSource = Join-Path $offlineNativeRoot 'offline_bridge.cpp'
$offlineOutput = Join-Path $offlineBuildRoot 'libkinjaoffline.so'
$offlineExtraFlags = @()
if ($DiagnosticNoWorker) { $offlineExtraFlags += '-DKINJA_OFFLINE_DIAGNOSTIC_NO_WORKER=1' }
if ($QaMaxProfile) { $offlineExtraFlags += '-DKINJA_OFFLINE_QA_MAX_PROFILE=1' }
if ($QaFusionRepeats) { $offlineExtraFlags += '-DKINJA_OFFLINE_QA_FUSION_REPEATS=1' }
& $offlineCompiler -std=c++17 -O2 -Wall -Wextra -fPIC -shared -static-libstdc++ '-Wl,-z,max-page-size=16384' @offlineExtraFlags $offlineSource -o $offlineOutput -llog -landroid -ldl
if ($LASTEXITCODE -ne 0) { throw "Native compilation failed with code $LASTEXITCODE" }
Get-FileHash -LiteralPath $offlineOutput -Algorithm SHA256
