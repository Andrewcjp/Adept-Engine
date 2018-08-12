#pragma once
#include "RHI/Shader.h"
#include <string>
#define MAX_LIGHTS 4
#include "RHI/RHICommandList.h"
typedef struct _LightUniformBuffer
{
	
	glm::vec3 position;
	float t;
	glm::vec3 color;
	float t2;
	glm::vec3 Direction;
	float t3;
	glm::mat4x4 LightVP;
	int type;//type 1 == point, type 0 == directional, tpye 2 == spot
	int ShadowID;
	int DirShadowID;
	int HasShadow;
	
}LightUniformBuffer;
struct MVBuffer
{
	glm::mat4 V;
	glm::mat4 P;
	glm::vec3 CameraPos;
};

struct LightBufferW
{
	LightUniformBuffer Light[MAX_LIGHTS];
};
namespace MainShaderRSBinds
{
	enum type
	{
		GODataCBV = 0,
		LightDataCBV = 1,
		MVCBV = 2,
		DirShadow = 3,
		PointShadow = 4,
		DiffuseIr = 5,
		SpecBlurMap = 6,
		EnvBRDF = 7,
		Limit
	};
}
/*__declspec(align(32))*/ struct SceneConstantBuffer//CBV need to be 256 aligned
{
	glm::mat4 M;
	int HasNormalMap = 0;
	float Roughness = 0.0f;
	float Metallic = 0.0f;
};
class Shader_Main :public Shader
{
public:
	Shader_Main(bool LoadForward = true);
	~Shader_Main();
	void SetNormalVis();
	void SetShadowVis();
	void SetFullBright();
	void RefreshLights() { Once = false; };
	bool ISWATER = false;
	float currentnumber = 0;
	bool IsPhysics = false;
	void ClearBuffer();
	void UpdateCBV();
	void UpdateUnformBufferEntry(const SceneConstantBuffer &bufer, int index);
	void SetActiveIndex(class RHICommandList * list, int index, int DeviceIndex = 0);
	static void GetMainShaderSig(std::vector<Shader::ShaderParameter>& out);

	void UpdateMV(Camera * c);
	void UpdateMV(glm::mat4 View, glm::mat4 Projection);
	SceneConstantBuffer CreateUnformBufferEntry(class GameObject * t);
	void UpdateLightBuffer(std::vector<Light*> lights);
	void BindLightsBuffer(RHICommandList * list,bool JustLight = false);
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	void BindMvBuffer(RHICommandList * list, int slot);

private:
	
	bool shadowvisstate = false;
	bool vistate = false;
	bool enabledFullBright = false;
	_MVPStruct UBuffer;
	LightUniformBuffer LightBuffer;
	const int CurrentLightcount = 10;
	//LightUniformBuffer* LightBuffers = nullptr;
	bool Once = false;
	//todo move to shader
	
	int MaxConstant = 125;
	std::vector<SceneConstantBuffer> SceneBuffer;
	//information for all the lights in the scene currently

	RHIBuffer* CLightBuffer;
	RHIBuffer* CMVBuffer = nullptr;
	RHIBuffer* GameObjectTransformBuffer[MAX_DEVICE_COUNT] = { nullptr };
	//the View and projection Matix in one place as each gameobject will not have diffrent ones.
	struct MVBuffer MV_Buffer;
	LightBufferW LightsBuffer;


};

