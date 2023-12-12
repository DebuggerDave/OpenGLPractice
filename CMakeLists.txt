cmake_minimum_required(VERSION 3.26)
project(OpenGLPractice)

add_executable(opengl_practice)

add_subdirectory(src)
add_subdirectory(third_party)
add_subdirectory(img)
add_subdirectory(glsl)
add_subdirectory(3d)

set(CMAKE_BUILD_TYPE "DEBUG")

target_include_directories(opengl_practice PUBLIC
							"${PROJECT_SOURCE_DIR}/include"
							)
							
target_compile_definitions(opengl_practice PRIVATE "BINARY_DIR=${CMAKE_CURRENT_BINARY_DIR}")