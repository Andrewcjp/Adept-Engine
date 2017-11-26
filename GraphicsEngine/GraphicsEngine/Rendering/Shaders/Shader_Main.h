#pragma once
#include "../RHI/Shader.h"

#include <string>
//typedef struct _ConstBuffer
//{
//	glm::mat4 M;
//	glm::mat4 V;
//	glm::mat4 P;
//}ConstBuffer;
typedef struct _LightUniformBuffer
{
	glm::vec3 position;
	glm::vec3 color;
	glm::vec3 Direction;
	int type;//type 1 == point, type 0 == directional, tpye 2 == spot
	int ShadowID;
	int DirShadowID;
	int HasShadow;
}LightUniformBuffer;
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
	LightUniformBuffer* LightBuffers = nullptr;
	bool Once = false;
	//todo move to shader
	GLuint ubo = 0;
	
	GLuint Buffer;

	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
};

