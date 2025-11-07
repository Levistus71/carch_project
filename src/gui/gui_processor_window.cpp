#include "../include/gui/gui_processor_window.h"
#include "gui/gui_common.h"
#include "sim_state.h"
#include "vm/rv5s/pipelined/core/instruction_context/instruction_context.h"

// alu struct, stores the top_left_coords of the alu and the height of the alu.
struct AluStruct{
    ImVec2 top_left_coords;
    float height;

    AluStruct(){}
    AluStruct(ImVec2 im, float h) : top_left_coords{im}, height{h}{}
    AluStruct(const AluStruct& other_alu){
        top_left_coords = other_alu.top_left_coords;
        height = other_alu.height;
    }
    AluStruct(AluStruct&& other_alu){
        if(this!=&other_alu){
            top_left_coords = std::move(other_alu.top_left_coords);
            height = other_alu.height;

            other_alu.top_left_coords = ImVec2();
        }
    }
    AluStruct operator=(const AluStruct& other_alu){
        top_left_coords = other_alu.top_left_coords;
        height = other_alu.height;
        return *this;
    }
    AluStruct operator=(AluStruct&& other_alu) noexcept {
        if(this!=&other_alu){
            top_left_coords = other_alu.top_left_coords;
            height = other_alu.height;

            other_alu.top_left_coords = ImVec2();
        }
        return *this;
    }
    ~AluStruct(){}

    inline float get_left_height() {return 0.444f * height;}
    inline float get_right_height() {return height/3;}
    inline float get_width() {return height/2;}
    inline float get_indent() {return height/12;}

    inline ImVec2 get_output_line_coords(){return {top_left_coords.x + get_width(), top_left_coords.y + height/2};}
    inline ImVec2 get_input1_line_coords(){return {top_left_coords.x, top_left_coords.y + height/4.5f};}
    inline ImVec2 get_input2_line_coords(){return {top_left_coords.x, top_left_coords.y + height - height/4.5f};}
};


// rectangle struct
struct Rectangle{
    ImVec2 top_left;
    ImVec2 bottom_right;

    Rectangle(){}
    Rectangle(ImVec2 top_left, ImVec2 bottom_right) : top_left{top_left}, bottom_right{bottom_right}{}
    Rectangle(const Rectangle& other_rect){
        top_left = other_rect.top_left;
        bottom_right = other_rect.bottom_right;
    }
    Rectangle(Rectangle&& other_rect){
        if(this!=&other_rect){
            top_left = std::move(other_rect.top_left);
            bottom_right = std::move(other_rect.bottom_right);

            other_rect.top_left = ImVec2();
            other_rect.bottom_right = ImVec2();
        }
    }
    Rectangle operator=(const Rectangle& other_rect){
        top_left = other_rect.top_left;
        bottom_right = other_rect.bottom_right;
        return *this;
    }
    Rectangle operator=(Rectangle&& other_rect){
        if(this!=&other_rect){
            top_left = std::move(other_rect.top_left);
            bottom_right = std::move(other_rect.bottom_right);

            other_rect.top_left = ImVec2();
            other_rect.bottom_right = ImVec2();
        }
        return *this;
    }

    inline float get_height(){return bottom_right.y - top_left.y;}
};


void draw_alu(ImDrawList* draw_list, AluStruct& alu, ImU32 color, float thickness){
    ImVec2& top_left_coords = alu.top_left_coords;

    float left_height = alu.get_left_height();
    float right_height = alu.get_right_height();
    float width = alu.get_width();
    float indent = alu.get_indent();

    ImVec2 points[] = {top_left_coords,
        {top_left_coords.x, top_left_coords.y + left_height},
        {top_left_coords.x + indent, top_left_coords.y + left_height + indent},
        {top_left_coords.x, top_left_coords.y + left_height + 2*indent},
        {top_left_coords.x, top_left_coords.y + 2*left_height + 2*indent},
        {top_left_coords.x + width, top_left_coords.y + 2*right_height},
        {top_left_coords.x + width, top_left_coords.y + right_height}
    };

    for(int i=0;i<6;i++){
        draw_list->AddLine(points[i],points[i+1],color,thickness);
    }
    draw_list->AddLine(points[6],points[0],color,thickness);
}


void draw_line_connecting_points(std::vector<ImVec2>& points, ImU32 color, float thickness){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for(size_t i=0;i<points.size()-1;i++){
        draw_list->AddLine(points[i],points[i+1],color,thickness);
    }
}


// All the data passed between functions
struct WindowConfig{
    ImVec2 WINDOW_SIZE = ImGui::GetContentRegionAvail();
    ImVec2 WINDOW_POS = ImGui::GetWindowPos();

    float processor_height;
    float instr_text_height;

    float stage_height = WINDOW_SIZE.y;
    float thickness = 2.0f;
    ImU32 col = ImGui::GetColorU32(ImVec4(255,255,255,255));

    ImVec2 exec_window_pos;
    ImVec2 mem_window_pos;

    AluStruct pc_4_adder;
    AluStruct branch_alu;
    AluStruct main_alu;
    
    Rectangle pc;
    Rectangle instruction_mem;
    Rectangle decoder;
    Rectangle register_file;
    Rectangle main_mem;

    Rectangle pc_adder_mux;
    Rectangle alu_mux;
    Rectangle wb_mux;
    
    ImVec2 immediate_gen_center;
    float immediate_gen_hradius;
    ImVec2 branch_shift_center;
    float branch_shift_radius;

    float IF_STAGE_WIDTH_FRAC = 0.25f;
    float ID_STAGE_WIDTH_FRAC = 0.27f;
    float EX_STAGE_WIDTH_FRAC = 0.17f;
    float MEM_STAGE_WIDTH_FRAC = 0.2f;
    float WB_STAGE_WIDTH_FRAC = 0.1f;

    float pipeline_registers_width = 10.0f;
    float hazard_detector_width = 70.0f;
    float hazard_detector_height = 30.0f;
    float data_forwarding_unit_width = 70.0f;
    float data_forwarding_unit_height = 30.0f;
    float hardware_buffer = 15.0f;

