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

#include "../../include/gui/gui_processor_window.h"
#include "../../include/gui/gui_editor.h"
#include "../../include/gui/gui_common.h"
#include "../../include/gui/gui_register.h"
#include "../../include/gui/gui_console.h"
#include "../../include/gui/gui_execute.h"
#include "../../include/gui/gui_memory.h"

#include "../../include/vm/rv5s/rv5s_vm.h"
#include "../../include/assembler/assembler.h"

#include "globals.h"


int gui_main();