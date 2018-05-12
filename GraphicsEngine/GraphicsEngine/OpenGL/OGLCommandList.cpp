#include "stdafx.h"
#include "OGLCommandList.h"
#include "Rendering/Core/RenderBaseTypes.h"
#include "../Core/Utils/StringUtil.h"
#include "OGLFrameBuffer.h"
#include "OGLShaderProgram.h"
#include "../RHI/Shader.h"
#if BUILD_OPENGL
OGLCommandList::OGLCommandList()
{}

OGLCommandList::~OGLCommandList()
{}
void test()
{
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
	glDisable(GL_BLEND);


	glEnableVertexAttribArray(0);
	GLsizei size = 36;// (sizeof(UIVertex));
	glVertexAttribDivisor(1, 0);//particles did not clear 
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		1,                  // size
		GL_INT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(sizeof(GLfloat) * 2)            // array buffer offset
	);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)((sizeof(GLfloat) * 2) + sizeof(GLint))            // array buffer offset
	);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(0 + sizeof(glm::vec2) + sizeof(GLint) + sizeof(glm::vec3))          // array buffer offset
	);
	// Draw the triangles !
	//glDrawArrays(GL_TRIANGLES, 0, (GLsizei)BatchedVerts.size()); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}
void OGLCommandList::ResetList()
{
	if (shaderprog != nullptr)
	{
		shaderprog->ActivateShaderProgram();
		glDisable(GL_CULL_FACE);
		if (state.DepthTest)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
		test();
	}
}

void OGLCommandList::SetRenderTarget(FrameBuffer * target)
{
	OGLFrameBuffer* fb = (OGLFrameBuffer*)target;
	fb->BindBufferAsRenderTarget();
}

void OGLCommandList::SetViewport(int MinX, int MinY, int MaxX, int MaxY, float MaxZ, float MinZ)
{
	glViewport(MinX, MinY, MaxX, MaxY);
}

void OGLCommandList::Execute()
{}

void OGLCommandList::DrawPrimitive(int VertexCountPerInstance, int InstanceCount, int StartVertexLocation, int StartInstanceLocation)
{
	glDrawArrays(GL_TRIANGLES, StartVertexLocation, VertexCountPerInstance);
}

void OGLCommandList::DrawIndexedPrimitive(int IndexCountPerInstance, int InstanceCount, int StartIndexLocation, int BaseVertexLocation, int StartInstanceLocation)
{}

void OGLCommandList::SetVertexBuffer(RHIBuffer * buffer)
{
	OGLBuffer* dbuffer = (OGLBuffer*)buffer;
	glBindVertexArray(dbuffer->m_vao);
	glBindBuffer(GL_ARRAY_BUFFER, dbuffer->m_vbo_verts);
}

void OGLCommandList::CreatePipelineState(Shader * shader)
{
	shaderprog = (OGLShaderProgram*)shader->GetShaderProgram();


}

void OGLCommandList::UpdateConstantBuffer(void * Array, int offset)
{}

void OGLCommandList::SetConstantBufferView(RHIBuffer * buffer, int offset, int Register)
{}

void OGLCommandList::SetTexture(BaseTexture * texture, int slot)
{}

void OGLCommandList::SetScreenBackBufferAsRT()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void OGLCommandList::ClearScreen()
{
	glClearColor(0.0f, 0.2f, 0.4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void OGLCommandList::SetFrameBufferTexture(FrameBuffer * buffer, int slot)
{}

void OGLCommandList::ClearFrameBuffer(FrameBuffer * buffer)
{}

void OGLCommandList::UAVBarrier(RHIUAV * target)
{}

void OGLCommandList::SetPipelineState(PipeLineState state)
{
	
}



OGLBuffer::OGLBuffer(RHIBuffer::BufferType type):RHIBuffer(type)
{}

OGLBuffer::~OGLBuffer()
{}

void OGLBuffer::CreateVertexBufferFromFile(std::string name)
{
	Triangle* mesh;

	VertexCount = importOBJMesh(StringUtils::ConvertStringToWide(name).c_str(), &mesh)*3;

	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo_verts);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
	glBufferData(GL_ARRAY_BUFFER, VertexCount * sizeof(OGLVertex), &(mesh[0].m_vertices[0].m_position[0]), GL_STATIC_DRAW);
	//'type cast': conversion from 'int' to 'void *' of greater size

#if 0

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + sizeof(glm::vec3)));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + 2 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + 3 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(OGLVertex), (void*)(NULL + 4 * sizeof(glm::vec3)));
	glEnableVertexAttribArray(4);

	glBindVertexArray(0);
#else 
	glVertexAttribDivisor(0, 0);
	glVertexAttribDivisor(1, 0);
	glVertexAttribDivisor(2, 0);
	glDisable(GL_BLEND);


	glEnableVertexAttribArray(0);
	GLsizei size = 0;// (sizeof(UIVertex));
	glVertexAttribDivisor(1, 0);//particles did not clear 
	glVertexAttribPointer(
		0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		2,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)0            // array buffer offset
	);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		1,                  // size
		GL_INT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(sizeof(GLfloat) * 2)            // array buffer offset
	);
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)((sizeof(GLfloat) * 2) + sizeof(GLint))            // array buffer offset
	);
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		3,                  // size
		GL_FLOAT,           // type
		GL_FALSE,           // normalized?
		size,                  // stride
		(void*)(0 + sizeof(glm::vec2) + sizeof(GLint) + sizeof(glm::vec3))          // array buffer offset
	);
	// Draw the triangles !
	//glDrawArrays(GL_TRIANGLES, 0, (GLsizei)BatchedVerts.size()); // 2*3 indices starting at 0 -> 2 triangles

	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
#endif

	delete[] mesh;
}

void OGLBuffer::CreateConstantBuffer(int StructSize, int Elementcount)
{
	GLuint binding_point_index = 1;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBindBufferBase(GL_UNIFORM_BUFFER, binding_point_index, ubo);
	//glNamedBufferData(ubo, sizeof(ConstBuffer), nullptr, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	mStructSize = StructSize;
}

void OGLBuffer::UpdateConstantBuffer(void * Array, int offset)
{
	/*if (!GPUStateCache::CheckCurrentUniformBuffer(ubo))
	{		
		GPUStateCache::UpdateCurrentUniformBuffer(ubo);
	}*/
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	//glBufferData(GL_UNIFORM_BUFFER, mStructSize*offset, &UBuffer, GL_DYNAMIC_DRAW);
	glBufferSubData(GL_UNIFORM_BUFFER, mStructSize*offset, mStructSize, Array);

}

void OGLBuffer::CreateVertexBuffer(int Stride, int ByteSize)
{
	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo_verts);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
	glBufferData(GL_ARRAY_BUFFER, ByteSize, nullptr, GL_STATIC_DRAW);
	}


void OGLBuffer::UpdateVertexBuffer(void * Array, int length)
{
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
	glBufferData(GL_ARRAY_BUFFER, length, Array, GL_STATIC_DRAW);
}
#endif