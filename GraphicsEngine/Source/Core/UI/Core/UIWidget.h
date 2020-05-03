#pragma once
#include "Core/Input/InputMouse.h"
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
struct UIInputEvent
{
	glm::ivec2 Pos = glm::ivec2();
	MouseButton::Type Mouse = MouseButton::Limit;
	InputButtonPressType::Type PressType = InputButtonPressType::Press;
	void SetHandled()
	{
		Handled = true;
	}
	bool IsHandled()const
	{
		return Handled;
	}
private:
	bool Handled = false;
};
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
		int Gap = 0;
	};

	Aligment AligmentStruct;
	static glm::vec3 DefaultColour;
	CORE_API UIWidget(int w, int h, int x = 0, int y = 0);
	CORE_API UIWidget();
	virtual ~UIWidget();
	virtual void Render(){};
	virtual void ResizeView(int w, int h, int x = 0, int y = 0);
	void SetRootSpaceSize(int w, int h, int x, int y);
	void SetRootSpaceScaled(int w, int h, int x, int y);
	void SetAbsoluteSize(int w, int h, int x = 0, int y = 0);
	CORE_API void SetScaled(float Width, float height, float xoff = 0.0f, float yoff = 0.0f);

	void SetOwner(UIWidgetContext* wc);
	glm::vec3 Colour;
	bool ContainsPoint(glm::ivec2 Point);
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
	UIWidget* Parent = nullptr;
	bool IsWithinParentBounds();
	std::vector<UIWidget*> Children;
	void AddChild(UIWidget* W);
	void RemoveChild(UIWidget * w);
	void RemoveAllChildren();
	void InvalidateTransform();
	EWidgetBatchMode::Type GetBatchMode();
	bool IgnoreboundsCheck = false;
	EWidetSizeSpace::Type GetScaleMode() const { return Transform.GetScalingMode(); }
	UIWidgetContext* GetOwningContext() const { return OwningContext; }
	void SetOwningContext(UIWidgetContext* val);
	UITransform* GetTransfrom();
	void DebugRenderBounds();
	bool RenderWidgetBounds = false;
	void ReceiveUIInputEvent(UIInputEvent& e);
protected:
	virtual void ProcessUIInputEvent(UIInputEvent& e) {};
	UITransform Transform;
	EWidgetBatchMode::Type BatchMode = EWidgetBatchMode::Auto;
	UIWidgetContext* OwningContext = nullptr;
	float TextDefaultScale = 0.3f;
	bool IsActive = true;
	bool TransfromDirty = false;
	virtual void OnGatherBatches(UIRenderBatch* Groupbatchptr = nullptr);
	glm::ivec2 ConvertScreenToRootSpace(glm::ivec2 In);
private:
	IntRect RootSpaceRect = IntRect();
	EWidetSizeSpace::Type ScaleMode = EWidetSizeSpace::Limit;
	glm::vec3 DebugColour = glm::vec3(1, 1, 1);
};

