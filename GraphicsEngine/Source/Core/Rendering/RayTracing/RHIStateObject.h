#pragma once

class DeviceContext;
class Shader;
class Camera;
class ShaderBindingTable;
class RHIStateObject
{
public:
	RHI_API RHIStateObject(DeviceContext* D);
	RHI_API virtual ~RHIStateObject();
	RHI_API virtual void Build() = 0;
	ShaderBindingTable* ShaderTable = nullptr;
	Camera* TempCam = nullptr;
	RHI_API virtual void RebuildShaderTable() = 0;
protected:
	DeviceContext* Device = nullptr;
};

