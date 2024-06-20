#ifndef SYSTEM_UTILITY_H
#define SYSTEM_UTILITY_H

#include "glm/fwd.hpp"

#include <iosfwd>
#include <cstddef>

// forward declarations
struct GLFWwindow;
class Shader;
class Model;
class World;
class LightBlock;
class Camera;
namespace std {
    template <typename T, typename Allocator>
    class vector;
}

GLFWwindow* init(Camera& camera);
void processInput(GLFWwindow* window, float delta_time, Camera& camera);
void processMouseInput(GLFWwindow* window, float delta_time, Camera& camera);
void processGamepadInput(GLFWwindow* window, float delta_time, Camera& camera);
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void joystickCallback(int jid, int event);
void findJoysticks();
unsigned int loadCubemap(const std::vector<std::string>& faces);
void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const Model& grass, const Model& dirt, const World& world);
void showImgui(LightBlock& light_block, bool* p_open = NULL);

#endif