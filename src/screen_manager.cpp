#include "screen_manager.h"

#include "utils.h"
#include "camera.h"
#include "constants.h"

#include "glfw.h"
#include "glad/gl.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

ScreenManager::ScreenManager(const std::shared_ptr<Camera>& camera) : camera(camera)
{
	if (!camera) {
		throw std::runtime_error("Failed to construct ScreenManager");
	}
	glfwInit();
	if (!(window = initWindow())) {
		throw std::runtime_error("Failed to construct ScreenManager");
	}
	initInput();
	if (!initOpenGL()) {
		throw std::runtime_error("Failed to construct ScreenManager");
	}
}

ScreenManager::ScreenManager(ScreenManager&& other) noexcept :
	window(other.window),
	camera{std::move(other.camera)}
{
	other.glfw_deleter.removeDeleter();
}

GLFWwindow* const ScreenManager::getWindow()
{
	return window;
}

void ScreenManager::processInput(const float delta_time)
{
	processMouseInput(delta_time);
	processGamepadInput(delta_time);
}

void ScreenManager::endFrame()
{
	glfwSwapBuffers(window);
	glfwPollEvents();
}

double ScreenManager::getTime()
{
	return glfwGetTime();
}

bool ScreenManager::shouldClose()
{
	return glfwWindowShouldClose(window);
}

GLFWwindow* const ScreenManager::initWindow()
{
	//opengl version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	GLFWwindow* const window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		LOG("Failed to create GLFW window")
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	return window;
}

void ScreenManager::initInput() {
	if (use_gamepad) {
		// TODO use init function and joystick callbacks to manage joysticks
	}
	if (use_mouse) {
		// callback user data
		glfwSetWindowUserPointer(window, reinterpret_cast<void*>(camera.get()));
		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetScrollCallback(window, scrollCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
}

bool ScreenManager::initOpenGL() {
	if (gladLoadGL(glfwGetProcAddress) == 0)
	{
		LOG("Failed to initialize GLAD")
		return false;
	}

	// depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// culling
	glEnable(GL_CULL_FACE);
	// gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);
	// offset depth calculation for shadow map
	glEnable(GL_POLYGON_OFFSET_FILL);
	// default color when clearing a frame
	glClearColor(CLEAR_COLOR[0], CLEAR_COLOR[1], CLEAR_COLOR[2], CLEAR_COLOR[3]);

	return true;
}

void ScreenManager::processMouseInput(const float delta_time)
{
	if (!use_mouse) return;
	static bool is_sprinting = false;
	bool moved = false;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		is_sprinting = true;
	}
	const float velocity = delta_time * (is_sprinting ? max_speed : base_speed);

	if ((glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)) {
		camera->processMovement(Camera::Movement::Forward, velocity);
		moved = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)) {
		camera->processMovement(Camera::Movement::Backward, velocity);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera->processMovement(Camera::Movement::Left, velocity);
		moved = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)) {
		camera->processMovement(Camera::Movement::Right, velocity);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera->processMovement(Camera::Movement::Up, velocity);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		camera->processMovement(Camera::Movement::Down, velocity);
		moved = true;
	}

	is_sprinting &= moved;
}

void ScreenManager::processGamepadInput(const float delta_time)
{
	if (!use_gamepad) return;
	static bool is_sprinting = false;

	// TODO setup callback for joysticks so we don't have to check every frame
	// callback user pointers require static variables which I don't want to use at the moment
	std::vector<int> joystick_ids;
	for (unsigned int i=GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
		glfwSetJoystickUserPointer(i, reinterpret_cast<void*>(&joystick_ids));
		if (glfwJoystickIsGamepad(i)) {
			joystick_ids.push_back(i);
		}
	}

	GLFWgamepadstate state;
	for (unsigned int i=0; i < static_cast<unsigned int>(joystick_ids.size()); i++) {
		if (!glfwGetGamepadState(i, &state)) continue;

		is_sprinting |= state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB];
		bool moved = false;
		const float velocity = delta_time * (is_sprinting ? max_speed : base_speed);
		float right_x_offset = 0;
		float right_y_offset = 0;

		if (state.buttons[GLFW_GAMEPAD_BUTTON_START]) {
			glfwSetWindowShouldClose(window, true);
		}
		if (std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]) > min_dead_zone) {
			camera->processMovement(Camera::Movement::Forward, velocity * -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
			moved = true;
		}
		if (std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]) > min_dead_zone) {
			camera->processMovement(Camera::Movement::Right, velocity * state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
			moved = true;
		}
		if (state.buttons[GLFW_GAMEPAD_BUTTON_A]) {
			camera->processMovement(Camera::Movement::Up, velocity);
			moved = true;
		}
		if (state.buttons[GLFW_GAMEPAD_BUTTON_B]) {
			camera->processMovement(Camera::Movement::Down, velocity);
			moved = true;
		}
		if (abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]) > min_dead_zone) {
			right_x_offset = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] * joystick_sensitivity;
		}
		if (abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]) > min_dead_zone) {
			right_y_offset = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] * joystick_sensitivity;
		}

		camera->processJoystickRotation(right_x_offset, right_y_offset);
		is_sprinting &= moved;
	}
}

void ScreenManager::imguiStartFrame(LightBlock& light_block, bool* p_open)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(SCR_WIDTH, SCR_HEIGHT), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Dear ImGui", p_open, 0))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    ImGui::End();
}

void ScreenManager::imguiEndFrame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void ScreenManager::imguiInit(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);
}

void ScreenManager::imguiShutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void ScreenManager::framebufferSizeCallback(GLFWwindow* window, const int width, const int height)
{
	// make sure the viewport matches the new window dimensions
	// width and height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void ScreenManager::cursorPosCallback(GLFWwindow* const window, const double xpos, const double ypos)
{
	Camera* const camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window));
	if (!camera) {
		LOG("Failed to access camera in cursor position callback")
	}

	static double lastX = xpos;
	static double lastY = ypos;

	const float xoffset = xpos - lastX;
	// y moves from bottom to top
	const float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera->processMouseRotation(xoffset, yoffset);
}

void ScreenManager::scrollCallback(GLFWwindow* const window, const double xoffset, const double yoffset)
{
	if (Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window))) {
		camera->processZoom(yoffset);
	} else {
		LOG("Failed to access camera in scroll callback")
	}
}