#pragma once
#include "config.h"
#include "imgui.h"
#include <algorithm>
#include <vector>

namespace csopesy {

struct ShellState;
using DrawAppFn  = void (*)(ShellState& shell, bool* open);
using DrawIconFn = void (*)(ImDrawList* dl, ImVec2 min, ImVec2 max);

struct AppWindow {
    const char* title;            // window title + taskbar tooltip
    DrawAppFn draw;
    DrawIconFn drawIcon;
    bool open = false;            // "running": drawn each frame + indicator on taskbar
    bool focusRequested = false;  // consumed by DrawApps next frame
};

void DrawAboutPc(ShellState& shell, bool* open);
void DrawAboutPcIcon(ImDrawList* dl, ImVec2 min, ImVec2 max);
void DrawFileExplorer(ShellState& shell, bool* open);
void DrawFileExplorerIcon(ImDrawList* dl, ImVec2 min, ImVec2 max);
void DrawTaskManager(ShellState& shell, bool* open);
void DrawTaskManagerIcon(ImDrawList* dl, ImVec2 min, ImVec2 max);

struct ShellState {
    Config config;                    // loaded from config.txt by main()
    bool showConfigWarnings = false;  // on-screen list of config.txt problems
    bool shutdownRequested = false;   // set only by the PWR dialog
    AppWindow apps[3] = {
        {"About This PC", DrawAboutPc, DrawAboutPcIcon},
        {"File Explorer", DrawFileExplorer, DrawFileExplorerIcon},
        {"Task Manager", DrawTaskManager, DrawTaskManagerIcon},
    };
    std::vector<bool> endedProcesses;  // Task Manager "End task" on config.processes rows, this session
};

inline float TaskbarHeight() { return ImGui::GetFrameHeight() * 2.2f; }

// First-use window rect in font units, relative to the work area (which excludes the taskbar)
// and clamped to it, so app windows start clear of a top taskbar and fit small windows.
inline void SetNextAppWindowRect(float x, float y, float width, float height) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    const float u = ImGui::GetFontSize();
    const ImVec2 size(std::min(width * u, vp->WorkSize.x), std::min(height * u, vp->WorkSize.y));
    const ImVec2 pos(vp->WorkPos.x + std::min(x * u, vp->WorkSize.x - size.x),
                     vp->WorkPos.y + std::min(y * u, vp->WorkSize.y - size.y));
    ImGui::SetNextWindowSize(size, ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(pos, ImGuiCond_FirstUseEver);
}

void DrawDesktop(ShellState& shell);
void DrawTaskbar(ShellState& shell);

}  // namespace csopesy
