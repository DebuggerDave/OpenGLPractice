#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext/vector_relational.hpp>

#include <iostream>
#include <fstream>
#include <limits>
#include <numbers>
#include <chrono>
#include <vector>
#include <string>

#include "stb_image.h"

#include "shader.h"
#include "camera.h"
#include "model.h"
#include "utils.h"

#include "shader_macros.h"

#define STRINGIFY_MACRO_EXPANSION(x) #x
#define STRINGIFY(x) STRINGIFY_MACRO_EXPANSION(x)

GLFWwindow* init();
void framebufferSizeCallback(GLFWwindow* window, int width, int height);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadCubemap(const std::vector<std::string>& faces);
void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const Model& grass, const Model& floor_model);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;
const unsigned int SHADOW_RESOLUTION = 1024;
const unsigned int SHADOW_WIDTH = SHADOW_RESOLUTION;
const unsigned int SHADOW_HEIGHT = SHADOW_RESOLUTION;
const float SHADOW_NEAR_PLANE = 5.0f;
const float FAR_PLANE = 100.0f;
const float NEAR_PLANE = 0.1f;

// camera
Camera camera(glm::vec3(0.0f, 5.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// world space positions of our cubes
static const glm::vec3 cube_positions[] = {
		glm::vec3( 0.0f,  0.5f,  0.0f),
		glm::vec3( 4.0f,  10.0f, -30.0f),
		glm::vec3(-3.4f,  6.0f, -15.0f),
		glm::vec3( 3.0f,  4.0f, -5.0f),
		glm::vec3(-2.6f,  2.0f, -3.0f)
};

// lights
float ambient_scale = 0.2f;
glm::vec4 light_color(1.0f);
LightBlock light_block = {
	.lights = {
		// point light
		{
			.dir = glm::vec4(0.0f),
			.pos = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f),
			.ambient = glm::vec4(0.0f),//light_color * ambient_scale,
			.diffuse = glm::vec4(0.0f),//light_color,
			.specular = glm::vec4(0.0f),//light_color,
			.inner_angle_cosine = glm::cos(std::numbers::pi),
			.outer_angle_cosine = glm::cos(std::numbers::pi),
			.constant = 1.0f,
			.linear = 0.0f,
			.quadratic = 0.5f,
		},
		//spotlight
		{
			.dir = glm::vec4(glm::normalize(camera.getFront()), 0.0f),
			.pos = glm::vec4(camera.getPosition(), 1.0f),
			.ambient = glm::vec4(0.0f),//light_color * ambient_scale,
			.diffuse = glm::vec4(0.0f),//light_color,
			.specular = glm::vec4(0.0f),//light_color,
			.inner_angle_cosine = glm::cos(glm::radians(15.0f)),
			.outer_angle_cosine = glm::cos(glm::radians(25.0f)),
			.constant = 1.0f,
			.linear = 0.0f,
			.quadratic = 0.5f,
		}
	},
	// direction light
	.directional_lights = {{
		.dir = glm::normalize(glm::vec4(1.0f, -0.5f, 3.0f, 0.0f)),
		.ambient = light_color * ambient_scale,
		.diffuse = light_color,
		.specular = light_color,
	}},
};

int main()
{
	GLFWwindow* window = init();
	if (!window) return -1;

	static Model floor_model("./assets/other_3d/floor.obj");
	static Model grass("./assets/other_3d/grass.obj");
	static Model cube("./assets/other_3d/cube.obj");
	static Model backpack("./assets/backpack/backpack.obj");

	Shader light_shader("./glsl/light.vert", "./glsl/light.frag", "", "./glsl/include/shader_macros.h");
	Shader skybox_shader("./glsl/skybox.vert", "./glsl/skybox.frag");
	Shader default_shader("./glsl/default.vert", "./glsl/default.frag", "", "./glsl/include/shader_macros.h");
	Shader normal_shader("./glsl/normal.vert", "./glsl/normal.frag", "./glsl/normal.geom");
	Shader shadow_shader("./glsl/shadow.vert", "./glsl/shadow.frag");

	default_shader.activate();
	default_shader.setFloat("material.shininess", std::pow(2, 4));

	unsigned int skybox = loadCubemap(
		std::vector<std::string>({
			"./assets/skybox/right.jpg",
			"./assets/skybox/left.jpg",
			"./assets/skybox/top.jpg",
			"./assets/skybox/bottom.jpg",
			"./assets/skybox/front.jpg",
			"./assets/skybox/back.jpg"
		})
	);
	skybox_shader.activate();
	skybox_shader.setInt("skybox", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, skybox);

	// shadow frame buffer
	// ----
	unsigned int depth_map_fbo;
	glGenFramebuffers(1, &depth_map_fbo);

	unsigned int depth_map;
	glGenTextures(1, &depth_map);
	glBindTexture(GL_TEXTURE_2D, depth_map);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	glm::vec4 border_color(1.0f, 1.0f, 1.0f, 1.0f);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, glm::value_ptr(border_color));

	glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_map, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float current_frame = glfwGetTime();
		deltaTime = current_frame - lastFrame;
		lastFrame = current_frame;
		// input
		processInput(window);

		// uniform buffer object
		unsigned int ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		GLuint ubo_index_default = glGetUniformBlockIndex(default_shader.getId(), "LightBlock");
		GLuint ubo_index_light = glGetUniformBlockIndex(light_shader.getId(), "LightBlock");
		GLint ubo_size_default, ubo_size_light = 0;
		glGetActiveUniformBlockiv(default_shader.getId(), ubo_index_default, GL_UNIFORM_BLOCK_DATA_SIZE, &ubo_size_default);
		if (sizeof(light_block) != ubo_size_default) {
			utils::err() << "uniform block sizes do not match" << utils::endl;
			return -1;
		}
		GLvoid* buffer = malloc(sizeof(light_block));
		if (buffer == NULL) {
			utils::err() << "failed to create uniform block buffer" << utils::endl;
			return -1;
		}
		light_block.lights[1].dir = glm::vec4(camera.getFront(), 1.0f);
		light_block.lights[1].pos = glm::vec4(camera.getPosition(), 1.0f);
		memcpy(buffer, &light_block, sizeof(light_block));
		glBufferData(GL_UNIFORM_BUFFER, sizeof(light_block), buffer, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, ubo_index_default, ubo);

		// start rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, NEAR_PLANE, FAR_PLANE);
		glm::mat4 view = camera.getViewMatrix();

		// shadows
		// ----
		glCullFace(GL_FRONT);
		glPolygonOffset(1.0f, 1.0f);
		glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
		glBindFramebuffer(GL_FRAMEBUFFER, depth_map_fbo);
		glClear(GL_DEPTH_BUFFER_BIT);
		float shadow_render_distance = 100;
		glm::vec3 world_up(0.0, 1.0, 0.0);
		glm::vec3 light_position((glm::vec3(-light_block.directional_lights[0].dir) * ((shadow_render_distance/2) + SHADOW_NEAR_PLANE)) + camera.getPosition());
		glm::vec3 light_front(light_block.directional_lights[0].dir);
		glm::vec3 light_right(glm::normalize(glm::cross(light_front, world_up)));
		glm::vec3 light_up(glm::normalize(glm::cross(light_right, light_front)));
		glm::mat4 light_view = glm::lookAt(light_position, light_position + light_front, light_up);
		glm::mat4 light_projection = glm::ortho(-shadow_render_distance/2, shadow_render_distance/2, -shadow_render_distance/2, shadow_render_distance/2, SHADOW_NEAR_PLANE, shadow_render_distance + SHADOW_NEAR_PLANE);
		renderScene(light_view, light_projection, shadow_shader, grass, floor_model);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
		glPolygonOffset(0.0f, 0.0f);
		glCullFace(GL_BACK);

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		default_shader.activate();
		default_shader.setInt("depth_map", 16);
		glActiveTexture(GL_TEXTURE0 + 16);
		glBindTexture(GL_TEXTURE_2D, depth_map);
		default_shader.setMat4("light_view", light_view);
		default_shader.setMat4("light_projection", light_projection);
		renderScene(view, projection, default_shader, grass, floor_model);

		// lights
		// ----
		light_shader.activate();
		light_shader.setMat4("view", view);
		light_shader.setMat4("projection", projection);

		// non-directional lights
		for (int i=0; i<NUM_LIGHTS; i++) {
			glm::vec4 zero(0.0f);
			Light cur_light = light_block.lights[i];
			if (glm::all(glm::equal(cur_light.pos, glm::vec4(camera.getPosition(), 1.0))) || (
					(glm::all(glm::equal(cur_light.ambient, zero))) &&
					(glm::all(glm::equal(cur_light.diffuse, zero))) &&
					(glm::all(glm::equal(cur_light.specular, zero)))
				)
			) {
				continue;
			}
			light_shader.setVec4("light_color", cur_light.diffuse);
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, glm::vec3(cur_light.pos));
			model = glm::scale(model, glm::vec3(0.2f));

			light_shader.setMat4("model", model);
			cube.Draw(light_shader);
		}

		// directional lights
		for (int i=0; i<NUM_DIRECTIONAL_LIGHTS; i++) {
			glm::vec4 zero(0.0f);
			Directional_Light cur_light = light_block.directional_lights[i];
			if ((glm::all(glm::equal(cur_light.ambient, zero))) &&
				(glm::all(glm::equal(cur_light.diffuse, zero))) &&
				(glm::all(glm::equal(cur_light.specular, zero)))) {
				continue;
			}
			light_shader.setVec4("light_color", cur_light.diffuse);
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, camera.getPosition() + (glm::vec3(-cur_light.dir) * 100.0f));
			model = glm::scale(model, glm::vec3(10.0f));
			
			light_shader.setMat4("model", model);
			cube.Draw(light_shader);
		}

		// skybox
		glCullFace(GL_FRONT);
		skybox_shader.activate();
		skybox_shader.setMat4("view", glm::mat4(glm::mat3(view)));
		skybox_shader.setMat4("projection", projection);
		cube.Draw(skybox_shader);
		glCullFace(GL_BACK);

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

