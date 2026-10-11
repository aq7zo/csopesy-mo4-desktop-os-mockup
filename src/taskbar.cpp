#include "shell.h"
#include "imgui_internal.h"  // BeginViewportSideBar, BringWindowToDisplayFront

namespace csopesy {
namespace {

constexpr ImU32 kAccent = IM_COL32(96, 170, 255, 255);

void TaskbarButton(AppWindow& app, float size) {
    ImGui::PushID(app.title);
    const bool clicked = ImGui::InvisibleButton("app", ImVec2(size, size));
    const bool hovered = ImGui::IsItemHovered();
    const ImVec2 min = ImGui::GetItemRectMin();
    const ImVec2 max = ImGui::GetItemRectMax();
    ImDrawList* dl = ImGui::GetWindowDrawList();
    if (hovered || app.open) dl->AddRectFilled(min, max, IM_COL32(255, 255, 255, hovered ? 40 : 18), 6.0f);
    const float pad = size * 0.22f;
    app.drawIcon(dl, ImVec2(min.x + pad, min.y + pad), ImVec2(max.x - pad, max.y - pad));
    if (app.open) {  // running-app indicator, like Windows
        const float cx = (min.x + max.x) * 0.5f;
        dl->AddRectFilled(ImVec2(cx - size * 0.18f, max.y - 3.0f), ImVec2(cx + size * 0.18f, max.y), kAccent, 1.5f);
    }
    if (hovered) ImGui::SetTooltip("%s", app.title);
    if (clicked) { app.open = true; app.focusRequested = true; }
    ImGui::PopID();
}

}  // namespace

void DrawTaskbar(ShellState& shell) {
    ImGuiViewport* vp = ImGui::GetMainViewport();
    const float height = TaskbarHeight();
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(16, 16, 22, 235));
    constexpr ImGuiWindowFlags kFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;
    // Side bar also shrinks the viewport work area, so ImGui's window clamping keeps
    // dragged app title bars above a bottom taskbar (DrawApps handles a top one).
    const ImGuiDir edge = shell.config.taskbarTop ? ImGuiDir_Up : ImGuiDir_Down;
    ImGui::BeginViewportSideBar("##taskbar", vp, edge, height, kFlags);
    // Keep the taskbar above app windows, but under open modals (shutdown dialog).
    if (!ImGui::IsPopupOpen("", ImGuiPopupFlags_AnyPopupId | ImGuiPopupFlags_AnyPopupLevel))
        ImGui::BringWindowToDisplayFront(ImGui::GetCurrentWindow());

    const ImGuiStyle& style = ImGui::GetStyle();
    const float buttonSize = height - style.WindowPadding.y * 2.0f;
    ImGui::AlignTextToFramePadding();
    ImGui::TextUnformatted("CSOPESY OS");

    constexpr int kCount = static_cast<int>(IM_ARRAYSIZE(shell.apps));
    const float rowWidth = buttonSize * kCount + style.ItemSpacing.x * (kCount - 1);
    ImGui::SameLine((vp->Size.x - rowWidth) * 0.5f);
    ImGui::SetCursorPosY(style.WindowPadding.y);
    for (int i = 0; i < kCount; ++i) {
        if (i > 0) ImGui::SameLine();
        TaskbarButton(shell.apps[i], buttonSize);
    }
    ImGui::End();
    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);
}

}  // namespace csopesy
