# Solid-Mist Secure Installation and Configuration Script
$repoUrl = "https://github.com/Acercandr0/Solid-Mist/archive/refs/heads/main.zip"
$windhawkPath = "$env:AppData\Windhawk\Engine\Profiles"
$backupPath = "$env:AppData\Windhawk\Engine\Profiles_Backup_$(Get-Date -Format 'yyyyMMdd_HHmmss')"
$tempZip = "$env:TEMP\SolidMist.zip"
$tempExtract = "$env:TEMP\SolidMist-Extract"

# 1. Check for Administrator privileges
if (-not ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)) {
    Write-Warning "Please run this script as an Administrator."
    exit
}

# 2. Verify and Install Windhawk
if (-not (Test-Path "$env:ProgramFiles\Windhawk\Windhawk.exe")) {
    Write-Host "[-] Windhawk not found. Installing via WinGet..." -ForegroundColor Cyan
    winget install --id RamenSoftware.Windhawk --source winget --accept-package-agreements --accept-source-agreements
} else {
    Write-Host "[+] Windhawk is already installed." -ForegroundColor Green
}

# 3. Close Windhawk instances to unlock files
Write-Host "[*] Closing Windhawk instances..." -ForegroundColor Yellow
Stop-Process -Name "Windhawk" -Force -ErrorAction SilentlyContinue
Start-Sleep -Seconds 2

# 4. Backup existing profiles to avoid data loss
if (Test-Path $windhawkPath) {
    Write-Host "[*] Creating a backup of your current profiles..." -ForegroundColor Gray
    Copy-Item -Path $windhawkPath -Destination $backupPath -Recurse -Force
    Write-Host "[+] Backup created at: $backupPath" -ForegroundColor Green
} else {
    New-Item -ItemType Directory -Force -Path $windhawkPath | Out-Null
}

# 5. Download and Extract YAML files from GitHub
Write-Host "[*] Downloading Solid-Mist configurations..." -ForegroundColor Yellow
Invoke-WebRequest -Uri $repoUrl -OutFile $tempZip
Expand-Archive -Path $tempZip -DestinationPath $tempExtract -Force

# 6. Copy YAML files (Only adds or updates, doesn't delete other mods)
$yamlFiles = Get-ChildItem -Path "$tempExtract\Solid-Mist-main" -Filter *.yaml -Recurse

foreach ($file in $yamlFiles) {
    Copy-Item -Path $file.FullName -Destination $windhawkPath -Force
    Write-Host "[+] Installed/Updated: $($file.Name)" -ForegroundColor Gray
}

# 7. Cleanup temporary files
Remove-Item $tempZip -Force
Remove-Item $tempExtract -Recurse -Force

# 8. Restart Windhawk
Write-Host "[*] Restarting Windhawk..." -ForegroundColor Cyan
Start-Process "$env:ProgramFiles\Windhawk\Windhawk.exe"

Write-Host "`n[!] Setup completed successfully. Your previous settings are safe in the backup folder." -ForegroundColor Green