#pragma once
#include "../Rendering/Core/Renderable.h"
class OGLPlane : public Renderable
{
private:
	struct SimpleVertex
	{
		float position[3];
		float colour[3];
		float uv[2];
	};

private:
	unsigned int	m_vao;
	unsigned int	m_vbo_verts;
	unsigned int	m_vbo_colours;
	unsigned int	m_vbo_indices;

	void			InitUnitCube();

public:
	OGLPlane();
	~OGLPlane();


	void			Render();
};

