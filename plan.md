# Development Plan

## Current Status: v6.2

### Recent Changes
- Fixed compilation error in main.cpp (missing braces in event loop)
- Cleaned up build system

---

## Roadmap

### Phase 1: Bug Fixes & Polish (v6.3 - v6.5)

#### v6.3 - Stability Fixes
- [ ] Add proper error handling for SDL2 initialization failures
- [ ] Handle window resize events gracefully
- [ ] Fix potential memory leaks on error paths
- [ ] Add null checks for OpenGL context creation

#### v6.4 - Input Improvements
- [ ] Add key rebinding support (config file)
- [ ] Fix input buffering for faster response
- [ ] Add gamepad/controller support

#### v6.5 - Visual Polish
- [ ] Add screen shake on damage
- [ ] Improve explosion particle variety
- [ ] Add hit flash effect on enemies
- [ ] Smooth camera transitions

---

### Phase 2: Content Expansion (v7.0)

#### New Enemy Types
- [ ] Kamikaze - fast, low HP, direct pursuit
- [ ] Tank - slow, high HP, heavy fire
- [ ] Splitter - splits into smaller enemies on death
- [ ] Teleporter - blinks around the screen

#### Power-ups System
- [ ] Spread Shot - fire multiple bullets
- [ ] Rapid Fire - increased fire rate
- [ ] Shield - temporary invincibility
- [ ] Health Pick-up - restore one life
- [ ] Bomb - clear screen of enemies

#### Boss Improvements
- [ ] Unique boss for each difficulty level
- [ ] Multi-phase boss fights
- [ ] Boss-specific mechanics and patterns

---

### Phase 3: Audio (v8.0)

- [ ] Background music (menu, gameplay, boss, victory/game over)
- [ ] Sound effects:
  - [ ] Player shooting
  - [ ] Enemy shooting
  - [ ] Explosions
  - [ ] Power-up collection
  - [ ] Boss spawn warning
  - [ ] Level complete fanfare
- [ ] Audio settings menu (volume controls)

---

### Phase 4: UI/UX Improvements (v9.0)

- [ ] Animated main menu
- [ ] Pause menu with options
- [ ] Settings screen:
  - [ ] Audio volume
  - [ ] Display brightness
  - [ ] Key bindings
- [ ] High score system with local leaderboard
- [ ] Level preview before starting
- [ ] Tutorial/Help screen

---

### Phase 5: Advanced Features (v10.0)

#### Game Modes
- [ ] Endless Mode - survive as long as possible
- [ ] Time Attack - complete levels as fast as possible
- [ ] Bullet Hell - extreme difficulty mode

#### Visual Upgrades
- [ ] Animated player ship sprites
- [ ] Enemy death animations
- [ ] Animated backgrounds (nebulae, asteroids)
- [ ] CRT/scanline shader option

#### Progression System
- [ ] Persistent upgrades between runs
- [ ] Unlockable ships with different stats
- [ ] Achievement system

---

## Known Issues

| Issue | Severity | Status |
|-------|----------|--------|
| Unused variable `now` warning in main.cpp | Low | Open |
| Window resize may cause rendering issues | Medium | Open |

---

## Technical Debt

- [ ] Add unit tests for GameLogic functions
- [ ] Document all Renderer functions with examples
- [ ] Consider entity-component-system architecture for scalability
- [ ] Profile and optimize particle system for large counts
- [ ] Add logging system for debugging

---

## Future Considerations

### Platform Support
- [ ] Windows build support
- [ ] Mobile (iOS/Android) touch controls
- [ ] WebAssembly browser version

### Multiplayer
- [ ] Local co-op (2 players)
- [ ] Online leaderboard integration

### Mod Support
- [ ] Custom level editor
- [ ] Mod loading API
- [ ] Steam Workshop integration

---

## Version History

| Version | Date | Changes |
|---------|------|---------|
| v6.2 | 2026-04-15 | Fixed main.cpp compilation error |
| v6.1 | - | Previous release |
| v6.0 | - | Previous release |
| v5.1 | - | Previous release |

---

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

---

*Last updated: 2026-04-15*
