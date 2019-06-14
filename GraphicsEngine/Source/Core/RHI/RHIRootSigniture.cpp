#include "RHIRootSigniture.h"

RHIRootSigniture::RHIRootSigniture()
{}

RHIRootSigniture::~RHIRootSigniture()
{}

void RHIRootSigniture::SetRootSig(std::vector<ShaderParameter>& parms)
{
	Parms = parms;
}

bool RHIRootSigniture::ValidateData(ShaderParameter* Parm, RSBind & bind)
{
	return true;
}

bool RHIRootSigniture::ComparePTypes(ShaderParamType::Type T, ERSBindType::Type bindt)
{
	switch (bindt)
	{
		case ERSBindType::FrameBuffer:
		case ERSBindType::Texture:
		case ERSBindType::BufferSRV:
			return T == ShaderParamType::RootSRV || T == ShaderParamType::SRV;
		case ERSBindType::CBV:
			return T == ShaderParamType::CBV || T == ShaderParamType::RootConstant;
		case ERSBindType::UAV:
			return T == ShaderParamType::UAV;
	}
	return false;
}


bool RHIRootSigniture::ValidateType(ShaderParameter* Parm, ERSBindType::Type type)
{
	if (!ComparePTypes(Parm->Type, type))
	{
		return false;
	}

	return true;
}
void RHIRootSigniture::SetTexture(int slot, BaseTextureRef Tex)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::Texture;
	Bind.Texture = Tex;
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(RSSlot != nullptr, "Failed to find slot");
		return;
	}
	CurrnetBinds[RSSlot->RegisterSlot] = Bind;
}

void RHIRootSigniture::SetFrameBufferTexture(int slot, FrameBuffer * Buffer, int resoruceindex)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::FrameBuffer;
	Bind.Framebuffer = Buffer;
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(RSSlot != nullptr, "Failed to find slot");
		return;
	}
	CurrnetBinds[RSSlot->RegisterSlot] = Bind;
}

void RHIRootSigniture::SetConstantBufferView(int slot, RHIBuffer * Target, int offset)
{
	RSBind Bind = {};
	Bind.BindType = ERSBindType::CBV;
	Bind.BufferTarget = Target;
	ShaderParameter* RSSlot = GetParm(slot);
	if (RSSlot == nullptr)
	{
		LogEnsureMsgf(RSSlot != nullptr, "Failed to find slot");
		return;
	}
	CurrnetBinds[RSSlot->RegisterSlot] = Bind;
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