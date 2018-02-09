#pragma once
#include "Component.h"
#include "Rendering/Core/Renderable.h"
#include "Rendering/Core/Material.h"
class MeshRendererComponent :
	public Component
{
public:

	MeshRendererComponent();
	MeshRendererComponent(Renderable * Mesh, Material * materal);
	virtual ~MeshRendererComponent();
	void SetUpMesh(Renderable * Mesh, Material * materal);
	void Render(bool DepthOnly, CommandListDef * list = nullptr);
	void Render(bool DepthOnly);
	Material* GetMaterial();
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props) override final;
	Renderable* GetMesh()
	{
		return m_mesh;
	}
private:
	Renderable* m_mesh;
	Material* m_mat;

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;


	// Inherited via Component
	virtual void InitComponent() override;


	// Inherited via Component
	virtual void Serialise(rapidjson::Value & v) override;

	virtual void Deserialise(rapidjson::Value & v) override;

};

