# Eggscellent Catch — Raster Grid Game

A retro arcade egg-catcher game built in Qt/C++ featuring a custom raster graphics pipeline, glowing pixels (with no grid lines), birds laying eggs with physics/gravity, golden bonus eggs, basket growth eggs, legendary restoration eggs, hazard bombs, and a 50-level progression system.

## Gameplay

- **Basket Control**: Move the enlarged woven wicker basket with `A` / `D`, `◄` / `►`, or Mouse horizontal tracking.
- **Regular Eggs**: Standard white eggs (+10 pts; miss = -1 heart).
- **Golden Eggs**: Glistening bonus eggs (+50 pts; miss = -1 heart).
- **Growth Eggs (Rare)**: Emerald eggs with pulsing expansion markers (+25 pts). When the basket shrinks to a small size in higher levels, this egg spawns to widen the basket (+2 width). Missing it is safe (no heart lost).
- **Restoration Eggs (Legendary & Very Rare)**: Radiant prismatic diamond crown eggs (+100 pts) that appear periodically after every 10–12 levels whenever the basket has shrunk. Catching one instantly restores your basket to its full original starting width (`halfWidth = 9`)! Missing it is safe (no heart lost).
- **Bombs**: Avoid false eggs (bombs with flickering fuses); catching one causes an instant Game Over!
- **Lives**: 3 hearts. Missing a falling egg loses 1 heart. Game over at 0 hearts.
- **Progression (Levels 1–50)**:
  - **Level Cap**: Scales all the way to **Level 50** with progressive difficulty.
  - **Dynamic Egg Frequency**: Egg drop frequency increases every 3 levels.
  - **Basket Sizing**: Basket gradually narrows across the levels from starting width 19 down to minimum width 9.
- **Perspective Pixel Environment**:
  - 100% pixelated world with authentic atmospheric aerial perspective.
  - **Brighter & Hazier Sky**: Stepped pastel daylight sky bands and gentle horizon haze.
  - **Grand Billowing Clouds**: Large, puffy pixel cumulus clouds drifting across the sky.
  - **Perspective View of the Farmhouse**: Set back in the midground on an elevated grassy knoll/plateau (higher than the foreground basket, but below the distant mountain ridges), with a winding dirt path, rustic barn, metal silo, smoking cobblestone chimney, and split-rail fence.
  - **Foreground Highlight**: High-contrast, vibrant, glowing foreground (birds, basket, falling eggs, and lush grass floor) highlighted clearly against the soft, hazy background.
- **Pause / Restart**: Press `Space` or Right-Click to pause/resume; press `R` or click "Restart" to play again.

## Project Structure

The project is organized into clean, modular components inside `src/`:

```
src/
├── core/                  # Core raster graphics primitives
│   ├── Pixel.h / .cpp     # MyPix and MyPixels coordinate/color hash map
│   └── Grid.h / .cpp      # MyGrid math-to-screen coordinate transforms & panning
├── entities/              # Game entity models and sprite rasterization
│   ├── GameTypes.h        # Enums (GameState, EggType: REGULAR, GOLDEN, BOMB, BASKET_GROW, BASKET_RESTORE)
│   ├── Particle.h         # GameParticle and FloatingText definitions
│   ├── Basket.h / .cpp    # Enlarged basket model & wicker basket rasterizer
│   ├── Bird.h / .cpp      # Enlarged 15x9 animated bird model (3 wing flap frames)
│   └── FallingEgg.h / .cpp# Falling egg physics & regular/golden/growth/restore/bomb rasterizers
├── game/                  # Game loop rules, physics simulation, difficulty
│   └── GameEngine.h / .cpp# State machine, 50-level scaling, 3-level frequency tiers, swept collisions
├── rendering/             # Multi-pass raster rendering and visual effects
│   ├── PixelFont.h / .cpp # 3x5 retro bitmap pixel font
│   └── GameRenderer.h/.cpp# Atmospheric hazy sky, grand clouds, perspective knoll farmhouse + glowing aura passes
├── ui/                    # Qt UI layer
│   ├── CanvasLabel.h/.cpp # Interactive canvas widget with mouse tracking & panning
│   ├── MainWindow.h/.cpp  # Slender UI coordinator & HUD updates
│   └── mainwindow.ui      # Clean Qt Designer form layout
└── main.cpp               # Application entry point
```

## Requirements

- Qt 6 (or Qt 5) with `Widgets` module
- C++17 compiler (GCC/MinGW, Clang, or MSVC)
- qmake

## Build and Run

### Run on Windows
Launch `EggscellentCatch-Portable.exe`. It is a single-file portable package that unpacks the game and its Qt runtime into a temporary folder, runs the game, and cleans up afterward. No Qt installation is needed. The package is for 64-bit Windows and requires Windows PowerShell (included with Windows).

To recreate the portable package after building the release version, run:

```powershell
.\packaging\package-portable.ps1
```

The script requires a MinGW-w64 `g++` compiler on `PATH`. You can pass its path with `-Compiler` if needed. The regular Qt release executable and its runtime files are also available in `release/`.

### Build from Source (MinGW / Command Line)
```bash
qmake DrawLine.pro
mingw32-make
```

The compiled binary will be placed in `release/EggscellentCatch.exe`.