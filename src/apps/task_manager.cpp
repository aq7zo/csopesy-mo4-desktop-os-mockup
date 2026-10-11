#include "shell.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <initializer_list>
#include <string_view>
#include <vector>

namespace csopesy {
namespace {

constexpr int kColumns = 6;
constexpr int kNameColumn = 0;
constexpr int kStatusColumn = 1;

// One visible row. Taskbar apps appear only while their window is open, like real apps.
struct Row {
    const char* name;
    const ProcessInfo* values;  // dummy values (spec: "Use dummy values on the Task Manager")
    int appIndex;               // index into ShellState::apps, or -1
    int processIndex;           // index into Config::processes, or -1
};

struct Selection { int appIndex = -1; int processIndex = -1; };

bool Matches(const Row& row, const Selection& selection) {
    return (row.appIndex >= 0 && row.appIndex == selection.appIndex) ||
           (row.processIndex >= 0 && row.processIndex == selection.processIndex);
}

bool EqualsIgnoreCase(std::string_view a, std::string_view b) {
    return a.size() == b.size() && std::equal(a.begin(), a.end(), b.begin(), [](char x, char y) {
        return std::tolower(static_cast<unsigned char>(x)) == std::tolower(static_cast<unsigned char>(y));
    });
}

bool IsShellApp(const ShellState& shell, std::string_view name) {
    for (const AppWindow& app : shell.apps)
        if (EqualsIgnoreCase(app.title, name)) return true;
    return false;
}

const ProcessInfo* FindProcess(const std::vector<ProcessInfo>& list, std::string_view name) {
    for (const ProcessInfo& process : list)
        if (EqualsIgnoreCase(process.name, name)) return &process;
    return nullptr;
}

// A taskbar app's dummy values: its process row, else zeros, so the header totals equal exactly the
// rows config.txt lists. (The built-in list has rows for all three apps.)
const ProcessInfo& AppValues(const Config& config, const char* title) {
    static const ProcessInfo kZero;
    const ProcessInfo* process = FindProcess(config.processes, title);
    return process != nullptr ? *process : kZero;
}

void CollectRows(const ShellState& shell, std::vector<Row>& apps, std::vector<Row>& background) {
    for (int i = 0; i < static_cast<int>(IM_ARRAYSIZE(shell.apps)); ++i)
        if (shell.apps[i].open) apps.push_back({shell.apps[i].title, &AppValues(shell.config, shell.apps[i].title), i, -1});
    const std::vector<ProcessInfo>& processes = shell.config.processes;
    for (int i = 0; i < static_cast<int>(processes.size()); ++i) {
        if (shell.endedProcesses[i] || IsShellApp(shell, processes[i].name)) continue;
        (processes[i].isApp ? apps : background).push_back({processes[i].name.c_str(), &processes[i], -1, i});
    }
}

float Metric(const ProcessInfo& values, int column) {
    const float metrics[kColumns] = {0.0f, 0.0f, values.cpuPct, values.memMb, values.diskMbs, values.netMbps};
    return metrics[column];
}

bool RowLess(const Row& a, const Row& b, int column) {
    if (column == kNameColumn) {
        return std::lexicographical_compare(a.name, a.name + std::strlen(a.name), b.name, b.name + std::strlen(b.name),
            [](char x, char y) { return std::tolower(static_cast<unsigned char>(x)) < std::tolower(static_cast<unsigned char>(y)); });
    }
    if (column == kStatusColumn) return a.values->status < b.values->status;
    return Metric(*a.values, column) < Metric(*b.values, column);
}

// Sorts within a group, like Windows. No spec (third header click) keeps config.txt order.
void SortRows(std::vector<Row>& rows, const ImGuiTableSortSpecs* specs) {
    if (specs == nullptr || specs->SpecsCount == 0) return;
    const int column = specs->Specs[0].ColumnIndex;
    const bool descending = specs->Specs[0].SortDirection == ImGuiSortDirection_Descending;
    std::stable_sort(rows.begin(), rows.end(), [&](const Row& a, const Row& b) {
        return descending ? RowLess(b, a, column) : RowLess(a, b, column);
    });
}

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

void DrawGroup(const char* label, const std::vector<Row>& rows, Selection& selection) {
    ImGui::TableNextRow();
    ImGui::TableSetColumnIndex(0);
    ImGui::TextColored(ImVec4(0.55f, 0.75f, 1.0f, 1.0f), "%s (%d)", label, static_cast<int>(rows.size()));
    for (const Row& row : rows) {
        const ProcessInfo& p = *row.values;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushID(row.appIndex >= 0 ? row.appIndex : 1000 + row.processIndex);
        ImGui::Indent();
        if (ImGui::Selectable(row.name, Matches(row, selection), ImGuiSelectableFlags_SpanAllColumns))
            selection = {row.appIndex, row.processIndex};
        ImGui::Unindent();
        ImGui::PopID();
        ImGui::TableSetColumnIndex(1);
        ImGui::TextDisabled("%s", p.status.c_str());
        MetricCell(2, p.cpuPct / 10.0f, "%.1f%%", p.cpuPct);
        MetricCell(3, p.memMb / 512.0f, "%.1f MB", p.memMb);
        MetricCell(4, p.diskMbs, "%.1f MB/s", p.diskMbs);
        MetricCell(5, p.netMbps, "%.1f Mbps", p.netMbps);
    }
}

void DrawProcessesTab(ShellState& shell) {
    if (shell.endedProcesses.size() != shell.config.processes.size())
        shell.endedProcesses.assign(shell.config.processes.size(), false);
    std::vector<Row> apps, background;
    CollectRows(shell, apps, background);

    // Windows-style headers: total of the listed rows on top, metric name below.
    float cpuTotal = 0.0f, memTotal = 0.0f;
    for (const std::vector<Row>* group : {&apps, &background})
        for (const Row& row : *group) { cpuTotal += row.values->cpuPct; memTotal += row.values->memMb; }
    char cpuHeader[32], memHeader[32];
    std::snprintf(cpuHeader, sizeof cpuHeader, "%.0f%%\nCPU", std::min(cpuTotal, 100.0f));
    std::snprintf(memHeader, sizeof memHeader, "%.0f%%\nMemory", std::min(memTotal / shell.config.totalMemoryMb * 100.0f, 100.0f));
    const char* headers[kColumns] = {"\nName", "\nStatus", cpuHeader, memHeader, "0%\nDisk", "0%\nNetwork"};

    static Selection selection;
    const float footer = ImGui::GetFrameHeightWithSpacing();
    constexpr ImGuiTableFlags kFlags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV |
        ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortTristate;
    if (ImGui::BeginTable("processes", kColumns, kFlags, ImVec2(0, -footer))) {
        constexpr ImGuiTableColumnFlags kMetric = ImGuiTableColumnFlags_WidthStretch | ImGuiTableColumnFlags_PreferSortDescending;
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch, 3.0f);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch, 1.0f);
        ImGui::TableSetupColumn("CPU", kMetric, 0.9f);
        ImGui::TableSetupColumn("Memory", kMetric, 1.1f);
        ImGui::TableSetupColumn("Disk", kMetric, 1.0f);
        ImGui::TableSetupColumn("Network", kMetric, 1.0f);
        ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
        for (int c = 0; c < kColumns; ++c) {
            ImGui::TableSetColumnIndex(c);
            ImGui::TableHeader(headers[c]);  // click to sort
        }
        const ImGuiTableSortSpecs* specs = ImGui::TableGetSortSpecs();
        SortRows(apps, specs);
        SortRows(background, specs);
        DrawGroup("Apps", apps, selection);
        DrawGroup("Background processes", background, selection);
        ImGui::EndTable();
    }

