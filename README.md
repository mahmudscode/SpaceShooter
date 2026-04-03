# ⬡ Space Shooter — C++ OpenGL Edition

A faithful C++/OpenGL port of the HTML5 canvas space shooter game.
Same gameplay, same visuals — parallax stars, custom ships, alien swarms, explosion particles.

## Features
- **Parallax star field** — 3 layers at different scroll speeds
- **Custom player ship** — sleek fighter with animated thrust flames & cockpit glow
- **Alien saucers** — pulsing, rotating lights, tractor beam when near bottom
- **Dual-barrel bullets** — with additive glow trails
- **Explosion particles** — colorful burst with additive blending
- **Increasing difficulty** — enemy speed & spawn rate scale with level
- **Letterbox rendering** — maintains 420×520 aspect at any window size
- **MSAA 4×** — smooth edges on all geometry

## Controls
| Key | Action |
|-----|--------|
| ← / A | Move left |
| → / D | Move right |
| Space | Fire |
| Enter | Start / Restart |
| Escape | Quit |

## Dependencies
- **SDL2** — window, input, OpenGL context
- **OpenGL 2.1** — fixed-function pipeline (no shaders needed)

## Build

### Linux
```bash
sudo apt install libsdl2-dev libgl1-mesa-dev   # Debian/Ubuntu
# or
sudo pacman -S sdl2 mesa                        # Arch

make
./SpaceShooter
```

### macOS
```bash
brew install sdl2
make PLATFORM=macos
./SpaceShooter
```

### Windows (MSVC + vcpkg)
```powershell
vcpkg install sdl2:x64-windows
cmake -B build -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build build --config Release
.\build\Release\SpaceShooter.exe
```

### Windows (MinGW)
```bash
# Install SDL2 dev package for MinGW, then:
g++ -std=c++17 -O2 -Iinclude src/*.cpp -o SpaceShooter.exe -lSDL2 -lSDL2main -lopengl32
```

### Using CMake (all platforms)
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Project Structure
```
SpaceShooter/
├── include/
│   ├── Game.h         # Data structures (Player, Enemy, Bullet, Particle, Star, GameData)
│   └── Renderer.h     # Render API declaration
├── src/
│   ├── main.cpp       # SDL2 window + game loop
│   ├── GameLogic.cpp  # Update: movement, collision, spawn, particles
│   └── Renderer.cpp   # Draw: all OpenGL rendering (no shaders, no textures)
├── CMakeLists.txt
├── Makefile
└── README.md
```

## How it maps to your HTML version

| HTML Canvas | C++ OpenGL |
|---|---|
| `canvas.getContext('2d')` | OpenGL 2.1 fixed pipeline |
| `requestAnimationFrame` | `SDL_GL_SwapWindow` + frame cap |
| `createLinearGradient` | `GL_QUADS` with per-vertex color |
| `createRadialGradient` | `GL_TRIANGLE_FAN` with center bright / edge transparent |
| `shadowBlur` / glow | Additive blending (`GL_ONE`) + large soft circle behind small one |
| `globalAlpha` | `glColor4f(r,g,b,alpha)` |
| `ellipse()` | `drawEllipseFill()` using `GL_TRIANGLE_FAN` |
| `Math.sin(frameCount)` | Same formula, same `frameCount` int |

## Scoring
- +10 per enemy destroyed
- Level up every 100 points (max level 8)
- 3 lives — lose one per enemy that reaches the bottom or hits you
