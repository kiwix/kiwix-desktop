param(
    [Parameter(Mandatory = $true)]
    [string]$PackageDir
)

$ErrorActionPreference = "Stop"
$packageDir = (Resolve-Path $PackageDir).Path
$launcher = Join-Path $packageDir "kiwix.exe"
$internalDir = Join-Path $packageDir "_internal"
$app = Join-Path $internalDir "kiwix-desktop.exe"
$readme = Join-Path $packageDir "README.txt"

foreach ($path in @($launcher, $internalDir, $app, $readme)) {
    if (-not (Test-Path -LiteralPath $path)) {
        throw "Missing package item: $path"
    }
}

if (Test-Path -LiteralPath (Join-Path $packageDir "Qt6Core.dll")) {
    throw "A runtime DLL was left beside kiwix.exe"
}
if (Test-Path -LiteralPath (Join-Path $packageDir "platforms")) {
    throw "A Qt plugin directory was left beside kiwix.exe"
}

Write-Output "Windows package structure is valid"

$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
$testSource = Join-Path $scriptDir "windows-launcher-test.cpp"
$cl = Get-Command "cl.exe" -ErrorAction Stop
$fakeApp = Join-Path $internalDir "kiwix-desktop.exe"
$outputFile = Join-Path $env:TEMP "kiwix-launcher-test-output.txt"
$backupApp = Join-Path $env:TEMP "kiwix-desktop-original-$PID.exe"
if (Test-Path -LiteralPath $outputFile) {
    Remove-Item -LiteralPath $outputFile -Force
}
if (Test-Path -LiteralPath $backupApp) {
    Remove-Item -LiteralPath $backupApp -Force
}
Copy-Item -LiteralPath $fakeApp -Destination $backupApp -Force

$oldOutput = $env:KIWIX_LAUNCHER_TEST_OUTPUT
try {
    & $cl.Source "/nologo" "/O2" "/DUNICODE" "/D_UNICODE" "/EHsc" "/W3" `
        "/Fe$fakeApp" $testSource "/link" "/SUBSYSTEM:WINDOWS"
    if ($LASTEXITCODE -ne 0) {
        throw "Launcher test application compilation failed with exit code $LASTEXITCODE"
    }

    $env:KIWIX_LAUNCHER_TEST_OUTPUT = $outputFile
    $argumentList = @(
        "--single-instance-test",
        "C:\Kiwix test files\wikipedia portable.zim",
        "second `".zim`" argument"
    )
    $process = Start-Process -FilePath $launcher -ArgumentList $argumentList `
        -Wait -PassThru -NoNewWindow
    if ($process.ExitCode -ne 0) {
        throw "Launcher returned exit code $($process.ExitCode)"
    }
}
finally {
    $env:KIWIX_LAUNCHER_TEST_OUTPUT = $oldOutput
    if (Test-Path -LiteralPath $backupApp) {
        Copy-Item -LiteralPath $backupApp -Destination $fakeApp -Force
        Remove-Item -LiteralPath $backupApp -Force
    }
}

if (-not (Test-Path -LiteralPath $outputFile)) {
    throw "Launcher did not start the application in _internal"
}

$forwardedCommandLine = [System.IO.File]::ReadAllText(
    $outputFile,
    [System.Text.Encoding]::Unicode
)
foreach ($expected in @(
    "--single-instance-test",
    "C:\Kiwix test files\wikipedia portable.zim",
    "second",
    ".zim",
    "argument"
)) {
    if (-not $forwardedCommandLine.Contains($expected)) {
        throw "Launcher did not forward expected argument text: $expected"
    }
}

Write-Output "Launcher forwarded arguments and started _internal\kiwix-desktop.exe"
