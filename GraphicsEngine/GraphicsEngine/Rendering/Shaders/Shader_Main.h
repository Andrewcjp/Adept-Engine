#pragma once
#include "../RHI/Shader.h"
#include <string>
#define MAX_LIGHTS 4
#include "../RHI/RHICommandList.h"
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
struct SceneConstantBuffer//CBV need to be 256 aligned
{
	glm::mat4 M;
	glm::mat4 V;
	glm::mat4 P;
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
	void UpdateUnformBufferEntry(const SceneConstantBuffer &bufer, int index);
	void SetActiveIndex(class RHICommandList * list, int index);
	static void GetMainShaderSig(std::vector<Shader::ShaderParameter>& out);
	std::vector<Shader::ShaderParameter> GetShaderParameters() override;
	void UpdateMV(Camera * c);
	void UpdateMV(glm::mat4 View, glm::mat4 Projection);
	SceneConstantBuffer CreateUnformBufferEntry(Transform* t);
	void UpdateLightBuffer(std::vector<Light*> lights);
	void BindLightsBuffer(RHICommandList * list);
	std::vector<Shader::VertexElementDESC> GetVertexFormat() override;
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
	std::vector<SceneConstantBuffer> SceneBuffer;
	//information for all the lights in the scene currently

	RHIBuffer* CLightBuffer;
	RHIBuffer* CMVBuffer = nullptr;
	RHIBuffer* GameObjectTransformBuffer = nullptr;
	//the View and projection Matix in one place as each gameobject will not have diffrent ones.
	struct MVBuffer MV_Buffer;
	LightBufferW LightsBuffer;
	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;

};

