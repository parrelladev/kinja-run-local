# Read/write helpers for docs/development/build-history.json.
# Works on Windows PowerShell 5.1 and PowerShell 7: forces JSON array enumeration
# and writes a stable 2-space, LF, UTF-8 (no BOM) layout so diffs stay small.

function Get-KinjaBuildHistoryPath {
    param([Parameter(Mandatory=$true)][string]$ProjectRoot)
    Join-Path ([IO.Path]::GetFullPath($ProjectRoot)) 'docs/development/build-history.json'
}

function Read-KinjaBuildHistory {
    param([Parameter(Mandatory=$true)][string]$ProjectRoot)
    $path = Get-KinjaBuildHistoryPath -ProjectRoot $ProjectRoot
    if (-not (Test-Path -LiteralPath $path)) { return @() }
    # Parentheses make PowerShell 5.1 enumerate the array instead of passing it as one object.
    $entries = @((Get-Content -LiteralPath $path -Raw -Encoding UTF8 | ConvertFrom-Json))
    foreach ($entry in $entries) {
        if (-not $entry.buildId -or -not $entry.package) { throw "Invalid build-history entry in $path" }
    }
    return ,$entries
}

function ConvertTo-KinjaJsonString([string]$Value) {
    if ($null -eq $Value) { return 'null' }
    '"' + ($Value -replace '\\','\\' -replace '"','\"') + '"'
}

function Write-KinjaBuildHistory {
    param(
        [Parameter(Mandatory=$true)][string]$ProjectRoot,
        [Parameter(Mandatory=$true)][AllowEmptyCollection()][object[]]$Entries
    )
    $path = Get-KinjaBuildHistoryPath -ProjectRoot $ProjectRoot
    $blocks = foreach ($entry in $Entries) {
        "  {`n" +
        '    "buildId": ' + (ConvertTo-KinjaJsonString $entry.buildId) + ",`n" +
        '    "package": ' + (ConvertTo-KinjaJsonString $entry.package) + ",`n" +
        '    "apkSha256": ' + (ConvertTo-KinjaJsonString $entry.apkSha256) + ",`n" +
        '    "nativeSha256": ' + (ConvertTo-KinjaJsonString $entry.nativeSha256) + "`n" +
        '  }'
    }
    $text = "[`n" + ($blocks -join ",`n") + "`n]`n"
    $null = $text | ConvertFrom-Json  # refuse to write malformed JSON
    $temp = $path + '.tmp'
    [IO.File]::WriteAllText($temp, $text, (New-Object System.Text.UTF8Encoding($false)))
    Move-Item -LiteralPath $temp -Destination $path -Force
}
