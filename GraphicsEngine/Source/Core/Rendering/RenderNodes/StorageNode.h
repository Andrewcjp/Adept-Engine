#pragma once
namespace EStorageType
{
	enum Type
	{
		Buffer,
		Framebuffer,
		Limit
	};
}
class StorageNode
{
public:
	StorageNode();
	~StorageNode();
	EStorageType::Type StoreType = EStorageType::Limit;
	std::string DataFormat = "";
};

