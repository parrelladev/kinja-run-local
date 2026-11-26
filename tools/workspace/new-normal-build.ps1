<#
.SYNOPSIS
Gera uma build normal (perfil do usuário) com todas as conferências do processo.

.DESCRIPTION
Etapas, na ordem; qualquer falha interrompe:
 1. Git: branch main e árvore limpa (o script não faz pull; rode `git pull` antes).
 2. Configuração: APK original com o hash 1.9.1, NDK, toolchain, keystore e senha.
 3. Package: o da build normal mais recente em build-history.json (não QA).
    Outro package exige -AllowPackageChange.
 4. BuildId: novo no histórico, em build/ e no acervo externo.
 5. Testes de host (tests/run-host-tests.ps1). "COMPILADO" é aceito e registrado.
 -- com -DryRun, para aqui --
 6. Compilação ARM64 sem opções de QA (build-native.ps1).
 7. Empacotamento e assinatura (offline/build.ps1), seeds normais, sem fixtures.
 8. Conferências: certificado igual ao de laboratório, .so do APK igual à compilada,
    seeds do APK iguais a offline/assets.
 9. Acrescenta a entrada em build-history.json e grava um resumo JSON ao lado do APK.

Não instala, não usa ADB (exceto se -TestSerial for passado para os testes),
não faz commit nem push. Depois: documentar no guia e commitar.

.EXAMPLE
./tools/workspace/new-normal-build.ps1 -BuildId v152-descricao -DryRun
./tools/workspace/new-normal-build.ps1 -BuildId v152-descricao
#>
param(
    [Parameter(Mandatory=$true)][ValidatePattern('^v[0-9]+-[a-z0-9][a-z0-9.-]*$')][string]$BuildId,
    [string]$PackageName,
    [switch]$AllowPackageChange,
    [string]$TestSerial,
    [switch]$DryRun
)
$ErrorActionPreference = 'Stop'
$project = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
. (Join-Path $PSScriptRoot 'paths.ps1')
. (Join-Path $PSScriptRoot 'build-history.ps1')
$paths = Get-KinjaWorkspacePaths -ProjectRoot $project
$expectedOriginalSha = '7953EB369BA9B8C814C4736DAC4A7E3AA116A5583837F6F12D795EA70C20595E'
# Public certificate digest of the existing lab keystore; every normal build must match it.
$expectedCertSha = '7508ca8dd3ad4914f42b7d6dbd537c564a78925fb8f41b76a905324220a75b3f'

function Step([string]$Text) { Write-Host "`n== $Text" -ForegroundColor Cyan }
function Ok([string]$Text) { Write-Host "   ok: $Text" -ForegroundColor Green }

