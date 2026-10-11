# Space Adventure Game

Небольшая 2D-игра на C++17 и SFML 3.0.2: управляйте космическим кораблём, уворачивайтесь от метеорита, собирайте монеты и бонусы лечения.

## Запуск

Соберите проект по инструкции ниже. Каталоги `assets/` и исполняемые DLL должны находиться рядом с `main.exe`. Игру можно запускать из любой рабочей папки.

Для запуска из репозитория выполните `.\main.exe` в PowerShell или `main.exe` в Command Prompt.

## Управление

- **В меню:** `Enter` — начать игру, `Esc` — выйти.
- **Во время игры:** `WASD` — движение корабля, `N` — сменить музыку, `Esc` — выйти.

## Требования

- Windows x64.
- MSYS2 UCRT64 с MinGW-w64 GCC и GNU Make (`mingw32-make`). Сборка проверена с GCC 15.2.0 и GNU Make 4.4.1.
- SFML 3.0.2 уже находится в `src/include` и `src/lib`; отдельно устанавливать SFML для сборки не требуется.

В терминале MSYS2 UCRT64 установите инструменты командой:

```text
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make
```

## Сборка

Откройте Windows Command Prompt, PowerShell или MSYS2 UCRT64 с `g++` и `mingw32-make` в `PATH`, перейдите в корень репозитория и выполните:

```text
mingw32-make
```

Исполняемый файл появится как `main.exe`. Makefile запускает компилятор из `%LOCALAPPDATA%\Temp`, чтобы runtime-DLL игры не конфликтовали с DLL GCC в корне репозитория.

Очистить результат сборки:

```text
mingw32-make clean
```

## Файлы для запуска

Для запуска `main.exe` рядом с ним должны находиться папка `assets/` и следующие DLL:

- `libgcc_s_seh-1.dll`
- `libstdc++-6.dll`
- `libwinpthread-1.dll`
- `sfml-audio-3.dll`
- `sfml-graphics-3.dll`
- `sfml-system-3.dll`
- `sfml-window-3.dll`

Шрифт PT Sans поставляется в `assets/`; его лицензия находится в `assets/PT_Sans-LICENSE.txt`.
