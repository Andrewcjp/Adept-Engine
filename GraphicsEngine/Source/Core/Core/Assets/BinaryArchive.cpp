#include "BinaryArchive.h"
#include <ios>
#include<iostream>
#include<fstream>


BinaryArchive::BinaryArchive()
{}

BinaryArchive::~BinaryArchive()
{}

void BinaryArchive::Open(std::string Filename, bool Write)
{
	Reading = !Write;
	if (Write)
	{
		OutStream = std::ofstream(Filename, std::ios::out | std::ios::binary);
		ensure(OutStream.is_open());
	}
	else
	{
		InStream = std::ifstream(Filename, std::ios::in | std::ios::binary);
		ensure(InStream.is_open());
	}
	LinkHeader<BinaryArchiveHeader>(Header);
	ensure(Header.Version == AD_BINARCHIVE_VERSION);
}

void BinaryArchive::Close()
{
	if (Reading)
	{
		InStream.close();
	}
	else
	{
		OutStream.close();
	}
}

void BinaryArchive::LinkVector(std::vector<std::string>& Vector)
{
	uint64 VectorSize = Vector.size();
	LinkHeader(VectorSize);
	if (Reading)
	{
		Vector.resize(VectorSize);
	}
	for (int i = 0; i < VectorSize; i++)
	{
		uint64 StringLength = Vector[i].size();
		LinkHeader(StringLength);
		if (Reading)
		{
			std::string V;
			V.resize(StringLength);
			ReadData((char*)V.data(), StringLength);
			Vector[i] = V;
		}
		else
		{
			WriteData((char*)Vector[i].data(), StringLength);
		}
	}
}

void BinaryArchive::LinkData(void* data, uint64 size)
{
	if (Reading)
	{
		ReadData(data, size);
	}
	else
	{
		WriteData(data, size);
	}
}

void BinaryArchive::WriteData(void * data, uint64 size)
{
	OutStream.write((char*)data, size);
}

void BinaryArchive::ReadData(void * data, uint64 size)
{
	InStream.read((char*)data, size);
}

#if 0

std::ofstream InStream = std::ofstream(path, std::ios::out | std::ios::binary);
CSOHeader header;
header.Version = CSO_VERSION;
header.Size = item->Blob->Length;
InStream.write((char*)&header, sizeof(header));
InStream.write((char*)item->Blob->ByteCode, header.Size);
InStream.close();

#endif