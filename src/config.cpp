#include "config.h"
#include <algorithm>
#include <cctype>
#include <charconv>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string_view>
#include <utility>

namespace csopesy {
namespace {

struct NamedColor { const char* name; ImU32 color; };
constexpr NamedColor kNamedColors[] = {  // CSS colour values, so "red" means what the quiz expects
    {"black", IM_COL32(0, 0, 0, 255)},         {"white", IM_COL32(255, 255, 255, 255)},
    {"gray", IM_COL32(128, 128, 128, 255)},    {"grey", IM_COL32(128, 128, 128, 255)},
    {"silver", IM_COL32(192, 192, 192, 255)},  {"red", IM_COL32(255, 0, 0, 255)},
    {"maroon", IM_COL32(128, 0, 0, 255)},      {"orange", IM_COL32(255, 165, 0, 255)},
    {"yellow", IM_COL32(255, 255, 0, 255)},    {"gold", IM_COL32(255, 215, 0, 255)},
    {"olive", IM_COL32(128, 128, 0, 255)},     {"lime", IM_COL32(0, 255, 0, 255)},
    {"green", IM_COL32(0, 128, 0, 255)},       {"teal", IM_COL32(0, 128, 128, 255)},
    {"cyan", IM_COL32(0, 255, 255, 255)},      {"skyblue", IM_COL32(135, 206, 235, 255)},
    {"blue", IM_COL32(0, 0, 255, 255)},        {"navy", IM_COL32(0, 0, 128, 255)},
    {"indigo", IM_COL32(75, 0, 130, 255)},     {"purple", IM_COL32(128, 0, 128, 255)},
    {"violet", IM_COL32(238, 130, 238, 255)},  {"magenta", IM_COL32(255, 0, 255, 255)},
    {"pink", IM_COL32(255, 192, 203, 255)},    {"brown", IM_COL32(165, 42, 42, 255)},
};

std::string Lower(std::string_view text) {
    std::string out(text);
    for (char& c : out) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// "Top-Right", "top_right" and "topright" all match "topright".
std::string Squash(std::string_view text) {
    std::string out;
    for (char c : text)
        if (c != '-' && c != '_') out += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return out;
}

// '#' followed only by hex digits, e.g. #162446 (or a mistyped #12345).
bool IsHexToken(std::string_view token) {
    return token.size() >= 2 && token[0] == '#' &&
           std::all_of(token.begin() + 1, token.end(), [](char c) { return std::isxdigit(static_cast<unsigned char>(c)) != 0; });
}

bool IsHexColor(std::string_view token) { return token.size() == 7 && IsHexToken(token); }

// Splits on whitespace, '=' and ','; "double quoted" text stays one token. A token starting with
// '#' begins a comment, unless it is a #RRGGBB value or a hex value right after the key (so a
// mistyped colour is reported instead of silently read as a comment).
std::vector<std::string> Tokenize(std::string_view line) {
    std::vector<std::string> tokens;
    size_t i = 0;
    while (i < line.size()) {
        const char c = line[i];
        if (std::isspace(static_cast<unsigned char>(c)) || c == '=' || c == ',') { ++i; continue; }
        if (c == '"') {
            const size_t end = line.find('"', i + 1);
            tokens.emplace_back(line.substr(i + 1, end == std::string_view::npos ? std::string_view::npos : end - i - 1));
            i = end == std::string_view::npos ? line.size() : end + 1;
            continue;
        }
        size_t end = i;
        while (end < line.size() && !std::isspace(static_cast<unsigned char>(line[end])) && line[end] != '=' && line[end] != ',') ++end;
        const std::string_view token = line.substr(i, end - i);
        if (token[0] == '#' && (tokens.empty() || !(IsHexColor(token) || (tokens.size() == 1 && IsHexToken(token))))) break;
        tokens.emplace_back(token);
        i = end;
    }
    return tokens;
}

std::optional<float> ParseFloat(std::string_view text) {
    if (!text.empty() && text.back() == '%') text.remove_suffix(1);
    float value = 0.0f;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || end != text.data() + text.size() || text.empty() || !std::isfinite(value)) return std::nullopt;
    return value;
}

std::optional<int> ParseInt(std::string_view text) {
    int value = 0;
    const auto [end, error] = std::from_chars(text.data(), text.data() + text.size(), value);
    if (error != std::errc() || end != text.data() + text.size() || text.empty()) return std::nullopt;
    return value;
}

std::optional<bool> ParseBool(std::string_view text) {
    const std::string word = Lower(text);
    if (word == "true" || word == "yes" || word == "on" || word == "1") return true;
    if (word == "false" || word == "no" || word == "off" || word == "0") return false;
    return std::nullopt;
}

// "#RRGGBB", "R G B" (0-255, also "R,G,B"), or a colour name.
std::optional<ImU32> ParseColor(const std::vector<std::string>& args) {
    if (args.size() == 3) {
        int rgb[3] = {};
        for (int i = 0; i < 3; ++i) {
            const std::optional<int> v = ParseInt(args[i]);
            if (!v || *v < 0 || *v > 255) return std::nullopt;
            rgb[i] = *v;
        }
        return IM_COL32(rgb[0], rgb[1], rgb[2], 255);
    }
    if (args.size() != 1) return std::nullopt;
    if (IsHexColor(args[0])) {
        const unsigned long hex = std::stoul(args[0].substr(1), nullptr, 16);
        return IM_COL32((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF, 255);
    }
    const std::string name = Squash(args[0]);
    for (const NamedColor& named : kNamedColors)
        if (name == named.name) return named.color;
    return std::nullopt;
}

template <typename T>
using Choices = std::initializer_list<std::pair<const char*, T>>;

template <typename T>
std::optional<T> ParseChoice(const std::vector<std::string>& args, Choices<T> choices) {
    if (args.size() != 1) return std::nullopt;
    const std::string word = Squash(args[0]);
    for (const auto& [name, value] : choices)
        if (word == Squash(name)) return value;
    return std::nullopt;
}

template <typename T>
std::string ListChoices(Choices<T> choices) {  // "a, b or c"
    std::string out;
    size_t i = 0;
    for (const auto& choice : choices) {
        if (i > 0) out += (i + 1 == choices.size()) ? " or " : ", ";
        out += choice.first;
        ++i;
    }
    return out;
}

const Choices<Corner> kCorners = {{"top-left", Corner::TopLeft}, {"top-right", Corner::TopRight},
                                  {"bottom-left", Corner::BottomLeft}, {"bottom-right", Corner::BottomRight}};

// process "Name" cpu mem [disk] [net] [app|background|suspended]
std::optional<ProcessInfo> ParseProcess(const std::vector<std::string>& args, std::string& error) {
    ProcessInfo process;
    if (!args.empty()) process.name = args[0];
    float* metrics[] = {&process.cpuPct, &process.memMb, &process.diskMbs, &process.netMbps};
    int count = 0;
    for (size_t i = 1; i < args.size(); ++i) {
        const std::string word = Lower(args[i]);
        if (word == "app") process.isApp = true;
        else if (word == "background") process.isApp = false;
        else if (word == "suspended") process.status = "Suspended";
        else if (word == "%" || word == "mb" || word == "mb/s" || word == "mbps") continue;
        else if (const std::optional<float> v = ParseFloat(args[i]); v && *v >= 0.0f && count < 4) *metrics[count++] = *v;
        else {
            error = "process \"" + process.name + "\": unexpected \"" + args[i] + "\" (put names with spaces in \"quotes\")";
            return std::nullopt;
        }
    }
    if (args.empty() || count < 2) {
        error = "process needs a name, CPU % and memory MB, e.g.  process \"Chrome\" 12.5 300";
        return std::nullopt;
    }
    process.cpuPct = std::min(process.cpuPct, 100.0f);
    return process;
}

std::string Utf8(const std::filesystem::path& path) {
    const std::u8string text = path.u8string();
    return std::string(text.begin(), text.end());
}

}  // namespace

std::vector<ProcessInfo> DefaultProcesses() {
    return {
        {"CSOPESY Shell", "", 3.2f, 182.4f, 0.1f, 0.0f, true},
        {"Task Manager", "", 1.1f, 41.7f, 0.0f, 0.0f, false},
        {"File Explorer", "", 0.4f, 63.2f, 0.2f, 0.0f, false},
        {"About This PC", "", 0.0f, 12.8f, 0.0f, 0.0f, false},
        {"Desktop Window Manager", "", 2.6f, 96.5f, 0.0f, 0.0f, false},
        {"Antimalware Service Executable", "", 1.8f, 211.9f, 0.4f, 0.0f, false},
        {"Service Host: Local System", "", 0.7f, 54.1f, 0.1f, 0.1f, false},
        {"Search Indexer", "", 0.2f, 27.6f, 0.3f, 0.0f, false},
        {"Audio Device Graph Isolation", "", 0.3f, 11.2f, 0.0f, 0.0f, false},
        {"Runtime Broker", "", 0.0f, 8.3f, 0.0f, 0.0f, false},
        {"Spooler SubSystem App", "", 0.0f, 6.9f, 0.0f, 0.0f, false},
        {"COM Surrogate", "Suspended", 0.0f, 3.4f, 0.0f, 0.0f, false},
    };
}

Config LoadConfig(const char* path) {
    Config config;
    std::error_code ec;
    std::filesystem::path file = std::filesystem::absolute(path, ec);
    if (ec) file = path;
    std::ifstream in(file);
    if (!in) {
        config.warnings.push_back(Utf8(file) + " not found, so built-in defaults are used.");
        return config;
    }
    config.sourcePath = Utf8(file);

    bool customProcesses = false;
    std::string line;
    for (int lineNumber = 1; std::getline(in, line); ++lineNumber) {
        if (lineNumber == 1 && line.starts_with("\xEF\xBB\xBF")) line.erase(0, 3);  // Notepad's UTF-8 BOM
        std::vector<std::string> tokens = Tokenize(line);
        if (tokens.empty()) continue;
        const std::string key = Lower(tokens[0]);
        const std::vector<std::string> args(tokens.begin() + 1, tokens.end());
        std::string got = args.empty() ? "nothing" : "\"";
        for (size_t i = 0; i < args.size(); ++i) got += (i > 0 ? " " : "") + args[i] + (i + 1 == args.size() ? "\"" : "");
        auto warn = [&](const std::string& message) {
            config.warnings.push_back("line " + std::to_string(lineNumber) + ": " + message);
        };
        auto expect = [&](const std::string& what) { warn(key + " expects " + what + " (got " + got + "); default kept."); };

        if (key == "fullscreen") {
            if (const auto v = ParseBool(args.size() == 1 ? args[0] : "")) config.fullscreen = *v; else expect("true or false");
        } else if (key == "window-width" || key == "window-height") {
            const std::optional<int> v = args.size() == 1 ? ParseInt(args[0]) : std::nullopt;
            if (v && *v >= 320 && *v <= 10000) (key == "window-width" ? config.windowWidth : config.windowHeight) = *v;
            else expect("a size in pixels from 320 to 10000");
        } else if (key == "ui-scale") {
            const std::optional<float> v = args.size() == 1 ? ParseFloat(args[0]) : std::nullopt;
            if (v && *v >= 0.5f && *v <= 3.0f) config.uiScale = *v; else expect("a number from 0.5 to 3.0");
        } else if (key == "wallpaper-style") {
            const auto kStyles = {std::pair{"gradient", WallpaperStyle::Gradient}, std::pair{"solid", WallpaperStyle::Solid}};
            if (const auto v = ParseChoice<WallpaperStyle>(args, kStyles)) config.wallpaperStyle = *v; else expect(ListChoices<WallpaperStyle>(kStyles));
        } else if (key == "wallpaper-color" || key == "wallpaper-color-2") {
            if (const auto v = ParseColor(args)) (key == "wallpaper-color" ? config.wallpaperColor : config.wallpaperColor2) = *v;
            else expect("#RRGGBB, R G B (0-255) or a colour name like red or navy");
        } else if (key == "wallpaper-direction") {
            const auto kDirections = {std::pair{"vertical", false}, std::pair{"horizontal", true}};
            if (const auto v = ParseChoice<bool>(args, kDirections)) config.wallpaperHorizontal = *v; else expect(ListChoices<bool>(kDirections));
        } else if (key == "wallpaper-pattern") {
            const auto kPatterns = {std::pair{"none", WallpaperPattern::None}, std::pair{"dots", WallpaperPattern::Dots},
                                        std::pair{"grid", WallpaperPattern::Grid}, std::pair{"stripes", WallpaperPattern::Stripes},
                                        std::pair{"checker", WallpaperPattern::Checker}};
            if (const auto v = ParseChoice<WallpaperPattern>(args, kPatterns)) config.wallpaperPattern = *v; else expect(ListChoices<WallpaperPattern>(kPatterns));
        } else if (key == "clock-corner" || key == "power-corner") {
            if (const auto v = ParseChoice<Corner>(args, kCorners)) (key == "clock-corner" ? config.clockCorner : config.powerCorner) = *v;
            else expect(ListChoices<Corner>(kCorners));
        } else if (key == "clock-format") {
            const auto kFormats = {std::pair{"12", false}, std::pair{"24", true}, std::pair{"12h", false}, std::pair{"24h", true}};
            if (const auto v = ParseChoice<bool>(args, kFormats)) config.clock24h = *v; else expect("12 or 24");
        } else if (key == "taskbar-position") {
            const auto kEdges = {std::pair{"bottom", false}, std::pair{"top", true}};
            if (const auto v = ParseChoice<bool>(args, kEdges)) config.taskbarTop = *v; else expect(ListChoices<bool>(kEdges));
        } else if (key == "total-memory-mb") {
            const std::optional<float> v = args.size() == 1 ? ParseFloat(args[0]) : std::nullopt;
            if (v && *v > 0.0f) config.totalMemoryMb = *v; else expect("a memory size in MB, e.g. 16384");
        } else if (key == "process") {
            std::string error;
            if (std::optional<ProcessInfo> process = ParseProcess(args, error)) {
                if (!customProcesses) config.processes.clear();  // the first process line replaces the defaults
                customProcesses = true;
                config.processes.push_back(std::move(*process));
            } else {
                warn(error + "; row skipped.");
            }
        } else {
            warn("unknown setting \"" + tokens[0] + "\"; ignored.");
        }
    }
    return config;
}

}  // namespace csopesy
