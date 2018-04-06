#include "TextRenderer.h"
#include "glm\glm.hpp"
#include "RHI/RHI.h"
#include "Rendering/Shaders/Text_Shader.h"
#include <algorithm>
#include <cstring>
#include "../Core/Engine.h"
#include "../Rendering/Core/GPUStateCache.h"
TextRenderer* TextRenderer::instance = nullptr;
TextRenderer::TextRenderer(int width, int height)
{
	m_width = width;
	m_height = height;
	m_TextShader = new Text_Shader();
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	LoadAsAtlas = true;

	instance = this;
	if (RHI::IsOpenGL())
	{
		LoadText();
	}

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

		if (RHI::GetType() == RenderSystemOGL)
		{
			Texture = RHI::CreateTextureWithData(w, h, 1, NULL, RHI::Text);
			/* Create a texture that will be used to hold all ASCII glyphs */
			glActiveTexture(GL_TEXTURE0);
			glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glUniform1i(uniform_tex, 0);

			glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, 0);

			/* We require 1 byte alignment when uploading texture data */
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

			/* Clamping to edges is important to prevent artifacts when scaling */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			/* Linear filtering usually looks best for text */
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		else if (RHI::GetType() == RenderSystemD3D12)
		{

		}
		/* Paste all glyph bitmaps into the texture, remembering the offset */
		int ox = 0;
		int oy = 0;

		rowh = 0;
		unsigned char* FinalData = new unsigned char[w*h];
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

			//glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
#if 0
			glTexSubImage2D(GL_TEXTURE_2D, 0, ox, oy, g->bitmap.width, g->bitmap.rows, GL_RED, GL_UNSIGNED_BYTE, g->bitmap.buffer);
#else
			int offset = ((ox + oy * h));
			
			memcpy((FinalData + soff), g->bitmap.buffer, (g->bitmap.width * g->bitmap.rows));
			if (i == 65)
			{
				SOIL_save_image("C:\\Users\\AANdr\\Dropbox\\Engine\\c.bmp", SOIL_SAVE_TYPE_BMP, g->bitmap.width, g->bitmap.rows, 1, (FinalData + soff));
			}
			soff += (g->bitmap.width * g->bitmap.rows);
#endif
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

	//	SOIL_save_image("C:\\Users\\AANdr\\Dropbox\\Engine\\t.bmp", SOIL_SAVE_TYPE_BMP, w, h, 1, FinalData);

	//	glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, FinalData);
		fprintf(stderr, "Generated a %d x %d (%d kb) texture atlas\n", w, h, w * h / 1024);
	}

	~atlas()
	{
		glDeleteTextures(1, &tex);
	}
};

TextRenderer::~TextRenderer()
{
	if (RHI::IsOpenGL())
	{
		glDeleteBuffers(1, &TextVBO);
	}
//	delete textat;
	for (int i = 0; i < Characters.size(); i++)
	{
		if (Characters[(const char)i].Texture != nullptr)
		{
			Characters[(const char)i].Texture->FreeTexture();
		}
	}
}
void TextRenderer::RenderText(std::string text, float x, float y, float scale, glm::vec3 color)
{
	if (!RHI::IsOpenGL())
	{
		return;
	}
	m_TextShader->SetShaderActive();
	m_TextShader->Colour = color;
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	m_TextShader->UpdateUniforms(nullptr, nullptr);
	/*glm::mat4 projection = glm::ortho(0.0f, static_cast<GLfloat>(m_width), 0.0f, static_cast<GLfloat>(m_height));
	glUniformMatrix4fv(glGetUniformLocation(m_TextShader->GetProgramHandle(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	glUniform3f(glGetUniformLocation(m_TextShader->GetProgramHandle(), "textColor"), color.x, color.y, color.z);
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(TExtVAO);*/
	//340
	glBindVertexArray(TExtVAO);
	glEnable(GL_BLEND);
	//370 / 367
	// Iterate through all characters in the input string
	std::string::const_iterator c;
	glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
	for (c = text.begin(); c != text.end(); c++)
	{
		Character ch = Characters[*c];

		GLfloat xpos = x + ch.Bearing.x * scale;
		GLfloat ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

		GLfloat w = ch.Size.x * scale;
		GLfloat h = ch.Size.y * scale;
		// Update VBO for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		if (ch.Texture != nullptr)
		{
			ch.Texture->Bind(0);
		}
		// Update content of VBO 

		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);

		// Render quad
		glDrawArrays(GL_TRIANGLES, 0, 6);

		x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	//clean up after wards
	glDisable(GL_BLEND);
	glBindVertexArray(0);
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
	if (!RHI::IsOpenGL())
	{
		return;
	}
	m_TextShader->SetShaderActive();
	m_TextShader->Colour = color;
	m_TextShader->Height = m_height;
	m_TextShader->Width = m_width;
	m_TextShader->UpdateUniforms(nullptr, nullptr);

	const uint8_t *p;
	glEnable(GL_BLEND);
	//	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		/* Use the texture containing the atlas */
	glBindTexture(GL_TEXTURE_2D, textat->tex);
	attribute_coord = 0;
	//USE GL_SHORT 
	//could use intiger manths here
	/* Set up the VBO for our vertex data */
	glBindVertexArray(TExtVAO);
	glEnableVertexAttribArray(attribute_coord);
	glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
	glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);
	atlas* a = textat;
	//	int l = strlen(text);
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

	/* Draw all the character on the screen in one go */
	glBufferData(GL_ARRAY_BUFFER, sizeof(point)*(text.length() * 6), coords, GL_DYNAMIC_DRAW);
	glDrawArrays(GL_TRIANGLES, 0, c);

	glDisableVertexAttribArray(attribute_coord);
	glBindTexture(GL_TEXTURE_2D, 0);
	GPUStateCache::UpdateUnitState(0, 0);
	delete[] coords;
}
void TextRenderer::LoadText()
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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
	FT_Set_Pixel_Sizes(face, 0, 48);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	// Load first 128 characters of ASCII set
	//Freetype is awesome!
	//we dont need every char right now so dont bother with the firwst 3

	if (LoadAsAtlas)
	{
		textat = new atlas(face, 48);
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
	glBindTexture(GL_TEXTURE_2D, 0);
	// Destroy FreeType once we're finished
	FT_Done_Face(face);
	FT_Done_FreeType(ft);
	//Say thanks you to Free type for being very useful

	glGenVertexArrays(1, &TExtVAO);
	glGenBuffers(1, &TextVBO);
	glBindVertexArray(TExtVAO);
	glBindBuffer(GL_ARRAY_BUFFER, TextVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);//Bind a DYNAMIC_DRAW buffer so that we can send our two vec2's when rendering text
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);//Pointor to a vec4 which we will us as two vec2's
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}