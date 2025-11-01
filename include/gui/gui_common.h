#pragma once

#include "imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <algorithm>
#include <vector>

#include "globals.h"
#include "vm/rv5s/vm.h"

extern ImFont* STANDARD_SMALL_FONT;
extern ImFont* STANDARD_MEDIUM_FONT;
extern ImFont* EDITOR_SMALL_FONT;
extern ImFont* EDITOR_MEDIUM_FONT;
extern ImFont* EDITOR_LARGE_FONT;

extern bool in_editor;
extern bool in_processor;
extern bool in_execute;
extern bool in_memory;

extern bool show_gpr;
extern bool show_fpr;

extern rv5s::VM vm;

void LoadFonts(ImGuiIO& io);