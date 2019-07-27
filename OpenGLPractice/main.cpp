#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <GLFW/glfw3.h>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

//shaders
const GLchar *vertexShaderSource = R"(
	#version 330 core
	layout(location = 0) in vec3 aPos;
	
	void main()
	{
		gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);
	}
	)";
const GLchar *fragmentShaderSourceTurquoise = R"(
	#version 330 core
	out vec4 FragColor;
	
	void main()
	{
		FragColor = vec4(0.0f, 1.0f, 1.0f, 1.0f);
	}
	)";
const GLchar *fragmentShaderSourceYellow = R"(
	#version 330 core
	out vec4 FragColor;
	
	void main()
	{
		FragColor = vec4(1.0f, 1.0f, 0.0f, 1.0f);
	}
	)";

int main() {

	// init window
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// define window object
	GLFWwindow* window = glfwCreateWindow(800, 600, "LearnOpenGL", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// size of rendering window
	glViewport(0, 0, 800, 600);

	// callback when window size is changed
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// vertice and indices to render
	// side by side triangles
	float vertices[] = {
		0.0f, -0.5f, 0.0f,
		1.0f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,
		-1.0f, -0.5f, 0.0f,
		-0.5f, 0.5f, 0.0f
	};
	unsigned int indices1[] = {
		0, 1, 2,
		0, 3, 4
	};
	unsigned int indices2[] = {
		0, 1, 4
	};

	// create vertex buffer object (VBO)
	unsigned int VBO;
	glGenBuffers(1, &VBO);

	// create element buffer object (EBO)
	unsigned int EBO1;
	glGenBuffers(1, &EBO1);
	unsigned int EBO2;
	glGenBuffers(1, &EBO2);

	// create vertex shader object
	unsigned int vertexShader;
	vertexShader = glCreateShader(GL_VERTEX_SHADER);
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

	// create fragment shader object
	unsigned int fragmentShaderTurquoise;
	fragmentShaderTurquoise = glCreateShader(GL_FRAGMENT_SHADER);
	// attach and compile shader
	glShaderSource(fragmentShaderTurquoise, 1, &fragmentShaderSourceTurquoise, NULL);
	glCompileShader(fragmentShaderTurquoise);
	// check if fragment shader compilation was successfull
	success = 1;
	glGetShaderiv(fragmentShaderTurquoise, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderTurquoise, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	unsigned int fragmentShaderYellow;
	fragmentShaderYellow = glCreateShader(GL_FRAGMENT_SHADER);
	// attach and compile shader
	glShaderSource(fragmentShaderYellow, 1, &fragmentShaderSourceYellow, NULL);
	glCompileShader(fragmentShaderYellow);
	// check if fragment shader compilation was successfull
	success = 1;
	glGetShaderiv(fragmentShaderYellow, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShaderYellow, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}

	// create shader program object
	unsigned int shaderProgram1;
	shaderProgram1 = glCreateProgram();
	// attach and link shaders
	glAttachShader(shaderProgram1, vertexShader);
	glAttachShader(shaderProgram1, fragmentShaderTurquoise);
	glLinkProgram(shaderProgram1);
	// check if shader program was linked successfully
	success = 1;
	glGetProgramiv(shaderProgram1, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram1, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAM::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
	}
	// create shader program object
	unsigned int shaderProgram2;
	shaderProgram2 = glCreateProgram();
	// attach and link shaders
	glAttachShader(shaderProgram2, vertexShader);
	glAttachShader(shaderProgram2, fragmentShaderYellow);
	glLinkProgram(shaderProgram2);
	// check if shader program was linked successfully
	success = 1;
	glGetProgramiv(shaderProgram2, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(shaderProgram2, 512, NULL, infoLog);
		std::cout << "ERROR::PROGRAM::SHADER::LINKING_FAILED\n" << infoLog << std::endl;
	}
	// don't need shader objects anymore
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShaderTurquoise);
	glDeleteShader(fragmentShaderYellow);

	// create vertex array object (VAO)
	unsigned int VAO1;
	glGenVertexArrays(1, &VAO1);
	unsigned int VAO2;
	glGenVertexArrays(1, &VAO2);

	//VAO init
	// 1. bind VAO
	glBindVertexArray(VAO1);
	// 2. copy our vertices array and indices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO1);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices1), indices1, GL_STATIC_DRAW);
	// 3. then set our vertex attributes pointers
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	// unbind
	glBindVertexArray(0);

	// 1. bind VAO
	glBindVertexArray(VAO2);
	// 2. copy our vertices array and indices array in a buffer for OpenGL to use
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices2), indices2, GL_STATIC_DRAW);
	// 3. then set our vertex attributes pointers
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
		// 4. draw the object
		glUseProgram(shaderProgram1);
		glBindVertexArray(VAO1);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		// unbind VAO
		glBindVertexArray(0);
		// 4. draw the object
		glUseProgram(shaderProgram2);
		glBindVertexArray(VAO2);
		glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
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