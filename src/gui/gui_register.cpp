#include "../../include/gui/gui_register.h"
#include <bit>

int REGISTER_DISPLAY_IDX = 0;
const char* DISPLAY_TYPES[] = {"uint64_t", "int64_t", "hex", "float", "double"};

std::string conv_type(uint64_t val){
    switch(REGISTER_DISPLAY_IDX){
        case 0 : {
            return std::to_string(val);
        }
        case 1 : {
            int64_t val_ = static_cast<int64_t>(val);
            return std::to_string(val_);
        }
        case 2 : {
            std::stringstream ss;
            ss << std::hex << std::setfill('0') << std::setw(16) << val;
            std::string hex_string = "0x";
            hex_string += ss.str();
            return hex_string;
        }
        case 3 : {
            float f;
            std::memcpy(&f, &val, sizeof(f));
            return std::to_string(f);
        }
        case 4 : {
            double d;
            std::memcpy(&d, &val, sizeof(d));
            return std::to_string(d);
        }
    }

    return "whoopsies";
}


void draw_gpr_register_file(){
    static const std::string register_names[32] = {
        "x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7", "x8", "x9",
        "x10", "x11", "x12", "x13", "x14", "x15", "x16", "x17", "x18", "x19",
        "x20", "x21", "x22", "x23", "x24", "x25", "x26", "x27", "x28", "x29",
        "x30", "x31"
    };

    static const std::string register_alias[32] = {
        "zero",
        "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",
        "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "s2",
        "s3", "s4", "s5", "s6", "s7", "s8", "s9", "s10", "s11",
        "t3", "t4", "t5", "t6"
    };

    if(ImGui::BeginTable("GPR register file", 3, ImGuiTableFlags_Borders)){
        const char* name_header = " Name ";
        const char* alias_header = " Alias ";
        const char* value_header = " Value ";
        auto name_width = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, name_header, nullptr, nullptr).x;
        auto alias_width = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, alias_header, nullptr, nullptr).x;
        ImGui::TableSetupColumn(name_header, ImGuiTableColumnFlags_WidthFixed, name_width);
        ImGui::TableSetupColumn(alias_header, ImGuiTableColumnFlags_WidthFixed, alias_width);
        ImGui::TableSetupColumn(value_header);

        ImGui::TableHeadersRow();

        ImU32 alternate_dark_color = ImGui::ColorConvertFloat4ToU32({40.0f/255.0f, 40.0f/255.0f, 40.0f/255.0f, 1.0f});
    
        static constexpr size_t NUM_GPR = 32;
        const std::array<uint64_t, NUM_GPR>& gpr_registers = vm.GetGprValues();

        for(size_t i=0;i<NUM_GPR;i++){
            if(i%2==1){
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, alternate_dark_color);
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", register_names[i].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s",register_alias[i].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", conv_type(gpr_registers[i]).c_str());
        }

        ImGui::EndTable();
    }
}


void draw_fpr_register_file(){
    static const std::string register_names[32] = {
        "f0", "f1", "f2", "f3", "f4", "f5", "f6", "f7", "f8", "f9",
        "f10", "f11", "f12", "f13", "f14", "f15", "f16", "f17", "f18", "f19",
        "f20", "f21", "f22", "f23", "f24", "f25", "f26", "f27", "f28", "f29",
        "f30", "f31"
    };

    static const std::string register_alias[32] = {
        "ft0", "ft1", "ft2", "ft3", "ft4", "ft5", "ft6", "ft7",
        "fs0", "fs1", "fa0", "fa1", "fa2", "fa3", "fa4", "fa5",
        "fa6", "fa7", "fs2", "fs3", "fs4", "fs5", "fs6", "fs7",
        "fs8", "fs9", "fs10", "fs11", "ft8", "ft9", "ft10", "ft11"
    };


    if(ImGui::BeginTable("FPR register file", 3, ImGuiTableFlags_Borders)){
        const char* name_header = " Name ";
        const char* alias_header = " Alias ";
        const char* value_header = " Value ";
        auto name_width = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, name_header, nullptr, nullptr).x;
        auto alias_width = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, alias_header, nullptr, nullptr).x;
        ImGui::TableSetupColumn(name_header, ImGuiTableColumnFlags_WidthFixed, name_width);
        ImGui::TableSetupColumn(alias_header, ImGuiTableColumnFlags_WidthFixed, alias_width);
        ImGui::TableSetupColumn(value_header);

        ImGui::TableHeadersRow();
        
        ImU32 alternate_dark_color = ImGui::ColorConvertFloat4ToU32({40.0f/255.0f, 40.0f/255.0f, 40.0f/255.0f, 1.0f});

        static constexpr size_t NUM_FPR = 32;
        const std::array<uint64_t, NUM_FPR>& fpr_registers = vm.GetFprValues();

        for(size_t i=0;i<NUM_FPR;i++){
            if(i%2==1){
                ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, alternate_dark_color);
            }
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%s", register_names[i].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s",register_alias[i].c_str());
            ImGui::TableNextColumn();
            ImGui::Text("%s", conv_type(fpr_registers[i]).c_str());
        }

        ImGui::EndTable();
    }
}

