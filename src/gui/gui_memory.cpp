#include "../../include/gui/gui_memory.h"
#include "vm/main_memory.h"

size_t NUM_ROWS = 32;
uint64_t MEM_START_ADDRESS = 0;
uint64_t MEM_END_ADDRESS = MEM_START_ADDRESS + NUM_ROWS;

std::vector<uint64_t> MEMORY_VALUES;

std::string conv_uint64_to_hex(uint64_t val){
    std::stringstream ss;
    ss << std::hex << std::setfill('0') << std::setw(16) << val;
    std::string hex_string = "0x";
    hex_string += ss.str();
    return hex_string;
}

std::string conv_byte_to_bin(uint8_t val){
    std::bitset<8> binary(val);
    return binary.to_string();
}

void update_memory(){
    MEMORY_VALUES.clear();
    for(size_t i=0;i<NUM_ROWS;i++){
        MEMORY_VALUES.push_back(vm.ReadMemDoubleWord(MEM_START_ADDRESS + i*8));
    }
}


void memory_main(){
    static bool inited = false;
    if(!inited || GUI_MEMORY_DIRTY_BIT){
        update_memory();
        GUI_MEMORY_DIRTY_BIT = false;
    }

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

    const char* name_header = "ADDRESS";
    const char* byte_headers[8] = {"BYTE 0", "BYTE 1", "BYTE 2", "BYTE 3", "BYTE 4", "BYTE 5", "BYTE 6", "BYTE 7"};
    const char* value_header = "VALUE";

    ImU32 header_color = ImGui::ColorConvertFloat4ToU32({220.0f/255.0f, 100.0f/255.0f, 50.0f/255.0f, 1.0f});
    ImU32 row_color = ImGui::ColorConvertFloat4ToU32({1.0f, 1.0f, 1.0f, 1.0f});

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    ImVec2 memory_table_pos{WINDOW_POS.x + (WINDOW_SIZE.x - TABLE_SIZE.x) / 2.0f, WINDOW_POS.y + (WINDOW_SIZE.y - TABLE_SIZE.y) / 2.0f};
    ImGui::SetNextWindowPos(memory_table_pos);
    ImGui::BeginChild("Memory Table", TABLE_SIZE, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
    {
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
            for(size_t i=0;i<NUM_ROWS;i++){
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
                std::string mem_address = conv_uint64_to_hex(MEM_START_ADDRESS + i*8);
                float text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, mem_address.c_str()).x;
                row_cursor.x -= text_size/2.0f;
                draw_list->AddText(row_cursor, row_color, mem_address.c_str());
                row_cursor.x += text_size/2.0f;
                
                // bytes
                uint64_t mask = 0xFF;
                row_cursor.x += address_block_size_x / 2.0f;

                for(int j=0;j<8;j++){
                    uint8_t byte_val = (MEMORY_VALUES[i] & mask) >> j*8;
                    mask <<= 8;
                    std::string byte_str = conv_byte_to_bin(byte_val);

                    row_cursor.x += byte_block_size_x / 2.0f;
                    text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, byte_str.c_str()).x;
                    row_cursor.x -= text_size/2.0f;
                    draw_list->AddText(row_cursor, row_color, byte_str.c_str());
                    row_cursor.x += text_size/2.0f;
                    row_cursor.x += byte_block_size_x / 2.0f;
                }

                // value
                std::string val = conv_uint64_to_hex(MEMORY_VALUES[i]);
                
                row_cursor.x += value_block_size_x / 2.0f;
                text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, val.c_str()).x;
                row_cursor.x -= text_size/2.0f;
                draw_list->AddText(row_cursor, row_color, val.c_str());
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

        float total_height = NUM_ROWS * row_size_y + header_size_y + table_buffer_y;
        ImGui::SetCursorPosY(total_height);
        ImGui::Dummy(ImVec2(0.0f, 0.0f));

    }    
    ImGui::EndChild();
}



void parse_address(std::string address){
    bool invalid = (address[0]!='0' || address[1]!='x' || address.size()>18);

    auto error = []() -> void {
        globals::vm_cout_file << "The address you entered is not a valid input address." << std::endl;
        globals::vm_cout_file << "Make sure you entered the address in hex (Eg: 0x1000, 0x0000000000100100)" << std::endl;
    };

    if(invalid){
        error();
        return;
    }

    std::string value = address.substr(2);
    
    if(value.size()==0){
        MEM_START_ADDRESS = 0;
    }
    else{
        std::stringstream ss;
        ss << std::hex;
        ss << value;
        if(!(ss >> MEM_START_ADDRESS)){
            error();
            return;
        }
    }

    if(vm_config::config.getMemorySize() - NUM_ROWS < MEM_START_ADDRESS){
        MEM_START_ADDRESS = vm_config::config.getMemorySize() - NUM_ROWS * 8;
        globals::vm_cout_file << "The was address buffered to " << conv_uint64_to_hex(MEM_START_ADDRESS) << std::endl;
    }

    MEM_END_ADDRESS = MEM_START_ADDRESS + NUM_ROWS;
    GUI_MEMORY_DIRTY_BIT = true;
}


void memory_vars_main(){
    ImVec2 WINDOW_SIZE = ImGui::GetWindowSize();

    float input_text_height = WINDOW_SIZE.y * 0.5f;
    float input_text_width = WINDOW_SIZE.x * 0.4f;

    ImGui::SetCursorPos({(WINDOW_SIZE.x - input_text_width)*0.5f, (WINDOW_SIZE.y - input_text_height)*0.5f});
    ImGui::SetNextItemWidth(input_text_width);

    static char address_buffer[19];
    if(ImGui::InputText("Address", address_buffer, sizeof(address_buffer), ImGuiInputTextFlags_EnterReturnsTrue)){
        std::string address = address_buffer;
        parse_address(address);
    }
}



void memory_navigator_main(){
    ImVec2 WINDOW_SIZE = ImGui::GetWindowSize();

    ImVec2 button_size{WINDOW_SIZE.x * 0.2f, WINDOW_SIZE.x * 0.2f};
    float button_buffer = button_size.y;

    ImVec2 cursor_pos{WINDOW_SIZE.x * 0.5f - button_size.x, WINDOW_SIZE.y * 0.5f -  button_size.y - button_buffer};
    ImGui::SetCursorPos(cursor_pos);
    if(ImGui::Button("UP", {button_size.x, button_size.y})){
        if(MEM_START_ADDRESS<NUM_ROWS){
            MEM_START_ADDRESS = 0;
            GUI_MEMORY_DIRTY_BIT = true;
        }
        else{
            MEM_START_ADDRESS -= NUM_ROWS;
            GUI_MEMORY_DIRTY_BIT = true;
        }
    }

    cursor_pos.y = WINDOW_SIZE.y * 0.5f + button_buffer;
    ImGui::SetCursorPos(cursor_pos);
    if(ImGui::Button("DOWN", {button_size.x, button_size.y})){
        if(vm_config::config.getMemorySize() - NUM_ROWS < MEM_START_ADDRESS){
            MEM_START_ADDRESS = vm_config::config.getMemorySize() - NUM_ROWS * 8;
            GUI_MEMORY_DIRTY_BIT = true;
        }
        else{
            MEM_START_ADDRESS += NUM_ROWS;
            GUI_MEMORY_DIRTY_BIT = true;
        }
    }
}