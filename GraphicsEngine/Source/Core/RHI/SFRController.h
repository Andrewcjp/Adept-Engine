#pragma once
struct SFRNode;
///This class controls how the frame is split across Devices
class SFRController
{
public:
	SFRController();
	~SFRController();
	void Tick();
	void Resize();
	void DualUpdatePC(float splittest);
	SFRNode* GetNode(int DeviceIndex);
	void Init();
	float DynamicAdjustSpeed = 0.5f;
	float PCMaxdelta = 0.05f;
	bool AllowDynmaicChanging = false;
private:
	float CurrentCoolDown = 0.0f;
	float CurrnetPC = 0.5f;
	float LastPc = 0.0f;
	std::vector<SFRNode*> Nodes;
};

