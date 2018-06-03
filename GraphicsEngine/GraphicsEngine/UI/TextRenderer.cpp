#include "TextRenderer.h"
#include "glm\glm.hpp"
#include "RHI/RHI.h"
#include "../RHI/DeviceContext.h"
#include "Rendering/Shaders/Text_Shader.h"
#include <algorithm>
#include <cstring>
#include "../Core/Engine.h"
#include "../RHI/BaseTexture.h"
#include "../Core/Utils/FileUtils.h"
#define MAXWIDTH 1024
TextRenderer* TextRenderer::instance = nullptr;
#include "../Rendering/PostProcessing/PostProcessing.h"
#include "../RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "../RHI/RenderAPIs/D3D12/D3D12Framebuffer.h"
#include "../RHI/RenderAPIs/D3D12/D3D12CommandList.h"
#include "../RHI/RenderAPIs/D3D12/D3D12TimeManager.h"
TextRenderer::TextRenderer(int width, int height)
{
	m_width = width;
	m_height = height;
	m_TextShader = new Text_Shader(D3D12RHI::GetDeviceContext(RunOnSecondDevice));
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	LoadText();
	instance = this;
	coords.reserve(100 * 6);
}

	
TextRenderer::~TextRenderer()
{
	delete TextAtlas;
	delete m_TextShader;
	delete VertexBuffer;
	coords.empty();

	if (Renderbuffer != nullptr)
	{
		delete Renderbuffer;
	}
	delete TextCommandList;
}

void TextRenderer::RenderFromAtlas(std::string text, float x, float y, float scale, glm::vec3 color,bool reset/* = true*/)
{
	if (reset)
	{
		Reset();
	}
	m_TextShader->SetShaderActive();
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	scale = scale * ScaleFactor;
	if (UseFrameBuffer)
	{
		TextCommandList->SetRenderTarget(Renderbuffer);
		if (NeedsClearRT)
		{
			TextCommandList->ClearFrameBuffer(Renderbuffer);
			NeedsClearRT = false;
		}	
	}
	else
	{
		TextCommandList->SetScreenBackBufferAsRT();
	}
	
	m_TextShader->Update(TextCommandList);
	const uint8_t *p;
	atlas* a = TextAtlas;
	unsigned int TargetDatalength = 6 * text.length();
	if (coords.size() < currentsize + TargetDatalength)
	{
		coords.resize(currentsize + TargetDatalength);
		ensure(MAX_BUFFER_SIZE > (coords.size() / 6));
	}
	int c = currentsize;

	/* Loop through all characters */
	for (p = (const uint8_t *)text.c_str(); *p; p++)
	{
		/* Calculate the vertex and texture coordinates */
		float x2 = x + a->c[*p].bl * scale;
		float y2 = -y - a->c[*p].bt * scale;
		float w = a->c[*p].bw * scale;
		float h = a->c[*p].bh * scale;

		/* Advance the cursor to the start of the next character */
		x += a->c[*p].ax * scale;
		y += a->c[*p].ay * scale;

		/* Skip glyphs that have no pixels */
		if (!w || !h)
		{
			continue;
		}

		coords[c++] =
		{ 
			x2, -y2, a->c[*p].tx, a->c[*p].ty,color
		};
		coords[c++] =
		{
			x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty,color
		};
		coords[c++] =
		{
			x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h,color
		};
		coords[c++] =
		{
			x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty,color
		};
		coords[c++] =
		{
			x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h,color
		};
		coords[c++] =
		{
			x2 + w, -y2 - h, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty + a->c[*p].bh / a->h,color
		};
		currentsize+=6;
	}
	if (reset)
	{
		VertexBuffer->UpdateVertexBuffer(coords.data(), sizeof(point)*(text.length() * 6));
	}
	else
	{
		VertexBuffer->UpdateVertexBuffer(coords.data(), sizeof(point)*(currentsize));
	}
	
	TextCommandList->SetVertexBuffer(VertexBuffer);
	TextCommandList->SetTexture(TextAtlas->Texture, 0);
	TextCommandList->DrawPrimitive(c, 1, 0, 0);
	
	///* Draw all the character on the screen in one go */
	if (reset)
	{
		Finish();
	}

}

void TextRenderer::Finish()
{
	TextCommandList->GetDevice()->GetTimeManager()->EndTimer(TextCommandList, D3D12TimeManager::eGPUTIMERS::Text);
	TextCommandList->Execute();
	currentsize = 0;
	D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)Renderbuffer;
	//TextCommandList->ResetList();
	if (RunOnSecondDevice)
	{
		DeviceContext* hostdevbice = D3D12RHI::GetDeviceContext(1);
		hostdevbice->ResetSharingCopyList();
		buffer->CopyToDevice(hostdevbice->GetSharedCopyList());
		hostdevbice->GetSharedCopyList()->Close();
		//buffer->MakeReadyOnTarget(((D3D12CommandList*)TextCommandList)->GetCommandList());
		hostdevbice->ExecuteCommandList(hostdevbice->GetSharedCopyList());
	}
	/*TextCommandList->Execute();*/
}
void TextRenderer::Reset()
{
	TextCommandList->ResetList();
	TextCommandList->GetDevice()->GetTimeManager()->StartTimer(TextCommandList, D3D12TimeManager::eGPUTIMERS::Text);
	currentsize = 0;
}

