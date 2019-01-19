#pragma once
struct SFRNode;
///This class controls how the frame is split across Devices
class SFRController
{
public:
	SFRController();
	~SFRController();
	void Tick();
	SFRNode* GetNode(int DeviceIndex);
	void Init();
private:
	std::vector<SFRNode*> Nodes;
};

