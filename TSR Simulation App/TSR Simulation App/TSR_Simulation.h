#pragma once
#include "ComunicationHandler.h"

enum class CameraType {
	OUTSIDE_CAMERA,
	INSIDE_CAMERA
};

extern int g_lastReceivedResult;
extern bool g_newDataReceived;

class TSR_Simulation {
private:

	const std::map<std::string, int> traffic_signs = {
	{"100-", 0}, {"110-", 1}, {"120-", 2}, {"130-", 3}, {"20-", 4},
	{"30-", 5}, {"40-", 6}, {"50-", 7}, {"60-", 8}, {"70-", 9},
	{"80-", 10}, {"90-", 11}, {"100", 12}, {"110", 13}, {"120", 14},
	{"130", 15}, {"20", 16}, {"30", 17}, {"40", 18}, {"50", 19},
	{"60", 20}, {"70", 21}, {"80", 22}, {"90", 23},
	{"konec vseh omejitev", 24}, {"odvzem prednosti", 25}, {"stop", 26} };

	std::string m_currentSign = "unknown";
	bool m_showTrafficSign = false;
	GLuint m_currentSignTexture = 0;
	std::chrono::steady_clock::time_point m_lastDetectionTime;
	bool m_signVisible;


	bool m_isFirstPersonView = false;
	bool m_enableGammaCorrection = false;
	bool m_showCameraControls = true;
	bool m_showEnvironment = true;
	bool m_showVehicle = true;
	bool m_showPerformance = true;
	bool m_showDebug = true;
	bool m_showControlsGuide = true;

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

		RenderObject water;
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
	bool m_isCameraFree = false;

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
	float m_oppCarSpeed = 1.f;
	int m_oppCarBias = 0;

	bool m_menuHover;
	bool m_enablePicking = false;
	bool m_imageCapture = false;
	bool m_imageSend = false;
	float m_imageCaptureInterval = 1.f;
	uint32_t m_capturedImageIndex = 0;
	int m_carSpeed = 1;
	int m_carSpeedPrev = 1;
	int m_passCounter = 0;
	float m_terrainGenerationSpeedSigns = 2.f;
	float m_terrainGenerationSpeedTrees = 2.f;
	float m_oppCerGenerationSpeed = 2.f;

	std::array<int, 27> m_generationBiases;

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

	void LoadTrafficSignTexture(const std::string& sign);

	void Update();
	void WaterUpdate();
	void InputUpdate();
	void ClutterUpdate();
	void TerrainGenerationSigns();
	void TerrainGenerationTrees();
	void TerrainGenerationCar();

	void Draw();
	void WaterDraw(CameraType type);
	void PickingDrawPass();
	void ObjectDrawPass(CameraType type);
	void ObjectDrawPassTextured(std::shared_ptr<RenderObject>& obj);
	void ObjectDrawPassUntextured(std::shared_ptr<RenderObject>& obj);
	void OutlineDrawPass();
	void CubemapDrawPass(CameraType type);
	void ShadowMapDrawPass();
	void RenderedWaterDrawPass();

	void sendScreenCapture(std::shared_ptr<std::vector<unsigned char>> pixels);
	void saveFboToImage(std::shared_ptr<std::vector<unsigned char>> pixels);
	void propabilityMenu();
	void InitRenderedWater();
public:

	TSR_Simulation();
	~TSR_Simulation();

	int Run();
};

