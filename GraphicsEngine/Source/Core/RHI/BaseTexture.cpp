
#include "BaseTexture.h"

BaseTexture::~BaseTexture()
{}

DeviceContext * BaseTexture::GetContext()const
{
	return Context;
}

