param([string]$Archive = '')
Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if (!$Archive) { $Archive = Join-Path $root 'dist/SpaceAdventure-local-win64.zip' }
$Archive = (Resolve-Path -LiteralPath $Archive).Path
$smoke = Join-Path $root 'smoke.exe'
if (!(Test-Path -LiteralPath $smoke -PathType Leaf)) { throw 'Build smoke.exe first: mingw32-make smoke' }

# Проверены пути с пробелами и кириллицей, без зависимости от текущей папки.
$temp = Join-Path ([IO.Path]::GetTempPath()) ('Проверка игры ' + [Guid]::NewGuid().ToString())
$oldPath = $env:PATH
$process = $null
try {
    Expand-Archive -LiteralPath $Archive -DestinationPath $temp
    $packages = @(Get-ChildItem -LiteralPath $temp -Directory)
    if ($packages.Count -ne 1) { throw 'Expected one top-level package directory' }
    $package = $packages[0].FullName
    $lines = Get-Content -LiteralPath (Join-Path $package 'SHA256SUMS.txt')
    $checked = @{}
    foreach ($line in $lines) {
        if ($line -notmatch '^([a-f0-9]{64})  (.+)$') { throw "Invalid checksum line: $line" }
        $expected = $Matches[1]
        $relative = $Matches[2]
        $file = [IO.Path]::GetFullPath((Join-Path $package $relative))
        if (!$file.StartsWith($package + '\', [StringComparison]::OrdinalIgnoreCase)) {
            throw "Invalid checksum path: $relative"
        }
        if ($checked.ContainsKey($relative)) { throw "Duplicate checksum: $relative" }
        $actual = (Get-FileHash -LiteralPath $file -Algorithm SHA256).Hash.ToLowerInvariant()
        if ($actual -ne $expected) { throw "Checksum mismatch: $relative" }
        $checked[$relative] = $true
    }
    $count = @(Get-ChildItem -LiteralPath $package -Recurse -File).Count
    if ($count -ne $checked.Count + 1) { throw 'Package contains files missing from SHA256SUMS.txt' }
    $backups = @(Get-ChildItem -LiteralPath (Join-Path $package 'assets') -File |
        Where-Object { $_.Name -match '1\.(png|jpg)$' -or $_.Name -eq 'space.jpg' })
    if ($backups.Count -ne 0) { throw 'Obsolete texture backups must not be packaged' }
    if (!(Test-Path -LiteralPath (Join-Path $package 'assets/space.png') -PathType Leaf)) {
        throw 'Missing new PNG background'
    }
    $audio = @(Get-ChildItem -LiteralPath (Join-Path $package 'assets') -File |
        Where-Object { $_.Extension -in @('.ogg', '.wav') } |
        Select-Object -ExpandProperty Name | Sort-Object)
    $expectedAudio = @(
        'pixelated-adventures_92797.ogg', 'retro-dreamscape_92772.ogg', 'retro-waves-of-love_92802.ogg',
        'hitHurt.ogg', 'pickupCoin.ogg', 'powerUp.ogg', 'explosion.ogg'
    )
    if (@(Compare-Object -ReferenceObject $expectedAudio -DifferenceObject $audio).Count -ne 0) {
        throw 'Package must contain only the three new music tracks and four sfxr effects'
    }
    # Проверены наличие и неизменность каждого уведомления из рабочей копии.
    foreach ($notice in (Get-ChildItem -LiteralPath (Join-Path $root 'licenses') -File)) {
        $relative = 'licenses/' + $notice.Name
        if (!$checked.ContainsKey($relative)) { throw "Missing license notice: $relative" }
        $packagedNotice = Join-Path $package $relative
        if ((Get-FileHash -LiteralPath $notice.FullName -Algorithm SHA256).Hash -ne
            (Get-FileHash -LiteralPath $packagedNotice -Algorithm SHA256).Hash) {
            throw "License notice differs from source: $relative"
        }
    }
    foreach ($license in @('LICENSE', 'assets/LICENSE.md', 'assets/PT_Sans-LICENSE.txt', 'licenses/CC0-1.0.txt')) {
        if (!$checked.ContainsKey($license)) { throw "Missing license: $license" }
        if ((Get-FileHash -LiteralPath (Join-Path $root $license) -Algorithm SHA256).Hash -ne
            (Get-FileHash -LiteralPath (Join-Path $package $license) -Algorithm SHA256).Hash) {
            throw "License differs from source: $license"
        }
    }
    $dlls = @(Get-ChildItem -LiteralPath $package -Filter '*.dll' -File)
    if ($dlls.Count -ne 7) { throw 'Expected exactly seven runtime DLLs' }
    if (@(Get-ChildItem -LiteralPath $package -Filter '*.exe' -File).Count -ne 1) {
        throw 'Test/build executables must not be packaged'
    }

    # Тестовый exe добавлен только во временную распакованную копию.
    Copy-Item -LiteralPath $smoke -Destination (Join-Path $package 'smoke.exe')
    $env:PATH = "$env:SystemRoot\System32;$env:SystemRoot"
    Push-Location -LiteralPath $temp
    try {
        & (Join-Path $package 'smoke.exe')
        if ($LASTEXITCODE -ne 0) { throw "Packaged smoke test failed: $LASTEXITCODE" }
        $process = Start-Process -FilePath (Join-Path $package 'main.exe') -WorkingDirectory $temp -PassThru
        Start-Sleep -Seconds 3
        if ($process.HasExited) { throw "Packaged game exited early: $($process.ExitCode)" }
    } finally {
        Pop-Location
    }
    Write-Output "Package checks passed: $($checked.Count) file hashes, isolated smoke test, game launch."
} finally {
    $env:PATH = $oldPath
    if ($null -ne $process) {
        if (!$process.HasExited) { Stop-Process -Id $process.Id -Force }
        $process.WaitForExit()
        $process.Dispose()
    }
    if (Test-Path -LiteralPath $temp) { Remove-Item -LiteralPath $temp -Recurse -Force }
}
