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

#include "stb_image.h"

#include "shader.h"
#include "camera.h"

#include "nlohmann/json.hpp"

#include "shader_macros.h"

#define STRINGIFY_MACRO_EXPANSION(x) #x
#define STRINGIFY(x) STRINGIFY_MACRO_EXPANSION(x)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow* window);
void setup_tex(unsigned int& tex, const char* file_path);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;


int main()
{
	// glfw init
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
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// Make sure fragment scene depth is calculated properly
	glEnable(GL_DEPTH_TEST);

	unsigned int top_diffuse;
	setup_tex(top_diffuse, STRINGIFY(BINARY_DIR) "/img/grass_top.png");
	unsigned int side_diffuse;
	setup_tex(side_diffuse, STRINGIFY(BINARY_DIR) "/img/grass_side.png");
	unsigned int bottom_diffuse;
	setup_tex(bottom_diffuse, STRINGIFY(BINARY_DIR) "/img/grass_bottom.png");
	unsigned int top_specular;
	setup_tex(top_specular, STRINGIFY(BINARY_DIR) "/img/grass_top_specular.png");
	unsigned int side_specular;
	setup_tex(side_specular, STRINGIFY(BINARY_DIR) "/img/grass_side_specular.png");
	unsigned int bottom_specular;
	setup_tex(bottom_specular, STRINGIFY(BINARY_DIR) "/img/grass_bottom_specular.png");

	// set up vertex data (and buffer(s)) and configure vertex attributes
	float vertices[] = {
		// front face
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  0.0f, // bottom left
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  0.0f, // bottom right
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f,  1.0f, // top right
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  0.0f,  1.0f, // top left
		// back face
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  0.0f, // bottom left
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  0.0f, // bottom right
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f,  1.0f, // top right
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f,  1.0f, // top left
		// left face
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom left
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // bottom right
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // top right
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // top left
		// right face
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  0.0f, // bottom left
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  0.0f, // bottom right
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f,  1.0f, // top right
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, // top left
		// bottom face
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  0.0f, // bottom left
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  0.0f, // bottom right
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f,  1.0f, // top right
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f,  1.0f, // top left
		// top face
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  0.0f, // bottom left
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  0.0f, // bottom right
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f,  1.0f, // top right
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f,  1.0f, // top left

	};
	// world space positions of our cubes
	glm::vec3 cube_positions[] = {
		glm::vec3( 0.0f,  0.0f,  -3.0f),
		glm::vec3( 2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3( 2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3( 1.3f, -2.0f, -2.5f),
		glm::vec3( 1.5f,  2.0f, -2.5f),
		glm::vec3( 1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};
	unsigned int indices[] = {
		0 , 1 , 3 , 1 , 2 , 3,
		4 , 5 , 7 , 5 , 6 , 7,
		8 , 9 , 11, 9 , 10, 11,
		12, 13, 15, 13, 14, 15,
		16, 17, 19, 17, 18, 19,
		20, 21, 23, 21, 22, 23
	};

	unsigned int VBO, VAO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	unsigned int EBO;
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	// we only need to bind to the VBO, the container's VBO's data already contains the data.
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// normal attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	// texture coord attribute
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);

	Shader default_shader(STRINGIFY(BINARY_DIR) "/glsl/default.vert", STRINGIFY(BINARY_DIR) "/glsl/default.frag", nullptr, STRINGIFY(BINARY_DIR) "/glsl/include/shader_macros.h");
	default_shader.use();
	float shininess = std::pow(2, 6);
	// Texture units
	default_shader.setInt("top_material.diffuse", 0);
	default_shader.setInt("side_material.diffuse", 1);
	default_shader.setInt("bottom_material.diffuse", 2);
	default_shader.setInt("top_material.specular", 3);
	default_shader.setInt("side_material.specular", 4);
	default_shader.setInt("bottom_material.specular", 5);
	default_shader.setFloat("top_material.shininess", shininess);
	default_shader.setFloat("side_material.shininess", shininess);
	default_shader.setFloat("bottom_material.shininess", shininess);

	Shader light_shader(STRINGIFY(BINARY_DIR) "/glsl/light.vert", STRINGIFY(BINARY_DIR) "/glsl/light.frag", nullptr, STRINGIFY(BINARY_DIR) "/glsl/include/shader_macros.h");
	light_shader.use();

	float ambient_scale = 0.2f;
	glm::vec4 light_color(1.0f);

	LightBlock light_block;
	{
		// direction light
		light_block.directional_lights[0].dir = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
		light_block.directional_lights[0].ambient = light_color * ambient_scale;
		light_block.directional_lights[0].diffuse = light_color;
		light_block.directional_lights[0].specular = light_color;

		// point light
		light_block.lights[0].dir = glm::vec4(0.0f);
		light_block.lights[0].pos = glm::vec4(0.0f, 0.0f, -5.0f, 1.0f);
		light_block.lights[0].ambient = light_color * ambient_scale;
		light_block.lights[0].diffuse = light_color;
		light_block.lights[0].specular = light_color;
		light_block.lights[0].inner_angle_cosine = glm::cos(std::numbers::pi);
		light_block.lights[0].outer_angle_cosine = glm::cos(std::numbers::pi);
		light_block.lights[0].constant = 1.0f;
		light_block.lights[0].linear = 0.09f;
		light_block.lights[0].quadratic = 0.032f;

		// spotlight
		light_block.lights[1].dir = glm::vec4(glm::normalize(camera.front), 0.0f);
		light_block.lights[1].pos = glm::vec4(camera.position, 1.0f);
		light_block.lights[1].ambient = light_color * ambient_scale;
		light_block.lights[1].diffuse = light_color;
		light_block.lights[1].specular = light_color;
		light_block.lights[1].inner_angle_cosine = glm::cos(glm::radians(15.0f));
		light_block.lights[1].outer_angle_cosine = glm::cos(glm::radians(25.0f));
		light_block.lights[1].constant = 1.0f;
		light_block.lights[1].linear = 0.09f;
		light_block.lights[1].quadratic = 0.032f;
	}

	// bind textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, top_diffuse);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, side_diffuse);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, bottom_diffuse);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, top_specular);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, side_specular);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, bottom_specular);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float current_frame = glfwGetTime();
		deltaTime = current_frame - lastFrame;
		lastFrame = current_frame;
		// input
		process_input(window);

		// uniform buffer object
		unsigned int ubo;
		glGenBuffers(1, &ubo);
		glBindBuffer(GL_UNIFORM_BUFFER, ubo);
		GLuint ubo_index_default = glGetUniformBlockIndex(default_shader.id, "LightBlock");
		GLuint ubo_index_light = glGetUniformBlockIndex(light_shader.id, "LightBlock");
		GLint ubo_size_default, ubo_size_light = 0;
		glGetActiveUniformBlockiv(default_shader.id, ubo_index_default, GL_UNIFORM_BLOCK_DATA_SIZE, &ubo_size_default);
		if (sizeof(light_block) != ubo_size_default) {
			std::cerr << "uniform block sizes do not match";
			return -1;
		}
		GLvoid* buffer = malloc(sizeof(light_block));
		if (buffer == NULL) {
			std::cout << "failed to create uniform block buffer";
			return -1;
		}
		light_block.lights[1].dir = glm::vec4(camera.front, 1.0f);
		light_block.lights[1].pos = glm::vec4(camera.position, 1.0f);
		memcpy(buffer, &light_block, sizeof(light_block));
		glBufferData(GL_UNIFORM_BUFFER, sizeof(light_block), buffer, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, ubo_index_default, ubo);

		// start rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();

		// lights
		// -------------------------------------------
		light_shader.use();
		light_shader.setMat4("view", view);
		light_shader.setMat4("projection", projection);
		glBindVertexArray(lightVAO);

		// non-directional lights
		for (int i=0; i<NUM_LIGHTS; i++) {
			glm::vec4 zero(0.0f);
			Light cur_light = light_block.lights[i];
			if (glm::all(glm::equal(cur_light.pos, glm::vec4(camera.position, 1.0))) || (
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
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
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

			model = glm::translate(model, camera.position + (glm::vec3(-cur_light.dir) * 100.0f));
			model = glm::scale(model, glm::vec3(10.0f));

			light_shader.setMat4("model", model);
			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// non-lights
		// --------------------------------------------------
		default_shader.use();
		default_shader.setMat4("view", view);
		default_shader.setMat4("projection", projection);
		default_shader.setMat3("light_normal_mat", glm::transpose(glm::inverse(glm::mat3(view))));
		glBindVertexArray(VAO);

		for (unsigned int i = 0; i < (sizeof(cube_positions)/sizeof(cube_positions[0])); i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, cube_positions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			default_shader.setMat4("model", model);
			default_shader.setMat3("normal_mat", glm::transpose(glm::inverse(glm::mat3(view * model))));

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);

	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void process_input(GLFWwindow *window)
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

void setup_tex(unsigned int& tex, const char* file_path) {
	// create diffuse texture
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	int width, height, nrChannels = 0;
	unsigned char *data = stbi_load(file_path, &width, &height, &nrChannels, 0);
	if (data)
	{
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else
	{
		std::cout << "Failed to load texture" << std::endl;
	}
	stbi_image_free(data);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions
	// width and height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos)
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.processZoom(yoffset);
}