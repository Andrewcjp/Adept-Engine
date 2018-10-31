#include "stdafx.h"
#include "SceneJSerialiser.h"
#include "Scene.h"
#include "Core/GameObject.h"
#include <fstream>
#include "Core/Engine.h"
#include "Core/Utils/FileUtils.h"

SceneJSerialiser::SceneJSerialiser()
{
}

SceneJSerialiser::~SceneJSerialiser()
{
}

void SceneJSerialiser::SaveScene(Scene* target, std::string path)
{
	SceneArchive = new Archive(path, true);
	SceneArchive->LinkProperty(target, "GOS");
	SceneArchive->Write();
}

void SceneJSerialiser::LoadScene(Scene* target,std::string path)
{
	if (!FileUtils::File_ExistsTest(path))
	{
		return;
	}
	SceneArchive = new Archive(path, false);
	SceneArchive->LinkProperty(target,"GOS");	
}

std::string SceneJSerialiser::GetFile(std::string filename)
{
	std::string out;
	std::ifstream myfile(filename);
	if (myfile.is_open())
	{
		std::string line;
		while (std::getline(myfile, line))
		{
			out.append(line);
		}
		myfile.close();
	}
	else
	{
		Log::OutS  << "failed to load " << filename << Log::OutS;
	}
	return out;
}

bool SceneJSerialiser::WriteToFile(std::string filename, std::string data)
{
	std::string out;

	if (!FileUtils::File_ExistsTest(filename))
	{
		FileUtils::CreateDirectoriesToFullPath(filename);
	}
	std::ofstream myfile(filename, std::ofstream::out);
	if (myfile.is_open())
	{
		std::string line;
		myfile.write(data.c_str(), data.length());
		myfile.close();
	}
	else
	{
		Log::OutS  << "failed to save " << filename << Log::OutS;
		return false;
	}
	return true;
}