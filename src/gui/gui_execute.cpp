#include "../../include/gui/gui_execute.h"

void editor_execute(){
	ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 size = {window_size.x * 0.99f, window_size.y * 0.99f};

	text_editor.SetLanguageDefinition(text_editor.mRiscVLangDef);
	text_editor.SetShowWhitespaces(false);
	text_editor.SetReadOnly(true);

    ImGui::BeginChild("Editor Read Only", size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {        
        ImVec2 text_area_size = {window_size.x * 0.98f, window_size.y * 0.98f};
        ImVec2 text_area_pos = {0.0f, 0.0f};
        ImGui::SetCursorPos(text_area_pos);

        ImGui::PushFont(EDITOR_MEDIUM_FONT);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f));

        text_editor.SetDebugMode(true);
        text_editor.SetDebugModeTypeSingleCycle(false);
		text_editor.SetDebugLines(3, 4, 5, 6, 7);

        text_editor.Render("Editor Read Only", text_area_size, true);

        ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    ImGui::EndChild();
}

static TextEditor assembled_editor{};

void assembled_editor_main(){
    ImVec2 window_size = ImGui::GetWindowSize();
    ImVec2 size = {window_size.x * 0.99f, window_size.y * 0.99f};

	assembled_editor.SetReadOnly(true);
    assembled_editor.SetColorizerEnable(false);
    assembled_editor.SetHandleKeyboardInputs(false);
    assembled_editor.SetHandleMouseInputs(false);
    assembled_editor.SetLanguageDefinition(assembled_editor.mConsoleLangDef);
    assembled_editor.SetPalette(assembled_editor.GetDarkPalette());
    assembled_editor.SetShowWhitespaces(false);
    assembled_editor.SetShowLineNumbers(true);
    assembled_editor.TieToFileAndRead("./vm_state/disassembly.txt");

    ImGui::BeginChild("Assembled Editor Read Only", size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
    {        
        ImVec2 text_area_size = {window_size.x * 0.98f, window_size.y * 0.98f};
        ImVec2 text_area_pos = {0.0f, 0.0f};
        ImGui::SetCursorPos(text_area_pos);

        ImGui::PushFont(EDITOR_MEDIUM_FONT);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f));

        assembled_editor.Render("Assembled Editor Read Only", text_area_size, true);

        ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    ImGui::EndChild();
}