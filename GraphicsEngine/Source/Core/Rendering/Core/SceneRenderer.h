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
#pragma pack(push, 16)
/*__declspec(align(32))*/ struct LightUniformBuffer
{
	glm::vec3 position;
	float t;
	glm::vec3 color;
	float t2;
	glm::vec3 Direction;
	float t4;
	glm::mat4x4 LightVP;
	int type;//type 1 == point, type 0 == directional, type 2 == spot
	int ShadowID;
	int DirShadowID;
	int HasShadow = 0;
	int PreSampled[4];//padding sucks!
	float Range;
	//float Pad[3];
};
#pragma pack(pop)
//static_assert(sizeof(LightUniformBuffer) % 16 == 0, "LightUniformBuffer padding bad");
struct MVBuffer
{
	glm::mat4 V;
	glm::mat4 P;
	glm::vec3 CameraPos;
	glm::mat4 INV_P;
	glm::ivec2 Res;
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
class RelfectionProbe;
//this class holds all data and managers needed to render the scene.
class SceneRenderer
{
public:
	static SceneRenderer* Get();
	SceneRenderer(class Scene* Target);
	~SceneRenderer();
	//CPU culls light etc. and updates all buffers needed to render the scene on all GPUs (if requested)
	void PrepareSceneForRender();

	void PrepareData();

	void RenderScene(RHICommandList* CommandList, bool PositionOnly, FrameBuffer* FrameBuffer = nullptr, bool IsCubemap = false, int index = 0);
	void Init();
	void UpdateReflectionParams(glm::vec3 lightPos);
	void UpdateMV(VRCamera * c);
	void UpdateMV(Camera * c, int index = 0);
	void UpdateMV(glm::mat4 View, glm::mat4 Projection);
	void UpdateLightBuffer(std::vector<Light*> lights);

	static LightUniformBuffer CreateLightEntity(Light * L);

	void BindLightsBuffer(RHICommandList * list, int Override = -1);
	TEMP_API void BindMvBuffer(RHICommandList * list, int slot);
	TEMP_API void BindMvBuffer(RHICommandList * list, int slot, int index);
	void SetScene(Scene* NewScene);
	void SetupBindsForForwardPass(RHICommandList* list, int eyeindex);
	void UpdateMVForMainPass();

	Scene* GetScene();
	
	void SetMVForProbe(RHICommandList * list, int index, int slot);
	MeshPipelineController* MeshController = nullptr;
	Shader_Skybox* SB = nullptr;
	std::vector< class RelfectionProbe*> probes;
	LightBufferW LightsBuffer;
	LightCullingEngine* GetLightCullingEngine();
	MeshPipelineController* GetPipelineController();
	CullingManager* GetCullingManager();
	static Camera* GetCurrentCamera();
	void SetEditorCamera(Editor_Camera* Cam);
private:

	RHIBuffer * CLightBuffer[MAX_GPU_DEVICE_COUNT] = { nullptr };
	RHIBuffer* CMVBuffer = nullptr;
	//the View and projection Matrix in one place as each game object will not have different ones.
	MVBuffer MV_Buffer;

	class Scene* TargetScene = nullptr;
	class Shader_NodeGraph* WorldDefaultMatShader = nullptr;
	//Cube map captures
	MVBuffer CubeMapViews[6];
	float zNear = 0.1f;
	float ZFar = 1000.0f;
	RHIBuffer* RelfectionProbeProjections = nullptr;
	LightCullingEngine* LightCulling = nullptr;
	CullingManager* Culling = nullptr;
	Camera* CurrentCamera = nullptr;
	Editor_Camera* EditorCam = nullptr;
};

