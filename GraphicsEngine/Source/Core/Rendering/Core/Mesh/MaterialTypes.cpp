#include "MaterialTypes.h"
#include "Core\Assets\Archive.h"

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

size_t ParmeterBindSet::GetSize()
{
	size_t total = 0;
	std::map<std::string, MaterialShaderParameter>::const_iterator it;
	for (it = BindMap.begin(); it != BindMap.end(); it++)
	{
		total += it->second.GetSize();
	}
	if (BindMap.size() == 0)
	{
		return cachedSize;
	}
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

size_t MaterialShaderParameter::GetSize() const
{
	switch (PropType)
	{
	case ShaderPropertyType::Float:
		return sizeof(float);
	}
	return size_t();
}
