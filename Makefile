CXX = g++
CXXFLAGS = -Isrc/include -std=c++17
LIBS = -Lsrc/lib -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio
EXECUTABLE = main.exe

all:
	$(CXX) $(CXXFLAGS) -o $(EXECUTABLE) coding/main.cpp $(LIBS)

clean:
	del $(EXECUTABLE)

.PHONY: all clean