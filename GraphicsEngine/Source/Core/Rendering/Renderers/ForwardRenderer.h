#pragma once
#include "RenderEngine.h"

#include "Rendering/Core/Camera.h"
#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "Rendering/Core/Light.h"
#include "Rendering/Core/Material.h"
#include "RHI/Shader.h"
#include "Rendering/Shaders/Shader_Main.h"
#include "Rendering/Shaders/Shader_Depth.h"
#include "Rendering/Shaders/Shader_Grass.h"
#include "Core/Assets/Scene.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Rendering/Shaders/Shader_Skybox.h"
#include "Core/Performance/PerfManager.h"
#include "RHI/RHICommandList.h"

#define USED3D12DebugP 1
struct DeviceObjects
{
	RHICommandList* MainCommandList = nullptr;
	FrameBuffer* FrameBuffer = nullptr;
};
class ForwardRenderer : public RenderEngine
{
public:
	ForwardRenderer(int width, int height);
	virtual ~ForwardRenderer();
	void OnRender() override;
	void PostInit() override;
	void SetupOnDevice(DeviceContext * TargetDevice);
	void CubeMapPass();
	
	virtual void DestoryRenderWindow() override;
	virtual void FinaliseRender() override;
	virtual void OnStaticUpdate() override;
	void Resize(int width, int height) override;

private:
	int DevicesInUse = 1;
	void MainPass(RHICommandList * Cmdlist);
	void RenderSkybox();
	DeviceObjects Objects[2];
	RHICommandList* CubemapCaptureList = nullptr;
	//debug
#if USED3D12DebugP
	class D3D12Plane* debugplane = nullptr;
#endif
	std::vector< class RelfectionProbe*> probes;
};

