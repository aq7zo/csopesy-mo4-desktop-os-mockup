@echo off
rem Usage: tools\vsdev.cmd make [target]  -- runs the command inside an MSVC x64 environment.
where cl >nul 2>nul && goto run
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
for /f "usebackq delims=" %%i in (`"%VSWHERE%" -latest -prerelease -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSDIR=%%i"
if not defined VSDIR (echo [vsdev] MSVC not found. Install VS 2026 Build Tools with "Desktop development with C++". & exit /b 1)
call "%VSDIR%\Common7\Tools\VsDevCmd.bat" -arch=x64 -host_arch=x64 -no_logo || exit /b 1
:run
%*
