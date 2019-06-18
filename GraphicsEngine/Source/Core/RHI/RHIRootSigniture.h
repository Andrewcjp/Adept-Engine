#pragma once
#include "ShaderBase.h"
#include "BaseTexture.h"

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
	ERSBindType::Type BindType = ERSBindType::Limit;
	/*union
	{*/
	BaseTextureRef Texture;
	FrameBuffer* Framebuffer;
	RHIBuffer* BufferTarget;
	/*};*/
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
};
class RHIRootSigniture
{
public:
	RHIRootSigniture();
	~RHIRootSigniture();
	void SetRootSig(std::vector<ShaderParameter>& parms);
	bool ValidateData(ShaderParameter * Parm, RSBind & bind);
	bool ComparePTypes(ShaderParamType::Type T, ERSBindType::Type bindt);
	bool ValidateType(ShaderParameter * Parm, ERSBindType::Type type);
	void SetTexture(int slot, BaseTextureRef Tex);
	void SetFrameBufferTexture(int slot, FrameBuffer* Buffer, int resoruceindex = 0);
	void SetConstantBufferView(int slot, RHIBuffer* Target, int offset = 0);
	void SetBufferReadOnly(int slot, RHIBuffer* Target);
	void Reset();
	ShaderParameter * GetParm(int slot);
	RHI_API const RSBind* GetBind(int slot)const;
	RHI_API int GetNumBinds()const;
private:
	std::vector<ShaderParameter> Parms;
	std::vector<RSBind> CurrnetBinds;
};

