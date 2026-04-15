

CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Iinclude
TARGET   = SpaceShooter

SRCS = src/main.cpp src/GameLogic.cpp src/Renderer.cpp
OBJS = $(SRCS:.cpp=.o)

PLATFORM ?= linux

ifeq ($(PLATFORM),macos)
    SDL2_CFLAGS = $(shell sdl2-config --cflags)
    SDL2_LIBS   = $(shell sdl2-config --libs)
    GL_LIBS     = -framework OpenGL
else ifeq ($(PLATFORM),windows)
    # Windows (MinGW/MSYS2) - adjust paths if using vcpkg or manual SDL2 install
    SDL2_CFLAGS = -I"C:/SDL2/include"
    SDL2_LIBS   = -L"C:/SDL2/lib" -lSDL2 -lSDL2main
    GL_LIBS     = -lopengl32 -lglu32 -lm
    TARGET      = SpaceShooter.exe
else
    SDL2_CFLAGS = $(shell sdl2-config --cflags)
    SDL2_LIBS   = $(shell sdl2-config --libs)
    GL_LIBS     = -lGL -lm
endif

CXXFLAGS += $(SDL2_CFLAGS)
LDFLAGS   = $(SDL2_LIBS) $(GL_LIBS)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)

.PHONY: all clean
