#pragma once
#pragma warning(push,0)
#include "rapidjson\document.h"
#pragma warning(pop)
#include <functional>
#include "SerialHelpers.h"
#define SERAL_VERSION_NUMBER 1
class Archive
{
public:
	Archive(std::string FilePath, bool Write = false);
	~Archive();

	//Basic Types
	CORE_API void LinkProperty(int & Value, const char * PropName);
	CORE_API void LinkProperty(float & Value, const char * PropName);
	CORE_API void LinkProperty(bool & Value, const char * PropName);
	CORE_API void LinkProperty(std::string & Value, const char * PropName);
	CORE_API void LinkProperty(glm::vec3 & Value, const char* PropName);
	CORE_API void LinkProperty(glm::vec2 & value, const char* PropName);
	//Complex Core Types
	CORE_API void LinkProperty(std::vector<class Component*>& Value, const char * PropName);
	void HandleArchiveBody(std::string Name,int Version = SERAL_VERSION_NUMBER);
	void EndHeaderWrite(std::string Name);
	CORE_API void LinkProperty(class Transform * Value, const char * PropName);
	CORE_API void LinkProperty(class Scene * Value, const char * PropName);

	void Write();
	bool IsReading()const;

	template<class T>
	void LinkEnumProperty(T & Value, const char * PropName)
	{
		int ValueTmp = (int)Value;
		LinkProperty(ValueTmp, PropName);
		if (IsReading())
		{
			Value = (T)ValueTmp;
		}
	}

	void LinkStringArray(std::vector<std::string> & Values, const char * PropName);
	template<class T>
	void LinkPropertyArray(std::vector<T*> & Value, const char * PropName, std::function<void(Archive*, T*)> func)
	{
		if (IsReading())
		{
			rapidjson::Value::MemberIterator starthead = CurrentReadHead;
			auto t = CurrentReadHead->value.GetArray();
			t.begin();
			for (unsigned int i = 0; i < t.Size(); i++)
			{
				T* newc = new T();
				rapidjson::Value* cv = &t[i];
				CurrentReadHead = cv->MemberBegin();
				func(this, newc);
				if (newc != nullptr)
				{
					Value.push_back(newc);
				}
			}
			CurrentReadHead = starthead;
			CurrentReadHead++;
		}
		else
		{
			rapidjson::Value comp(rapidjson::kArrayType);
			rapidjson::Value* PreValueptr = valueptr;
			for (int i = 0; i < Value.size(); i++)
			{
				rapidjson::Value jsv(rapidjson::kObjectType);
				valueptr = &jsv;
				//Value[i]->ProcessSerialArchive(this);
				func(this, Value[i]);
				comp.PushBack(jsv, *jallocator);
			}
			valueptr = PreValueptr;
			SerialHelpers::addJsonValue(*valueptr, *jallocator, PropName, comp);
		}
	}
	template<class T>
	void LinkPropertyArrayValue(std::vector<T> & Value, const char * PropName, std::function<void(Archive*, T&)> func)
	{
		if (IsReading())
		{
			rapidjson::Value::MemberIterator starthead = CurrentReadHead;
			auto t = CurrentReadHead->value.GetArray();
			t.begin();
			for (unsigned int i = 0; i < t.Size(); i++)
			{
				T newc = T();
				rapidjson::Value* cv = &t[i];
				CurrentReadHead = cv->MemberBegin();
				func(this, newc);				
				Value.push_back(newc);
			}
			CurrentReadHead = starthead;
			CurrentReadHead++;
		}
		else
		{
			rapidjson::Value comp(rapidjson::kArrayType);
			rapidjson::Value* PreValueptr = valueptr;
			for (int i = 0; i < Value.size(); i++)
			{
				rapidjson::Value jsv(rapidjson::kObjectType);
				valueptr = &jsv;
				//Value[i]->ProcessSerialArchive(this);
				func(this, Value[i]);
				comp.PushBack(jsv, *jallocator);
			}
			valueptr = PreValueptr;
			SerialHelpers::addJsonValue(*valueptr, *jallocator, PropName, comp);
		}
	}
	template<class T>
	void LinkPropertyArrayInt(std::vector<T> & Value, const char * PropName)
	{
		if (IsReading())
		{
			rapidjson::Value::MemberIterator starthead = CurrentReadHead;
			auto t = CurrentReadHead->value.GetArray();
			t.begin();
			for (unsigned int i = 0; i < t.Size(); i++)
			{
				rapidjson::Value* cv = &t[i];
				Value.push_back(cv->GetInt());
			}
			CurrentReadHead = starthead;
			CurrentReadHead++;
		}
		else
		{
			rapidjson::Value comp(rapidjson::kArrayType);
			rapidjson::Value* PreValueptr = valueptr;
			for (int i = 0; i < Value.size(); i++)
			{
				comp.PushBack((int)Value[i],*jallocator);
			}
			valueptr = PreValueptr;
			SerialHelpers::addJsonValue(*valueptr, *jallocator, PropName, comp);
		}
	}
	template<class K, class T>
	void LinkPropertyMap(std::map<K, T> & Value, const char * PropName, std::function<void(Archive*, T*)> func)
	{
		if (IsReading())
		{
			rapidjson::Value::MemberIterator starthead = CurrentReadHead;
			auto t = CurrentReadHead->value.GetArray();
			t.begin();
			for (unsigned int i = 0; i < t.Size(); i++)
			{
				T newc = T();
				rapidjson::Value* cv = &t[i];
				CurrentReadHead = cv->MemberBegin();
				K Key;
				LinkProperty(Key, "Pair.first");
				func(this, &newc);
				Value.emplace(Key, newc);
			}
			CurrentReadHead = starthead;
			CurrentReadHead++;
		}
		else
		{
			rapidjson::Value comp(rapidjson::kArrayType);
			rapidjson::Value* PreValueptr = valueptr;
			for (auto & Pair : Value)
			{
				rapidjson::Value jsv(rapidjson::kObjectType);
				valueptr = &jsv;
				//Value[i]->ProcessSerialArchive(this);
				//ArchiveProp(Pair.first);
				std::string temp = Pair.first;
				LinkProperty(temp, "Pair.first");
				func(this, &Pair.second);
				comp.PushBack(jsv, *jallocator);
			}
			valueptr = PreValueptr;
			SerialHelpers::addJsonValue(*valueptr, *jallocator, PropName, comp);
		}
	}
private:
	rapidjson::Document::AllocatorType* jallocator;
	rapidjson::Value::MemberIterator CurrentReadHead;
	rapidjson::Value* valueptr = nullptr;
	bool IsAchiveReading = false;
	void Init();
	std::string FileName = "";
	rapidjson::Document doc = rapidjson::Document();
	struct ReadHeadPop
	{
		ReadHeadPop(rapidjson::Value::MemberIterator* head)
		{
			CurrentHead = head;
			OldHead = *CurrentHead;
			*CurrentHead = (*CurrentHead)->value.MemberBegin();

		}
		~ReadHeadPop()
		{
			*CurrentHead = OldHead;
		}
	private:
		rapidjson::Value::MemberIterator* CurrentHead = nullptr;
		rapidjson::Value::MemberIterator OldHead;
	};

};
#define Scope_PopReadHead(Head) ReadHeadPop ReadHeadPop_##Head = ReadHeadPop(&Head);

#define ArchiveProp(Property) A->LinkProperty(Property,#Property);
#define ArchiveProp_Alias(Property,Alias) A->LinkProperty(Property,#Alias);
#define ArchiveProp_Enum(Property,Type) A->LinkEnumProperty<Type>(Property,#Property);
