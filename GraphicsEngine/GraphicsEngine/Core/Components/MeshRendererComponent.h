#pragma once
#include "Component.h"
#include "Rendering/Core/Renderable.h"
#include "Rendering/Core/Material.h"
class MeshRendererComponent :
	public Component
{
public:
	
	MeshRendererComponent(Renderable * Mesh, Material * materal);
	virtual ~MeshRendererComponent();
	void Render(bool DepthOnly);
	Material* GetMaterial();
	void GetInspectorProps(std::vector<Inspector::InspectorProperyGroup> &props);
private:
	Renderable* m_mesh;
	Material* m_mat;

	// Inherited via Component
	virtual void BeginPlay() override;
	virtual void Update(float delta) override;
	
};

