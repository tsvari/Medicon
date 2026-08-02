# fetch_sqlapi.ps1 — Restore the SQLAPI++ prebuilt tree (gitignored) on Windows.
#
# SQLAPI++ (~1.4 GB of prebuilt binaries) is intentionally NOT kept in git.
# This script restores the parts the MSVC build needs:
#   source/cpp/backend/source/3party/SQLAPI/windows/{include,vs2022,*.txt,*.html}
#
# Sources, in order of preference:
#   1. Local backup      : assets/SQLAPI_installers/sqlapi-backup/windows/
#   2. Local installer   : assets/SQLAPI_installers/sqlapi-5.3.5.exe  (silent install)
#   3. Manual download   : place the installer in assets/SQLAPI_installers/ and re-run
#
# Usage:  powershell -ExecutionPolicy Bypass -File scripts/fetch_sqlapi.ps1

$ErrorActionPreference = "Stop"
$RepoRoot = Split-Path -Parent $PSScriptRoot
$TargetBase = Join-Path $RepoRoot "source\cpp\backend\source\3party\SQLAPI\windows"
$Installers = Join-Path $RepoRoot "assets\SQLAPI_installers"

function Copy-IfExists([string]$Src, [string]$Dest) {
    if (Test-Path $Src) {
        Copy-Item -Path $Src -Destination $Dest -Recurse -Force
        Write-Host "  copied: $Src"
    }
}

Write-Host "SQLAPI++ restore for Windows (MSVC vs2022)"
Write-Host "Target: $TargetBase"

if (-not (Test-Path (Join-Path $TargetBase "vs2022\x86_64\sqlapis.lib"))) {
    # --- Source 1: local backup ---
    $Backup = Join-Path $Installers "sqlapi-backup\windows"
    if (Test-Path $Backup) {
        Write-Host "Using local backup at $Backup"
        New-Item -ItemType Directory -Path $TargetBase -Force | Out-Null
        Copy-IfExists (Join-Path $Backup "include")    (Join-Path $TargetBase "include")
        Copy-IfExists (Join-Path $Backup "vs2022")     (Join-Path $TargetBase "vs2022")
        Copy-IfExists (Join-Path $Backup "ReadMe.txt") $TargetBase
        Copy-IfExists (Join-Path $Backup "documentation.html") $TargetBase
        Copy-IfExists (Join-Path $Backup "license.txt") $TargetBase
    }
    else {
        # --- Source 2: silent install from the .exe (Inno Setup) ---
        $Exe = Join-Path $Installers "sqlapi-5.3.5.exe"
        if (Test-Path $Exe) {
            Write-Host "Extracting $Exe (silent install) ..."
            $Tmp = Join-Path $env:TEMP "sqlapi_install"
            if (Test-Path $Tmp) { Remove-Item $Tmp -Recurse -Force }
            Start-Process -FilePath $Exe -ArgumentList "/VERYSILENT", "/SUPPRESSMSGBOXES", "/NORESTART", "/DIR=`"$Tmp`"" -Wait
            $Installed = Get-ChildItem -Path $Tmp -Recurse -Filter "sqlapis.lib" -ErrorAction SilentlyContinue |
                         Select-Object -First 1
            if ($Installed) {
                $WinDir = Split-Path -Parent (Split-Path -Parent $Installed.FullName)  # .../vs2022
                New-Item -ItemType Directory -Path $TargetBase -Force | Out-Null
                Copy-IfExists (Join-Path $WinDir "include") (Join-Path $TargetBase "include")
                Copy-IfExists (Join-Path $WinDir "vs2022")  (Join-Path $TargetBase "vs2022")
            }
            Remove-Item $Tmp -Recurse -Force -ErrorAction SilentlyContinue
        }
    }
}
else {
    Write-Host "SQLAPI++ already present — nothing to do."
}

# --- Final check ---
if (Test-Path (Join-Path $TargetBase "vs2022\x86_64\sqlapis.lib") -and
    Test-Path (Join-Path $TargetBase "include")) {
    Write-Host "OK: SQLAPI++ headers + vs2022 libs ready." -ForegroundColor Green
}
else {
    Write-Host "SQLAPI++ NOT restored. Download sqlapi-5.3.5.exe from https://www.sqlapi.com/" -ForegroundColor Yellow
    Write-Host "place it in assets/SQLAPI_installers/ and re-run this script." -ForegroundColor Yellow
    exit 1
}
