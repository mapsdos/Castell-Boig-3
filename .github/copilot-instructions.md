# Castell Boig 3 - Project Instructions

## Project Overview
**Castell Boig 3** is a 2D platformer game developed in C++ using modern graphics libraries. The game features multiple levels with a player character that must navigate through tile-based maps while handling physics (jumping, collision detection) and various game states.

### Key Features
- **Multiple Game Scenes**: Main Menu, Instructions, Credits, and Level Progression
- **Player Mechanics**: Movement, jumping, and collision detection
- **Graphics Rendering**: Using OpenGL with SFML and custom shader support
- **Audio System**: Background music and sound effects (SFX)
- **Level System**: Multiple level scenes with tilemap-based level design
- **Input Handling**: Keyboard and mouse controls

---

## System Requirements

### Software
- **Visual Studio 2015 or later** (project uses Visual C++)
- **Windows OS** (x86 architecture)
- **OpenGL 3.3+** compatible graphics hardware

### Development Environment
- C++11 or later
- Windows SDK

---

## Project Structure

```
Castell-Boig-3/
├── src/                          # Source code files
│   ├── main.cpp                  # Application entry point
│   ├── Game.h/cpp                # Main game logic (singleton)
│   ├── Player.h/cpp              # Player character controller
│   ├── MenuScene.h/cpp           # Main menu scene
│   ├── LevelScene.h/cpp          # Level gameplay scene
│   ├── CreditsScene.h/cpp        # Credits scene
│   ├── InstructionsScene.h/cpp   # Instructions scene
│   ├── Sprite.h/cpp              # Sprite rendering
│   ├── TileMap.h/cpp             # Tile-based level maps
│   ├── Texture.h/cpp             # Texture management
│   ├── ShaderProgram.h/cpp       # OpenGL shader programs
│   ├── SFX.h/cpp                 # Sound effects and music
│   ├── Entity.h/cpp              # Base entity class
│   ├── GraphicsConfig.h           # Graphics initialization
│   ├── Key.h/cpp                 # Keyboard/input handling
│   └── AnimKeyframes.h            # Animation keyframe data
│
├── assets/                        # Game resources
│   ├── audio/                     # Music and sound files
│   │   └── main_menu.mp3
│   ├── textures/                  # Sprite sheets and textures
│   └── levels/                    # Level data files
│
├── libs/                          # External libraries
│   ├── glfw-3.3.8/               # Window and input management
│   ├── glm/                       # Math library (vectors, matrices)
│   └── SFML-3.0.2/               # Graphics/audio (partial use)
│
├── bin/                           # Compiled binaries
│   └── (exe and dll files)
│
├── dlls/                          # Runtime DLL dependencies
│
├── Castell Boig 3.sln            # Visual Studio solution file
├── Castell Boig 3.vcxproj        # Visual Studio project file
└── Castell Boig 3.vcxproj.user   # User-specific project settings
```

---

## Build Instructions

### Prerequisites
1. Ensure all external libraries are present in the `libs/` directory:
   - GLFW 3.3.8 (window and input handling)
   - GLM (mathematics library)
   - SFML 3.0.2 (graphics and audio)

2. Required DLLs should be in the `dlls/` directory for runtime linking.

### Building the Project

#### Using Visual Studio
1. Open `Castell Boig 3.sln` in Visual Studio
2. Select desired configuration:
   - **Debug**: For development with debugging symbols
   - **Release**: For optimized distribution build
3. Build the solution:
   - **Build → Build Solution** (Ctrl+Shift+B)
   - Or right-click solution → **Build Solution**
4. Compiled executable will be created in the `bin/` directory

#### Using Command Line (MSBuild)
```bash
cd "C:\Users\arnau\source\repos\mapsdos\Castell-Boig-3"
msbuild "Castell Boig 3.sln" /p:Configuration=Release /p:Platform=Win32
```

#### Using Command Line (Visual Studio Developer Command Prompt)
```bash
devenv "Castell Boig 3.sln" /build Release
```

---

## Running the Game

1. **From Visual Studio**:
   - Set startup project: Right-click `Castell Boig 3` → **Set as Startup Project**
   - Press **F5** (Debug) or **Ctrl+F5** (Run without debugging)

2. **From Windows Explorer**:
   - Navigate to `bin/` directory
   - Execute the `.exe` file

3. **Ensure assets are accessible**:
   - The executable must be run from the project root directory or have access to the `assets/` folder
   - Check that `assets/audio/main_menu.mp3` and other resources exist

---

## Game Controls

