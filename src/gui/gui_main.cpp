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

#include "../../include/gui/gui_processor_window.h"
#include "../../include/gui/gui_editor.h"
#include "../../include/gui/gui_common.h"
#include "../../include/gui/gui_register.h"
#include "../../include/gui/gui_console.h"
#include "../../include/gui/gui_execute.h"
#include "../../include/gui/gui_memory.h"


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
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
    GLFWwindow* window = glfwCreateWindow((int)(1280 * main_scale), (int)(800 * main_scale), "RISCV Simulator", nullptr, nullptr);
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
    
    LoadFonts(io);
    ImGui::PushFont(STANDARD_SMALL_FONT);
    
    // Main loop
    while (!glfwWindowShouldClose(window))
    {
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

        ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImVec2 main_viewport_pos = main_viewport->Pos;
        ImVec2 main_viewport_size = main_viewport->Size; 
        
        float top_panel_height = 0.05f * main_viewport_size.y;
        float left_panel_width = 0.1f * main_viewport_size.y;

        // Left Panel (Window selector)
        {
            ImVec2 left_size = ImVec2(left_panel_width, main_viewport_size.y - top_panel_height);
            ImVec2 left_pos = ImVec2(main_viewport_pos.x, main_viewport_pos.y + top_panel_height);
            ImGui::SetNextWindowPos(left_pos);
            ImGui::SetNextWindowSize(left_size);
            ImGui::Begin("Left Panel", nullptr, 
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            );
            {
                if(ImGui::Button("Editor", {left_panel_width * 0.9f, left_panel_width * 0.9f})){
                    in_editor = true;
                    in_processor = false;
                    in_execute = false;
                    in_memory = false;
                }
                if(ImGui::Button("Processor", {left_panel_width * 0.9f, left_panel_width * 0.9f})){
                    in_editor = false;
                    in_processor = true;
                    in_execute = false;
                    in_memory = false;
                }
                if(ImGui::Button("Execute", {left_panel_width * 0.9f, left_panel_width * 0.9f})){
                    in_editor = false;
                    in_processor = false;
                    in_execute = true;
                    in_memory = false;
                }
                if(ImGui::Button("Memory", {left_panel_width * 0.9f, left_panel_width * 0.9f})){
                    in_editor = false;
                    in_processor = false;
                    in_execute = false;
                    in_memory = true;
                }
            }
            ImGui::End();
        }


        // Top panel
        {
            ImVec2 top_size = ImVec2(main_viewport_size.x, top_panel_height);
            ImVec2 top_pos = ImVec2(main_viewport_pos.x, main_viewport_pos.y);
            ImGui::SetNextWindowPos(top_pos);
            ImGui::SetNextWindowSize(top_size);
            ImGui::Begin("Top Panel", nullptr, 
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            );
            {
                float offset = 0;
                if(ImGui::Button("Assemble", {left_panel_width * 0.8f, top_panel_height * 0.9f})){
                    
                }
                offset+=left_panel_width;
                ImGui::SameLine(offset, 1.0f);
                if(ImGui::Button("Undo", {left_panel_width * 0.8f, top_panel_height * 0.9f})){
                    
                }
                offset+=left_panel_width;
                ImGui::SameLine(offset, 1.0f);
                if(ImGui::Button("Step Forward", {left_panel_width * 0.8f, top_panel_height * 0.9f})){
                    
                }
                offset+=left_panel_width;
                ImGui::SameLine(offset, 1.0f);
                if(ImGui::Button("Run (debug)", {left_panel_width * 0.8f, top_panel_height * 0.9f})){
                    
                }
                offset+=left_panel_width;
                ImGui::SameLine(offset, 1.0f);
                if(ImGui::Button("Run (no interaction)", {left_panel_width * 0.8f, top_panel_height * 0.9f})){
                    
                }
            }
            ImGui::End();
        }

        // Central panel
        {
            ImVec2 CENTER_SIZE = ImVec2(main_viewport_size.x - left_panel_width, main_viewport_size.y - top_panel_height);
            ImVec2 CENTER_POS = ImVec2(main_viewport_pos.x + left_panel_width, main_viewport_pos.y + top_panel_height);
            ImGui::SetNextWindowPos(CENTER_POS);
            ImGui::SetNextWindowSize(CENTER_SIZE);
            ImGui::Begin("Central Window", nullptr, 
                ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse
            );

            // processor
            if(in_processor)
            {
                // processor
                ImVec2 PROCESSOR_WINDOW_POS{CENTER_POS.x, CENTER_POS.y};
                ImVec2 PROCESSOR_SHEATH_SIZE{CENTER_SIZE.x * 0.75f, CENTER_SIZE.y * 0.65f};
                ImGui::SetNextWindowPos(PROCESSOR_WINDOW_POS);
                ImGui::SetNextWindowSize(PROCESSOR_SHEATH_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // loading gray color bg
                ImGui::Begin("Processor Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize);
                {
                    processor_main();
                }
                ImGui::End();
                ImGui::PopStyleColor();

                // registers
                ImVec2 REGISTER_WINDOW_POS{CENTER_POS.x + PROCESSOR_SHEATH_SIZE.x, CENTER_POS.y};
                ImVec2 REGISTER_WINDOW_SIZE{CENTER_SIZE.x - PROCESSOR_SHEATH_SIZE.x, PROCESSOR_SHEATH_SIZE.y};
                ImGui::SetNextWindowPos(REGISTER_WINDOW_POS);
                ImGui::SetNextWindowSize(REGISTER_WINDOW_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(31.0f/255.0f, 31.0f/255.0f, 31.0f/255.0f, 1.00f)); // loading gray color bg
                ImGui::Begin("Register Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    register_main();
                }
                ImGui::End();
                ImGui::PopStyleColor();

                // console
                ImVec2 CONSOLE_WINDOW_POS{CENTER_POS.x, CENTER_POS.y + PROCESSOR_SHEATH_SIZE.y};
                ImVec2 CONSOLE_WINDOW_SIZE{PROCESSOR_SHEATH_SIZE.x, CENTER_SIZE.y - PROCESSOR_SHEATH_SIZE.y};
                ImGui::SetNextWindowPos(CONSOLE_WINDOW_POS);
                ImGui::SetNextWindowSize(CONSOLE_WINDOW_SIZE);
                ImGui::Begin("Console Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    console_main();
                }
                ImGui::End();


                // stats (or) instructions {like in ripes}, not decided yet
                ImVec2 STATS_WINDOW_POS{CENTER_POS.x + CONSOLE_WINDOW_SIZE.x, CONSOLE_WINDOW_POS.y};
                ImVec2 STATS_WINDOW_SIZE{CENTER_SIZE.x - CONSOLE_WINDOW_SIZE.x, CONSOLE_WINDOW_SIZE.y};
                ImGui::SetNextWindowPos(STATS_WINDOW_POS);
                ImGui::SetNextWindowSize(STATS_WINDOW_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(255.0f/255.0f, 255.0f/255.0f, 255.0f/255.0f, 1.00f));
                ImGui::Begin("Stats Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    
                }
                ImGui::PopStyleColor();
                ImGui::End();
            }
            // editor
            else if(in_editor)
            {
                ImVec2 EDITOR_WINDOW_POS{CENTER_POS.x, CENTER_POS.y};
                ImVec2 EDITOR_SHEATH_SIZE{CENTER_SIZE.x, CENTER_SIZE.y};
                ImGui::SetNextWindowPos(EDITOR_WINDOW_POS);
                ImGui::SetNextWindowSize(EDITOR_SHEATH_SIZE);
                ImGui::Begin("Editor Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    editor_main();
                }
                ImGui::End();
            }
            // execute
            else if(in_execute){
                // The Editor for side by side comparision (like in RARS)
                ImVec2 EDITOR_WINDOW_POS{CENTER_POS.x, CENTER_POS.y};
                ImVec2 EDITOR_SHEATH_SIZE{CENTER_SIZE.x * 0.75f / 2.0f, CENTER_SIZE.y * 0.65f};
                ImGui::SetNextWindowPos(EDITOR_WINDOW_POS);
                ImGui::SetNextWindowSize(EDITOR_SHEATH_SIZE);
                ImGui::Begin("Editor Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    editor_execute();
                }
                ImGui::End();

                ImVec2 ASSEMBLED_WINDOW_POS{CENTER_POS.x + EDITOR_SHEATH_SIZE.x, CENTER_POS.y};
                ImVec2 ASSEMBLED_WINDOW_SIZE{CENTER_SIZE.x * 0.75f / 2.0f, CENTER_SIZE.y * 0.65f};
                ImGui::SetNextWindowPos(ASSEMBLED_WINDOW_POS);
                ImGui::SetNextWindowSize(ASSEMBLED_WINDOW_SIZE);
                ImGui::Begin("Assembled Editor Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    assembled_editor_main();
                }
                ImGui::End();

                // console
                ImVec2 CONSOLE_WINDOW_POS{CENTER_POS.x, CENTER_POS.y + EDITOR_SHEATH_SIZE.y};
                ImVec2 CONSOLE_WINDOW_SIZE{EDITOR_SHEATH_SIZE.x + ASSEMBLED_WINDOW_SIZE.x, CENTER_SIZE.y - EDITOR_SHEATH_SIZE.y};
                ImGui::SetNextWindowPos(CONSOLE_WINDOW_POS);
                ImGui::SetNextWindowSize(CONSOLE_WINDOW_SIZE);
                ImGui::Begin("Console Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    console_main();
                }
                ImGui::End();

                // registers
                ImVec2 REGISTER_WINDOW_POS{CENTER_POS.x + EDITOR_SHEATH_SIZE.x + ASSEMBLED_WINDOW_SIZE.x, CENTER_POS.y};
                ImVec2 REGISTER_WINDOW_SIZE{CENTER_SIZE.x - (EDITOR_SHEATH_SIZE.x + ASSEMBLED_WINDOW_SIZE.x), CENTER_SIZE.y};
                ImGui::SetNextWindowPos(REGISTER_WINDOW_POS);
                ImGui::SetNextWindowSize(REGISTER_WINDOW_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(31.0f/255.0f, 31.0f/255.0f, 31.0f/255.0f, 1.00f)); // loading gray color bg
                ImGui::Begin("Register Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    register_execute();
                }
                ImGui::End();
                ImGui::PopStyleColor();
            }
            // memory
            else if(in_memory){
                ImVec2 MEMORY_SHEATH_POS{CENTER_POS.x, CENTER_POS.y};
                ImVec2 MEMORY_SHEATH_SIZE{CENTER_SIZE.x, CENTER_SIZE.y};
                ImGui::SetNextWindowPos(MEMORY_SHEATH_POS);
                ImGui::SetNextWindowSize(MEMORY_SHEATH_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // loading gray color bg
                ImGui::Begin("Memory Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    memory_main();
                }
                ImGui::PopStyleColor();
                ImGui::End();
            }

            ImGui::End();
        }

        // ImGui::ShowDemoWindow();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    ImGui::PopFont();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
