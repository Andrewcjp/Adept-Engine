#pragma once
#include "Core/Reflection/ClassReflectionNode.h"
class IReflect
{
public:
	const ClassReflectionData* AccessReflection()const
	{
		return &m_RelfectionData;
	};
	ClassReflectionData m_RelfectionData;
private:
};

