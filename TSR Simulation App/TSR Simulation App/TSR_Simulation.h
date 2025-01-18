#pragma once
#include "ComunicationHandler.h"

enum class CameraType {
	OUTSIDE_CAMERA,
	INSIDE_CAMERA
};

class TSR_Simulation {
private:

	struct BufferContainer {
		GLuint materialUBO = -1;
		GLuint lightsUBO = -1;

		GLuint pickingFBO = -1;
		GLuint pickingTexture = -1; 
		GLuint pickingDepthRBO = -1;

		GLuint secondViewFBO = -1;
		GLuint secondViewTexture = -1;
		GLuint secondViewDepthRBO = -1;

		GLuint cubemapVAO = 0;
		GLuint cubemapVBO = 0;
		TextureCbMp cubemapTexture;

		GLuint shadowMapFBO = 0;
		GLuint shadowMapTexture;

		RenderObject waterObject;
		uint32_t indexCount;
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
	uint32_t M_CAR_SCR_WIDTH;
	uint32_t M_CAR_SCR_HEIGHT;
	uint32_t M_SHADOW_SCR_WIDTH;
	uint32_t M_SHADOW_SCR_HEIGHT;

	GLFWwindow* m_window = nullptr;

	ShaderHandler m_shaderHandler;

	ObjectHandler m_objectHandler;

	CameraHandler m_cameraHandlerOuter;
	CameraHandler m_cameraHandlerInner;
	CameraHandler m_cameraHandlerShadow;

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
	glm::vec3 m_ambientColor;

	Timer m_timer;

	Waves m_water = Waves(128, 128, 1.0f, 0.03f, 4.0f, 0.2f);
	int buffersize;

	double m_lastMouseX = 0.0, m_lastMouseY = 0.0;
	bool m_firstMouse = true;
	float t_base = 0.0f;

	bool m_imageCapture = false;
	float m_imageCaptureInterval = 1.f;
	uint32_t m_capturedImageIndex = 0;
	int m_carSpeed = 1;

	void Init();
	void InitCamera();
	void InitGLFW();
	void InitImgui();
	void InitOpenGL();
	void InitRenderObjects();
	void InitCubemap();
	void InitCubemapTextures();
	void InitCubemapBuffers();
	void InitLights();
	void InitWater();

	void InitBuffers();
	void InitMaterialsBuffers();
	void InitLightBuffers();
	void InitPickingBuffers();
	void InitSecondViewBuffers();
	void InitShadowBuffers();

	void InitShaders();

	void Update();
	void WaterUpdate();
	void InputUpdate();
	void ClutterUpdate();
	void TerrainGeneration();

	void Draw();
	void WaterDraw(CameraType type);
	void PickingDrawPass();
	void ObjectDrawPass(CameraType type);
	void ObjectDrawPassTextured(std::shared_ptr<RenderObject>& obj);
	void ObjectDrawPassUntextured(std::shared_ptr<RenderObject>& obj);
	void OutlineDrawPass();
	void CubemapDrawPass(CameraType type);
	void ShadowMapDrawPass();

	void sendScreenCapture(std::shared_ptr<std::vector<unsigned char>> pixels);
public:

	TSR_Simulation();
	~TSR_Simulation();

	int Run();
};
