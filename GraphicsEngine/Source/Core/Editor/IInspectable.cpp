#include "stdafx.h"
#include "IInspectable.h"

#if WITH_EDITOR
IInspectable::IInspectable()
{
}


IInspectable::~IInspectable()
{
}

void IInspectable::OnInspected()
{
}

void IInspectable::OnDeselected()
{
}
#endif