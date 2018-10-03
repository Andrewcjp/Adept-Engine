#pragma once
#include "Component.h"
#include "Rendering/Core/Mesh.h"
#include "Rendering/Core/Material.h"
class MeshRendererComponent :
	public Component
{
public:

	MeshRendererComponent();
	CORE_API MeshRendererComponent(Mesh * Mesh, Material * materal);
	virtual ~MeshRendererComponent();
	void SetUpMesh(Mesh * Mesh, Material * materal);

	void Render(bool DepthOnly,class RHICommandList * list);
	/*void Render(bool DepthOnly);*/
	Material* GetMaterial();
#if WITH_EDITOR
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props) override final;
#endif
	Mesh* GetMesh()
	{
		return m_mesh;
	}
private:
	Mesh* m_mesh = nullptr;
	Material* m_mat = nullptr;

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;


	// Inherited via Component
	virtual void InitComponent() override;
	virtual void ProcessSerialArchive(class Archive* Arch) override;
};

