# Eggscellent Catch

Eggscellent Catch is a Qt/C++ arcade game built around a custom pixel-raster graphics pipeline. Catch eggs in a woven basket, avoid bird droppings, and progress through increasingly difficult levels.

## Features

- Three difficulty modes: Easy, Medium, and Hard
- Up to 50 levels; each level requires 100 points and increases the challenge
- Moving birds, falling eggs, animated pixel art, particle effects, and a score/high-score HUD
- Day/night cycle, occasional thunderstorms, lightning, and sticky ground puddles
- Background music and sound effects, with audio settings
- Mouse, keyboard, touch, virtual-button, and gyroscope control modes
- Special growth eggs and a temporary companion basket in Hard mode

## Gameplay

You start with three hearts. Catch regular and golden eggs to score points, and avoid catching bird droppings, which cost a heart. Missing a dropping is safe; missing a regular or golden egg costs a heart.

| Item | Effect |
| --- | --- |
| Regular egg | +10 points; missing it costs one heart |
| Golden egg | +50 points; missing it costs one heart |
| Growth egg | +25 points and adds one growth egg to your inventory; safe to miss |
| Restoration egg | +100 points and restores the basket to its starting width; safe to miss |
| Bird dropping | Catching it costs one heart; safe to miss |

Growth eggs appear when the basket has become small. Press `H` to use one and widen the basket. In Hard mode, press `C` to spend two growth eggs and summon a temporary companion basket. Restoration eggs may appear after the basket has narrowed.

## Controls

### Desktop

- Move: `A` / `D` or `Left Arrow` / `Right Arrow`
- Mouse movement: move the basket horizontally
- Pause or resume: `Esc`; `Space` or right-click also resumes from the pause screen
- Restart: `R`
- Use a growth egg: `H`
- Summon a companion basket (Hard mode, costs two growth eggs): `C`
- Mute/unmute audio: `M`

On the start screen, select Easy/Medium/Hard with `1` / `2` / `3`, cycle modes with `Tab`, and start with `Space` or `Enter`.

The settings screen also provides touch, virtual-button, and gyroscope control modes for supported devices.

## Requirements

- Qt 6 (or Qt 5) with the **Widgets** and **Multimedia** modules
- C++17 compiler
- qmake
- For the Windows portable package: 64-bit MinGW-w64 `g++` and Windows PowerShell

## Build and Run

### Get the source

```bash
git clone https://github.com/Sdey555/Eggscellent-Catch.git
cd Eggscellent-Catch
```

### Build with Qt Creator

Open `DrawLine.pro` in Qt Creator, select a desktop kit matching the installed Qt version and compiler, choose a **Release** build, and build the project. The executable is produced in the selected build directory.

### Build from a MinGW command prompt

Run these commands from the project root in a Qt command prompt configured for the matching MinGW kit:

```powershell
qmake DrawLine.pro "CONFIG+=release" "CONFIG-=debug"
mingw32-make
```

Run the resulting `EggscellentCatch.exe`. Its location depends on the build directory; if you want to create the portable package, make sure the Release executable and its deployed Qt runtime files are in `release\`.

### Create the portable Windows executable

With `release\EggscellentCatch.exe` built, deploy the matching Qt runtime into `release\` if it is not already there:

```powershell
windeployqt release\EggscellentCatch.exe
```

Then create the single-file portable package:

```powershell
.\packaging\package-portable.ps1
```

The script writes `EggscellentCatch-Portable.exe` to the project root. It packages the app and runtime files from `release\`; the portable app extracts them to a temporary directory when launched.

## Project Structure

```text
src/
├── core/       # Pixel/grid primitives and motion-sensor support
├── entities/   # Basket, birds, falling eggs, and shared game types
├── game/       # Game state, rules, physics, difficulty, and weather
├── rendering/  # Pixel font and scene/game rendering
├── audio/      # Music and sound-effect playback
├── ui/         # Qt window, interactive canvas, and UI form
└── main.cpp    # Application entry point

audio/          # Music and sound-effect assets
audio.qrc       # Qt audio resource collection
packaging/      # Windows portable-app launcher and packaging script
DrawLine.pro    # qmake project
```

## Contributors

- Shuvadip Dey
- Sanvi Bhowmik
- Akshay Gupta
- Toufik Mia
