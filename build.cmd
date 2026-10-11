@echo off
rem CSOPESY OS mockup: build without GNU make (same flags as the Makefile).
rem Usage:  build [debug] [run] [clean]      (PowerShell: .\build ...)
rem   build              release build  -> build\release\csopesy.exe
rem   build run          release build, then launch it
rem   build debug        debug build (console window + symbols) -> build\debug\csopesy.exe
rem   build debug run    debug build, then launch it
rem   build clean        delete the build folder (add other options to rebuild)
setlocal
rem Remember this script's folder now: "shift" below also moves the script-name parameter.
set "ROOT=%~dp0"
cd /d "%ROOT%"

set "CONFIG=release"
set "RUN="
set "CLEAN="
set "BUILD="
:args
if "%~1"=="" goto args_done
if /i "%~1"=="release" (set "CONFIG=release") else if /i "%~1"=="debug" (set "CONFIG=debug") else if /i "%~1"=="run" (set "RUN=1") else if /i "%~1"=="clean" (set "CLEAN=1") else (echo [build] unknown option "%~1". Usage: build [debug] [run] [clean] & exit /b 1)
if /i not "%~1"=="clean" set "BUILD=1"
shift
goto args
:args_done

if defined CLEAN if exist build (
  rmdir /s /q build || exit /b 1
  echo [build] deleted build\
)
if defined CLEAN if not defined BUILD exit /b 0

set "OUT=build\%CONFIG%"
set "OBJ=%OUT%\obj"
rem Skip the build when src\ still matches the copy saved by the last successful build (like make).
rem Each relink makes a new exe, and endpoint antivirus may block a new exe until it has checked it.
rem After changing flags in this file, run "build clean".
if not exist "%OUT%\csopesy.exe" goto needs_build
robocopy src "%OBJ%\src-built" /MIR /L /NJH /NJS /NFL /NDL /NP >nul
if not errorlevel 1 (echo [build] %OUT%\csopesy.exe is up to date & goto run)
:needs_build

rem Load the MSVC x64 environment unless cl.exe is already on PATH.
where cl >nul 2>nul || call tools\vsdev.cmd rem
cd /d "%ROOT%"
where cl >nul 2>nul || (echo [build] cl.exe not found. Install Visual Studio Build Tools with "Desktop development with C++". & exit /b 1)
rem Use the link.exe beside cl.exe: Git's usr\bin also has a "link" that can come first on PATH.
set "MSVC_BIN="
for /f "delims=" %%i in ('where cl') do if not defined MSVC_BIN set "MSVC_BIN=%%~dpi"

set "CXXFLAGS=/nologo /std:c++20 /EHsc /permissive- /utf-8 /MD /FC /Isrc /external:W0 /external:Ivendor\imgui /external:Ivendor\imgui\backends /external:Ivendor\glfw\include"
set "LDFLAGS=/nologo /LIBPATH:vendor\glfw\lib-vc2022"
set "LDLIBS=glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib"
if "%CONFIG%"=="debug" (
  set "CXXFLAGS=%CXXFLAGS% /Od /Z7"
  set "LDFLAGS=%LDFLAGS% /DEBUG /SUBSYSTEM:CONSOLE"
) else (
  set "CXXFLAGS=%CXXFLAGS% /O2 /DNDEBUG"
  set "LDFLAGS=%LDFLAGS% /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
)
if not exist "%OBJ%\imgui" mkdir "%OBJ%\imgui"
if not exist "%OBJ%\app" mkdir "%OBJ%\app"

rem Dear ImGui never changes, so compile it once per config; "build clean" forces a redo.
if exist "%OBJ%\imgui\done.stamp" goto compile_app
echo [build] compiling Dear ImGui - first %CONFIG% build only
cl %CXXFLAGS% /W3 /MP /c vendor\imgui\imgui.cpp vendor\imgui\imgui_draw.cpp vendor\imgui\imgui_tables.cpp vendor\imgui\imgui_widgets.cpp vendor\imgui\imgui_demo.cpp vendor\imgui\backends\imgui_impl_glfw.cpp vendor\imgui\backends\imgui_impl_opengl3.cpp /Fo%OBJ%\imgui\ || exit /b 1
type nul > "%OBJ%\imgui\done.stamp"

:compile_app
rem App sources are small, so always recompile them; drop old objects so renamed/deleted files can't link.
echo [build] compiling src
robocopy src "%OBJ%\src-pending" /MIR /NJH /NJS /NFL /NDL /NP >nul
del /q "%OBJ%\app\*.obj" 2>nul
cl %CXXFLAGS% /W4 /MP /c src\*.cpp src\apps\*.cpp /Fo%OBJ%\app\ || exit /b 1

echo [build] linking %OUT%\csopesy.exe
"%MSVC_BIN%link.exe" %LDFLAGS% /OUT:%OUT%\csopesy.exe %OBJ%\app\*.obj %OBJ%\imgui\*.obj %LDLIBS% || (echo [build] link failed. If csopesy.exe is still open, shut it down with PWR and try again. & exit /b 1)
rem Save the sources this exe was built from, for the up-to-date check above.
if exist "%OBJ%\src-built" rmdir /s /q "%OBJ%\src-built"
move "%OBJ%\src-pending" "%OBJ%\src-built" >nul

:run
if not defined RUN exit /b 0
echo [build] running %OUT%\csopesy.exe - close it with the PWR button
"%OUT%\csopesy.exe"
