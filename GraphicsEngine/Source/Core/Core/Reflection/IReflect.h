#pragma once
#include "ClassReflectionNode.h"

class BinaryArchive;
class Archive;

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
	virtual void OnPropertyUpdate(ClassReflectionNode* Node) {}

	virtual void SeralizeText(Archive* A);
	void Validate();
protected:
	uint64 ClassIdHash = 0;
	void SerializeThis(BinaryArchive * A, std::vector<ClassReflectionNode*>& Nodes);
	void SerializeThisText(Archive* A, std::vector<ClassReflectionNode*>& Nodes);
};

