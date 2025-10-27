#include "../../include/gui/gui_memory.h"


void draw_memory(){
    ImVec2 WINDOW_SIZE = ImGui::GetWindowSize();
    ImVec2 WINDOW_POS = ImGui::GetWindowPos();
    ImVec2 TABLE_SIZE{WINDOW_SIZE.x * 0.99f, WINDOW_SIZE.y * 0.99f};

    auto header_font = STANDARD_MEDIUM_FONT;
    auto row_font = STANDARD_SMALL_FONT;

    float address_block_size_x= TABLE_SIZE.x / 6.0f;
    float value_block_size_x= TABLE_SIZE.x / 6.0f;
    float byte_block_size_x = TABLE_SIZE.x / 12.0f;
    float row_size_y = 30.0f;
    float header_size_y = 35.0f;

    ImGui::PushFont(header_font);
    float table_buffer_y = (header_size_y - ImGui::GetFontSize())/2.0f;
    ImGui::PopFont();

    size_t num_rows = static_cast<int>((TABLE_SIZE.y-header_size_y-table_buffer_y) / row_size_y);
    // size_t num_rows = 32;

    const char* name_header = "ADDRESS";
    const char* byte_headers[8] = {"BYTE 0", "BYTE 1", "BYTE 2", "BYTE 3", "BYTE 4", "BYTE 5", "BYTE 6", "BYTE 7"};
    const char* value_header = "VALUE";

    ImU32 header_color = ImGui::ColorConvertFloat4ToU32({220.0f/255.0f, 100.0f/255.0f, 50.0f/255.0f, 1.0f});
    ImU32 row_color = ImGui::ColorConvertFloat4ToU32({1.0f, 1.0f, 1.0f, 1.0f});

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2 memory_table_pos{WINDOW_POS.x + (WINDOW_SIZE.x - TABLE_SIZE.x) / 2.0f, WINDOW_POS.y + (WINDOW_SIZE.y - TABLE_SIZE.y) / 2.0f};
    ImGui::SetNextWindowPos(memory_table_pos);
    if(ImGui::BeginChild("Memory Table", TABLE_SIZE)){
        ImVec2 TABLE_POS = ImGui::GetWindowPos();

        ImVec2 cursor = ImGui::GetCursorPos();
        cursor.x += TABLE_POS.x;
        cursor.y += TABLE_POS.y;
        cursor.y += table_buffer_y;

        float cursor_start_x = cursor.x;

        // loading the headers
        ImGui::PushFont(header_font);

        ImU32 alternate_dark_color = ImGui::ColorConvertFloat4ToU32({20.0f/255.0f, 20.0f/255.0f, 20.0f/255.0f, 1.0f});
        ImU32 alternate_light_color = ImGui::ColorConvertFloat4ToU32({70.0f/255.0f, 70.0f/255.0f, 70.0f/255.0f, 1.0f});

        cursor.x += address_block_size_x / 2.0f;
        float text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, name_header).x;
        cursor.x -= text_size/2.0f;
        draw_list->AddText(cursor, header_color, name_header);
        cursor.x += text_size/2.0f;
        
        cursor.x += address_block_size_x / 2.0f;
        for(int i=0;i<8;i++){
            cursor.x += byte_block_size_x / 2.0f;
            text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, byte_headers[i]).x;
            cursor.x -= text_size/2.0f;
            draw_list->AddText(cursor, header_color, byte_headers[i]);
            cursor.x += text_size/2.0f;
            cursor.x += byte_block_size_x / 2.0f;
        }

        text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, value_header).x;
        cursor.x += value_block_size_x / 2.0f;
        cursor.x -= text_size / 2.0f;
        draw_list->AddText(cursor, header_color, value_header);
        cursor.x += text_size / 2.0f;
        cursor.x += value_block_size_x / 2.0f;

        ImGui::PopFont();

        // loading the rows
        ImGui::PushFont(row_font);
        cursor.y += header_size_y;
        
        for(size_t i=0;i<num_rows;i++){
            cursor.x = cursor_start_x;

            // darker bg for even rows
            if(i%2==0){
                float t = (row_size_y - ImGui::GetFontSize())/2.0f;
                ImVec2 top_left = {cursor.x, cursor.y - t};
                ImVec2 bottom_right = {cursor.x + TABLE_SIZE.x, cursor.y + t + ImGui::GetFontSize()};
                draw_list->AddRectFilled(top_left, bottom_right, alternate_dark_color);
            }
            else {
                float t = (row_size_y - ImGui::GetFontSize())/2.0f;
                ImVec2 top_left = {cursor.x, cursor.y - t};
                ImVec2 bottom_right = {cursor.x + TABLE_SIZE.x, cursor.y + t + ImGui::GetFontSize()};
                draw_list->AddRectFilled(top_left, bottom_right, alternate_light_color);
            }
            
            // address
            cursor.x += address_block_size_x / 2.0f;
            text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "0x0000000000000000").x;
            cursor.x -= text_size/2.0f;
            draw_list->AddText(cursor, row_color, "0x0000000000000000");
            cursor.x += text_size/2.0f;
            
            // bytes
            cursor.x += address_block_size_x / 2.0f;
            for(int i=0;i<8;i++){
                cursor.x += byte_block_size_x / 2.0f;
                text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "00000000").x;
                cursor.x -= text_size/2.0f;
                draw_list->AddText(cursor, row_color, "00000000");
                cursor.x += text_size/2.0f;
                cursor.x += byte_block_size_x / 2.0f;
            }

            // value
            cursor.x += value_block_size_x / 2.0f;
            text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "0x0000000000000000").x;
            cursor.x -= text_size/2.0f;
            draw_list->AddText(cursor, row_color, "0x0000000000000000");
            cursor.x += text_size/2.0f;
            cursor.x += value_block_size_x / 2.0f;
            
            cursor.y += row_size_y;
        }
        ImGui::PopFont();

        ImGui::EndChild();
    }
}


void memory_main(){
    ImVec2 WINDOW_POS = ImGui::GetWindowPos();
    ImVec2 WINDOW_SIZE = ImGui::GetWindowSize();

    ImVec2 memory_window_size{WINDOW_SIZE.x * 0.9f, WINDOW_SIZE.y * 0.8f};
    ImVec2 memory_window_pos{WINDOW_POS.x + memory_window_size.x*0.02f, WINDOW_POS.y + memory_window_size.x*0.02f};
    ImGui::SetNextWindowPos(memory_window_pos);

    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p_min{memory_window_pos};
    ImVec2 p_max{memory_window_pos.x + memory_window_size.x, memory_window_pos.y + memory_window_size.y};
    draw_list->AddRectFilled(p_min, p_max, ImGui::ColorConvertFloat4ToU32({50.0f/255.0f, 50.0f/255.0f, 50.0f/255.0f, 1.0f}));
    ImGui::BeginChild("Memory Window", memory_window_size);
    {
        draw_memory();
    }
    ImGui::EndChild();



    ImVec2 memory_vars_pos{WINDOW_POS.x, WINDOW_POS.y + memory_window_size.y};
    ImVec2 memory_vars_size{WINDOW_SIZE.x, WINDOW_SIZE.y - memory_window_size.y};
    ImGui::BeginChild("Memory Window Variables", memory_vars_size);
    {

    }
    ImGui::EndChild();
}