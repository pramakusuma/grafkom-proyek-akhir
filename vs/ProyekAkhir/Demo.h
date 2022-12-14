#pragma once
#include "RenderEngine.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/vector_angle.hpp>
#include <SOIL/SOIL.h>

class Demo :
	public RenderEngine
{
public:
	Demo();
	~Demo();
private:
	GLuint depthmapShader, shadowmapShader, shaderProgram, skyBoxProgram, depthMapFBO, VBO, VAO, skyboxVAO, EBO, texture, VBO2, VAO2, EBO2, depthMap, shadowMapping, textureSkyBox, texture2, textureWall, textureBridge, textureTarget, lightPos;
	float angle = 0;
	const unsigned int SHADOW_WIDTH = 1024, SHADOW_HEIGHT = 1024;
	virtual void Init();
	virtual void DeInit();
	virtual void Update(double deltaTime);
	virtual void Render();
	virtual void ProcessInput(GLFWwindow *window);
	void BuildColoredCube();
	void BuildColoredPlane();
	void BuildSkyBox();
	void BuildDepthMap();
	void BuildShaders();
	void DrawBox(GLuint shader, float posx, float posy, float posz);
	void DrawWallHorizontal(GLuint shader, float posx, float posy, float posz);
	void DrawWallVertical(GLuint shader, float posx, float posy, float posz);
	void DrawColoredPlane(GLuint shader);
	void DrawWeapon(GLuint shader);
	void DrawCrossHair();
	void DrawBridge(GLuint shader, float posx, float posy);
	void DrawStair(GLuint shader, float posx, float posy);
	void DrawTarget(GLuint shader, float posx, float posy, float posz, float rotate);
	void DrawSkyBox();
};

