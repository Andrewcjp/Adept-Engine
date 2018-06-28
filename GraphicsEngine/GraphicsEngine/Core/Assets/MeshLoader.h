#pragma once

#include <string>
#include <vector>
#include "Rendering/Core/RenderBaseTypes.h"

//this class loads mesh data into mesh;
class MeshLoader
{
public:
	static const glm::vec3 DefaultScale;
	struct FMeshLoadingSettings
	{
		glm::vec3 Scale = DefaultScale;
		bool InitOnAllDevices = true;
	};
	static bool LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings,std::vector<OGLVertex> &vertices,std::vector<int> &indices);


};

