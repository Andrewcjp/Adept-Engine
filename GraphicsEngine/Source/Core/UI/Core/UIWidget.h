#pragma once
namespace EWidgetBatchMode
{
	enum Type
	{
		Auto,
		On,
		ForceOff,
		Limit
	};
}
class UIWidgetContext;
struct UIRenderBatch;

typedef unsigned long long UINT_PTR;
#define USE_BATCHED_DRAW 1

#include "UITransform.h"
class UIWidget
{
public:
	struct Aligment
	{
		float SizeMax = 0.05f;
		//float SizeMin = 0.05f;
	};

	Aligment AligmentStruct;
	static glm::vec3 DefaultColour;
	CORE_API UIWidget(int w, int h, int x = 0, int y = 0);
	virtual ~UIWidget();
	virtual void Render() = 0;
	virtual void ResizeView(int w, int h, int x = 0, int y = 0);
	void SetRootSpaceSize(int w, int h, int x, int y);
	void SetRootSpaceScaled(int w, int h, int x, int y);
	void SetAbsoluteSize(int w, int h, int x = 0, int y = 0);
	CORE_API void SetScaled(float Width, float height, float xoff = 0.0f, float yoff = 0.0f);

	void SetOwner(UIWidgetContext* wc);
	glm::vec3 Colour;

	virtual void MouseMove(int x, int y);
	virtual bool MouseClick(int x, int y);
	virtual void MouseClickUp(int x, int y);
	virtual void PostBatchRender();
	int mwidth;
	int mheight;
	int X;
	int Y;
	virtual void ProcessKeyDown(UINT_PTR key);
	void ProcessKeyUp(UINT_PTR);
	CORE_API void SetEnabled(bool state);
	CORE_API bool GetEnabled()
	{
		return IsActive;
	}
	int Priority = 0;
	bool operator<(UIWidget* that) const;
	void InvalidateRenderstate();

	float WidthScale = 0;
	float HeightScale = 0;
	float XoffsetScale = 0;
	float YoffsetScale = 0;
	bool IsPendingKill = false;
	virtual void OnOwnerSet(UIWidgetContext* wc)
	{};

	//tree traveral fucntions;
	virtual void UpdateScaled();
	virtual void UpdateData();
	void GatherBatches(UIRenderBatch* BatchPtr = nullptr);
	UIWidget* Parent;
	bool IsWithinParentBounds();
	std::vector<UIWidget*> Children;
	void AddChild(UIWidget* W);
	void RemoveChild(UIWidget * w);
	void InvalidateTransform();
	EWidgetBatchMode::Type GetBatchMode();
	bool IgnoreboundsCheck = false;
	EWidetSizeSpace::Type GetScaleMode() const { return Transform.GetScalingMode(); }
	UIWidgetContext* GetOwningContext() const { return OwningContext; }
	void SetOwningContext(UIWidgetContext* val);
	UITransform* GetTransfrom();
protected:
	UITransform Transform;
	EWidgetBatchMode::Type BatchMode = EWidgetBatchMode::Auto;
	UIWidgetContext* OwningContext = nullptr;
	float TextDefaultScale = 0.3f;
	bool IsActive = true;
	bool TransfromDirty = false;
	virtual void OnGatherBatches(UIRenderBatch* Groupbatchptr = nullptr);
private:
	IntRect RootSpaceRect = IntRect();
	EWidetSizeSpace::Type ScaleMode = EWidetSizeSpace::Limit;

};

