#include "Source/Core/Stdafx.h"
#include "BaseTexture.h"

BaseTexture::~BaseTexture()
{}

DeviceContext * BaseTexture::GetContext()const
{
	return Context;
}

