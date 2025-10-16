cmake_minimum_required(VERSION 3.21)

project(vm LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SRC_DIR "src")
set(INCLUDE_DIR "include")

file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp")

# Exclude anything under src/gui
file(GLOB_RECURSE GUI_FILES
    "${SRC_DIR}/gui/*.cpp"
)
list(REMOVE_ITEM SRC_FILES ${GUI_FILES})

add_executable(${PROJECT_NAME} ${SRC_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -pedantic -frounding-math -ffloat-store -g -O3)
target_link_libraries(${PROJECT_NAME} PRIVATE m)

add_custom_target(run
    COMMAND ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target(debug
    COMMAND gdb ${PROJECT_NAME}
    DEPENDS ${PROJECT_NAME}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)


message(STATUS "Done")
