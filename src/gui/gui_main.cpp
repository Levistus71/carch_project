// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)

// Learn about Dear ImGui:
// - FAQ                  https://dearimgui.com/faq
// - Getting Started      https://dearimgui.com/getting-started
// - Documentation        https://dearimgui.com/docs (same as your local docs/ folder).
// - Introduction, links and more at the top of imgui.cpp

#include "../../include/gui/gui_main.h"
#include "../../include/gui/gui_set_processor_type.h"
#include "gui/gui_stats.h"


static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

// Main code
int gui_main()
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

    
    LoadFonts(io);
    ImGui::PushFont(STANDARD_SMALL_FONT);


    // main()
    globals::vm_cout_file << "Virtual Machine started. Hello!" << std::endl;
    
    
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
            const ImVec2 left_panel_size = ImVec2(left_panel_width, main_viewport_size.y - top_panel_height);
            const ImVec2 left_panel_pos  = ImVec2(main_viewport_pos.x, main_viewport_pos.y + top_panel_height);

            ImGui::SetNextWindowPos(left_panel_pos, ImGuiCond_Always);
            ImGui::SetNextWindowSize(left_panel_size, ImGuiCond_Always);

            ImGui::Begin("Left Panel", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBackground
            );

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_Button,ImVec4(0.16f, 0.19f, 0.23f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImVec4(0.22f, 0.26f, 0.32f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,ImVec4(0.28f, 0.33f, 0.40f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text,ImVec4(0.88f, 0.91f, 0.95f, 1.0f));

            const float icon_button_size = left_panel_width * 0.88f;
            const float vertical_spacing = 16.0f;

            if(ImGui::Button("Editor",ImVec2(icon_button_size, icon_button_size))){
                in_editor = true;
                in_processor = in_execute = in_memory = false;
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+vertical_spacing);

            if(ImGui::Button("Processor",ImVec2(icon_button_size,icon_button_size))){
                in_processor = true;
                in_editor = in_execute = in_memory = false;
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+vertical_spacing);

            if(ImGui::Button("Execute",ImVec2(icon_button_size,icon_button_size))){
                in_execute = true;
                in_editor = in_processor = in_memory = false;
            }
            ImGui::SetCursorPosY(ImGui::GetCursorPosY()+vertical_spacing);

            if(ImGui::Button("Memory",ImVec2(icon_button_size,icon_button_size))){
                in_memory = true;
                in_editor = in_processor = in_execute = false;
            }

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);
            ImGui::End();
        }

        static bool SHOW_CONSOLE_IN_PROCESSOR = true;
        {
            const ImVec2 top_panel_size   = ImVec2(main_viewport_size.x, top_panel_height);
            const ImVec2 top_panel_origin = ImVec2(main_viewport_pos.x, main_viewport_pos.y);

            ImGui::SetNextWindowPos(top_panel_origin, ImGuiCond_Always);
            ImGui::SetNextWindowSize(top_panel_size, ImGuiCond_Always);

            ImGui::Begin("Top Panel", nullptr,
                ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse |
                ImGuiWindowFlags_NoBackground
            );

            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10.0f, 8.0f));
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.18f, 0.22f, 0.28f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.25f, 0.30f, 0.38f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.32f, 0.38f, 0.48f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.92f, 0.94f, 0.97f, 1.0f));


            const float button_width  = left_panel_width * 0.82f;
            const float button_height = top_panel_height * 0.86f;
            const float spacing       = 12.0f;

            if(ImGui::Button("Assemble",ImVec2(button_width,button_height))){
                text_editor.SaveFile();
                try{
                    vm.LoadVM(assemble(text_editor.FilePath()));
                    globals::vm_cout_file << "Assembly successful!" << std::endl << std::endl;
                    in_editor = false; in_execute = true; in_processor = false; in_memory = false;
                } catch(const std::runtime_error& e){
                    globals::vm_cout_file << "Assembly failed." << std::endl;
                    std::cerr << "Error: " << e.what() << std::endl;
                }
            }
            ImGui::SameLine(0.0f, spacing);

            if(ImGui::Button("Undo", ImVec2(button_width,button_height))){
                vm.Undo();
            }
            ImGui::SameLine(0.0f, spacing);

            if(ImGui::Button("Step", ImVec2(button_width, button_height))){
                vm.Step();
            }
            ImGui::SameLine(0.0f, spacing);

            if(ImGui::Button("Run", ImVec2(button_width,button_height))) {
                vm.DebugRun();
            }
            ImGui::SameLine(0.0f, spacing);

            if(ImGui::Button("Run FF", ImVec2(button_width,button_height))) {
                vm.Run();
            }

            if(in_processor){
                ImGui::SameLine(0.0f, spacing);
                if(ImGui::Button("Console", ImVec2(button_width, button_height))){
                    SHOW_CONSOLE_IN_PROCESSOR = !SHOW_CONSOLE_IN_PROCESSOR;
                }
            }

            ImGui::SameLine(0.0f, spacing);
            if(ImGui::Button("Processor", ImVec2(button_width, button_height))){
                ImGui::OpenPopup("Processor Selection Modal");
            }

            ImGui::PopStyleColor(4);
            ImGui::PopStyleVar(2);

            // Processor selection modal
            // The Processor Selection Salon – a modal of refined architectural discernment
            {
                ImGui::SetNextWindowSize(ImVec2(500.0f, 300.0f), ImGuiCond_Appearing);
    
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    
                const ImGuiWindowFlags modal_flags = ImGuiWindowFlags_NoCollapse;
                if(ImGui::BeginPopupModal("Processor Selection Modal", NULL, modal_flags)){
                    set_processor_type();
    
                    if (ImGui::Button("Close"))
                        ImGui::CloseCurrentPopup();
    
                    ImGui::EndPopup();
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
                ImVec2 PROCESSOR_SHEATH_SIZE = (SHOW_CONSOLE_IN_PROCESSOR) ? ImVec2(CENTER_SIZE.x * 0.75f, CENTER_SIZE.y * 0.65f) : ImVec2(CENTER_SIZE.x * 0.75f, CENTER_SIZE.y);
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
                ImVec2 REGISTER_WINDOW_SIZE{CENTER_SIZE.x - PROCESSOR_SHEATH_SIZE.x, CENTER_SIZE.y * 0.65f};
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
                if(SHOW_CONSOLE_IN_PROCESSOR){
                    ImVec2 CONSOLE_WINDOW_POS{CENTER_POS.x, CENTER_POS.y + PROCESSOR_SHEATH_SIZE.y};
                    ImVec2 CONSOLE_WINDOW_SIZE{PROCESSOR_SHEATH_SIZE.x, CENTER_SIZE.y - PROCESSOR_SHEATH_SIZE.y};
                    ImGui::SetNextWindowPos(CONSOLE_WINDOW_POS);
                    ImGui::SetNextWindowSize(CONSOLE_WINDOW_SIZE);
                    ImGui::Begin("Console Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                    {
                        console_main();
                    }
                    ImGui::End();
                }


                // stats (or) instructions {like in ripes}, not decided yet
                ImVec2 STATS_WINDOW_POS{CENTER_POS.x + PROCESSOR_SHEATH_SIZE.x, CENTER_POS.y + REGISTER_WINDOW_SIZE.y};
                ImVec2 STATS_WINDOW_SIZE{CENTER_SIZE.x - PROCESSOR_SHEATH_SIZE.x, CENTER_SIZE.y - REGISTER_WINDOW_SIZE.y};
                ImGui::SetNextWindowPos(STATS_WINDOW_POS);
                ImGui::SetNextWindowSize(STATS_WINDOW_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(60.0f/255.0f, 60.0f/255.0f, 60.0f/255.0f, 1.00f));
                ImGui::Begin("Stats Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    stats_main();
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
                float navigator_width = 50.0f;
                ImVec2 MEMORY_SHEATH_SIZE{CENTER_SIZE.x - navigator_width, CENTER_SIZE.y * 0.90f};
                ImVec2 MEMORY_SHEATH_POS{CENTER_POS.x, CENTER_POS.y};
                ImGui::SetNextWindowPos(MEMORY_SHEATH_POS);
                ImGui::SetNextWindowSize(MEMORY_SHEATH_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // loading gray color bg
                ImGui::Begin("Memory Sheath Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    memory_main();
                }
                ImGui::PopStyleColor();
                ImGui::End();


                ImVec2 MEMORY_NAVIGATOR_POS{CENTER_POS.x + MEMORY_SHEATH_SIZE.x, MEMORY_SHEATH_POS.y};
                ImVec2 MEMORY_NAVIGATOR_SIZE{navigator_width, MEMORY_SHEATH_SIZE.y};
                ImGui::SetNextWindowPos(MEMORY_NAVIGATOR_POS);
                ImGui::SetNextWindowSize(MEMORY_NAVIGATOR_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // loading gray color bg
                ImGui::Begin("Memory Navigator", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    memory_navigator_main();
                }
                ImGui::End();
                ImGui::PopStyleColor();

                ImVec2 MEMORY_VARS_POS{CENTER_POS.x, CENTER_POS.y + MEMORY_SHEATH_SIZE.y};
                ImVec2 MEMORY_VARS_SIZE{CENTER_SIZE.x, CENTER_SIZE.y - MEMORY_SHEATH_SIZE.y};
                ImGui::SetNextWindowPos(MEMORY_VARS_POS);
                ImGui::SetNextWindowSize(MEMORY_VARS_SIZE);
                ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.1211f, 0.1211f, 0.1211f, 1.00f)); // loading gray color bg
                ImGui::Begin("Memory Window Variables", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
                {
                    memory_vars_main();
                }
                ImGui::End();
                ImGui::PopStyleColor();
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
