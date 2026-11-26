<#
.SYNOPSIS
Roda todos os testes de host do projeto e mostra um resumo.

.DESCRIPTION
- BackupStoreTest (Java): sempre executado, com o JDK do toolchain configurado.
- Testes C++ (tests/*_test.cpp): usam um compilador de host (g++ ou clang++ no PATH),
  se existir, e então são executados aqui. Sem compilador de host, são compilados
  com o clang do NDK para Android e:
    * sem -Serial: ficam só COMPILADOS (não executados);
    * com -Serial <dispositivo>: são enviados para /data/local/tmp/kinja-host-tests
      no emulador/aparelho por ADB e executados lá. Isso não instala APK nem
      toca em dados de aplicativos.
- account_progression_test precisa da tabela AccountLevelCfg do acervo externo;
  sem ela, é marcado como IGNORADO.

Código de saída 1 se algum teste FALHOU. "COMPILADO" não é falha, mas também
não é aprovação: registre assim ao validar uma build.

.EXAMPLE
./tests/run-host-tests.ps1
./tests/run-host-tests.ps1 -Serial 127.0.0.1:5555
#>
param(
    [string]$Serial,
    [ValidateSet('x86_64','aarch64')][string]$AndroidAbi = 'x86_64',
    [string]$OutputDirectory
)
$ErrorActionPreference = 'Stop'
$project = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
. (Join-Path $project 'tools/workspace/paths.ps1')
$paths = Get-KinjaWorkspacePaths -ProjectRoot $project
if (-not $OutputDirectory) {
    $OutputDirectory = Join-Path $project ('build/tests/host-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss'))
}
$out = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $out) { throw "A pasta de saída precisa ser nova: $out" }
New-Item -ItemType Directory -Path $out -Force | Out-Null

# Runs a native program capturing stdout+stderr without PowerShell 5.1 turning stderr into errors.
function Invoke-Native([string]$File, [string[]]$Arguments) {
    $info = New-Object System.Diagnostics.ProcessStartInfo
    $info.FileName = $File
    $info.Arguments = ($Arguments | ForEach-Object { if ($_ -match '[\s"]') { '"' + ($_ -replace '"','\"') + '"' } else { $_ } }) -join ' '
    $info.UseShellExecute = $false
    $info.RedirectStandardOutput = $true
    $info.RedirectStandardError = $true
    $info.CreateNoWindow = $true
    $process = [Diagnostics.Process]::Start($info)
    $stderrTask = $process.StandardError.ReadToEndAsync()
    $stdout = $process.StandardOutput.ReadToEnd()
    $process.WaitForExit()
    [pscustomobject]@{ ExitCode = $process.ExitCode; Output = ($stdout + $stderrTask.Result).Trim() }
}

$results = New-Object System.Collections.Generic.List[object]
function Add-Result([string]$Name, [string]$Status, [string]$Detail) {
    $results.Add([pscustomobject]@{ Teste = $Name; Resultado = $Status; Detalhe = $Detail })
}

# --- Java: BackupStoreTest -------------------------------------------------------------
$backupOut = Join-Path $out 'backup'
$backup = Invoke-Native 'powershell.exe' @('-NoProfile','-ExecutionPolicy','Bypass','-File',
    (Join-Path $project 'tests/backup/run.ps1'),'-OutputDirectory',$backupOut)
Set-Content -LiteralPath (Join-Path $out 'backup.log') -Value $backup.Output -Encoding UTF8
$backupLine = ($backup.Output -split "`r?`n" | Where-Object { $_ -match '^(PASS|FAIL)' } | Select-Object -Last 1)
if ($backup.ExitCode -eq 0 -and $backupLine -match '^PASS') { Add-Result 'BackupStoreTest' 'PASSOU' $backupLine }
else { Add-Result 'BackupStoreTest' 'FALHOU' 'ver backup.log' }

# --- C++ ---------------------------------------------------------------------------------
$hostCompiler = $null
foreach ($candidate in @('g++','clang++')) {
    $found = Get-Command $candidate -ErrorAction SilentlyContinue
    if ($found) { $hostCompiler = $found.Source; break }
}
$ndkClang = Join-Path $paths.ndkDirectory 'toolchains/llvm/prebuilt/windows-x86_64/bin/clang++.exe'
$adb = Join-Path $paths.toolchainDirectory 'platform-tools/adb.exe'
$remoteDir = '/data/local/tmp/kinja-host-tests'
if ($Serial -and -not $hostCompiler) {
    $prep = Invoke-Native $adb @('-s',$Serial,'shell','mkdir','-p',$remoteDir)
    if ($prep.ExitCode -ne 0) { throw "ADB não alcançou o dispositivo ${Serial}: $($prep.Output)" }
}

$fixtureDir = Join-Path $out 'fixtures'
New-Item -ItemType Directory -Path $fixtureDir -Force | Out-Null
$accountTable = if ($paths.archiveRoot) {
    Join-Path $paths.archiveRoot 'analysis/bundles/tables/clientdata.ht/150393219823674535_AccountLevelCfgCfgData.json'
} else { '' }

foreach ($source in Get-ChildItem -LiteralPath $PSScriptRoot -Filter '*_test.cpp' | Sort-Object Name) {
    $name = $source.BaseName
    $includes = @()
    if ($name -eq 'account_progression_test') {
        if (-not ($accountTable -and (Test-Path -LiteralPath $accountTable))) {
            Add-Result $name 'IGNORADO' 'tabela AccountLevelCfg do acervo indisponível'
            continue
        }
        # Parentheses force PowerShell 5.1 to enumerate the JSON array before sorting.
        $rows = @((Get-Content -LiteralPath $accountTable -Raw | ConvertFrom-Json) | Sort-Object level)
        if ($rows.Count -ne 200) { Add-Result $name 'FALHOU' 'tabela AccountLevelCfg inesperada'; continue }
        $fixture = "#include <vector>`nconst std::vector<int> costs = {" + (($rows | ForEach-Object { $_.cost_exp }) -join ',') + '};'
        Set-Content -LiteralPath (Join-Path $fixtureDir 'account_cost_fixture.h') -Value $fixture -Encoding ASCII
        $includes = @("-I$fixtureDir")
    }
    $common = @('-std=c++17','-O1','-Wall','-Wextra') + $includes
    if ($hostCompiler) {
        $binary = Join-Path $out ($name + '.exe')
        $build = Invoke-Native $hostCompiler ($common + @($source.FullName,'-o',$binary))
        if ($build.ExitCode -ne 0) { Add-Result $name 'FALHOU' "compilação: $($build.Output)"; continue }
        $run = Invoke-Native $binary @()
        if ($run.ExitCode -eq 0) { Add-Result $name 'PASSOU' 'host' } else { Add-Result $name 'FALHOU' $run.Output }
        continue
    }
    if (-not (Test-Path -LiteralPath $ndkClang)) { Add-Result $name 'FALHOU' "clang do NDK ausente: $ndkClang"; continue }
    $binary = Join-Path $out $name
    $target = "--target=$AndroidAbi-linux-android23"
    $build = Invoke-Native $ndkClang (@($target) + $common + @('-static-libstdc++',$source.FullName,'-o',$binary))
    if ($build.ExitCode -ne 0) { Add-Result $name 'FALHOU' "compilação: $($build.Output)"; continue }
    if ($build.Output) { Set-Content -LiteralPath ($binary + '.compile.log') -Value $build.Output -Encoding UTF8 }
    if (-not $Serial) { Add-Result $name 'COMPILADO' "NDK $AndroidAbi; não executado (sem compilador de host nem -Serial)"; continue }
    $remote = "$remoteDir/$name"
    $push = Invoke-Native $adb @('-s',$Serial,'push',$binary,$remote)
    if ($push.ExitCode -ne 0) { Add-Result $name 'FALHOU' "adb push: $($push.Output)"; continue }
    $run = Invoke-Native $adb @('-s',$Serial,'shell',"chmod 755 $remote && $remote; echo EXIT=`$?")
    if ($run.Output -match 'EXIT=0\s*$') { Add-Result $name 'PASSOU' "dispositivo $Serial" }
    else { Add-Result $name 'FALHOU' $run.Output }
}

$results | Format-Table -AutoSize -Wrap | Out-String -Width 200 | Write-Output
$results | ConvertTo-Json | Set-Content -LiteralPath (Join-Path $out 'summary.json') -Encoding UTF8
Write-Output "Saída: $out"
if ($results | Where-Object { $_.Resultado -eq 'FALHOU' }) { exit 1 }
exit 0
