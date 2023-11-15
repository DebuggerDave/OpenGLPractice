#glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(glfw)
target_link_libraries(opengl_practice glfw)

#glad
target_sources(opengl_practice PUBLIC
                "${CMAKE_CURRENT_SOURCE_DIR}/glad/src/glad.c"
                )
target_include_directories(opengl_practice PUBLIC
                            "${CMAKE_CURRENT_SOURCE_DIR}/glad/include"
                            )

#glm
add_subdirectory(glm)
target_link_libraries(opengl_practice glm::glm)

#std
target_include_directories(opengl_practice PUBLIC
                            ${CMAKE_CURRENT_SOURCE_DIR}/stb)