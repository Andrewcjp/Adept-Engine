#include "RHIRootSigniture.h"

RHIRootSigniture::RHIRootSigniture()
{}

RHIRootSigniture::~RHIRootSigniture()
{}

void RHIRootSigniture::SetRootSig(std::vector<ShaderParameter>& parms)
{
	Parms = parms;
	if (parms.size() != CurrnetBinds.size())
	{
		CurrnetBinds.clear();
		CurrnetBinds.resize(parms.size());
		DefaultParams();
		Invalidate();
	}//todo
	
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
			return T == ShaderParamType::RootSRV || T == ShaderParamType::SRV;
		case ERSBindType::CBV:
			return T == ShaderParamType::CBV || T == ShaderParamType::RootConstant || T == ShaderParamType::Buffer;
		case ERSBindType::UAV:
			return T == ShaderParamType::UAV;
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

void RHIRootSigniture::SetTexture(int slot, BaseTextureRef Tex)
{
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(false, "Failed to find slot");
		return;
	}
	RSBind* Bind = &CurrnetBinds[RSSlot->SignitureSlot];
	Bind->BindType = ERSBindType::Texture;
#if USE_VALIDATION
	if (!ValidateType(RSSlot, Bind->BindType))
	{
		LogEnsureMsgf(false, "Invalid Bind");
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
}

void RHIRootSigniture::SetFrameBufferTexture(int slot, FrameBuffer * Buffer, int resoruceindex)
{
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(false, "Failed to find slot");
		return;
	}
	RSBind* Bind = &CurrnetBinds[RSSlot->SignitureSlot];
	Bind->BindType = ERSBindType::FrameBuffer;
#if USE_VALIDATION
	if (!ValidateType(RSSlot, Bind->BindType))
	{
		LogEnsureMsgf(false, "Invalid Bind");
		return;
	}
#endif
	if (Bind->Framebuffer == Buffer && Bind->Offset == resoruceindex)
	{
		return;
	}
	Bind->Framebuffer = Buffer;
	Bind->Offset = resoruceindex;	
	Bind->BindParm = RSSlot;
	Bind->HasChanged = true;
}

void RHIRootSigniture::SetConstantBufferView(int slot, RHIBuffer * Target, int offset)
{

	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(false, "Failed to find slot");
		return;
	}
	RSBind* Bind = &CurrnetBinds[RSSlot->SignitureSlot];
	Bind->BindType = ERSBindType::CBV;
#if USE_VALIDATION
	if (!ValidateType(RSSlot, Bind->BindType))
	{
		LogEnsureMsgf(false, "Invalid Bind");
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

void RHIRootSigniture::SetBufferReadOnly(int slot, RHIBuffer * Target)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::BufferSRV;
	Bind.BufferTarget = Target;
	ShaderParameter* RSSlot = GetParm(slot);
#if USE_VALIDATION
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(false, "Failed to find slot");
		return;
	}
	if (!ValidateType(RSSlot, Bind.BindType))
	{
		LogEnsureMsgf(false, "Invalid Bind");
		return;
	}
#endif
	Bind.BindParm = RSSlot;
	Bind.HasChanged = true;
	CurrnetBinds[RSSlot->SignitureSlot] = Bind;
}

void RHIRootSigniture::SetUAV(int slot, RHIUAV * Target)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::UAV;
	Bind.UAVTarget = Target;
	ShaderParameter* RSSlot = GetParm(slot);
#if USE_VALIDATION
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(false, "Failed to find slot");
		return;
	}
	if (!ValidateType(RSSlot, Bind.BindType))
	{
		LogEnsureMsgf(false, "Invalid Bind");
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

}

void RHIRootSigniture::DefaultParams()
{
	for (int i = 0; i < Parms.size(); i++)
	{
		CurrnetBinds[i].BindParm = &Parms[i];
		CurrnetBinds[i].BindType = RSBind::ConvertBind(Parms[i].Type);
	}
}

ERSBindType::Type RSBind::ConvertBind(ShaderParamType::Type T)
{
	switch (T)
	{
		case ShaderParamType::RootConstant:
			break;
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
