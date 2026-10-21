param(
    [ValidatePattern('^[A-Za-z0-9][A-Za-z0-9._-]*$')]
    [string]$Version = 'local'
)

Set-StrictMode -Version Latest
$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$dist = Join-Path $root 'dist'
$name = "SpaceAdventure-$Version-win64"

# Проверены все обязательные файлы до создания архива.
$files = @(
    'main.exe',
    'libgcc_s_seh-1.dll', 'libstdc++-6.dll', 'libwinpthread-1.dll',
    'sfml-audio-3.dll', 'sfml-graphics-3.dll', 'sfml-system-3.dll', 'sfml-window-3.dll',
    'README.md', 'SPEC.md', 'CREDITS.md', 'LICENSE',
    'licenses/README.md', 'licenses/runtime-sha256.txt', 'licenses/CC0-1.0.txt',
    'licenses/runtime-sources.md',
    'licenses/runtime-sources-sha256.txt', 'licenses/gcc-libs.BUILDINFO.txt',
    'licenses/libwinpthread.BUILDINFO.txt',
    'licenses/SFML.md', 'licenses/FreeType.txt', 'licenses/Ogg.txt',
    'licenses/Vorbis.txt', 'licenses/FLAC.txt', 'licenses/miniaudio.txt',
    'licenses/minimp3-CC0.txt', 'licenses/stb_image.txt', 'licenses/stb_image_write.txt',
    'licenses/glad.txt', 'licenses/Khronos.txt', 'licenses/Vulkan-notices.txt',
    'licenses/Apache-2.0.txt', 'licenses/Wine-dinput.txt', 'licenses/LGPL-2.1.txt',
    'licenses/GCC-GPL-3.0.txt', 'licenses/GCC-Runtime-Exception-3.1.txt',
    'licenses/MinGW-winpthreads.txt', 'licenses/MinGW-w64-runtime.txt',
    'assets/LICENSE.md', 'assets/PT_Sans-Regular.ttf', 'assets/PT_Sans-LICENSE.txt',
    'assets/spritelist.png', 'assets/meteor.png', 'assets/bonus.png', 'assets/coin.png',
    'assets/space.png', 'assets/gamepanel.png', 'assets/gameover.png',
    'assets/healthbar.png', 'assets/icon.png',
    'assets/retro-waves-of-love_92802.ogg', 'assets/retro-dreamscape_92772.ogg',
    'assets/pixelated-adventures_92797.ogg',
    'assets/hitHurt.ogg', 'assets/pickupCoin.ogg',
    'assets/powerUp.ogg', 'assets/explosion.ogg',
    'docs/screenshots/menu.png', 'docs/screenshots/gameplay.png',
    'docs/screenshots/pause.png', 'docs/screenshots/gameover.png'
)
foreach ($file in $files) {
    $path = Join-Path $root $file
    if (!(Test-Path -LiteralPath $path -PathType Leaf) -or (Get-Item -LiteralPath $path).Length -eq 0) {
        throw "Missing or empty package file: $file"
    }
}

# Изменение DLL требовало повторной проверки происхождения и лицензий.
foreach ($line in (Get-Content -LiteralPath (Join-Path $root 'licenses/runtime-sha256.txt'))) {
    if ($line -notmatch '^([a-f0-9]{64})  ([A-Za-z0-9_+.-]+\.dll)$') {
        throw "Invalid runtime checksum line: $line"
    }
    $expected = $Matches[1]
    $dll = $Matches[2]
    if ((Get-FileHash -LiteralPath (Join-Path $root $dll) -Algorithm SHA256).Hash.ToLowerInvariant() -ne $expected) {
        throw "Runtime changed; review licenses/README.md and runtime-sha256.txt: $dll"
    }
}

New-Item -ItemType Directory -Path $dist -Force | Out-Null
$staging = Join-Path $dist ([Guid]::NewGuid().ToString())
$package = Join-Path $staging $name
$tempZip = Join-Path $staging "$name.zip"
$archive = Join-Path $dist "$name.zip"
try {
    New-Item -ItemType Directory -Path $package -Force | Out-Null
    foreach ($file in $files) {
        $destination = Join-Path $package $file
        New-Item -ItemType Directory -Path (Split-Path -Parent $destination) -Force | Out-Null
        Copy-Item -LiteralPath (Join-Path $root $file) -Destination $destination
    }

    # Архив предназначен для локальной проверки, а не для публикации.
    @'
ЛОКАЛЬНАЯ ПРОВЕРОЧНАЯ СБОРКА — НЕ ПУБЛИЧНЫЙ РЕЛИЗ

Распакуйте весь архив и запустите main.exe. Устанавливать SFML или компилятор не нужно.
Не перемещайте main.exe отдельно от DLL и assets/. Рабочая папка может быть любой.
Управление: Enter — старт; WASD — движение; N — музыка; Esc/P — пауза.
На паузе: Enter — продолжить; M — меню; Q — выход.
После поражения: R/Enter — новая игра; M — меню; Esc — выход.
Рекорд хранится в %LOCALAPPDATA%\SpaceAdventure\highscore.txt, отдельно от игры.

Код — MIT (LICENSE), графика и эффекты — CC0 в пределах применимых прав
(assets/LICENSE.md), шрифт — SIL OFL. Музыка Zvukogram имеет отдельные условия
и не предоставляется под MIT или CC0; источники и ограничения — в CREDITS.md.
Уведомления библиотек и версии пакетов находятся в licenses/.
При публичной раздаче игры разместите рядом ZIP исходников runtime GCC/MinGW
с равнозначным доступом. Инструкция — licenses/runtime-sources.md.
Локальная сборка не заменяет проверку на отдельном Windows-компьютере.

SHA256SUMS.txt содержит контрольные суммы файлов (кроме самого списка).
'@ | Set-Content -LiteralPath (Join-Path $package 'START-HERE.txt') -Encoding UTF8

    $hashes = Get-ChildItem -LiteralPath $package -Recurse -File | Sort-Object FullName | ForEach-Object {
        $relative = $_.FullName.Substring($package.Length + 1).Replace('\', '/')
        $hash = (Get-FileHash -LiteralPath $_.FullName -Algorithm SHA256).Hash.ToLowerInvariant()
        "$hash  $relative"
    }
    $hashes | Set-Content -LiteralPath (Join-Path $package 'SHA256SUMS.txt') -Encoding ASCII
    Compress-Archive -LiteralPath $package -DestinationPath $tempZip -CompressionLevel Optimal
    Move-Item -LiteralPath $tempZip -Destination $archive -Force
    $archiveHash = (Get-FileHash -LiteralPath $archive -Algorithm SHA256).Hash.ToLowerInvariant()
    "$archiveHash  $name.zip" | Set-Content -LiteralPath "$archive.sha256" -Encoding ASCII
    Write-Warning 'Local build: test on a separate Windows PC before release and publish the runtime source ZIP alongside the game.'
    Write-Output "Created: $archive"
    Write-Output "SHA256: $archiveHash"
} finally {
    if (Test-Path -LiteralPath $staging) {
        Remove-Item -LiteralPath $staging -Recurse -Force
    }
}
