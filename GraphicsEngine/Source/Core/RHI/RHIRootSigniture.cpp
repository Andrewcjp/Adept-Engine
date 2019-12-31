#include "RHIRootSigniture.h"
#if 0
#define VD_ASSERT(a,b) LogEnsureMsgf(a,b)
#else
#define VD_ASSERT(a,b) ensureMsgf(a,b)
#endif
RHIRootSigniture::RHIRootSigniture()
{}

RHIRootSigniture::~RHIRootSigniture()
{}
bool RHIRootSigniture::IsCompatable(const std::vector<ShaderParameter>& parms)
{
	if (parms.size() != CurrnetBinds.size())
	{
		return false;
	}
	for (int i = 0; i < parms.size(); i++)
	{
		if (CurrnetBinds[i].BindType == ERSBindType::Limit)
		{
			return false;
		}
		if (CurrnetBinds[i].BindParm->Type != parms[i].Type)
		{
			return false;
		}
	}
	return true;
}

void RHIRootSigniture::SetRootSig(const std::vector<ShaderParameter>& parms)
{
	Parms = parms;
	if (!IsCompatable(parms))
	{
		Reset();
	}
}

#if USE_VALIDATION
bool RHIRootSigniture::ValidateData(ShaderParameter* Parm, RSBind & bind)
{
	return true;
}
#endif
bool RHIRootSigniture::ComparePTypes(ShaderParamType::Type T, ERSBindType::Type bindt)
{
	switch (bindt)
	{
		case ERSBindType::FrameBuffer:
		case ERSBindType::Texture:
		case ERSBindType::BufferSRV:
			return T == ShaderParamType::RootSRV || T == ShaderParamType::SRV || T == ShaderParamType::Buffer;
		case ERSBindType::CBV:
			return T == ShaderParamType::CBV || T == ShaderParamType::RootConstant || T == ShaderParamType::Buffer;
		case ERSBindType::UAV:
			return T == ShaderParamType::UAV;
		case ERSBindType::TextureArray:
			return T == ShaderParamType::SRV;
		case ERSBindType::Texture2:
			return true;
	}
	return false;
}

#if USE_VALIDATION
bool RHIRootSigniture::ValidateType(ShaderParameter* Parm, ERSBindType::Type type)
{
	if (!ComparePTypes(Parm->Type, type))
	{
		return false;
	}

	return true;
}
#endif

void RHIRootSigniture::SetTexture(int slot, BaseTextureRef Tex, RHIViewDesc View)
{
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		VD_ASSERT(false, "Failed to find slot");
		return;
	}
	RSBind* Bind = &CurrnetBinds[RSSlot->SignitureSlot];
	Bind->BindType = ERSBindType::Texture;
#if USE_VALIDATION
	if (!ValidateType(RSSlot, Bind->BindType))
	{
		VD_ASSERT(false, "Invalid Bind");
		return;
	}
#endif
	if (Bind->Texture.Get() == Tex.Get())
	{
		return;
	}
	Bind->Texture = Tex;
	Bind->BindParm = RSSlot;
	Bind->HasChanged = true;
	Bind->View = View;
}

void RHIRootSigniture::SetFrameBufferTexture(int slot, FrameBuffer* Buffer, RHIViewDesc View /*= RHIViewDesc()*/)
{
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		VD_ASSERT(false, "Failed to find slot");
		return;
	}
	RSBind* Bind = &CurrnetBinds[RSSlot->SignitureSlot];
	Bind->BindType = ERSBindType::FrameBuffer;
#if USE_VALIDATION
	if (!ValidateType(RSSlot, Bind->BindType))
	{
		VD_ASSERT(false, "Invalid Bind");
		return;
	}
#endif
	if (Bind->Framebuffer == Buffer && Bind->Offset == View.ResourceIndex)
	{
		return;
	}
	Bind->Framebuffer = Buffer;
	Bind->Offset = View.ResourceIndex;
	Bind->BindParm = RSSlot;
	Bind->HasChanged = true;
	Bind->View = View;
}

void RHIRootSigniture::SetConstantBufferView(int slot, RHIBuffer * Target, int offset, RHIViewDesc View)
{
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		VD_ASSERT(false, "Failed to find slot");
		return;
	}
	RSBind* Bind = &CurrnetBinds[RSSlot->SignitureSlot];
	Bind->BindType = ERSBindType::CBV;
	Bind->View = View;
#if USE_VALIDATION
	if (!ValidateType(RSSlot, Bind->BindType))
	{
		VD_ASSERT(false, "Invalid Bind");
		return;
	}
#endif
	if (Bind->BufferTarget == Target && Bind->Offset == offset)
	{
		return;
	}
	Bind->BufferTarget = Target;
	Bind->Offset = offset;

	Bind->BindParm = RSSlot;
	Bind->HasChanged = true;
}

void RHIRootSigniture::SetBufferReadOnly(int slot, RHIBuffer * Target, const RHIViewDesc & desc)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::BufferSRV;
	Bind.BufferTarget = Target;
	ShaderParameter* RSSlot = GetParm(slot);
#if USE_VALIDATION
	if (RSSlot == nullptr)
	{
		VD_ASSERT(false, "Failed to find slot");
		return;
	}
	if (!ValidateType(RSSlot, Bind.BindType))
	{
		VD_ASSERT(false, "Invalid Bind");
		return;
	}
#endif
	Bind.BindParm = RSSlot;
	Bind.HasChanged = true;
	Bind.View = desc;
	Bind.View.ViewType = EViewType::SRV;
	CurrnetBinds[RSSlot->SignitureSlot] = Bind;
}
void RHIRootSigniture::SetUAV(int slot, FrameBuffer* target, const RHIViewDesc& view)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::UAV;
	Bind.Framebuffer = target;
	return In_CreateUAV(Bind, view, slot);
}