void TextRenderer::LoadText()
{	
	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Vertex, D3D12RHI::GetDeviceContext(RunOnSecondDevice));
	VertexBuffer->CreateVertexBuffer(sizeof(point), (sizeof(point) * 6) * MAX_BUFFER_SIZE, RHIBuffer::BufferAccessType::Dynamic);//max text length?

	TextCommandList = RHI::CreateCommandList(D3D12RHI::GetDeviceContext(RunOnSecondDevice));
	TextCommandList->SetPipelineState(PipeLineState{ false,false ,true });
	TextCommandList->CreatePipelineState(m_TextShader);
	if (UseFrameBuffer)
	{
		Renderbuffer = RHI::CreateFrameBuffer(1280, 720, D3D12RHI::GetDeviceContext(RunOnSecondDevice), 1.0f, FrameBuffer::ColourDepth, glm::vec4(0.0f, 0.2f, 0.4f, 0.0f));
		PostProcessing::Instance->AddCompostPass(Renderbuffer);
		if (D3D12RHI::Instance)
		{
		//	D3D12RHI::Instance->AddLinkedFrameBuffer(Renderbuffer);
			D3D12FrameBuffer* buffer = (D3D12FrameBuffer*)Renderbuffer;
			buffer->SetupCopyToDevice(RHI::GetDeviceContext(0));
		}
	}

	if (FT_Init_FreeType(&ft))
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}
	std::string fontpath = Engine::GetRootDir();
	fontpath.append("\\asset\\fonts\\arial.ttf");
	if (FT_New_Face(ft, fontpath.c_str(), 0, &face))
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}
	float scale = 2;
	int facesize = 48 * scale;//48
	ScaleFactor = (float)1 / scale;
	FT_Set_Pixel_Sizes(face, 0, facesize);

	// Load first 128 characters of ASCII set
	//Freetype is awesome!
	TextAtlas = new atlas(face, facesize, RunOnSecondDevice);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	//Say thanks you to Free type for being very useful

}

void TextRenderer::UpdateSize(int width, int height)
{
	m_width = width;
	m_height = height;
	if (m_TextShader != nullptr)
	{
		m_TextShader->Height = m_height;
		m_TextShader->Width = m_width;
	}
}

void TextRenderer::NotifyFrameEnd()
{
	NeedsClearRT = true;
}

TextRenderer::atlas::atlas(FT_Face face, int height, bool RunOnSecondDevice)
{

	FT_Set_Pixel_Sizes(face, 0, height);
	FT_GlyphSlot g = face->glyph;

	unsigned int roww = 0;
	unsigned int rowh = 0;
	w = 0;
	h = 0;

	memset(c, 0, sizeof c);

	/* Find minimum size for a texture holding all visible ASCII characters */
	for (int i = 0; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			fprintf(stderr, "Loading character %c failed!\n", i);
			continue;
		}
		if (roww + g->bitmap.width + 1 >= MAXWIDTH)
		{
			w = std::max(w, roww);
			h += rowh;
			roww = 0;
			rowh = 0;
		}
		roww += g->bitmap.width + 1;
		rowh = std::max(rowh, (unsigned int)g->bitmap.rows);
	}

	w = std::max(w, roww);
	h += rowh;

	/* Paste all glyph bitmaps into the texture, remembering the offset */
	int ox = 0;
	int oy = 0;

	rowh = 0;
	unsigned char* FinalData = new unsigned char[w*h];
	for (unsigned int i = 0; i < w*h; i++)
	{
		FinalData[i] = '\0';
	}
	int soff = 0;
	for (int i = 0; i < 128; i++)
	{
		if (FT_Load_Char(face, i, FT_LOAD_RENDER))
		{
			fprintf(stderr, "Loading character %c failed!\n", i);
			continue;
		}

		if (ox + g->bitmap.width + 1 >= MAXWIDTH)
		{
			oy += rowh;
			rowh = 0;
			ox = 0;
		}

		int offset = ((ox + oy * h));
		//scan into texture
		int lastoff = 0;
		for (int suby = 0; suby < g->bitmap.rows; suby++)
		{
			int neo = ((ox + (oy + suby)*w));
			memcpy((FinalData + neo), g->bitmap.buffer + (suby*g->bitmap.width), (g->bitmap.width));
			lastoff = neo;
		}

		c[i].ax = (float)(g->advance.x >> 6);
		c[i].ay = (float)(g->advance.y >> 6);

		c[i].bw = (float)g->bitmap.width;
		c[i].bh = (float)g->bitmap.rows;

		c[i].bl = (float)g->bitmap_left;
		c[i].bt = (float)g->bitmap_top;

		c[i].tx = ox / (float)w;
		c[i].ty = oy / (float)h;

		rowh = std::max(rowh, (unsigned int)g->bitmap.rows);
		ox += g->bitmap.width + 1;
	}
	if (RunOnSecondDevice)
	{
		Texture = RHI::CreateTextureWithData(w, h, 1, NULL, D3D12RHI::GetDeviceContext(1));
	}
	else
	{
		Texture = RHI::CreateTextureWithData(w, h, 1, NULL, D3D12RHI::GetDeviceContext(0));
	}
	Texture->CreateTextureFromData(FinalData, RHI::TextureType::Text, w, h, 1);

	//printf("Generated a %d x %d (%d kb) texture atlas\n", w, h, w * h / 1024);
}

TextRenderer::atlas::~atlas()
{
	delete Texture;
}
