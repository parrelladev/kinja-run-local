# Paths only: never reads keystore/password contents or starts external processes.
function Get-KinjaWorkspacePaths {
    param([Parameter(Mandatory=$true)][string]$ProjectRoot)
    $kinjaRoot = [IO.Path]::GetFullPath($ProjectRoot)
    $kinjaConfigFile = Join-Path $kinjaRoot 'config/local.json'
    $kinjaConfig = if (Test-Path -LiteralPath $kinjaConfigFile) { Get-Content -LiteralPath $kinjaConfigFile -Raw | ConvertFrom-Json } else { [pscustomobject]@{} }
    $kinjaDefaults = [ordered]@{
        archiveRoot = ''
        originalApk = 'kinja-run-1-9-1.apk'
        ndkDirectory = 'tools/android/native-toolchain/android-ndk-r27d'
        toolchainDirectory = 'tools/android/toolchain'
        keystore = 'build/offline-lab.jks'
        storePasswordFile = 'build/offline-lab.keystore-password.txt'
        methodMap = 'analysis/native/game-method-map.json'
    }
    $kinjaResolved = [ordered]@{}
    foreach ($kinjaKey in $kinjaDefaults.Keys) {
        $kinjaValue = if ($kinjaConfig.PSObject.Properties.Name -contains $kinjaKey) { [string]$kinjaConfig.$kinjaKey } else { $kinjaDefaults[$kinjaKey] }
        if ($kinjaValue) {
            $kinjaValue = [Environment]::ExpandEnvironmentVariables($kinjaValue)
            if (-not [IO.Path]::IsPathRooted($kinjaValue)) { $kinjaValue = Join-Path $kinjaRoot $kinjaValue }
            $kinjaValue = [IO.Path]::GetFullPath($kinjaValue)
        }
        $kinjaResolved[$kinjaKey] = $kinjaValue
    }
    [pscustomobject]$kinjaResolved
}
