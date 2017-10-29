#pragma once
#include "include\glm\glm.hpp"
#include "Core/Transform.h"
#include "../Rendering/Core/Camera.h"
#include "OpenGL\OGLShader.h"
#include <GLEW\GL\glew.h>
#include <vector>
#include "../Rendering/Core/Light.h"
#include "include/glm/gtc/type_ptr.hpp"

//this is a basis for a shader 
//shaders will derive from this class so that the correct unifroms can be updated
//this owns the API indie program/shaderthing
class Shader
{
public:
	Shader();
	virtual ~Shader();

	void UpdateUniforms(Transform* t, Camera* c, std::vector<Light*> lights = std::vector<Light*>());
	virtual void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights) = 0;
	virtual void UpdateD3D11Uniforms(Transform* t, Camera* c, std::vector<Light*> lights) = 0;
	virtual void SetShaderActive() {
		if (m_Shader != nullptr)
		{
			m_Shader->ActivateShaderProgram();
		}
		
	}
	ShaderProgramHandle GetShaderID() {
		return m_Shader->GetProgramHandle();
	}
	ShaderProgramBase* GetShaderProgram()
	{
		return m_Shader;
	}
	const int ShadowFarPlane = 500;
	
protected:
	ShaderProgramBase* m_Shader;

};

const int ALBEDOMAP = 0;
const int SHADOWCUBEMAP = 10;
const int SHADOWCUBEMAP2 = 11;
const int SHADOWDIRMAP1 = 9;
const int NORMALMAP = 4;
const int DISPMAP = 5;