    Rectangle data_forwarding_unit;
    Rectangle hazard_detector;
    Rectangle alu_mux_forward;


    ImU32 red_col = ImGui::ColorConvertFloat4ToU32({200.0f/255.0f, 40.0f/255.0f, 25.0f/255.0f, 1.0f});
    ImU32 white_col = ImGui::ColorConvertFloat4ToU32({1.0f, 1.0f, 1.0f, 1.0f});
    ImU32 yellow_col = ImGui::ColorConvertFloat4ToU32({200.0f/255.0f, 200.0f/255.0f, 40.0f/255.0f, 1.0f});
    ImU32 green_col = ImGui::ColorConvertFloat4ToU32({40.0f/255.0f, 220.0f/255.0f, 55.0f/255.0f, 1.0f});
};



// Instruction Fetch stage:
void draw_if_stage(WindowConfig& window_config){
    float stage_width = window_config.IF_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    ImGui::BeginChild("Instruction Fetch", ImVec2(stage_width, window_config.stage_height), true, ImGuiWindowFlags_NoDecoration);
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 top_left_window = ImGui::GetWindowPos();

        // pc = pc+4 alu
        ImVec2 top_left_alu = {top_left_window.x + stage_width * 0.4f, top_left_window.y + window_config.stage_height * 0.15f};
        float alu_height = 0.15f * window_config.stage_height;
        window_config.pc_4_adder = AluStruct(top_left_alu, alu_height);
        draw_alu(draw_list, window_config.pc_4_adder, window_config.col, window_config.thickness);

        // pc
        ImVec2 top_left_pc = {top_left_window.x + stage_width * 0.25f, top_left_window.y + window_config.stage_height * 0.4f};
        float pc_height = 0.1f * window_config.stage_height;
        float pc_width = 0.5f * pc_height;
        ImVec2 bottom_right_pc = {top_left_pc.x+pc_width, top_left_pc.y+pc_height};
        window_config.pc = Rectangle{top_left_pc, bottom_right_pc};
        draw_list->AddRect(top_left_pc, bottom_right_pc, window_config.col, 0.0f, 0, window_config.thickness);

        // instruction memory
        ImVec2 top_left_insmem = {top_left_window.x + stage_width*0.45f, top_left_window.y + window_config.stage_height*0.4f};
        float insmem_height = 0.4f * window_config.stage_height;
        float insmem_width = 0.5f * insmem_height;
        ImVec2 bottom_right_insmem = {top_left_insmem.x + insmem_width, top_left_insmem.y + insmem_height};
        window_config.instruction_mem = Rectangle(top_left_insmem, bottom_right_insmem);
        draw_list->AddRect(top_left_insmem, bottom_right_insmem, window_config.col, 0.0f, 0, window_config.thickness);

        // mux for pc updating
        ImVec2 top_left_mux = {top_left_window.x + stage_width*0.10f, top_left_window.y + window_config.stage_height*0.40f};
        float mux_height = pc_height;
        float mux_width = mux_height/2;
        ImVec2 bottom_right_mux = {top_left_mux.x + mux_width, top_left_mux.y + mux_height};
        window_config.pc_adder_mux = Rectangle(top_left_mux, bottom_right_mux);
        draw_list->AddRect(top_left_mux, bottom_right_mux, window_config.col, mux_width/2, 0, window_config.thickness);
    }
    ImGui::EndChild();
}


// Instruction Decode Stage:
void draw_id_stage(WindowConfig& window_config){
    float stage_width = window_config.ID_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    ImGui::BeginChild("Instruction Decode", ImVec2(stage_width, window_config.stage_height), true, ImGuiWindowFlags_NoDecoration);
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 top_left_window = ImGui::GetWindowPos();

        // Decoder (separates alu flags, registers, mux controls etc from instruction)
        ImVec2 top_left_decoder = {top_left_window.x + stage_width*0.07f, top_left_window.y + window_config.stage_height*0.4f};
        float decoder_height = 0.3f * window_config.stage_height;
        float decoder_width = 0.4f * decoder_height;
        ImVec2 bottom_right_decoder = {top_left_decoder.x + decoder_width, top_left_decoder.y + decoder_height};
        window_config.decoder = Rectangle{top_left_decoder, bottom_right_decoder};
        draw_list->AddRect(top_left_decoder, bottom_right_decoder, window_config.col, 0.0f, 0, window_config.thickness);

        // Register file
        ImVec2 top_left_register_file = {top_left_window.x + stage_width*0.4f, top_left_window.y + window_config.stage_height*0.3f};
        float register_file_height = 0.4f * window_config.stage_height;
        float register_file_width = 0.6f * register_file_height;
        ImVec2 bottom_right_register_file = {top_left_register_file.x+register_file_width,top_left_register_file.y+register_file_height};
        window_config.register_file = Rectangle{top_left_register_file, bottom_right_register_file};
        draw_list->AddRect(top_left_register_file, bottom_right_register_file, window_config.col, 0.0f, 0, window_config.thickness);

        // Immediate Generator
        window_config.immediate_gen_center = {top_left_window.x + stage_width*0.65f, top_left_window.y + window_config.stage_height*0.85f};
        float imm_gen_height = window_config.stage_height*0.2f;
        ImVec2 mux_radius = {imm_gen_height/4, imm_gen_height/2};
        window_config.immediate_gen_hradius = imm_gen_height/4;
        draw_list->AddEllipse(window_config.immediate_gen_center, mux_radius, window_config.col, 0.0f, 0, window_config.thickness);
    }
    ImGui::EndChild();
}


