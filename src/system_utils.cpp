#include "system_utils.h"

#include "pch.h"
#include "constants.h"
#include "utils.h"
#include "camera.h"
#include "shader.h"
#include "component.h"
#include "world.h"
#include "model.h"
#include "light_block.h"

static std::unordered_set<int> joystick_ids = {};

GLFWwindow* init(Camera& camera)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		LOG("Failed to create GLFW window")
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetJoystickCallback(joystickCallback);
	if (USE_GAMEPAD)
		findJoysticks();
	if (USE_MOUSE) {
		glfwSetWindowUserPointer(window, reinterpret_cast<void*>(&camera));
		glfwSetCursorPosCallback(window, cursorPosCallback);
		glfwSetScrollCallback(window, scrollCallback);
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	// load all OpenGL function pointers
	if (gladLoadGL(glfwGetProcAddress) == 0)
	{
		LOG("Failed to initialize GLAD")
		return nullptr;
	}

	// Make sure fragment scene depth is calculated properly
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	// culling
	glEnable(GL_CULL_FACE);
	// gamma correction
	glEnable(GL_FRAMEBUFFER_SRGB);
	// offset depth calculation for shadow map
	glEnable(GL_POLYGON_OFFSET_FILL);

	// imgui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(nullptr);

	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window, float delta_time, Camera& camera)
{
	processMouseInput(window, delta_time, camera);
	processGamepadInput(window, delta_time, camera);
}

void processMouseInput(GLFWwindow *window, float delta_time, Camera& camera)
{
	if (USE_MOUSE) {
		float velocity = BASE_SPEED * delta_time;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
			velocity = MAX_SPEED * delta_time;
		if ((glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS))
			camera.processMovement(Camera::Movement::Forward, velocity);
		if ((glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS))
			camera.processMovement(Camera::Movement::Backward, velocity);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera.processMovement(Camera::Movement::Left, velocity);
		if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))
			camera.processMovement(Camera::Movement::Right, velocity);
		if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
			camera.processMovement(Camera::Movement::Up, velocity);
		if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
			camera.processMovement(Camera::Movement::Down, velocity);
	}
}

void processGamepadInput(GLFWwindow *window, float delta_time, Camera& camera)
{
	static bool is_sprinting = false;

	if (USE_GAMEPAD) {
		GLFWgamepadstate state;
		using joystickIdsSizeType = decltype(joystick_ids)::size_type;
		for (joystickIdsSizeType i=0; i<joystick_ids.size(); i++) {
			static const float min_dead_zone = 0.1f;
			bool moved = false;
			float velocity = BASE_SPEED * delta_time;
			if (glfwGetGamepadState(i, &state)) {
				is_sprinting |= state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_THUMB];
				if (is_sprinting)
					velocity = MAX_SPEED * delta_time;

				if (state.buttons[GLFW_GAMEPAD_BUTTON_START])
					glfwSetWindowShouldClose(window, true);
				if (std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]) > min_dead_zone) {
					camera.processMovement(Camera::Movement::Forward, velocity * -state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
					moved = true;
				}
				if (std::abs(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]) > min_dead_zone) {
					camera.processMovement(Camera::Movement::Right, velocity * state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
					moved = true;
				}
				if (state.buttons[GLFW_GAMEPAD_BUTTON_A])
					camera.processMovement(Camera::Movement::Up, velocity);
				if (state.buttons[GLFW_GAMEPAD_BUTTON_B])
					camera.processMovement(Camera::Movement::Down, velocity);
				float right_x_offset = (abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X]) < min_dead_zone) ? 0 : state.axes[GLFW_GAMEPAD_AXIS_RIGHT_X];
				float right_y_offset = (abs(state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y]) < min_dead_zone) ? 0 : -state.axes[GLFW_GAMEPAD_AXIS_RIGHT_Y];
				camera.processJoystickRotation(right_x_offset, right_y_offset);

				is_sprinting &= moved;
			}
		}
	}
}

void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions
	// width and height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

void cursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window))) {
		static double lastX = xpos;
		static double lastY = ypos;

		float xoffset = xpos - lastX;
		// y moves from bottom to top
		float yoffset = lastY - ypos;

		lastX = xpos;
		lastY = ypos;

		camera->processMouseRotation(xoffset, yoffset);
	} else {
		LOG("Failed to access camera in cursor position callback")
	}
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (Camera* camera = reinterpret_cast<Camera*>(glfwGetWindowUserPointer(window))) {
		camera->processZoom(yoffset);
	} else {
		LOG("Failed to access camera in scroll callback")
	}
}

void joystickCallback(int jid, int event)
{
	if ((event = GLFW_CONNECTED) && glfwJoystickIsGamepad(jid)) {
		joystick_ids.emplace(jid);
	} else {
		joystick_ids.erase(jid);
	}
}

void findJoysticks()
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

void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const Model& grass, const Model& dirt, const World& world)
{
	shader.activate();
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	shader.setMat3("light_normal_mat", glm::transpose(glm::inverse(glm::mat3(view))));

	auto entity_view = world.getRegistry().view<ALLCOMPONENTS>();
	for (World::Entity entity: entity_view) {
		const Position& position = entity_view.get<Position>(entity);
		const BlockId& block_id = entity_view.get<BlockId>(entity);
		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::translate(model, position.data);
		glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(view * model)));

		shader.setMat4("model", model);
		shader.setMat3("normal_mat", normal_mat);
		if (block_id == BlockId::Grass) {
			grass.draw(shader);
		}
		else if (block_id == BlockId::Dirt) {
			dirt.draw(shader);
		}
	}
}

void showImgui(LightBlock& light_block, bool* p_open)
{

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 10, main_viewport->WorkPos.y + 10), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(550, 100), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("Dear ImGui", p_open, 0))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

	static glm::vec4 static_dir(0.0f, -1.0, 0.0, 0.0);
	ImGui::SliderFloat3("shadow caster direction", glm::value_ptr(static_dir), -1.0f, 1.0f);
	light_block.updateDirection(LightBlock::LightType::Directional, 0, glm::normalize(static_dir));

    ImGui::End();
}
