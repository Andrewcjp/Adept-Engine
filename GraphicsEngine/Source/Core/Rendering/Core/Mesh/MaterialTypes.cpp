#include "MaterialTypes.h"
#include "Core\Assets\Archive.h"
#include "Core\Assets\Asset types\TextureAsset.h"
#include "RHI\Streaming\TextureStreamingCommon.h"
#include "RHI\Streaming\TextureStreamingEngine.h"
#include "RHI\RHICommandList.h"
#include "Core\Assets\ImageIO.h"
#include "Rendering\Core\Mesh\MaterialTypes.generated.h"
ParmeterBindSet::ParmeterBindSet()
{
	CALL_CONSTRUCTOR();
}

ParmeterBindSet::ParmeterBindSet(const ParmeterBindSet& a)
{
	BindMap = a.BindMap;
	for (auto itor = BindMap.begin(); itor != BindMap.end(); itor++)
	{
		itor->second.InitReflection();
	}
	InitReflection();
}

void ParmeterBindSet::AddParameter(std::string name, ShaderPropertyType::Type tpye)
{
	MaterialShaderParameter p;
	p.PropType = tpye;
	p.OffsetInBuffer = GetSize();
	BindMap.emplace(name, p);
	cachedSize = GetSize();
}

void ParmeterBindSet::SetFloat(std::string name, float f)
{
	auto itor = BindMap.find(name);
	if (itor == BindMap.end())
	{
		Log::LogMessage("Failed to find shader parameter named \"" + name + "\"", Log::Error);
		return;
	}
	float* ptr = (float*)&data[itor->second.OffsetInBuffer];
	*ptr = f;
}
float ParmeterBindSet::GetFloat(std::string name)
{
	auto itor = BindMap.find(name);
	if (itor == BindMap.end())
	{
		Log::LogMessage("Failed to find shader parameter named \"" + name + "\"", Log::Error);
		return 0.0f;
	}
	float* ptr = (float*)&data[itor->second.OffsetInBuffer];
	return *ptr;
}
void ParmeterBindSet::SetTexture(std::string name, TextureAsset* asset)
{
	auto itor = BindMap.find(name);
	if (itor == BindMap.end())
	{
		Log::LogMessage("Failed to find shader parameter named \"" + name + "\"", Log::Error);
		return;
	}
	itor->second.m_TextureAsset.SetAssetDirect(asset);
}
size_t ParmeterBindSet::GetSize()
{
	size_t total = 0;
	std::map<std::string, MaterialShaderParameter>::const_iterator it;
	for (it = BindMap.begin(); it != BindMap.end(); ++it)
	{
		total += it->second.GetSize();
	}
	//if (BindMap.size() == 0)
	//{
	//	return cachedSize;
	//}
	return total;
}

void ParmeterBindSet::AllocateMemeory()
{
	ensure(data == nullptr);
	data = new unsigned char[GetSize()];
	for (int i = 0; i < GetSize(); i++)
	{
		data[i] = 0;
	}
}

ParmeterBindSet::~ParmeterBindSet()
{
	delete[] data;
}

void * ParmeterBindSet::GetDataPtr()
{
	return data;
}



void ParmeterBindSet::BindTextures(RHICommandList * list)
{
	for (auto itor = BindMap.begin(); itor != BindMap.end(); itor++)
	{
		if (itor->second.PropType == ShaderPropertyType::Texture)
		{
			if (itor->second.Handle != nullptr)
			{
				itor->second.Handle->Bind(list, itor->first);
			}
			else
			{
				list->SetTexture(ImageIO::GetDefaultTexture(), itor->first);
			}
		}
	}
}

void ParmeterBindSet::MakeActive()
{
	for (auto itor = BindMap.begin(); itor != BindMap.end(); itor++)
	{
		if (itor->second.Handle == nullptr && itor->second.m_TextureAsset.IsValid())
		{
			itor->second.Handle = TextureStreamingEngine::RequestTexture(itor->second.m_TextureAsset.GetAsset()->GetAssetPath());
		}
	}
}
void ParmeterBindSet::ProcessSerialArchive(Archive * A)
{
	/*for (auto itor = BindMap.begin(); itor != BindMap.end(); itor++)
	{
		std::string name = itor->first;
		ArchiveProp(name);
		ArchiveProp_Enum(itor->second.PropType);
		ArchiveProp_Enum(itor->second.OffsetInBuffer);
	}*/
	std::string Data = std::string((const char*)data, GetSize());
	ArchiveProp(Data);
	ArchiveProp_Enum(cachedSize);
	if (A->IsReading())
	{
		data = (unsigned char*)malloc(cachedSize);
		unsigned char* ptr = (unsigned char*)Data.c_str();
		memcpy(data, ptr, cachedSize);
	}
}

void ParmeterBindSet::SeralizeText(Archive* A)
{
	int MapSize = BindMap.size();
	ArchiveProp(MapSize);
#if 0
	if (A->IsReading())
	{
		for (int i = 0; i < MapSize; i++)
		{
			std::string data;
			ArchiveProp(data);
			BindMap.emplace(data, MaterialShaderParameter());
			//BindMap.at(data).InitReflection();
			//BindMap.at(data).SeralizeText(A);
		}
	}
	else
	{
		for (auto itor = BindMap.begin(); itor != BindMap.end(); itor++)
		{
			std::string data = itor->first;
			ArchiveProp(data);
			itor->second.InitReflection();
			itor->second.Validate();
			itor->second.SeralizeText(A);
		}
	}
#endif
}

MaterialShaderParameter::MaterialShaderParameter()
{
	CALL_CONSTRUCTOR();
}

MaterialShaderParameter::MaterialShaderParameter(const MaterialShaderParameter& other)
{

	OffsetInBuffer = other.OffsetInBuffer;
	PropType = other.PropType;
	m_TextureAsset = other.m_TextureAsset;
	Handle = other.Handle;
	CALL_CONSTRUCTOR();
}

size_t MaterialShaderParameter::GetSize() const
{
	switch (PropType)
	{
	case ShaderPropertyType::Float:
		return sizeof(float);
	}
	return 0;
}
