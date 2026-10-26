param(
    [string]$CliPath = 'arduino-cli',
    [string]$ConfigFile = '',
    [ValidateRange(1,32)][int]$Jobs = 4
)
$ErrorActionPreference = 'Stop'
$taskRoot = $PSScriptRoot
$taskSketch = Join-Path $taskRoot 'boards/21b/bringup'
$taskBuild = Join-Path $taskRoot '.build/21b-bringup'
$taskOutput = Join-Path $taskBuild 'output'
if (!(Test-Path -LiteralPath (Join-Path $taskSketch 'bringup.ino'))) {
    throw "Bring-up sketch not found: $taskSketch"
}
New-Item -ItemType Directory -Force $taskBuild, $taskOutput | Out-Null

$taskArgs = @()
if ($ConfigFile) { $taskArgs += @('--config-file', $ConfigFile) }
$taskArgs += @(
    'compile',
    '--jobs', "$Jobs",
    '--libraries', (Join-Path $taskRoot 'libraries'),
    '--build-path', (Join-Path $taskBuild 'build'),
    '--fqbn', 'esp32:esp32:esp32s3:FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,CDCOnBoot=cdc',
    '--output-dir', $taskOutput,
    $taskSketch
)
Write-Host "Compiling isolated ESP32-S3-Touch-LCD-2.1B bring-up sketch..."
Write-Host "No MotoNav firmware image will be packaged by this script."
& $CliPath @taskArgs
if ($LASTEXITCODE -ne 0) {
    throw "Arduino CLI compile failed ($LASTEXITCODE). Check the first compiler error above."
}
Write-Host "Compile succeeded. Output files:"
Get-ChildItem -LiteralPath $taskOutput -File | Select-Object Name, Length | Format-Table -AutoSize
Write-Host "This confirms compilation only; physical display/touch validation is still required."
