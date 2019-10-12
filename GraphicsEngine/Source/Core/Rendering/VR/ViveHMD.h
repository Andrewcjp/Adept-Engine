#pragma once
#include "HMD.h"
#if BUILD_STEAMVR
#include "..\headers\openvr.h"

#include "../Core/Core/Transform.h"

class SteamVRInputInterface;
namespace vr
{
	class IVRSystem;
}
class ViveHMD: public HMD
{
public:
	ViveHMD();
	virtual ~ViveHMD();
	static bool CanCreate();
	void Init();


	virtual void Update() override;

	virtual void OutputToEye(FrameBuffer* buffer, EEye::Type eye) override;
	virtual glm::ivec2 GetDimentions() override;

	void SetPosAndRot(glm::vec3 pos, glm::quat Rot);
	virtual bool IsActive() override;
	SteamVRInputInterface* VRInterface = nullptr;
private:
	float RenderScale = 1.0f;

};

#endif