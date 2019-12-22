#pragma once
#include "RHI/RHI.h"
#include "RHI/RHI_inc_fwd.h"
#define MAX_POSSIBLE_LIGHTS 256
class MeshPipelineController;
class VRCamera;
class Shader_Skybox;
class Shader_Convolution;
class Shader_EnvMap;
class LightCullingEngine;
class CullingManager;
class Camera;
class Editor_Camera;
class ReflectionEnviroment;

#include "LightInterop.h"
typedef LightEntry LightUniformBuffer;
//static_assert(sizeof(LightUniformBuffer) % 16 == 0, "LightUniformBuffer padding bad");
struct MVBuffer
{
	glm::mat4 V;
	glm::mat4 P;
	glm::vec3 CameraPos;
	float pad;
	glm::ivec2 Res;
	glm::mat4 INV_P;
	glm::vec2 INV_res;
};

struct LightBufferW
{
	int LightCount;
	uint TileX;
	uint TileY;
	int pad[3] = { 1,1,1 };
	LightUniformBuffer Light[MAX_POSSIBLE_LIGHTS];
};

struct MeshTransfromBuffer
{
	glm::mat4 M;
};

struct GPUSceneData
{
	Shader_Convolution* ConvShader = nullptr;
	Shader_EnvMap* EnviromentMap = nullptr;
	Shader_Skybox* SkyBoxShader = nullptr;

};
class ReflectionProbe;
//this class holds all data and managers needed to render the scene.
class SceneRenderer
{
public:
	static void StartUp();
	static void Shutdown();
	static SceneRenderer* Get();
	SceneRenderer();
	~SceneRenderer();
	//CPU culls light etc. and updates all buffers needed to render the scene on all GPUs (if requested)
	void PrepareSceneForRender();

	void PrepareData();

	void RenderScene(RHICommandList* CommandList, bool PositionOnly, FrameBuffer* FrameBuffer = nullptr, bool IsCubemap = false, int index = 0);
	void Init();

	void UpdateMV(VRCamera * c);
	void UpdateMV(Camera * c, int index = 0);
	void UpdateMV(glm::mat4 View, glm::mat4 Projection);
	void UpdateLightBuffer(std::vector<Light*> lights);

	static LightUniformBuffer CreateLightEntity(Light * L, int devindex);

	void BindLightsBufferB(RHICommandList * list, int Override = -1);
	void BindLightsBuffer(RHICommandList * list, std::string slot = std::string());

	void BindMvBuffer(RHICommandList * list, std::string slot = std::string(), int index = 0);
	void BindMvBufferB(RHICommandList * list, int slot, int index = 0);
	//this is the master scene for the rendering code.
	void SetScene(Scene* NewScene);
	void SetupBindsForForwardPass(RHICommandList* list, int eyeindex, FrameBuffer* TargetBuffer);
	void UpdateMVForMainPass();

	Scene* GetScene();

	MeshPipelineController* MeshController = nullptr;
	Shader_Skybox* SB = nullptr;
	std::vector< class ReflectionProbe*> probes;
	LightBufferW LightsBuffer = LightBufferW();
	LightCullingEngine* GetLightCullingEngine();
	MeshPipelineController* GetPipelineController();
	CullingManager* GetCullingManager();
	static Camera* GetCurrentCamera();
	void SetEditorCamera(Editor_Camera* Cam);

	ReflectionEnviroment* GetReflectionEnviroment();
	static void DrawScreenQuad(RHICommandList* list);
private:
	bool SceneChanged = false;
	ReflectionEnviroment* Enviroment = nullptr;
	static SceneRenderer* Instance;
	RHIBuffer * CLightBuffer[MAX_GPU_DEVICE_COUNT] = { nullptr };
	RHIBuffer* CMVBuffer = nullptr;
	//the View and projection Matrix in one place as each game object will not have different ones.
	MVBuffer MV_Buffer;

	class Scene* TargetScene = nullptr;
	class Shader_NodeGraph* WorldDefaultMatShader = nullptr;

	LightCullingEngine* LightCulling = nullptr;
	CullingManager* Culling = nullptr;
	Camera* CurrentCamera = nullptr;
	Editor_Camera* EditorCam = nullptr;
	RHIBuffer* QuadBuffer = nullptr;
};

