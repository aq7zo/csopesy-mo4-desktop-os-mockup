<div align="center">

# CSOPESY OS

**CSOPESY MO4: a desktop-style OS mock-up built with Dear ImGui**

![C++20](https://img.shields.io/badge/C%2B%2B-20-00599C?logo=cplusplus&logoColor=white)
![Dear ImGui](https://img.shields.io/badge/Dear%20ImGui-1.92.9b-1f6feb)
![GLFW](https://img.shields.io/badge/GLFW-3.4-ff6600)
![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6?logo=windows&logoColor=white)
![Toolchain](https://img.shields.io/badge/MSVC-VS%202026-5C2D91?logo=visualstudio&logoColor=white)

[Features](#features) • [Getting started](#getting-started) • [Make targets](#make-targets) • [Configuration](#configuration) • [Project structure](#project-structure)

![CSOPESY OS desktop with Task Manager, File Explorer and About This PC open](docs/screenshot.png)

</div>

Windows, macOS and Android all have a compositor that draws the desktop and manages windows every frame. CSOPESY OS has its own compositor too, built on **GLFW + OpenGL 3 + Dear ImGui**. It isn't a working OS. It's a real-time graphical shell that shows how to build a desktop in immediate-mode UI.

## Features

- **Desktop:** a full-window gradient or solid wallpaper with an optional pattern (dots, grid, stripes or checker), drawn first every frame using ImGui draw commands.
- **Live clock:** the current time and date, refreshed every frame, pinned to a corner (top-right by default), in 12- or 24-hour format.
- **PWR button:** opens a shutdown dialog and is the *only* way to close the app. Alt+F4, the window's X button and the Windows "Close window" command are ignored.
- **Taskbar:** a fixed panel at the bottom (or top) with three custom-drawn icon buttons. Each open app gets a running indicator under its icon, and the taskbar always stays on top.
- **About This PC:** a placeholder spec sheet with storage usage bars, plus the config file that was loaded.
- **File Explorer:** a navigation pane, a placeholder file list and a status bar.
- **Task Manager:** modelled on the Windows one. It has a **Processes** tab, CPU and memory totals in the column headers, and cells shaded yellow by load. **Apps** lists the apps that are actually open, and **Background processes** lists the dummy processes from `config.txt`. Click a header to sort; **End task** closes an app or hides a dummy process.
- **Parameters without recompiling:** everything above that can vary is read from [`config.txt`](config.txt) at startup (see [Configuration](#configuration)), as the quiz rules require.
- **High-DPI aware:** sizes are based on the font size, so the shell scales with Windows display scaling, and `ui-scale` can enlarge it further.

## Getting started

### Prerequisites

| Tool | Notes |
|---|---|
| [Visual Studio 2026](https://visualstudio.microsoft.com/) (Community or Build Tools) | Install the **Desktop development with C++** workload (MSVC x64 + Windows SDK). |
| [GNU make](https://community.chocolatey.org/packages/make) | For example `choco install make`. |
| [VS Code](https://code.visualstudio.com/) + the [C/C++ extension](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools) | Optional. Lets you build with Ctrl+Shift+B and debug with F5. |

Everything else ships with the repo: Dear ImGui sources and the prebuilt GLFW library are in `vendor/`.

### Build and run

```bash
git clone https://github.com/aq7zo/csopesy-mo4-desktop-os-mockup.git
```

```bash
cd csopesy-mo4-desktop-os-mockup
```

```bash
tools\vsdev.cmd make run
```

`tools\vsdev.cmd` finds your Visual Studio install, loads the MSVC x64 environment and then runs the command you give it. That means `make` works from any terminal: PowerShell, cmd or the VS Code terminal.

> [!TIP]
> If you're already in a **Developer PowerShell for VS 2026** or **x64 Native Tools Command Prompt**, you can skip the wrapper and run `make` directly.

> [!IMPORTANT]
> The app opens full-screen (unless `config.txt` says `fullscreen false`) and can only be closed with the red **PWR** button, top-left by default. If it ever stops responding, end `csopesy.exe` from the real Windows Task Manager (Ctrl+Shift+Esc).

## Make targets

| Command | What it does |
|---|---|
| `make` | Release build → `build/release/csopesy.exe` |
| `make run` | Builds the release version if needed, then launches it |
| `make debug` | Debug build with symbols → `build/debug/csopesy.exe` |
| `make run CONFIG=debug` | Builds and runs the debug version |
| `make clean` | Deletes the `build/` folder |
| `make rebuild` | Clean, then a full release build |

Prefix each command with `tools\vsdev.cmd` unless you're already in a VS developer shell, for example `tools\vsdev.cmd make rebuild`.

> [!NOTE]
> The debug build opens a console window, and pressing Ctrl+C there ends the app. **Use the release build for demos**, so PWR really is the only way out.

### Building from VS Code

The workspace already includes the tasks and launch settings. The tasks call `build.cmd` (below), so they don't need make, and they skip the build when `src\` hasn't changed:

- **Ctrl+Shift+B** runs a release build (`build` task).
- **F5** builds if needed and starts the configuration chosen in Run and Debug. **Run CSOPESY (release)** has no console window, so use it for demos and quiz videos. **Debug CSOPESY** runs the debug build in the debugger.
- **Terminal → Run Task** has `build debug`, `run`, `clean` and `rebuild`.

### Building without make

`build.cmd` does the same build with plain `cl` and `link`, so you don't need GNU make. Like `tools\vsdev.cmd`, it finds Visual Studio by itself. In PowerShell, type `.\build` instead of `build`.

| Command | What it does |
|---|---|
| `build` | Release build → `build\release\csopesy.exe` |
| `build run` | Release build, then launches it |
| `build debug` | Debug build with a console window and symbols → `build\debug\csopesy.exe` |
| `build debug run` | Debug build, then launches it |
| `build clean` | Deletes the `build\` folder. Add other options to rebuild, for example `build clean run` |

Like `make`, it only rebuilds when something in `src\` has changed since the last build. Dear ImGui is compiled once per config, so a rebuild takes a few seconds.

> [!WARNING]
> If the build succeeds but launching fails with **Access is denied**, your antivirus is blocking the newly built, unsigned `csopesy.exe`. Endpoint products with zero-trust application control, such as WatchGuard Endpoint Security, block each new build until their cloud scan clears it. In testing, that took between 1 and 20 minutes. Wait and try again. A cleared exe keeps working, and `build run` won't replace it unless `src\` changes. To avoid the wait, ask whoever manages the antivirus to allow the project's `build\` folder.

## Configuration

The quiz rules say to change only the parameters and not recompile, so every value that can vary is read from [`config.txt`](config.txt) when the program starts. Edit a value, save, and run again. Each line is `key value`, and `#` starts a comment. You can also pass another file as the first argument: `csopesy.exe my-test.txt`.

| Key | Values (default first) |
|---|---|
| `fullscreen` | `true` \| `false` (a resizable window) |
| `window-width`, `window-height` | pixels (1280, 720), used when not full screen |
| `ui-scale` | 0.5 to 3.0 (1.0), size of text and UI |
| `wallpaper-style` | `gradient` \| `solid` |
| `wallpaper-color`, `wallpaper-color-2` | `#RRGGBB`, `R G B` or a colour name (`#162446` navy, `#562662` purple) |
| `wallpaper-direction` | `vertical` \| `horizontal` |
| `wallpaper-pattern` | `dots` \| `none` \| `grid` \| `stripes` \| `checker` |
| `clock-corner`, `power-corner` | `top-right`, `top-left`, `bottom-left`, `bottom-right` (clock top-right, PWR top-left) |
| `clock-format` | `12` \| `24` |
| `taskbar-position` | `bottom` \| `top` |
| `total-memory-mb` | 16384, the base for the Task Manager Memory % |
| `process "Name" cpu mem [disk] [net] [app\|background\|suspended]` | one Task Manager row; the first one replaces the built-in list |

A mistake never crashes the program. A **Config warnings** window lists each bad line, and that setting keeps its default.

## Project structure

```
csopesy-mo4-desktop-os-mockup/
├─ src/
│  ├─ main.cpp               # GLFW/OpenGL/ImGui setup and the compositor frame loop
│  ├─ shell.h                # ShellState, AppWindow, Draw* declarations
│  ├─ config.h / config.cpp  # reads config.txt into Config (quiz parameters)
│  ├─ desktop.cpp            # wallpaper, clock, PWR button and shutdown dialog
│  ├─ taskbar.cpp            # taskbar, icon buttons, running-app indicators
│  └─ apps/
│     ├─ about_pc.cpp        # About This PC
│     ├─ file_explorer.cpp   # File Explorer
│     └─ task_manager.cpp    # Task Manager (dummy process data)
├─ vendor/                   # Dear ImGui 1.92.9b sources, GLFW 3.4 prebuilt lib
├─ tools/vsdev.cmd           # loads the MSVC environment, then runs a command
├─ docs/                     # implementation plan and requirements checklist
├─ .vscode/                  # build tasks, debugger launch config, IntelliSense
├─ build.cmd                 # build without make: build [debug] [run] [clean]
├─ config.txt                # runtime parameters (edit these, not the code)
├─ README.txt                # submission readme: members, how to run, entry point
└─ Makefile
```

Each frame draws three layers in order: **desktop**, then **app windows**, then **taskbar**. Every app is one `.cpp` file that exposes a `Draw<App>(bool* open)` function and a `Draw<App>Icon(...)` function, registered in `ShellState::apps` in [`src/shell.h`](src/shell.h).

## Documentation

- [`docs/PLAN.md`](docs/PLAN.md): implementation plan, naming conventions and notes on where the code differs from the plan.
- [`docs/CHECKLIST.md`](docs/CHECKLIST.md): every spec requirement, mapped to the code that implements it and how it was verified.
