#pragma once
#include "rapidjson\document.h"
class Archive
{
public:
	Archive(std::string FilePath, bool Write = false);
	~Archive();
	
	//Basic Types
	void LinkProperty(int & Value, const char * PropName);
	void LinkProperty(float & Value, const char * PropName);
	void LinkProperty(bool & Value, const char * PropName);
	void LinkProperty(std::string & Value, const char * PropName);
	void LinkProperty(glm::vec3 & Value, const char* PropName);
	//Complex Core Types
	void LinkProperty(std::vector<class Component*>& Value, const char * PropName);
	void LinkProperty(class Transform * Value, const char * PropName);
	void LinkProperty(class Scene * Value, const char * PropName);

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
private:
	rapidjson::Document::AllocatorType* jallocator;
	rapidjson::Value::MemberIterator CurrentReadHead;
	rapidjson::Value* valueptr = nullptr;
	bool IsAchiveReading = false;
	void Init();
	std::string FileName = "";
	rapidjson::Document doc = rapidjson::Document();
};

#define ArchiveProp(Property) A->LinkProperty(Property,#Property);
#define ArchiveProp_Alias(Property,Alias) A->LinkProperty(Property,#Property);
#define ArchiveProp_Enum(Property,Type) A->LinkEnumProperty<Type>(Property,#Property);
