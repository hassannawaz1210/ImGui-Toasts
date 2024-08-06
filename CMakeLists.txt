cmake_minimum_required(VERSION 3.10)
project(NotificationSystem)

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add source files
set(SOURCES
    main.cpp
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
)

# Add executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Find OpenGL
find_package(OpenGL REQUIRED)

# Include directories
include_directories(${OPENGL_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/Libraries/glfw/include)
include_directories(${CMAKE_SOURCE_DIR}/imgui)

# Link GLFW library
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} ${CMAKE_SOURCE_DIR}/Libraries/glfw/lib/glfw3.lib)

# Create source group for ImGui files
source_group("ImGui" FILES
    imgui/imgui.cpp
    imgui/imgui_draw.cpp
    imgui/imgui_tables.cpp
    imgui/imgui_widgets.cpp
    imgui/imgui_impl_glfw.cpp
    imgui/imgui_impl_opengl3.cpp
)

# Copy icons directory to the build directory
file(COPY "${CMAKE_SOURCE_DIR}/icons" DESTINATION "${CMAKE_BINARY_DIR}")

# Copy fonts directory to the build directory
file(COPY "${CMAKE_SOURCE_DIR}/fonts" DESTINATION "${CMAKE_BINARY_DIR}")