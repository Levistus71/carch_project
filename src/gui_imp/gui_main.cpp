// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "imgui.h"
#include "../imgui/backends/imgui_impl_glfw.h"
#include "../imgui/backends/imgui_impl_opengl3.h"
#include <stdio.h>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers
#include <algorithm>
#include <vector>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}


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


// Main code
int main(int, char**)
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
    // GL ES 2.0 + GLSL 100 (WebGL 1.0)
    const char* glsl_version = "#version 100";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
    // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
    const char* glsl_version = "#version 300 es";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif

    // Create window with graphics context
    float main_scale = ImGui_ImplGlfw_GetContentScaleForMonitor(glfwGetPrimaryMonitor()); // Valid on GLFW 3.3+ only
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "Dear ImGui GLFW+OpenGL3 example", nullptr, nullptr);
    if (window == nullptr)
        return 1;
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsLight();

    // Setup scaling
    ImGuiStyle& style = ImGui::GetStyle();
    style.ScaleAllSizes(main_scale);        // Bake a fixed style scale. (until we have a solution for dynamic style scaling, changing this requires resetting Style + calling this again)
    style.FontScaleDpi = main_scale;        // Set initial font scale. (using io.ConfigDpiScaleFonts=true makes this unnecessary. We leave both here for documentation purpose)

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
    ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return a nullptr. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype for higher quality font rendering.
    // - Read 'docs/FONTS.md' for more instructions and details. If you like the default font but want it to scale better, consider using the 'ProggyVector' from the same author!
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    // - Our Emscripten build process allows embedding fonts to be accessible at runtime from the "fonts/" folder. See Makefile.emscripten for details.
    //style.FontSizeBase = 20.0f;
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf");
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf");
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf");
    //IM_ASSERT(font != nullptr);


    // Control variables for window
    bool show_demo_window = false;



    // Main loop
#ifdef __EMSCRIPTEN__
    // For an Emscripten build we are disabling file-system access, so let's not attempt to do a fopen() of the imgui.ini file.
    // You may manually call LoadIniSettingsFromMemory() to load settings from your own storage.
    io.IniFilename = nullptr;
    EMSCRIPTEN_MAINLOOP_BEGIN
#else
    while (!glfwWindowShouldClose(window))
