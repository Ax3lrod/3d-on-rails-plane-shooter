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
# GLM (Mathematics library)
# ---------------------------------------------------------
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glm)

# ---------------------------------------------------------
# GLAD
# ---------------------------------------------------------
add_library(glad src/glad.c)
target_include_directories(glad PUBLIC include)

set(SOURCES
    src/main.cpp
    src/core/Engine.cpp
    src/core/Camera.cpp
    src/core/Input.cpp
    src/renderer/Mesh.cpp
    src/renderer/shaderClass.cpp
    src/renderer/VAO.cpp
    src/renderer/VBO.cpp
    src/renderer/EBO.cpp
    src/entities/PlayerStarfighter.cpp
    src/entities/EnemyManager.cpp
    src/entities/BossDreadnought.cpp
    src/combat/CombatSystem.cpp
    src/world/WorldEnvironment.cpp
    src/ui/HUD.cpp
    src/audio/SoundManager.cpp
)
add_executable(OpenGLProject ${SOURCES})

target_include_directories(OpenGLProject PUBLIC
    include
    headers
    headers/core
    headers/renderer
    headers/entities
    headers/world
    headers/combat
    headers/ui
    headers/audio
)

# Link dependencies
target_link_libraries(OpenGLProject PRIVATE glfw glad glm::glm)

# Windows needs extra libs
if (WIN32)
    target_link_libraries(OpenGLProject PRIVATE opengl32 winmm)
endif()

# macOS framework linking
if (APPLE)
    target_link_libraries(OpenGLProject PRIVATE "-framework OpenGL")
endif()
