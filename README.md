# OpenGLPractice
Learning OpenGL and a little CMake I guess

### Notes
* Asperite executable is optional, it enables generation of sprites from the .aseprite files
* The Build directions require ninja-build to be installed, but Make can probably be used instead after the `-G` option
* This project is mainly tested and developed on Windows

### How to Build
* `git clone PROJECT_GIT_URL`
* `cd PROJECT_DIRECTORY`
* `git submodule update --init --recursive`
* `mkdir -p build`
* `cd build`
* `ASEPRITE=PATH_TP_ASEPRITE_EXECUTABLE CC=PATH_TO_C_COMPILER CXX=PATH_TO_CPP_COMPILER cmake -G "Ninja" ..`
* `ASEPRITE=PATH_TP_ASEPRITE_EXECUTABLE cmake --build .`
* `./opengl_practice.exe`
