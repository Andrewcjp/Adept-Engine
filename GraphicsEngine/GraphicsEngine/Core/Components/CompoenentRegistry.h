#pragma once
#include <vector>

class CompoenentRegistry
{
public:
	CompoenentRegistry();
	~CompoenentRegistry();
	static CompoenentRegistry* Instance;
	enum BaseComponentTypes
	{
		LightComp,
		CameraComp,
		ParticleComp,
		RigidComp,
		MeshComp,
		GrassPatchComp
	};
	
	static class Component* CreateAdditonalComponent(int id);
	static Component * CreateBaseComponent(BaseComponentTypes id);
	void RegisterExtraComponents(class ExtraComponentRegister* ECR);
private:
	std::vector<int> AdditonalGameComponents;
	Component* Internal_CreateAdditonalComponent(int id);
	ExtraComponentRegister* ECR;
	Component * Internal_CreateBaseComponent(BaseComponentTypes id);
};

