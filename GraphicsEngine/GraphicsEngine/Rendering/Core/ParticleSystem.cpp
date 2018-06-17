#include "ParticleSystem.h"
#include "../Core/Performance/PerfManager.h"
#include "../RHI/RHI.h"
#include "../RHI/BaseTexture.h"
ParticleSystem::ParticleSystem()
{
	texture = AssetManager::DirectLoadTextureAsset("\\asset\\texture\\smoke.png");
	shader = new Shader_Particle();
	static const float plane_positions[] = {
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f,
		-0.5f,  0.5f, 0.0f,
		0.5f,  0.5f, 0.0f,
	};
#if BUILD_OPENGL
	glGenBuffers(1, &billboard_VB);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_VB);
	glBufferData(GL_ARRAY_BUFFER, sizeof(plane_positions), plane_positions, GL_STATIC_DRAW);


	glGenBuffers(1, &position_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	//Tell the buffer that we will stream data to it at a later time
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(float), NULL, GL_STREAM_DRAW);

	glGenBuffers(1, &color_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);

	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
#endif
	Init();
	Life = 25.0f;
}


ParticleSystem::~ParticleSystem()
{

	delete shader;
	delete texture;
}
void ParticleSystem::Add(double delta)
{
	int newparticles = static_cast<int>(delta*newParticlespersecond);
	/*if (newparticles > static_cast<int>(0.016f*newParticlespersecond)) {
		newparticles = static_cast<int>(0.016f*newParticlespersecond);
	}*/

	for (int i = 0; i < newparticles; i++)
	{
		int particleIndex = FindUnusedParticle();
		Particles[particleIndex].life = Life; // This particle will live 5 seconds.
		Particles[particleIndex].pos = GetParticlePositon();

		/*	glm::vec3 randomdir = glm::vec3(
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f,
				(rand() % 2000 - 1000.0f) / 1000.0f
			);*/

		Particles[particleIndex].speed = maindir;// +randomdir * spread;

		Particles[particleIndex].colour = StartColour;
		Particles[particleIndex].size = (rand() % MaxSize + minsize) / 1000.0f;
	}

}
void ParticleSystem::Simulate(float delta, glm::vec3 CameraPosition)
{
	//PerfManager::StartTimer("Stream");
	delta *= SystemSpeed;
	ParticlesCount = 0;
	StartParticle = -1;
	for (int i = 0; i < MaxParticles; i++)
	{
		Particle& p = Particles[i]; // ptr to the current particle
		if (p.life > 0.0f)
		{
			if (StartParticle == -1)
			{
				StartParticle = i;
			}
			p.life -= delta;//decrement life by time passed this frame
			// Simulate  physics
			p.speed += glm::vec3(0.0f, SystemGravity, 0.0f) * (float)delta * 0.005f;
			p.pos += p.speed * delta;
			p.cameradistance = (glm::length2(p.pos - CameraPosition));
			//occlude particles
			//ParticlesContainer[i].pos += glm::vec3(0.0f,10.0f, 0.0f) * (float)delta;

			// Fill the GPU buffer
			g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
			g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
			g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

			g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

			g_particule_color_data[4 * ParticlesCount + 0] = p.colour.x;
			g_particule_color_data[4 * ParticlesCount + 1] = p.colour.y;
			g_particule_color_data[4 * ParticlesCount + 2] = p.colour.z;
			g_particule_color_data[4 * ParticlesCount + 3] = p.colour.w;
			ParticlesCount++;
		}
		else
		{
			//dead particles are set behind the camera
			p.cameradistance = -1.0f;
		}

	}
	//PerfManager::EndTimer("Stream");
	PerfManager::StartTimer("Sort");
	SortParticles();
	PerfManager::EndTimer("Sort");
	StreamDataToGPU();

}
void ParticleSystem::UpdateUniforms(Transform* t, Camera* c, std::vector<Light*> lights)
{

//	texture->Bind(0);//bind the texture
	shader->SetShaderActive();
	shader->UpdateUniforms(t, c, lights);

}
void ParticleSystem::StreamDataToGPU()
{
#if BUILD_OPENGL
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(float) * 4, g_particule_position_size_data);

	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(float), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(float) * 4, g_particule_color_data);
#endif
}
void ParticleSystem::Render()
{
#if BUILD_OPENGL
	glEnable(GL_BLEND);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, billboard_VB);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 1);
	glVertexAttribDivisor(2, 1);
	//draw particles
	glDrawArrayEngine::CompRegistryd(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
#endif
}
void ParticleSystem::SortParticles()
{
	if (StartParticle == -1)
	{
		StartParticle = 0;
	}
	std::sort(&Particles[0], &Particles[MaxParticles]);
}
void ParticleSystem::Init()
{
	for (int i = 0; i < MaxParticles; i++)
	{
		Particles[i].life = -1.0f;
		Particles[i].cameradistance = -1.0f;
	}
}
int ParticleSystem::FindUnusedParticle()
{

	for (int i = LastUsedParticle; i < MaxParticles; i++)
	{
		if (Particles[i].life < 0)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	for (int i = 0; i < LastUsedParticle; i++)
	{
		if (Particles[i].life < 0)
		{
			LastUsedParticle = i;
			return i;
		}
	}

	return 0; // All particles are taken, override the first one
}
