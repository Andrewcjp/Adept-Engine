#pragma once
#include "..\RenderNode.h"
#include "..\..\RayTracing\Tables\ReflectionsBindingTable.h"

class RHIStateObject;
class RayTracingCommandList;
class ShaderBindingTable;
class RHIBuffer;
class RayTraceReflectionsNode : public RenderNode
{
public:
	RayTraceReflectionsNode();
	~RayTraceReflectionsNode();

	virtual void OnExecute() override;
	virtual bool IsNodeSupported(const RenderSettings& settings) override;

	NameNode("RayTraced Reflections");

protected:
	virtual void OnNodeSettingChange() override;
	virtual void OnSetupNode() override;

	virtual void OnValidateNode(RenderGraph::ValidateArgs & args) override;

private:
	RHIStateObject* StateObject = nullptr;
	RayTracingCommandList* RTList = nullptr;
	ReflectionsBindingTable* BindingTable = nullptr;
	struct RTCameraData
	{
		glm::mat4x4 IView;
		glm::mat4x4 IProj;
		glm::vec3 CamPos;
	};
	RTCameraData Data;
	RHIBuffer* CBV = nullptr;
};

