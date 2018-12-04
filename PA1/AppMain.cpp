#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <SOIL2/SOIL2.h>

using namespace std;

int width, height;
const double PI = 3.14159;
const float toRadians = PI / 180.0f;

// Declare Input Callback Function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mode);

// Declare View Matrix
glm::mat4 viewMatrix;

// Camera Field of View
GLfloat fov = 45.0f;

void initiateCamera();

// Define Camera Attributes
glm::vec3 cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f); // Move 3 units back in z towards screen
glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f); // What the camera points to
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target); // direction z
glm::vec3 worldUp = glm::vec3(0.0, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));// right vector x
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight)); // up vector y
glm::vec3 CameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // 1 unit away from lense



// Camera Transformation Prototype
void TransformCamera();

// Boolean array for keys and mouse buttons
bool keys[1024], mouseButtons[3];

// Input state booleans
bool isPanning = false, isOrbiting = false;

// Pitch and Yaw
GLfloat radius = 3.0f, rawYaw = 0.0f, rawPitch = 0.0f, degYaw, degPitch;

GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat lastX = 320, lastY = 240, xChange, yChange; // Center mouse cursor
bool firstMouseMove = true;


// Draw Primitive(s)
void draw()
{
	GLenum mode = GL_TRIANGLES;
	GLsizei indices = 6;
	glDrawElements(mode, indices, GL_UNSIGNED_BYTE, nullptr);
}



// Create and Compile Shaders
static GLuint CompileShader(const string& source, GLuint shaderType)
{
	// Create Shader object
	GLuint shaderID = glCreateShader(shaderType);
	const char* src = source.c_str();

	// Attach source code to Shader object
	glShaderSource(shaderID, 1, &src, nullptr);

	// Compile Shader
	glCompileShader(shaderID);

	// Return ID of Compiled shader
	return shaderID;

}

// Create Program Object
static GLuint CreateShaderProgram(const string& vertexShader, const string& fragmentShader)
{
	// Compile vertex shader
	GLuint vertexShaderComp = CompileShader(vertexShader, GL_VERTEX_SHADER);

	// Compile fragment shader
	GLuint fragmentShaderComp = CompileShader(fragmentShader, GL_FRAGMENT_SHADER);

	// Create program object
	GLuint shaderProgram = glCreateProgram();

	// Attach vertex and fragment shaders to program object
	glAttachShader(shaderProgram, vertexShaderComp);
	glAttachShader(shaderProgram, fragmentShaderComp);

	// Link shaders to create executable
	glLinkProgram(shaderProgram);

	// Delete compiled vertex and fragment shaders
	glDeleteShader(vertexShaderComp);
	glDeleteShader(fragmentShaderComp);

	// Return Shader Program
	return shaderProgram;

}


