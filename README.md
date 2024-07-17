# OpenGLPractice
Learning OpenGL and some other stuff

### Notes
* Asperite executable is optional, it enables generation of sprites from the .aseprite files

### Controls
* WASD and mouse - move and look
* joysticks - move and look
* escape - quit
* start - quit

### Requirements
* C++ 20
* Cmake 3.16

### How to Build
* `git clone PROJECT_GIT_URL`
* `cd OpenGLPractice`
* `git submodule update --init --recursive`
* `mkdir -p build`
* `cd build`
* Linux only, install glfw dependencies
  * `sudo apt install libwayland-dev libxkbcommon-dev xorg-dev`
* `ASEPRITE=PATH_TP_ASEPRITE_EXECUTABLE cmake -S ..`
* `cmake --build .`
* `./opengl_practice.exe`

![image](https://github.com/user-attachments/assets/43d6a7fe-c71e-4634-89d5-5c5b7c2f4e13)
