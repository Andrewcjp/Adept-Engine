#pragma once
#include "..\RenderNode.h"

class RHIStateObject;
class ShaderBindingTable;
class RayTracingCommandList;
class RHIBuffer;
class PathTraceBindingTable;
class PathTraceSceneNode : public RenderNode
{
public:
	PathTraceSceneNode();
	~PathTraceSceneNode();

	virtual void OnExecute() override;


	virtual std::string GetName() const override;

protected:
	virtual void OnNodeSettingChange() override;


	virtual void OnSetupNode() override;
private:
	RHIStateObject* StateObject = nullptr;
	RayTracingCommandList* RTList = nullptr;
	PathTraceBindingTable* DefaultTable = nullptr;
	struct RTCameraData
	{
		glm::mat4x4 IView;
		glm::mat4x4 IProj;
		glm::vec3 CamPos;
	};
	RTCameraData Data;
	RHIBuffer* CBV = nullptr;
};