#endif
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();
        if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0)
        {
            ImGui_ImplGlfw_Sleep(10);
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        
        // Our window (ALU rendering):
        ImGui::SetNextWindowSize(ImVec2(1100, 600));
        // ImGui::PushStyleVar(ImGuiStyleVar_WindowMinSize, ImVec2(800, 400));
        {
            ImGui::Begin("Alu window", nullptr, ImGuiWindowFlags_MenuBar);
            
            // Temporary Menu Bar for the window
            if(ImGui::BeginMenuBar()){
                if(ImGui::BeginMenu("Placeholder")){
                    ImGui::MenuItem("Show demo window", NULL, &show_demo_window);
                    ImGui::EndMenu();
                }
                ImGui::EndMenuBar();
            }
            
            // remove spacing between stages
            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));      
            
            // vars:
            ImVec2 window_size = ImGui::GetContentRegionAvail();
            float stage_height = window_size.y;
            float thickness = 2.0f;
            ImU32 white_col = ImGui::GetColorU32(ImVec4(255,255,255,255));

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

            constexpr float IF_STAGE_WIDTH_FRAC = 0.25f;
            constexpr float ID_STAGE_WIDTH_FRAC = 0.27f;
            constexpr float EX_STAGE_WIDTH_FRAC = 0.17f;
            constexpr float MEM_STAGE_WIDTH_FRAC = 0.2f;
            constexpr float WB_STAGE_WIDTH_FRAC = 0.1f;

            
            // Instruction Fetch Stage:
            {
                float stage_width = IF_STAGE_WIDTH_FRAC * window_size.x;
                ImGui::BeginChild("Instruction Fetch", ImVec2(stage_width, stage_height), true, ImGuiWindowFlags_NoDecoration);

                ImGui::Text("Instruction Fetch");
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 top_left_window = ImGui::GetWindowPos();


                // pc = pc+4 alu
                ImVec2 top_left_alu = {top_left_window.x + stage_width*0.4f, top_left_window.y + stage_height*0.15f};
                float alu_height = 0.15*stage_height;
                pc_4_adder = AluStruct(top_left_alu, alu_height);
                draw_alu(draw_list, pc_4_adder, white_col, thickness);

                // pc
                ImVec2 top_left_pc = {top_left_window.x + stage_width*0.25f, top_left_window.y + stage_height*0.4f};
                float pc_height = 0.1f*stage_height;
                float pc_width = 0.5*pc_height;
                ImVec2 bottom_right_pc = {top_left_pc.x+pc_width, top_left_pc.y+pc_height};
                pc = Rectangle{top_left_pc, bottom_right_pc};
                draw_list->AddRect(top_left_pc, bottom_right_pc, white_col, 0.0f, 0, thickness);

                // instruction memory
                ImVec2 top_left_insmem = {top_left_window.x + stage_width*0.45f, top_left_window.y + stage_height*0.4f};
                float insmem_height = 0.4*stage_height;
                float insmem_width = 0.5*insmem_height;
                ImVec2 bottom_right_insmem = {top_left_insmem.x + insmem_width, top_left_insmem.y + insmem_height};
                instruction_mem = Rectangle(top_left_insmem, bottom_right_insmem);
                draw_list->AddRect(top_left_insmem, bottom_right_insmem, white_col, 0.0f, 0, thickness);

                // mux for pc updating
                ImVec2 top_left_mux = {top_left_window.x + stage_width*0.10f, top_left_window.y + stage_height*0.40f};
                float mux_height = pc_height;
                float mux_width = mux_height/2;
                ImVec2 bottom_right_mux = {top_left_mux.x + mux_width, top_left_mux.y + mux_height};
                pc_adder_mux = Rectangle(top_left_mux, bottom_right_mux);
                draw_list->AddRect(top_left_mux, bottom_right_mux, white_col, mux_width/2, 0, thickness);

                ImGui::EndChild();
            }

            ImGui::SameLine();

            // Instruction Decode Stage:
            {
                float stage_width = ID_STAGE_WIDTH_FRAC * window_size.x;
                ImGui::BeginChild("Instruction Decode", ImVec2(stage_width, stage_height), true, ImGuiWindowFlags_NoDecoration);
                ImGui::Text("Instruction Decode");
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                ImVec2 top_left_window = ImGui::GetWindowPos();

                // Decoder (separates alu flags, registers, mux controls etc from instruction)
                ImVec2 top_left_decoder = {top_left_window.x + stage_width*0.07f, top_left_window.y + stage_height*0.4f};
                float decoder_height = 0.3f*stage_height;
                float decoder_width = 0.4f*decoder_height;
                ImVec2 bottom_right_decoder = {top_left_decoder.x + decoder_width, top_left_decoder.y + decoder_height};
                decoder = Rectangle{top_left_decoder, bottom_right_decoder};
                draw_list->AddRect(top_left_decoder, bottom_right_decoder, white_col, 0.0f, 0, thickness);

                // Register file
                ImVec2 top_left_register_file = {top_left_window.x + stage_width*0.4f, top_left_window.y + stage_height*0.3f};
                float register_file_height = 0.4f*stage_height;
                float register_file_width = 0.6f*register_file_height;
                ImVec2 bottom_right_register_file = {top_left_register_file.x+register_file_width,top_left_register_file.y+register_file_height};
                register_file = Rectangle{top_left_register_file, bottom_right_register_file};
                draw_list->AddRect(top_left_register_file, bottom_right_register_file, white_col, 0.0f, 0, thickness);

                // Immediate Generator
                immediate_gen_center = {top_left_window.x + stage_width*0.65f, top_left_window.y + stage_height*0.85f};
                float imm_gen_height = stage_height*0.2f;
                ImVec2 mux_radius = {imm_gen_height/4, imm_gen_height/2};
                immediate_gen_hradius = imm_gen_height/4;
                draw_list->AddEllipse(immediate_gen_center, mux_radius, white_col, 0.0f, 0, thickness);


                ImGui::EndChild();
            }

            ImGui::SameLine();

            // Execution Stage:
            {
                float stage_width = EX_STAGE_WIDTH_FRAC * window_size.x;
                ImGui::BeginChild("Execution", ImVec2(stage_width, stage_height), true, ImGuiWindowFlags_NoDecoration);
                exec_window_pos = ImGui::GetWindowPos();
                ImGui::Text("Execution");
                ImVec2 top_left_window = ImGui::GetWindowPos();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Main alu:
                ImVec2 top_left_main_alu = {top_left_window.x + stage_width*0.4f, top_left_window.y + stage_height*0.4f};
                float main_alu_height = stage_height*0.3f;
                main_alu = AluStruct(top_left_main_alu, main_alu_height);
                draw_alu(draw_list, main_alu, white_col, thickness);

                // Branch alu:
                ImVec2 top_left_branch_alu = {top_left_window.x + stage_width*0.4f, top_left_window.y + stage_height*0.1f};
                float branch_alu_height = stage_height*0.2f;
                branch_alu = AluStruct(top_left_branch_alu, branch_alu_height);
                draw_alu(draw_list, branch_alu, white_col, thickness);

                // Alu mux for immediate / register file
                float mux_height = stage_height*0.1f;
                float mux_width = mux_height/2;
                ImVec2 mux_top_left{top_left_window.x + stage_width*0.15f, main_alu.get_input2_line_coords().y-mux_height*0.5f};
                ImVec2 mux_bottom_right{mux_top_left.x + mux_width, mux_top_left.y + mux_height};
                alu_mux = Rectangle{mux_top_left, mux_bottom_right};
                draw_list->AddRect(mux_top_left, mux_bottom_right, white_col, mux_width/2, 0, thickness);

                // Shift left 1 for branch target
                branch_shift_center = {top_left_window.x + stage_width*0.2f, branch_alu.get_input2_line_coords().y};
                float shift_height = stage_height*0.07f;
                ImVec2 shift_radius = {shift_height/4, shift_height/2};
                branch_shift_radius = shift_height/4;
                draw_list->AddEllipse(branch_shift_center, shift_radius, white_col, 0.0f, 0, thickness);

                ImGui::EndChild();
            }

            ImGui::SameLine();

            // Memory Stage:
            {
                float stage_width = MEM_STAGE_WIDTH_FRAC * window_size.x;
                ImGui::BeginChild("Memory", ImVec2(stage_width, stage_height), true, ImGuiWindowFlags_NoDecoration);
                mem_window_pos = ImGui::GetWindowPos();
                ImGui::Text("Memory Access");
                ImVec2 top_left_window = ImGui::GetWindowPos();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Main memory
                ImVec2 top_left_memory = {top_left_window.x + stage_width*0.2f, top_left_window.y + stage_height*0.3f};
                float memory_height = 0.6f*stage_height;
                float memory_width = 0.4f*memory_height;
                ImVec2 bottom_right_memory = {top_left_memory.x+memory_width,top_left_memory.y+memory_height};
                main_mem = Rectangle{top_left_memory, bottom_right_memory};
                draw_list->AddRect(top_left_memory, bottom_right_memory, white_col, 0.0f, 0, thickness);

                ImGui::EndChild();
            }

            ImGui::SameLine();

            // Write Back Stage:
            {
                float stage_width = WB_STAGE_WIDTH_FRAC * window_size.x;
                ImGui::BeginChild("Write Back", ImVec2(stage_width, stage_height), true, ImGuiWindowFlags_NoDecoration);
                ImGui::Text("Write Back");
                ImVec2 top_left_window = ImGui::GetWindowPos();
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Mux for selecting alu ouput / memory out
                float mux_height = stage_height*0.15f;
                float mux_width = stage_width*0.4f;
                ImVec2 mux_top_left = {top_left_window.x + (stage_width - mux_width)*0.5f, top_left_window.y + stage_height*0.5f - mux_height/2};
                ImVec2 mux_bottom_right = {mux_top_left.x + mux_width, mux_top_left.y + mux_height};
                wb_mux = Rectangle(mux_top_left, mux_bottom_right);
                draw_list->AddRect(mux_top_left, mux_bottom_right, white_col, mux_width/2.0f, 0, thickness);

                ImGui::EndChild();
            }

            // Instruction fetch lines;
            {
                float if_window_width = IF_STAGE_WIDTH_FRAC * window_size.x;
                float mux_height = pc_adder_mux.get_height();
                // Branch alu to pc mux line:
                {
                    float exec_window_width = EX_STAGE_WIDTH_FRAC * window_size.x;
                    float line_offset = pc_adder_mux.top_left.x - if_window_width*0.06f;
                    
                    ImVec2 branch_alu_output_line_pos = branch_alu.get_output_line_coords();
                    ImVec2 p1{branch_alu_output_line_pos.x + exec_window_width*0.1f, branch_alu_output_line_pos.y};
                    ImVec2 p2{p1.x, p1.y - stage_height*0.13f};
                    ImVec2 p3{line_offset, p2.y};
                    ImVec2 p4{line_offset, pc_adder_mux.top_left.y+mux_height*0.75f};
                    ImVec2 p5{pc_adder_mux.top_left.x, p4.y};
                    std::vector<ImVec2> points = {branch_alu_output_line_pos, p1, p2, p3, p4, p5};

                    draw_line_connecting_points(points, white_col, thickness);
                }

                // pc 4 adder to pc mux line:
                {
                    float line_offset = pc_adder_mux.top_left.x - if_window_width*0.03f;

                    ImVec2 pc_4_adder_output_line_pos = pc_4_adder.get_output_line_coords();
                    ImVec2 p1{pc_4_adder_output_line_pos.x + if_window_width*0.1f, pc_4_adder_output_line_pos.y};
                    ImVec2 p2{p1.x, p1.y - stage_height*0.13f};
                    ImVec2 p3{line_offset, p2.y};
                    ImVec2 p4{line_offset, pc_adder_mux.top_left.y+mux_height*0.25f};
                    ImVec2 p5{pc_adder_mux.top_left.x, p4.y};
                    std::vector<ImVec2> points = {pc_4_adder_output_line_pos, p1, p2, p3, p4, p5};

                    draw_line_connecting_points(points, white_col, thickness);
                }

                // pc to adders and instruction memory lines:
                {
                    // pc to adders lines:

                    ImVec2 input2_pc_4_adder = pc_4_adder.get_input2_line_coords();
                    ImVec2 input1_branch_alu = branch_alu.get_input1_line_coords();
                    ImVec2 p1{input2_pc_4_adder.x - 0.05f*if_window_width, input2_pc_4_adder.y};
                    ImVec2 p2{p1.x, pc.top_left.y - stage_height*0.05f};
                    ImVec2 p3{(pc.bottom_right.x + instruction_mem.top_left.x) / 2.0f, p2.y};
                    ImVec2 p4{if_window_width - 0.1f*if_window_width, p3.y};
                    ImVec2 p5{p4.x, input1_branch_alu.y};
                    std::vector<ImVec2> points = {input2_pc_4_adder, p1, p2, p3, p4, p5, input1_branch_alu};

                    draw_line_connecting_points(points, white_col, thickness);

                    ImVec2 p6{p3.x,(pc.bottom_right.y + pc.top_left.y) / 2.0f};
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    draw_list->AddLine(p3,p6,white_col,thickness);


                    // mux to pc line:
                    ImVec2 pc_adder_mux_output{pc_adder_mux.bottom_right.x, (pc_adder_mux.top_left.y + pc_adder_mux.bottom_right.y) / 2.0f};
                    ImVec2 pc_input{pc.top_left.x,(pc.top_left.y + pc.bottom_right.y) / 2.0f};
                    draw_list->AddLine(pc_adder_mux_output, pc_input, white_col, thickness);

                    // pc to instruction memory line:
                    ImVec2 pc_output_line_coords{pc.bottom_right.x, p6.y};
                    ImVec2 instruction_mem_input_coords{instruction_mem.top_left.x, p6.y};
                    draw_list->AddLine(pc_output_line_coords, instruction_mem_input_coords, white_col, thickness);

                    // instructon memory to decoder line:
                    ImVec2 instruction_mem_output_coords{instruction_mem.bottom_right.x, instruction_mem_input_coords.y};
                    ImVec2 decoder_input_coords{decoder.top_left.x, instruction_mem_output_coords.y};
                    draw_list->AddLine(instruction_mem_output_coords, decoder_input_coords, white_col, thickness);
                }
            }


            // Instruction Decode Lines:
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                float ex_stage_width = EX_STAGE_WIDTH_FRAC * window_size.x;
                // Decoder to register file / immediate generator lines:
                {
                    ImVec2 decoder_output{decoder.bottom_right.x, (decoder.top_left.y + decoder.bottom_right.y) * 0.5f};
                    ImVec2 decoder_offset{decoder_output.x + ex_stage_width*0.05f, decoder_output.y};
                    
                    float register_file_height = register_file.get_height();
                    ImVec2 register_file_input1{register_file.top_left.x, register_file.top_left.y + register_file_height/4.0f};
                    ImVec2 register_file_input2{register_file.top_left.x, register_file.top_left.y + register_file_height/2.0f};

                    ImVec2 imm_gen_input{immediate_gen_center.x - immediate_gen_hradius, immediate_gen_center.y};

                    ImVec2 rf_s1{decoder_offset.x, register_file_input1.y};
                    ImVec2 rf_s2{decoder_offset.x, register_file_input2.y};

                    ImVec2 imm_s{decoder_offset.x, imm_gen_input.y};

                    draw_list->AddLine(decoder_output, decoder_offset, white_col, thickness);
                    draw_list->AddLine(decoder_offset, rf_s1, white_col, thickness);
                    draw_list->AddLine(decoder_offset, rf_s2, white_col, thickness);
                    draw_list->AddLine(decoder_offset, imm_s, white_col, thickness);

                    draw_list->AddLine(rf_s1, register_file_input1, white_col, thickness);
                    draw_list->AddLine(rf_s2, register_file_input2, white_col, thickness);

                    draw_list->AddLine(imm_s, imm_gen_input, white_col, thickness);
                }

                // register file to alu and memory
                {
                    // register file to alu
                    float alu_mux_height = alu_mux.get_height();
                    ImVec2 main_alu_input1 = main_alu.get_input1_line_coords();
                    ImVec2 alu_mux_input1{alu_mux.top_left.x, alu_mux.top_left.y + alu_mux_height*0.25f};
                    ImVec2 register_file_output_reg1{register_file.bottom_right.x, main_alu.get_input1_line_coords().y};
                    ImVec2 register_file_output_reg2{register_file.bottom_right.x, alu_mux_input1.y};
    
                    draw_list->AddLine(register_file_output_reg1, main_alu_input1, white_col, thickness);
                    draw_list->AddLine(register_file_output_reg2, alu_mux_input1, white_col, thickness);


                    // register file to memory
                    ImVec2 write_port_main_mem{main_mem.top_left.x, main_mem.top_left.y + main_mem.get_height()*0.85f};

                    float ex_stage_x_offset = exec_window_pos.x;
                    float ex_stage_width = EX_STAGE_WIDTH_FRAC * window_size.x;
                    ImVec2 d_offset{ex_stage_x_offset + ex_stage_width*0.05f, register_file_output_reg2.y};
                    ImVec2 wpmm_s{d_offset.x, write_port_main_mem.y};

                    draw_list->AddLine(d_offset, wpmm_s, white_col, thickness);
                    draw_list->AddLine(wpmm_s, write_port_main_mem, white_col, thickness);
                }

                // immediate generator to alu_mux and branch shift
                {
                    float ex_stage_x_offset = exec_window_pos.x;
                    float ex_stage_width = EX_STAGE_WIDTH_FRAC * window_size.x;

                    ImVec2 imm_gen_output{immediate_gen_center.x + immediate_gen_hradius, immediate_gen_center.y};
                    ImVec2 imm_gen_offset{ex_stage_x_offset + ex_stage_width*0.1f, imm_gen_output.y};

                    float alu_mux_height = alu_mux.get_height();
                    ImVec2 alu_mux_input2{alu_mux.top_left.x, alu_mux.top_left.y + alu_mux_height*0.75f};

                    ImVec2 branch_shift_input{branch_shift_center.x - branch_shift_radius, branch_shift_center.y};

                    ImVec2 bs_s{imm_gen_offset.x, branch_shift_input.y};
                    ImVec2 am_s{imm_gen_offset.x, alu_mux_input2.y};

                    draw_list->AddLine(imm_gen_output, imm_gen_offset, white_col, thickness);
                    draw_list->AddLine(imm_gen_offset, bs_s, white_col, thickness);
                    draw_list->AddLine(bs_s, branch_shift_input, white_col, thickness);
                    draw_list->AddLine(imm_gen_offset, am_s, white_col, thickness);
                    draw_list->AddLine(am_s, alu_mux_input2, white_col, thickness);
                }
            }


            // Execution Stage Lines:
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // alu_mux to main_alu:
                {
                    float alu_mux_height = alu_mux.get_height();
                    ImVec2 mux_output{alu_mux.bottom_right.x, alu_mux.top_left.y + alu_mux_height*0.5f};
                    ImVec2 alu_input2 = main_alu.get_input2_line_coords();

                    draw_list->AddLine(alu_input2,mux_output,white_col,thickness);
                }
                
                // branch_shift to branch_alu line:
                {
                    ImVec2 branch_shift_output{branch_shift_center.x+branch_shift_radius, branch_shift_center.y};
                    ImVec2 branch_alu_input2 = branch_alu.get_input2_line_coords();

                    draw_list->AddLine(branch_shift_output, branch_alu_input2, white_col, thickness);
                }

                // alu to main memory and wb_mux
                {
                    float mem_window_size = MEM_STAGE_WIDTH_FRAC * window_size.x;
                    float wb_window_size = WB_STAGE_WIDTH_FRAC * window_size.x;

                    ImVec2 main_alu_out = main_alu.get_output_line_coords();
                    ImVec2 alu_out_offset{mem_window_pos.x + mem_window_size * 0.1f, main_alu_out.y};
                    ImVec2 alu_u_off{alu_out_offset.x, mem_window_pos.y + stage_height*0.2f};
                    ImVec2 mux_u_off{wb_mux.top_left.x - wb_window_size*0.1f, alu_u_off.y};
                    ImVec2 mux_l_off{mux_u_off.x, wb_mux.top_left.y + wb_mux.get_height()/3.0f};
                    ImVec2 mux_input1{wb_mux.top_left.x, mux_l_off.y};

                    ImVec2 memory_input{main_mem.top_left.x, main_alu_out.y};

                    draw_list->AddLine(main_alu_out, memory_input, white_col, thickness);
                    std::vector<ImVec2> points{alu_out_offset, alu_u_off, mux_u_off, mux_l_off, mux_input1};
                    draw_line_connecting_points(points, white_col, thickness);
                }
            }

            // Memory Lines
            {
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                // memory to wb_mux
                {
                    ImVec2 wb_mux_input2{wb_mux.top_left.x, wb_mux.top_left.y + wb_mux.get_height()*2/3.0f};
                    ImVec2 memory_out{main_mem.bottom_right.x, wb_mux_input2.y};

                    draw_list->AddLine(wb_mux_input2, memory_out, white_col, thickness);
                }
            }

            // Write Back Lines
            {
                float wb_window_size = WB_STAGE_WIDTH_FRAC * window_size.x;
                ImVec2 wb_mux_out{wb_mux.bottom_right.x, wb_mux.top_left.y + wb_mux.get_height()*0.5f};
                ImVec2 wb_mux_r_off{wb_mux_out.x + wb_window_size*0.1f, wb_mux_out.y};
                ImVec2 wb_mux_d_off{wb_mux_r_off.x, wb_mux_r_off.y + stage_height*0.48f};

                float id_window_size = ID_STAGE_WIDTH_FRAC * window_size.x;
                ImVec2 write_register_file{register_file.top_left.x, register_file.top_left.y + register_file.get_height()*0.75f};
                ImVec2 wrf_l_off{write_register_file.x - id_window_size*0.05f, write_register_file.y};
                ImVec2 wrf_d_off{wrf_l_off.x, wb_mux_d_off.y};

                std::vector<ImVec2> points{wb_mux_out, wb_mux_r_off, wb_mux_d_off, wrf_d_off, wrf_l_off, write_register_file};
                draw_line_connecting_points(points, white_col, thickness);
            }


            ImGui::PopStyleVar(); // restore spacing

            ImGui::End();
        }
        // ImGui::PopStyleVar();
        
        // Demo window
        if(show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
#ifdef __EMSCRIPTEN__
    EMSCRIPTEN_MAINLOOP_END;
#endif

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
