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

	// load and create a texture 
	// -------------------------
	unsigned int texture;
	// texture 1
	// ---------
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	// set the texture wrapping parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// set texture filtering parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	// texture metadata
	using json = nlohmann::json;
	std::ifstream f(STRINGIFY(SRC_DIR) "/img/texture.json");
	json texMeta = json::parse(f)["meta"];
	int widthPixels, heightPixels, dirtHeight, dirtWidth, dirtStartX, dirtStartY;
	widthPixels = static_cast<int>(texMeta["size"]["w"].template get<double>());
	heightPixels = static_cast<int>(texMeta["size"]["h"].template get<double>());
	json slices = texMeta["slices"];
	for (int i=0; i<slices.size(); i++) {
		if (strcmp(slices[i]["name"].template get<std::string>().c_str(), "dirt") == 0) {
			dirtStartX = static_cast<int>(slices[i]["keys"][0]["bounds"]["x"].template get<double>());
			dirtStartY = heightPixels - (static_cast<int>(slices[i]["keys"][0]["bounds"]["y"].template get<double>()) + static_cast<int>(slices[i]["keys"][0]["bounds"]["h"].template get<double>()));
			dirtWidth = static_cast<int>(slices[i]["keys"][0]["bounds"]["w"].template get<double>());
			dirtHeight = static_cast<int>(slices[i]["keys"][0]["bounds"]["h"].template get<double>());
		}
	}

	float dirtStartXPerc = dirtStartX/static_cast<float>(widthPixels);
	float dirtEndXPerc = (dirtStartX+dirtWidth)/static_cast<float>(widthPixels);
	float dirtStartYPerc = dirtStartY/static_cast<float>(heightPixels);
	float dirtEndYPerc = (dirtStartY+dirtHeight)/static_cast<float>(heightPixels);
	float dirtXPerc = dirtWidth/static_cast<float>(widthPixels);
	float dirtYPerc = dirtHeight/static_cast<float>(heightPixels);

	// load image, create texture and generate mipmaps
	int width, height, nrChannels;
	stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
	unsigned char *data = stbi_load(STRINGIFY(SRC_DIR) "/img/texture.png", &width, &height, &nrChannels, 0);
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
		-0.5f, -0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom left
		 0.5f, -0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom right
		 0.5f,  0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*2/3), // top right
		-0.5f,  0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*2/3), // top left
		// back face
		 0.5f, -0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*3/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom left
		-0.5f, -0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*4/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom right
		-0.5f,  0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*4/4), dirtStartYPerc + (dirtYPerc*2/3), // top right
		 0.5f,  0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*3/4), dirtStartYPerc + (dirtYPerc*2/3), // top left
		// left face
		-0.5f, -0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*0/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom left
		-0.5f, -0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom right
		-0.5f,  0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*2/3), // top right
		-0.5f,  0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*0/4), dirtStartYPerc + (dirtYPerc*2/3), // top left
		// right face
		 0.5f, -0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom left
		 0.5f, -0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*3/4), dirtStartYPerc + (dirtYPerc*1/3), // bottom right
		 0.5f,  0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*3/4), dirtStartYPerc + (dirtYPerc*2/3), // top right
		 0.5f,  0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*2/3), // top left
		// bottom face
		-0.5f, -0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*0/3), // bottom left
		 0.5f, -0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*0/3), // bottom right
		-0.5f, -0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*1/3), // top right
		 0.5f, -0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*1/3), // top left
		// top face
		-0.5f,  0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*2/3), // bottom left
		 0.5f,  0.5f,  0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*2/3), // bottom right
		 0.5f,  0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*2/4), dirtStartYPerc + (dirtYPerc*3/3), // top right
		-0.5f,  0.5f, -0.5f, dirtStartXPerc + (dirtXPerc*1/4), dirtStartYPerc + (dirtYPerc*3/3), // top left
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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// texture coord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);

	Shader ourShader(STRINGIFY(SRC_DIR) "/glsl/vertexShader.glsl", STRINGIFY(SRC_DIR) "/glsl/fragmentShader.glsl");
	ourShader.use();
	// Set to texture unit 0
	ourShader.setInt("texture", 0);

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// bind textures on corresponding texture units
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		// activate shader
		ourShader.use();

		// pass projection matrix to shader (note that in this case it could change every frame)
		glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		
		ourShader.setMat4("projection", projection);

		// camera/view transformation
		glm::mat4 view = camera.getViewMatrix();
		ourShader.setMat4("view", view);

		// render boxes
		glBindVertexArray(VAO);
		for (unsigned int i = 0; i < 10; i++)
		{
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
			model = glm::translate(model, cubePositions[i]);
			float angle = 20.0f * i;
			model = glm::rotate(model, glm::radians(angle), glm::vec3(1.0f, 0.3f, 0.5f));
			ourShader.setMat4("model", model);

			glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
		}

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
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