// Execution Stage:
void draw_ex_stage(WindowConfig& window_config){
    float stage_width = window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    ImGui::BeginChild("Execution", ImVec2(stage_width, window_config.stage_height), true, ImGuiWindowFlags_NoDecoration);
    {
        window_config.exec_window_pos = ImGui::GetWindowPos();
        ImVec2 top_left_window = ImGui::GetWindowPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        // Main alu:
        ImVec2 top_left_main_alu = {top_left_window.x + stage_width*0.5f, top_left_window.y + window_config.stage_height*0.4f};
        float main_alu_height = window_config.stage_height*0.3f;
        window_config.main_alu = AluStruct(top_left_main_alu, main_alu_height);
        draw_alu(draw_list, window_config.main_alu, window_config.col, window_config.thickness);

        // Branch alu:
        ImVec2 top_left_branch_alu = {top_left_window.x + stage_width*0.5f, top_left_window.y + window_config.stage_height*0.1f};
        float branch_alu_height = window_config.stage_height * 0.2f;
        window_config.branch_alu = AluStruct(top_left_branch_alu, branch_alu_height);
        draw_alu(draw_list, window_config.branch_alu, window_config.col, window_config.thickness);

        // Alu mux for immediate / register file
        float mux_height = window_config.stage_height * 0.1f;
        float mux_width = mux_height/2;
        ImVec2 mux_top_left{top_left_window.x + stage_width * 0.25f, window_config.main_alu.get_input2_line_coords().y-mux_height*0.5f};
        ImVec2 mux_bottom_right{mux_top_left.x + mux_width, mux_top_left.y + mux_height};
        window_config.alu_mux = Rectangle{mux_top_left, mux_bottom_right};
        draw_list->AddRect(mux_top_left, mux_bottom_right, window_config.col, mux_width/2, 0, window_config.thickness);

        // Alu mux if data forwarding is enabled
        if(vm.ForwardingEnabled()){
            mux_top_left.y = window_config.main_alu.get_input1_line_coords().y-mux_height*0.5f;
            mux_bottom_right.y = mux_top_left.y + mux_height;
            window_config.alu_mux_forward = Rectangle{mux_top_left, mux_bottom_right};
            draw_list->AddRect(mux_top_left, mux_bottom_right, window_config.col, mux_width/2, 0, window_config.thickness);
        }

        // Shift left 1 for branch target
        window_config.branch_shift_center = {top_left_window.x + stage_width*0.3f, window_config.branch_alu.get_input2_line_coords().y};
        float shift_height = window_config.stage_height*0.07f;
        ImVec2 shift_radius = {shift_height/4, shift_height/2};
        window_config.branch_shift_radius = shift_height/4;
        draw_list->AddEllipse(window_config.branch_shift_center, shift_radius, window_config.col, 0.0f, 0, window_config.thickness);
    }
    ImGui::EndChild();
}


// Memory Stage:
void draw_mem_stage(WindowConfig& window_config){
    float stage_width = window_config.MEM_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    ImGui::BeginChild("Memory", ImVec2(stage_width, window_config.stage_height), true, ImGuiWindowFlags_NoDecoration);
    {
        window_config.mem_window_pos = ImGui::GetWindowPos();
        ImVec2 top_left_window = ImGui::GetWindowPos();
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
    
        // Main memory
        ImVec2 top_left_memory = {top_left_window.x + stage_width*0.2f, top_left_window.y + window_config.stage_height*0.3f};
        float memory_height = 0.6f*window_config.stage_height;
        float memory_width = 0.4f*memory_height;
        ImVec2 bottom_right_memory = {top_left_memory.x+memory_width,top_left_memory.y+memory_height};
        window_config.main_mem = Rectangle{top_left_memory, bottom_right_memory};
        draw_list->AddRect(top_left_memory, bottom_right_memory, window_config.col, 0.0f, 0, window_config.thickness);
    }

    ImGui::EndChild();
}


// Write Back Stage:
void draw_wb_stage(WindowConfig& window_config){
    float stage_width = window_config.WB_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    ImGui::BeginChild("Write Back", ImVec2(stage_width, window_config.stage_height), true, ImGuiWindowFlags_NoDecoration);
    ImVec2 top_left_window = ImGui::GetWindowPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Mux for selecting alu ouput / memory out
    float mux_height = window_config.stage_height * 0.15f;
    float mux_width = stage_width*0.4f;
    ImVec2 mux_top_left = {top_left_window.x + (stage_width - mux_width)*0.5f, top_left_window.y + window_config.stage_height*0.5f - mux_height/2};
    ImVec2 mux_bottom_right = {mux_top_left.x + mux_width, mux_top_left.y + mux_height};
    window_config.wb_mux = Rectangle(mux_top_left, mux_bottom_right);
    draw_list->AddRect(mux_top_left, mux_bottom_right, window_config.col, mux_width/2.0f, 0, window_config.thickness);

    ImGui::EndChild();
}


