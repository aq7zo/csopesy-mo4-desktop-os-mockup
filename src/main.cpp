#include "shell.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cstdio>
#include <filesystem>

static void OnGlfwError(int code, const char* description) {
    std::fprintf(stderr, "GLFW error %d: %s\n", code, description);
}

static void DrawApps(csopesy::ShellState& shell) {
    for (csopesy::AppWindow& app : shell.apps) {
        if (!app.open) continue;
        if (app.focusRequested) { ImGui::SetNextWindowFocus(); app.focusRequested = false; }
        app.draw(&app.open);
    }
}

int main() {
    glfwSetErrorCallback(OnGlfwError);
    if (!glfwInit()) return 1;
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);  // borderless: no title bar, no X button

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    // +1 px height: a window exactly covering the monitor gets promoted to exclusive fullscreen,
    // which freezes screen capture/recording/sharing. The extra row sits off-screen.
    GLFWwindow* window = glfwCreateWindow(mode->width, mode->height + 1, "CSOPESY OS", nullptr, nullptr);
    if (!window) { glfwTerminate(); return 1; }
    int monitorX = 0, monitorY = 0;
    glfwGetMonitorPos(monitor, &monitorX, &monitorY);
    glfwSetWindowPos(window, monitorX, monitorY);
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
    const float dpiScale = ImGui_ImplGlfw_GetContentScaleForMonitor(monitor);
    ImGui::GetStyle().ScaleAllSizes(dpiScale);
    ImGui::GetStyle().FontScaleDpi = dpiScale;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 130");

    csopesy::ShellState shell;
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
