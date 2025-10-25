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

extern ImFont* STANDARD_FONT;
extern ImFont* EDITOR_SMALL_FONT;
extern ImFont* EDITOR_MEDIUM_FONT;
extern ImFont* EDITOR_LARGE_FONT;

void LoadFonts(ImGuiIO& io);