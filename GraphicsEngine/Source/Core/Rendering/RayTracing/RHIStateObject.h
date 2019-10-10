#pragma once
#include "RHI/RHI.h"

class DeviceContext;
class Shader;
class Camera;
class ShaderBindingTable;
struct RHIStateObjectDesc
{
	uint64_t AttibuteSize = 0;
	uint64_t PayloadSize = 0;
	// PERFOMANCE TIP: Set max recursion depth as low as needed 
	// as drivers may apply optimization strategies for low recursion depths.
	uint MaxRecursionDepth = 1;
};
class RHIStateObject : public IRHIResourse
{
public:
	RHI_API RHIStateObject(DeviceContext* D, RHIStateObjectDesc Desc);
	RHI_API virtual ~RHIStateObject();
	RHI_API virtual void Build() = 0;
	ShaderBindingTable* ShaderTable = nullptr;
	RHI_API virtual void RebuildShaderTable() = 0;
protected:
	DeviceContext* Device = nullptr;
	RHIStateObjectDesc Desc;
};

