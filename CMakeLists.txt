cmake_minimum_required(VERSION 3.16)
project(OpenGLPractice)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(opengl_practice)

add_subdirectory(src)
add_subdirectory(third_party)
add_subdirectory(glsl)
add_subdirectory(assets)
add_subdirectory(include)