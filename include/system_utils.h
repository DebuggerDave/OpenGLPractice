#ifndef SYSTEM_UTILITY_H
#define SYSTEM_UTILITY_H

class Shader;
class Model;
class LightBlock;
class Camera;
struct DirectionalLight;
class World;

struct GLFWwindow;
#include "glm/fwd.hpp"

#include <iosfwd>
#include <vector>

GLFWwindow* init(Camera& camera);
void processInput(GLFWwindow* const window, const float delta_time, Camera& camera);
void processMouseInput(GLFWwindow* const window, const float delta_time, Camera& camera);
void processGamepadInput(GLFWwindow* const window, const float delta_time, Camera& camera);
void framebufferSizeCallback(GLFWwindow* window, const int width, const int height);
void cursorPosCallback(GLFWwindow* const window, const double xpos, const double ypos);
void scrollCallback(GLFWwindow* const window, const double xoffset, const double yoffset);
void joystickCallback(const int jid, const int event);
void initJoysticks();
unsigned int loadCubemap(const std::vector<std::string>& faces);
void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const std::vector<Model>& models, const World& world);
void imguiStartFrame(LightBlock& light_block, bool* p_open = NULL);
void imguiEndFrame();
void imguiInit(GLFWwindow* window);
void imguiShutdown();
void shutdown();
// Helper function for drawLight
glm::mat4 lightModelMatrix(const DirectionalLight& light, const glm::vec3& camera_pos);
// Helper function for drawLight
template <typename T>
glm::mat4 lightModelMatrix(const T& light, const glm::vec3& camera_pos);
template <typename T>
void drawLight(const Model& model, const Shader& shader, const std::vector<T>& lights, const glm::vec3& camera_pos);

#endif