#pragma once
#include "../RHI/Shader.h"

#include <string>
//typedef struct _ConstBuffer
//{
//	glm::mat4 M;
//	glm::mat4 V;
//	glm::mat4 P;
//}ConstBuffer;
#include "../D3D12/D3D12Shader.h"
#define MAX_LIGHTS 4
typedef struct _LightUniformBuffer
{
	
	glm::vec3 position;
	float t;
	glm::vec3 color;
	float t2;
	glm::vec3 Direction;
	float t3;
	glm::mat4 LightVP;
	int type;//type 1 == point, type 0 == directional, tpye 2 == spot
	int ShadowID;
	int DirShadowID;
	int HasShadow;
	
}LightUniformBuffer;
struct MVBuffer
{
	glm::mat4 V;
	glm::mat4 P;
};
struct LightBufferW
{
	LightUniformBuffer Light[MAX_LIGHTS];
};
class Shader_Main :public Shader
{
public:
	Shader_Main();
	~Shader_Main();
	void SetNormalVis();
	void SetShadowVis();
	void SetFullBright();
	void SetNormalState(bool t, bool dispstate, bool refelction);
	void RefreshLights() { Once = false; };
	void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights) override;
	bool ISWATER = false;
	float currentnumber = 0;
	bool IsPhysics = false;
	void ClearBuffer();
	void UpdateCBV();
	void UpdateUnformBufferEntry(D3D12Shader::SceneConstantBuffer &bufer, int index);
	void SetActiveIndex(CommandListDef * list, int index);

	void UpdateMV(Camera * c);

	void UpdateMV(glm::mat4 View, glm::mat4 Projection);

	D3D12Shader::SceneConstantBuffer CreateUnformBufferEntry(Transform* t, Camera* c);
	void BindLightsBuffer(CommandListDef * list);
	void UpdateLightBuffer(std::vector<Light*> lights);
private:
	int						m_uniform_model;
	int						m_uniform_View;
	int                     m_uniform_texture;
	int						m_UniformMVP;
	int						m_texDefaultSampler;
	int						m_uniform_LightNumber;
	int						m_DepthTexture;
	int						Uniform_Cam_Pos;
	int						m_IsMapUniform;
	int						m_FarPlane;
	int						m_MV33;
	int						IsReflect;
	bool shadowvisstate = false;
	bool vistate = false;
	bool enabledFullBright = false;
	_MVPStruct UBuffer;
	LightUniformBuffer LightBuffer;
	const int CurrentLightcount = 10;
	//LightUniformBuffer* LightBuffers = nullptr;
	bool Once = false;
	//todo move to shader
	GLuint ubo = 0;
	int MaxConstant =25;
	GLuint Buffer;
	std::vector<D3D12Shader::SceneConstantBuffer> SceneBuffer;
	//information for all the lights in the scene currently
	class D3D12CBV* LightCBV;
	//the View and projection Matix in one place as each gameobject will not have diffrent ones.
	class D3D12CBV* MVCBV;
	struct MVBuffer MV_Buffer;
//	std::vector<LightUniformBuffer> LightsBuffer;
	LightBufferW LightsBuffer;
	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
	virtual void UpdateD3D12Uniforms(Transform* t, Camera* c, std::vector<Light*> lights) override;

};

