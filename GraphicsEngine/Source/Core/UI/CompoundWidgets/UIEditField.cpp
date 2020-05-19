
#include "UIEditField.h"
#include "UI/BasicWidgets/UILabel.h"
#include "Editor/EditorWindow.h"
#include <cctype>
#include "UI/UIManager.h"
#include "UI/CompoundWidgets/UIButton.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/Platform/Windows/WindowsWindow.h"
#include "Core/Input/TextInputHandler.h"
#include "Core/Utils/StringUtil.h"
#include "Core/Assets/Asset types/BaseAsset.h"
#include "Core/Assets/AssetPtr.h"
UIEditField::UIEditField(int w, int h, int x, int y) :UIBox(w, h, x, y)
{
	Colour = colour;
	//Namelabel = new UILabel("name ", w / 2, h, x, y);
	Textlabel = new UILabel(" data", w, h, x, y);
	//AddChild(Namelabel);
	AddChild(Textlabel);
	Rect = CollisionRect(w, h, x, y);
	Enabled = true;
}

UIEditField::UIEditField(ClassReflectionNode* Targetprop) : UIEditField(0, 0, 0, 0)
{
	FilterType = Targetprop->m_Type;
	//	Namelabel->SetText(Targetprop->GetDisplayName());
	Node = Targetprop;
	if (Targetprop->m_Type == MemberValueType::Bool)
	{
		Textlabel->SetEnabled(false);
	}
	else
	{
		Textlabel->SetText(nextext);
	}

	if (FilterType != MemberValueType::Bool)
	{
		//TextBox = new UIBox(0, 0, 0, 0);

		BackgoundColour = glm::vec3(0.25f);
		Colour = glm::vec3(0.7f);
		//AddChild(TextBox);
	}
	else if (FilterType == MemberValueType::Bool)
	{
		Toggle = new UIButton(0, 0, 0, 0);
		Toggle->BackgoundColour = glm::vec3(0.25f);
		Toggle->Colour = glm::vec3(0.7f);
		Toggle->BindTarget(std::bind(&UIEditField::SendValue, this));
		Toggle->Priority = Priority + 1;
		AddChild(Toggle);
	}
	SupportsScroll = (FilterType == MemberValueType::Float);//todo: int
	Update();
	//AddChild(Namelabel);
	AddChild(Textlabel);
}

UIEditField::~UIEditField()
{}

void UIEditField::SetLabel(std::string lavel)
{
	//	Namelabel->SetText(lavel);
}

void UIEditField::MouseMove(int x, int y)
{
	if (!Enabled)
	{
		return;
	}
	if (Scrolling)
	{
		if (Node == nullptr)
		{
			Scrolling = false;
		}
		else
		{
			Node->SetFloat(StartValue + ((startx - x)*ScrollScale));
			GetValueText(nextext);
			Textlabel->SetText(nextext);
			SendValue();
			return;
		}
	}
	if (FilterType == MemberValueType::Bool)
	{
		Toggle->MouseMove(x, y);
		return;
	}
	if (ContainsPoint(ConvertScreenToRootSpace(glm::ivec2(x, y))))
	{
		WasSelected = true;
		PlatformWindow::SetCursorType(GenericWindow::CursorType::IBeam);
	}
	else
	{
		if (WasSelected)
		{
			//works currently but might leak resources;			
			PlatformWindow::SetCursorType(GenericWindow::CursorType::Normal);
			WasSelected = false;
		}
	}
}

void UIEditField::GetValueText(std::string & string)
{
	if (FilterType == MemberValueType::Float)
	{
		float t = Node->GetAsFloat();
		string = StringUtils::ToString(t);
	}
	if (FilterType == MemberValueType::Bool)
	{
		Toggle->SetCheckBoxState(Node->GetAsBool());
	}
	if (FilterType == MemberValueType::String)
	{
		string = Node->GetAsString();
	}
	if (FilterType == MemberValueType::Vector3)
	{
		if (Targetcomponent == X)
		{
			string = StringUtils::ToString(Node->GetAsFloat3().x);
		}
		else if (Targetcomponent == Y)
		{
			string = StringUtils::ToString(Node->GetAsFloat3().y);
		}
		else if (Targetcomponent == Z)
		{
			string = StringUtils::ToString(Node->GetAsFloat3().z);
		}
	}
	if (FilterType == MemberValueType::AssetPtr)
	{
		AssetPtr<BaseAsset>*  asset = Node->GetAsT<AssetPtr<BaseAsset>>();
		if (asset != nullptr && asset->IsValid())
		{
			string = asset->GetAsset()->GetName();
		}
	}
}
bool UIEditField::MouseClick(int x, int y)
{
	if (FilterType == MemberValueType::Bool)
	{
		Toggle->MouseClick(x, y);
		return false;
	}
	if (ContainsPoint(ConvertScreenToRootSpace(glm::ivec2(x, y))))
	{
		nextext = Textlabel->GetText();
		LastText = Textlabel->GetText();
		if (Node != nullptr)
		{
			GetValueText(nextext);
		}
		ProcessKeyDown(0);
		MemberValueType::Type Txtboxfilter = FilterType;
		if (FilterType == MemberValueType::Vector3 || FilterType == MemberValueType::Vector2 || FilterType == MemberValueType::Vector4)
		{
			Txtboxfilter = MemberValueType::Float;
		}
		TextInputHandler::Get()->SetInputContext(this, Txtboxfilter);
	}
	else
	{
		if (TextInputHandler::Get()->IsUsing(this))
		{
			TextInputHandler::Get()->AcceptValue();
		}
	}
	if (ValueDrawChangeRect.Contains(x, y))
	{
		if (Node != nullptr)
		{
			if (FilterType == MemberValueType::Float)
			{
				Scrolling = true;
				startx = x;
				StartValue = Node->GetAsFloat();
			}
		}
	}
	return true;
}

