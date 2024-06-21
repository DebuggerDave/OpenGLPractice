#ifndef OPENGLPRACTICE_PCH_H
#define OPENGLPRACTICE_PCH_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <utility>
#include <cstdio>
#include <random>
#include <array>
#include <type_traits>

#include "glad/gl.h"

#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "assimp/Importer.hpp"
#include "assimp/scene.h"
#include "assimp/postprocess.h"

#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "stb_image.h"

#include "entt/entity/registry.hpp"
#include "entt/entity/snapshot.hpp"

#include "PerlinNoise.hpp"

#include "cereal/archives/binary.hpp"

#endif