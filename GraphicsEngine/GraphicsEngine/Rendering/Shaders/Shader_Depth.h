#pragma once
#include "../RHI/Shader.h"
#include <string>
#include "OpenGL\OGLTexture.h"
#define CUBE_SIDES 6
#include "../Rendering/Core/FrameBuffer.h"
#include "glm\fwd.hpp"
class Shader_Depth :public Shader
{
public:
	struct ConstantBuffer
	{
		glm::mat4		m_worldMat;
		glm::mat4		m_viewMat;
		glm::mat4		m_projection;
	};
	int SHADOW_WIDTH = 2048, SHADOW_HEIGHT = 2048;
	
	void INIT();
	Shader_Depth(Light* targetlight,bool LoadGeo = true);
	~Shader_Depth();
	void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights);
	void SetShadowRes(int width, int height) {
		SHADOW_WIDTH = width;
		SHADOW_HEIGHT = height;
	}
	void SetShaderActive() override;
	void SetShaderActive(CommandListDef* List);
	int m_Model, m_farplane, m_lightpos = 0;
	int depthCubemap;
	int m_ShadowBuffer;
	void BindShadowmmap(CommandListDef* List);
	bool LoadGeomShader = true;
	OGLTexture* CubeMaptex;
	FrameBuffer* shadowbuffer;
	Light* targetlight;
	ConstantBuffer				m_cbuffer;
	// Inherited via Shader
	virtual void UpdateD3D11Uniforms(Transform * t, Camera * c, std::vector<Light*> lights) override;
private:
	float znear = 1;
	float zfar = 50; 
};

