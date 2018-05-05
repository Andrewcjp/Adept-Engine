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

	void RenderFromAtlas(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1, 1, 1),bool Reset = true);
	void Finish();
	void Reset();
	void LoadText();
	void UpdateSize(int width, int height);
private:
	int TextDataLength = 0;
	int m_width, m_height = 0;
	Text_Shader * m_TextShader;
	FT_Library ft;
	FT_Face face;
	atlas* TextAtlas;
	RHIBuffer* VertexBuffer = nullptr;
	RHICommandList* TextCommandList = nullptr;
	float ScaleFactor = 1.0f;
	struct point
	{
		float x;
		float y;
		float s;
		float t;
	};

	std::vector<point> coords;
	int currentsize = 0;
	const int MAX_BUFFER_SIZE = 350;
};

