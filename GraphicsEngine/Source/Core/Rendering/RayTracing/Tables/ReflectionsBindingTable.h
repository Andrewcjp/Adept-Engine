#pragma once
#include "..\ShaderBindingTable.h"
class ReflectionsBindingTable : public ShaderBindingTable
{
public:
	ReflectionsBindingTable();
	virtual ~ReflectionsBindingTable();
	virtual void InitTable() override;
protected:
	virtual void OnMeshProcessed(Mesh* Mesh, MeshEntity* E, Shader_RTBase* Shader) override;

};

