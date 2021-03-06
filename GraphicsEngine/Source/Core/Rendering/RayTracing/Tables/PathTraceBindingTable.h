#pragma once
#include "..\ShaderBindingTable.h"
class PathTraceBindingTable : public ShaderBindingTable
{
public:
	PathTraceBindingTable();
	virtual ~PathTraceBindingTable();
	virtual void InitTable() override;

	virtual Shader_RTBase* GetMaterialShader() override;

protected:
	virtual void OnMeshProcessed(Mesh* Mesh, MeshEntity* E, ShaderHitGroupInstance* Shader) override;

};

