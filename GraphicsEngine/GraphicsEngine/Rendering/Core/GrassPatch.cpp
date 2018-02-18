#include "GrassPatch.h"
#include "GPUStateCache.h"
#include "../RHI/RHI.h"

GrassPatch::GrassPatch()
{
	m_grassmesh = RHI::CreateMesh(("Grass.obj"), nullptr);
	m_tex = new OGLTexture("\\asset\\texture\\billboardgrass0002.png");
	Shader = new Shader_Grass();
	m_transfrom = new Transform();
	m_transfrom->SetPos(glm::vec3(30, 0, 0));
	m_transfrom->SetScale(glm::vec3(0.8f));//eleimiate some z fighting
	CurrentGrassSize = static_cast<int>(ceil(m_width*m_depth));
	//m_transfrom->SetEulerRot(glm::vec3(10, 0, 0));
	for (float x = -(m_width / 2.0f); x < (m_width / 2.0f); x += (1.0f / Density))
	{
		for (float y = -(m_depth / 2.0f); y < (m_depth / 2.0f); y += (1.0f / Density))
		{
			transforms.push_back(glm::vec2(x, y));
			rotations.push_back(glm::toMat4(glm::quat(glm::radians(glm::vec3(0, 0, 0)))));
			offsets.push_back(offsetsize*((rand() % 100) / 100.0f));
			Colours.push_back(glm::vec3(fabs(x / m_width), 1, 0));
		}
	}
	Colours[10] = glm::vec3(0, 0, 0);
	//unsigned int block_index = glGetUniformBlockIndex(Shader->GetShaderProgram()->GetProgramHandle(), "RotBuffer");
	//GLuint binding_point_index = 1;
	//glGenBuffers(1, &ubo);
	//glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	//glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, ubo);
	////glNamedBufferData(ubo, sizeof(ConstBuffer), nullptr, GL_DYNAMIC_DRAW);
	//glBindBuffer(GL_UNIFORM_BUFFER, 0);
	RotationsMatrixUniform = glGetUniformLocation(Shader->GetShaderID(), "Rotations");
	BindExtraData();
}


GrassPatch::~GrassPatch()
{
	glDeleteBuffers(1, &PositionsBuffer);
	delete m_grassmesh;
	delete m_tex;
	delete Shader;
}
void GrassPatch::UpdateUniforms(Camera* c, std::vector<Light*> lights, float deltatime)
{
	currenttime += deltatime;
	m_tex->Bind(0);

	Shader->SetShaderActive();
	Shader->UpdateUniforms(m_transfrom, c, lights);

	glUniformMatrix4fv(RotationsMatrixUniform, (GLsizei)rotations.size(), GL_FALSE, glm::value_ptr(rotations[0]));

	////todo: UniformBuffer!
}
void GrassPatch::BindExtraData()
{
	glBindVertexArray(((OGLMesh*)m_grassmesh)->GetVao());
	glGenBuffers(1, &PositionsBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, PositionsBuffer);
	//	glBufferData(GL_ARRAY_BUFFER, MAX_SHADER_Array * sizeof(glm::vec2), NULL, GL_STREAM_DRAW);
	glBufferData(GL_ARRAY_BUFFER, CurrentGrassSize * sizeof(glm::vec2), &transforms[0], GL_STATIC_DRAW);

	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);

	//	glGenBuffers(1, &ColourBuffer);

		//glBindBuffer(GL_ARRAY_BUFFER, ColourBuffer);
		//glBufferData(GL_ARRAY_BUFFER, MAX_SHADER_Array * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
		////glBufferData(GL_ARRAY_BUFFER, MAX_SHADER_Array * sizeof(glm::vec3), &transforms[0], GL_STATIC_DRAW);
		//glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		//glEnableVertexAttribArray(7);
		//glVertexAttribDivisor(7, 1);
	StreamDataToGPU();
}
void GrassPatch::Render()
{

	//glBindBuffer(GL_ARRAY_BUFFER, ColourBuffer);
	//glVertexAttribPointer(7, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
	//glEnableVertexAttribArray(7);
	//glVertexAttribDivisor(7, 1);

	glBindBuffer(GL_ARRAY_BUFFER, PositionsBuffer);
	glVertexAttribPointer(5, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
	glEnableVertexAttribArray(5);
	glVertexAttribDivisor(5, 1);

	((OGLMesh*)m_grassmesh)->RenderInstance(static_cast<int>(transforms.size()));
	glDisableVertexAttribArray(5);
	//	glDisableVertexAttribArray(7);

}
///gpu recusrion
void GrassPatch::StreamDataToGPU()
{
	glBindBuffer(GL_ARRAY_BUFFER, ColourBuffer);
	glBufferData(GL_ARRAY_BUFFER, MAX_SHADER_Array * sizeof(glm::vec3), NULL, GL_STREAM_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, Colours.size() * sizeof(glm::vec3), &Colours[0]);

	//glBindBuffer(GL_ARRAY_BUFFER, PositionsBuffer);
	//glBufferData(GL_ARRAY_BUFFER, MAX_SHADER_Array * sizeof(glm::vec2), NULL, GL_STREAM_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, transforms.size() * sizeof(glm::vec2), &transforms[0]);

	//glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
	//glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
	//glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_color_data);
}
void GrassPatch::UpdateAnimation(float dt)
{

	if (currentangle > maxDelta)
	{
		forward = false;
	}
	if (currentangle < -maxDelta)
	{
		forward = true;
	}
	for (size_t i = 0; i < rotations.size(); i++)
	{
		//glm::vec3 rotationvec = glm::vec3(0, sinf(currentangle + (offsets[i])), 0);
		if (forward)
		{
			//glm::mat(glm::rotateX(glm::vec3(0, 1, 0), Rotationamt*sinf(currentangle + (offsets[i]))));
			//glm::vec3 rotationvec = glm::vec3(0, Rotationamt*sinf(currentangle + (offsets[i])), 0);
			//rotations[i] = glm::eulerAngleYXZ(rotationvec.x, rotationvec.y, rotationvec.z);
			rotations[i] = glm::toMat4(glm::quat(glm::radians(glm::vec3(Rotationamt*sinf(currentangle + (offsets[i])), 1, 0))));
			currentangle += Speed*dt;
		}
		else
		{

			//rotations[i] = glm::eulerAngleYXZ(rotationvec.x, rotationvec.y, rotationvec.z);
			rotations[i] = glm::toMat4(glm::quat(glm::radians(glm::vec3(Rotationamt*sinf(currentangle + (offsets[i])), 1, 0))));
			currentangle -= Speed*dt;
		}
	}
}
