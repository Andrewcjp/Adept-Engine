#pragma once
#include "RHI\Shader.h"
//As this shader is a Genration one it currently holds its own processing code. This might change later.
class Shader_Convolution :
	public Shader
{
public:
	Shader_Convolution();
	~Shader_Convolution();
	void init();
	void ComputeConvolution(BaseTexture * Target);
	std::vector<ShaderParameter> GetShaderParameters() override;
	std::vector<VertexElementDESC> GetVertexFormat() override;
	BaseTexture* TargetCubemap = nullptr;
	FrameBuffer * CubeBuffer = nullptr;
	struct QuadDrawer
	{
		~QuadDrawer()
		{
			delete VertexBuffer;
		}
		RHIBuffer* VertexBuffer = nullptr;
		void init()
		{
			float g_quad_vertex_buffer_data[] = {
				-1.0f, -1.0f, 0.0f,0.0f,
				1.0f, -1.0f, 0.0f,0.0f,
				-1.0f,  1.0f, 0.0f,0.0f,
				-1.0f,  1.0f, 0.0f,0.0f,
				1.0f, -1.0f, 0.0f,0.0f,
				1.0f,  1.0f, 0.0f,0.0f,
			};
			VertexBuffer = RHI::CreateRHIBuffer(RHIBuffer::BufferType::Vertex);
			VertexBuffer->CreateVertexBuffer(sizeof(float) * 4, sizeof(float) * 6 * 4);
			VertexBuffer->UpdateVertexBuffer(&g_quad_vertex_buffer_data, sizeof(float) * 6 * 4);
		}
		void RenderScreenQuad(RHICommandList * list)
		{
			//todo: less than full screen!
			list->SetVertexBuffer(VertexBuffer);
			list->DrawPrimitive(6, 1, 0, 0);

		}
	};
private:
	
	RHICommandList* CmdList = nullptr;
	RHIBuffer* ShaderData = nullptr;
	
	struct SData
	{		
		glm::mat4x4 VP;
		int faceID = 0;
	};
	SData Data[6];
	Renderable* Cube = nullptr;

	QuadDrawer* Test = nullptr;
};

