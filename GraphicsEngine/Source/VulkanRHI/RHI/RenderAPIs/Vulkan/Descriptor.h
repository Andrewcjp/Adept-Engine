#pragma once

class VKanTexture;
class VKanBuffer;
struct EDescriptorType
{
	enum Type
	{
		CBV,
		SRV,
		UAV,
		Limit
	};
};
//handle that is added to command list to build the descriptor set for each draw
class Descriptor
{
public:
	Descriptor(EDescriptorType::Type type);
	~Descriptor();
	EDescriptorType::Type DescType = EDescriptorType::Limit;
	int bindpoint = 0;
	int Offset = 0;
	VKanBuffer* Buffer;
	VKanTexture* Texture;
};

