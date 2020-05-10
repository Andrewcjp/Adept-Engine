#include "IReflect.h"
#include "Core/Assets/BinaryArchive.h"

void IReflect::SerializeThis(BinaryArchive * A, std::vector<ClassReflectionNode*> &Nodes)
{
	for (int i = 0; i < Nodes.size(); i++)
	{
		if (!A->Reading)
		{
			Nodes[i]->GetFunc();
		}
		if (Nodes[i]->m_Type == MemberValueType::String)
		{
			std::string t = Nodes[i]->GetAsString();
			int Size = t.size() + 1;
			A->LinkHeader(Size);
			char* Data = nullptr;

			Data = new char[Size];
			memset(Data, 0, Size);
			if (!A->Reading)
			{
				memcpy(Data, t.c_str(), Size);
			}
			if (Size > 0)
			{
				A->LinkData(Data, Size);
				if (A->Reading)
				{
					Nodes[i]->SetString(Data);
					delete[] Data;
				}
			}
		}
		else
		{
			A->LinkData(Nodes[i]->m_pDataPtr, Nodes[i]->GetSize());
		}
		if (A->Reading)
		{
			Nodes[i]->SetFunc();
		}
	}
}

void IReflect::Serialize(BinaryArchive* Achive)
{
	SerializeThis(Achive, m_RelfectionData.Data);
}
