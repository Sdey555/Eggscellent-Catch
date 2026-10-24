# Eggscellent Catch

A retro arcade egg-catcher built with Qt and C++. The game uses a custom raster graphics pipeline, glowing pixel art, layered backgrounds, and a progressively harder 50-level challenge system.

## Features

- Woven basket movement using keyboard or mouse input
- Regular eggs, golden bonus eggs, rare growth eggs, legendary restoration eggs, and dangerous bombs
- 3-heart life system with instant game-over on bomb contact
- 50-level progression with increasing spawn pressure and basket narrowing
- Atmospheric retro scene with clouds, farmhouse detail, haze, and glowing foreground elements
- Pause, restart, and HUD-driven gameplay loop

## Controls

- Move left/right: `A` / `D` or `◀` / `▶`
- Move with mouse: horizontal pointer tracking
- Pause/resume: `Space` or right-click
- Restart: `R` or the in-game Restart button

## Gameplay Overview

- Regular eggs: +10 points, missing one costs 1 heart
- Golden eggs: +50 points, missing one costs 1 heart
- Growth eggs: +25 points; they expand the basket when needed and are safe to miss
- Restoration eggs: +100 points; they restore the basket to full width and are safe to miss
- Bombs: instantly end the run if caught
- Progression: levels scale up to 50, with more frequent drops and a shrinking basket over time

## Requirements

- Qt 6 (or Qt 5) with the `Widgets` module
- C++17 compiler
- qmake
- On Windows, a MinGW-w64 toolchain is recommended for building the portable package

## Build and Run

### 1. Clone the project

```bash
git clone https://github.com/Sdey555/Eggscellent-Catch.git
cd Eggscellent-Catch
```

### 2. Build from source

```bash
qmake DrawLine.pro
mingw32-make
```

The built executable is output to `release/EggscellentCatch.exe`.

### 3. Run the game

- Windows desktop build: run `release/EggscellentCatch.exe`
- Portable executable: run `EggscellentCatch-Portable.exe` from the project root after packaging

### 4. Create the portable Windows package

After a Release build is available, run:

```powershell
.\packaging\package-portable.ps1
```

This script packages the Qt runtime and app into a single portable `.exe` file. It requires a MinGW-w64 `g++` compiler on `PATH`.

## Project Structure

```text
src/
├── core/
│   ├── Pixel.h / .cpp
│   └── Grid.h / .cpp
├── entities/
│   ├── GameTypes.h
│   ├── Particle.h
│   ├── Basket.h / .cpp
│   ├── Bird.h / .cpp
│   └── FallingEgg.h / .cpp
├── game/
│   └── GameEngine.h / .cpp
├── rendering/
│   ├── PixelFont.h / .cpp
│   └── GameRenderer.h / .cpp
├── ui/
│   ├── CanvasLabel.h / .cpp
│   ├── MainWindow.h / .cpp
│   └── mainwindow.ui
├── main.cpp
└── ...
```

## Notes

This project combines a hand-authored raster rendering pipeline with a lightweight Qt UI, giving the game a classic arcade look while keeping the code organized by system: core math, entities, game logic, rendering, and UI.
