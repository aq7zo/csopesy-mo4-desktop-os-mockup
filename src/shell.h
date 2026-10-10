#pragma once
#include "imgui.h"

namespace csopesy {

struct ShellState {
    bool shutdownRequested = false;  // set only by the PWR dialog
};

void DrawDesktop(ShellState& shell);

}  // namespace csopesy
