param([Parameter(Mandatory=$true)][string]$Serial)
$ErrorActionPreference = 'Stop'
$project = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
. (Join-Path $project 'tools/workspace/paths.ps1')
$paths = Get-KinjaWorkspacePaths -ProjectRoot $project
$output = Join-Path $project 'build/tests/account-progression'
New-Item -ItemType Directory -Path $output -Force | Out-Null
$table = Join-Path $paths.archiveRoot 'analysis/bundles/tables/clientdata.ht/150393219823674535_AccountLevelCfgCfgData.json'
$rows = @(Get-Content -LiteralPath $table -Raw | ConvertFrom-Json | Sort-Object level)
if ($rows.Count -ne 200) { throw 'Expected recovered 1.9.1 table with 200 rows' }
for ($i = 0; $i -lt $rows.Count; $i++) {
    if ($rows[$i].level -ne ($i + 1) -or $rows[$i].cost_exp -le 0) { throw 'Invalid recovered level table' }
}
$fixture = '#include <vector>' + "`n" + 'const std::vector<int> costs = {' + (($rows | ForEach-Object { $_.cost_exp }) -join ',') + '};'
Set-Content -LiteralPath (Join-Path $output 'account_cost_fixture.h') -Value $fixture -Encoding utf8
# The pure planner runs as x86_64 in BlueStacks; the bridge itself is ARM64.
$compiler = Join-Path $paths.ndkDirectory 'toolchains/llvm/prebuilt/windows-x86_64/bin/x86_64-linux-android23-clang++.cmd'
$binary = Join-Path $output 'account-progression-test'
& $compiler -std=c++17 -O2 -Wall -Wextra -static-libstdc++ '-Wl,-z,max-page-size=16384' "-I$output" (Join-Path $PSScriptRoot 'account_progression_test.cpp') -o $binary
if ($LASTEXITCODE -ne 0) { throw 'Planner test compilation failed' }
$adb = Join-Path $paths.toolchainDirectory 'platform-tools/adb.exe'
$remote = '/data/local/tmp/kinja-account-progression-test'
& $adb -s $Serial push $binary $remote
if ($LASTEXITCODE -ne 0) { throw 'Cannot copy isolated test executable' }
& $adb -s $Serial shell chmod 755 $remote
if ($LASTEXITCODE -ne 0) { throw 'Cannot enable isolated test executable' }
& $adb -s $Serial shell $remote | Tee-Object -FilePath (Join-Path $output 'result.txt')
if ($LASTEXITCODE -ne 0) { throw 'Planner regression failed' }
# No access to the app private files, inventory, or saves.
