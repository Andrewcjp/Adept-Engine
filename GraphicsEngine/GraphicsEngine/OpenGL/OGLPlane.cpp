#include "OGLPlane.h"


#include <windows.h>
#include <GLEW\GL\glew.h>

OGLPlane::OGLPlane()
{
	InitUnitCube();
}

OGLPlane::~OGLPlane()
{
	glDeleteVertexArrays(1, &m_vao);
	glDeleteBuffers(1, &m_vbo_verts);
	glDeleteBuffers(1, &m_vbo_colours);
	glDeleteBuffers(1, &m_vbo_indices);
}

void OGLPlane::InitUnitCube()
{
	SimpleVertex Vertex[] = { (-1.0f,-1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,0.0f),
							(-1.0f,1.0f,0.0f,0.0f,0.0f,0.0f,1.0f,1.0f),
							(1.0f,1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,1.0f),
							(1.0f,-1.0f,0.0f,0.0f,0.0f,0.0f,0.0f,0.0f) };

	int indices[] = { 2, 1, 0,
		3, 2, 0 };


	glGenVertexArrays(1, &m_vao);
	glBindVertexArray(m_vao);

	glGenBuffers(1, &m_vbo_verts);
	glBindBuffer(GL_ARRAY_BUFFER, m_vbo_verts);
	glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(SimpleVertex), Vertex, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(NULL + 12));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(NULL + 24));
	glEnableVertexAttribArray(2);


	glGenBuffers(1, &m_vbo_indices);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(int), indices, GL_STATIC_DRAW);

	glBindVertexArray(0);
}

void OGLPlane::Render()
{
	/*unsigned int texHandle = dynamic_cast<OGLTexture*>(m_tex)->m_syshandle;

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, texHandle );*/

	glBindVertexArray(m_vao);

	glDrawElements(GL_TRIANGLES, 10, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}