cmake_minimum_required(VERSION 3.26)
project(OpenGLPractice)

add_executable(opengl_practice)

set_property(TARGET opengl_practice PROPERTY CXX_STANDARD 23)

add_subdirectory(src)
add_subdirectory(third_party)
add_subdirectory(glsl)
add_subdirectory(assets)

target_include_directories(opengl_practice PRIVATE "${PROJECT_SOURCE_DIR}/include")