// Instruction fetch buses:
void draw_if_buses(WindowConfig& window_config){
    float if_window_width = window_config.IF_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    float mux_height = window_config.pc_adder_mux.get_height();
    // Branch alu to pc mux line:
    {
        float exec_window_width = window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
        float line_offset = window_config.pc_adder_mux.top_left.x - if_window_width*0.06f;
        
        ImVec2 branch_alu_output_line_pos = window_config.branch_alu.get_output_line_coords();
        ImVec2 p1{branch_alu_output_line_pos.x + exec_window_width*0.1f, branch_alu_output_line_pos.y};
        ImVec2 p2{p1.x, p1.y - window_config.stage_height*0.13f};
        ImVec2 p3{line_offset, p2.y};
        ImVec2 p4{line_offset, window_config.pc_adder_mux.top_left.y+mux_height*0.75f};
        ImVec2 p5{window_config.pc_adder_mux.top_left.x, p4.y};
        std::vector<ImVec2> points = {branch_alu_output_line_pos, p1, p2, p3, p4, p5};

        draw_line_connecting_points(points, window_config.col, window_config.thickness);
    }

    // pc 4 adder to pc mux line:
    {
        float line_offset = window_config.pc_adder_mux.top_left.x - if_window_width*0.03f;

        ImVec2 pc_4_adder_output_line_pos = window_config.pc_4_adder.get_output_line_coords();
        ImVec2 p1{pc_4_adder_output_line_pos.x + if_window_width*0.1f, pc_4_adder_output_line_pos.y};
        ImVec2 p2{p1.x, p1.y - window_config.stage_height*0.13f};
        ImVec2 p3{line_offset, p2.y};
        ImVec2 p4{line_offset, window_config.pc_adder_mux.top_left.y+mux_height*0.25f};
        ImVec2 p5{window_config.pc_adder_mux.top_left.x, p4.y};
        std::vector<ImVec2> points = {pc_4_adder_output_line_pos, p1, p2, p3, p4, p5};

        draw_line_connecting_points(points, window_config.col, window_config.thickness);
    }

    // pc to adders and instruction memory lines:
    {
        // pc to adders lines:

        ImVec2 input2_pc_4_adder = window_config.pc_4_adder.get_input2_line_coords();
        ImVec2 input1_branch_alu = window_config.branch_alu.get_input1_line_coords();
        ImVec2 p1{input2_pc_4_adder.x - 0.05f*if_window_width, input2_pc_4_adder.y};
        ImVec2 p2{p1.x, window_config.pc.top_left.y - window_config.stage_height*0.05f};
        ImVec2 p3{(window_config.pc.bottom_right.x + window_config.instruction_mem.top_left.x) / 2.0f, p2.y};
        ImVec2 p4{p3.x + 0.4f*if_window_width, p3.y};
        ImVec2 p5{p4.x, input1_branch_alu.y};
        std::vector<ImVec2> points = {input2_pc_4_adder, p1, p2, p3, p4, p5, input1_branch_alu};

        draw_line_connecting_points(points, window_config.col, window_config.thickness);

        ImVec2 p6{p3.x,(window_config.pc.bottom_right.y + window_config.pc.top_left.y) / 2.0f};
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddLine(p3,p6,window_config.col,window_config.thickness);


        // mux to pc line:
        ImVec2 pc_adder_mux_output{window_config.pc_adder_mux.bottom_right.x, (window_config.pc_adder_mux.top_left.y + window_config.pc_adder_mux.bottom_right.y) / 2.0f};
        ImVec2 pc_input{window_config.pc.top_left.x,(window_config.pc.top_left.y + window_config.pc.bottom_right.y) / 2.0f};
        draw_list->AddLine(pc_adder_mux_output, pc_input, window_config.col, window_config.thickness);

        // pc to instruction memory line:
        ImVec2 pc_output_line_coords{window_config.pc.bottom_right.x, p6.y};
        ImVec2 instruction_mem_input_coords{window_config.instruction_mem.top_left.x, p6.y};
        draw_list->AddLine(pc_output_line_coords, instruction_mem_input_coords, window_config.col, window_config.thickness);

        // instructon memory to decoder line:
        ImVec2 instruction_mem_output_coords{window_config.instruction_mem.bottom_right.x, instruction_mem_input_coords.y};
        ImVec2 decoder_input_coords{window_config.decoder.top_left.x, instruction_mem_output_coords.y};
        draw_list->AddLine(instruction_mem_output_coords, decoder_input_coords, window_config.col, window_config.thickness);
    }
}


// Instruction Decode Buses:
void draw_id_buses(WindowConfig& window_config){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    float ex_stage_width = window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    // Decoder to register file / immediate generator lines:
    {
        Rectangle& register_file = window_config.register_file;
        ImVec2& immediate_gen_center = window_config.immediate_gen_center;
        float immediate_gen_hradius = window_config.immediate_gen_hradius;

        ImVec2 decoder_output{window_config.decoder.bottom_right.x, (window_config.decoder.top_left.y + window_config.decoder.bottom_right.y) * 0.5f};
        ImVec2 decoder_offset{decoder_output.x + ex_stage_width*0.05f, decoder_output.y};
        
        float register_file_height = window_config.register_file.get_height();
        ImVec2 register_file_input1{register_file.top_left.x, register_file.top_left.y + register_file_height/4.0f};
        ImVec2 register_file_input2{register_file.top_left.x, register_file.top_left.y + register_file_height/2.0f};

        ImVec2 imm_gen_input{immediate_gen_center.x - immediate_gen_hradius, immediate_gen_center.y};

        ImVec2 rf_s1{decoder_offset.x, register_file_input1.y};
        ImVec2 rf_s2{decoder_offset.x, register_file_input2.y};

        ImVec2 imm_s{decoder_offset.x, imm_gen_input.y};

        draw_list->AddLine(decoder_output, decoder_offset, window_config.col, window_config.thickness);
        draw_list->AddLine(decoder_offset, rf_s1, window_config.col, window_config.thickness);
        draw_list->AddLine(decoder_offset, rf_s2, window_config.col, window_config.thickness);
        draw_list->AddLine(decoder_offset, imm_s, window_config.col, window_config.thickness);

        draw_list->AddLine(rf_s1, register_file_input1, window_config.col, window_config.thickness);
        draw_list->AddLine(rf_s2, register_file_input2, window_config.col, window_config.thickness);

        draw_list->AddLine(imm_s, imm_gen_input, window_config.col, window_config.thickness);
    }

    // register file to alu and memory
    {
        Rectangle& alu_mux = window_config.alu_mux;
        AluStruct& main_alu = window_config.main_alu;
        Rectangle& register_file = window_config.register_file;
        Rectangle& main_mem = window_config.main_mem;


        // register file to alu
        float alu_mux_height = alu_mux.get_height();
        ImVec2 main_alu_input1 = main_alu.get_input1_line_coords();
        ImVec2 alu_mux_input1{alu_mux.top_left.x, alu_mux.top_left.y + alu_mux_height*0.25f};
        ImVec2 register_file_output_reg1{register_file.bottom_right.x, main_alu.get_input1_line_coords().y};
        ImVec2 register_file_output_reg2{register_file.bottom_right.x, alu_mux_input1.y};

        draw_list->AddLine(register_file_output_reg2, alu_mux_input1, window_config.col, window_config.thickness);
        if(vm.ForwardingEnabled()){
            ImVec2 alu_mux_forward_in{window_config.alu_mux_forward.top_left.x, (window_config.alu_mux_forward.bottom_right.y + window_config.alu_mux_forward.top_left.y)/2.0f};
            ImVec2 alu_mux_forward_out{window_config.alu_mux_forward.bottom_right.x, alu_mux_forward_in.y};
            draw_list->AddLine(register_file_output_reg1, alu_mux_forward_in, window_config.col, window_config.thickness);
            draw_list->AddLine(alu_mux_forward_out, main_alu_input1, window_config.col, window_config.thickness);
        }
        else{
            draw_list->AddLine(register_file_output_reg1, main_alu_input1, window_config.col, window_config.thickness);
        }


        // register file to memory
        ImVec2 write_port_main_mem{main_mem.top_left.x, main_mem.top_left.y + main_mem.get_height()*0.85f};

        float ex_stage_x_offset = window_config.exec_window_pos.x;
        float ex_stage_width = window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
        ImVec2 d_offset{ex_stage_x_offset + ex_stage_width*0.15f, register_file_output_reg2.y};
        ImVec2 wpmm_s{d_offset.x, write_port_main_mem.y};

        draw_list->AddLine(d_offset, wpmm_s, window_config.col, window_config.thickness);
        draw_list->AddLine(wpmm_s, write_port_main_mem, window_config.col, window_config.thickness);
    }

    // immediate generator to alu_mux and branch shift
    {
        ImVec2& immediate_gen_center = window_config.immediate_gen_center;
        float& immediate_gen_hradius = window_config.immediate_gen_hradius;
        ImVec2& branch_shift_center = window_config.branch_shift_center;
        float& branch_shift_radius = window_config.branch_shift_radius;
        Rectangle& alu_mux = window_config.alu_mux;
        

        float ex_stage_x_offset = window_config.exec_window_pos.x;
        float ex_stage_width = window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;

        ImVec2 imm_gen_output{immediate_gen_center.x + immediate_gen_hradius, immediate_gen_center.y};
        ImVec2 imm_gen_offset{ex_stage_x_offset + ex_stage_width*0.2f, imm_gen_output.y};

        float alu_mux_height = alu_mux.get_height();
        ImVec2 alu_mux_input2{alu_mux.top_left.x, alu_mux.top_left.y + alu_mux_height*0.75f};

        ImVec2 branch_shift_input{branch_shift_center.x - branch_shift_radius, branch_shift_center.y};

        ImVec2 bs_s{imm_gen_offset.x, branch_shift_input.y};
        ImVec2 am_s{imm_gen_offset.x, alu_mux_input2.y};

        draw_list->AddLine(imm_gen_output, imm_gen_offset, window_config.col, window_config.thickness);
        draw_list->AddLine(imm_gen_offset, bs_s, window_config.col, window_config.thickness);
        draw_list->AddLine(bs_s, branch_shift_input, window_config.col, window_config.thickness);
        draw_list->AddLine(imm_gen_offset, am_s, window_config.col, window_config.thickness);
        draw_list->AddLine(am_s, alu_mux_input2, window_config.col, window_config.thickness);
    }
}

