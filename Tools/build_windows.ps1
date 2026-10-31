# Run from a Windows checkout or the complete local handoff archive.
[CmdletBinding()]
param(
    [string]$EngineRoot = $env:UNREAL_ENGINE_ROOT,
    [ValidateSet('Development','Shipping')][string]$Configuration = 'Development'
)
$ErrorActionPreference = 'Stop'
if ($env:OS -ne 'Windows_NT') { throw 'Run this script on Windows with Unreal Engine 5.8.4 installed.' }
$ProjectRoot = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $ProjectRoot 'BadCatLab.uproject'
if (-not $EngineRoot) {
    $Registry = Get-ItemProperty 'HKLM:\SOFTWARE\EpicGames\Unreal Engine\5.8' -ErrorAction SilentlyContinue
    if ($Registry) { $EngineRoot = $Registry.InstalledDirectory }
}
if (-not $EngineRoot) { $EngineRoot = 'C:\Program Files\Epic Games\UE_5.8' }
$Uat = Join-Path $EngineRoot 'Engine\Build\BatchFiles\RunUAT.bat'
$Editor = Join-Path $EngineRoot 'Engine\Binaries\Win64\UnrealEditor-Cmd.exe'
if (-not (Test-Path -LiteralPath $Uat) -or -not (Test-Path -LiteralPath $Editor)) {
    throw 'Windows Unreal installation not found. Pass -EngineRoot with the full UE_5.8 folder.'
}
$Version = Get-Content -LiteralPath (Join-Path $EngineRoot 'Engine\Build\Build.version') -Raw | ConvertFrom-Json
if ($Version.MajorVersion -ne 5 -or $Version.MinorVersion -ne 8 -or $Version.PatchVersion -ne 4) {
    throw 'This project is locked to Unreal 5.8.4. Use that engine version before packaging.'
}
# Prevent a public-source-only clone producing a room with missing licensed art.
$Required = @(
    'Content/Maps/LivingRoom.umap',
    'Content/Characters/Ahuai/SK_Ahuai.uasset',
    'Content/Characters/Jutuan/SK_Jutuan.uasset',
    'Content/Characters/Wumei/SK_Wumei.uasset',
    'Content/Characters/Dafu/SK_Dafu.uasset',
    'Content/FabLocal/Furniture/SM_Sofa_41.uasset',
    'Content/FabLocal/Characters/Owner_Idle.uasset',
    'Content/FabLocal/Characters/Guest_Idle.uasset',
    'Content/CozyLocal/Crystal_Ball.uasset',
    'Content/CozyLocal/Pumpkin_Fireplace.uasset',
    'Content/Audio/Gameplay/CanCanGameplay.uasset',
    'Content/Audio/Pixabay/Menu.uasset',
    'Content/Audio/Pixabay/Chase.uasset',
    'Content/UI/Fonts/NotoSansCJKsc-Regular.otf',
    'Content/UI/Fonts/NotoSansCJKsc-Bold.otf',
    'Content/ResidentsLocal/Owner_Idle.uasset',
    'Content/ResidentsLocal/Owner_Idle_Anim.uasset',
    'Content/ResidentsLocal/Owner_Walk_Anim.uasset',
    'Content/ResidentsLocal/Owner_Run_Anim.uasset',
    'Content/ResidentsLocal/Owner_Confused_Anim.uasset',
    'Content/Art/Halloween/M_Sticker.uasset',
    'Content/Art/Halloween/T_StickerAtlas.uasset',
    'Content/Art/M_CameraFootprint.uasset',
    'Content/UI/Title/T_TitleMark.uasset'
)
$Missing = @($Required | Where-Object { -not (Test-Path -LiteralPath (Join-Path $ProjectRoot $_)) })
if ($Missing.Count -gt 0) { throw ('Full project assets are missing: ' + ($Missing -join ', ')) }
$Archive = Join-Path $ProjectRoot 'Builds'
New-Item -ItemType Directory -Path $Archive -Force | Out-Null
$Log = Join-Path $Archive 'windows-build.log'
$Arguments = @('BuildCookRun', ('-project="' + $Project + '"'), '-noP4', '-platform=Win64',
    ('-clientconfig=' + $Configuration), '-build', '-cook', '-stage', '-pak', '-package', '-archive',
    ('-archivedirectory="' + $Archive + '"'), '-prereqs', '-unattended', '-utf8output')
& $Uat @Arguments 2>&1 | Tee-Object -FilePath $Log
$BuildExit = $LASTEXITCODE
if ($BuildExit -ne 0) { throw "Unreal packaging failed (exit $BuildExit). See $Log" }
$Packaged = Join-Path $Archive 'Windows'
$Exe = Join-Path $Packaged 'BadCatLab.exe'
if (-not (Test-Path -LiteralPath $Exe)) { throw 'UAT exited without the expected Builds/Windows/BadCatLab.exe. Inspect the archive and log.' }
$Zip = Join-Path $Archive ('Delivery-Chaos-Windows-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.zip')
Compress-Archive -LiteralPath $Packaged -DestinationPath $Zip
(Get-FileHash -LiteralPath $Zip -Algorithm SHA256).Hash | Set-Content -LiteralPath ($Zip + '.sha256')
Write-Host "Packaged: $Exe"
Write-Host "Share the entire ZIP, not the EXE alone: $Zip"
Write-Host 'Packaging is complete; run the executable on a Windows PC to validate before marking it playable.'
