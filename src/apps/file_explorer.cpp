#include "shell.h"

namespace csopesy {
namespace {

struct FileEntry { const char* name; const char* modified; const char* type; const char* size; };
constexpr const char* kPlaces[] = {"Home", "Desktop", "Documents", "Downloads", "Pictures", "This PC"};
constexpr FileEntry kFiles[] = {
    {"CSOPESY", "10/02/2026 9:14 PM", "File folder", ""},
    {"MO1 - Process Scheduler", "09/21/2026 4:02 PM", "File folder", ""},
    {"lecture-notes.pdf", "10/08/2026 11:30 AM", "PDF Document", "2,418 KB"},
    {"scheduler.cpp", "10/09/2026 8:45 PM", "C++ Source", "14 KB"},
    {"config.txt", "10/01/2026 7:20 PM", "Text Document", "1 KB"},
    {"screenshot-01.png", "10/05/2026 3:12 PM", "PNG Image", "842 KB"},
};

}  // namespace

void DrawFileExplorer(ShellState& /*shell*/, bool* open) {
    SetNextAppWindowRect(16.0f, 9.5f, 38.0f, 23.0f);  // font units: scales with DPI
    if (!ImGui::Begin("File Explorer", open)) { ImGui::End(); return; }
    static int place = 2;  // "Documents"

    ImGui::BeginDisabled();
    ImGui::ArrowButton("back", ImGuiDir_Left);
    ImGui::SameLine();
    ImGui::ArrowButton("forward", ImGuiDir_Right);
    ImGui::EndDisabled();
    ImGui::SameLine();
    ImGui::Text("This PC  >  %s", kPlaces[place]);
    ImGui::Separator();

    const float statusBar = ImGui::GetFrameHeightWithSpacing();
    ImGui::BeginChild("places", ImVec2(ImGui::GetFontSize() * 9.0f, -statusBar), ImGuiChildFlags_Borders);
    for (int i = 0; i < IM_ARRAYSIZE(kPlaces); ++i)
        if (ImGui::Selectable(kPlaces[i], place == i)) place = i;
    ImGui::EndChild();
    ImGui::SameLine();

    constexpr ImGuiTableFlags kFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    if (ImGui::BeginTable("files", 4, kFlags, ImVec2(0, -statusBar))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("Date modified", ImGuiTableColumnFlags_WidthStretch, 2.0f);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthStretch, 1.5f);
        ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableHeadersRow();
        for (const FileEntry& file : kFiles) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0); ImGui::TextUnformatted(file.name);
            ImGui::TableSetColumnIndex(1); ImGui::TextDisabled("%s", file.modified);
            ImGui::TableSetColumnIndex(2); ImGui::TextDisabled("%s", file.type);
            ImGui::TableSetColumnIndex(3); ImGui::TextUnformatted(file.size);
        }
        ImGui::EndTable();
    }
    ImGui::TextDisabled("%d items", static_cast<int>(IM_ARRAYSIZE(kFiles)));
    ImGui::End();
}

void DrawFileExplorerIcon(ImDrawList* dl, ImVec2 min, ImVec2 max) {  // folder
    const float w = max.x - min.x, h = max.y - min.y;
    const ImU32 col = IM_COL32(255, 196, 64, 255);
    dl->AddRectFilled(ImVec2(min.x, min.y + h * 0.15f), ImVec2(min.x + w * 0.45f, min.y + h * 0.35f), col, 2.0f);
    dl->AddRectFilled(ImVec2(min.x, min.y + h * 0.3f), ImVec2(max.x, max.y - h * 0.1f), col, 3.0f);
}

}  // namespace csopesy