### Main Menu & Navigation
| Key | Action |
|-----|--------|
| **1** | Start Game (Play Level) |
| **2** | Instructions |
| **3** | Credits |
| **M** | Return to Main Menu |
| **ESC** | Exit Game |

### Gameplay
| Control | Action |
|---------|--------|
| **Arrow Keys** or **WASD** | Move player left/right |
| **SPACE** | Jump |
| **Mouse** | Interact (if implemented) |
| **M** | Return to Main Menu |
| **ESC** | Exit Game |

(Exact controls may vary - refer to the in-game instructions scene for details)

---

## Project Architecture

### Core Components

#### Game (Singleton Pattern)
- Central game manager handling all state transitions
- Manages scene lifecycle (Menu, Level, Instructions, Credits)
- Processes input (keyboard and mouse)
- Coordinates rendering and updates

#### Scene System
- **Base Scene Class**: Abstract interface for all game scenes
- **MenuScene**: Main menu UI
- **LevelScene**: Gameplay area with player and tilemap
- **CreditsScene**: Game credits
- **InstructionsScene**: Game instructions

#### Player & Entity System
- **Entity**: Base class for game objects
- **Player**: Inherits from Sprite, handles player physics and movement
- **TileMap**: Level geometry defined by tiles

#### Rendering System
- **Sprite**: 2D sprite rendering with animations
- **Texture**: Texture loading and management
- **ShaderProgram**: OpenGL shader compilation and linking
- **GraphicsConfig**: OpenGL initialization and configuration

#### Audio System
- **SFX**: Singleton audio manager for background music and sound effects

---

## Configuration

### Game Settings (src/Game.h)
```cpp
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define TARGET_FRAMERATE 60.0f  // (in main.cpp)
```

### Modify these values to change:
- Screen resolution
- Target frame rate

### Graphics Configuration (src/GraphicsConfig.h)
- OpenGL version and profile settings
- Window initialization parameters
- Viewport configuration

---

## Troubleshooting

### Build Issues

| Issue | Solution |
|-------|----------|
| **Missing include files (glfw, glm, SFML)** | Verify `libs/` directory contents; update include paths in project properties if needed |
| **Linker errors (unresolved external symbols)** | Check that library `.lib` files are correctly linked in project properties |
| **DLL not found at runtime** | Ensure all required DLLs are in `dlls/` directory or project output folder |
| **OpenGL context creation fails** | Update graphics drivers; verify OpenGL 3.3+ support |

### Runtime Issues

| Issue | Solution |
|-------|----------|
| **Black screen or no rendering** | Check `GraphicsConfig.h` OpenGL initialization; verify shaders compile correctly |
| **Assets not loading** | Run executable from project root directory; verify `assets/` path is correct |
| **No audio** | Check `assets/audio/` exists with required .mp3 files; verify SFML audio libraries are linked |
| **Input not responding** | Ensure GLFW window has focus; check key callback registration in `main.cpp` |

---

## Development Workflow

### Adding New Features

1. **New Game Scene**:
   - Create `.h` file inheriting from `Scene`
   - Create corresponding `.cpp` implementation
   - Add to `Game.h` as a member variable
   - Initialize in `Game::init()`
   - Add state transitions in `Game::keyPressed()`

2. **New Sprites/Entities**:
   - Create new class inheriting from `Sprite` or `Entity`
   - Add required textures to `assets/textures/`
   - Initialize and update in appropriate scene

3. **Level Design**:
   - Modify tilemap data in `LevelScene`
   - Create new `LevelScene` instances for additional levels
   - Update level progression logic in `Game::keyPressed()`

### Code Style Guidelines
- Use consistent naming conventions (camelCase for variables, PascalCase for classes)
- Follow RAII principles for resource management
- Use const correctness
- Keep single responsibility principle for classes
- Add comments for complex logic

---

## Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| GLFW | 3.3.8 | Window management and input handling |
| GLM | Latest | Mathematics (vectors, matrices, transformations) |
| SFML | 3.0.2 | Graphics and audio support |
| OpenGL | 3.3+ | Rendering API |

---

## Additional Resources

- **OpenGL Tutorials**: https://learnopengl.com/
- **GLFW Documentation**: https://www.glfw.org/documentation.html
- **GLM Documentation**: https://glm.g-truc.net/
- **SFML Documentation**: https://www.sfml-dev.org/documentation.php

---

## Support & Contact

For issues or questions regarding this project, refer to the GitHub repository:  
https://github.com/mapsdos/Castell-Boig-3

---

**Last Updated**: 2026-03-06  
**Project Status**: Active Development (main-menu branch)
