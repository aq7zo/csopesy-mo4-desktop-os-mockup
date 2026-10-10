#include "shell.h"

namespace csopesy {

void DrawTaskManager(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(820, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(120, 80), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Task Manager", open)) { ImGui::End(); return; }
    ImGui::TextDisabled("Processes table arrives in Task 4.");
    ImGui::End();
}

void DrawTaskManagerIcon(ImDrawList* dl, ImVec2 min, ImVec2 max) {  // bar chart
    const float w = max.x - min.x, h = max.y - min.y;
    const ImU32 col = IM_COL32(110, 220, 140, 255);
    constexpr float kBars[] = {0.45f, 0.85f, 0.6f};
    const float barW = w * 0.24f, gap = w * 0.14f;
    for (int i = 0; i < 3; ++i) {
        const float x = min.x + i * (barW + gap);
        dl->AddRectFilled(ImVec2(x, max.y - h * kBars[i]), ImVec2(x + barW, max.y), col, 2.0f);
    }
}

}  // namespace csopesy
