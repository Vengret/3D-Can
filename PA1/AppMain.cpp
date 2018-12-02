#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>

// GLM Mathematics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

const double PI = 3.14159;
const float toRadians = PI / 180.0f;
int width = 640, height = 480;

// input function prototypes
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

// declare init camera
void initCamera();

// declare view matrix
glm::mat4 viewMatrix;

// initialize FOV
GLfloat fov = 45.f;

// define camera attributes
glm::vec3 cameraPosition = glm::vec3(0.f, 0.f, 3.f);
glm::vec3 target = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 cameraDirection = glm::normalize(cameraPosition - target);
glm::vec3 worldUp = glm::vec3(0.f, 1.f, 0.f);
glm::vec3 cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
glm::vec3 cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
glm::vec3 cameraFront = glm::normalize(glm::vec3(0.f, 0.f, -1.f));

// declare target prototype
glm::vec3 getTarget();

// Camera transformation prototype
void transformCamera();

// boolean for keys and mouse buttons
bool keys[1024], mouseButtons[2];

// Boolean to check camera transformations
bool isPanning = false, isOrbiting = false;

// Radius, Pitch, and Yaw
GLfloat radius = 3.f, rawYaw = 0.f, rawPitch = 0.f, degYaw, degPitch;

// variable used to ensure program runs the same of different computers
GLfloat deltaTime = 0.f, lastFrame = 0.f;
GLfloat lastX = width / 2, lastY = height / 2, xChange, yChange;
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

	// Set callback functions
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);
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
		glm::vec3(-0.5f, 0.0f,  0.0f)
	};

	glm::float32 planeRotations[] = {
		0.0f, 90.0f, 0.0f, 90.0f
	};

	// Setup some OpenGL options
	glEnable(GL_DEPTH_TEST);

	// Wireframe mode
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	GLuint VBO, EBO, VAO;

	glGenBuffers(1, &VBO); // Create VBO
	glGenBuffers(1, &EBO); // Create EBO

	glGenVertexArrays(1, &VAO); // Create VOA
	glBindVertexArray(VAO);

	// VBO and EBO Placed in User-Defined VAO
	glBindBuffer(GL_ARRAY_BUFFER, VBO); // Select VBO
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO); // Select EBO


	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW); // Load vertex attributes
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW); // Load indices 

																					 // Specify attribute location and layout to GPU
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	glBindVertexArray(0); // Unbind VOA or close off (Must call VOA explicitly in loop)

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
		// set delta time
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

		viewMatrix = glm::lookAt(cameraPosition, getTarget(), worldUp);

		projectionMatrix = glm::perspective(fov, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);

		// Get matrix's uniform location and set matrix
		GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
		GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
		GLint projLoc = glGetUniformLocation(shaderProgram, "projection");

		//glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
		glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(viewMatrix));
		glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projectionMatrix));


		glBindVertexArray(VAO); // User-defined VAO must be called before draw. 

		for (GLuint i = 0; i < 4; i++)
		{
			glm::mat4 modelMatrix;
			modelMatrix = glm::translate(modelMatrix, planePositions[i]);
			modelMatrix = glm::rotate(modelMatrix, planeRotations[i] * toRadians, glm::vec3(0.0f, 1.0f, 0.0f));
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(modelMatrix));
			// Draw primitive(s)
			draw();
		}

		// Unbind Shader exe and VOA after drawing per frame
		glBindVertexArray(0); //Incase different VAO wii be used after
		glUseProgram(0); // Incase different shader will be used after

	    /* Swap front and back buffers */
		glfwSwapBuffers(window);

		/* Poll for and process events */
		glfwPollEvents();

		// Poll camera transformation
		transformCamera();
	}

	//Clear GPU resources
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);



	glfwTerminate();
	return 0;
}

// Define input callback functions
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	// assign keys as true if pressed and false when released
	if (action == GLFW_PRESS)
	{
		keys[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		keys[key] = false;
	}	
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	// clamp fov
	if (fov >= 1.f && fov <= 55.f)
		fov -= yoffset * 0.02;
	
	// default fov (keep between 1 and 55
	if (fov < 1.f)
		fov = 1.f;
	if (fov > 55)
		fov = 55.f;
}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouseMove)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouseMove = false;
	}

	// calculate cursor offset
	xChange = xpos - lastX;
	yChange = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	// Pan camera
	if (isPanning)
	{
		GLfloat cameraSpeed = xChange * deltaTime;
		cameraPosition -= cameraSpeed * cameraRight;
		target.x -= cameraSpeed;

		cameraSpeed = yChange * deltaTime;
		cameraPosition -= cameraSpeed * cameraUp;
		target.y -= cameraSpeed;
			
	}

	// Orbit camera
	if (isOrbiting)
	{
		rawYaw += xChange;
		rawPitch += yChange;

		// convert to degrees
		degYaw = glm::radians(rawYaw);
		//degPitch = glm::radians(rawPitch);
		degPitch = glm::clamp(glm::radians(rawPitch), -glm::pi<float>() / 2.f + .1f, glm::pi<float>() / 2.f - .1f);

		// Azimuth altitude formula
		cameraPosition.x = target.x - radius * cosf(degPitch) * sinf(degYaw);
		cameraPosition.y = target.y - radius * sinf(degPitch);
		cameraPosition.z = target.z + radius * cosf(degYaw)  * cosf(degPitch);

	}
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	// assign button as true when pressed and false when released
	if (action == GLFW_PRESS)
	{
		mouseButtons[button] = true;
	}
	else if (action == GLFW_RELEASE)
		mouseButtons[button] = false;
}

// define get target function
glm::vec3 getTarget()
{
	//if (isPanning) {
	//	target = cameraPosition + cameraFront;
	//}

	return target;
}

// define transform camera
void transformCamera()
{
	// Pan camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_RIGHT])
		isPanning = true;
	else
		isPanning = false;

	// Orbit camera
	if (keys[GLFW_KEY_LEFT_ALT] && mouseButtons[GLFW_MOUSE_BUTTON_LEFT])
		isOrbiting = true;
	else
		isOrbiting = false;

	// reset camera
	if (keys[GLFW_KEY_F])
		initCamera();
}

// define init camera
void initCamera()
{
	cameraPosition = glm::vec3(0.f, 0.f, 3.f);
	target = glm::vec3(0.f, 0.f, 0.f);
	cameraDirection = glm::normalize(cameraPosition - target);
	worldUp = glm::vec3(0.f, 1.f, 0.f);
	cameraRight = glm::normalize(glm::cross(worldUp, cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
	cameraFront = glm::normalize(glm::vec3(0.f, 0.f, -1.f));
}