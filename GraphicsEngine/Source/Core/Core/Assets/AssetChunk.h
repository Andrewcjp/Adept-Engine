#pragma once

class BinaryArchive;
struct AssetHeader
{
	uint64 ID = 0;
	uint64 Size = 0;
	std::string FileName;
	void* DataBlob;
};
struct AssetFileHeader
{
	uint64 VersionId = 0;
	uint64 ItemCount = 0;
};
class AssetChunk
{
public:
	void StartWrite(std::string path);
	void Load(std::string FilePath);
	void Close();
	void AddData(BinaryArchive* File);
	AssetHeader* GetNextItem();

private:
	BinaryArchive* m_File = nullptr;
};

