# Уведомления сторонних компонентов

Эти тексты относятся к сторонним компонентам, а не к авторскому коду игры под MIT. Сохраняйте каталог `licenses/` при распространении сборки. Шрифт PT Sans имеет отдельную лицензию в `assets/PT_Sans-LICENSE.txt`. `CC0-1.0.txt` — полный текст CC0 для собственной графики и эффектов, перечисленных в `assets/LICENSE.md`; скопирован без изменений из включённого текста CC0 для minimp3.

## Происхождение поставляемого комплекта

Автор проекта сообщил, что скачал библиотеки с официального сайта SFML примерно год назад. Это зафиксированный источник со слов автора, но не побайтное подтверждение конкретного архива.

Для проверки выбран официальный комплект SFML 3.0.2 / GCC 14.2.0 / MinGW x64:
https://www.sfml-dev.org/files/SFML-3.0.2-windows-gcc-14.2.0-mingw-64-bit.zip

Прежние попытки скачивания завершались неполным ZIP. Затем автор предоставил распакованную папку `SFML-3.0.2/`. Сравнение показало:

- Все 10 SFML DLL в корне проекта побайтно совпадают с её `bin/` по SHA-256, включая четыре DLL локального ZIP игры.
- Все 114 заголовков в `src/include/` совпадают по содержимому после нормализации CRLF/LF; побайтного совпадения нет из-за переводов строк.
- Из 103 файлов `src/lib/` 95 совпадают побайтно (включая import-библиотеки), восемь CMake-файлов отличаются только переводами строк. Дополнительных или отсутствующих файлов нет.

SFML DLL не заменялись. Это подтверждает соответствие предоставленной папке, но не является независимой проверкой её происхождения: точное имя исходного архива и источник новой копии автор отдельно не подтвердил. В этой папке нет трёх runtime-DLL GCC/MinGW; их источник установлен отдельно ниже. Перед публикацией остаются подтверждение источника комплекта SFML и проверка обязательств по исходникам runtime.

## SFML и встроенные зависимости

Игра поставляет `sfml-graphics-3.dll`, `sfml-window-3.dll`, `sfml-system-3.dll`, `sfml-audio-3.dll`. Отсутствие отдельных DLL кодеков не означает отсутствие их кода: SFML может включать зависимости статически.

