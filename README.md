# Eggscellent Catch

Eggscellent Catch is a Qt/C++ arcade game built around a custom pixel-raster graphics pipeline. Catch eggs in a woven basket, avoid bird droppings, and progress through increasingly difficult levels.

## Features

- Three difficulty modes: Easy, Medium, and Hard
- Up to 50 levels; levels 1 and 2 are short (50 and 70 points), later levels need 100 points each and increases the challenge
- Moving birds, falling eggs, animated pixel art, particle effects, and a score/high-score HUD
- Day/night cycle, occasional thunderstorms, lightning, and sticky ground puddles
- **Boss levels** at level 3 and then every 10 levels (5 unique bosses) with volley attack patterns, a rage phase, and a trophy-egg reward
- **Gameplay weather**: crosswinds that push eggs, rain that makes the basket slippery, and night fog that hides distant eggs
- **Hazard patches**: sticky slime, slowing mud, and slippery ice that form on the ground
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
| Trophy egg | Dropped by a defeated boss; catch it for a big bonus; missing it costs nothing |

Growth eggs appear when the basket has become small. Press `H` to use one and widen the basket. In Hard mode, press `C` to spend two growth eggs and summon a temporary companion basket. Restoration eggs may appear after the basket has narrowed.

### Boss levels

Level 3 is a first boss fight, and then so is every 10th level (10, 20, 30, 40, 50; the level-50 finale rematches the level-40 boss). Level progress freezes, the normal egg flow stops, and a boss bird hovers above the field and unleashes volleys of droppings with one real egg hidden among them (fan, burst, sweep and rain patterns, getting harder per tier). Catch the real egg to damage the boss; catching a dropping costs a heart. Below 40% health the boss enters a rage phase with faster, denser volleys. Boss health scales with difficulty (Easy 0.85x, Medium 1.0x, Hard 1.2x). Defeating a boss banks a bonus (150 + 50 per tier) and drops a trophy egg. Every boss egg is always reachable by the basket.

### Weather that matters

- **Wind**: a warning arrow shows the incoming direction, then a gust pushes falling eggs sideways (they bounce off the walls).
- **Rain**: wet ground makes the basket slide with momentum, so plan your stops.
- **Fog**: at night, fog thickens and hides distant eggs until they get close.
- **Cloud banks**: birds glide into drifting clouds and disappear inside; eggs drop directly from the cloud's underside with day/sunset/night puff wisps.
- **Fog dive at night**: birds dip into the night fog and emerge as shadowy silhouettes with a faint glowing eye glint so players can track their flight.

### Hazard patches

Patches form with a short animation and melt away after a while. **Slime** is sticky and slows you, **mud** slows you more, and **ice** makes the basket slide. A badge appears on the HUD while you are standing on one. Birds' droppings can leave patches too.

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

The settings screen also provides touch, virtual-button, and gyroscope control modes for supported devices. Choose a control mode there to match your device; gyroscope control requires a compatible motion sensor.

## Requirements

### Desktop

- Qt 6 (or Qt 5) with the **Widgets** and **Multimedia** modules
- C++17 compiler and qmake

### Android

- Qt 6 for Android with an **arm64-v8a** kit
- Android SDK, NDK, and JDK configured in Qt Creator

### Windows portable package

- 64-bit MinGW-w64 `g++`
- Windows PowerShell

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

### Build for Android

In Qt Creator, open `DrawLine.pro`, select a configured Qt 6 Android kit, and build and deploy the application to an Android device or emulator. The project is configured for the `arm64-v8a` ABI. Touch, virtual-button, and gyroscope controls can be selected in the in-game settings; gyroscope control requires a device with a compatible motion sensor.

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
├── entities/   # Basket, birds, bosses (Boss.cpp/h), falling eggs, and shared game types
├── game/       # Game state, rules, physics, boss fights, weather, and hazard patches
├── rendering/  # Pixel font and scene/game rendering
├── audio/      # Music and sound-effect playback
├── ui/         # Qt window, interactive canvas, and UI form
└── main.cpp    # Application entry point

audio/          # Music and sound-effect assets (regenerate the synthesized SFX with `python3 generate_audio.py`)
audio.qrc       # Qt audio resource collection
packaging/      # Windows portable-app launcher and packaging script
DrawLine.pro    # qmake project
```

## Contributors

- Shuvadip Dey
- Sanvi Bhowmik
- Akshay Gupta
- Toufik Mia
