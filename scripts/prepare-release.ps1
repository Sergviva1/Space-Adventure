param(
    [Parameter(Mandatory = $true)]
    [ValidatePattern('^\d+\.\d+\.\d+$')]
    [string]$Version
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
if ($Version -ne (Get-Content -LiteralPath (Join-Path $root 'VERSION') -Raw).Trim()) {
    throw 'Release version differs from VERSION'
}
$dist = Join-Path $root 'dist'
$sourceName = 'SpaceAdventure-runtime-sources-gcc-15.2.0-8-winpthreads-13.0.0.r124.g2717de84e-1.zip'
$archives = @("SpaceAdventure-$Version-win64.zip", $sourceName)
Add-Type -AssemblyName System.IO.Compression.FileSystem

# Проверены все файлы обоих ZIP по внутренним спискам SHA-256.
function Test-Package([string]$Path) {
    $zip = [IO.Compression.ZipFile]::OpenRead($Path)
    try {
        $manifests = @($zip.Entries | Where-Object { $_.FullName.Replace('\', '/') -match '^[^/]+/SHA256SUMS\.txt$' })
        if ($manifests.Count -ne 1) { throw "Expected one checksum manifest: $Path" }
        $manifest = $manifests[0]
        $manifestPath = $manifest.FullName.Replace('\', '/')
        $prefix = $manifestPath.Substring(0, $manifestPath.LastIndexOf('/') + 1)
        $reader = New-Object IO.StreamReader($manifest.Open())
        try { $lines = $reader.ReadToEnd() -split '\r?\n' } finally { $reader.Dispose() }
        $checked = @{}
        foreach ($line in $lines) {
            if (!$line) { continue }
            if ($line -notmatch '^([a-f0-9]{64})  (.+)$') { throw "Invalid checksum: $line" }
            $expected = $Matches[1]
            $relative = $Matches[2]
            if ($checked.ContainsKey($relative)) { throw "Duplicate checksum: $relative" }
            $entries = @($zip.Entries | Where-Object { $_.FullName.Replace('\', '/') -eq ($prefix + $relative) })
            if ($entries.Count -ne 1 -or !$entries[0].Name) { throw "Missing or duplicate ZIP file: $relative" }
            $stream = $entries[0].Open()
            $sha = [Security.Cryptography.SHA256]::Create()
            try { $actual = [BitConverter]::ToString($sha.ComputeHash($stream)).Replace('-', '').ToLowerInvariant() }
            finally { $sha.Dispose(); $stream.Dispose() }
            if ($actual -ne $expected) { throw "ZIP checksum mismatch: $relative" }
            $checked[$relative] = $true
        }
        $files = @($zip.Entries | Where-Object { $_.Name })
        if ($files.Count -ne $checked.Count + 1) { throw "Unlisted ZIP files: $Path" }
        Write-Output "Verified $($checked.Count) file hashes: $Path"
    } finally { $zip.Dispose() }
}

$staging = Join-Path $dist ([Guid]::NewGuid().ToString())
$destination = Join-Path $dist "release-$Version"
try {
    New-Item -ItemType Directory -Path $staging -Force | Out-Null
    foreach ($name in $archives) {
        $path = Join-Path $dist $name
        $checksum = "$path.sha256"
        $line = (Get-Content -LiteralPath $checksum -Raw).Trim()
        if ($line -notmatch '^([a-f0-9]{64})  (.+)$') { throw "Invalid archive checksum: $checksum" }
        $expected = $Matches[1]
        if ($Matches[2] -cne $name) { throw "Unexpected archive name in $checksum" }
        if ((Get-FileHash -LiteralPath $path -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) {
            throw "Archive checksum mismatch: $path"
        }
        Test-Package $path
        Copy-Item -LiteralPath $path -Destination $staging
        Copy-Item -LiteralPath $checksum -Destination $staging
    }
    if (Test-Path -LiteralPath $destination) { Remove-Item -LiteralPath $destination -Recurse -Force }
    Move-Item -LiteralPath $staging -Destination $destination
    Write-Output "Release files ready: $destination"
    Write-Output 'Upload all four files together. Version changes: CHANGELOG.md'
} finally {
    if (Test-Path -LiteralPath $staging) { Remove-Item -LiteralPath $staging -Recurse -Force }
}
