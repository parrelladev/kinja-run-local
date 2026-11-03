$ErrorActionPreference = 'Stop'
& python (Join-Path $PSScriptRoot 'build_inventory.py')
if ($LASTEXITCODE -ne 0) { throw 'Atlas generation failed' }
Start-Process -FilePath (Join-Path $PSScriptRoot 'index.html')
