# Space Shooter

A classic 2D space shooter game built with SDL2 and OpenGL.

![Version](https://img.shields.io/badge/version-6.2-blue)
![License](https://img.shields.io/badge/license-MIT-green)

## Features

- **3 Difficulty Modes**: Easy, Medium, and Hard
- **10 Progressive Levels**: Each level increases in difficulty with more enemies, faster speeds, and tighter spawn rates
- **Dynamic Enemy Behavior**: Enemies can zigzag, dive at the player, and shoot back
- **Boss Battles**: Face challenging bosses at levels 5, 8, and 10 (difficulty-dependent)
- **Particle Effects**: Explosions and visual feedback
- **Animated Starfield**: Multi-layer parallax star background

## Game Controls

| Key | Action |
|-----|--------|
| Arrow Keys / WASD | Move the player ship |
| Space | Fire weapon |
| Enter | Select / Confirm / Restart |
| Backspace | Go back (in difficulty select) |
| Escape | Quit game |

## Requirements

- **Linux**: SDL2, OpenGL, g++ with C++17 support
- **macOS**: SDL2, OpenGL framework
- **Windows 11**: SDL2, OpenGL, MinGW or MSVC

### Installing Dependencies

**Ubuntu/Debian:**
```bash
sudo apt-get install libsdl2-dev libgl1-mesa-dev
```

**Fedora:**
```bash
sudo dnf install SDL2-devel mesa-libGL-devel
```

**Arch Linux:**
```bash
sudo pacman -S sdl2 mesa
```

**macOS (with Homebrew):**
```bash
brew install sdl2
```

**Windows 11 (MSYS2):**
```bash
# Install MSYS2 from https://www.msys2.org/
# Then in MSYS2 UCRT64 terminal:
pacman -S mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-gcc
```

**Windows 11 (vcpkg):**
```bash
# Install vcpkg, then:
vcpkg install sdl2:x64-windows
vcpkg integrate install
```

## Building

### Using Make

**Linux/macOS:**
```bash
make clean          # Clean previous builds
make                # Build the game
./SpaceShooter      # Run the game (Linux/macOS)
```

**Windows 11:**
```bash
make PLATFORM=windows   # Build for Windows
.\SpaceShooter.exe      # Run the game
```

### Build Options

```bash
make PLATFORM=linux     # Linux build (default)
make PLATFORM=macos     # macOS build
make PLATFORM=windows   # Windows 11 build
```

## Project Structure

```
SpaceShooter/
├── include/
│   ├── Game.h       # Game data structures, entities, state machine
│   └── Renderer.h   # OpenGL rendering functions
├── src/
│   ├── main.cpp     # Entry point, SDL2 initialization, game loop
│   ├── GameLogic.cpp # Game mechanics, collision, spawning
│   └── Renderer.cpp  # OpenGL drawing routines
├── Makefile
├── README.md
└── plan.md          # Development roadmap
```

## Gameplay

### Objective
Destroy enemies, survive waves of attacks, and complete all 10 levels to achieve victory.

### Scoring
- Each enemy killed grants points (varies by difficulty)
- Survive and rack up the highest score possible

### Lives System
- Start with 3 lives (Easy), 3 lives (Medium), or 2 lives (Hard)
- Brief invincibility after being hit
- Game over when all lives are lost

## Level Progression

| Level | Features Unlocked |
|-------|-------------------|
| 1-2   | Basic enemies |
| 3-4   | Increased spawn rate |
| 5     | First boss battle (Easy/Medium) |
| 6-7   | Zigzag movement, diving attacks |
| 8     | Boss battle |
| 9     | Maximum challenge |
| 10    | Final boss battle |

## Technical Details

- **Renderer**: OpenGL 2.1 with 4x MSAA antialiasing
- **Target FPS**: 60 FPS with vsync
- **Window**: 840x1040 (portrait orientation), resizable
- **Double Buffering**: Enabled for smooth rendering

## License

MIT License - See LICENSE file for details

## Acknowledgments

Built with:
- [SDL2](https://www.libsdl.org/) - Cross-platform development library
- [OpenGL](https://www.opengl.org/) - Graphics rendering
