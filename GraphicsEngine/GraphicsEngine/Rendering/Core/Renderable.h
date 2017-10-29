#pragma once

#include "RHI/BaseTexture.h"
#include "include\glm\glm.hpp"
//This is an abstract class
//It cannot be instantiated without being inherited by a concrete class
class Renderable
{
	protected:
		BaseTexture				*m_tex;

	public:

		//this is a pure virtual function
								Renderable() { m_tex = 0; }
		virtual void			Render() = 0;
		virtual ~Renderable() {}
		
		inline void				SetTexture( BaseTexture* tex )
		{
			m_tex = tex;
		}
};