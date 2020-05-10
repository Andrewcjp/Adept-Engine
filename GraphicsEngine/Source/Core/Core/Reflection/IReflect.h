#pragma once
#include "ClassReflectionNode.h"

class BinaryArchive;

class IReflect
{
public:
	virtual ~IReflect() {}
	const ClassReflectionData* AccessReflection()const
	{
		return &m_RelfectionData;
	};
	ClassReflectionData m_RelfectionData;
	virtual void Serialize(BinaryArchive* Achive);
	uint64 GetId()const { return ClassIdHash; }
protected:
	uint64 ClassIdHash = 0;
	void SerializeThis(BinaryArchive * A, std::vector<ClassReflectionNode*>& Nodes);
};

