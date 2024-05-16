# no dev warnings for third party libraries
set(no_dev_warnings_backup "$CACHE{CMAKE_SUPPRESS_DEVELOPER_WARNINGS}")
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)

# libstd
target_link_libraries(opengl_practice PRIVATE -static-libgcc -static-libstdc++)

# OpenGL
find_package(OpenGL REQUIRED)
target_link_libraries(opengl_practice PRIVATE OpenGL::GL)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
target_link_libraries(opengl_practice PRIVATE glfw)

# glad
target_sources(opengl_practice PRIVATE ./glad/src/glad.c)
target_include_directories(opengl_practice PRIVATE ./glad/include)

# glm
add_subdirectory(glm)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
target_link_libraries(opengl_practice PRIVATE glm::glm)

# stb
target_include_directories(opengl_practice PRIVATE ./stb)

# assimp
set(BUILD_SHARED_LIBS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_INSTALL OFF)
set(ASSIMP_NO_EXPORT ON)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF)
set(ASSIMP_INSTALL_PDB OFF)
set(USE_STATIC_CRT ON)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF)
set(ASSIMP_IGNORE_GIT_HASH ON)
add_subdirectory(assimp)
target_link_libraries(opengl_practice PRIVATE assimp)

# imgui
# global files
target_sources(opengl_practice PRIVATE
                ./imgui/imgui_demo.cpp
                ./imgui/imgui_draw.cpp
                ./imgui/imgui_tables.cpp
                ./imgui/imgui_widgets.cpp
                ./imgui/imgui.cpp
                )
target_include_directories(opengl_practice PRIVATE ./imgui)
# graphics api specific
target_sources(opengl_practice PRIVATE
                ./imgui/backends/imgui_impl_opengl3.cpp
                ./imgui/backends/imgui_impl_glfw.cpp
                )
target_include_directories(opengl_practice PRIVATE ./imgui/backends)
# std specific
target_sources(opengl_practice PRIVATE ./imgui/misc/cpp/imgui_stdlib.cpp)
target_include_directories(opengl_practice PRIVATE ./imgui/misc/cpp)

# reset dev warning variable after building third party libraries
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ${no_dev_warnings_backup} CACHE INTERNAL "" FORCE)