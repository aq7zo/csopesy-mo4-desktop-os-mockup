#include "shell.h"
#include <algorithm>
#include <cfloat>
#include <ctime>

namespace csopesy {
namespace {

constexpr ImU32 kPatternInk = IM_COL32(255, 255, 255, 22);
constexpr ImU32 kCheckerInk = IM_COL32(255, 255, 255, 14);

struct Box { ImVec2 min, max; };

bool IsLeft(Corner corner) { return corner == Corner::TopLeft || corner == Corner::BottomLeft; }
bool IsTop(Corner corner) { return corner == Corner::TopLeft || corner == Corner::TopRight; }

// Top-left of a `size` box pinned to `corner` of the work area, i.e. the window minus the taskbar.
ImVec2 PinToCorner(const ImGuiViewport* vp, Corner corner, ImVec2 size, float margin) {
    const float x = IsLeft(corner) ? vp->WorkPos.x + margin : vp->WorkPos.x + vp->WorkSize.x - margin - size.x;
    const float y = IsTop(corner) ? vp->WorkPos.y + margin : vp->WorkPos.y + vp->WorkSize.y - margin - size.y;
    return ImVec2(x, y);
}

void DrawPattern(ImDrawList* dl, ImVec2 min, ImVec2 max, WallpaperPattern pattern) {
    const float spacing = ImGui::GetFontSize() * 2.0f;
    switch (pattern) {
    case WallpaperPattern::None:
        break;
    case WallpaperPattern::Dots:
        for (float y = min.y + spacing; y < max.y; y += spacing)
            for (float x = min.x + spacing; x < max.x; x += spacing)
                dl->AddRectFilled(ImVec2(x - 1.0f, y - 1.0f), ImVec2(x + 1.0f, y + 1.0f), kPatternInk);
        break;
    case WallpaperPattern::Grid:
        for (float x = min.x + spacing; x < max.x; x += spacing) dl->AddLine(ImVec2(x, min.y), ImVec2(x, max.y), kPatternInk);
        for (float y = min.y + spacing; y < max.y; y += spacing) dl->AddLine(ImVec2(min.x, y), ImVec2(max.x, y), kPatternInk);
        break;
    case WallpaperPattern::Stripes: {  // 45-degree diagonals
        const float height = max.y - min.y;
        for (float x = min.x - height; x < max.x; x += spacing)
            dl->AddLine(ImVec2(x, max.y), ImVec2(x + height, min.y), kPatternInk, 2.0f);
        break;
    }
    case WallpaperPattern::Checker: {
        int row = 0;
        for (float y = min.y; y < max.y; y += spacing, ++row) {
            int column = 0;
            for (float x = min.x; x < max.x; x += spacing, ++column)
                if ((row + column) % 2 == 0) dl->AddRectFilled(ImVec2(x, y), ImVec2(x + spacing, y + spacing), kCheckerInk);
        }
        break;
    }
    }
}

void DrawWallpaper(ImDrawList* dl, ImVec2 min, ImVec2 max, const Config& config) {
    const ImU32 from = config.wallpaperColor;
    const ImU32 to = config.wallpaperStyle == WallpaperStyle::Solid ? from : config.wallpaperColor2;
    if (config.wallpaperHorizontal) dl->AddRectFilledMultiColor(min, max, from, to, to, from);
    else dl->AddRectFilledMultiColor(min, max, from, from, to, to);
    DrawPattern(dl, min, max, config.wallpaperPattern);
}

// Returns the clock's screen rect so PWR can stack beside it when both share a corner.
Box DrawClock(ImDrawList* dl, const ImGuiViewport* vp, const Config& config) {
    const std::time_t now = std::time(nullptr);  // re-read every frame
    std::tm local{};
    localtime_s(&local, &now);
    char timeText[16];
    char dateText[48];
    std::strftime(timeText, sizeof timeText, config.clock24h ? "%H:%M:%S" : "%I:%M:%S %p", &local);
    std::strftime(dateText, sizeof dateText, "%A, %B %d, %Y", &local);

    ImFont* font = ImGui::GetFont();
    const float unit = ImGui::GetFontSize();
    const float timeSize = unit * 3.0f;
    const ImVec2 timeDim = font->CalcTextSizeA(timeSize, FLT_MAX, 0.0f, timeText);
    const ImVec2 dateDim = font->CalcTextSizeA(unit, FLT_MAX, 0.0f, dateText);
    const ImVec2 size(std::max(timeDim.x, dateDim.x), timeDim.y + dateDim.y);
    const ImVec2 min = PinToCorner(vp, config.clockCorner, size, unit);  // fixed corner
    const bool left = IsLeft(config.clockCorner);
    const float timeX = left ? min.x : min.x + size.x - timeDim.x;
    const float dateX = left ? min.x : min.x + size.x - dateDim.x;
    dl->AddText(font, timeSize, ImVec2(timeX, min.y), IM_COL32_WHITE, timeText);
    dl->AddText(font, unit, ImVec2(dateX, min.y + timeDim.y), IM_COL32(255, 255, 255, 190), dateText);
    return {min, ImVec2(min.x + size.x, min.y + size.y)};
}

void DrawPowerButton(ShellState& shell, const ImGuiViewport* vp, Box clock) {
    const Config& config = shell.config;
    const float unit = ImGui::GetFontSize();
    const ImVec2 size(unit * 4.0f, unit * 2.0f);
    ImVec2 pos = PinToCorner(vp, config.powerCorner, size, unit);
    if (config.powerCorner == config.clockCorner)  // same corner: stack PWR inward of the clock
        pos.y = IsTop(config.powerCorner) ? clock.max.y + unit * 0.5f : clock.min.y - unit * 0.5f - size.y;
    ImGui::SetCursorScreenPos(pos);
    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(176, 36, 48, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, IM_COL32(214, 52, 64, 255));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, IM_COL32(140, 24, 36, 255));
    if (ImGui::Button("PWR", size)) ImGui::OpenPopup("Shut down");
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

// Lists config.txt problems on screen so a typo is spotted at once, not after a test case fails.
void DrawConfigWarnings(ShellState& shell) {
    if (!shell.showConfigWarnings) return;
    const float u = ImGui::GetFontSize();
    ImGui::SetNextWindowPos(ImGui::GetMainViewport()->GetCenter(), ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::Begin("Config warnings", &shell.showConfigWarnings, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoCollapse)) {
        ImGui::PushTextWrapPos(u * 36.0f);
        for (const std::string& warning : shell.config.warnings) ImGui::BulletText("%s", warning.c_str());
        ImGui::PopTextWrapPos();
        ImGui::Spacing();
        if (ImGui::Button("OK", ImVec2(u * 5.0f, 0.0f))) shell.showConfigWarnings = false;
    }
    ImGui::End();
}

}  // namespace

void DrawDesktop(ShellState& shell) {
    const ImGuiViewport* vp = ImGui::GetMainViewport();
    // Background draw list renders beneath every window -> desktop is the base layer.
    ImDrawList* bg = ImGui::GetBackgroundDrawList();
    DrawWallpaper(bg, vp->Pos, ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + vp->Size.y), shell.config);
    const Box clock = DrawClock(bg, vp, shell.config);

    // Full-screen host window for desktop widgets; never rises above app windows.
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    constexpr ImGuiWindowFlags kFlags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoFocusOnAppearing;
    ImGui::Begin("##desktop", nullptr, kFlags);
    DrawPowerButton(shell, vp, clock);
    ImGui::End();
    DrawConfigWarnings(shell);
}

}  // namespace csopesy
