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
target_sources(opengl_practice PRIVATE
                "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c"
                )
target_include_directories(opengl_practice PRIVATE
                            "${CMAKE_CURRENT_SOURCE_DIR}/glad/include"
                            )

# glm
add_subdirectory(glm)
set(GLM_TEST_ENABLE OFF CACHE BOOL "" FORCE)
target_link_libraries(opengl_practice PRIVATE glm::glm)

# stb
target_include_directories(opengl_practice PRIVATE
                            ${CMAKE_CURRENT_SOURCE_DIR}/stb)

# nlohmann_json
set(JSON_BuildTests OFF CACHE INTERNAL "")
add_subdirectory(json)
target_link_libraries(opengl_practice PRIVATE nlohmann_json::nlohmann_json)

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

# aseprite
add_subdirectory(aseprite)