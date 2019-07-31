#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
GLFWwindow* init(int width, int height);

int main() {

	GLFWwindow* window;
	try {
		window = init(800, 600);
	} catch (std::exception e) { return -1; }

	// vertice and indices to render
	float vertices[] = {
		// positions         // colors
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,   // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,   // bottom left
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f    // top 
	};
	unsigned int indices[] = {
		0, 1, 2
	};

	// create vertex buffer object (VBO)
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// create element buffer object (EBO)
	unsigned int EBO;
	glGenBuffers(1, &EBO);

	Shader ourShader("vertexShader.glsl", "fragmentShader.glsl");

	// create vertex array object (VAO)
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);

	//VAO init
	// bind VAO
	glBindVertexArray(VAO);
	// copy our vertices array and indices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	// then set our vertex attributes pointers
	// position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	// color
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	// unbind
	glBindVertexArray(0);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		// clear the screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// rendering commands here
		ourShader.use();
		// draw the object
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// unbind VAO
		glBindVertexArray(0);

		// check and call events and swap the buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	// release resources
	glfwTerminate();
	return 0;
}

// change render window size
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

// check if escape key was pressed
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// init project
GLFWwindow* init(int width, int height) {
	// init window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// define window object
	GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		throw std::exception();
	}
	glfwMakeContextCurrent(window);

	// init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		throw std::exception();
	}

	// size of rendering window
	glViewport(0, 0, width, height);

	// callback when window size is changed
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	return window;
}