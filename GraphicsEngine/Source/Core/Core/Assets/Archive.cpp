#include "Stdafx.h"
#include "Archive.h"
#include "Core/Assets/SceneJSerialiser.h"
#include "Core/Platform/PlatformCore.h"
#include "Core/GameObject.h"
#include "Core/Assets/Scene.h"
#include "rapidjson\prettywriter.h"
#include "Core/Transform.h"
#include "Core/Components/Component.h"
#include "Core/Components/CompoenentRegistry.h"
#define SERAL_VERSION_NUMBER 1

#define IN_ArchiveProp(Property) LinkProperty(Property,#Property);
Archive::Archive(std::string FilePath, bool Write)
{
	FileName = FilePath;
	IsAchiveReading = !Write;
	Init();
}

Archive::~Archive()
{}

void Archive::LinkProperty(glm::vec3 & Value, const char * PropName)
{
	if (IsReading())
	{
		std::string key = (CurrentReadHead->name.GetString());
		ensure(key == PropName);
		Value = SerialHelpers::GetFloatVec3(CurrentReadHead->value);
		CurrentReadHead++;
	}
	else
	{
		SerialHelpers::addVector(*valueptr, *jallocator, PropName, Value);
	}
}

void Archive::LinkProperty(Transform * Value, const char * PropName)
{
	Value->Serilise(this);
}


void Archive::LinkProperty(int & Value, const char * PropName)
{
	if (IsReading())
	{
		std::string key = (CurrentReadHead->name.GetString());
		ensure(key == PropName);
		Value = CurrentReadHead->value.GetInt();
		CurrentReadHead++;
	}
	else
	{
		SerialHelpers::addLiteral(*valueptr, *jallocator, PropName, Value);
	}
}

void Archive::LinkProperty(float & Value, const char * PropName)
{
	if (IsReading())
	{
		ensure(CurrentReadHead->name.IsString());
		std::string key = (CurrentReadHead->name.GetString());
		ensure(key == PropName);
		Value = CurrentReadHead->value.GetFloat();
		CurrentReadHead++;
	}
	else
	{
		SerialHelpers::addLiteral(*valueptr, *jallocator, PropName, Value);
	}
}

void Archive::LinkProperty(bool & Value, const char * PropName)
{
	if (IsReading())
	{
		std::string key = (CurrentReadHead->name.GetString());
		ensure(key == PropName);
		ensure(CurrentReadHead->value.IsBool());
		Value = CurrentReadHead->value.GetBool();
		CurrentReadHead++;
	}
	else
	{
		SerialHelpers::addBool(*valueptr, *jallocator, PropName, Value);
	}
}
void Archive::LinkProperty(std::string & Value, const char * PropName)
{
	if (IsReading())
	{
		const std::string key = (CurrentReadHead->name.GetString());
		ensure(key == PropName);
		ensure(CurrentReadHead->value.IsString());
		Value = CurrentReadHead->value.GetString();
		CurrentReadHead++;
	}
	else
	{
		SerialHelpers::addString(*valueptr, *jallocator, PropName, Value);
	}
}

void Archive::LinkProperty(std::vector<Component*> & Value, const char * PropName)
{
	if (IsReading())
	{
		auto t = CurrentReadHead->value.GetArray();
		t.begin();
		for (unsigned int i = 0; i < t.Size(); i++)
		{
			Component* newc = nullptr;
			rapidjson::Value*  cv = &t[i];
			for (auto& cit = cv->MemberBegin(); cit != cv->MemberEnd(); cit++)
			{
				//read the first part of the object for the components ID
				if (cit->name == "TypeID")
				{
					newc = CompoenentRegistry::CreateBaseComponent((CompoenentRegistry::BaseComponentTypes)cit->value.GetInt());
					CurrentReadHead++;
				}
			}
			CurrentReadHead = cv->MemberBegin();
			CurrentReadHead++;
			if (newc != nullptr)
			{
				newc->ProcessSerialArchive(this);
				Value.push_back(newc);
				//AttachComponent(newc);
			}
		}
	}
	else
	{
		rapidjson::Value comp(rapidjson::kArrayType);
		rapidjson::Value* PreValueptr = valueptr;
		for (int i = 0; i < Value.size(); i++)
		{
			rapidjson::Value jsv(rapidjson::kObjectType);
			valueptr = &jsv;
			Value[i]->ProcessSerialArchive(this);
			comp.PushBack(jsv, *jallocator);
		}
		valueptr = PreValueptr;
		SerialHelpers::addJsonValue(*valueptr, *jallocator, PropName, comp);
	}
}

void Archive::LinkProperty(Scene* Value, const char * PropName)
{
	int VersionNumber = SERAL_VERSION_NUMBER;
	if (IsReading())
	{		
		CurrentReadHead = doc.MemberBegin();
	}
	if (!IsReading())
	{
		rapidjson::Value jsongovalue(rapidjson::kObjectType);
		valueptr = &jsongovalue;
		IN_ArchiveProp(VersionNumber);
		SerialHelpers::addJsonValue(doc, doc.GetAllocator(), "SceneData", jsongovalue);
	}
	else
	{
		Scope_PopReadHead(CurrentReadHead);
		IN_ArchiveProp(VersionNumber);
		ensureFatalMsgf(SERAL_VERSION_NUMBER == VersionNumber, "Incorrect Version from file");
	}
	if (IsReading())
	{
		const auto& gos = doc.FindMember("GOS");
		if (gos == doc.MemberEnd())
		{
			return;
		}
		for (unsigned int i = 0; i < gos->value.Size(); i++)
		{
			GameObject* NewGo = new GameObject();
			valueptr = &gos->value[i];
			CurrentReadHead = valueptr->MemberBegin();
			NewGo->ProcessSerialArchive(this);
			Value->AddGameobjectToScene(NewGo);
		}
	}
	else
	{
		std::vector<GameObject*> objects = *Value->GetObjects();
		rapidjson::Value jsonGOs(rapidjson::kArrayType);
		for (int i = 0; i < objects.size(); i++)
		{
			rapidjson::Value jsongovalue(rapidjson::kObjectType);
			valueptr = &jsongovalue;
			objects[i]->ProcessSerialArchive(this);
			jsonGOs.PushBack(jsongovalue, doc.GetAllocator());
		}
		SerialHelpers::addJsonValue(doc, doc.GetAllocator(), "GOS", jsonGOs);
	}
}

void Archive::Write()
{
	rapidjson::StringBuffer sb;
	rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(sb);
	doc.Accept(writer);    // Accept() traverses the DOM and generates Handler events.
	SceneJSerialiser::WriteToFile(FileName, sb.GetString());
}

bool Archive::IsReading() const
{
	return IsAchiveReading;
}

void Archive::Init()
{
	if (IsAchiveReading)
	{
		std::string FileData = SceneJSerialiser::GetFile(FileName);
		rapidjson::StringStream JStream(FileData.c_str());
		doc.ParseStream(JStream);
	}
	else
	{
		jallocator = &doc.GetAllocator();
		doc.SetObject();
		valueptr = new rapidjson::Value(rapidjson::kObjectType);

	}
}
