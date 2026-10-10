#include "shell.h"
#include <cstdio>
#include <initializer_list>

namespace csopesy {
namespace {

struct Process {
    const char* name;
    const char* status;
    float cpuPct;
    float memMb;
    float diskMbs;
    float netMbps;
    bool isApp;
};

// Dummy values only (spec: "Use dummy values on the Task Manager").
constexpr Process kProcesses[] = {
    {"CSOPESY Shell", "", 3.2f, 182.4f, 0.1f, 0.0f, true},
    {"Task Manager", "", 1.1f, 41.7f, 0.0f, 0.0f, true},
    {"File Explorer", "", 0.4f, 63.2f, 0.2f, 0.0f, true},
    {"About This PC", "", 0.0f, 12.8f, 0.0f, 0.0f, true},
    {"Desktop Window Manager", "", 2.6f, 96.5f, 0.0f, 0.0f, false},
    {"Antimalware Service Executable", "", 1.8f, 211.9f, 0.4f, 0.0f, false},
    {"Service Host: Local System", "", 0.7f, 54.1f, 0.1f, 0.1f, false},
    {"Search Indexer", "", 0.2f, 27.6f, 0.3f, 0.0f, false},
    {"Audio Device Graph Isolation", "", 0.3f, 11.2f, 0.0f, 0.0f, false},
    {"Runtime Broker", "", 0.0f, 8.3f, 0.0f, 0.0f, false},
    {"Spooler SubSystem App", "", 0.0f, 6.9f, 0.0f, 0.0f, false},
    {"COM Surrogate", "Suspended", 0.0f, 3.4f, 0.0f, 0.0f, false},
};
constexpr float kTotalMemMb = 16384.0f;
constexpr int kColumns = 6;

void TextRightAligned(const char* text) {
    const float avail = ImGui::GetContentRegionAvail().x;
    const float width = ImGui::CalcTextSize(text).x;
    if (avail > width) ImGui::SetCursorPosX(ImGui::GetCursorPosX() + avail - width);
    ImGui::TextUnformatted(text);
}

// Windows tints busier cells a deeper yellow.
void HeatCell(float load) {
    const float clamped = load < 0.0f ? 0.0f : (load > 1.0f ? 1.0f : load);
    ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, IM_COL32(255, 196, 0, 25 + static_cast<int>(clamped * 150.0f)));
}

void MetricCell(int column, float load, const char* fmt, float value) {
    ImGui::TableSetColumnIndex(column);
    HeatCell(load);
    char text[32];
    std::snprintf(text, sizeof text, fmt, value);
    TextRightAligned(text);
}

void DrawGroup(const char* label, bool apps) {
    int count = 0;
    for (const Process& p : kProcesses) count += (p.isApp == apps);
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "%s (%d)", label, count);
    for (const Process& p : kProcesses) {
        if (p.isApp != apps) continue;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Indent();
        ImGui::TextUnformatted(p.name);
        ImGui::Unindent();
        ImGui::TableSetColumnIndex(1);
        ImGui::TextDisabled("%s", p.status);
        MetricCell(2, p.cpuPct / 10.0f, "%.1f%%", p.cpuPct);
        MetricCell(3, p.memMb / 512.0f, "%.1f MB", p.memMb);
        MetricCell(4, p.diskMbs, "%.1f MB/s", p.diskMbs);
        MetricCell(5, p.netMbps, "%.1f Mbps", p.netMbps);
    }
}

void DrawProcessesTab() {
    float cpuTotal = 0.0f, memTotal = 0.0f;
    for (const Process& p : kProcesses) { cpuTotal += p.cpuPct; memTotal += p.memMb; }

    // Windows-style headers: total on top, metric name below.
    char cpuHeader[32], memHeader[32];
    std::snprintf(cpuHeader, sizeof cpuHeader, "%.0f%%\nCPU", cpuTotal);
    std::snprintf(memHeader, sizeof memHeader, "%.0f%%\nMemory", memTotal / kTotalMemMb * 100.0f);
    const char* headers[kColumns] = {"\nName", "\nStatus", cpuHeader, memHeader, "0%\nDisk", "0%\nNetwork"};

    const float footer = ImGui::GetFrameHeightWithSpacing();
    constexpr ImGuiTableFlags kFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("processes", kColumns, kFlags, ImVec2(0, -footer))) {
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("CPU", ImGuiTableColumnFlags_WidthStretch, 0.9f);
        ImGui::TableSetupColumn("Memory", ImGuiTableColumnFlags_WidthStretch, 1.1f);
        ImGui::TableSetupColumn("Disk", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("Network", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        for (int c = 0; c < kColumns; ++c) {
            ImGui::TableSetColumnIndex(c);
            ImGui::TableHeader(headers[c]);
        }
        DrawGroup("Apps", true);
        DrawGroup("Background processes", false);
        ImGui::EndTable();
    }

    const ImGuiStyle& style = ImGui::GetStyle();
    const float buttonW = ImGui::CalcTextSize("End task").x + style.FramePadding.x * 2.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonW);
    ImGui::BeginDisabled();  // mockup: no real processes to end
    ImGui::Button("End task");
    ImGui::EndDisabled();
}

}  // namespace

void DrawTaskManager(bool* open) {
    ImGui::SetNextWindowSize(ImVec2(820, 500), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowPos(ImVec2(120, 80), ImGuiCond_FirstUseEver);
    if (!ImGui::Begin("Task Manager", open)) { ImGui::End(); return; }
    if (ImGui::BeginTabBar("tabs")) {
        if (ImGui::BeginTabItem("Processes")) { DrawProcessesTab(); ImGui::EndTabItem(); }
        for (const char* tab : {"Performance", "App history", "Startup apps", "Users", "Details", "Services"}) {
            if (ImGui::BeginTabItem(tab)) {
                ImGui::TextDisabled("%s is not available in this mockup.", tab);
                ImGui::EndTabItem();
            }
        }
        ImGui::EndTabBar();
    }
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