// Execution Stage Lines:
void draw_ex_buses(WindowConfig& window_config){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // alu_mux to main_alu:
    {
        float alu_mux_height = window_config.alu_mux.get_height();
        ImVec2 mux_output{window_config.alu_mux.bottom_right.x, window_config.alu_mux.top_left.y + alu_mux_height*0.5f};
        ImVec2 alu_input2 = window_config.main_alu.get_input2_line_coords();

        draw_list->AddLine(alu_input2,mux_output,window_config.col,window_config.thickness);
    }

    // branch_shift to branch_alu line:
    {
        ImVec2 branch_shift_output{window_config.branch_shift_center.x+window_config.branch_shift_radius, window_config.branch_shift_center.y};
        ImVec2 branch_alu_input2 = window_config.branch_alu.get_input2_line_coords();

        draw_list->AddLine(branch_shift_output, branch_alu_input2, window_config.col, window_config.thickness);
    }

    // alu to main memory and wb_mux
    {
        AluStruct& main_alu = window_config.main_alu;
        ImVec2& mem_window_pos = window_config.mem_window_pos;
        Rectangle& wb_mux = window_config.wb_mux;

        float mem_window_size = window_config.MEM_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
        float wb_window_size = window_config.WB_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;

        ImVec2 main_alu_out = main_alu.get_output_line_coords();
        ImVec2 alu_out_offset{mem_window_pos.x + mem_window_size * 0.1f, main_alu_out.y};
        ImVec2 alu_u_off{alu_out_offset.x, mem_window_pos.y + window_config.stage_height*0.2f};
        ImVec2 mux_u_off{wb_mux.top_left.x - wb_window_size*0.1f, alu_u_off.y};
        ImVec2 mux_l_off{mux_u_off.x, wb_mux.top_left.y + wb_mux.get_height()/3.0f};
        ImVec2 mux_input1{wb_mux.top_left.x, mux_l_off.y};

        ImVec2 memory_input{window_config.main_mem.top_left.x, main_alu_out.y};

        draw_list->AddLine(main_alu_out, memory_input, window_config.col, window_config.thickness);
        std::vector<ImVec2> points{alu_out_offset, alu_u_off, mux_u_off, mux_l_off, mux_input1};
        draw_line_connecting_points(points, window_config.col, window_config.thickness);
    }
}


// Memory Buses
void draw_mem_buses(WindowConfig& window_config){
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    // memory to wb_mux
    {
        ImVec2 wb_mux_input2{window_config.wb_mux.top_left.x, window_config.wb_mux.top_left.y + window_config.wb_mux.get_height()*2/3.0f};
        ImVec2 memory_out{window_config.main_mem.bottom_right.x, wb_mux_input2.y};

        draw_list->AddLine(wb_mux_input2, memory_out, window_config.col, window_config.thickness);
    }
}


