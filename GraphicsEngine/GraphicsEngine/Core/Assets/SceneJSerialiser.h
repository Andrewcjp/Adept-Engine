#pragma once
#include "rapidjson\document.h"
#include "SerialHelpers.h"
class SceneJSerialiser
{
public:
	SceneJSerialiser();
	~SceneJSerialiser();
	void SaveScene(class Scene* target);

	void LoadScene(Scene * target);

	static	rapidjson::Document::AllocatorType* jallocator;
private:
	rapidjson::Document doc;
	std::string testpath = "test.txt";
	void SerialiseObjects(Scene* target);

	void DeserialiseObjects(Scene * Target);

	std::string GetFile(std::string filename);

	bool WriteToFile(std::string filename, std::string data);

};

