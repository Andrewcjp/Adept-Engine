#include "RenderGraphPatchLibrary.h"
#include "RenderGraph.h"


RenderGraphPatchLibrary::RenderGraphPatchLibrary()
{
	Funcs.resize(EBuiltInRenderGraphPatch::Limit);
	OnInit();
}


RenderGraphPatchLibrary::~RenderGraphPatchLibrary()
{}

bool RenderGraphPatchLibrary::ApplyPatch(EBuiltInRenderGraphPatch::Type Type, RenderGraph * graph)
{
	if (Type == EBuiltInRenderGraphPatch::NONE)
	{
		return true;
	}
	if (Funcs[Type] == nullptr)
	{
		return false;
	}
	return Funcs[Type](graph);
}

void RenderGraphPatchLibrary::RegisterPatchFunction(EBuiltInRenderGraphPatch::Type Type, PatchFunc Func)
{
	Funcs[Type] = Func;
}

void RenderGraphPatchLibrary::OnInit()
{
	RegisterPatchFunction(EBuiltInRenderGraphPatch::VRX, std::bind(&RenderGraphPatchLibrary::AddVRX, this, std::placeholders::_1));
	RegisterPatchFunction(EBuiltInRenderGraphPatch::RT_Reflections, std::bind(&RenderGraphPatchLibrary::AddRT_Reflections, this, std::placeholders::_1));
	RegisterPatchFunction(EBuiltInRenderGraphPatch::Voxel_Reflections, std::bind(&RenderGraphPatchLibrary::AddVoxelReflections, this, std::placeholders::_1));
	RegisterPatchFunction(EBuiltInRenderGraphPatch::RT_Voxel_Reflections, std::bind(&RenderGraphPatchLibrary::AddVoxel_RTReflections, this, std::placeholders::_1));
}

bool RenderGraphPatchLibrary::AddVRX(RenderGraph * Graph)
{
	Graph->AddVRXSupport();
	return true;
}
bool RenderGraphPatchLibrary::AddRT_Reflections(RenderGraph * Graph)
{
	Graph->CreateDefGraphWithRT();
	return true;
}

bool RenderGraphPatchLibrary::AddVoxelReflections(RenderGraph * Graph)
{
	if (!RHI::GetRenderSettings()->GetVoxelSet().Enabled)
	{
		return false;
	}
	Graph->CreateDefGraphWithVoxelRT();
	return true;
}

bool RenderGraphPatchLibrary::AddVoxel_RTReflections(RenderGraph * Graph)
{
	Graph->CreateDefGraphWithRT_VOXEL();
	return true;
}
