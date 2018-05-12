#pragma once

#include <string>
#include <vector>
#include "../Rendering/Core/Mesh.h"
#include "../Rendering/Core/RenderBaseTypes.h"

//this class loads mesh data into mesh;
class MeshLoader
{
public:
	MeshLoader();
	~MeshLoader();
	static bool LoadMeshFromFile(std::string filename, std::vector<OGLVertex> &vertices,std::vector<int> &indices);


};

