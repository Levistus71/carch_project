cmake_minimum_required(VERSION 3.21)

project(vm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_CXX_FLAGS_DEBUG "-O1 -g")

set(SRC_DIR "src")
set(INCLUDE_DIR "include")
set(IMGUI_DIR "imgui")

# --- (non-GUI) ---
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp")

# Exclude anything under src/gui
file(GLOB_RECURSE GUI_FILES
    "${SRC_DIR}/gui/*.cpp"
)
list(REMOVE_ITEM SRC_FILES ${GUI_FILES})

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -frounding-math -ffloat-store -g)
target_link_libraries(${PROJECT_NAME} PRIVATE m)

# --- 'vm_gui' executable ---
set(GUI_SRC_FILES
    # GUI specific files from src/gui
    ${SRC_DIR}/gui/gui_main.cpp
    ${SRC_DIR}/gui/gui_processor_window.cpp
    ${SRC_DIR}/gui/gui_editor.cpp
    ${SRC_DIR}/gui/gui_common.cpp
    ${SRC_DIR}/gui/gui_register.cpp
    ${SRC_DIR}/gui/gui_console.cpp
    ${SRC_DIR}/gui/gui_execute.cpp
    ${SRC_DIR}/gui/gui_memory.cpp
)

# Use file(GLOB) for the external library sources
file(GLOB IMGUI_CORE_SOURCES
    "${IMGUI_DIR}/*.cpp"
)
file(GLOB IMGUI_MISC_SOURCES
    "${IMGUI_DIR}/misc/cpp/*.cpp"
)
set(IMGUI_BACKEND_SOURCES
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_executable(vm_gui
    ${GUI_SRC_FILES}
    ${IMGUI_CORE_SOURCES}
    ${IMGUI_MISC_SOURCES}
    ${IMGUI_BACKEND_SOURCES}
)

target_include_directories(vm_gui PRIVATE
    ${INCLUDE_DIR}
    ${IMGUI_DIR}
    /opt/homebrew/include
    /usr/local/include
)

target_compile_options(vm_gui PRIVATE
    -Wall -Wextra -pedantic -g -O2
)

## 6. Link Libraries
find_package(OpenGL REQUIRED)

target_link_libraries(vm_gui PRIVATE
    /opt/homebrew/lib/libglfw3.a  # Explicit path to libglfw3.a, or just 'glfw3' if the linker can find it

    ${OPENGL_LIBRARIES}
    "-framework Cocoa"
    "-framework IOKit"
    "-framework CoreVideo"
    "-framework OpenGL"
)

add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(run_gui
    COMMAND vm_gui
    DEPENDS vm_gui
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(debug
    COMMAND gdb ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(debug_gui
    COMMAND gdb vm_gui
    DEPENDS vm_gui
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

message(STATUS "SRC_FILES: ${SRC_FILES}")
message(STATUS "Done")