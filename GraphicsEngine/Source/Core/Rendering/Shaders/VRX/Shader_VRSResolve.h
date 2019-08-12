#pragma once
#include "RHI\Shader.h"

class DeviceContext;
class Shader_VRSResolve : public Shader
{
public:
	Shader_VRSResolve(DeviceContext* device);
	~Shader_VRSResolve();
	DECLARE_GLOBAL_SHADER(Shader_VRSResolve);
	void BindBuffer(RHICommandList* list);
private:
	RHIBuffer* DataBuffer = nullptr;
	struct VRSData
	{
		int Resolution[2];
		bool Debug;
		//bool ShowGrid;
	};
	VRSData DataInst;
};

