#pragma once

class UIWidgetContext;
namespace EAnchorPoint
{
	enum Type
	{
		None,
		Top,
		Bottom,
		Left,
		Right
	};
}
namespace EAxisStretch
{
	enum Type
	{
		None,
		Width,
		Height,
		WidthRootSpace,
		HeightRootSpace,
		ALL,
	};
}
namespace EWidetSizeSpace
{
	enum Type
	{
		Normalised,
		RootSpace,
		ABS,
		RootSpaceScaled,
		Limit
	};
}
class UITransform
{
public:
	void Set(int width, int height, int x = 0, int y = 0, EWidetSizeSpace::Type SpaceMode = EWidetSizeSpace::RootSpace);

	IntPoint GetPos() const { return Pos; }
	IntPoint GetSize() const { return Size; }
	EAnchorPoint::Type GetAnchourPoint() const { return AnchourPoint; }
	EAxisStretch::Type GetStretchMode() const { return StretchMode; }
	EWidetSizeSpace::Type GetScalingMode() const { return ScalingMode; }
	
	void SetStretchMode(EAxisStretch::Type val) { StretchMode = val; }
	void SetAnchourPoint(EAnchorPoint::Type val) { AnchourPoint = val; }
	void SetPos(IntPoint val);
	void SetSize(IntPoint val);
	void SetScalingMode(EWidetSizeSpace::Type val) { ScalingMode = val; }

	glm::vec2 GetPositionForWidget();
	int GetWidthScaled(float v);
	int GetHeightScaled(float v);
	glm::vec2 GetTransfromedSize();
	glm::vec4 GetTransfromRect();
	void SetContext(UIWidgetContext* val) { Context = val; }
private:

	UIWidgetContext* Context = nullptr;
	EAxisStretch::Type StretchMode = EAxisStretch::None;
	EAnchorPoint::Type AnchourPoint = EAnchorPoint::None;
	EWidetSizeSpace::Type ScalingMode = EWidetSizeSpace::Normalised;
	IntPoint Pos;
	IntPoint Size;
	IntPoint RectMax;
};