void register_main(){
    ImVec2 WINDOW_POS = ImGui::GetWindowPos();
    ImVec2 WINDOW_SIZE = ImGui::GetWindowSize();

    ImVec2 register_file_selection_pos{WINDOW_POS.x, WINDOW_POS.y};
    ImVec2 register_file_selection_size{WINDOW_SIZE.x, WINDOW_SIZE.y * 0.15f};
    ImGui::SetNextWindowPos(register_file_selection_pos);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.4f, 0.5f, 1.0f));
    ImGui::BeginChild("Register File Selection Window", register_file_selection_size);
    {
        ImVec4 selection_button_colors{77.0f/255.0f, 77.0f/255.0f, 77.0f/255.0f, 1.0f};
        ImVec2 button_size{register_file_selection_size.x * 0.15f, register_file_selection_size.y * 0.4f};
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, selection_button_colors);
        {
            ImVec2 gpr_position{register_file_selection_size.x * 0.5f - button_size.x * 1.3f, register_file_selection_size.y * 0.3f};
            ImGui::SetCursorPos(gpr_position);
            if (ImGui::Button("GPR", button_size))
            {
                show_gpr = true;
                show_fpr = false;
            }
            
            ImVec2 fpr_position{register_file_selection_size.x * 0.5f + button_size.x * 0.3f, gpr_position.y};
            ImGui::SetCursorPos(fpr_position);
            if (ImGui::Button("FPR", button_size))
            {
                show_gpr = false;
                show_fpr = true;
            }
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();


    ImVec2 register_file_pos{WINDOW_POS.x + WINDOW_SIZE.x * 0.1f, WINDOW_POS.y + register_file_selection_size.y};
    ImVec2 register_file_size{WINDOW_SIZE.x * 0.89f, WINDOW_SIZE.y * 0.8f};
    ImGui::SetNextWindowPos(register_file_pos);
    ImGui::BeginChild("Register File", register_file_size);
    {
        if(show_gpr){
            draw_gpr_register_file();
        }
        else if(show_fpr){
            draw_fpr_register_file();
        }
    }
    ImGui::EndChild();


    ImVec2 register_display_type_pos(WINDOW_POS.x + WINDOW_SIZE.x * 0.1f, WINDOW_POS.y + register_file_selection_size.y + register_file_size.y);
    ImVec2 register_display_type_size(WINDOW_SIZE.x * 0.89f, WINDOW_SIZE.y - register_file_size.y - register_file_selection_size.y);
    ImGui::SetNextWindowPos(register_display_type_pos);
    ImGui::BeginChild("Register Display Type Selection", register_display_type_size);
    {
        // ImVec2 cursor_pos{register_display_type_pos.x + register_display_type_size.x/2.0f, register_display_type_pos.y + register_display_type_size.y/2.0f};
        // ImGui::SetCursorPos(cursor_pos);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(100.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f)); // Lighter background
        if(ImGui::BeginCombo("Selection Combo", DISPLAY_TYPES[REGISTER_DISPLAY_IDX]))
        {
            for(int i=0;i<IM_ARRAYSIZE(DISPLAY_TYPES);i++){
                const bool is_selected = (REGISTER_DISPLAY_IDX == i);

                if(ImGui::Selectable(DISPLAY_TYPES[i], is_selected)){
                    REGISTER_DISPLAY_IDX = i;
                }

                if(is_selected){
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();

}


void register_execute(){
    ImVec2 WINDOW_POS = ImGui::GetWindowPos();
    ImVec2 WINDOW_SIZE = ImGui::GetWindowSize();

    ImVec2 register_file_selection_pos{WINDOW_POS.x, WINDOW_POS.y};
    ImVec2 register_file_selection_size{WINDOW_SIZE.x, WINDOW_SIZE.y * 0.1f};
    ImGui::SetNextWindowPos(register_file_selection_pos);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1f, 0.4f, 0.5f, 1.0f));
    ImGui::BeginChild("Register File Selection Window", register_file_selection_size);
    {
        ImVec4 selection_button_colors{77.0f/255.0f, 77.0f/255.0f, 77.0f/255.0f, 1.0f};
        ImVec2 button_size{register_file_selection_size.x * 0.15f, register_file_selection_size.y * 0.3f};
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, selection_button_colors);
        {
            ImVec2 gpr_position{register_file_selection_size.x * 0.5f - button_size.x * 1.3f, register_file_selection_size.y * 0.3f};
            ImGui::SetCursorPos(gpr_position);
            if (ImGui::Button("GPR", button_size))
            {
                show_gpr = true;
                show_fpr = false;
            }
            
            ImVec2 fpr_position{register_file_selection_size.x * 0.5f + button_size.x * 0.3f, gpr_position.y};
            ImGui::SetCursorPos(fpr_position);
            if (ImGui::Button("FPR", button_size))
            {
                show_gpr = false;
                show_fpr = true;
            }
        }
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }
    ImGui::PopStyleColor();
    ImGui::EndChild();


    ImVec2 register_file_pos{WINDOW_POS.x + WINDOW_SIZE.x * 0.1f, WINDOW_POS.y + register_file_selection_size.y};
    ImVec2 register_file_size{WINDOW_SIZE.x * 0.89f, WINDOW_SIZE.y * 0.85f};
    ImGui::SetNextWindowPos(register_file_pos);
    ImGui::PushFont(STANDARD_MEDIUM_FONT);
    ImGui::BeginChild("Register File", register_file_size);
    {
        if(show_gpr){
            draw_gpr_register_file();
        }
        else if(show_fpr){
            draw_fpr_register_file();
        }
    }
    ImGui::EndChild();
    ImGui::PopFont();


    ImVec2 register_display_type_pos(WINDOW_POS.x + WINDOW_SIZE.x * 0.1f, WINDOW_POS.y + register_file_selection_size.y + register_file_size.y);
    ImVec2 register_display_type_size(WINDOW_SIZE.x * 0.89f, WINDOW_SIZE.y - register_file_size.y - register_file_selection_size.y);
    ImGui::SetNextWindowPos(register_display_type_pos);
    ImGui::BeginChild("Register Display Type Selection", register_display_type_size);
    {
        // ImVec2 cursor_pos{register_display_type_pos.x + register_display_type_size.x/2.0f, register_display_type_pos.y + register_display_type_size.y/2.0f};
        // ImGui::SetCursorPos(cursor_pos);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(100.0f/255.0f, 100.0f/255.0f, 100.0f/255.0f, 1.0f)); // Lighter background
        if(ImGui::BeginCombo("Display Type", DISPLAY_TYPES[REGISTER_DISPLAY_IDX]))
        {
            for(int i=0;i<IM_ARRAYSIZE(DISPLAY_TYPES);i++){
                const bool is_selected = (REGISTER_DISPLAY_IDX == i);

                if(ImGui::Selectable(DISPLAY_TYPES[i], is_selected)){
                    REGISTER_DISPLAY_IDX = i;
                }

                if(is_selected){
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }
        ImGui::PopStyleColor();
    }
    ImGui::EndChild();
}