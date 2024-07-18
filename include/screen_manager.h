#ifndef GLFW_MANAGER_H
#define GLFW_MANAGER_H

class GLFWwindow;
class Camera;
class LightBlock;
#include "utils.h"

#include "glfw.h"

#include <memory>
#include <vector>

class ScreenManager
{
public:
	ScreenManager(const std::shared_ptr<Camera>& camera);
	~ScreenManager() = default;
	ScreenManager(const ScreenManager& other) = delete;
	ScreenManager(ScreenManager&& other) noexcept;
	ScreenManager& operator=(const ScreenManager& other) = delete;
	ScreenManager& operator=(ScreenManager&& other) = delete;

	// getters
	GLFWwindow* const getWindow();
	// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
	void processInput(const float delta_time);
	// buffer swap and input poll
	void endFrame();
	double getTime();
	// return true if the program is ready to close
	bool shouldClose();

	static constexpr bool use_mouse = false;
	static constexpr bool use_gamepad = true;
	static constexpr float base_speed = 2.5f;
	static constexpr float max_speed = base_speed * 10.0f;
	static constexpr float min_dead_zone = 0.1f;
	static constexpr float joystick_sensitivity = 3.0f;
private:
	// initalization
	GLFWwindow* const initWindow();
	void initInput();
	bool initOpenGL();
	// input
	void processMouseInput(const float delta_time);
	void processGamepadInput(const float delta_time);
	// imgui management
	void imguiStartFrame(LightBlock& light_block, bool* p_open = NULL);
	void imguiEndFrame();
	void imguiInit(GLFWwindow* window);
	void imguiShutdown();
	// callbacks
	static void framebufferSizeCallback(GLFWwindow* window, const int width, const int height);
	static void cursorPosCallback(GLFWwindow* const window, const double xpos, const double ypos);
	static void scrollCallback(GLFWwindow* const window, const double xoffset, const double yoffset);

	// window is deleted by glfwTerminate
	GLFWwindow* window;
	// call glfwTerminate even if we fail to construct
	utils::ScopedDeleter glfw_deleter{&glfwTerminate};
	// TODO add input_manager so screen_manager isn't dependent on camera
	std::shared_ptr<Camera> camera;
};

#endif