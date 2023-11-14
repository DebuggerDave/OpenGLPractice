cmake_minimum_required(VERSION 3.13)
project(OpenGLPractice)

add_executable(opengl_practice)

add_subdirectory(src)
add_subdirectory(third_party)

set(CMAKE_BUILD_TYPE "DEBUG")

target_include_directories(opengl_practice PUBLIC
							"${PROJECT_SOURCE_DIR}/include"
							)
							
target_compile_definitions(opengl_practice PRIVATE "SRC_DIR=${PROJECT_SOURCE_DIR}")