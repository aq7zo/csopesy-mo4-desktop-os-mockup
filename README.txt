CSOPESY MO4 (Semi-Major Output 2) - Desktop-Style OS Mock-up: "CSOPESY OS"
===========================================================================

Group members
-------------
  ANDREA GAYLE GARCIA
  BENETTE ENZO CAMPO
  JACK OWEN MORGAN
  JEROEN RALPH TENORIO

GitHub: https://github.com/aq7zo/csopesy-mo4-desktop-os-mockup

Entry point
-----------
  The main function is in src/main.cpp  ->  int main(int argc, char** argv)
  It loads config.txt, opens the window, then runs the compositor loop, which
  draws the Desktop, then the app windows, then the Taskbar, every frame.

What you need
-------------
  - Windows 10 or 11 (x64)
  - Visual Studio Build Tools 2022 or newer (or Visual Studio Community) with
    the "Desktop development with C++" workload
  - VS Code with the "C/C++" extension (ms-vscode.cpptools), to run from the IDE
  Dear ImGui and GLFW are already in the vendor/ folder. Nothing else to install.

How to run
----------
  From VS Code:
    1. File > Open Folder... and choose this folder.
    2. Open Run and Debug (Ctrl+Shift+D) and choose "Run CSOPESY (release)".
    3. Press F5. It builds the program if the code changed, then starts it.
    "Debug CSOPESY" does the same with a console window for error messages.

  From a terminal in this folder (in PowerShell, type .\build instead of build):
    build run          build if needed, then run the release version
    build debug run    the same for the debug version (console window)
    build clean        delete the build folder
  GNU make also works: tools\vsdev.cmd make run

How to close it
---------------
  Click the red PWR button, then "Shut down". This is the only way out:
  Alt+F4 and the window's X button are ignored on purpose.

Parameters (config.txt)
-----------------------
  config.txt in this folder holds every parameter. Edit a value, save, and run
  again. No recompiling. Each line is "key value", and # starts a comment.
  If a value is wrong, a "Config warnings" window names the line, and that
  setting keeps its default. If a setting appears twice, the last valid line
  is used, and the window says so.

  Key                  Values (default first)
  fullscreen           true | false
  window-width         pixels (1280), used when fullscreen is false
  window-height        pixels (720)
  ui-scale             0.5 to 3.0 (1.0), size of text and UI
  wallpaper-style      gradient | solid
  wallpaper-color      #RRGGBB, R G B, or a colour name (default #162446)
  wallpaper-color-2    gradient end colour (default #562662)
  wallpaper-direction  vertical | horizontal
  wallpaper-pattern    dots | none | grid | stripes | checker
  clock-corner         top-right | top-left | bottom-left | bottom-right
  clock-format         12 | 24
  power-corner         top-left | top-right | bottom-left | bottom-right
  taskbar-position     bottom | top
  total-memory-mb      16384, the base for the Task Manager Memory %
  process "Name" CPU% Memory-MB [Disk-MB/s] [Network-Mbps] [app|background|suspended]
                       one Task Manager row (dummy values); repeat for more rows.
                       The first process line replaces the built-in list.
                       An open taskbar app with no process line shows 0.

  Example: solid red wallpaper, 24-hour clock bottom-left, taskbar on top:
    wallpaper-style   solid
    wallpaper-color   red
    clock-corner      bottom-left
    clock-format      24
    taskbar-position  top

Using the shell
---------------
  Taskbar icons, left to right: About This PC, File Explorer, Task Manager.
  A bar under an icon means that app is open. Task Manager lists the open apps
  under "Apps" and the dummy processes from config.txt under "Background
  processes". Click a column header to sort. Select a row and press "End task"
  to close that app or hide that dummy process.

Antivirus note
--------------
  Some antivirus products (for example WatchGuard Endpoint Security) block a
  newly built csopesy.exe for a few minutes ("Access is denied") until their
  cloud scan finishes. Wait and run again. Editing config.txt never rebuilds
  the program, so it never triggers this.
