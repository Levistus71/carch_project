#include "../../include/gui/gui_execute.h"
#include "vm/rv5s/core/core.h"

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
        if(!vm.PipeliningEnabled()){
            text_editor.SetDebugModeTypeSingleCycle(true);
            size_t text_line_number = vm.program_.instruction_number_line_number_mapping[vm.GetInstructionPCs()[0] / 4];
            text_editor.SetDebugLines(text_line_number-1, -1, -1, -1, -1);
            text_editor.SetCursorPosition({static_cast<int>(text_line_number),0});
        }
        else{
            text_editor.SetDebugModeTypeSingleCycle(false);
            // std::vector<uint64_t> pcs = vm.GetInstructionPCs();
            // for(int i=0;i<5;i++)
            //     pcs[i] = vm.program_.instruction_number_line_number_mapping[pcs[i] / 4];
            // text_editor.SetDebugLines(pcs[0]-1, pcs[1]-1, pcs[2]-1, pcs[3]-1, pcs[4]-1);
            // text_editor.SetCursorPosition({static_cast<int>(pcs[3]),0});

            std::vector<std::reference_wrapper<const rv5s::InstrContext>> instructions = vm.GetInstructions();
            std::vector<uint64_t> line_numbers(5);

            for(int i=0;i<5;i++){
                const rv5s::InstrContext& instruction = instructions[i].get();
                if(instruction.bubbled || instruction.nopped){
                    line_numbers[i] = -1;
                }
                else{
                    uint64_t instr_pc = instruction.pc;
                    line_numbers[i] = vm.program_.instruction_number_line_number_mapping[instr_pc/4] - 1;
                }
            }

            text_editor.SetDebugLines(line_numbers[0], line_numbers[1], line_numbers[2], line_numbers[3], line_numbers[4]);
            if(static_cast<int>(line_numbers[2])>=0){
                text_editor.SetCursorPosition({static_cast<int>(line_numbers[2]),0});
            }
            else{
                text_editor.SetCursorPosition({0,0});
            }
        }
        

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


        assembled_editor.SetDebugMode(true);
        if(!vm.PipeliningEnabled()){
            assembled_editor.SetDebugModeTypeSingleCycle(true);
            size_t text_line_number = vm.program_.instruction_number_disassembly_mapping[vm.GetInstructionPCs()[0] / 4];
            assembled_editor.SetDebugLines(text_line_number-1, -1, -1, -1, -1);
            assembled_editor.SetCursorPosition({static_cast<int>(text_line_number), 0});
        }
        else{
            assembled_editor.SetDebugModeTypeSingleCycle(false);
            // std::vector<uint64_t> pcs = vm.GetInstructionPCs();
            // for(int i=0;i<5;i++)
            //     pcs[i] = vm.program_.instruction_number_disassembly_mapping[pcs[i] / 4];
            // assembled_editor.SetDebugLines(pcs[0]-1, pcs[1]-1, pcs[2]-1, pcs[3]-1, pcs[4]-1);
            // assembled_editor.SetCursorPosition({static_cast<int>(pcs[3]),0});
            
            std::vector<std::reference_wrapper<const rv5s::InstrContext>> instructions = vm.GetInstructions();
            std::vector<uint64_t> line_numbers(5);

            for(int i=0;i<5;i++){
                const rv5s::InstrContext& instruction = instructions[i].get();
                if(instruction.bubbled || instruction.nopped){
                    line_numbers[i] = -1;
                }
                else{
                    uint64_t instr_pc = instruction.pc;
                    line_numbers[i] = vm.program_.instruction_number_disassembly_mapping[instr_pc/4] - 1;
                }
            }

            assembled_editor.SetDebugLines(line_numbers[0], line_numbers[1], line_numbers[2], line_numbers[3], line_numbers[4]);
            if(static_cast<int>(line_numbers[2])>=0){
                assembled_editor.SetCursorPosition({static_cast<int>(line_numbers[2]),0});
            }
            else{
                assembled_editor.SetCursorPosition({0,0});
            }
        }

        assembled_editor.Render("Assembled Editor Read Only", text_area_size, true);

        ImGui::PopStyleColor();
        ImGui::PopFont();
    }
    ImGui::EndChild();
}