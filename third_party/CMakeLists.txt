# no dev warnings for third party libraries
set(no_dev_warnings_backup "$CACHE{CMAKE_SUPPRESS_DEVELOPER_WARNINGS}")
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ON CACHE INTERNAL "" FORCE)

# static libs
option(BUILD_SHARED_LIBS OFF)

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
target_sources(opengl_practice PRIVATE ./glad/src/gl.c)
target_include_directories(opengl_practice PRIVATE ./glad/include)

# glm
add_subdirectory(glm)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
target_link_libraries(opengl_practice PRIVATE glm::glm)

# stb
target_include_directories(opengl_practice PRIVATE ./stb)

# assimp
set(ASSIMP_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_OBJ_IMPORTER ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INJECT_DEBUG_POSTFIX OFF CACHE BOOL "" FORCE)
set(ASSIMP_IGNORE_GIT_HASH ON CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL_PDB OFF CACHE BOOL "" FORCE)
set(USE_STATIC_CRT ON CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_VIEW OFF CACHE BOOL "" FORCE)
add_subdirectory(assimp)
target_link_libraries(opengl_practice PRIVATE assimp)

# imgui
### fixes for pre-compiled headers, imgui needs it's internal math operators
##target_compile_definitions(opengl_practice PRIVATE "-DIMGUI_DEFINE_MATH_OPERATORS")
### fixes for pre-compiled headers, ignore opengl loader code from imgui, pch has glad2 instead
##target_compile_definitions(opengl_practice PRIVATE "-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM")
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

#entt
add_subdirectory(entt)
target_link_libraries(opengl_practice PRIVATE EnTT)

#perlin noise
target_include_directories(opengl_practice PRIVATE ./PerlinNoise)

#cereal
target_include_directories(opengl_practice PRIVATE ./cereal/include)

# reset dev warning variable after building third party libraries
set(CMAKE_SUPPRESS_DEVELOPER_WARNINGS ${no_dev_warnings_backup} CACHE INTERNAL "" FORCE)