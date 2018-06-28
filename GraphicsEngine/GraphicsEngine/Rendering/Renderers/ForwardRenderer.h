#pragma once
#include "RenderEngine.h"
#include <vector>
#include "Rendering/Core/Camera.h"
#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "Rendering/Core/Light.h"
#include "RHI/RenderAPIs/D3D12/D3D12RHI.h"
#include "Rendering/Core/Material.h"
#include "RHI/Shader.h"
#include "Rendering\Shaders\Shader_Main.h"
#include "Rendering\Shaders\Shader_Depth.h"
#include "Rendering\Shaders\Shader_Grass.h"
#include "Rendering\Shaders\Shader_Querry.h"
#include "Core/Assets/Scene.h"
#include "Rendering\Shaders\ShaderOutput.h"
#include "Rendering/Core/FrameBuffer.h"
#include "Rendering/Core/ParticleSystem.h"
#include <memory>
#include "Rendering/Core/ShadowRenderer.h"
#include "Rendering/Core/GPUStateCache.h"
#include "Rendering/Core/GrassPatch.h"
#include "Rendering\Shaders\Shader_Skybox.h"
#include "Core/Performance/PerfManager.h"
#include "EngineGlobals.h"
#include "RHI/RHICommandList.h"
#define USED3D12DebugP 0
class ForwardRenderer : public RenderEngine
{
public:
	ForwardRenderer(int width, int height);
	virtual ~ForwardRenderer();
	void OnRender() override;
	void PostInit() override;
	virtual void DestoryRenderWindow() override;
	virtual void FinaliseRender() override;
	virtual void OnStaticUpdate() override;
	void Resize(int width, int height) override;
private:	
	void RenderDebugPlane();
	void MainPass();
	void RenderSkybox();
	RHICommandList* MainCommandList;

	//debug
#if USED3D12DebugP
	class D3D12Plane* debugplane = nullptr;
#endif

};

