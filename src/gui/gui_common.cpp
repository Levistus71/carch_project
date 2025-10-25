#include "../include/gui/gui_common.h"

ImFont* STANDARD_FONT;
ImFont* EDITOR_SMALL_FONT;
ImFont* EDITOR_MEDIUM_FONT;
ImFont* EDITOR_LARGE_FONT;

void LoadFonts(ImGuiIO& io){
    STANDARD_FONT = io.Fonts->AddFontFromFileTTF("../../../imgui/misc/fonts/Roboto-Medium.ttf", 15.0f);
    EDITOR_SMALL_FONT = io.Fonts->AddFontFromFileTTF("../../../imgui/misc/fonts/Karla-Regular.ttf", 15.0f);
    EDITOR_MEDIUM_FONT = io.Fonts->AddFontFromFileTTF("../../../imgui/misc/fonts/Karla-Regular.ttf", 20.0f);
    EDITOR_LARGE_FONT = io.Fonts->AddFontFromFileTTF("../../../imgui/misc/fonts/Karla-Regular.ttf", 25.0f);

    io.Fonts->Build();
}