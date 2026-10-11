#pragma once
#include "imgui.h"
#include <string>
#include <vector>

namespace csopesy {

enum class Corner { TopLeft, TopRight, BottomLeft, BottomRight };
enum class WallpaperStyle { Gradient, Solid };
enum class WallpaperPattern { None, Dots, Grid, Stripes, Checker };

// One Task Manager row. Values are dummy data from config.txt (spec: "Use dummy values").
struct ProcessInfo {
    std::string name;
    std::string status;  // "" or "Suspended"
    float cpuPct = 0.0f;
    float memMb = 0.0f;
    float diskMbs = 0.0f;
    float netMbps = 0.0f;
    bool isApp = false;  // always listed under Apps (taskbar apps are matched by name instead)
};

std::vector<ProcessInfo> DefaultProcesses();

// Every parameter the quiz may change without recompiling. Defaults reproduce the original look.
struct Config {
    std::string sourcePath;             // file the values came from; empty = built-in defaults
    std::vector<std::string> warnings;  // problems found while reading; shown on screen

    bool fullscreen = true;
    int windowWidth = 1280;
    int windowHeight = 720;
    float uiScale = 1.0f;

    WallpaperStyle wallpaperStyle = WallpaperStyle::Gradient;
    ImU32 wallpaperColor = IM_COL32(22, 36, 70, 255);   // navy
    ImU32 wallpaperColor2 = IM_COL32(86, 38, 98, 255);  // purple
    bool wallpaperHorizontal = false;
    WallpaperPattern wallpaperPattern = WallpaperPattern::Dots;
    Corner clockCorner = Corner::TopRight;
    bool clock24h = false;
    Corner powerCorner = Corner::TopLeft;

    bool taskbarTop = false;

    float totalMemoryMb = 16384.0f;
    std::vector<ProcessInfo> processes = DefaultProcesses();
};

// Reads `path` (CSOPESY-style "key value" lines). Never fails: problems become warnings
// and the affected settings keep their defaults.
Config LoadConfig(const char* path);

}  // namespace csopesy
