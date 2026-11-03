[CmdletBinding(SupportsShouldProcess=$true)]
param([Parameter(Mandatory=$true)][ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._-]*$')][string]$KeepBuildId)
$ErrorActionPreference = 'Stop'
$project = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
$buildRoot = [IO.Path]::GetFullPath((Join-Path $project 'build'))
$boundary = $buildRoot + [IO.Path]::DirectorySeparatorChar
$rootItem = Get-Item -LiteralPath $buildRoot
if ($rootItem.Attributes -band [IO.FileAttributes]::ReparsePoint) { throw 'Build directory cannot be a link' }
. (Join-Path $PSScriptRoot 'paths.ps1')
$paths = Get-KinjaWorkspacePaths -ProjectRoot $project
$apkName = "kinja-run-offline-lab-$KeepBuildId.apk"
$apk = Join-Path $buildRoot $apkName
$signatureName = $apkName + '.signature-verification.txt'
foreach ($name in @($apkName,$signatureName)) {
    if (-not (Test-Path -LiteralPath (Join-Path $buildRoot $name) -PathType Leaf)) { throw "Missing retained artifact: $name" }
}
# Refuse linked descendants, even inside otherwise disposable generated folders.
if (Get-ChildItem -LiteralPath $buildRoot -Force -Recurse -Attributes ReparsePoint) {
    throw 'Build contains links; review targets before cleanup'
}
$historyPath = Join-Path $project 'docs/development/build-history.json'
$history = if (Test-Path -LiteralPath $historyPath) { @(Get-Content -LiteralPath $historyPath -Raw | ConvertFrom-Json) } else { @() }
$known = @{}
foreach ($entry in $history) { $known[$entry.buildId] = $entry }
foreach ($directory in Get-ChildItem -LiteralPath $buildRoot -Directory -Filter 'offline-bridge-*') {
    $planPath = Join-Path $directory.FullName 'build-plan.json'
    if (-not (Test-Path -LiteralPath $planPath)) { continue }
    $plan = Get-Content -LiteralPath $planPath -Raw | ConvertFrom-Json
    $id = $directory.Name.Substring('offline-bridge-'.Length)
    $signed = Join-Path $buildRoot "kinja-run-offline-lab-$id.apk"
    $digest = if (Test-Path -LiteralPath $signed) { (Get-FileHash -LiteralPath $signed -Algorithm SHA256).Hash } else { $null }
    if ($known.ContainsKey($id) -and $known[$id].apkSha256 -and $digest -and $known[$id].apkSha256 -ne $digest) {
        throw "Historical APK hash mismatch: $id"
    }
    $known[$id] = [ordered]@{buildId=$id;package=$plan.package;apkSha256=$digest;nativeSha256=$plan.nativeSha256}
}
if (-not $known.ContainsKey($KeepBuildId)) { throw 'Retained BuildId has no build plan or historical identity' }
$retainedHash = (Get-FileHash -LiteralPath $apk -Algorithm SHA256).Hash
if ($known[$KeepBuildId].apkSha256 -ne $retainedHash) { throw 'Retained APK differs from historical identity' }
$targets = @()
foreach ($item in Get-ChildItem -LiteralPath $buildRoot -Force) {
    if ($item.Name -in @($apkName,$signatureName)) { continue }
    $absolute = [IO.Path]::GetFullPath($item.FullName)
    if (-not $absolute.StartsWith($boundary,[StringComparison]::OrdinalIgnoreCase)) { throw "Target outside build: $absolute" }
    # Preserve configured credentials/inputs even if someone provisions them inside build.
    foreach ($protected in @($paths.originalApk,$paths.keystore,$paths.storePasswordFile,$paths.archiveRoot,$paths.ndkDirectory,$paths.toolchainDirectory,$paths.methodMap)) {
        if ($protected -and ($protected.Equals($absolute,[StringComparison]::OrdinalIgnoreCase) -or
            $protected.StartsWith($absolute + [IO.Path]::DirectorySeparatorChar,[StringComparison]::OrdinalIgnoreCase))) {
            throw "Configured input is inside a cleanup target: $absolute"
        }
    }
    $generated = if ($item.PSIsContainer) { $item.Name -match '^(offline-bridge-.+|tests|migration-wrapper-check)$' }
        else { $item.Name -match '^(kinja-run-offline-lab-.+\.(apk|txt|json)|v[0-9]+-build-output\.txt)$' }
    if (-not $generated) { throw "Unrecognized file in build; refusing to delete: $absolute" }
    $targets += $item
}
$before = [int64](Get-ChildItem -LiteralPath $buildRoot -File -Recurse -Force | Measure-Object Length -Sum).Sum
if ($PSCmdlet.ShouldProcess($buildRoot,"Keep $apkName and signature report; record build identities; remove $($targets.Count) generated entries")) {
    @($known.Values | Sort-Object buildId) | ConvertTo-Json -Depth 5 | Set-Content -LiteralPath $historyPath -Encoding utf8NoBOM
    # All resolved targets and descendant links have been checked above. One shell end-to-end.
    foreach ($item in $targets) { Remove-Item -LiteralPath $item.FullName -Recurse -Force }
    if ((Get-FileHash -LiteralPath $apk -Algorithm SHA256).Hash -ne $retainedHash) { throw 'Retained APK verification failed' }
    $after = [int64](Get-ChildItem -LiteralPath $buildRoot -File -Recurse -Force | Measure-Object Length -Sum).Sum
    [pscustomobject]@{retained=$apkName;sha256=$retainedHash;bytesBefore=$before;bytesAfter=$after;bytesFreed=($before-$after)}
}