// Write Back Buses
void draw_wb_buses(WindowConfig& window_config){
    Rectangle& wb_mux = window_config.wb_mux;
    Rectangle& register_file = window_config.register_file;

    float wb_window_size = window_config.WB_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    ImVec2 wb_mux_out{wb_mux.bottom_right.x, wb_mux.top_left.y + wb_mux.get_height()*0.5f};
    ImVec2 wb_mux_r_off{wb_mux_out.x + wb_window_size*0.1f, wb_mux_out.y};
    ImVec2 wb_mux_d_off{wb_mux_r_off.x, wb_mux_r_off.y + window_config.stage_height*0.48f};

    float id_window_size = window_config.ID_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    ImVec2 write_register_file{register_file.top_left.x, register_file.top_left.y + register_file.get_height()*0.75f};
    ImVec2 wrf_l_off{write_register_file.x - id_window_size*0.03f, write_register_file.y};
    ImVec2 wrf_d_off{wrf_l_off.x, wb_mux_d_off.y};

    std::vector<ImVec2> points{wb_mux_out, wb_mux_r_off, wb_mux_d_off, wrf_d_off, wrf_l_off, write_register_file};
    draw_line_connecting_points(points, window_config.col, window_config.thickness);
}


void draw_pipeline_registers(WindowConfig& window_config){
    ImDrawList* drawlist = ImGui::GetWindowDrawList();

    float stage_fracs[5] = {window_config.IF_STAGE_WIDTH_FRAC, window_config.ID_STAGE_WIDTH_FRAC, window_config.EX_STAGE_WIDTH_FRAC, window_config.MEM_STAGE_WIDTH_FRAC};
    float stage_start = 0;
    for(int i=0;i<4;i++){
        stage_start += stage_fracs[i] * window_config.WINDOW_SIZE.x;
        ImVec2 top_left{window_config.WINDOW_POS.x + stage_start - window_config.pipeline_registers_width/2.0f, window_config.WINDOW_POS.y};
        ImVec2 bottom_right{window_config.WINDOW_POS.x + stage_start + window_config.pipeline_registers_width/2.0f, window_config.WINDOW_POS.y + window_config.stage_height};
        drawlist->AddRectFilled(top_left, bottom_right, window_config.col, 0.0f, 0);
    }
}



void draw_data_forwarding_unit(WindowConfig& window_config){
    float if_stage_start = window_config.WINDOW_POS.x;
    float id_stage_start = if_stage_start + window_config.IF_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    float ex_stage_start = id_stage_start + window_config.ID_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    float mem_stage_start = ex_stage_start + window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    float wb_stage_start = mem_stage_start + window_config.MEM_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;

    ImVec2 top_left{mem_stage_start - window_config.data_forwarding_unit_width/2.0f, window_config.WINDOW_POS.y + window_config.stage_height + window_config.hardware_buffer};
    ImVec2 bottom_right{mem_stage_start + window_config.data_forwarding_unit_width/2.0f, window_config.WINDOW_POS.y + window_config.stage_height + window_config.hardware_buffer + window_config.data_forwarding_unit_height};
    window_config.data_forwarding_unit = Rectangle{top_left, bottom_right};

    ImDrawList* drawlist = ImGui::GetWindowDrawList();

    drawlist->AddRect(top_left, bottom_right, window_config.col, 0.0f, 0, window_config.thickness);

    // buses
    {
        float ex_stage_width = window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
        
        static int frame_count = 0;
        static bool lit_up_rs1 = false;
        static bool lit_up_rs2 = false;
        static bool lit_up_ex_mem = false;
        static bool lit_up_mem_wb = false;

        if(SimState_.DATA_FORWARD){
            frame_count = 120;
            SimState_.DATA_FORWARD = false;

            lit_up_rs1 = false;
            lit_up_rs2 = false;
            lit_up_ex_mem = false;
            lit_up_mem_wb = false;

            if(SimState_.DF_PATH == SimState::DataForwardPaths::EXEC_MEM_RS1){
                lit_up_rs1 = true;
                lit_up_ex_mem = true;
            }
            else if(SimState_.DF_PATH == SimState::DataForwardPaths::EXEC_MEM_RS2){
                lit_up_rs2 = true;
                lit_up_ex_mem = true;
            }
            else if(SimState_.DF_PATH == SimState::DataForwardPaths::MEM_WB_RS1){
                lit_up_rs1 = true;
                lit_up_mem_wb = true;
            }
            else if(SimState_.DF_PATH == SimState::DataForwardPaths::MEM_WB_RS2){
                lit_up_rs2 = true;
                lit_up_mem_wb = true;
            }

            if(SimState_.DF_ALL){
                lit_up_rs1 = true;
                lit_up_rs2 = true;
                lit_up_ex_mem = true;
                lit_up_mem_wb = true;
            }
        }
        
        // alu_mux_forward to data forwarding unit
        {
            if(frame_count>0 && lit_up_rs1){
                window_config.col = window_config.green_col;
                frame_count--;
            }

            ImVec2 mux_connector{(window_config.alu_mux_forward.top_left.x + window_config.alu_mux_forward.bottom_right.x) / 2.0f, window_config.alu_mux_forward.bottom_right.y};
            ImVec2 p1{mux_connector.x, mux_connector.y + window_config.stage_height * 0.02f};
            ImVec2 p2{p1.x + ex_stage_width*0.10f, p1.y};
            ImVec2 p3{p2.x, window_config.data_forwarding_unit.top_left.y + window_config.data_forwarding_unit_height/3.0f};
            ImVec2 p4{window_config.data_forwarding_unit.top_left.x, p3.y};

            std::vector<ImVec2> points{mux_connector, p1, p2, p3, p4};
            draw_line_connecting_points(points, window_config.col, window_config.thickness);

            window_config.col = window_config.white_col;
        }

        // alu_mux to data forwarding unit
        {
            if(frame_count>0 && lit_up_rs2){
                window_config.col = window_config.green_col;
                frame_count--;
            }

            ImVec2 mux_connector{(window_config.alu_mux.top_left.x + window_config.alu_mux.bottom_right.x) / 2.0f, window_config.alu_mux.bottom_right.y};
            ImVec2 p1{mux_connector.x, mux_connector.y + window_config.stage_height * 0.02f};
            ImVec2 p2{p1.x, window_config.data_forwarding_unit.top_left.y + window_config.data_forwarding_unit_height/3.0f * 2.0f};
            ImVec2 p3{window_config.data_forwarding_unit.top_left.x, p2.y};

            std::vector<ImVec2> points{mux_connector, p1, p2, p3};
            draw_line_connecting_points(points, window_config.col, window_config.thickness);

            window_config.col = window_config.white_col;
        }

        // exec-mem register to data forwarding unit
        {
            if(frame_count>0 && lit_up_ex_mem){
                window_config.col = window_config.green_col;
            }

            ImVec2 p1{mem_stage_start, window_config.WINDOW_POS.y + window_config.stage_height};
            ImVec2 p2{p1.x, window_config.data_forwarding_unit.top_left.y};

            drawlist->AddLine(p1, p2, window_config.col, window_config.thickness);

            window_config.col = window_config.white_col;
        }

        // mem-wb register to data forwarding unit
        {
            if(frame_count>0 && lit_up_mem_wb){
                window_config.col = window_config.green_col;
            }

            ImVec2 p1{wb_stage_start, window_config.WINDOW_POS.y + window_config.stage_height};
            ImVec2 p2{p1.x, (window_config.data_forwarding_unit.top_left.y + window_config.data_forwarding_unit.bottom_right.y)/2.0f};
            ImVec2 p3{window_config.data_forwarding_unit.bottom_right.x, p2.y};

            drawlist->AddLine(p1, p2, window_config.col, window_config.thickness);
            drawlist->AddLine(p2, p3, window_config.col, window_config.thickness);

            window_config.col = window_config.white_col;
        }
    }
}


