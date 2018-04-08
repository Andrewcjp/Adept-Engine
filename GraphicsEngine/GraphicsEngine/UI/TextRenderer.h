#pragma once
#include "OpenGL\OGLShader.h"
#include <string>
#include <GLEW\GL\glew.h>
#include "glm\fwd.hpp"
#include <ft2build.h>
#include <map>
#include "include/glm/ext.hpp"
#include "OpenGL/OGLShaderProgram.h"
#include FT_FREETYPE_H  
class BaseTexture;
class Text_Shader;
class ShaderProgramBase;
struct Character
{
	BaseTexture* Texture;   // ID of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset 
	GLuint Advance;    // offset to next glyph
};
struct atlas;
class TextRenderer
{
public:
	static TextRenderer* instance;
	TextRenderer(int width, int height);
	~TextRenderer();

	void RenderFromAtlas(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1, 1, 1));

	void Finish();

	void Reset();

	void LoadText();
	void UpdateSize(int width, int height)
	{
		m_width = width;
		m_height = height;
	}
private:
	int m_width, m_height = 0;
	Text_Shader * m_TextShader;
	std::map<char, Character> Characters;
	FT_Library ft;
	FT_Face face;
	GLuint TExtVAO;
	GLuint TextVBO;
	BaseTexture* Atlas;
	atlas* textat;
	GLint attribute_coord;
	bool LoadAsAtlas = false;
	RHIBuffer* VertexBuffer = nullptr;
	RHICommandList* TextCommandList = nullptr;
	float ScaleFactor = 1.0f;
};

