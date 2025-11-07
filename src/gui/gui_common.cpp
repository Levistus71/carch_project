#include "../include/gui/gui_common.h"

ImFont* STANDARD_SMALL_FONT;
ImFont* STANDARD_BOLD_SMALL_FONT;
ImFont* STANDARD_MEDIUM_FONT;
ImFont* STANDARD_BOLD_MEDIUM_FONT;
ImFont* EDITOR_SMALL_FONT;
ImFont* EDITOR_MEDIUM_FONT;
ImFont* EDITOR_LARGE_FONT;

bool in_editor = false;
bool in_processor = true;
bool in_execute = false;
bool in_memory = false;

bool show_gpr = true;
bool show_fpr = false;

VM vm;

void LoadFonts(ImGuiIO& io){
    STANDARD_SMALL_FONT = io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/Roboto-Medium.ttf", 15.0f);
    STANDARD_MEDIUM_FONT = io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/Roboto-Medium.ttf", 17.0f);
    STANDARD_BOLD_SMALL_FONT = io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/open-sans/OpenSans-Bold.ttf", 15.0f);
    STANDARD_BOLD_MEDIUM_FONT = io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/open-sans/OpenSans-Bold.ttf", 17.0f);

    EDITOR_SMALL_FONT = io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/Karla-Regular.ttf", 15.0f);
    EDITOR_MEDIUM_FONT = io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/Karla-Regular.ttf", 20.0f);
    EDITOR_LARGE_FONT = io.Fonts->AddFontFromFileTTF("../imgui/misc/fonts/Karla-Regular.ttf", 25.0f);

    io.Fonts->Build();
}