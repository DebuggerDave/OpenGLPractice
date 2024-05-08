# OpenGLPractice
Learning OpenGL and some other stuff

### Notes
* Asperite executable is optional, it enables generation of sprites from the .aseprite files

### Controls
* WASD and mouse - move and look
* joysticks - move and look
* escape - quit
* start - quit

### How to Build
* `git clone PROJECT_GIT_URL`
* `cd OpenGLPractice`
* `git submodule update --init --recursive`
* `mkdir -p build`
* `cd build`
* `ASEPRITE=PATH_TP_ASEPRITE_EXECUTABLE cmake -S ..`
* `cmake --build .`
* `./opengl_practice.exe`

![opengl](https://github.com/DebuggerDave/OpenGLPractice/assets/33384202/d0011744-9f23-4b7c-867a-e7431f051615)
