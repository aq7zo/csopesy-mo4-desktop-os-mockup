#include "shell.h"

namespace csopesy {
namespace {

struct Spec { const char* key; const char* value; };
constexpr Spec kSpecs[] = {
    {"Device name", "CSOPESY-M04"},
    {"Processor", "Virtual CPU @ 3.20 GHz (8 cores)"},
    {"Installed RAM", "16.0 GB"},
    {"System type", "64-bit operating system, x64-based processor"},
    {"OS build", "CSOPESY OS 0.4 (Build 2026.10)"},
    {"Registered to", "CSOPESY Student"},
};

}  // namespace

void DrawAboutPc(ShellState& shell, bool* open) {
    SetNextAppWindowRect(9.0f, 7.0f, 27.0f, 24.0f);  // font units: scales with DPI
    if (!ImGui::Begin("About This PC", open)) { ImGui::End(); return; }
    ImGui::TextUnformatted("CSOPESY OS");
    ImGui::TextDisabled("Desktop shell mockup - Dear ImGui %s", IMGUI_VERSION);
    ImGui::SeparatorText("Device specifications");
    if (ImGui::BeginTable("specs", 2, ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg)) {
        for (const Spec& spec : kSpecs) {
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::TextDisabled("%s", spec.key);
            ImGui::TableSetColumnIndex(1);
            ImGui::TextUnformatted(spec.value);
        }
        ImGui::EndTable();
    }
    ImGui::SeparatorText("Storage");
    ImGui::ProgressBar(0.62f, ImVec2(-FLT_MIN, 0), "Local Disk (C:)  297 GB used of 476 GB");
    ImGui::ProgressBar(0.18f, ImVec2(-FLT_MIN, 0), "Data (D:)  168 GB used of 931 GB");
    ImGui::SeparatorText("Configuration");
    if (shell.config.sourcePath.empty()) ImGui::TextDisabled("Built-in defaults (config.txt not found)");
    else ImGui::TextWrapped("Loaded from %s", shell.config.sourcePath.c_str());
    ImGui::End();
}

void DrawAboutPcIcon(ImDrawList* dl, ImVec2 min, ImVec2 max) {  // monitor
    const float w = max.x - min.x, h = max.y - min.y;
    const ImU32 col = IM_COL32(120, 190, 255, 255);
    dl->AddRect(ImVec2(min.x, min.y + h * 0.1f), ImVec2(max.x, min.y + h * 0.7f), col, 3.0f, 0, 2.0f);
    dl->AddLine(ImVec2(min.x + w * 0.5f, min.y + h * 0.7f), ImVec2(min.x + w * 0.5f, min.y + h * 0.88f), col, 2.0f);
    dl->AddLine(ImVec2(min.x + w * 0.25f, min.y + h * 0.9f), ImVec2(min.x + w * 0.75f, min.y + h * 0.9f), col, 2.0f);
}

}  // namespace csopesy
