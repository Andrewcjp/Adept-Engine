#pragma once
#include "HMD.h"
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


	virtual bool IsActive() override;

private:
	vr::IVRSystem* system;
};