void draw_hazard_detector(WindowConfig& window_config){
    float if_stage_start = window_config.WINDOW_POS.x;
    float id_stage_start = if_stage_start + window_config.IF_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    float ex_stage_start = id_stage_start + window_config.ID_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    float mem_stage_start = ex_stage_start + window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    float wb_stage_start = mem_stage_start + window_config.MEM_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;

    ImVec2 top_left{mem_stage_start - window_config.data_forwarding_unit_width/2.0f, window_config.WINDOW_POS.y + window_config.WINDOW_SIZE.y - window_config.hardware_buffer - window_config.hazard_detector_height - window_config.instr_text_height};
    ImVec2 bottom_right{mem_stage_start + window_config.data_forwarding_unit_width/2.0f, window_config.WINDOW_POS.y + window_config.WINDOW_SIZE.y - window_config.hardware_buffer - window_config.instr_text_height};
    window_config.hazard_detector = Rectangle{top_left, bottom_right};

    ImDrawList* drawlist = ImGui::GetWindowDrawList();

    drawlist->AddRect(top_left, bottom_right, window_config.col, 0.0f, 0, window_config.thickness);

    static int frame_count = 0;

    // buses
    {
        // bus from {ex-mem register, detector}
        {
            if(SimState_.HAZARD_DETECTED && SimState_.HZ_PATH == SimState::HazardPaths::EXEC_MEM){
                frame_count = 120;
                SimState_.HAZARD_DETECTED = false;
            }

            if(frame_count>0){
                frame_count--;
                window_config.col = window_config.red_col;
            }
            
            // bypass data forwarding unit
            if(vm.ForwardingEnabled()){
                ImVec2 p1{mem_stage_start, window_config.WINDOW_POS.y + window_config.stage_height};
                ImVec2 p2{p1.x, (window_config.data_forwarding_unit.top_left.y + p1.y) / 2.0f};
                ImVec2 p3{p2.x + window_config.data_forwarding_unit_width * 0.6f, p2.y};
                ImVec2 p4{p3.x, window_config.hazard_detector.top_left.y + window_config.hazard_detector.get_height()/2.0f};
                ImVec2 p5{window_config.hazard_detector.bottom_right.x, p4.y};

                std::vector<ImVec2> points{p1, p2, p3, p4, p5};
                draw_line_connecting_points(points, window_config.col, window_config.thickness);
            }
            else{
                ImVec2 p1{mem_stage_start, window_config.WINDOW_POS.y + window_config.stage_height};
                ImVec2 p2{p1.x, window_config.hazard_detector.top_left.y};

                drawlist->AddLine(p1, p2, window_config.col, window_config.thickness);
            }

            window_config.col = window_config.white_col;
        }

        // bus from {mem-wb register, detector}
        {
            if(SimState_.HAZARD_DETECTED && SimState_.HZ_PATH == SimState::HazardPaths::MEM_WB){
                frame_count = 120;
                SimState_.HAZARD_DETECTED = false;
            }

            if(frame_count>0){
                frame_count--;
                window_config.col = window_config.red_col;
            }

            if(!vm.ForwardingEnabled()){
                ImVec2 p1{wb_stage_start, window_config.WINDOW_POS.y + window_config.stage_height};
                ImVec2 p2{p1.x, (window_config.hazard_detector.top_left.y + window_config.hazard_detector.bottom_right.y) / 2.0f};
                ImVec2 p3{window_config.hazard_detector.bottom_right.x, p2.y};
                
                std::vector<ImVec2> points{p1,p2,p3};
                draw_line_connecting_points(points, window_config.col, window_config.thickness);
            }

            window_config.col = window_config.white_col;
        }

        // bus from {id-ex register, detector}
        {
            if(frame_count>0){
                window_config.col = window_config.red_col;
            }

            ImVec2 p1{ex_stage_start, window_config.WINDOW_POS.y + window_config.stage_height};
            ImVec2 p2{p1.x, window_config.hazard_detector.top_left.y + window_config.hazard_detector.get_height()/2.0f};
            ImVec2 p3{window_config.hazard_detector.top_left.x, p2.y};

            drawlist->AddLine(p1, p2, window_config.col, window_config.thickness);
            drawlist->AddLine(p2, p3, window_config.col, window_config.thickness);
            
            window_config.col = window_config.white_col;
        }
    }
}


