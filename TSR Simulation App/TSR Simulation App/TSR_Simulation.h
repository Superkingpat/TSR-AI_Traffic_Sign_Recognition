#pragma once
#include "ObjectHandler.h"

class TSR_Simulation {
private:

	struct BufferContainer {
		GLuint materialUBO = -1;
		GLuint lightsUBO = -1;

		GLuint pickingFBO = -1;
		GLuint pickingTexture = -1; 
		GLuint pickingDepthRBO = -1;

		GLuint cubemapVAO = 0;
		GLuint cubemapVBO = 0;
		Texture cubemapTexture;
	} buffers;

	struct Light {
		float FalloffStart = 2.5f;
		float FalloffEnd = 5.f;
		int Type = -1;
		float padding4;

		glm::vec3 Strength = glm::vec3(1.f, 0.f, 0.f);
		float padding1;

		glm::vec3 Direction = glm::vec3(1.f, 1.f, 1.f);
		float padding2;

		glm::vec3 Position = glm::vec3(0.5f, 0.5f, 0.5f);
		float padding3;
	};

	uint32_t M_SCR_WIDTH;
	uint32_t M_SCR_HEIGHT;

	GLFWwindow* m_window = nullptr;

	ShaderHandler m_shaderHandler;

	ObjectHandler m_objectHandler;

	CameraHandler m_cameraHandler;

	std::vector<float> m_cubemapFaces = {       
		-1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
		 1.0f, -1.0f, -1.0f, 1.0f, -1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f, -1.0f, 1.0f, -1.0f, -1.0f,
		-1.0f, -1.0f,  1.0f, -1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, 1.0f, -1.0f,  1.0f, -1.0f, -1.0f,  1.0f,
		-1.0f,  1.0f, -1.0f, 1.0f,  1.0f, -1.0f, 1.0f,  1.0f,  1.0f, 1.0f,  1.0f,  1.0f, -1.0f,  1.0f,  1.0f, -1.0f,  1.0f, -1.0f,
		-1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f,  1.0f, 1.0f, -1.0f,  1.0f
	};

	int m_pickedObjectIndex = 0;
	//std::shared_ptr<std::vector<WorldData>> m_pickedObjectWorldDataVec;
	std::shared_ptr<RenderObject> m_pickedRenderObject;

	std::vector<Light> m_lights;
	const uint32_t M_MAX_NUM_OF_LIGHTS = 20;
	glm::vec3 m_ambientColor = glm::vec3(0.5f, 0.5f, 0.5f);

	void Init();
	void InitCamera();
	void InitGLFW();
	void InitOpenGL();
	void InitRenderObjects();
	void InitCubemap();
	void InitCubemapTextures();
	void InitCubemapBuffers();
	void InitLights();

	void InitBuffers();
	void InitMaterialsBuffers();
	void InitLightBuffers();
	void InitPickingBuffers();

	void InitShaders();

	void Update();
	void InputUpdate();

	void Draw();
	void PickingDrawPass();
	void ObjectDrawPass();
	void ObjectDrawPassTextured(std::shared_ptr<RenderObject>& obj);
	void ObjectDrawPassUntextured(std::shared_ptr<RenderObject>& obj);
	void OutlineDrawPass();
	void CubemapDrawPass();
public:

	TSR_Simulation();
	~TSR_Simulation();

	int Run();
};
