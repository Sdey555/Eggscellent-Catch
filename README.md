# Eggscellent Catch — Raster Grid Game

A Qt-based arcade game where the player controls a basket and catches falling eggs while avoiding bombs. The game uses a custom raster/grid rendering style, smooth animation, score tracking, and simple arcade progression.

## Gameplay

- Move the basket with A/D or the left/right arrow keys.
- Catch regular eggs to score points.
- Catch golden eggs for a larger bonus.
- Avoid bombs; touching a bomb ends the run immediately.
- Missing too many eggs causes game over.
- Use Space to pause or resume, and R to restart.

## Features

- Score and best-score tracking
- Three-heart life system
- Pause and restart controls
- Toggleable grid lines and adjustable render scale
- Animated birds and particle effects for catches and misses
- Qt-based UI with custom raster-style rendering

## Controls

- A / D or Left / Right: Move basket
- Space: Pause / Resume
- R: Restart

## Project files

- `DrawLine.pro` — Qt project configuration
- `mainwindow.cpp` — game loop, rendering, and gameplay logic
- `mainwindow.h` — core game structures and class definitions
- `mainwindow.ui` — application layout and controls
- `my_label.cpp` / `my_label.h` — custom label event handling

## Requirements

- Qt 5 or Qt 6 with Widgets module
- C++17 compiler
- qmake

## Build and run

From the project root:

```bash
qmake DrawLine.pro
make
```

Then run the generated executable from the build directory (typically named after the project, such as `DrawLine`).

On Windows, open the project in Qt Creator or build it with MinGW:

```bash
qmake DrawLine.pro
mingw32-make
```

Then run the generated executable from the build directory.

## Notes

This project is structured as a small desktop game demo and can be used as a learning example for Qt widgets, custom rendering, object animation, and simple game state management.