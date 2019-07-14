#include "RayTraceReflectionsNode.h"
#include "Rendering/RayTracing/RayTracingEngine.h"
#include "Rendering/RenderNodes/RenderNode.h"
#include "Rendering/RenderNodes/StorageNodeFormats.h"
#include "RHI/DeviceContext.h"

RayTraceReflectionsNode::RayTraceReflectionsNode()
{
	OnNodeSettingChange();
}


RayTraceReflectionsNode::~RayTraceReflectionsNode()
{}

void RayTraceReflectionsNode::OnExecute()
{
	FrameBuffer* Output = GetFrameBufferFromInput(0);
	FrameBuffer* Gbuffer = GetFrameBufferFromInput(1);

	RayTracingEngine::Get()->BuildStructures();
	RayTracingEngine::Get()->TraceRaysForReflections(Output, Gbuffer);
	PassNodeThough(0, StorageFormats::ScreenReflectionData);
}

bool RayTraceReflectionsNode::IsNodeSupported(const RenderSettings& settings)
{
	if (Context->GetCaps().RTSupport == ERayTracingSupportType::None)
	{
		return false;
	}
	if (!settings.RaytracingEnabled())
	{
		return false;
	}
	return true;
}

void RayTraceReflectionsNode::OnNodeSettingChange()
{
	AddInput(EStorageType::Framebuffer, StorageFormats::DefaultFormat, "OutputBuffer");
	AddInput(EStorageType::Framebuffer, StorageFormats::GBufferData, "GBuffer");
	AddOutput(EStorageType::Framebuffer, StorageFormats::ScreenReflectionData, "Screen Data");
}

void RayTraceReflectionsNode::OnSetupNode()
{

}
