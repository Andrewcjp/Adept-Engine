
#include "DebugLineDrawer.h"
#include "Core/Performance/PerfManager.h"
#include "Core/Platform/PlatformCore.h"
#include "Editor/EditorWindow.h"
#include "Rendering/Shaders/Shader_Line.h"
#include "RHI/RHICommandList.h"
#if _DEBUG
#pragma optimize("g",on)
#pragma runtime_checks( "sc", off )  
#pragma auto_inline( on )  
#endif

DebugLineDrawer* DebugLineDrawer::instance = nullptr;
DebugLineDrawer::DebugLineDrawer(bool DOnly)
{
	Is2DOnly = DOnly;
	LineShader = ShaderComplier::GetShader_Default<Shader_Line, bool>(Is2DOnly);
	ensure(LineShader);
	DataBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Constant);
	DataBuffer->CreateConstantBuffer(sizeof(glm::mat4x4), 1);
	ReallocBuffer(CurrentMaxVerts);
	CmdList = RHI::CreateCommandList();
	PipeLineState state = {};
	state.DepthTest = false;
	state.RasterMode = PRIMITIVE_TOPOLOGY_TYPE::PRIMITIVE_TOPOLOGY_TYPE_LINE;
	CmdList->SetPipelineState(state);
	CmdList->CreatePipelineState(LineShader);
	if (!DOnly)
	{
		instance = this;
	}
	ResizeVertexStream(CurrentVertStreamLength);
}

DebugLineDrawer::~DebugLineDrawer()
{
	EnqueueSafeRHIRelease(CmdList);
	EnqueueSafeRHIRelease(VertexBuffer);
	EnqueueSafeRHIRelease(DataBuffer);
}

void DebugLineDrawer::GenerateLines()
{
	if (Lines.size() == 0)
	{
		return;
	}
	RegenerateVertBuffer();
	ClearLines();
}

void DebugLineDrawer::UpdateLineBuffer(int offset)
{
	//if (offset != Lines.size())
	{
		//ensure(Verts.size() < CurrentMaxVerts);
		VertsOnGPU = GetArraySize();
		VertexBuffer->UpdateVertexBuffer(VertArray, sizeof(VERTEX) * GetArraySize());
		//VertexBuffer->UpdateVertexBuffer(Verts.data(), sizeof(VERTEX) * 10);
	}
}

void DebugLineDrawer::CreateLineVerts(WLine& line)
{
	VERTEX AVert = {};
	AVert.pos = line.startpos;
	AVert.colour = line.colour;
	//Verts.push_back(AVert);
	InsertVertex(AVert);

	VERTEX BVert = {};
	BVert.pos = line.endpos;
	BVert.colour = line.colour;
	InsertVertex(BVert);
	//Verts.push_back(BVert);
}

void DebugLineDrawer::RegenerateVertBuffer()
{
	//	Verts.clear();
	ResetVertexStream();
	for (int i = 0; i < Lines.size(); i++)
	{
		CreateLineVerts(Lines[i]);
	}
	UpdateLineBuffer(0);
}

void DebugLineDrawer::RenderLines()
{
	RenderLines(Projection);
}

void DebugLineDrawer::ReallocBuffer(int NewSize)
{
	if (NewSize == maxSize)
	{
		return;
	}
	ensure(NewSize < maxSize);
	EnqueueSafeRHIRelease(VertexBuffer);
	CurrentMaxVerts = NewSize;
	VertexBuffer = RHI::CreateRHIBuffer(ERHIBufferType::Vertex);
	const int vertexBufferSize = sizeof(VERTEX) * CurrentMaxVerts;
	VertexBuffer->CreateVertexBuffer(sizeof(VERTEX), vertexBufferSize, EBufferAccessType::Dynamic);
}

void DebugLineDrawer::RenderLines(glm::mat4& matrix)
{
	if (VertsOnGPU == 0)
	{
		return;
	}
	CmdList->ResetList();
	CmdList->SetScreenBackBufferAsRT();
	CmdList->SetVertexBuffer(VertexBuffer);
	if (!Is2DOnly)
	{
		DataBuffer->UpdateConstantBuffer(glm::value_ptr(matrix), 0);
#if WITH_EDITOR
		if (EditorWindow::GetInstance()->UseSmallerViewPort())
		{
			IntRect rect = EditorWindow::GetInstance()->GetViewPortRect();
			CmdList->SetViewport(rect.Min.x, rect.Min.y, rect.Max.x, rect.Max.y, 0, 0);
		}
#endif
	}
	CmdList->SetConstantBufferView(DataBuffer, 0, 0);
	CmdList->DrawPrimitive((int)VertsOnGPU, 1, 0, 0);
	CmdList->Execute();
}

void DebugLineDrawer::FlushDebugLines()
{
	Lines.clear();
}

void DebugLineDrawer::ClearLines()
{
	for (int i = (int)Lines.size() - 1; i >= 0; i--)
	{
		if (Lines[i].Time > 0.0f || Lines[i].Persistent)
		{
			Lines[i].Time -= PerfManager::GetDeltaTime();
		}
		if (Lines[i].Time <= 0.0f)
		{
			Lines.erase(Lines.begin() + i);
			RegenNeeded = true;
		}
	}
}

void DebugLineDrawer::InsertVertex(VERTEX v)
{
	if (InsertPtr < CurrentVertStreamLength)
	{
		VertArray[InsertPtr] = v;
		InsertPtr++;
	}
	else
	{
		__debugbreak();
	}
}

void DebugLineDrawer::ResizeVertexStream(int newsize)
{
	if (CurrentVertStreamLength == newsize)
	{
		return;
	}
	VERTEX* newarray = new VERTEX[newsize];
	if (VertArray != nullptr)
	{
		memcpy(newarray, VertArray, CurrentVertStreamLength);
		delete[] VertArray;
	}
	VertArray = newarray;
	CurrentVertStreamLength = newsize;
}

void DebugLineDrawer::ResetVertexStream()
{
	InsertPtr = 0;
}

void DebugLineDrawer::AddLine(glm::vec3 Start, glm::vec3 end, glm::vec3 colour, float time)
{
	if (Lines.size() * 2 >= CurrentMaxVerts)
	{
		const int newbuffersize = (int)Lines.size() * 2 + 100;
		if (newbuffersize >= maxSize)
		{
			ReallocBuffer(maxSize);
			ResizeVertexStream(maxSize);
			return;
		}
		ReallocBuffer(newbuffersize);
		ResizeVertexStream(newbuffersize);
	}
	WLine l = {};
	l.startpos = Start;
	l.endpos = end;
	l.colour = colour;
	l.Time = time;
	Lines.push_back(l);
}

void DebugLineDrawer::OnResize(int newwidth, int newheight)
{
	if (Is2DOnly)
	{
		Projection = glm::ortho(0.0f, static_cast<float>(newwidth), 0.0f, static_cast<float>(newheight));
		DataBuffer->UpdateConstantBuffer(glm::value_ptr(Projection), 0);
	}
}

DebugLineDrawer * DebugLineDrawer::Get()
{
	return instance;
}
#if _DEBUG
#pragma auto_inline( off ) 
#pragma runtime_checks( "", restore ) 
#pragma optimize("",off)
#endif