#pragma once

class RHIBuffer;
class RHICommandList;
class DynamicQualityEngine
{
public:
	DynamicQualityEngine();
	~DynamicQualityEngine();
	void Update();

	void UpdateVXRTCrossover();

	void BindRTBuffer(RHICommandList* list, std::string Name);
	void BindRTBuffer(RHICommandList * list, int slot);
	static DynamicQualityEngine* Get();
private:
	RHIBuffer* RTBuffer;
	struct RTBufferData
	{
		float RT_RoughnessThreshold = 0.8f;
		float VX_MaxRoughness = 0.8f;
		float VX_MinRoughness = 0.2f;
		float VX_RT_BlendStart = 0.7f;
		float VX_RT_BlendEnd = 0.8f;
		float VX_RT_BlendFactor = 0.5f;
		int Max_VXRayCount = 5;
		int Max_RTRayCount = 5;
		int FrameCount = 0;
	};
	RTBufferData RTQualityData = {};
};

