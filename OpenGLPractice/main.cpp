#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
GLFWwindow* init(int width, int height);

//shaders
const GLchar *vertexShaderSource = R"(
	#version 330 core
	layout(location = 0) in vec3 aPos;
	
	void main()
	{
		gl_Position = vec4(aPos, 1.0);
	}
	)";
const GLchar *fragmentShaderSource = R"(
	#version 330 core
	out vec4 FragColor;

	uniform vec4 ourColor; // we set this variable in the OpenGL code.
	
	void main()
	{
		FragColor = ourColor;
	}
	)";

int main() {

	GLFWwindow* window;
	try {
		window = init(800, 600);
	} catch (std::exception e) { return -1; }

	// vertice and indices to render
	float vertices[] = {
	-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f
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

	// create shader objects
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	unsigned int fragmentShader;
	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

	// attach and compile shader
	glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
	glCompileShader(vertexShader);
	// check if vertex shader compilation was successful
	int  success;
	char infoLog[512];
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// attach and compile shader
	glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
	glCompileShader(fragmentShader);
	// check if fragment shader compilation was successfull
	success = 1;
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// create shader program object
	unsigned int shaderProgram;
	shaderProgram = glCreateProgram();
	// attach and link shaders
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);
	glLinkProgram(shaderProgram);
	// check if shader program was linked successfully
	success = 1;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAM::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
	}

	// don't need shader objects anymore
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

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
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// unbind
	glBindVertexArray(0);

	// draw in wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	// draw in normal mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		//input
		processInput(window);

		// clear the screen
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		// rendering commands here
		glUseProgram(shaderProgram);
		// set shader program color
		float timeValue = glfwGetTime();
		float greenValue = (sin(timeValue) / 2.0f) + 0.5f;
		int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
		glUniform4f(vertexColorLocation, 0.0f, greenValue, 0.0f, 1.0f);
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