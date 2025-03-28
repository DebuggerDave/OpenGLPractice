cmake_minimum_required(VERSION 3.16)
project(OpenGLPractice)
set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(
       -Wall -Werror
       $<$<CONFIG:RELEASE>:-Ofast>
       $<$<CONFIG:DEBUG>:-O0>
       $<$<CONFIG:DEBUG>:-ggdb3>
)

if (WIN32)
       add_compile_options(
              # assimp fix, UCRT isn't available in msys2/mingw64 for some reason, use a version before that
              -D__MSVCRT_VERSION__=0xA00
              # assimp has pragmas warnings, but we don't know what they are?
              -Wno-error=unknown-pragmas
       )
endif()

add_executable(opengl_practice)

add_subdirectory(src)
add_subdirectory(third_party)
add_subdirectory(glsl)
add_subdirectory(assets)
add_subdirectory(include)