#include "TextRenderer.h"
#include "Core/EngineInc.h"
#include "Core/Platform/PlatformCore.h"
#include "Rendering/PostProcessing/PostProcessing.h"
#include "Rendering/Shaders/Text_Shader.h"
#include "RHI/DeviceContext.h"
#include "RHI/RHITypes.h"

#define MAXWIDTH 1024
TextRenderer* TextRenderer::instance = nullptr;
TextRenderer::TextRenderer(int width, int height, bool SetInstance /*= false*/)
{
	m_width = width;
	m_height = height;
	m_TextShader = new Text_Shader(RHI::GetDeviceContext(0));
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	LoadText();
	if (SetInstance)
	{
		instance = this;
	}
	coords.reserve(100 * 6);
	UpdateSize(Engine::EngineInstance->GetWidth(), Engine::EngineInstance->GetHeight());
}


TextRenderer::~TextRenderer()
{
	SafeDelete(TextAtlas);
	SafeDelete(m_TextShader);
	EnqueueSafeRHIRelease(VertexBuffer);
	coords.clear();
	EnqueueSafeRHIRelease(Renderbuffer);
	SafeRelease(PSO);
	EnqueueSafeRHIRelease(TextCommandList);
}

void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
	instance->RenderFromAtlas(text, x, y, scale, color);
}

void TextRenderer::RenderFromAtlas(std::string text, float x, float y, float scale, glm::vec3 color, bool reset/* = true*/)
{
	if (reset)
	{
		Reset();
	}
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	scale = scale * ScaleFactor;
	m_TextShader->Update(TextCommandList);
	const uint8_t *p;
	atlas* a = TextAtlas;
	size_t TargetDatalength = 6 * text.length();
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
		currentsize += 6;
	}
}

void TextRenderer::RenderAllText()
{
	if (coords.size() > 0)
	{
		VertexBuffer->UpdateVertexBuffer(coords.data(), sizeof(point)*(currentsize));
		TextCommandList->SetVertexBuffer(VertexBuffer);
		TextCommandList->SetTexture(TextAtlas->Texture, 0);
		TextCommandList->BeginRenderPass(RHI::GetRenderPassDescForSwapChain());
		TextCommandList->DrawPrimitive(currentsize, 1, 0, 0);
		TextCommandList->EndRenderPass();
	}
}

void TextRenderer::Finish(bool final /*= false*/)
{
	RenderAllText();
	TextCommandList->GetDevice()->GetTimeManager()->EndTimer(TextCommandList, EGPUTIMERS::Text);
	if (instance == this && final)
	{
		TextCommandList->GetDevice()->GetTimeManager()->EndTotalGPUTimer(TextCommandList);
	}
	TextCommandList->Execute();
	TextCommandList->GetDevice()->InsertGPUWait(DeviceContextQueue::InterCopy, DeviceContextQueue::Graphics);
	currentsize = 0;
}

void TextRenderer::Reset()
{
	TextCommandList->ResetList();
	TextCommandList->GetDevice()->GetTimeManager()->StartTimer(TextCommandList, EGPUTIMERS::Text);
	TextCommandList->SetPipelineStateObject(PSO);
	currentsize = 0;
}

void TextRenderer::LoadText()
{
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex, RHI::GetDeviceContext(0));
	VertexBuffer->CreateVertexBuffer(sizeof(point), (sizeof(point) * 6) * MAX_BUFFER_SIZE, EBufferAccessType::Dynamic);//max text length?

	TextCommandList = RHI::CreateCommandList(ECommandListType::Graphics, RHI::GetDeviceContext(0));
	RHIPipeLineStateDesc Desc;
	Desc.Blending = true;
	Desc.Cull = false;
	Desc.DepthStencilState.DepthEnable = false;
	Desc.BlendState.AlphaToCoverageEnable = true;
	Desc.ShaderInUse = m_TextShader;
	Desc.Mode = Text;
	PSO = RHI::CreatePipelineStateObject(Desc);
	//TextCommandList->ResetList();
	TextCommandList->SetPipelineStateObject(PSO);
	if (UseFrameBuffer)
	{
		RHIFrameBufferDesc desc = RHIFrameBufferDesc::CreateColour(Engine::EngineInstance->GetWidth(), Engine::EngineInstance->GetHeight());
		desc.clearcolour = glm::vec4(0.0f, 0.2f, 0.4f, 0.0f);
		Renderbuffer = RHI::CreateFrameBuffer(RHI::GetDeviceContext(0), desc);
		//PostProcessing::Instance->AddCompostPass(Renderbuffer);
		RHI::AddLinkedFrameBuffer(Renderbuffer);
	}

	if (FT_Init_FreeType(&ft))
	{
		Log::OutS << "ERROR::FREETYPE: Could not init FreeType Library" << Log::OutS;
	}
	std::string fontpath = AssetManager::GetContentPath();
	fontpath.append("\\AlwaysCook\\fonts\\arial.ttf");
	if (FT_New_Face(ft, fontpath.c_str(), 0, &face))
	{
		Log::OutS << "ERROR::FREETYPE: Failed to load font" << Log::OutS;
	}
	float scale = 1;
	int facesize = 48 * std::lround(scale);//48
	ScaleFactor = (float)1 / scale;
	FT_Set_Pixel_Sizes(face, 0, facesize);

	// Load first 128 characters of ASCII set
	//Freetype is awesome!
	TextAtlas = new atlas(face, facesize);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	//Say thanks you to Free type for being very useful

}

void TextRenderer::UpdateSize(int width, int height, glm::ivec2 offset)
{
	m_width = width;
	m_height = height;
	if (m_TextShader != nullptr)
	{
		m_TextShader->Height = m_height;
		m_TextShader->Width = m_width;
	}
	UITextOffset = offset;
}

void TextRenderer::NotifyFrameEnd()
{
	NeedsClearRT = true;
}

TextRenderer::atlas::atlas(FT_Face face, int height)
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

		//scan into texture
		for (int suby = 0; suby < g->bitmap.rows; suby++)
		{
			int neo = ((ox + (oy + suby)*w));
			memcpy((FinalData + neo), g->bitmap.buffer + (suby*g->bitmap.width), (g->bitmap.width));
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
	Texture = RHI::CreateTextureWithData(w, h, 1, NULL, RHI::GetDeviceContext(0));
	Texture->TextureName = "TextAtlas";
	TextureDescription desc = TextureDescription::DefaultTextTexture(w, h);
	desc.PtrToData = FinalData;
	Texture->CreateTextureFromDesc(desc);
	//printf("Generated a %d x %d (%d kb) texture atlas\n", w, h, w * h / 1024);
}

TextRenderer::atlas::~atlas()
{
//	EnqueueSafeRHIRelease(Texture);
}
