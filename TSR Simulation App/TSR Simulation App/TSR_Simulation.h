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

	const std::unordered_map<std::string, int> traffic_signs = {
		{"10", 0}, {"100", 1}, {"120", 2}, {"20", 3}, {"30", 4},
		{"30-", 5}, {"40", 6}, {"40-", 7}, {"50", 8}, {"50-", 9},
		{"60", 10}, {"60-", 11}, {"70", 12}, {"70-", 13}, {"80", 14},
		{"80-", 15}, {"delo_na_cestiscu", 16}, {"kolesarji_na_cestiscu", 17},
		{"konec_omejitev", 18}, {"odvzem_prednosti", 19}, {"otroci_na_cestiscu", 20},
		{"prednost", 21}, {"prehod_za_pesce", 22}, {"stop", 23}, {"unknown", 24}
	};

	std::string m_currentSign = "unknown";
	bool m_showTrafficSign = false;
	GLuint m_currentSignTexture = 0;


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

	bool m_menuHover;
	bool m_imageCapture = false;
	bool m_imageSend = false;
	float m_imageCaptureInterval = 1.f;
	uint32_t m_capturedImageIndex = 0;
	int m_carSpeed = 1;
	int m_carSpeedPrev = 1;
	int m_passCounter = 0;
	float m_terrainGenerationSpeedSigns = 2.f;
	float m_terrainGenerationSpeedTrees = 2.f;

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
	void InitRenderedWater();

	void LoadTrafficSignTexture(const std::string& sign);

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
	void TerrainGenerationSigns();
	void TerrainGenerationTrees();

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
	void saveFboToImage(std::shared_ptr<std::vector<unsigned char>> pixels);
	void propabilityMenu();
public:

	TSR_Simulation();
	~TSR_Simulation();

	int Run();
};
