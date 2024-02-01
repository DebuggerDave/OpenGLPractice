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

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// world space positions of our cubes
static const glm::vec3 cube_positions[] = {
		glm::vec3( 0.0f,  0.0f,  -6.0f),
		glm::vec3( 4.0f,  10.0f, -30.0f),
		glm::vec3(-3.0f, -4.4f, -5.0f),
		glm::vec3(-7.6f, -4.0f, -24.6f),
		glm::vec3( 4.8f, -0.8f, -7.0f),
		glm::vec3(-3.4f,  6.0f, -15.0f),
		glm::vec3( 2.6f, -4.0f, -5.0f),
		glm::vec3( 3.0f,  4.0f, -5.0f),
		glm::vec3( 3.0f,  0.4f, -3.0f),
		glm::vec3(-2.6f,  2.0f, -3.0f)
};

// lights
float ambient_scale = 0.05f;
glm::vec4 light_color(1.0f);
LightBlock light_block = {
	.lights = {
		// point light
		{
			.dir = glm::vec4(0.0f),
			.pos = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f),
			.ambient = light_color * ambient_scale,
			.diffuse = light_color,
			.specular = light_color,
			.inner_angle_cosine = glm::cos(std::numbers::pi),
			.outer_angle_cosine = glm::cos(std::numbers::pi),
			.constant = 1.0f,
			.linear = 0.09f,
			.quadratic = 0.032f,
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
			.linear = 0.09f,
			.quadratic = 0.032f,
		}
	},
	// direction light
	.directional_lights = {{
		.dir = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		.ambient = light_color * ambient_scale,
		.diffuse = light_color,
		.specular = light_color,
	}},
};


int main()
{
	GLFWwindow* window = init();
	if (!window) return -1;

	Shader light_shader("./glsl/light.vert", "./glsl/light.frag", "", "./glsl/include/shader_macros.h");
	Shader skybox_shader("./glsl/skybox.vert", "./glsl/skybox.frag");
	Shader default_shader("./glsl/default.vert", "./glsl/default.frag", "", "./glsl/include/shader_macros.h");
	Shader normal_shader("./glsl/normal.vert", "./glsl/normal.frag", "./glsl/normal.geom");
	default_shader.activate();
	default_shader.setFloat("material.shininess", std::pow(2, 6));

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


	auto start_time = std::chrono::high_resolution_clock::now();
	Model grass("./assets/other_3d/grass.obj");
	Model cube("./assets/other_3d/cube.obj");
	Model backpack("./assets/backpack/backpack.obj");
	auto end_time = std::chrono::high_resolution_clock::now();
	std::cout << "Loaded models in " << std::chrono::duration_cast<std::chrono::seconds>(end_time - start_time).count() << " seconds\n";

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
		glm::mat4 projection = glm::perspective(glm::radians(camera.getZoom()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();

		// lights
		// -------------------------------------------
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

		// non-lights
		// --------------------------------------------------
		default_shader.activate();
		default_shader.setMat4("view", view);
		default_shader.setMat4("projection", projection);
		default_shader.setMat3("light_normal_mat", glm::transpose(glm::inverse(glm::mat3(view))));

		normal_shader.activate();
		normal_shader.setMat4("view", view);
		normal_shader.setMat4("projection", projection);

		for (unsigned int i = 0; i < (sizeof(cube_positions)/sizeof(cube_positions[0])); i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, cube_positions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(view * model)));

			default_shader.activate();
			default_shader.setMat4("model", model);
			default_shader.setMat3("normal_mat", normal_mat);
			grass.Draw(default_shader);

			normal_shader.activate();
			normal_shader.setMat4("model", model);
			normal_shader.setMat3("normal_mat",  normal_mat);
			grass.Draw(normal_shader);
		}

		// skybox
		glDisable(GL_CULL_FACE);
		skybox_shader.activate();
		skybox_shader.setMat4("view", glm::mat4(glm::mat3(view)));
		skybox_shader.setMat4("projection", projection);
		cube.Draw(skybox_shader);
		glEnable(GL_CULL_FACE);

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
	glEnable(GL_CULL_FACE);

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
