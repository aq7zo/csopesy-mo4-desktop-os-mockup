#include "shell.h"
#include <cfloat>
#include <ctime>

namespace csopesy {
namespace {

constexpr ImU32 kWallTop    = IM_COL32(22, 36, 70, 255);
constexpr ImU32 kWallBottom = IM_COL32(86, 38, 98, 255);
constexpr ImU32 kWallDot    = IM_COL32(255, 255, 255, 22);

void DrawWallpaper(ImDrawList* dl, ImVec2 min, ImVec2 max) {
    dl->AddRectFilledMultiColor(min, max, kWallTop, kWallTop, kWallBottom, kWallBottom);
    const float spacing = ImGui::GetFontSize() * 2.0f;
    for (float y = min.y + spacing; y < max.y; y += spacing)
        for (float x = min.x + spacing; x < max.x; x += spacing)
            dl->AddRectFilled(ImVec2(x - 1.0f, y - 1.0f), ImVec2(x + 1.0f, y + 1.0f), kWallDot);
}

void DrawClock(ImDrawList* dl, const ImGuiViewport* vp) {
    const std::time_t now = std::time(nullptr);  // re-read every frame
    std::tm local{};
    localtime_s(&local, &now);
    char timeText[16];
    char dateText[48];
    std::strftime(timeText, sizeof timeText, "%I:%M:%S %p", &local);
    std::strftime(dateText, sizeof dateText, "%A, %B %d, %Y", &local);

    ImFont* font = ImGui::GetFont();
    const float unit = ImGui::GetFontSize();
    const float timeSize = unit * 3.0f;
    const ImVec2 timeDim = font->CalcTextSizeA(timeSize, FLT_MAX, 0.0f, timeText);
    const ImVec2 dateDim = font->CalcTextSizeA(unit, FLT_MAX, 0.0f, dateText);
    const float right = vp->Pos.x + vp->Size.x - unit;  // fixed top-right corner
    const float top = vp->Pos.y + unit;
    dl->AddText(font, timeSize, ImVec2(right - timeDim.x, top), IM_COL32_WHITE, timeText);
    dl->AddText(font, unit, ImVec2(right - dateDim.x, top + timeDim.y), IM_COL32(255, 255, 255, 190), dateText);
}

void DrawPowerButton(ShellState& shell, const ImGuiViewport* vp) {
    const float unit = ImGui::GetFontSize();
    ImGui::SetCursorScreenPos(ImVec2(vp->Pos.x + unit, vp->Pos.y + unit));
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(176, 36, 48, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(214, 52, 64, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(140, 24, 36, 255));
    if (ImGui::Button("PWR", ImVec2(unit * 4.0f, unit * 2.0f))) ImGui::OpenPopup("Shut down");
    ImGui::PopStyleColor(3);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("Shut down CSOPESY OS");

    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Shut down", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextUnformatted("Shut down CSOPESY OS?");
        ImGui::Spacing();
        if (ImGui::Button("Shut down")) shell.shutdownRequested = true;
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || ImGui::IsKeyPressed(ImGuiKey_Escape)) ImGui::CloseCurrentPopup();
        ImGui::EndPopup();
    }
}

}  // namespace

void DrawDesktop(ShellState& shell) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    // Background draw list renders beneath every window -> desktop is the base layer.
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    DrawWallpaper(bg, vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y));
    DrawClock(bg, vp);

    // Full-screen host window for desktop widgets; never rises above app windows.
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    constexpr ImGuiWindowFlags kFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::Begin("##desktop", nullptr, kFlags);
    DrawPowerButton(shell, vp);
    ImGui::End();
}

}  // namespace csopesy
