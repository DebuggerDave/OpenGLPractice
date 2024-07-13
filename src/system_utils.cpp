#include "system_utils.h"

#include "shader.h"
#include "model.h"
#include "light_block.h"
#include "camera.h"
#include "light_uniform_buffer.h"
#include "world.h"
#include "constants.h"
#include "utils.h"

#include "GLFW/glfw3.h"
#include "glm/mat4x4.hpp"
#include "glm/mat3x3.hpp"
#include "glm/vec4.hpp"
#include "glm/vec3.hpp"
#include "glm/vector_relational.hpp"
#include "glm/geometric.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "stb_image.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <string>
#include <vector>
#include <unordered_set>

// TODO find a better place for this
static std::unordered_set<int> joystick_ids = {};

GLFWwindow* init(Camera& camera)
{
	glfwInit();

	//opengl version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// window
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		LOG("Failed to create GLFW window")
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	// input
	if (USE_GAMEPAD) {
		glfwSetJoystickCallback(joystickCallback);
		initJoysticks();
	}
	if (USE_MOUSE) {
		// callback user data
		glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&camera));
		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetScrollCallback(window, scrollCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	// load OpenGL functions
	if (gladLoadGL(glfwGetProcAddress) == 0)
	{
		LOG("Failed to initialize GLAD")
		return nullptr;
	}

	// ---- opengl settings ----
	// depth testing
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// culling
	glEnable(GL_CULL_FACE);
	// gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);
	// offset depth calculation for shadow map
	glEnable(GL_POLYGON_OFFSET_FILL);

	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* const window, const float delta_time, Camera& camera)
{
	processMouseInput(window, delta_time, camera);
	processGamepadInput(window, delta_time, camera);
}

void processMouseInput(GLFWwindow* const window, const float delta_time, Camera& camera)
{
	if (!USE_MOUSE) return;
	static bool is_sprinting = false;
	bool moved = false;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
		is_sprinting = true;
	}
	const float velocity = delta_time * (is_sprinting ? MAX_SPEED : BASE_SPEED);

	if ((glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)) {
		camera.processMovement(Camera::Movement::Forward, velocity);
		moved = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)) {
		camera.processMovement(Camera::Movement::Backward, velocity);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		camera.processMovement(Camera::Movement::Left, velocity);
		moved = true;
	}
	if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)) {
		camera.processMovement(Camera::Movement::Right, velocity);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		camera.processMovement(Camera::Movement::Up, velocity);
		moved = true;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		camera.processMovement(Camera::Movement::Down, velocity);
		moved = true;
	}

	is_sprinting &= moved;
}

void processGamepadInput(GLFWwindow* const window, const float delta_time, Camera& camera)
{
	if (!USE_GAMEPAD) return;
	static bool is_sprinting = false;

	GLFWgamepadstate state;
	for (unsigned int i=0; i < (unsigned int)joystick_ids.size(); i++) {
		if (!glfwGetGamepadState(i, &state)) continue;

		is_sprinting |= state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB];
		bool moved = false;
		const float velocity = delta_time * (is_sprinting ? MAX_SPEED : BASE_SPEED);
		float right_x_offset = 0;
		float right_y_offset = 0;

		if (state.buttons[GLFW_GAMEPAD_BUTTON_START]) {
			glfwSetWindowShouldClose(window, true);
		}
		if (std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]) > MIN_DEAD_ZONE) {
			camera.processMovement(Camera::Movement::Forward, velocity * -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
			moved = true;
		}
		if (std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]) > MIN_DEAD_ZONE) {
			camera.processMovement(Camera::Movement::Right, velocity * state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
			moved = true;
		}
		if (state.buttons[GLFW_GAMEPAD_BUTTON_A]) {
			camera.processMovement(Camera::Movement::Up, velocity);
			moved = true;
		}
		if (state.buttons[GLFW_GAMEPAD_BUTTON_B]) {
			camera.processMovement(Camera::Movement::Down, velocity);
			moved = true;
		}
		if (abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]) > MIN_DEAD_ZONE) {
			right_x_offset = state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X] * JOYSTICK_SENSITIVITY;
		}
		if (abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]) > MIN_DEAD_ZONE) {
			right_y_offset = -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y] * JOYSTICK_SENSITIVITY;
		}

		camera.processJoystickRotation(right_x_offset, right_y_offset);
		is_sprinting &= moved;
	}
}

