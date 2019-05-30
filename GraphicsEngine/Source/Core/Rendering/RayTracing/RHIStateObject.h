#pragma once

class DeviceContext;
class Shader;
class RHIStateObject
{
public:
	RHI_API RHIStateObject(DeviceContext* D);
	RHI_API virtual ~RHIStateObject();
	RHI_API virtual void Build() = 0;
	Shader* Target = nullptr;
protected:
	DeviceContext* Device = nullptr;
};

