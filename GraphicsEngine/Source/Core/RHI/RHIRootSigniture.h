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
class RHITexture;
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
		RootConstant,
		TextureArray,
		Texture2,
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
	RHITextureArray* TextureArray = nullptr;
	RHITexture* Texture2 = nullptr;
	RHIViewDesc View = RHIViewDesc();
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
	RHI_API bool IsBound()const;
	bool HasChanged = true;
	static ERSBindType::Type ConvertBind(ShaderParamType::Type T);
};

class RHIRootSigniture
{
public:
	RHI_API RHIRootSigniture();
	RHI_API ~RHIRootSigniture();
	bool IsCompatable(const std::vector<ShaderParameter>& parms);
	RHI_API void SetRootSig(const std::vector<ShaderParameter>& parms);
#if USE_VALIDATION
	bool ValidateData(ShaderParameter * Parm, RSBind & bind);
#endif
	bool ComparePTypes(ShaderParamType::Type T, ERSBindType::Type bindt);
	bool ValidateType(ShaderParameter * Parm, ERSBindType::Type type);
	RHI_API void SetTexture(int slot, BaseTextureRef Tex, RHIViewDesc View = RHIViewDesc());
	RHI_API void SetFrameBufferTexture(int slot, FrameBuffer* Buffer, RHIViewDesc View = RHIViewDesc());
	RHI_API void SetConstantBufferView(int slot, RHIBuffer* Target, int offset = 0, RHIViewDesc View = RHIViewDesc());
	RHI_API void SetBufferReadOnly(int slot, RHIBuffer* Target, const RHIViewDesc & desc = RHIViewDesc());
	RHI_API void SetUAV(int slot, FrameBuffer * target, const RHIViewDesc & view);
	RHI_API void SetUAV(int slot, RHIBuffer* Target, const RHIViewDesc& view);
	RHI_API void SetTexture2(int slot, RHITexture* Target, const RHIViewDesc& view);
	void In_CreateUAV(RSBind &Bind, const RHIViewDesc& view, int slot);

	RHI_API void SetTextureArray(int slot, RHITextureArray* array, const RHIViewDesc& view);
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

