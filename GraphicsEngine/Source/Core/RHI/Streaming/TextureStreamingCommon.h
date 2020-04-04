#pragma  once

class TextureAsset;
class BaseTexture;
namespace EStreamPriority
{
	enum Type
	{
		Critial,
		High,
		Medium,
		Low
	};
}
static const int StreamAllValue = -1;
struct TextureStreamRequest
{
	BaseTexture* TargetTexture = nullptr;
	int TargetMip = StreamAllValue;
	int TargetFace = StreamAllValue;
	TextureAsset* Asset = nullptr;
	EStreamPriority::Type Priority = EStreamPriority::Medium;
	void Validate();
};