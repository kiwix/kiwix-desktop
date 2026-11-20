param(
    [Parameter(Mandatory = $true)]
    [string]$AppExe,

    [Parameter(Mandatory = $true)]
    [string]$OutDir,

    [string]$LauncherSource = "",
    [string]$ArchiveName = "Kiwix.zip"
)

$ErrorActionPreference = "Stop"
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$projectDir = Split-Path -Parent $scriptDir

if ([string]::IsNullOrWhiteSpace($LauncherSource)) {
    $LauncherSource = Join-Path $scriptDir "windows-launcher.cpp"
}

$appExe = (Resolve-Path $AppExe).Path
$windeployqt = Get-Command "windeployqt.exe" -ErrorAction Stop
$cl = Get-Command "cl.exe" -ErrorAction Stop

$null = New-Item -ItemType Directory -Force -Path $OutDir
$outDir = (Resolve-Path $OutDir).Path
$packageDir = Join-Path $outDir "Kiwix"
$internalDir = Join-Path $packageDir "_internal"

if (Test-Path $packageDir) {
    Remove-Item -LiteralPath $packageDir -Recurse -Force
}
New-Item -ItemType Directory -Force -Path $internalDir | Out-Null

Copy-Item -LiteralPath $appExe -Destination (Join-Path $internalDir "kiwix-desktop.exe")

$appDirectory = Split-Path -Parent $appExe
Get-ChildItem -LiteralPath $appDirectory -Filter "*.dll" -File -ErrorAction SilentlyContinue |
    Copy-Item -Destination $internalDir -Force

& $windeployqt.Source --release --compiler-runtime --dir $internalDir $appExe
if ($LASTEXITCODE -ne 0) {
    throw "windeployqt failed with exit code $LASTEXITCODE"
}

$launcherExe = Join-Path $packageDir "kiwix.exe"
$launcherObjects = Join-Path $outDir "windows-launcher.obj"
& $cl.Source "/nologo" "/O2" "/DUNICODE" "/D_UNICODE" "/EHsc" "/W3" `
    "/Fo$launcherObjects" "/Fe$launcherExe" $LauncherSource `
    "/link" "/SUBSYSTEM:WINDOWS"
if ($LASTEXITCODE -ne 0) {
    throw "Launcher compilation failed with exit code $LASTEXITCODE"
}

$readme = @"
Kiwix Desktop

Double-click kiwix.exe to start Kiwix.

Application files are kept in the _internal directory.
"@
Set-Content -LiteralPath (Join-Path $packageDir "README.txt") -Value $readme -Encoding ASCII

$archivePath = Join-Path $outDir $ArchiveName
if (Test-Path $archivePath) {
    Remove-Item -LiteralPath $archivePath -Force
}
Compress-Archive -LiteralPath $packageDir -DestinationPath $archivePath -CompressionLevel Optimal

Write-Output "Packaged application: $packageDir"
Write-Output "Created archive: $archivePath"
