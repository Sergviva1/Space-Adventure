SHELL = cmd.exe
.SHELLFLAGS = /c

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
PROJECT_DIR = $(CURDIR)
LIBS = -L"$(PROJECT_DIR)/src/lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
EXECUTABLE = main.exe
COMMON_SOURCES = "$(PROJECT_DIR)/coding/Resources.cpp"
VERSION ?= local
RELEASE_VERSION := $(strip $(file <VERSION))

# Компилятор запускался из временной папки пользователя, чтобы DLL игры в корне не конфликтовали с DLL GCC.
all:
	set "TMP=%LOCALAPPDATA%\Temp" && set "TEMP=%LOCALAPPDATA%\Temp" && set "TMPDIR=%LOCALAPPDATA%\Temp" && cd /d "%LOCALAPPDATA%\Temp" && $(CXX) -I"$(PROJECT_DIR)/src/include" $(CXXFLAGS) -o "$(PROJECT_DIR)/$(EXECUTABLE)" "$(PROJECT_DIR)/coding/main.cpp" $(COMMON_SOURCES) $(LIBS)

# Тест включал main.cpp с доступом к внутренним полям только в тестовой сборке.
smoke:
	set "TMP=%LOCALAPPDATA%\Temp" && set "TEMP=%LOCALAPPDATA%\Temp" && set "TMPDIR=%LOCALAPPDATA%\Temp" && cd /d "%LOCALAPPDATA%\Temp" && $(CXX) -I"$(PROJECT_DIR)/src/include" $(CXXFLAGS) -fno-access-control -o "$(PROJECT_DIR)/smoke.exe" "$(PROJECT_DIR)/tests/smoke.cpp" $(COMMON_SOURCES) $(LIBS)
	"$(PROJECT_DIR)/smoke.exe"

screenshots:
	set "TMP=%LOCALAPPDATA%\Temp" && set "TEMP=%LOCALAPPDATA%\Temp" && set "TMPDIR=%LOCALAPPDATA%\Temp" && cd /d "%LOCALAPPDATA%\Temp" && $(CXX) -I"$(PROJECT_DIR)/src/include" $(CXXFLAGS) -fno-access-control -o "$(PROJECT_DIR)/capture.exe" "$(PROJECT_DIR)/tests/capture.cpp" $(COMMON_SOURCES) $(LIBS)
	"$(PROJECT_DIR)/capture.exe"

# Версия local оставлена для повседневной проверки сборки.
dist: all smoke
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(PROJECT_DIR)/scripts/package.ps1" -Version "$(VERSION)"

dist-check: dist
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(PROJECT_DIR)/tests/check-package.ps1" -Archive "$(PROJECT_DIR)/dist/SpaceAdventure-$(VERSION)-win64.zip"

# Подготовлен комплект релиза с проверкой игры и исходниками runtime.
release:
	"$(MAKE)" dist-check VERSION=$(RELEASE_VERSION)
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(PROJECT_DIR)/scripts/package-runtime-sources.ps1"
	powershell.exe -NoProfile -ExecutionPolicy Bypass -File "$(PROJECT_DIR)/scripts/prepare-release.ps1" -Version "$(RELEASE_VERSION)"

clean:
	if exist $(EXECUTABLE) del $(EXECUTABLE)
	if exist smoke.exe del smoke.exe
	if exist capture.exe del capture.exe

.PHONY: all smoke screenshots dist dist-check release clean
