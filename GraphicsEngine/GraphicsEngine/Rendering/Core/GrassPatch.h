#pragma once
#include "OpenGL\OGLMesh.h"
#include "OpenGL\OGLTexture.h"
#include "Rendering\Shaders\Shader_Grass.h"
#include "Core/Transform.h"

class GrassPatch
{
public:
	GrassPatch();
	~GrassPatch();
	void UpdateUniforms(Camera * c, std::vector<Light*> lights, float deltatime);
	void BindExtraData();
	void Render();
	void StreamDataToGPU();
	void UpdateAnimation(float t);
private:
	OGLMesh* m_grassmesh;
	OGLTexture* m_tex;
	Shader_Grass* Shader;
	std::vector<glm::vec2> transforms;
	std::vector<glm::mat4> rotations;
	std::vector<glm::vec3> Colours;
	std::vector<float> offsets;
	const int MAX_SHADER_Array = 10000;
	int CurrentGrassSize = 0;
	float m_width = 20;
	float m_depth = 20;
	float Density = 1;
	Transform* m_transfrom;
	glm::vec3 m_centerposition;
	float currenttime = 0;
	float currentangle = 0;
	float Speed = 0.01f;
	float Rotationamt = 5;
	float maxDelta = 50;
	float offsetsize = 10;
	bool forward = true;
	GLuint PositionsBuffer;
	GLuint RotationsBuffer;
	GLuint ColourBuffer;
	GLuint ubo = 0;
	unsigned int block_index = 0;
	GLubyte * blockBuffer;
	GLuint RotationsMatrixUniform;

};

