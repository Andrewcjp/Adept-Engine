#pragma once
#include "include\glm\glm.hpp"
#include "Core/Transform.h"
#include "../Rendering/Core/Camera.h"
#include "OpenGL\OGLShader.h"
#include <GLEW\GL\glew.h>
#include <vector>
#include "../Rendering/Core/Light.h"
#include "include/glm/gtc/type_ptr.hpp"
#include <d3d12.h>
//this is a basis for a shader 
//shaders will derive from this class so that the correct unifroms can be updated
class Shader
{
public:

	enum ShaderParamType { SRV, UAV, CBV };
	enum RHI_SHADER_VISIBILITY
	{
		SHADER_VISIBILITY_ALL = 0,
		SHADER_VISIBILITY_VERTEX = 1,
		SHADER_VISIBILITY_HULL = 2,
		SHADER_VISIBILITY_DOMAIN = 3,
		SHADER_VISIBILITY_GEOMETRY = 4,
		SHADER_VISIBILITY_PIXEL = 5
	};
	typedef
	enum INPUT_CLASSIFICATION
	{
		INPUT_CLASSIFICATION_PER_VERTEX_DATA = 0,
		INPUT_CLASSIFICATION_PER_INSTANCE_DATA = 1
	} 	INPUT_CLASSIFICATION;
	typedef struct VertexElementDESC
	{
		char* SemanticName;
		UINT SemanticIndex;
		DXGI_FORMAT Format;
		UINT InputSlot;
		UINT AlignedByteOffset;
		INPUT_CLASSIFICATION InputSlotClass;
		UINT InstanceDataStepRate;
	} 	VertexElementDESC;
	struct ShaderParameter
	{
		ShaderParameter()
		{}
		ShaderParameter(ShaderParamType it, int sslot, int reg , RHI_SHADER_VISIBILITY Vis = RHI_SHADER_VISIBILITY::SHADER_VISIBILITY_ALL)
		{
			Type = it;
			SignitureSlot = sslot;
			RegisterSlot = reg;
			Visiblity = Vis;
		}
		ShaderParamType Type;
		RHI_SHADER_VISIBILITY Visiblity;
		int SignitureSlot = 0;
		int RegisterSlot = 0;
	};
	//todo: migrate to New system
	typedef struct _MVPStruct
	{
		glm::mat4 M;
		glm::mat4 V;
		glm::mat4 P;
	}MVPStruct;
	Shader();
	virtual ~Shader();

	void UpdateUniforms(Transform* t, Camera* c, std::vector<Light*> lights = std::vector<Light*>());
	virtual void UpdateOGLUniforms(Transform* t, Camera* c, std::vector<Light*> lights) {};
	virtual void UpdateD3D11Uniforms(Transform* t, Camera* c, std::vector<Light*> lights) {};
	virtual void SetShaderActive();
	ShaderProgramHandle GetShaderID();
	ShaderProgramBase* GetShaderProgram();
	virtual std::vector<ShaderParameter> GetShaderParameters();
	const int ShadowFarPlane = 500;
	virtual bool SupportsAPI(ERenderSystemType Type);
	virtual std::vector<VertexElementDESC> GetVertexFormat();
protected:
	ShaderProgramBase * m_Shader;

};

const int ALBEDOMAP = 0;
const int SHADOWCUBEMAP = 10;
const int SHADOWCUBEMAP2 = 11;
const int SHADOWDIRMAP1 = 9;
const int NORMALMAP = 4;
const int DISPMAP = 5;