Список ниже составлен по исходникам [SFML 3.0.2](https://github.com/SFML/SFML/tree/3.0.2), включая `src/SFML/Audio/CMakeLists.txt`, `src/SFML/Graphics/CMakeLists.txt` и `extlibs/headers/`. Версии внешних библиотек — версии, заданные штатной сборкой SFML 3.0.2; точное соответствие имеющихся DLL официальному бинарному архиву независимо не подтверждено. При замене DLL необходимо повторно проверить состав и условия.

| Компонент | Текст в этом каталоге | Источник текста |
| --- | --- | --- |
| SFML 3.0.2, zlib/png | `SFML.md` | [license.md](https://github.com/SFML/SFML/blob/3.0.2/license.md) |
| FreeType 2.13.2, FreeType License | `FreeType.txt` | [FTL.TXT](https://github.com/freetype/freetype/blob/VER-2-13-2/docs/FTL.TXT) |
| Ogg 1.3.5, BSD | `Ogg.txt` | [COPYING](https://github.com/xiph/ogg/blob/v1.3.5/COPYING) |
| Vorbis 1.3.7, BSD | `Vorbis.txt` | [COPYING](https://github.com/xiph/vorbis/blob/v1.3.7/COPYING) |
| FLAC 1.4.3, BSD для libFLAC | `FLAC.txt` | [COPYING.Xiph](https://github.com/xiph/flac/blob/1.4.3/COPYING.Xiph) |
| miniaudio, выбор MIT No Attribution | `miniaudio.txt` | Лицензионный блок `extlibs/headers/miniaudio/miniaudio.h` из SFML 3.0.2 |
| minimp3, CC0 1.0 | `minimp3-CC0.txt` | [LICENSE](https://github.com/lieff/minimp3/blob/master/LICENSE); указание CC0 в заголовках SFML 3.0.2 |
| stb_image / stb_image_write, выбор MIT | `stb_image.txt`, `stb_image_write.txt` | Лицензионные блоки одноимённых заголовков из SFML 3.0.2 |
| glad / Khronos | `glad.txt`, `Khronos.txt` | [glad v2.0.0 LICENSE](https://github.com/Dav1dde/glad/blob/v2.0.0/LICENSE) и блок Khronos из `extlibs/headers/glad/include/glad/gl.h` SFML 3.0.2 |
| Vulkan headers, Apache 2.0 | `Vulkan-notices.txt`, `Apache-2.0.txt` | Заголовки `vk_platform.h`, `vulkan_core.h` из SFML 3.0.2; стандартный текст Apache 2.0 из [Apache httpd LICENSE](https://github.com/apache/httpd/blob/2.4.x/LICENSE), без уведомлений о компонентах httpd |
| Wine DirectInput header, LGPL 2.1 или новее | `Wine-dinput.txt`, `LGPL-2.1.txt` | Уведомление `extlibs/headers/mingw/dinput.h` SFML 3.0.2; текст LGPL из локального пакета MSYS2 gcc-libs |

Portions of this software are copyright © 2023 The FreeType Project (www.freetype.org). All rights reserved.

Для компонентов с выбором лицензии здесь выбран указанный в таблице вариант; сохранённые исходные блоки могут также содержать альтернативные варианты. `glad.txt` взят из релиза генератора 2.0.0; поставляемые SFML заголовки помечены 2.0.0-beta, поэтому отдельно сохранено встроенное уведомление Khronos. Исходники заголовков доступны в указанном теге SFML.

## Runtime MinGW-w64 / GCC

| Поставляемые файлы | Условия / тексты |
| --- | --- |
| `libgcc_s_seh-1.dll`, `libstdc++-6.dll` | GPL 3.0 с GCC Runtime Library Exception 3.1: `GCC-GPL-3.0.txt`, `GCC-Runtime-Exception-3.1.txt` |
| `libwinpthread-1.dll` | Уведомления MIT и BSD из `MinGW-winpthreads.txt` |
| Runtime MinGW-w64 | Совокупность уведомлений `MinGW-w64-runtime.txt` |

Тексты runtime скопированы без изменений из установленного MSYS2 UCRT64: `share/licenses/gcc-libs/{COPYING3,COPYING.RUNTIME,COPYING.LIB}`, `share/licenses/libwinpthread/COPYING`, `share/licenses/crt/COPYING.MinGW-w64-runtime.txt`.

GCC Runtime Library Exception не переводит авторский код игры под GPL. Однако исключение не отменяет автоматически обязательства при отдельной раздаче самих runtime-DLL. Один набор текстов лицензий не является полным подтверждением соблюдения условий их распространения.

**Перед публичной раздачей DLL:** обеспечить соответствующие исходники/допустимый способ их предоставления согласно применимым лицензиям. Ссылка на последнюю версию проекта не заменяет соответствующие исходники конкретной сборки. Исходники runtime в текущий локальный ZIP не включены; письменное предложение об их предоставлении не заявляется.

Автор уточнил: прежние три runtime-DLL были скопированы из компилятора до переустановки системы. Они отличались от текущей установки; старый libgcc содержал строки GCC 14.2.0.

Три runtime-DLL заменены копиями из `C:/Program Files/Development/msys2/ucrt64/bin/` текущего MSYS2 UCRT64. Принадлежность файлов проверена через `pacman -Qo`, версии — через `pacman -Q`:

| Файлы | Установленный пакет | Версия |
| --- | --- | --- |
| `libgcc_s_seh-1.dll`, `libstdc++-6.dll` | `mingw-w64-ucrt-x86_64-gcc-libs` | `15.2.0-8` |
| `libwinpthread-1.dll` | `mingw-w64-ucrt-x86_64-libwinpthread` | `13.0.0.r124.g2717de84e-1` |
| Компилятор `g++` | `mingw-w64-ucrt-x86_64-gcc` | `15.2.0-8` |
| CRT установки | `mingw-w64-ucrt-x86_64-crt-git` | `13.0.0.r124.g2717de84e-1` |

Новые копии побайтно совпадают с установленными файлами; SHA-256 обновлены в `runtime-sha256.txt`. Резервные копии прежних DLL сохранены вне репозитория, в пользовательском временном каталоге, и не включаются в ZIP. Исходники и рецепты именно этих версий пакетов ещё не подготовлены для распространения; установленный источник бинарников не закрывает это обязательство.

Исходные проекты: [GCC](https://gcc.gnu.org/), [MinGW-w64](https://www.mingw-w64.org/), [рецепты MSYS2](https://github.com/msys2/MINGW-packages). Проверенные текущие бинарные файлы перечислены с SHA-256 в `runtime-sha256.txt`; это идентификация файлов, а не подтверждение их происхождения.

Системные DLL Windows не включаются в архив и не перелицензируются. Перечень уведомлений подготовлен для локальной сборки; окончательная проверка происхождения бинарных файлов и обязательств по исходникам остаётся задачей перед публичным релизом.