    // A selection only counts while its row is still listed (e.g. the app wasn't closed meanwhile).
    bool hasSelection = false;
    for (const std::vector<Row>* group : {&apps, &background})
        for (const Row& row : *group) hasSelection = hasSelection || Matches(row, selection);

    const ImGuiStyle& style = ImGui::GetStyle();
    const float buttonW = ImGui::CalcTextSize("End task").x + style.FramePadding.x * 2.0f;
    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x - buttonW);
    ImGui::BeginDisabled(!hasSelection);
    // Ending a taskbar app closes its window; ending a dummy row hides it until the next launch.
    // Never exits the shell: PWR stays the only way out.
    if (ImGui::Button("End task")) {
        if (selection.appIndex >= 0) shell.apps[selection.appIndex].open = false;
        else shell.endedProcesses[selection.processIndex] = true;
        selection = {};
    }
    ImGui::EndDisabled();
}

}  // namespace

void DrawTaskManager(ShellState& shell, bool* open) {
    SetNextAppWindowRect(7.0f, 4.5f, 46.0f, 28.0f);  // font units: scales with DPI, stays clear of PWR
    if (!ImGui::Begin("Task Manager", open)) { ImGui::End(); return; }
    if (ImGui::BeginTabBar("tabs")) {
        if (ImGui::BeginTabItem("Processes")) { DrawProcessesTab(shell); ImGui::EndTabItem(); }
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
