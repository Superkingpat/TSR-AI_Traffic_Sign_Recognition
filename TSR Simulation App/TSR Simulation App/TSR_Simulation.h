#pragma once

#include "ObjectHandler.h"

class TSR_Simulation {
private:
	uint32_t M_SCR_WIDTH;
	uint32_t M_SCR_HEIGHT;

	GLFWwindow* m_window = nullptr;

	ShaderHandler m_shaderHandler;

	ObjectHandler m_objectHandler;

	CameraHandler m_cameraHandler;

	void Init();
	void InitCamera();
	void InitGLFW();
	void InitOpenGL();
	void InitRenderObjects();
	void InitBuffers();
	void InitShaders();

	void Update();

	void Draw();
	void DrawCubemap();
public:

	TSR_Simulation();

	int Run();
};
