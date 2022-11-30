#include "Demo.h"
#include <vector>
#include <string>
#include <iostream>



Demo::Demo() {

}


Demo::~Demo() {
}



void Demo::Init() {
	// build and compile our shader program
	// ------------------------------------
	//shaderProgram = BuildShader("vertexShader.vert", "fragmentShader.frag", nullptr);
	skyBoxProgram = BuildShader("skyBoxShader.vert", "skyBoxFragment.frag", nullptr);

	BuildDepthMap();
	BuildShaders();

	BuildColoredCube();

	BuildColoredPlane();

	BuildSkyBox();
}

void Demo::BuildDepthMap() {
	// configure depth map FBO
	// -----------------------
	glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	glGenTextures(1, &depthMap);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, this->SHADOW_WIDTH, this->SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

	// attach depth texture as FBO's depth buffer
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Demo::BuildShaders()
{
	// build and compile our shader program
	// ------------------------------------
	shadowmapShader = BuildShader("shadowMapping.vert", "shadowMapping.frag", nullptr);
	depthmapShader = BuildShader("depthMap.vert", "depthMap.frag", nullptr);
}

void Demo::DeInit() {
	// optional: de-allocate all resources once they've outlived their purpose:
	// ------------------------------------------------------------------------
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteBuffers(1, &VBO2);
	glDeleteBuffers(1, &EBO2);
	glDeleteBuffers(1, &depthMapFBO);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void Demo::ProcessInput(GLFWwindow *window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	}

	
}

void Demo::Update(double deltaTime) {
	angle += (float) ((deltaTime * 1.5f) / 1000);


}



//Camera Position
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

//Mouse Position
double lastX = 0, lastY = 0;
bool firstMouse = true;
float yaw;
float pitch;
glm::vec3 direction;



void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	double xoffset = xpos - lastX;
	double yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;

	float sensitivity = 0.1f;
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	
	direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(pitch));
	direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);
}

void Demo::Render() {

	glClearColor(0.98f, 0.839f, 0.647f, 1.0f);
	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	// Step 1 Render depth of scene to texture
	// ----------------------------------------
	glm::mat4 lightProjection, lightView;
	glm::mat4 lightSpaceMatrix;
	float near_plane = 1.0f, far_plane = 7.5f;
	//lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	lightProjection = glm::ortho(1.0f, 0.0f, 1.0f, 1.0f, near_plane, far_plane);
	//lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0, 1.0, 0.0));
	lightView = glm::lookAt(glm::vec3(0.0f, 4.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0, 1.0, 0.0));
	lightSpaceMatrix = lightProjection * lightView;
	// render scene from light's point of view
	UseShader(this->depthmapShader);
	glUniformMatrix4fv(glGetUniformLocation(this->depthmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	glViewport(0, 0, this->SHADOW_WIDTH, this->SHADOW_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	DrawColoredCube(this->depthmapShader);
	DrawColoredPlane(this->depthmapShader);
	DrawBridge(this->depthmapShader);
	DrawWeapon(this->depthmapShader);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Step 2 Render scene normally using generated depth map
	// ------------------------------------------------------
	glViewport(0, 0, this->screenWidth, this->screenHeight);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//glClearColor(0.98f, 0.839f, 0.647f, 1.0f);

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);

;	// Pass perspective projection matrix
	UseShader(this->shadowmapShader);

	// Pass perspective projection matrix
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)this->screenWidth / (GLfloat)this->screenHeight, 0.1f, 100.0f);
	GLint projLoc = glGetUniformLocation(this->shadowmapShader, "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

	// LookAt camera (position, target/direction, up)
	const float cameraSpeed = 0.01f; // adjust accordingly
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		cameraPos += cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		cameraPos -= cameraSpeed * cameraFront;
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
	//glm::mat4 view = glm::lookAt(glm::vec3(posX, posY, posZ), glm::vec3(dirX, dirY, dirZ), glm::vec3(upX, upY, upZ));
	//glm::mat4 view = glm::lookAt(glm::vec3(-2, 3, 10), glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
	glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "view"), 1, GL_FALSE, glm::value_ptr(view));


	//GLint viewLoc = glGetUniformLocation(this->shadowmapShader, "view");
	//glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

	//Mouse Movement
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, mouse_callback);

	//diffuse lighting
	// set lighting attribute
	//GLint lightPosLoc = glGetUniformLocation(this->shaderProgram, "lightPos");
	//glUniform3f(lightPosLoc, 0, 1, 0);
	//GLint viewPosLoc = glGetUniformLocation(this->shaderProgram, "viewPos");
	//glUniform3f(viewPosLoc, 0, 2, 3);
	//GLint lightColorLoc = glGetUniformLocation(this->shaderProgram, "lightColor");
	//glUniform3f(lightColorLoc, 1.0f, 1.0f, 1.0f);

	// Setting Light Attributes
	glUniformMatrix4fv(glGetUniformLocation(this->shadowmapShader, "lightSpaceMatrix"), 1, GL_FALSE, glm::value_ptr(lightSpaceMatrix));
	//glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "viewPos"), 5.0f, 20.0f, 0.0f);
	//glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), -2.0f, 4.0f, -1.0f);
	glUniform3f(glGetUniformLocation(this->shadowmapShader, "lightPos"), 10.0f, 10.0f, -1.0f);

	// Configure Shaders
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "diffuseTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "shadowMap"), 1);

	// Render floor
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredPlane(this->shadowmapShader);

	// Render wall
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawColoredCube(this->shadowmapShader);

	//Render bridge
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureBridge);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawBridge(this->shadowmapShader);

	//Render weapon
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	DrawWeapon(this->shadowmapShader);
	
	
	//DrawColoredCube();

	//DrawColoredPlane();

	//DrawWeapon();

	DrawCrossHair();

	//DrawBridge();

	glDisable(GL_DEPTH_TEST);
}




