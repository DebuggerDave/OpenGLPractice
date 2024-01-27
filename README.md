# OpenGLPractice
Learning OpenGL and some other stuff

### Notes
* Asperite executable is optional, it enables generation of sprites from the .aseprite files
* Ninja is also optional, you can use make
* This project is mainly tested and developed on Windows

### How to Build
* `git clone PROJECT_GIT_URL`
* `cd OpenGLPractice`
* `git submodule update --init --recursive`
* `mkdir -p build`
* `cd build`
* `ASEPRITE=PATH_TP_ASEPRITE_EXECUTABLE CC=PATH_TO_C_COMPILER CXX=PATH_TO_CPP_COMPILER cmake -S .. -D CMAKE_BULID_TYPE=Release -G "Ninja" ..`  
  OR JUST  
  `cmake ..`
* `cmake --build .`
* `./opengl_practice.exe`
