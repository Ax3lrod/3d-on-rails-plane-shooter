cmake_minimum_required(VERSION 3.10)
project(OpenGLProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 17)

# ---------------------------------------------------------
# GLFW (downloads automatically)
# ---------------------------------------------------------
include(FetchContent)
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG latest
)
FetchContent_MakeAvailable(glfw)

# ---------------------------------------------------------
# GLAD
# ---------------------------------------------------------
add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)

# ---------------------------------------------------------
# Build main executable
# ---------------------------------------------------------
file(GLOB SOURCES "src/*.cpp")
add_executable(OpenGLProject ${SOURCES})

target_include_directories(OpenGLProject PUBLIC include)

# Link dependencies
target_link_libraries(OpenGLProject PRIVATE glfw glad)

# Windows needs extra libs
if (WIN32)
    target_link_libraries(OpenGLProject PRIVATE opengl32)
endif()

# macOS framework linking
if (APPLE)
    target_link_libraries(OpenGLProject PRIVATE "-framework OpenGL")
endif()
