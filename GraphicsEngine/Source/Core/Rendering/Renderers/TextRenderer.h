#pragma once
#include <string>
#include <ft2build.h>
#include <map>
#include "RHI/RHI.h"
#include "RHI/BaseTexture.h"

#if BUILD_FREETTYPE
#include FT_FREETYPE_H  
#endif
class BaseTexture;
class Text_Shader;
class ShaderProgramBase;
struct Character
{
	BaseTexture* Texture;   // ID of the glyph texture
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset 
	int Advance;    // offset to next glyph
};
struct TextBatch
{
	std::string Text = "";
	glm::vec2 pos;
	float scale = 1.0f;
	glm::vec3 color = glm::vec3(1, 1, 1);
	glm::vec4 Rect = glm::vec4(0, 0, 0, 0);
};
struct atlas;
class TextRenderer
{
public:
	static TextRenderer* instance;
	TextRenderer(int width, int height, bool SetInstance = false);
	~TextRenderer();

	CORE_API static void RenderText(std::string text, float x, float y, float scale, glm::vec3 color);

	void RenderBatches(std::vector<TextBatch*>& batches, RHICommandList * list);

	void RenderDirect(RHICommandList * list, std::string text, glm::vec2 pos, float scale, glm::vec3 colour);

	int CreateGlyphs(std::string text, glm::vec2 pos, float scale, glm::vec3 color);

	void RenderFromAtlas(std::string text, float x, float y, float scale, glm::vec3 color = glm::vec3(1, 1, 1), bool Reset = false);
	
	void Finish(bool final = false);
	void Reset();
	void LoadText();
	void UpdateSize(int width, int height, glm::ivec2 offset = glm::ivec2(0));
	void NotifyFrameEnd();
private:
	void RenderAllText();
	bool UseFrameBuffer = false;
	int TextDataLength = 0;
	int m_width, m_height = 0;
	Text_Shader * m_TextShader;
#if BUILD_FREETTYPE
	FT_Library ft;
	FT_Face face;
#endif
	RHIBuffer* VertexBuffer = nullptr;
	RHICommandList* TextCommandList = nullptr;
	//when run on second gpu
	FrameBuffer* Renderbuffer = nullptr;
	float ScaleFactor = 1.0f;
	struct point
	{
		float x;
		float y;
		float s;
		float t;
		glm::vec3 colour;
	};
	RHIPipeLineStateObject* PSO;
	glm::ivec2 UITextOffset;
	std::vector<point> coords;
	int currentsize = 0;
	const int MAX_BUFFER_SIZE = 10000;
	bool NeedsClearRT = true;
	struct atlas
	{
		BaseTextureRef Texture;
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
#if BUILD_FREETTYPE
		atlas(FT_Face face, int height);
#endif
		~atlas();
	};
	atlas* TextAtlas;
	int Offset = 0;
	std::vector<TextBatch> Batches;
};

