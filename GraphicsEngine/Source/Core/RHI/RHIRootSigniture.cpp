#include "Stdafx.h"
#include "RHIRootSigniture.h"
#include "Core/Asserts.h"


RHIRootSigniture::RHIRootSigniture()
{}

RHIRootSigniture::~RHIRootSigniture()
{}

void RHIRootSigniture::SetRootSig(std::vector<ShaderParameter>& parms)
{
	Parms = parms;
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