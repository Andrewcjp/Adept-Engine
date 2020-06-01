#pragma once
#include "ClassReflectionNode.h"

class BinaryArchive;
class Archive;

class IReflect
{
public:
	IReflect();
	virtual ~IReflect() {}
	const ClassReflectionData* AccessReflection()
	{
		Validate();
		return &m_RelfectionData;
	};
	IReflect(const IReflect& a);
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

