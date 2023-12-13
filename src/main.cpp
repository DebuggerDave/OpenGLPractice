#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>
#include <fstream>

#include "stb_image.h"

#include "shader.h"
#include "camera.h"

#include "nlohmann/json.hpp"

#define STRINGIFY_MACRO_EXPANSION(x) #x
#define STRINGIFY(x) STRINGIFY_MACRO_EXPANSION(x)

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
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
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
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

	// create top texture
	unsigned int top_texture;
	glGenTextures(1, &top_texture);
	glBindTexture(GL_TEXTURE_2D, top_texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	int width, height, nrChannels = 0;
	unsigned char *data = stbi_load(STRINGIFY(BINARY_DIR) "/img/grass_top.png", &width, &height, &nrChannels, 0);
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

	// create side texture
	unsigned int side_texture;
	glGenTextures(1, &side_texture);
	glBindTexture(GL_TEXTURE_2D, side_texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	width = 0, height = 0, nrChannels = 0; data = nullptr;
	data = stbi_load(STRINGIFY(BINARY_DIR) "/img/grass_side.png", &width, &height, &nrChannels, 0);
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

	// create bottom texture
	unsigned int bottom_texture;
	glGenTextures(1, &bottom_texture);
	glBindTexture(GL_TEXTURE_2D, bottom_texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// load image, create texture and generate mipmaps
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	width = 0; height = 0; nrChannels = 0; data = nullptr;
	data = stbi_load(STRINGIFY(BINARY_DIR) "/img/grass_bottom.png", &width, &height, &nrChannels, 0);
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

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
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
	glm::vec3 cubePositions[] = {
		glm::vec3( 0.0f,  0.0f,  0.0f),
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

	Shader default_shader(STRINGIFY(BINARY_DIR) "/glsl/default_vert.glsl", STRINGIFY(BINARY_DIR) "/glsl/default_frag.glsl");
	default_shader.use();
	// Set to texture unit 0
	default_shader.setInt("top_texture", 0);
	default_shader.setInt("side_texture", 1);
	default_shader.setInt("bottom_texture", 2);


	Shader light_shader(STRINGIFY(BINARY_DIR) "/glsl/light_vert.glsl", STRINGIFY(BINARY_DIR) "/glsl/light_frag.glsl");
	light_shader.use();

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// input
		processInput(window);

		// start rendering
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind textures
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, top_texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, side_texture);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, bottom_texture);

		// transformations
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.getViewMatrix();

		light_shader.use();
		glBindVertexArray(lightVAO);
		glm::vec3 light_pos(1.2f, 1.0f, 2.0f);
		glm::mat4 model = glm::mat4(1.0f);
		model = glm::translate(model, light_pos);
		model = glm::scale(model, glm::vec3(0.2f));
		glm::vec3 light_color(1.0f, 1.0f, 1.0f);
		light_shader.setVec3("light_color", light_color);
		light_shader.setMat4("model", model);
		light_shader.setMat4("view", view);
		light_shader.setMat4("projection", projection);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

		default_shader.use();
		default_shader.setVec3("light_color", light_color);
		default_shader.setVec3("light_pos_world_space", light_pos);
		default_shader.setMat4("view", view);
		default_shader.setMat4("projection", projection);
		// render boxes
		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			default_shader.setMat4("model", model);

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
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if ((glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS))
		camera.processMovement(Camera::moveForward, deltaTime);
	if ((glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS))
		camera.processMovement(Camera::moveBackward, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.processMovement(Camera::moveLeft, deltaTime);
	if ((glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) || (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS))
		camera.processMovement(Camera::moveRight, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.processMovement(Camera::moveUp, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.processMovement(Camera::moveDown, deltaTime);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
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