void UIEditField::MouseClickUp(int x, int y)
{
	Scrolling = false;
}

void UIEditField::Render()
{
	UIBox::Render();
	if (FilterType != MemberValueType::String && FilterType != MemberValueType::Bool)
	{
		TextBox->Render();
		Textlabel->Render();
	}
	if (FilterType == MemberValueType::Bool)
	{
		Toggle->Render();
	}
	//Namelabel->Render();
	if (Node != nullptr && !(TextInputHandler::Get()->IsUsing(this)))
	{
		GetValueText(nextext);
		Textlabel->SetText(nextext);
	}
}
void UIEditField::Update()
{
	if (Node != nullptr)
	{
		GetValueText(nextext);
		Textlabel->SetText(nextext);
	}
}
void UIEditField::SendValue()
{
	if (Node != nullptr)
	{
		if (FilterType == MemberValueType::String)
		{
			Node->SetString(nextext);
		}
		else if (FilterType == MemberValueType::Float)
		{
			float out = (float)atof(nextext.c_str());

			Node->SetFloat(out);
		}
		else if (FilterType == MemberValueType::Bool)
		{
			Node->SetBool(!Node->GetAsBool());
			//	Toggle->SetText(Node->GetAsBool() ? "True" : "False");
			Toggle->SetCheckBoxState(Node->GetAsBool());
		}
		else if (FilterType == MemberValueType::Int)
		{
			int out = (int)atoi(nextext.c_str());
			Node->SetInt(out);
		}
		if (FilterType == MemberValueType::Vector3)
		{
			float out = (float)atof(nextext.c_str());
			glm::vec3 v = Node->GetAsFloat3();
			if (Targetcomponent == X)
			{
				v.x = out;
			}
			else if (Targetcomponent == Y)
			{
				v.y = out;
			}
			else if (Targetcomponent == Z)
			{
				v.z = out;
			}
			Node->SetFloat3(v);
		}
#if WITH_EDITOR
		/*if (Property.ChangesEditor)
		{
			UIManager::instance->RefreshGameObjectList();
		}*/
#endif
	}
}

bool UIEditField::CheckValidInput(char c)
{
	if (FilterType == MemberValueType::String)
	{
		return true;
	}
	else if (FilterType == MemberValueType::Int || FilterType == MemberValueType::Float)
	{
		std::string Filter = "1234567890.";
		std::string IntFilter = "1234567890";
		if (FilterType == MemberValueType::Float)
		{
			return (Filter.find(c) != -1);
		}
		else if (FilterType == MemberValueType::Int)
		{
			return (IntFilter.find(c) != -1);
		}

	}
	return false;
}

void UIEditField::UpdateScaled()
{
	UIBox::UpdateScaled();
	int w = GetTransfrom()->GetSizeRootSpace().x;
	int h = GetTransfrom()->GetSizeRootSpace().y;
	Textlabel->TextScale = 0.3f;

	Textlabel->SetRootSpaceSize(w, h, 0, 0);
	if (FilterType == MemberValueType::Bool)
	{
		Toggle->SetRootSpaceSize(w, h, 0, 0);
		Enabled = false;
	}
	else
	{
		Enabled = true;
	}
	Rect = CollisionRect(w, h, 0, 0);
	if (FilterType == MemberValueType::Bool)
	{
		Toggle->SetRootSpaceSize(h, h, (w / 2) - h / 2, 0);
		Toggle->SetCheckBox();
		SkipRender = true;
	}
}

void UIEditField::ProcessUIInputEvent(UIInputEvent& e)
{
	if (e.Mouse == MouseButton::ButtonRight)
	{
		PlatformContextMenu Menu;
		Menu.AddItem("Play");
		Menu.AddItem("exit");
#ifdef PLATFORM_WINDOWS
		PlatformWindow::ShowContextMenu(Menu);
#endif
		e.SetHandled();
	}
}

void UIEditField::ReceiveCommitedText(const std::string& text)
{
	Textlabel->SetText(text);
	nextext = text;
	SendValue();
}

std::string UIEditField::GetStartValue()
{
	return nextext;
}

void UIEditField::OnUpdate(const std::string & DisplayText)
{
	Textlabel->SetText(DisplayText);
}
