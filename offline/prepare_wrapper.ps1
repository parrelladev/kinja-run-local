param([Parameter(Mandatory=$true)][string]$OutputDirectory)

$ErrorActionPreference = 'Stop'
$kinjaProject = [IO.Path]::GetFullPath((Join-Path $PSScriptRoot '..'))
. (Join-Path $kinjaProject 'tools/workspace/paths.ps1')
$kinjaToolchain = (Get-KinjaWorkspacePaths -ProjectRoot $kinjaProject).toolchainDirectory
$kinjaToolPaths = Get-Content -LiteralPath (Join-Path $kinjaToolchain 'paths.json') -Raw | ConvertFrom-Json
$kinjaJavaRoot = Split-Path -Parent (Split-Path -Parent (Join-Path $kinjaToolchain $kinjaToolPaths.java))
$kinjaJava = Join-Path $kinjaJavaRoot 'bin\java.exe'
$kinjaJavac = Join-Path $kinjaJavaRoot 'bin\javac.exe'
$kinjaJar = Join-Path $kinjaJavaRoot 'bin\jar.exe'
$kinjaAndroidJar = Join-Path $kinjaToolchain 'android-api33/android.jar'
$kinjaD8 = Join-Path $kinjaToolchain 'android-build-tools/android-37.0/lib/d8.jar'
$kinjaOut = [IO.Path]::GetFullPath($OutputDirectory)
if (Test-Path -LiteralPath $kinjaOut) { throw 'Wrapper output directory must be new' }
foreach ($kinjaRequired in @($kinjaJava,$kinjaJavac,$kinjaJar,$kinjaAndroidJar,$kinjaD8)) {
    if (-not (Test-Path -LiteralPath $kinjaRequired)) { throw ('Missing compiler dependency: ' + $kinjaRequired) }
}
$kinjaStubClasses = Join-Path $kinjaOut 'compile-only-classes'
$kinjaProgramClasses = Join-Path $kinjaOut 'program-classes'
$kinjaDexDir = Join-Path $kinjaOut 'dex'
foreach ($kinjaDir in @($kinjaStubClasses,$kinjaProgramClasses,$kinjaDexDir)) {
    [IO.Directory]::CreateDirectory($kinjaDir) | Out-Null
}
& $kinjaJavac --release 8 -classpath $kinjaAndroidJar -d $kinjaStubClasses (Join-Path $PSScriptRoot 'compile-only\com\unity3d\player\UnityPlayerActivity.java')
if ($LASTEXITCODE -ne 0) { throw 'Compile-only Unity signature compilation failed' }
$kinjaClasspath = $kinjaAndroidJar + [IO.Path]::PathSeparator + $kinjaStubClasses
$kinjaSources = @(Get-ChildItem -LiteralPath (Join-Path $PSScriptRoot 'java\local\kinja') -Filter '*.java' -File | ForEach-Object { $_.FullName })
& $kinjaJavac --release 8 -encoding UTF-8 -g -classpath $kinjaClasspath -d $kinjaProgramClasses @kinjaSources
if ($LASTEXITCODE -ne 0) { throw 'Launcher compilation failed' }
$kinjaStubJar = Join-Path $kinjaOut 'compile-only-stubs.jar'
& $kinjaJar --create --file $kinjaStubJar -C $kinjaStubClasses 'com/unity3d/player/UnityPlayerActivity.class'
if ($LASTEXITCODE -ne 0) { throw 'Compile-only classpath packaging failed' }
# Only real wrapper classes are D8 inputs. Unity stubs remain classpath-only.
$kinjaClasses = @(Get-ChildItem -LiteralPath $kinjaProgramClasses -Recurse -Filter '*.class' -File | ForEach-Object { $_.FullName })
& $kinjaJava -cp $kinjaD8 com.android.tools.r8.D8 --release --min-api 23 --lib $kinjaAndroidJar --classpath $kinjaStubJar --output $kinjaDexDir @kinjaClasses
if ($LASTEXITCODE -ne 0) { throw 'Launcher DEX compilation failed' }
& python (Join-Path $PSScriptRoot 'validate_wrapper.py') --dex (Join-Path $kinjaDexDir 'classes.dex') --report (Join-Path $kinjaOut 'wrapper-report.json')
if ($LASTEXITCODE -ne 0) { throw 'Launcher DEX contract validation failed' }
Write-Output ('Prepared launcher DEX: ' + (Join-Path $kinjaDexDir 'classes.dex'))
