#pragma once
#include "SerialHelpers.h"
#include "Archive.h"
class SceneJSerialiser
{
public:
	SceneJSerialiser();
	~SceneJSerialiser();
	void SaveScene(class Scene * target, std::string path);
	void LoadScene(Scene * target, std::string path);
	static std::string GetFile(std::string filename);
	static bool WriteToFile(std::string filename, std::string data);
private:
	Archive* SceneArchive = nullptr;
};