GLFWwindow* init()
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
		utils::err() << "Failed to create GLFW window" << utils::endl;
		glfwTerminate();
		return nullptr;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
	glfwSetCursorPosCallback(window, mouseCallback);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		utils::err() << "Failed to initialize GLAD" << utils::endl;
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

	return window;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	float speed = 1.0f;
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		speed = 2.0f;
	if ((glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS))
		camera.processMovement(Camera::moveForward, deltaTime * speed);
	if ((glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS))
		camera.processMovement(Camera::moveBackward, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processMovement(Camera::moveLeft, deltaTime * speed);
	if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))
		camera.processMovement(Camera::moveRight, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.processMovement(Camera::moveUp, deltaTime * speed);
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		camera.processMovement(Camera::moveDown, deltaTime * speed);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebufferSizeCallback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions
	// width and height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


void mouseCallback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	// y moves from bottom to top
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.processRotation(xoffset, yoffset);
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processZoom(yoffset);
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
			GLenum format;
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

void renderScene(const glm::mat4& view, const glm::mat4& projection, const Shader& shader, const Model& grass, const Model& floor_model)
{
	shader.activate();
	shader.setMat4("view", view);
	shader.setMat4("projection", projection);
	shader.setMat3("light_normal_mat", glm::transpose(glm::inverse(glm::mat3(view))));

	// floor
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(25.0f, 1.0f, 25.0f));
	glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(view * model)));
	shader.setMat4("model", model);
	shader.setMat3("normal_mat", normal_mat);
	floor_model.Draw(shader);

	// floor flipped and lowered
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, -0.1, 0.0f));
	model = glm::rotate(model, glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	model = glm::scale(model, glm::vec3(25.0f, 1.0f, 25.0f));
	normal_mat = glm::transpose(glm::inverse(glm::mat3(view * model)));
	shader.setMat4("model", model);
	shader.setMat3("normal_mat", normal_mat);
	floor_model.Draw(shader);

	for (unsigned int i = 0; i < (sizeof(cube_positions)/sizeof(cube_positions[0])); i++)
	{
		// calculate the model matrix for each object and pass it to shader before drawing
		glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
		model = glm::translate(model, cube_positions[i]);
		float angle = 20.0f * i;
		//model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
		glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(view * model)));

		shader.setMat4("model", model);
		shader.setMat3("normal_mat", normal_mat);
		grass.Draw(shader);
	}
}
