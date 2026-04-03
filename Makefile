# ==============================================================================
#  SpaceShooter — Makefile (Linux / macOS)
# ==============================================================================
#
#  AUTHOR         : PK
#                   [ Build System & Compilation Pipeline ]
#                   Responsible for:
#                     - GCC/G++ compilation flags (C++17, -O2, -Wall)
#                     - sdl2-config integration for automatic SDL2 flags
#                     - Platform switch: linux vs macos (GL vs OpenGL fw)
#                     - Object file dependency tracking
#                     - clean target
#
#  Project        : Space Shooter C++/OpenGL
#  File           : Makefile
#  Usage          : make              (Linux)
#                   make PLATFORM=macos  (macOS)
#
#  Prime Author   : Mahmudur Rahman  (Renderer.cpp)
#  Contributors   : Era (GameLogic) · Mitu (Headers) · Tripty (main.cpp)
# ==============================================================================

# SpaceShooter Makefile (Linux / macOS)
# Usage:
#   Linux:  make
#   macOS:  make PLATFORM=macos

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
