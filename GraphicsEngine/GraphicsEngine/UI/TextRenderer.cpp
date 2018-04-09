#include "TextRenderer.h"
#include "glm\glm.hpp"
#include "RHI/RHI.h"
#include "Rendering/Shaders/Text_Shader.h"
#include <algorithm>
#include <cstring>
#include "../Core/Engine.h"
#include "../Rendering/Core/GPUStateCache.h"
#include "../RHI/BaseTexture.h"
TextRenderer* TextRenderer::instance = nullptr;
TextRenderer::TextRenderer(int width, int height)
{
	m_width = width;
	m_height = height;
	m_TextShader = new Text_Shader();
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	LoadAsAtlas = true;
	LoadText();
	instance = this;
}
#include "../Core/Utils/FileUtils.h"
#include "../Core/Assets/ImageIO.h"
#include "SOIL.h"
#define MAXWIDTH 1024
struct atlas
{
	GLuint tex;		// texture object
	BaseTexture* Texture;
	GLint uniform_tex;
	unsigned int w;			// width of texture in pixels
	unsigned int h;			// height of texture in pixels

	struct
	{
		float ax;	// advance.x
		float ay;	// advance.y

		float bw;	// bitmap.width;
		float bh;	// bitmap.height;

		float bl;	// bitmap_left;
		float bt;	// bitmap_top;

		float tx;	// x offset of glyph in texture coordinates
		float ty;	// y offset of glyph in texture coordinates
	} c[128];		// character information

	atlas(FT_Face face, int height)
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
		Texture = RHI::CreateTextureWithData(w, h, 1, NULL, RHI::Text);

		/* Paste all glyph bitmaps into the texture, remembering the offset */
		int ox = 0;
		int oy = 0;

		rowh = 0;
		unsigned char* FinalData = new unsigned char[w*h];
		for (int i = 0; i < w*h; i++)
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

			//memcpy((FinalData + offset), g->bitmap.buffer, (g->bitmap.width * g->bitmap.rows));
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
		Texture->CreateTextureFromData(FinalData, RHI::TextureType::Text, w, h, 1);
		//SOIL_save_image("C:\\Users\\AANdr\\Dropbox\\Engine\\t.bmp", SOIL_SAVE_TYPE_BMP, w, h, 1, FinalData);

		printf("Generated a %d x %d (%d kb) texture atlas\n", w, h, w * h / 1024);
	}

	~atlas()
	{
		glDeleteTextures(1, &tex);
	}
};

TextRenderer::~TextRenderer()
{
	//	delete textat;
	for (int i = 0; i < Characters.size(); i++)
	{
		if (Characters[(const char)i].Texture != nullptr)
		{
			Characters[(const char)i].Texture->FreeTexture();
		}
	}
}
struct point
{
	GLfloat x;
	GLfloat y;
	GLfloat s;
	GLfloat t;
};
void TextRenderer::RenderFromAtlas(std::string text, float x, float y, float scale, glm::vec3 color)
{
	Reset();
	m_TextShader->SetShaderActive();
	m_TextShader->Colour = color;
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	scale = scale * ScaleFactor;
	TextCommandList->SetScreenBackBufferAsRT();
	m_TextShader->Update(TextCommandList);
	const uint8_t *p;
	attribute_coord = 0;
	atlas* a = textat;
	point* coords = new point[6 * text.length()];
	int c = 0;

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
			continue;

		coords[c++] =
		{ x2, -y2, a->c[*p].tx, a->c[*p].ty };
		coords[c++] =
		{
			x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty
		};
		coords[c++] =
		{
			x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h
		};
		coords[c++] =
		{
			x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty
		};
		coords[c++] =
		{
			x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h
		};
		coords[c++] =
		{
			x2 + w, -y2 - h, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty + a->c[*p].bh / a->h
		};
	}
	VertexBuffer->UpdateVertexBuffer(coords, sizeof(point)*(text.length() * 6));
	TextCommandList->SetVertexBuffer(VertexBuffer);
	TextCommandList->SetTexture(textat->Texture, 0);
	TextCommandList->DrawPrimitive(c, 1, 0, 0);
	///* Draw all the character on the screen in one go */
	Finish();
	delete[] coords;
}
void TextRenderer::Finish()
{
	TextCommandList->Execute();
	//not great 

}
void TextRenderer::Reset()
{
	TextCommandList->ResetList();
}
void TextRenderer::LoadText()
{

	VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Vertex);
	VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 4 * 6 * 150,RHIBuffer::BufferAccessType::Dynamic);//max text length?
	TextCommandList = RHI::CreateCommandList();
	TextCommandList->SetPipelineState(PipeLineState{ false,false ,true });
	TextCommandList->CreatePipelineState(m_TextShader);

	/*glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);*/
	//glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(m_width), 0.0f, static_cast<GLfloat>(m_height));
	//glUniformMatrix4fv(glGetUniformLocation(m_TextShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

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
	ScaleFactor = 1 / scale;
	FT_Set_Pixel_Sizes(face, 0, facesize);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Load first 128 characters of ASCII set
	//Freetype is awesome!
	//we dont need every char right now so dont bother with the firwst 3

	if (LoadAsAtlas)
	{
		textat = new atlas(face, facesize);
	}
	else
	{
		for (GLubyte c = 0; c < 128; c++)
		{
			// Load character glyph 
			if (FT_Load_Char(face, c, FT_LOAD_RENDER))
			{
				std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
				continue;
			}
			if (LoadAsAtlas)
			{
				if (face->glyph->bitmap.buffer == nullptr)
				{
					continue;
				}

			}
			else
			{
				// Now store character for later use
				Character character = {
					RHI::CreateTextureWithData(face->glyph->bitmap.width,face->glyph->bitmap.rows,1,face->glyph->bitmap.buffer,RHI::Text),
					glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
					glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
					static_cast<GLuint>(face->glyph->advance.x)
				};
				Characters.insert(std::pair<GLchar, Character>(c, character));
			}
		}
	}
	//glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	//Say thanks you to Free type for being very useful

	//glGenVertexArrays(1, &TExtVAO);
	//glGenBuffers(1, &TextVBO);
	//glBindVertexArray(TExtVAO);
	//glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
	//glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);//Bind a DYNAMIC_DRAW buffer so that we can send our two vec2's when rendering text
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);//Pointor to a vec4 which we will us as two vec2's
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glBindVertexArray(0);
}