void RHIRootSigniture::SetUAV(int slot, RHITexture * Target, const RHIViewDesc& view)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::UAV;
	Bind.Texture2 = Target;
	return In_CreateUAV(Bind, view, slot);
}

void RHIRootSigniture::SetUAV(int slot, RHIBuffer * Target, const RHIViewDesc& view)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::UAV;
	Bind.BufferTarget = Target;
	return In_CreateUAV(Bind, view, slot);
}

void RHIRootSigniture::SetTexture2(int slot, RHITexture * Target, const RHIViewDesc & view)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::Texture2;
	Bind.Texture2 = Target;
	Bind.View = view;
	ShaderParameter* RSSlot = GetParm(slot);
#if USE_VALIDATION
	if (RSSlot == nullptr)
	{
		VD_ASSERT(false, "Failed to find slot");
		return;
	}
	if (!ValidateType(RSSlot, Bind.BindType))
	{
		VD_ASSERT(false, "Invalid Bind");
		return;
	}
#endif
	Bind.BindParm = RSSlot;
	Bind.HasChanged = true;
	CurrnetBinds[RSSlot->SignitureSlot] = Bind;
}

void RHIRootSigniture::In_CreateUAV(RSBind &Bind, const RHIViewDesc& view, int slot)
{
	Bind.View = view;
	ShaderParameter* RSSlot = GetParm(slot);
#if USE_VALIDATION
	if (RSSlot == nullptr)
	{
		VD_ASSERT(false, "Failed to find slot");
		return;
	}
	if (!ValidateType(RSSlot, Bind.BindType))
	{
		VD_ASSERT(false, "Invalid Bind");
		return;
	}
#endif
	Bind.BindParm = RSSlot;
	Bind.HasChanged = true;
	CurrnetBinds[RSSlot->SignitureSlot] = Bind;
}

void RHIRootSigniture::SetTextureArray(int slot, RHITextureArray* array, const RHIViewDesc& view)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::TextureArray;
	Bind.TextureArray = array;
	Bind.View = view;
	ShaderParameter* RSSlot = GetParm(slot);
#if USE_VALIDATION
	if (RSSlot == nullptr)
	{
		VD_ASSERT(false, "Failed to find slot");
		return;
	}
	if (!ValidateType(RSSlot, Bind.BindType))
	{
		VD_ASSERT(false, "Invalid Bind");
		return;
	}
#endif
	Bind.BindParm = RSSlot;
	Bind.HasChanged = true;
	CurrnetBinds[RSSlot->SignitureSlot] = Bind;
}

void RHIRootSigniture::Reset()
{
	CurrnetBinds.clear();
	CurrnetBinds.resize(Parms.size());
	DefaultParams();
	Invalidate();
}

ShaderParameter* RHIRootSigniture::GetParm(int slot)
{
	if (slot >= Parms.size() || slot < 0)
	{
		return nullptr;
	}
	return &Parms[slot];
}

const RSBind * RHIRootSigniture::GetBind(int slot) const
{
	return &CurrnetBinds[slot];
}

int RHIRootSigniture::GetNumBinds() const
{
	return Parms.size();
}

void RHIRootSigniture::SetUpdated()
{
	for (int i = 0; i < CurrnetBinds.size(); i++)
	{
		CurrnetBinds[i].HasChanged = false;
	}
}

void RHIRootSigniture::Invalidate()
{
	for (int i = 0; i < CurrnetBinds.size(); i++)
	{
		CurrnetBinds[i].HasChanged = true;
	}
}

void RHIRootSigniture::ValidateAllBound()
{
	for (int i = 0; i < CurrnetBinds.size(); i++)
	{
		RSBind* Bind = &CurrnetBinds[i];
		//	ensure(Bind->IsBound());
	}
}

uint RHIRootSigniture::GetMaxDescriptorsNeeded() const
{
	uint count = 0;
	for (int i = 0; i < Parms.size(); i++)
	{
		count += Parms[i].NumDescriptors;
	}
	return count;
}

void RHIRootSigniture::DefaultParams()
{
	for (int i = 0; i < Parms.size(); i++)
	{
		CurrnetBinds[i].BindParm = &Parms[i];
		CurrnetBinds[i].BindType = RSBind::ConvertBind(Parms[i].Type);
	}
}

bool RSBind::IsBound() const
{
	switch (BindType)
	{
		case ERSBindType::Texture:
			return Texture != nullptr;
		case ERSBindType::FrameBuffer:
			return Framebuffer != nullptr;
		case ERSBindType::UAV:
			return BufferTarget != nullptr || Framebuffer != nullptr || Texture2 != nullptr;
		case ERSBindType::BufferSRV:		
			return BufferTarget != nullptr;
		case ERSBindType::CBV://direct in the root sig
		case ERSBindType::RootConstant:
			return true;//not bound here 
		case ERSBindType::TextureArray:
			return TextureArray != nullptr;
		case ERSBindType::Texture2:
			return Texture2 != nullptr;
		case ERSBindType::Limit:
			break;
	}
	return false;
}

ERSBindType::Type RSBind::ConvertBind(ShaderParamType::Type T)
{
	switch (T)
	{
		case ShaderParamType::RootConstant:
			return ERSBindType::RootConstant;
		case ShaderParamType::SRV:
			return ERSBindType::Texture;
		case ShaderParamType::Buffer:
		case ShaderParamType::CBV:
			return ERSBindType::CBV;
		case ShaderParamType::UAV:
			return ERSBindType::UAV;
	}
	return ERSBindType::Limit;
}