void Demo::BuildSkyBox() {
	std::vector<std::string> faces
	{
			"right.jpg",
			"left.jpg",
			"top.jpg",
			"bottom.jpg",
			"front.jpg",
			"back.jpg"
	};

	unsigned int textureID;
	glGenTextures(1, &textureSkyBox);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureSkyBox);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = SOIL_load_image(faces[i].c_str(), &width, &height, &nrChannels, SOIL_LOAD_RGBA);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			SOIL_free_image_data(data);
		}
		else
		{
			std::cout << "Cubemap tex failed to load at path: " << faces[i] << std::endl;
			SOIL_free_image_data(data);
		}
	}
	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	float skyboxVertices[] = {
		// positions          
		-1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f, -1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,

		-1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f,
		-1.0f, -1.0f,  1.0f,

		-1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f, -1.0f,
		 1.0f,  1.0f,  1.0f,
		 1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f,

		-1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f,
		 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f,  1.0f
	};

	//return textureID;
}

void Demo::DrawSkyBox() {
	glUseProgram(skyBoxProgram);
	//glDepthMask(GL_FALSE);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureSkyBox);
	glUniform1i(glGetUniformLocation(this->skyBoxProgram, "skybox"), 0);

	glBindVertexArray(skyboxVAO);

	glm::mat4 model;
	GLint modelLoc = glGetUniformLocation(this->skyBoxProgram, "view");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	
	//skyboxShader.use();
	// ... set view and projection matrix
	//glBindVertexArray(skyboxVAO);
	//glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glDepthMask(GL_TRUE);
	// ... draw rest of the scene

	glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
	glBindVertexArray(0);
}