void draw_instructions(WindowConfig& window_config){
    std::vector<std::unique_ptr<const rv5s::InstrContext>> instructions = vm.GetInstructions();
    
    if(vm.PipeliningEnabled()){
        float if_stage_start = window_config.WINDOW_POS.x;
        float id_stage_start = if_stage_start + window_config.IF_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
        float ex_stage_start = id_stage_start + window_config.ID_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
        float mem_stage_start = ex_stage_start + window_config.EX_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
        float wb_stage_start = mem_stage_start + window_config.MEM_STAGE_WIDTH_FRAC * window_config.WINDOW_SIZE.x;
    
        float offset_arr[6] = {if_stage_start, id_stage_start, ex_stage_start, mem_stage_start, wb_stage_start, window_config.WINDOW_SIZE.x + window_config.WINDOW_POS.x};
        float offset = id_stage_start;

        for(size_t i=0;i<instructions.size();i++){
            std::string instr_dissassembled;
            const rv5s::PipelinedInstrContext* instruction = dynamic_cast<const rv5s::PipelinedInstrContext*>(instructions[i].get());
            if(!instruction){
                std::cerr << "Tried to downcast InstrContext to PipelinedInstrContext while setting debug lines" << std::endl;
                continue;
            }
            uint64_t instr_pc = instruction->pc;
            if(vm.program_.intermediate_code.size()>instr_pc/4){
                ICUnit& t = vm.program_.intermediate_code[instr_pc/4].first;
                instr_dissassembled += t.to_string();
            }
            else if(!instruction->bubbled)
                continue;

            if(instruction->bubbled){
                instr_dissassembled = "BUBBLE";
            }

            if(instruction->nopped || instruction->bubbled){
                window_config.col = window_config.red_col;
            }
    
            float avail_height = window_config.instr_text_height;
            float font_size = ImGui::GetFontSize();
            float text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, instr_dissassembled.c_str(), nullptr, nullptr).x;
    
            offset = (offset_arr[i+1]+offset_arr[i])/2.0f;
            ImVec2 text_start{offset - text_size/2.0f, window_config.WINDOW_POS.y + window_config.WINDOW_SIZE.y - (avail_height - font_size)/2.0f};
    
            ImDrawList* drawlist = ImGui::GetWindowDrawList();
            drawlist->AddText(text_start, window_config.col, instr_dissassembled.c_str());

            if(instruction->nopped || instruction->bubbled){
                window_config.col = ImGui::ColorConvertFloat4ToU32({1.0f, 1.0f, 1.0f, 1.0f});
            }
        }
    }
    else{
        std::string instr_dissassembled;
        const rv5s::InstrContext& instruction = *instructions[0].get();
        uint64_t instr_pc = instruction.pc;
        if(vm.program_.intermediate_code.size()>instr_pc/4){
            ICUnit& t = vm.program_.intermediate_code[instr_pc/4].first;
            instr_dissassembled += t.to_string();
        }
        else
            return;

        instr_dissassembled += " {done}";

        float avail_height = window_config.instr_text_height;
        float font_size = ImGui::GetFontSize();
        float text_size = ImGui::GetFont()->CalcTextSizeA(ImGui::GetFontSize(), FLT_MAX, -1.0f, instr_dissassembled.c_str(), nullptr, nullptr).x;

        float offset = window_config.WINDOW_POS.x + window_config.WINDOW_SIZE.x / 2.0f;
        ImVec2 text_start{offset - text_size/2.0f, window_config.WINDOW_POS.y + window_config.WINDOW_SIZE.y - (avail_height - font_size)/2.0f};

        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        drawlist->AddText(text_start, window_config.col, instr_dissassembled.c_str());
    }
}


// Processor Main
void processor_main() {
    static float PROCESSOR_HEIGHT = 500.0f;
    ImVec2 PROCESSOR_SIZE{1000.0f, PROCESSOR_HEIGHT};
    
    ImGui::BeginChild("Processor window", PROCESSOR_SIZE, false, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_HorizontalScrollbar);
    {
        WindowConfig window_config;
        window_config.processor_height = PROCESSOR_HEIGHT;
        window_config.instr_text_height = 50.0f;
        window_config.stage_height -= window_config.instr_text_height;

        // remove spacing between stages
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));      
        

        // Pipelining:
        if(vm.PipeliningEnabled()){
            if(vm.HazardEnabled() || vm.ForwardingEnabled()){
                if(vm.HazardEnabled() && vm.ForwardingEnabled()){
                    window_config.stage_height -= 3.0f * window_config.hardware_buffer;
                }
                else{
                    window_config.stage_height -= 2.0f * window_config.hardware_buffer;
                }

                if(vm.HazardEnabled()){
                    window_config.stage_height -= window_config.hazard_detector_height;
                }
                if(vm.ForwardingEnabled()){
                    window_config.stage_height -= window_config.data_forwarding_unit_height;
                }
            }
        }

        // stages
        draw_if_stage(window_config);
        ImGui::SameLine();
        draw_id_stage(window_config);
        ImGui::SameLine();
        draw_ex_stage(window_config);
        ImGui::SameLine();
        draw_mem_stage(window_config);
        ImGui::SameLine();
        draw_wb_stage(window_config);


        // change color to yellow for 60 frames
        static int frame_count = 0;
        if(SimState_.LIT_UP){
            frame_count = 30;
            SimState_.LIT_UP = false;
        }

        if(frame_count>0){
            window_config.col = ImGui::ColorConvertFloat4ToU32({200.0f/255.0f, 200.0f/255.0f, 40.0f/255.0f, 1.0f});
            frame_count--;
        }

        // Buses
        draw_if_buses(window_config);
        draw_id_buses(window_config);
        draw_ex_buses(window_config);
        draw_mem_buses(window_config);
        draw_wb_buses(window_config);

        window_config.col = window_config.white_col;

        // Pipelining:
        if(vm.PipeliningEnabled()){
            if(vm.ForwardingEnabled()){
                draw_data_forwarding_unit(window_config);
            }
            if(vm.HazardEnabled()){
                draw_hazard_detector(window_config);
            }
            draw_pipeline_registers(window_config);
        }

        ImGui::PushFont(STANDARD_BOLD_MEDIUM_FONT);
        draw_instructions(window_config);
        ImGui::PopFont();

        ImGui::PopStyleVar(); // restore spacing
    }
    ImGui::EndChild();
}
