cmake_minimum_required(VERSION 3.27.7)
project(OpenGLPractice)

add_executable(opengl_practice)

add_subdirectory(src)

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)
target_link_libraries(opengl_practice glfw)

find_package(OpenGL Required) 

target_include_directories(opengl_practice PUBLIC
							"${PROJECT_SOURCE_DIR}/include"
							"${PROJECT_SOURCE_DIR}/"
							)
							
target_compile_definitions(opengl_practice PRIVATE SRC_DIR="${PROJECT_BINARY_DIR}")