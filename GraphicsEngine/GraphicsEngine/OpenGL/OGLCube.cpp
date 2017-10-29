#include "OGLCube.h"

#include <windows.h>
#include <GLEW\GL\glew.h>

OGLCube::OGLCube()
{
	InitUnitCube();
}

OGLCube::~OGLCube()
{
	glDeleteVertexArrays( 1, &m_vao );
	glDeleteBuffers( 1, &m_vbo_verts );
	glDeleteBuffers( 1, &m_vbo_colours );
	glDeleteBuffers( 1, &m_vbo_indices );
}

void OGLCube::InitUnitCube()
{
	SimpleVertex corners[] = 
	{
		{-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f},// 0
		{0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f}, // 1
		{0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 1.0f}, // 2
		{-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f}, // 3

		{0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f}, // 4
		{0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f}, // 5
		{0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // 6
		{0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f}, // 7

		{0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f}, // 8
		{-0.5f, -0.5, -0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f}, //9
		{-0.5f, 0.5, -0.5f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // 10
		{0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // 11
		
		{-0.5f, -0.5, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}, //12
		{-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f},// 13
		{-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f}, // 14
		{-0.5f, 0.5, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // 15

		{-0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f}, // 16
		{0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f}, // 17
		{0.5f, 0.5f, -0.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}, // 18
		{-0.5f, 0.5, -0.5f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f}, // 19
	
		{-0.5f, -0.5, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f}, //20
		{0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f}, // 21
		{0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f}, // 22
		{-0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f}// 23
	};

	//We will use triangles instead of triangle strips
	unsigned int indices[] = 
	{
		0, 1, 3, 
		1, 2, 3,

		4, 5, 7, 
		5, 6, 7,

		8, 9, 11, 
		9, 10, 11,

		12, 14, 15, 
		12, 13, 14,

		16, 18, 19, 
		16, 17, 18,

		20, 22, 23, 
		20, 21, 22
	};

	glGenVertexArrays( 1, &m_vao );
	glBindVertexArray( m_vao );

	glGenBuffers( 1, &m_vbo_verts );
	glBindBuffer( GL_ARRAY_BUFFER, m_vbo_verts );
	glBufferData( GL_ARRAY_BUFFER, 24*sizeof(SimpleVertex), corners, GL_STATIC_DRAW );
	
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), 0 );
	glEnableVertexAttribArray ( 0 );
	
	glVertexAttribPointer( 1, 3, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(NULL+12) );
	glEnableVertexAttribArray ( 1 );

	glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof(SimpleVertex), (void*)(NULL+24) );
	glEnableVertexAttribArray ( 2 );

		
	glGenBuffers( 1, &m_vbo_indices );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_vbo_indices);
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, 36*sizeof(int), indices, GL_STATIC_DRAW );

	glBindVertexArray( 0 );
}

void OGLCube::Render()
{
	/*unsigned int texHandle = dynamic_cast<OGLTexture*>(m_tex)->m_syshandle;

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, texHandle );*/

	glBindVertexArray( m_vao );
		
	glDrawElements( GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0 );
	
	glBindVertexArray( 0 );
}