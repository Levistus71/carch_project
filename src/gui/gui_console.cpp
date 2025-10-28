#include "../include/gui/gui_console.h"
// #include <iostream>

void console_main(){
    static TextEditor console;
    console.SetReadOnly(true);
    console.SetHandleKeyboardInputs(false);
    console.SetHandleMouseInputs(false);
    console.SetColorizerEnable(false);
    console.SetShowWhitespaces(false);
    console.SetShowLineNumbers(false);
    console.SetPalette(console.GetConsolePalette());
    console.SetLanguageDefinition(console.mConsoleLangDef);
    console.TieToFileAndRead("./vm_state/vm_cout.txt");

    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 size = {window_size.x, window_size.y};
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // loading gray color for editor bg
    ImGui::BeginChild("Console", size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {        
        ImVec2 text_area_size = {window_size.x, window_size.y};
        ImVec2 text_area_pos = {0.0f, 0.0f};
        ImGui::SetCursorPos(text_area_pos);

        ImGui::PushFont(EDITOR_MEDIUM_FONT);

        console.Render("Console", text_area_size, true);

        ImGui::PopFont();
    }
    ImGui::PopStyleColor();
    ImGui::EndChild(); 
}