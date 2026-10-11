#include "shell.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"  // FindWindowByName
#include <GLFW/glfw3.h>
#include <algorithm>
#include <cstdio>
#include <filesystem>

static void OnGlfwError(int code, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

static void DrawApps(csopesy::ShellState& shell) {
    const float workTop = ImGui::GetMainViewport()->WorkPos.y;
    for (csopesy::AppWindow& app : shell.apps) {
        if (!app.open) continue;
        if (app.focusRequested) { ImGui::SetNextWindowFocus(); app.focusRequested = false; }
        app.draw(shell, &app.open);
        // ImGui only keeps part of a window inside the work area, so a window dragged up could hide
        // its title bar (and X) under a top taskbar. Push it back below the work-area top.
        if (const ImGuiWindow* window = ImGui::FindWindowByName(app.title); window && window->Pos.y < workTop)
            ImGui::SetWindowPos(app.title, ImVec2(window->Pos.x, workTop));
    }
}

int main(int argc, char** argv) {
    csopesy::ShellState shell;
    shell.config = csopesy::LoadConfig(argc > 1 ? argv[1] : "config.txt");
    for (const std::string& warning : shell.config.warnings) std::fprintf(stderr, "config: %s\n", warning.c_str());
    shell.showConfigWarnings = !shell.config.warnings.empty();
    const csopesy::Config& config = shell.config;

    glfwSetErrorCallback(OnGlfwError);
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    int monitorX = 0, monitorY = 0;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);
    GLFWwindow* window = nullptr;
    if (config.fullscreen) {
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);  // borderless: no title bar, no X button
        // +1 px height: a window exactly covering the monitor gets promoted to exclusive fullscreen,
        // which freezes screen capture/recording/sharing. The extra row sits off-screen.
        window = glfwCreateWindow(mode->width, mode->height + 1, "CSOPESY OS", nullptr, nullptr);
        if (!window) { glfwTerminate(); return 1; }
        glfwSetWindowPos(window, monitorX, monitorY);
    } else {
        // Normal resizable window; the close veto below still makes its X button do nothing.
        // Fit it, title bar and borders included, inside the work area (above the Windows taskbar).
        int workX = 0, workY = 0, workW = 0, workH = 0;
        glfwGetMonitorWorkarea(monitor, &workX, &workY, &workW, &workH);
        window = glfwCreateWindow(std::min(config.windowWidth, workW), std::min(config.windowHeight, workH),
                                  "CSOPESY OS", nullptr, nullptr);
        if (!window) { glfwTerminate(); return 1; }
        int left = 0, top = 0, right = 0, bottom = 0;
        glfwGetWindowFrameSize(window, &left, &top, &right, &bottom);
        const int width = std::min(config.windowWidth, workW - left - right);
        const int height = std::min(config.windowHeight, workH - top - bottom);
        glfwSetWindowSize(window, width, height);
        glfwSetWindowPos(window, workX + left + (workW - left - right - width) / 2,
                         workY + top + (workH - top - bottom - height) / 2);
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    // Veto every OS close request (Alt+F4, taskbar "Close window"): PWR is the only exit.
    glfwSetWindowCloseCallback(window, [](GLFWwindow* w) { glfwSetWindowShouldClose(w, GLFW_FALSE); });

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    constexpr const char* kUiFont = "C:/Windows/Fonts/segoeui.ttf";
    if (std::filesystem::exists(kUiFont)) io.Fonts->AddFontFromFileTTF(kUiFont, 18.0f);
    ImGui::StyleColorsDark();
    const float uiScale = ImGui_ImplGlfw_GetContentScaleForMonitor(monitor) * config.uiScale;  // DPI x ui-scale
    ImGui::GetStyle().ScaleAllSizes(uiScale);
    ImGui::GetStyle().FontScaleDpi = uiScale;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    // Some drivers (seen: NVIDIA Optimus) busy-wait in SwapBuffers or ignore the swap interval,
    // burning a full core; sleep-pace frames to the monitor refresh so the shell idles.
    const double frameBudget = 1.0 / (mode->refreshRate > 0 ? mode->refreshRate : 60);
    double nextFrame = glfwGetTime();
    while (!shell.shutdownRequested) {
        nextFrame += frameBudget;
        const double now = glfwGetTime();
        if (nextFrame > now) ImGui_ImplGlfw_Sleep(static_cast<int>((nextFrame - now) * 1000.0));
        else nextFrame = now;
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED)) { ImGui_ImplGlfw_Sleep(10); continue; }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        csopesy::DrawDesktop(shell);  // layer 0: always first
        DrawApps(shell);              // layer 1: app windows
        csopesy::DrawTaskbar(shell);  // layer 2: forced on top

        ImGui::Render();
        int width = 0, height = 0;
        glfwGetFramebufferSize(window, &width, &height);
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
