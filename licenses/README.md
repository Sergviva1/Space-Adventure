# Сторонние компоненты

Каталог содержит уведомления библиотек и runtime, поставляемых с игрой. Сохраняйте его при распространении сборки. Лицензия MIT в корне проекта относится только к коду, тестам, скриптам и авторской документации.

Шрифт PT Sans имеет отдельную лицензию в `assets/PT_Sans-LICENSE.txt`. `CC0-1.0.txt` относится к графике и эффектам, перечисленным в `assets/LICENSE.md`.

## SFML 3.0.2

Комплект для Windows x64 / GCC 14.2.0 / MinGW:
https://www.sfml-dev.org/files/SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit.zip

SHA-256 проверенного локального архива:
`9e62934512b710a2439134a413165bb87301373fced59b4ff6aedf5d736a7019`.

Четыре SFML DLL и используемые import-библиотеки совпадают с архивом побайтно; 114 заголовков совпадают после нормализации CRLF/LF. Независимое сравнение с повторно загруженным официальным архивом не выполнено. В `src/lib/` оставлены только библиотеки динамической release-сборки для graphics, window, system и audio; CMake-конфигурации, статические и отладочные библиотеки не поставляются.

Игра распространяется с `sfml-graphics-3.dll`, `sfml-window-3.dll`, `sfml-system-3.dll`, `sfml-audio-3.dll`. Встроенные в них зависимости также требуют сохранения уведомлений.

Состав и версии ниже соответствуют штатной конфигурации [SFML 3.0.2](https://github.com/SFML/SFML/tree/3.0.2). Точное соответствие бинарников официальной сборке независимо не установлено; при замене DLL состав зависимостей необходимо проверить заново.

| Компонент | Уведомление | Источник |
| --- | --- | --- |
| SFML 3.0.2, zlib/png | `SFML.md` | [license.md](https://github.com/SFML/SFML/blob/3.0.2/license.md) |
| FreeType 2.13.2, FreeType License | `FreeType.txt` | [FTL.TXT](https://github.com/freetype/freetype/blob/VER-2-13-2/docs/FTL.TXT) |
| Ogg 1.3.5, BSD | `Ogg.txt` | [COPYING](https://github.com/xiph/ogg/blob/v1.3.5/COPYING) |
| Vorbis 1.3.7, BSD | `Vorbis.txt` | [COPYING](https://github.com/xiph/vorbis/blob/v1.3.7/COPYING) |
| FLAC 1.4.3, BSD для libFLAC | `FLAC.txt` | [COPYING.Xiph](https://github.com/xiph/flac/blob/1.4.3/COPYING.Xiph) |
| miniaudio, MIT No Attribution | `miniaudio.txt` | Лицензионный блок `extlibs/headers/miniaudio/miniaudio.h` SFML 3.0.2 |
| minimp3, CC0 1.0 | `minimp3-CC0.txt` | [LICENSE](https://github.com/lieff/minimp3/blob/master/LICENSE), указание CC0 в заголовках SFML 3.0.2 |
| stb_image / stb_image_write, MIT | `stb_image.txt`, `stb_image_write.txt` | Лицензионные блоки заголовков SFML 3.0.2 |
| glad / Khronos | `glad.txt`, `Khronos.txt` | [glad v2.0.0 LICENSE](https://github.com/Dav1dde/glad/blob/v2.0.0/LICENSE), блок Khronos в `extlibs/headers/glad/include/glad/gl.h` |
| Vulkan headers, Apache 2.0 | `Vulkan-notices.txt`, `Apache-2.0.txt` | `vk_platform.h`, `vulkan_core.h` SFML 3.0.2; [текст Apache 2.0](https://github.com/apache/httpd/blob/2.4.x/LICENSE) |
| Wine DirectInput header, LGPL 2.1 или новее | `Wine-dinput.txt`, `LGPL-2.1.txt` | `extlibs/headers/mingw/dinput.h` SFML 3.0.2, текст LGPL из MSYS2 gcc-libs |

Portions of this software are copyright © 2023 The FreeType Project (www.freetype.org). All rights reserved.

Для компонентов с выбором лицензии в таблице указан используемый вариант. `glad.txt` относится к генератору 2.0.0; заголовки SFML помечены 2.0.0-beta, поэтому отдельно сохранено уведомление Khronos. Тексты лицензий сохранены без изменений.

## Runtime GCC / MinGW-w64

| Файлы | Пакет MSYS2 UCRT64 | Версия | Условия |
| --- | --- | --- | --- |
| `libgcc_s_seh-1.dll`, `libstdc++-6.dll` | `mingw-w64-ucrt-x86_64-gcc-libs` | `15.2.0-8` | GPL 3.0 с GCC Runtime Library Exception 3.1 |
| `libwinpthread-1.dll` | `mingw-w64-ucrt-x86_64-libwinpthread` | `13.0.0.r124.g2717de84e-1` | MIT и BSD |

Файлы побайтно совпадают с соответствующими бинарными пакетами. Контрольные суммы семи поставляемых DLL находятся в `runtime-sha256.txt`. Окружение сборки пакетов зафиксировано в `gcc-libs.BUILDINFO.txt` и `libwinpthread.BUILDINFO.txt`.

Тексты условий: `GCC-GPL-3.0.txt`, `GCC-Runtime-Exception-3.1.txt`, `LGPL-2.1.txt`, `MinGW-winpthreads.txt`, `MinGW-w64-runtime.txt`. Они скопированы без изменений из уведомлений MSYS2 UCRT64.

GCC Runtime Library Exception не переводит код игры под GPL. При раздаче самих DLL нужно отдельно соблюдать их условия. Для сетевого распространения подготовлен ZIP соответствующих исходников, рецептов и патчей MSYS2: его следует публиковать рядом с игровым ZIP с равнозначным доступом по GPLv3 §6(d). Письменное предложение об исходниках не используется.

Инструкция упаковки и воспроизведения проверки — [runtime-sources.md](runtime-sources.md). Подписи исходных архивов и соответствие рецептов бинарным пакетам проверены; полная пересборка runtime не выполнялась.

Исходные проекты: [GCC](https://gcc.gnu.org/), [MinGW-w64](https://www.mingw-w64.org/), [рецепты MSYS2](https://github.com/msys2/MINGW-packages). Системные DLL Windows не включаются в архив игры.