int main(void)
{
	width = 640; height = 480;

	GLFWwindow* window;

	/* Initialize the library */
	if (!glfwInit())
		return -1;

	/* Create a windowed mode window and its OpenGL context */
	window = glfwCreateWindow(width, height, "Main Window", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		return -1;
	}

	// Set input callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);

	/* Make the window's context current */
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	if (glewInit() != GLEW_OK)
		cout << "Error!" << endl;


	GLfloat vertices[] = {

		// Triangle 1
		-0.5, -0.5, 0.0, // index 0
		1.0, 0.0, 0.0, // red

		-0.5, 0.5, 0.0, // index 1
		0.0, 1.0, 0.0, // green

		0.5, -0.5, 0.0,  // index 2	
		0.0, 0.0, 1.0, // blue

		// Triangle 2	
		0.5, 0.5, 0.0,  // index 3	
		1.0, 0.0, 1.0 // purple
	};

	// Define element indices
	GLubyte indices[] = {
		0, 1, 2,
		1, 2, 3
	};

	// Plane Transforms
	glm::vec3 planePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.5f),
		glm::vec3(0.5f,  0.0f,  0.0f),
		glm::vec3(0.0f,  0.0f,  -0.5f),
		glm::vec3(-0.5f, 0.0f,  0.0f),
		glm::vec3(0.0f, 0.5f,  0.0f),
		glm::vec3(0.0f, -0.5f,  0.0f)
	};

	glm::float32 planeRotations[] = {
		0.0f, 90.0f, 0.0f, 90.0f, 90.f, 90.f
	};

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GLuint cubeVBO, cubeEBO, cubeVAO, floorVBO, floorEBO, floorVAO;

	glGenBuffers(1, &cubeVBO); // Create VBO
	glGenBuffers(1, &cubeEBO); // Create EBO

	glGenBuffers(1, &floorVBO); // Create VBO
	glGenBuffers(1, &floorEBO); // Create EBO

	glGenVertexArrays(1, &cubeVAO); // Create VOA
	glGenVertexArrays(1, &floorVAO); // Create VOA

	glBindVertexArray(cubeVAO);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, cubeVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 

																					 // Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)


	glBindVertexArray(floorVAO);

	glBindBuffer(GL_ARRAY_BUFFER, floorVBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO); // Select EBO

	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	// Vertex shader source code
	string vertexShaderSource =
		"#version 330 core\n"
		"layout(location = 0) in vec4 vPosition;"
		"layout(location = 1) in vec4 aColor;"
		"out vec4 oColor;"
		"uniform mat4 model;"
		"uniform mat4 view;"
		"uniform mat4 projection;"
		"void main()\n"
		"{\n"
		"gl_Position = projection * view * model * vPosition;"
		"oColor = aColor;"
		"}\n";

	// Fragment shader source code
	string fragmentShaderSource =
		"#version 330 core\n"
		"in vec4 oColor;"
		"out vec4 fragColor;"
		"void main()\n"
		"{\n"
		"fragColor = oColor;"
		"}\n";

	// Creating Shader Program
	GLuint shaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);


	/* Loop until the user closes the window */
	while (!glfwWindowShouldClose(window))
	{
		// Set frame time
		GLfloat currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Resize window and graphics simultaneously
		glfwGetFramebufferSize(window, &width, &height);
		glViewport(0, 0, width, height);

		/* Render here */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use Shader Program exe and select VAO before drawing 
		glUseProgram(shaderProgram); // Call Shader per-frame when updating attributes


		// Declare transformations (can be initialized outside loop)		
		glm::mat4 projectionMatrix;

		// Define LookAt Matrix
		viewMatrix = glm::lookAt(cameraPosition, target, worldUp);

		// Define projection matrix
		projectionMatrix = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		// Get matrix's uniform location and set matrix
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

		
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


		glBindVertexArray(cubeVAO); // User-defined VAO must be called before draw. 

		// Transform planes to form cube
		for (GLuint i = 0; i < 6; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, planePositions[i]);
			modelMatrix = glm::rotate(modelMatrix, planeRotations[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			if (i >= 4)
				modelMatrix = glm::rotate(modelMatrix, planeRotations[i] * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw();
		}

		// Unbind Shader exe and VOA after drawing per frame
		glBindVertexArray(0); //Incase different VAO wii be used after

		// Select and transform floor
		glBindVertexArray(floorVAO);
		glm::mat4 modelMatrix;
		modelMatrix = glm::translate(modelMatrix, glm::vec3(0.f, -.5f, 0.f));
		modelMatrix = glm::rotate(modelMatrix, 90.f * toRadians, glm::vec3(1.0f, 0.0f, 0.0f));
		modelMatrix = glm::scale(modelMatrix, glm::vec3(5.f, 5.f, 5.f));
		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		draw();
		glBindVertexArray(0); //Incase different VAO will be used after



		glUseProgram(0); // Incase different shader will be used after

		/* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		// Poll Camera Transformations
		TransformCamera();

	}

	//Clear GPU resources
	glDeleteVertexArrays(1, &cubeVAO);
	glDeleteBuffers(1, &cubeVBO);
	glDeleteBuffers(1, &cubeEBO);
	glDeleteVertexArrays(1, &floorVAO);
	glDeleteBuffers(1, &floorVBO);
	glDeleteBuffers(1, &floorEBO);


	glfwTerminate();
	return 0;
}

// Define input functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// Display ASCII Key code
	//std::cout <<"ASCII: "<< key << std::endl;	

	// Close window
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	// Assign true to Element ASCII if key pressed
	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE) // Assign false to Element ASCII if key released
		keys[key] = false;

}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{

	// Clamp FOV
	if (fov >= 1.0f && fov <= 55.0f)
		fov -= yoffset * 0.01;

	// Default FOV
	if (fov < 1.0f)
		fov = 1.0f;
	if (fov > 55.0f)
		fov = 55.0f;

}
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{

	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}
	// Calculate mouse offset (Easing effect)
	xChange = xpos - lastX;
	yChange = lastY - ypos; // Inverted cam

							// Get current mouse (always starts at 0)
	lastX = xpos;
	lastY = ypos;


	if (isOrbiting)
	{
		// Update raw yaw and pitch with mouse movement
		rawYaw += xChange;
		rawPitch += yChange;

		// Conver yaw and pitch to degrees, and clamp pitch
		degYaw = glm::radians(rawYaw);
		degPitch = glm::clamp(glm::radians(rawPitch), -glm::pi<float>() / 2.f + .1f, glm::pi<float>() / 2.f - .1f);

		// Azimuth Altitude formula
		cameraPosition.x = target.x + radius * cosf(degPitch) * sinf(degYaw);
		cameraPosition.y = target.y + radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degPitch) * cosf(degYaw);
	}
}
void mouse_button_callback(GLFWwindow* window, int button, int action, int mode)
{
	// Assign boolean state to element Button code
	if (action == GLFW_PRESS)
		mouseButtons[button] = true;
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}



// Define TransformCamera function
void TransformCamera()
{

	// Orbit camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
		isOrbiting = true;
	else
		isOrbiting = false;

	// Focus camera
	if (keys[GLFW_KEY_F])
		initiateCamera();
}

// Define 
void initiateCamera()
{	// Define Camera Attributes
	cameraPosition = glm::vec3(0.0f, 0.0f, 3.0f); // Move 3 units back in z towards screen
	target = glm::vec3(0.0f, 0.0f, 0.0f); // What the camera points to
	cameraDirection = glm::normalize(cameraPosition - cameraDirection); // direction z
	worldUp = glm::vec3(0.0, 1.0f, 0.0f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));// right vector x
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight)); // up vector y
	CameraFront = glm::vec3(0.0f, 0.0f, -1.0f); // 1 unit away from lense
}
