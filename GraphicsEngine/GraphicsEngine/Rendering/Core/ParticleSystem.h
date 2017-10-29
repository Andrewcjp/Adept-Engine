#pragma once
#include "glm\glm.hpp"
#include <algorithm>
#include <GLEW\GL\glew.h>
#include "OpenGL\OGLTexture.h"
#include "Rendering\Shaders\Shader_Particle.h"

const int MaxParticles = 5000;
struct Particle
{
	glm::vec3 pos, speed;
	glm::vec4 colour;
	float size, angle, weight;//physcical propeties
	float life; // Remaining life of the particle. if < 0 : dead and unused.
	float cameradistance; // used to depth sort
	bool operator<(const Particle& that) const
	{
		// Sort in reverse order : far particles drawn first.
		return /*this->life != 0 && that.life != 0 && */(this->cameradistance > that.cameradistance);
	}
};
class ParticleSystem
{
public:
	ParticleSystem();
	~ParticleSystem();
	void Add(double delta);

	void Simulate(float delta, glm::vec3 CameraPosition);

	void UpdateUniforms(Transform * t, Camera * c, std::vector<Light*> lights);

	void StreamDataToGPU();

	void Render();

	void SortParticles();
	void Init();
	float SystemSpeed = 2;
	float spread = 0;
	glm::vec3 maindir = glm::vec3(10.0f, 0, 0.0f);
	float Life = 3.0;
	glm::vec3 startingposition = glm::vec3(-150, 30, 0);
	float Size = 1;
	float SystemGravity = 0;// -9.81;
	int newParticlespersecond = 300;
	//to give accuracy this value is / by 100 
	int MaxSize = 10000;
	int minsize = 500;
	int StartParticle = 0;
	bool RandomColour;
	glm::vec4 StartColour = glm::vec4(1, 1, 1, 1);
	bool Randomwithinbounds = true;
	float xbounds = 10;
	float zbounds = 250;
private:
	glm::vec3 GetParticlePositon()
	{
		if (Randomwithinbounds == false)
			return startingposition;
		glm::vec3 finalpos = startingposition;
		finalpos.x = startingposition.x + (-xbounds * 100 + rand() % (int)(xbounds * 100 - (-xbounds * 100))) / 100.0f;
		finalpos.z = startingposition.z + (-zbounds * 100 + rand() % (int)(zbounds * 100 - (-zbounds * 100))) / 100.0f;
		return finalpos;
	}
	int ParticlesCount = 0;
	int FindUnusedParticle();
	int LastUsedParticle = 0;


	Particle Particles[MaxParticles];
	GLfloat* g_particule_position_size_data = new GLfloat[MaxParticles * 4];
	GLfloat* g_particule_color_data = new GLfloat[MaxParticles * 4];
	GLuint billboard_VB;
	GLuint position_buffer;
	GLuint color_buffer;
	OGLTexture* texture;
	Shader_Particle* shader;
};


