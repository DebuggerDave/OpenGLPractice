target_include_directories(opengl_practice PRIVATE ./)

# pre-compiled header
#target_precompile_headers(opengl_practice PRIVATE "$<$<COMPILE_LANGUAGE:CXX>:pch.h>")