#pragma once
#include "Component.h"
#include "Rendering/Core/Renderable.h"
#include "Rendering/Core/Material.h"
class MeshRendererComponent :
	public Component
{
public:

	MeshRendererComponent();
	CORE_API MeshRendererComponent(Renderable * Mesh, Material * materal);
	virtual ~MeshRendererComponent();
	void SetUpMesh(Renderable * Mesh, Material * materal);

	void Render(bool DepthOnly,class RHICommandList * list);
	/*void Render(bool DepthOnly);*/
	Material* GetMaterial();
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props) override final;
	Renderable* GetMesh()
	{
		return m_mesh;
	}
private:
	Renderable* m_mesh = nullptr;
	Material* m_mat = nullptr;

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;


	// Inherited via Component
	virtual void InitComponent() override;
	virtual void ProcessSerialArchive(class Archive* Arch) override;
};