void framebufferSizeCallback(GLFWwindow* window, const int width, const int height)
{
	// make sure the viewport matches the new window dimensions
	// width and height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void cursorPosCallback(GLFWwindow* const window, const double xpos, const double ypos)
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

void scrollCallback(GLFWwindow* const window, const double xoffset, const double yoffset)
{
	if (Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window))) {
		camera->processZoom(yoffset);
	} else {
		LOG("Failed to access camera in scroll callback")
	}
}

void joystickCallback(const int jid, const int event)
{
	if ((event == GLFW_CONNECTED) && glfwJoystickIsGamepad(jid)) {
		joystick_ids.emplace(jid);
	} else {
		joystick_ids.erase(jid);
	}
}

void initJoysticks()
{
	joystick_ids.clear();
	for (int i=GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_LAST; i++) {
		if (glfwJoystickIsGamepad(i)) {
			joystick_ids.emplace();
		}
	}
}

unsigned int loadCubemap(const std::vector<std::string>& faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
	
	int width, height, num_components;
	stbi_set_flip_vertically_on_load(false);
	for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &num_components, 0);
        if (data)
        {
			GLenum format = GL_RGBA;
			if (num_components == 1)
				format = GL_RED;
			else if (num_components == 3)
				format = GL_RGB;
			else if (num_components == 4)
				format = GL_RGBA;

			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const std::vector<Model>& models, const World& world)
{
	shader.activate();
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	shader.setMat3("light_normal_mat", glm::transpose(glm::inverse(glm::mat3(view))));

	for (const auto& model : models) {
		model.draw(shader, world.numObjects(model.id));
	}
}

void imguiStartFrame(LightBlock& light_block, bool* p_open)
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

void imguiEndFrame()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void imguiInit(GLFWwindow* window)
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);
}

void imguiShutdown() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void shutdown()
{
	glfwTerminate();
}

glm::mat4 lightModelMatrix(const DirectionalLight& light, const glm::vec3& camera_pos) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, camera_pos + (glm::vec3(-light.dir) * FAR_PLANE));
	model = glm::scale(model, glm::vec3(10.0f));
	return model;
}

template <typename T>
glm::mat4 lightModelMatrix(const T& light, [[maybe_unused]] const glm::vec3& camera_pos) {
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(light.pos));
	model = glm::scale(model, glm::vec3(10.0f));
	return model;
}
template
glm::mat4 lightModelMatrix(const SpotLight& light, const glm::vec3& camera_pos);
template
glm::mat4 lightModelMatrix(const PointLight& light, const glm::vec3& camera_pos);

template <typename T>
void drawLight(const Model& model, const Shader& shader, const std::vector<T>& lights, const glm::vec3& camera_pos)
{
	using LightSizeType = std::remove_cvref_t<decltype(lights)>::size_type;
	for (LightSizeType i=0; i < lights.size(); i++) {
		constexpr glm::vec4 zero(0.0f);
		const T cur_light = lights[i];
		if ((glm::all(glm::equal(cur_light.color.ambient, zero))) &&
			(glm::all(glm::equal(cur_light.color.diffuse, zero))) &&

			(glm::all(glm::equal(cur_light.color.specular, zero)))) {
			return;
		}
		shader.setVec4("light_color", cur_light.color.diffuse);
		const glm::mat4 model_mat = lightModelMatrix(cur_light, camera_pos);
			
		shader.setMat4("model", std::move(model_mat));
		model.draw(shader);
	}
}
template
void drawLight(const Model& model, const Shader& shader, const std::vector<DirectionalLight>& lights, const glm::vec3& camera_pos);
template
void drawLight(const Model& model, const Shader& shader, const std::vector<SpotLight>& lights, const glm::vec3& camera_pos);
template
void drawLight(const Model& model, const Shader& shader, const std::vector<PointLight>& lights, const glm::vec3& camera_pos);