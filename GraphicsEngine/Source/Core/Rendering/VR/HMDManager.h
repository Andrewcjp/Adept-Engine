#pragma once
namespace EVRMirrorMode
{
	enum Type
	{
		None,
		Both,
		Left,
		Right,
		Limit
	};
}
struct VRSettings
{
	EVRMirrorMode::Type MirrorMode = EVRMirrorMode::Both;
	float EyeDistance = -0.3f;
};
class HMD;
class HMDManager
{
public:
	HMDManager();
	~HMDManager();
	void Init();
	HMD* GetHMD();
	VRSettings VrSettings;
	void Update();
private:
	HMD* HeadSet = nullptr;	
};

