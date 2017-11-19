#include "SceneSerialiser.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <ios>
#include <fstream>
#include "Scene.h"



SceneSerialiser::~SceneSerialiser()
{
}


// trim from start
inline std::string &SceneSerialiser::ltrim(std::string &s)
{
	/*s.erase(s.begin(), std::find_if(s.begin(), s.end(),
		std::not1(std::ptr_fun<int, int>(std::isspace))));*/
	return s;
}

// trim from end
inline std::string &SceneSerialiser::rtrim(std::string &s)
{
	/*s.erase(std::find_if(s.rbegin(), s.rend(),
		std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());*/
	return s;
}

// trim from both ends
inline std::string &SceneSerialiser::trim(std::string &s)
{
	return ltrim(rtrim(s));
}



template<typename Out>
void SceneSerialiser::tsplit(const std::string &s, char delim, Out result)
{
	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim))
	{
		*(result++) = item;
	}
}


std::vector<std::string> SceneSerialiser::split(const std::string &s, char delim)
{
	std::vector<std::string> elems;
	tsplit(s, delim, std::back_inserter(elems));
	return elems;
}
glm::vec3 SceneSerialiser::StringToVector3(std::string input)
{
	input = trim(input);
	input.erase(input.find('('), 1);
	input.erase(input.find(')'), 1);
	//input = input.Trim(')');
	std::vector<std::string> vector = split(input, ',');
	glm::vec3 nvec;
	nvec.x = std::stof(vector[0]);
	nvec.y = std::stof(vector[1]);
	nvec.z = std::stof(vector[2]);
	return nvec;
}

SceneSerialiser::DataS SceneSerialiser::DeserialiseLine(std::string line)
{
	DataS d;
	std::vector<std::string> splitstring = split(line, '|');
	d.name = splitstring[0];
	d.id = atoi(splitstring[1].c_str());
	d.position = StringToVector3(splitstring[2]);
	d.rotation = StringToVector3(splitstring[3]);
	d.scale = StringToVector3(splitstring[4]);
	d.MeshName = splitstring[5];
	d.TextureName = splitstring[6];
	return d;
}
void SceneSerialiser::AddDataToScene(Scene* scene, DataS d)
{
	GameObject* go = new GameObject(d.name, GameObject::Static, d.id);
	if (d.MeshName == "Cube")
	{
		//go->SetMesh(new OGLCube());
	}
	else
	{
		//go->SetMesh(new Mesh(("../asset/models/" + d.MeshName).c_str()));
	}
	Material* mat = new Material(new OGLTexture(("../asset/texture/" + d.TextureName).c_str()));
	//mat->NormalMap = new OGLTexture("../asset/texture/pbr/rusted_iron/normal.png");
	//go->SetMaterial(mat);
	go->GetTransform()->SetScale(d.scale);
	go->GetTransform()->SetPos(d.position);
	go->GetTransform()->SetEulerRot(d.rotation);

	//todo: handle phyx objects
	//go->actor = phyxengine->CreateActor(PxVec3(5, 10, 0));
	//Objects.push_back(go)
	scene->AddGameobjectToScene(go);
}


void SceneSerialiser::LoadFromFile(std::string filename,Scene* scene)
{
	std::ifstream file(filename);
	std::string temp;
	while (std::getline(file, temp))
	{
		//Do with temp
		if (temp.find('|') != std::string::npos)
		{
			Data d = DeserialiseLine(temp);
			AddDataToScene(scene, d);
			std::cout << d.name << " " << d.id << " " << d.position.x << " " << d.position.y << " " << d.position.z << std::endl;
		}

	}
}
