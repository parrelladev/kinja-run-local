param([string]$OutputDirectory)
$ErrorActionPreference = 'Stop'
$kinjaProject = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '../..'))
. (Join-Path $kinjaProject 'tools/workspace/paths.ps1')
$kinjaToolchain = (Get-KinjaWorkspacePaths -ProjectRoot $kinjaProject).toolchainDirectory
$kinjaToolPaths = Get-Content -LiteralPath (Join-Path $kinjaToolchain 'paths.json') -Raw | ConvertFrom-Json
$kinjaJava = Join-Path $kinjaToolchain $kinjaToolPaths.java
$kinjaJavac = Join-Path (Split-Path -Parent $kinjaJava) 'javac.exe'
if (-not $OutputDirectory) { $OutputDirectory = Join-Path $kinjaProject ('build/tests/manual-backup-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')) }
$kinjaOutput = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $kinjaOutput) { throw 'Test output must be a new directory' }
New-Item -ItemType Directory -Path $kinjaOutput -Force | Out-Null
$kinjaClasses = Join-Path $kinjaOutput 'classes'
& $kinjaJavac --release 8 -encoding UTF-8 -d $kinjaClasses (Join-Path $kinjaProject 'offline/java/local/kinja/BackupStore.java') (Join-Path $kinjaProject 'offline/java/local/kinja/BackupCatalog.java') (Join-Path $PSScriptRoot 'BackupStoreTest.java')
if ($LASTEXITCODE -ne 0) { throw 'Backup host test compilation failed' }
& $kinjaJava -cp $kinjaClasses local.kinja.BackupStoreTest (Join-Path $kinjaProject 'offline/assets') (Join-Path $kinjaOutput 'fixtures') | Tee-Object -FilePath (Join-Path $kinjaOutput 'result.txt')
if ($LASTEXITCODE -ne 0) { throw 'Backup host regressions failed' }
