# CSOPESY OS mockup: GNU make + MSVC. Run inside a VS 2026 x64 dev shell,
# or from anywhere via:  tools\vsdev.cmd make [all|debug|run|clean|rebuild]
SHELL := cmd.exe
.SHELLFLAGS := /c

CONFIG ?= release
OUT    := build/$(CONFIG)
TARGET := $(OUT)/csopesy.exe

IMGUI := vendor/imgui
GLFW  := vendor/glfw

APP_SRC   := $(wildcard src/*.cpp src/apps/*.cpp)
APP_HDR   := $(wildcard src/*.h)
IMGUI_SRC := $(IMGUI)/imgui.cpp $(IMGUI)/imgui_draw.cpp $(IMGUI)/imgui_tables.cpp \
             $(IMGUI)/imgui_widgets.cpp $(IMGUI)/imgui_demo.cpp \
             $(IMGUI)/backends/imgui_impl_glfw.cpp $(IMGUI)/backends/imgui_impl_opengl3.cpp
APP_OBJ   := $(APP_SRC:%.cpp=$(OUT)/obj/%.obj)
IMGUI_OBJ := $(IMGUI_SRC:%.cpp=$(OUT)/obj/%.obj)

CXXFLAGS := /nologo /std:c++20 /EHsc /permissive- /utf-8 /MD /FC /Isrc \
            /external:W0 /external:I$(IMGUI) /external:I$(IMGUI)/backends /external:I$(GLFW)/include
LDFLAGS  := /nologo /LIBPATH:$(GLFW)/lib-vc2022
LDLIBS   := glfw3.lib opengl32.lib gdi32.lib user32.lib shell32.lib

ifeq ($(CONFIG),debug)
  CXXFLAGS += /Od /Z7
  LDFLAGS  += /DEBUG /SUBSYSTEM:CONSOLE
else
  CXXFLAGS += /O2 /DNDEBUG
  LDFLAGS  += /SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup
endif

ifeq ($(filter clean,$(MAKECMDGOALS)),)
  ifeq ($(shell where cl 2>nul),)
    $(error cl.exe not on PATH. Use "Developer PowerShell for VS 2026" or: tools\vsdev.cmd make)
  endif
endif

mkdir_for = @if not exist "$(subst /,\,$(@D))" mkdir "$(subst /,\,$(@D))"

.PHONY: all debug run clean rebuild
all: $(TARGET)

debug:
	$(MAKE) CONFIG=debug

run: $(TARGET)
	$(subst /,\,$(TARGET))

clean:
	if exist build rmdir /s /q build

rebuild: clean
	$(MAKE) all

$(TARGET): $(APP_OBJ) $(IMGUI_OBJ)
	link $(LDFLAGS) /OUT:$@ $^ $(LDLIBS)

# ponytail: every app object depends on every src header; fine at this size.
$(APP_OBJ): $(OUT)/obj/%.obj: %.cpp $(APP_HDR)
	$(mkdir_for)
	cl $(CXXFLAGS) /W4 /c $< /Fo$@

$(IMGUI_OBJ): $(OUT)/obj/%.obj: %.cpp
	$(mkdir_for)
	cl $(CXXFLAGS) /W3 /c $< /Fo$@
