#pragma once
#include "ShaderBase.h"
#include "BaseTexture.h"
#if BUILD_SHIPPING
#define USE_VALIDATION 0
#else
#define USE_VALIDATION 1
#endif
class RHIBuffer;
class FrameBuffer;
//this handles DXR local root signature bindings.
namespace ERSBindType
{
	enum Type
	{
		Texture,
		FrameBuffer,
		BufferSRV,
		CBV,
		UAV,
		Limit
	};
};

struct RSBind
{
	RSBind() {}
	~RSBind() {}
	ShaderParameter* BindParm = nullptr;
	ERSBindType::Type BindType = ERSBindType::Limit;
	int Offset = 0;
	/*union
	{*/
	BaseTextureRef Texture;
	FrameBuffer* Framebuffer = nullptr;
	RHIBuffer* BufferTarget = nullptr;
	RHIUAV* UAVTarget = nullptr;
	/*};*/
#if 0
	RSBind(const RSBind &other)
	{
		BindType = other.BindType;
		switch (BindType)
		{
			case ERSBindType::Texture:
				Texture = other.Texture;
				break;
			case ERSBindType::FrameBuffer:
				Framebuffer = other.Framebuffer;
				break;
			case ERSBindType::CBV:
			case ERSBindType::BufferSRV:
				BufferTarget = other.BufferTarget;
				break;
		}
	}
	RSBind &operator=(const RSBind &other)
	{
		BindType = other.BindType;
		switch (BindType)
		{
			case ERSBindType::Texture:
				Texture = other.Texture;
				break;
			case ERSBindType::FrameBuffer:
				Framebuffer = other.Framebuffer;
				break;
			case ERSBindType::CBV:
			case ERSBindType::BufferSRV:
				BufferTarget = other.BufferTarget;
				break;
		}

		return *this;
	}
#endif
	bool HasChanged = true;
	static ERSBindType::Type ConvertBind(ShaderParamType::Type T);
};

class RHIRootSigniture
{
public:
	RHI_API RHIRootSigniture();
	RHI_API ~RHIRootSigniture();
	RHI_API void SetRootSig(std::vector<ShaderParameter>& parms);
#if USE_VALIDATION
	bool ValidateData(ShaderParameter * Parm, RSBind & bind);
#endif
	bool ComparePTypes(ShaderParamType::Type T, ERSBindType::Type bindt);
	bool ValidateType(ShaderParameter * Parm, ERSBindType::Type type);
	RHI_API void SetTexture(int slot, BaseTextureRef Tex);
	RHI_API void SetFrameBufferTexture(int slot, FrameBuffer* Buffer, int resoruceindex = 0);
	RHI_API void SetConstantBufferView(int slot, RHIBuffer* Target, int offset = 0);
	RHI_API void SetBufferReadOnly(int slot, RHIBuffer* Target);
	RHI_API void SetUAV(int slot, RHIUAV* Target);
	RHI_API void Reset();
	ShaderParameter * GetParm(int slot);
	RHI_API const RSBind* GetBind(int slot)const;
	RHI_API int GetNumBinds()const;
	RHI_API void SetUpdated();
	RHI_API void Invalidate();
	RHI_API void ValidateAllBound();
private:
	void DefaultParams();
	std::vector<ShaderParameter> Parms;
	std::vector<RSBind> CurrnetBinds;
};

