#pragma once
#include "include\glm\glm.hpp"
#include <string>
#include <vector>
#include "OpenGL\OGLTexture.h"
#include "OpenGL\OGLCube.h"
#include "../Rendering/Core/Mesh.h"
class Scene;//foward decare scene
class SceneSerialiser
{
public:
	SceneSerialiser() { }
	~SceneSerialiser();
	void LoadFromFile(std::string filename, Scene * scene);
private:
	typedef struct Data
	{
		std::string name;
		int id;
		glm::vec3 position;
		glm::vec3 rotation;
		glm::vec3 scale;
		std::string MeshName;
		std::string TextureName;
	}DataS;
	static std::string & ltrim(std::string & s);
	static std::string & rtrim(std::string & s);
	static std::string & trim(std::string & s);
	std::vector<std::string> split(const std::string & s, char delim);
	glm::vec3 StringToVector3(std::string input);
	DataS DeserialiseLine(std::string line);
	void AddDataToScene(Scene * scene, DataS Data);		
	template<typename Out>
	void tsplit(const std::string & s, char delim, Out result);

};

