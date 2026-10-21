param()

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$dist = Join-Path $root 'dist'
$cache = Join-Path $dist 'runtime-sources'
$name = 'SpaceAdventure-runtime-sources-gcc-15.2.0-8-winpthreads-13.0.0.r124.g2717de84e-1'
New-Item -ItemType Directory -Path $cache -Force | Out-Null
$files = @()

# Проверены зафиксированные исходные архивы и подписи, а не последние версии.
foreach ($line in (Get-Content -LiteralPath (Join-Path $root 'licenses/runtime-sources-sha256.txt'))) {
    if ($line -notmatch '^([a-f0-9]{64})  ([A-Za-z0-9._-]+)$') {
        throw "Invalid source checksum line: $line"
    }
    $expected = $Matches[1]
    $file = $Matches[2]
    $path = Join-Path $cache $file
    if (!(Test-Path -LiteralPath $path)) {
        $partial = "$path.part"
        try {
            Invoke-WebRequest -UseBasicParsing -TimeoutSec 300 -Uri "https://mirror.msys2.org/mingw/sources/$file" -OutFile $partial
            if ((Get-FileHash -LiteralPath $partial -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) {
                throw "Downloaded source checksum mismatch: $file"
            }
            Move-Item -LiteralPath $partial -Destination $path
        } finally {
            if (Test-Path -LiteralPath $partial) { Remove-Item -LiteralPath $partial -Force }
        }
    }
    if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) {
        throw "Source checksum mismatch; remove the damaged cache file and retry: $path"
    }
    $files += $path
}

$staging = Join-Path $dist ([Guid]::NewGuid().ToString())
$package = Join-Path $staging $name
$archive = Join-Path $dist "$name.zip"
try {
    New-Item -ItemType Directory -Path $package -Force | Out-Null
    foreach ($file in $files) { Copy-Item -LiteralPath $file -Destination $package }
    foreach ($file in @('runtime-sources.md', 'runtime-sources-sha256.txt', 'runtime-sha256.txt',
        'gcc-libs.BUILDINFO.txt', 'libwinpthread.BUILDINFO.txt', 'GCC-GPL-3.0.txt',
        'GCC-Runtime-Exception-3.1.txt', 'LGPL-2.1.txt', 'MinGW-winpthreads.txt', 'MinGW-w64-runtime.txt')) {
        Copy-Item -LiteralPath (Join-Path $root "licenses/$file") -Destination $package
    }
    Get-ChildItem -LiteralPath $package -File | Sort-Object Name | ForEach-Object {
        $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        "$hash  $($_.Name)"
    } | Set-Content -LiteralPath (Join-Path $package 'SHA256SUMS.txt') -Encoding ASCII
    $tempZip = Join-Path $staging "$name.zip"
    Compress-Archive -LiteralPath $package -DestinationPath $tempZip -CompressionLevel Optimal
    Move-Item -LiteralPath $tempZip -Destination $archive -Force
    $hash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
    "$hash  $name.zip" | Set-Content -LiteralPath "$archive.sha256" -Encoding ASCII
    Write-Output "Created: $archive"
    Write-Warning 'Publish this source ZIP alongside the game ZIP with equally accessible download links; do not rely on upstream URLs alone.'
} finally {
    if (Test-Path -LiteralPath $staging) { Remove-Item -LiteralPath $staging -Recurse -Force }
}
