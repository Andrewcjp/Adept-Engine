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

	void Render(bool DepthOnly, class RHICommandList * list);
	/*void Render(bool DepthOnly);*/
	Material* GetMaterial(int index);
#if WITH_EDITOR
	void GetInspectorProps(std::vector<InspectorProperyGroup> &props) override final;
#endif
	Mesh* GetMesh()
	{
		return m_mesh;
	}
	void SetMaterial(Material * mat, int index);
	CORE_API void SetVisiblity(bool state);
	CORE_API void LoadAnimation(std::string filename, std::string name);
	CORE_API void LoadAnimation(std::string filename, std::string name, MeshLoader::FMeshLoadingSettings & Settings);
	CORE_API void PlayAnim(std::string name);
private:
	Mesh* m_mesh = nullptr;
	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;

	bool IsVisible = true;
	// Inherited via Component
	virtual void InitComponent() override;
	virtual void ProcessSerialArchive(class Archive* Arch) override;
};

