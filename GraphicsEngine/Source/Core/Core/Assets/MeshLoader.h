#pragma once
#include "Rendering/Core/RenderBaseTypes.h"
struct MeshEntity;
//this class loads mesh data into mesh;
class Archive;
class MeshLoader
{
public:
	static const glm::vec3 DefaultScale;
	struct FMeshLoadingSettings
	{
		glm::vec3 Scale = glm::vec3(1,1,1);
		glm::vec2 UVScale = glm::vec2(1, 1);
		bool InitOnAllDevices = true;
		bool CreatePhysxMesh = false;
		bool GenerateIndexed = true;
		bool FlipUVs = false;
		void Serialize(Archive* A);
		std::vector<std::string> IgnoredMeshObjectNames;
	};
	static bool LoadMeshFromFile(std::string filename, FMeshLoadingSettings& Settings, std::vector<MeshEntity*> &Meshes);
	static bool LoadMeshFromFile_Direct(std::string filename, FMeshLoadingSettings & Settings, std::vector<OGLVertex>& vertices, std::vector<int>& indices);
};

