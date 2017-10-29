#include "OGLFrameBuffer.h"
#include "../Rendering/Core/GPUStateCache.h"
OGLFrameBuffer::~OGLFrameBuffer()
{
	Cleanup();
}
void OGLFrameBuffer::CreateBuffer()
{
	if (m_ftype == FrameBufferType::ColourDepth)
	{
		//std::cout << "Current Render Size is " << m_width << " X " << m_height << std::endl;
		BufferTexture = new OGLTexture();
		BufferTexture->CreateTextureAsRenderTarget(m_width, m_height);
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
		glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, BufferTexture->m_syshandle, 0);

		glGenRenderbuffers(1, &FrameRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, FrameRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, FrameRenderBuffer);

		glDrawBuffer(GL_COLOR_ATTACHMENT0);

	}
	else if (m_ftype == FrameBufferType::Depth || m_ftype == FrameBufferType::CubeDepth)
	{
		//std::cout << "ERROR FrameBuffer Not Depth" << std::endl;
		glGenFramebuffers(1, &frameBuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
		BufferTexture = new OGLTexture();
		if (m_ftype == FrameBufferType::CubeDepth)
		{
			BufferTexture->CreateRenderCubeMap(m_width, m_height);
			glBindTexture(GL_TEXTURE_CUBE_MAP, BufferTexture->m_syshandle);

		}
		else
		{
			BufferTexture->CreateTextureAsRenderTarget(m_width, m_height, true);
			glBindTexture(GL_TEXTURE_2D, BufferTexture->m_syshandle);
		}
		glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, BufferTexture->m_syshandle, 0);
	}
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		std::cout << "FrameBuffer Error " << std::endl;
		return;
	}
}
void OGLFrameBuffer::Cleanup()
{
	glDeleteFramebuffers(1, &frameBuffer);
	glDeleteRenderbuffers(1, &FrameRenderBuffer);
	//glDeleteTextures(1, &BufferTexture->m_syshandle);
	BufferTexture->FreeTexture();
	//BufferTexture->FreeTexture();
}
void OGLFrameBuffer::BindToTextureUnit(int unit)
{
	glActiveTexture((GL_TEXTURE0 + unit));
	if (m_ftype == FrameBufferType::CubeDepth)
	{
		glBindTexture(GL_TEXTURE_CUBE_MAP, BufferTexture->m_syshandle);		
	}
	else
	{
		glBindTexture(GL_TEXTURE_2D, BufferTexture->m_syshandle);
	}
	GPUStateCache::UpdateUnitState(unit, BufferTexture->m_syshandle);
}

void OGLFrameBuffer::BindBufferAsRenderTarget()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, m_width, m_height);
}

void OGLFrameBuffer::UnBind()
{
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void OGLFrameBuffer::ClearBuffer()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
