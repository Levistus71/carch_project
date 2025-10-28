#include "../../include/gui/gui_memory.h"


void memory_main(){
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

    // size_t num_rows = static_cast<int>((TABLE_SIZE.y-header_size_y-table_buffer_y) / row_size_y);
    size_t num_rows = 32;

    const char* name_header = "ADDRESS";
    const char* byte_headers[8] = {"BYTE 0", "BYTE 1", "BYTE 2", "BYTE 3", "BYTE 4", "BYTE 5", "BYTE 6", "BYTE 7"};
    const char* value_header = "VALUE";

    ImU32 header_color = ImGui::ColorConvertFloat4ToU32({220.0f/255.0f, 100.0f/255.0f, 50.0f/255.0f, 1.0f});
    ImU32 row_color = ImGui::ColorConvertFloat4ToU32({1.0f, 1.0f, 1.0f, 1.0f});

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2 memory_table_pos{WINDOW_POS.x + (WINDOW_SIZE.x - TABLE_SIZE.x) / 2.0f, WINDOW_POS.y + (WINDOW_SIZE.y - TABLE_SIZE.y) / 2.0f};
    ImGui::SetNextWindowPos(memory_table_pos);
    if(ImGui::BeginChild("Memory Table", TABLE_SIZE, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize)){
        ImVec2 TABLE_POS = ImGui::GetWindowPos();

        ImVec2 cursor_ = ImGui::GetCursorPos();
        cursor_.x += TABLE_POS.x;
        cursor_.y += TABLE_POS.y;

        ImVec2 row_cursor{cursor_};
        ImVec2 header_cursor{cursor_};

        header_cursor.y += table_buffer_y;

        row_cursor.y += table_buffer_y + header_size_y;
        row_cursor.y -= ImGui::GetScrollY();

        float header_start_y = header_cursor.y;
        float cursor_start_x = row_cursor.x;

        // loading the rows
        ImU32 alternate_dark_color = ImGui::ColorConvertFloat4ToU32({20.0f/255.0f, 20.0f/255.0f, 20.0f/255.0f, 1.0f});
        ImU32 alternate_light_color = ImGui::ColorConvertFloat4ToU32({70.0f/255.0f, 70.0f/255.0f, 70.0f/255.0f, 1.0f});
        ImGui::PushFont(row_font);
        {
            for(size_t i=0;i<num_rows;i++){
                if(row_cursor.y + row_size_y <= header_start_y + header_size_y){
                    row_cursor.y += row_size_y;
                    continue;
                }

                if (row_cursor.y > TABLE_POS.y + TABLE_SIZE.y){
                    continue;
                }

                row_cursor.x = cursor_start_x;

                // darker bg for even rows
                float t = (row_size_y - ImGui::GetFontSize())/2.0f;
                ImVec2 top_left = {row_cursor.x, row_cursor.y - t};
                ImVec2 bottom_right = {row_cursor.x + TABLE_SIZE.x * 0.98f, row_cursor.y + t + ImGui::GetFontSize()};
                if(i%2==0){
                    draw_list->AddRectFilled(top_left, bottom_right, alternate_dark_color);
                }
                else {
                    draw_list->AddRectFilled(top_left, bottom_right, alternate_light_color);
                }
                
                // address
                row_cursor.x += address_block_size_x / 2.0f;
                float text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "0x8888888888888888").x;
                row_cursor.x -= text_size/2.0f;
                draw_list->AddText(row_cursor, row_color, "0x8888888888888888");
                row_cursor.x += text_size/2.0f;
                
                // bytes
                row_cursor.x += address_block_size_x / 2.0f;
                for(int i=0;i<8;i++){
                    row_cursor.x += byte_block_size_x / 2.0f;
                    text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "88888888").x;
                    row_cursor.x -= text_size/2.0f;
                    draw_list->AddText(row_cursor, row_color, "88888888");
                    row_cursor.x += text_size/2.0f;
                    row_cursor.x += byte_block_size_x / 2.0f;
                }

                // value
                row_cursor.x += value_block_size_x / 2.0f;
                text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, "0x8888888888888888").x;
                row_cursor.x -= text_size/2.0f;
                draw_list->AddText(row_cursor, row_color, "0x8888888888888888");
                row_cursor.x += text_size/2.0f;
                row_cursor.x += value_block_size_x / 2.0f;
                
                row_cursor.y += row_size_y;
            }
        }
        ImGui::PopFont();


        // loading the headers
        ImGui::PushFont(header_font);
        draw_list->AddRectFilled(cursor_, {cursor_.x + TABLE_SIZE.x * 0.98f, cursor_.y + header_size_y}, ImGui::ColorConvertFloat4ToU32({0.0f, 0.0f, 0.0f, 1.0f}));

        header_cursor.x += address_block_size_x / 2.0f;
        float text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, name_header).x;
        header_cursor.x -= text_size/2.0f;
        draw_list->AddText(header_cursor, header_color, name_header);
        header_cursor.x += text_size/2.0f;
        
        header_cursor.x += address_block_size_x / 2.0f;
        for(int i=0;i<8;i++){
            header_cursor.x += byte_block_size_x / 2.0f;
            text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, byte_headers[i]).x;
            header_cursor.x -= text_size/2.0f;
            draw_list->AddText(header_cursor, header_color, byte_headers[i]);
            header_cursor.x += text_size/2.0f;
            header_cursor.x += byte_block_size_x / 2.0f;
        }

        text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, value_header).x;
        header_cursor.x += value_block_size_x / 2.0f;
        header_cursor.x -= text_size / 2.0f;
        draw_list->AddText(header_cursor, header_color, value_header);
        header_cursor.x += text_size / 2.0f;
        header_cursor.x += value_block_size_x / 2.0f;

        ImGui::PopFont();

        float total_height = num_rows * row_size_y + header_size_y + table_buffer_y;
        ImGui::SetCursorPosY(total_height);
        ImGui::Dummy(ImVec2(0.0f, 0.0f));

        ImGui::EndChild();
    }
}