void Demo::BuildColoredCube() {
	// load image into texture memory
	// ------------------------------
	// Load and create a texture 

	int width, height;

	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* image = SOIL_load_image("metal.png", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
	SOIL_free_image_data(image);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &textureWall);
	glBindTexture(GL_TEXTURE_2D, textureWall);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* imageWall = SOIL_load_image("ground4.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageWall);
	SOIL_free_image_data(imageWall);
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenTextures(1, &textureBridge);
	glBindTexture(GL_TEXTURE_2D, textureBridge);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	unsigned char* imageBridge = SOIL_load_image("orange-tile.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageBridge);
	SOIL_free_image_data(imageBridge);
	glBindTexture(GL_TEXTURE_2D, 0);

	// set up vertex data (and buffer(s)) and configure vertex attributes
	// ------------------------------------------------------------------
	float vertices[] = {
		// format position, tex coords
		//front
		-0.5, -0.5, 0.5, 0, 0, 0.0f,  0.0f,  1.0f, // 0
		0.5, -0.5, 0.5, 1, 0,  0.0f,  0.0f,  1.0f, // 1
		0.5,  0.5, 0.5, 1, 1,  0.0f,  0.0f,  1.0f, // 2
		-0.5,  0.5, 0.5, 0, 1, 0.0f,  0.0f,  1.0f, // 3

		// right
		0.5,  0.5,  0.5, 0, 0, 1.0f,  0.0f,  0.0f, // 4
		0.5,  0.5, -0.5, 1, 0, 1.0f,  0.0f,  0.0f, // 5
		0.5, -0.5, -0.5, 1, 1, 1.0f,  0.0f,  0.0f, // 6
		0.5, -0.5,  0.5, 0, 1, 1.0f,  0.0f,  0.0f, // 7

		// back
		-0.5, -0.5, -0.5, 0, 0, 0.0f,  0.0f,  -1.0f, // 8 
		0.5,  -0.5, -0.5, 1, 0, 0.0f,  0.0f,  -1.0f, // 9
		0.5,   0.5, -0.5, 1, 1, 0.0f,  0.0f,  -1.0f, // 10
		-0.5,  0.5, -0.5, 0, 1, 0.0f,  0.0f,  -1.0f, // 11

		// left
		-0.5, -0.5, -0.5, 0, 0, -1.0f,  0.0f,  0.0f, // 12
		-0.5, -0.5,  0.5, 1, 0, -1.0f,  0.0f,  0.0f, // 13
		-0.5,  0.5,  0.5, 1, 1, -1.0f,  0.0f,  0.0f, // 14
		-0.5,  0.5, -0.5, 0, 1, -1.0f,  0.0f,  0.0f, // 15

		// upper
		0.5, 0.5,  0.5, 0, 0,   0.0f,  1.0f,  0.0f, // 16
		-0.5, 0.5, 0.5, 1, 0,   0.0f,  1.0f,  0.0f, // 17
		-0.5, 0.5, -0.5, 1, 1,  0.0f,  1.0f,  0.0f, // 18
		0.5, 0.5, -0.5, 0, 1,   0.0f,  1.0f,  0.0f, // 19

		// bottom
		-0.5, -0.5, -0.5, 0, 0, 0.0f,  -1.0f,  0.0f, // 20
		0.5, -0.5, -0.5, 1, 0,  0.0f,  -1.0f,  0.0f, // 21
		0.5, -0.5,  0.5, 1, 1,  0.0f,  -1.0f,  0.0f, // 22
		-0.5, -0.5,  0.5, 0, 1, 0.0f,  -1.0f,  0.0f, // 23
	};

	unsigned int indices[] = {
		0,  1,  2,  0,  2,  3,   // front
		4,  5,  6,  4,  6,  7,   // right
		8,  9,  10, 8,  10, 11,  // back
		12, 14, 13, 12, 15, 14,  // left
		16, 18, 17, 16, 19, 18,  // upper
		20, 22, 21, 20, 23, 22   // bottom
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	// bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// define position pointer layout 0
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(0 * sizeof(GLfloat)));
	glEnableVertexAttribArray(0);

	//diffuse lighting
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	//glEnableVertexAttribArray(0);

	// define texcoord pointer layout 1
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	// note that this is allowed, the call to glVertexAttribPointer registered VBO as the vertex attribute's bound vertex buffer object so afterwards we can safely unbind
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
	// VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
	glBindVertexArray(0);

	// remember: do NOT unbind the EBO while a VAO is active as the bound element buffer object IS stored in the VAO; keep the EBO bound.
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

}


void Demo::DrawColoredCube(GLuint shader)
{
	UseShader(shader);
	//glUseProgram(shaderProgram);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, textureWall);
	//glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 0);

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized


	// Wall Build
	glm::mat4 wall1;
	wall1 = glm::translate(wall1, glm::vec3(10, 1, 0));
	//model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	wall1 = glm::scale(wall1, glm::vec3(1, 3, 10));

	//GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wall1));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 wall2;
	wall2 = glm::translate(wall2, glm::vec3(20, 1, 0));
	//model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	wall2 = glm::scale(wall2, glm::vec3(10, 3, 1));

	//glGetUniformLocation(this->shaderProgram, "model");
	glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(wall2));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 box1;
	box1 = glm::translate(box1, glm::vec3(20, 1, 6));
	//model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	box1 = glm::scale(box1, glm::vec3(3, 3, 3));

	//glGetUniformLocation(this->shaderProgram, "model");
	glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(box1));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawBridge(GLuint shader) {
	//glUseProgram(shaderProgram);
	UseShader(shader);
	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, textureBridge);
	//glUniform1i(glGetUniformLocation(this->shader, "ourTexture"), 0);
	//glUniform1i(glGetUniformLocation(shader, "ourTexture"), 0);

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized


	glm::mat4 bridge1;
	bridge1 = glm::translate(bridge1, glm::vec3(20, 3, 2.5));
	//model = glm::rotate(model, angle, glm::vec3(0, 1, 0));
	bridge1 = glm::scale(bridge1, glm::vec3(3, 1, 10));

	//GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bridge1));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 bridge2;
	bridge2 = glm::translate(bridge2, glm::vec3(15.2, 1.5, 6));
	bridge2 = glm::rotate(bridge2, glm::radians(30.0f), glm::vec3(0, 0, 1));
	bridge2 = glm::scale(bridge2, glm::vec3(7.5, 0.1, 3));

	//glGetUniformLocation(this->shaderProgram, "model");
	glUniform1i(glGetUniformLocation(shader, "model"), 0);
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bridge2));

	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawWeapon(GLuint shader) {

	//glUseProgram(shaderProgram);
	UseShader(shader);

	//glActiveTexture(GL_TEXTURE0);
	//glBindTexture(GL_TEXTURE_2D, texture);
	//glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 0);

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized


	//Weapon Position
	glm::vec3 weaponPos = glm::vec3(cameraPos.x + 0.1, cameraPos.y - 0.1, cameraPos.z - 0.5);


	glm::mat4 model6;
	model6 = glm::translate(model6, glm::vec3(direction + cameraPos));
	model6 = glm::rotate(model6, glm::radians(yaw), glm::vec3(0, -0.5, 0));
	model6 = glm::rotate(model6, glm::radians(pitch), glm::vec3(0, 0, 0.5));
	model6 = glm::rotate(model6, glm::radians(10.0f), glm::vec3(0, 0.5, 0));
	model6 = glm::translate(model6, glm::vec3(-0.5, -0.1, 0.1));

	model6 = glm::scale(model6, glm::vec3(1, 0.1, 0.1));

	//GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
	GLint modelLoc = glGetUniformLocation(shader, "model");;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model6));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 model7;
	model7 = glm::translate(model7, glm::vec3(direction + cameraPos));
	model7 = glm::rotate(model7, glm::radians(yaw), glm::vec3(0, -0.5, 0));
	model7 = glm::rotate(model7, glm::radians(pitch), glm::vec3(0, 0, 0.5));
	model7 = glm::rotate(model7, glm::radians(10.0f), glm::vec3(0, 0.5, 0));
	model7 = glm::translate(model7, glm::vec3(-0.6, -0.2, 0.1));

	model7 = glm::scale(model7, glm::vec3(0.1, 0.2, 0.1));

	//glGetUniformLocation(this->shaderProgram, "model");
	glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model7));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::DrawCrossHair() {
	//glUseProgram(shaderProgram);
	UseShader(shadowmapShader);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, depthMap);
	//glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 0);
	glUniform1i(glGetUniformLocation(this->shadowmapShader, "model"), 0);

	glBindVertexArray(VAO); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized


	glm::mat4 crosshair1;
	crosshair1 = glm::translate(crosshair1, glm::vec3(direction + cameraPos));
	crosshair1 = glm::rotate(crosshair1, glm::radians(yaw), glm::vec3(0, -0.5, 0));
	crosshair1 = glm::rotate(crosshair1, glm::radians(pitch), glm::vec3(0, 0, 0.5));
	crosshair1 = glm::rotate(crosshair1, glm::radians(10.0f), glm::vec3(0, 0.5, 0));
	//crosshair = glm::translate(crosshair, glm::vec3(0.5, -0.1, 0.1));

	crosshair1 = glm::scale(crosshair1, glm::vec3(0.005, 0.005, 0.04));

	//GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");;
	GLint modelLoc = glGetUniformLocation(this->shadowmapShader, "model");;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(crosshair1));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glm::mat4 crosshair2;
	crosshair2 = glm::translate(crosshair2, glm::vec3(direction + cameraPos));
	crosshair2 = glm::rotate(crosshair2, glm::radians(yaw), glm::vec3(0, -0.5, 0));
	crosshair2 = glm::rotate(crosshair2, glm::radians(pitch), glm::vec3(0, 0, 0.5));
	crosshair2 = glm::rotate(crosshair2, glm::radians(10.0f), glm::vec3(0, 0.5, 0));
	//crosshair = glm::translate(crosshair, glm::vec3(0.5, -0.1, 0.1));

	crosshair2 = glm::scale(crosshair2, glm::vec3(0.005, 0.04, 0.005));

	//glGetUniformLocation(this->shaderProgram, "model");;
	glGetUniformLocation(this->shadowmapShader, "model");;
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(crosshair2));
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}

