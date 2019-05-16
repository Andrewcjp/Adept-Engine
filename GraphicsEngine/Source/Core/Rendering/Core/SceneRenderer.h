#pragma once
#include "RHI/RHI.h"
#include "RHI/RHI_inc_fwd.h"
#define MAX_POSSIBLE_LIGHTS 256
class MeshPipelineController;
class VRCamera;
class Shader_Skybox;
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
	float t3[3];
};
#pragma pack(pop)
static_assert(sizeof(LightUniformBuffer) % 16 == 0, "LightUniformBuffer padding bad");
struct MVBuffer
{
	glm::mat4 V;
	glm::mat4 P;
	glm::vec3 CameraPos;
};

struct LightBufferW
{
	int LightCount;
	int  pad[3];
	LightUniformBuffer Light[MAX_POSSIBLE_LIGHTS];	
};

struct MeshTransfromBuffer
{
	glm::mat4 M;
};
class RelfectionProbe;
class SceneRenderer
{
public:
	SceneRenderer(class Scene* Target);
	~SceneRenderer();
	void RenderScene(RHICommandList* CommandList, bool PositionOnly, FrameBuffer* FrameBuffer = nullptr, bool IsCubemap = false, int index = 0);
	void Init();
	void UpdateReflectionParams(glm::vec3 lightPos);
	void UpdateMV(VRCamera * c);
	void UpdateMV(Camera * c, int index = 0);
	void UpdateMV(glm::mat4 View, glm::mat4 Projection);
	void UpdateLightBuffer(std::vector<Light*> lights);
	void BindLightsBuffer(RHICommandList * list, int Override = -1);
	void BindMvBuffer(RHICommandList * list, int slot);
	void BindMvBuffer(RHICommandList * list, int slot, int index);
	void SetScene(Scene* NewScene);
	void SetupBindsForForwardPass(RHICommandList* list);
	void UpdateRelflectionProbes(RHICommandList * commandlist);
	bool AnyProbesNeedUpdate();
	Scene* GetScene();
	void RenderCubemap(RelfectionProbe * Map, RHICommandList * commandlist);
	void SetMVForProbe(RHICommandList * list, int index, int slot);
	MeshPipelineController* Controller = nullptr;
	Shader_Skybox* SB = nullptr;
	std::vector< class RelfectionProbe*> probes;
private:

	RHIBuffer * CLightBuffer[MAX_GPU_DEVICE_COUNT] = { nullptr };
	RHIBuffer* CMVBuffer = nullptr;


	//the View and projection Matix in one place as each gameobject will not have diffrent ones.
	struct MVBuffer MV_Buffer;
	LightBufferW LightsBuffer;


	class Scene* TargetScene = nullptr;
	class Shader_NodeGraph* WorldDefaultMatShader = nullptr;
	//Cube map captures
	MVBuffer CubeMapViews[6];
	float zNear = 0.1f;
	float ZFar = 1000.0f;
	RHIBuffer* RelfectionProbeProjections = nullptr;
};

