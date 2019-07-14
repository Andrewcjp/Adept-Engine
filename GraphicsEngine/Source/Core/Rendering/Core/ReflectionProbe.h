#pragma once
#include "RHI/RHI_inc_fwd.h"
#include "Core/EngineTypes.h"
#include "SceneRenderer.h"

class RHIBuffer;
namespace EReflectionProbeMode
{
	enum Type
	{
		ERealTime,
		EOneSidePreframe,
		Baked,
		Limit
	};
}
class ReflectionProbe
{
public:
	ReflectionProbe(glm::vec3 Pos = glm::vec3(0,0,0));
	~ReflectionProbe();
	FrameBuffer* CapturedTexture = nullptr;
	FrameBuffer* ConvolutionBuffer = nullptr;
	IntPoint GetDimentions()const;
	EReflectionProbeMode::Type ProbeMode = EReflectionProbeMode::Baked;
	bool NeedsCapture() const;
	void SetCaptured();
	bool GetNeedsDownSample() const;
	void SetNeedsDownSample(bool val);
	void BindViews(RHICommandList * List, int index, int slot);
	void UpdateReflectionParams(glm::vec3 lightPos);
	glm::vec3 GetPosition() const;
	void SetPosition(glm::vec3 val);
private:
	glm::vec3 Position = glm::vec3(0, 0, 0);

	IntPoint Dimentions = IntPoint(1024, 1024);
	int ConvolutionRes = 128;
	bool IsCaptured = false;
	bool NeedsDownSample = false;
	MVBuffer CubeMapViews[6];
	float zNear = 0.1f;
	float ZFar = 1000.0f;
	RHIBuffer* RelfectionProbeProjections = nullptr;
};