void Demo::BuildColoredPlane()
{
	// Load and create a texture 
	glGenTextures(1, &texture2);
	glBindTexture(GL_TEXTURE_2D, texture2);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	int width, height;
	unsigned char* imageFloor = SOIL_load_image("ground3.jpg", &width, &height, 0, SOIL_LOAD_RGBA);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageFloor);
	glGenerateMipmap(GL_TEXTURE_2D);
	SOIL_free_image_data(imageFloor);
	glBindTexture(GL_TEXTURE_2D, 0);

	// Build geometry
	//GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		//-50.0, -0.5, -50.0,  0,  0,
		// 50.0, -0.5, -50.0, 50,  0,
		// 50.0, -0.5,  50.0, 50, 50,
		//-50.0, -0.5,  50.0,  0, 50,


	//};


	// Build geometry
	GLfloat vertices[] = {
		// format position, tex coords
		// bottom
		-50.0f,	-0.5f, -50.0f,  0,  0, 0.0f,  1.0f,  0.0f,
		50.0f,	-0.5f, -50.0f, 50,  0, 0.0f,  1.0f,  0.0f,
		50.0f,	-0.5f,  50.0f, 50, 50, 0.0f,  1.0f,  0.0f,
		-50.0f,	-0.5f,  50.0f,  0, 50, 0.0f,  1.0f,  0.0f,
	};

	GLuint indices[] = { 0,  2,  1,  0,  3,  2 };

	glGenVertexArrays(1, &VAO2);
	glGenBuffers(1, &VBO2);
	glGenBuffers(1, &EBO2);

	glBindVertexArray(VAO2);

	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO2);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	// Position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), 0);
	glEnableVertexAttribArray(0);
	// TexCoord attribute
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(1);

	// define normal pointer layout 2
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (GLvoid*)(5 * sizeof(GLfloat)));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0); // Unbind VAO
}


void Demo::DrawColoredPlane(GLuint shader)
{
	//glUseProgram(shaderProgram);
	UseShader(shader);

	//glActiveTexture(GL_TEXTURE1);
	//glBindTexture(GL_TEXTURE_2D, texture2);
	//glUniform1i(glGetUniformLocation(this->shaderProgram, "ourTexture"), 1);

	glBindVertexArray(VAO2); // seeing as we only have a single VAO there's no need to bind it every time, but we'll do so to keep things a bit more organized

	glm::mat4 model;
	//GLint modelLoc = glGetUniformLocation(this->shaderProgram, "model");
	GLint modelLoc = glGetUniformLocation(shader, "model");
	glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);
}




int main(int argc, char** argv) {
	RenderEngine &app = Demo();
	app.Start("Proyek Akhir", 800, 600, false, false);
}