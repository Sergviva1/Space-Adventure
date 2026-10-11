SHELL = cmd.exe
.SHELLFLAGS = /c

CXX = g++
CXXFLAGS = -std=c++17
PROJECT_DIR = $(CURDIR)
LIBS = -L"$(PROJECT_DIR)/src/lib" -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
EXECUTABLE = main.exe

# Компилятор запускался из временной папки пользователя, чтобы DLL игры в корне не конфликтовали с DLL GCC.
all:
	set "TMP=%LOCALAPPDATA%\Temp" && set "TEMP=%LOCALAPPDATA%\Temp" && set "TMPDIR=%LOCALAPPDATA%\Temp" && cd /d "%LOCALAPPDATA%\Temp" && $(CXX) -I"$(PROJECT_DIR)/src/include" $(CXXFLAGS) -o "$(PROJECT_DIR)/$(EXECUTABLE)" "$(PROJECT_DIR)/coding/main.cpp" $(LIBS)

clean:
	del $(EXECUTABLE)

.PHONY: all clean
