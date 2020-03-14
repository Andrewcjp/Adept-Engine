#pragma once
#define AD_BINARCHIVE_VERSION 1
struct BinaryArchiveHeader
{
	int Version = AD_BINARCHIVE_VERSION;
};
class BinaryArchive
{
public:
	BinaryArchive();
	~BinaryArchive();
	void Open(std::string Filename, bool Write = false);
	void Close();
	template<class T>
	void LinkHeader(T& data)
	{
		if (Reading)
		{
			ReadData(&data, sizeof(T));
		}
		else
		{
			WriteData(&data, sizeof(T));
		}
	}

	template<class T>
	void LinkVector(std::vector<T>& Vector)
	{
		uint64 VectorSize = Vector.size();
		LinkHeader(VectorSize);
		if (Reading)
		{
			Vector.resize(VectorSize);
			ReadData(Vector.data(), VectorSize * sizeof(T));
		}
		else
		{
			WriteData(Vector.data(), VectorSize * sizeof(T));
		}
	}
	void LinkVector(std::vector<std::string>& Vector);
	void LinkData(void* data, uint64 size);

	void WriteData(void* data, uint64 size);
	void ReadData(void* data, uint64 size);

	bool Reading = false;
private:
	std::ofstream OutStream;
	std::ifstream InStream;
	BinaryArchiveHeader Header;
};

