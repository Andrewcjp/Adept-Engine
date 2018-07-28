#pragma once
#include "rapidjson\document.h"
#include "SerialHelpers.h"
class SceneJSerialiser
{
public:
	SceneJSerialiser();
	~SceneJSerialiser();
	void SaveScene(class Scene * target, std::string path);
	void LoadScene(Scene * target, std::string path);
	static	rapidjson::Document::AllocatorType* jallocator;
private:
	rapidjson::Document doc = rapidjson::Document();
	std::string testpath = "test.txt";
	void SerialiseObjects(Scene* target);

	void DeserialiseObjects(Scene * Target);

	std::string GetFile(std::string filename);

	bool WriteToFile(std::string filename, std::string data);

};

