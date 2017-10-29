#pragma once

#include "../Rendering/Core/Renderable.h"
//#include "Listener.h"
#include "OGLTexture.h"

class OGLCube : public Renderable
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
						OGLCube();
						~OGLCube();
		

		void			Render();
};