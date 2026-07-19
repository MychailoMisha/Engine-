# 🎮 Engine Plus Plus

> A custom 2D game engine and editor based on real **C++20** and **Qt 6**.

Engine Plus Plus allows you to create scenes in a visual editor, place nodes and control the game via C++ scripts. Changes in the code directly affect the graphics, physics, interface and behavior of objects.

## ✨ Features

- creation of multiple scenes, nodes and layers;
- Inspector for position, size, color, transparency and effects;
- C++ scripts for game logic and graphics control;
- characters, physics, gravity, collisions, hitboxes and platforms;
- adaptive camera, different screen proportions and resolutions;
- Grid Map and Tile Map with palette and cell drawing;
- buttons, panels, containers, lists, Slider and Progress Bar;
- Toggle, Check Box, Radio Button and Scroll View;
- images, custom fonts, videos, music and sound effects;
- transparency, blur, glass effect and rounding of elements;
- keyboard, mouse or touch control on Android;
- saving and restoring game progress;
- Android vibration via node or C++ code;
- name and custom icon of the finished game;
- export for **Windows** and creation of **Android APK** installation;
- generation of CMake projects for **Linux** and **macOS**.

## 🚀 Quick launch of the editor

1. Download or unpack the entire Engine Plus Plus folder.
2. Do not move the `bin` and `runtime` folders separately.
3. Run **`Start Engine Plus Plus.bat`**.

To normally launch the finished editor on **Windows 10/11 x64, nothing additional needs to be installed**. The required Qt DLLs, multimedia modules, FFmpeg and plugins are already in the `runtime` folder.

> Do not run `bin/EnginePlusPlus.exe` separately from the package: the editor needs the `runtime` folder.

## 🛠️ What you need to compile games

### Windows Run and Windows Export

The editor generates and compiles real C++, so for the **Run** button and to create a Windows game you need to install:

- [MSYS2](https://www.msys2.org/) in `C:\msys64`;

- GCC `g++` for the UCRT64 environment;

- Qt 6 for UCRT64, including Widgets and Multimedia.

The compiler is expected at the path:

```text
C:\msys64\ucrt64\bin\g++.exe
```

In the **MSYS2 UCRT64** console, the required packages can be installed as follows:

```bash
pacman -Syu
pacman -S --needed mingw-w64-ucrt-x86_64-toolchain \
mingw-w64-ucrt-x86_64-qt6-base \
mingw-w64-ucrt-x86_64-qt6-multimedia
```

### Android APK Export

For Android export, you need:

- Qt **6.9.1**: `mingw_64` and `android_arm64_v8a`;
- Android Studio with Android SDK;
- Android SDK Platform **API 35**;
- Android NDK **27.2.12479018 (r27c)**;
- CMake and Ninja from the Qt package;
- Java/JBR from Android Studio;
- debug keystore in `%USERPROFILE%\.android\debug.keystore`.

The current version looks for these components in the standard paths `C:\Qt` and `%LOCALAPPDATA%\Android\Sdk`. The export is done in the background: the `.so` file is an intermediate one, and the final signed `.apk` is created after the Gradle packaging stage.

### Linux and macOS

The exporter prepares a CMake project. To build it on the appropriate system, you need:

- CMake 3.21 or later;
- a compiler with C++20 support;
- Qt 6 Widgets and Qt 6 Multimedia;
- Ninja or another build system supported by CMake.

## 🧩 Example of a C++ script

```cpp
void onStart(Scene& scene) { 
if (auto* player = scene.findType("Character")) { 
player->velocityX = 90.0f; 
}
}

void onUpdate(Scene& scene, float dt) { 
if (auto* player = scene.findType("Character")) { 
if (scene.input.left) player->x -= 180.0f * dt; 
if (scene.input.right) player->x += 180.0f * dt; 
}
}
```

## 📁 Build Structure

```text
Engine Plus Plus/
├── bin/ # Ready-made editor
├── runtime/ # Qt, FFmpeg, DLLs and plugins
├── scripts/ # C++ script examples
├── source/ # Editor source code
├── Start Engine Plus Plus.bat # Recommended launch
└── README.md
```

The `generated_game.cpp` file is the temporary code of the current game that the editor creates from the scene and connected scripts before launching or exporting.

## 💾 Projects

Scenes, nodes, Inspector settings and paths to resources are stored in the project file. It is recommended to save each game in a separate folder and not delete its `assets` and `scripts` directories.

---

**Engine Plus Plus** - Create 2D games visually and control every detail with real C++.