Push-Location $project
try {
    Step 'Git'
    $branch = (git rev-parse --abbrev-ref HEAD).Trim()
    if ($branch -ne 'main') { throw "A build normal sai da main; branch atual: $branch" }
    $dirty = git status --porcelain
    if ($dirty) { throw "Há alterações não commitadas:`n$($dirty -join "`n")" }
    $head = (git log -1 --format='%h %s').Trim()
    Ok "main limpa em $head"

    Step 'Configuração'
    foreach ($key in @('originalApk','keystore','storePasswordFile')) {
        if (-not (Test-Path -LiteralPath $paths.$key -PathType Leaf)) { throw "config/local.json: '$key' não encontrado em $($paths.$key)" }
    }
    foreach ($key in @('ndkDirectory','toolchainDirectory')) {
        if (-not (Test-Path -LiteralPath $paths.$key -PathType Container)) { throw "config/local.json: '$key' não encontrado em $($paths.$key)" }
    }
    $originalSha = (Get-FileHash -LiteralPath $paths.originalApk -Algorithm SHA256).Hash
    if ($originalSha -ne $expectedOriginalSha) { throw "APK original com hash inesperado: $originalSha" }
    Ok 'APK original 1.9.1, NDK, toolchain e keystore presentes'

    Step 'Package'
    $history = Read-KinjaBuildHistory -ProjectRoot $project
    $lastNormal = $history | Where-Object { $_.package -notmatch '\.qa(\.|$)' } | Select-Object -Last 1
    if (-not $lastNormal) { throw 'Nenhuma build normal no histórico; informe -PackageName e -AllowPackageChange' }
    if (-not $PackageName) { $PackageName = $lastNormal.package }
    if ($PackageName -match '\.qa(\.|$)') { throw "Package QA não é build normal: $PackageName" }
    if ($PackageName -ne $lastNormal.package -and -not $AllowPackageChange) {
        throw "A última build normal ($($lastNormal.buildId)) usou $($lastNormal.package). Trocar de package perde o save do usuário; use -AllowPackageChange só com autorização."
    }
    Ok "$PackageName (última normal: $($lastNormal.buildId))"

    Step 'BuildId'
    if ($history | Where-Object { $_.buildId -eq $BuildId }) { throw "BuildId já registrado no histórico: $BuildId" }
    $apk = Join-Path $project "build/kinja-run-offline-lab-$BuildId.apk"
    $roots = @(Join-Path $project 'build')
    if ($paths.archiveRoot) { $roots += (Join-Path $paths.archiveRoot 'build') }
    foreach ($root in $roots) {
        foreach ($name in @("offline-bridge-$BuildId","kinja-run-offline-lab-$BuildId.apk","kinja-run-offline-lab-$BuildId.unsigned.apk")) {
            if (Test-Path -LiteralPath (Join-Path $root $name)) { throw "BuildId já usado em ${root}: $name" }
        }
    }
    Ok "$BuildId é novo"

    Step 'Testes de host'
    $testArgs = @('-NoProfile','-ExecutionPolicy','Bypass','-File',(Join-Path $project 'tests/run-host-tests.ps1'))
    if ($TestSerial) { $testArgs += @('-Serial',$TestSerial) }
    & powershell.exe @testArgs
    if ($LASTEXITCODE -ne 0) { throw 'Algum teste de host falhou; veja a tabela acima' }
    Ok 'nenhuma falha (testes só COMPILADOS não foram executados)'

    if ($DryRun) { Write-Host "`n-DryRun: conferências concluídas; nada foi compilado nem empacotado." -ForegroundColor Yellow; return }

    Step 'Compilação ARM64 (sem QA)'
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $project 'offline/native/build-native.ps1')
    if ($LASTEXITCODE -ne 0) { throw 'Compilação nativa falhou' }
    $nativeLibrary = Join-Path $project 'offline/build/arm64-v8a/libkinjaoffline.so'
    $nativeSha = (Get-FileHash -LiteralPath $nativeLibrary -Algorithm SHA256).Hash
    Ok "libkinjaoffline.so $nativeSha"

    Step 'Empacotamento e assinatura'
    & powershell.exe -NoProfile -ExecutionPolicy Bypass -File (Join-Path $project 'offline/build.ps1') `
        -BuildId $BuildId -PackageName $PackageName -NativeLibrary $nativeLibrary -PackageApk
    if ($LASTEXITCODE -ne 0 -or -not (Test-Path -LiteralPath $apk)) { throw 'Empacotamento falhou' }
    $apkSha = (Get-FileHash -LiteralPath $apk -Algorithm SHA256).Hash
    Ok "APK $apkSha"

    Step 'Conferências do APK'
    $report = Get-Content -LiteralPath ($apk + '.signature-verification.txt') -Raw
    $certs = [regex]::Matches($report, 'certificate SHA-256 digest: ([0-9a-f]{64})') | ForEach-Object { $_.Groups[1].Value } | Select-Object -Unique
    if (@($certs).Count -ne 1 -or $certs -ne $expectedCertSha) { throw "Certificado inesperado: $($certs -join ', ')" }
    Ok 'certificado de laboratório esperado'
    Add-Type -AssemblyName System.IO.Compression.FileSystem
    $zip = [IO.Compression.ZipFile]::OpenRead($apk)
    try {
        function Get-EntrySha($entry) {
            $stream = $entry.Open()
            try { ([BitConverter]::ToString([Security.Cryptography.SHA256]::Create().ComputeHash($stream))).Replace('-','') }
            finally { $stream.Dispose() }
        }
        $soEntry = $zip.GetEntry('lib/arm64-v8a/libkinjaoffline.so')
        if (-not $soEntry -or (Get-EntrySha $soEntry) -ne $nativeSha) { throw 'A .so dentro do APK difere da compilada' }
        Ok '.so do APK igual à compilada'
        $seedEntries = @($zip.Entries | Where-Object { $_.FullName -like 'assets/offline/seed_*.json' })
        $sources = @(Get-ChildItem -LiteralPath (Join-Path $project 'offline/assets') -Filter 'seed_*.json')
        if ($seedEntries.Count -ne $sources.Count) { throw "Quantidade de seeds no APK ($($seedEntries.Count)) difere de offline/assets ($($sources.Count))" }
        foreach ($source in $sources) {
            $entry = $zip.GetEntry('assets/offline/' + $source.Name)
            if (-not $entry -or (Get-EntrySha $entry) -ne (Get-FileHash -LiteralPath $source.FullName -Algorithm SHA256).Hash) {
                throw "Seed do APK difere de offline/assets: $($source.Name)"
            }
        }
        Ok "$($sources.Count) seeds iguais a offline/assets"
    } finally { $zip.Dispose() }

    Step 'Histórico'
    $entry = [pscustomobject]@{ buildId = $BuildId; package = $PackageName; apkSha256 = $apkSha; nativeSha256 = $nativeSha.ToLowerInvariant() }
    Write-KinjaBuildHistory -ProjectRoot $project -Entries (@($history) + $entry)
    $summary = [ordered]@{
        buildId = $BuildId; package = $PackageName; commit = $head; apk = $apk
        apkSha256 = $apkSha; nativeSha256 = $nativeSha; certificateSha256 = $expectedCertSha
        installed = $false; note = 'Compilada e assinada; não instalada nem testada no jogo.'
    }
    $summary | ConvertTo-Json | Set-Content -LiteralPath ($apk + '.release-summary.json') -Encoding UTF8
    Ok 'docs/development/build-history.json atualizado'

    Write-Host "`nAPK: $apk" -ForegroundColor Green
    Write-Host 'Próximo: atualizar a seção da build normal no MAINTENANCE_GUIDE.md e commitar (sem push).'
} finally { Pop-Location }
