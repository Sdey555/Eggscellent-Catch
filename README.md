# Eggscellent Catch — Raster Grid Game

A retro arcade egg-catcher game built in Qt/C++ featuring a custom raster graphics pipeline, glowing pixels (with no grid lines), birds laying eggs with physics/gravity, golden bonus eggs, hazard bombs, and a 10-level progression system.

## Gameplay

- **Basket Control**: Move the basket with `A` / `D`, `◄` / `►`, or Mouse horizontal tracking.
- **Eggs**: Catch regular eggs (+10 pts) and glowing golden eggs (+50 pts).
- **Bombs**: Avoid false eggs (bombs with flickering fuses); catching one causes an instant Game Over!
- **Lives**: 3 hearts. Missing a falling egg loses 1 heart. Game over at 0 hearts.
- **Leveling**: 10 progressive levels (every 100 points). Higher levels feature faster falling eggs, higher bomb probabilities, and narrower baskets!
- **Pause / Restart**: Press `Space` or Right-Click to pause/resume; press `R` or click "Restart" to play again.

## Project Structure

The project is organized into clean, modular components inside `src/`:

```
src/
├── core/                  # Core raster graphics primitives
│   ├── Pixel.h / .cpp     # MyPix and MyPixels coordinate/color hash map
│   └── Grid.h / .cpp      # MyGrid math-to-screen coordinate transforms & panning
├── entities/              # Game entity models and sprite rasterization
│   ├── GameTypes.h        # Enums (GameState, EggType)
│   ├── Particle.h         # GameParticle and FloatingText definitions
│   ├── Basket.h / .cpp    # Basket model & wicker basket rasterizer
│   ├── Bird.h / .cpp      # Animated bird model & bird rasterizer
│   └── FallingEgg.h / .cpp# Falling egg physics model & egg/bomb rasterizers
├── game/                  # Game loop rules, physics simulation, difficulty
│   └── GameEngine.h / .cpp# State machine, swept collisions, gravity, particle logic
├── rendering/             # Multi-pass raster rendering and visual effects
│   ├── PixelFont.h / .cpp # 3x5 retro bitmap pixel font
│   └── GameRenderer.h/.cpp# Multi-pass glowing raster engine, HUD & overlays
├── ui/                    # Qt UI layer
│   ├── CanvasLabel.h/.cpp # Interactive canvas widget with mouse tracking & panning
│   ├── MainWindow.h/.cpp  # Slender UI coordinator & HUD updates
│   └── mainwindow.ui      # Qt Designer form layout
└── main.cpp               # Application entry point
```

## Requirements

- Qt 6 (or Qt 5) with `Widgets` module
- C++17 compiler (GCC/MinGW, Clang, or MSVC)
- qmake

## Build and Run

### Run on Windows
Launch `EggscellentCatch.exe` directly in the project root. (Required Qt DLLs and platform plugins are already bundled in the root directory).

### Build from Source (MinGW / Command Line)
```bash
qmake DrawLine.pro
mingw32-make
```

The compiled binary will be placed in `release/EggscellentCatch.exe`.