#pragma once
#include "imgui.h"

namespace csopesy {

using DrawAppFn  = void (*)(bool* open);
using DrawIconFn = void (*)(ImDrawList* dl, ImVec2 min, ImVec2 max);

struct AppWindow {
    const char* title;            // taskbar tooltip
    DrawAppFn draw;
    DrawIconFn drawIcon;
    bool open = false;            // "running": drawn each frame + indicator on taskbar
    bool focusRequested = false;  // consumed by DrawApps next frame
};

void DrawAboutPc(bool* open);
void DrawAboutPcIcon(ImDrawList* dl, ImVec2 min, ImVec2 max);
void DrawFileExplorer(bool* open);
void DrawFileExplorerIcon(ImDrawList* dl, ImVec2 min, ImVec2 max);
void DrawTaskManager(bool* open);
void DrawTaskManagerIcon(ImDrawList* dl, ImVec2 min, ImVec2 max);

struct ShellState {
    bool shutdownRequested = false;  // set only by the PWR dialog
    AppWindow apps[3] = {
        {"About This PC", DrawAboutPc, DrawAboutPcIcon},
        {"File Explorer", DrawFileExplorer, DrawFileExplorerIcon},
        {"Task Manager", DrawTaskManager, DrawTaskManagerIcon},
    };
};

inline float TaskbarHeight() { return ImGui::GetFrameHeight() * 2.2f; }

void DrawDesktop(ShellState& shell);
void DrawTaskbar(ShellState& shell);

}  // namespace csopesy
