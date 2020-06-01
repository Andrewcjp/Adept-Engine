#include "IReflect.h"
#include "Core/Assets/BinaryArchive.h"
#include "Core/Assets/Archive.h"
#include "Core/Assets/AssetPtr.h"
#include "Core/Assets/Asset types/BaseAsset.h"

void IReflect::SeralizeText(Archive * A)
{
	SerializeThisText(A, m_RelfectionData.Data);
}

void IReflect::Validate()
{
	ensure(m_RelfectionData.Owner == this);
}

void IReflect::SerializeThisText(Archive * A, std::vector<ClassReflectionNode*>& Nodes)
{
	for (int i = 0; i < Nodes.size(); i++)
	{
		if (!A->IsReading())
		{
			Nodes[i]->GetFunc();
		}

		if (Nodes[i]->m_Type == MemberValueType::Float)
		{
			float Value = Nodes[i]->GetAsFloat();
			A->LinkProperty(Value, Nodes[i]->m_MemberName.c_str());
			Nodes[i]->SetFloat(Value);
		}
		else if (Nodes[i]->m_Type == MemberValueType::Int)
		{
			int Value = Nodes[i]->GetAsInt();
			A->LinkProperty(Value, Nodes[i]->m_MemberName.c_str());
			Nodes[i]->SetInt(Value);
		}
		else if (Nodes[i]->m_Type == MemberValueType::String)
		{
			std::string v = Nodes[i]->GetAsString();
			A->LinkProperty(v, Nodes[i]->m_MemberName.c_str());
			Nodes[i]->SetString(v);
		}
		else if (Nodes[i]->m_Type == MemberValueType::AssetPtr)
		{
			AssetPtr<BaseAsset>*  asset = Nodes[i]->GetAsT<AssetPtr<BaseAsset>>();
			if (asset != nullptr)
			{
				std::string v = asset->GetAssetPath();
				A->LinkProperty(v, Nodes[i]->m_MemberName.c_str());
				if (A->IsReading())
				{
					asset->SetAsset(v);
				}
			}
		}

		if (A->IsReading())
		{
			Nodes[i]->SetFunc();
		}
	}
}

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


IReflect::IReflect()
{}

IReflect::IReflect(const IReflect & a)
{
	
}

void IReflect::Serialize(BinaryArchive* Achive)
{
	SerializeThis(Achive, m_RelfectionData.Data);
}
