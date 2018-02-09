#pragma once
#include <vector>
#include <map>
class CompoenentRegistry
{
public:
	CompoenentRegistry();
	~CompoenentRegistry();
	void RegisterComponent(std::string name, int id);
	static CompoenentRegistry* Instance;
	enum BaseComponentTypes
	{
		LightComp,
		CameraComp,
		ParticleComp,
		RigidComp,
		MeshComp,
		GrassPatchComp,
		Limit
	};
	
	static class Component* CreateAdditonalComponent(int id);
	static Component * CreateBaseComponent(BaseComponentTypes id);
	void RegisterExtraComponents(class ExtraComponentRegister* ECR);
	std::string GetNameById(int id);
	int GetCount();
private:
	std::vector<int> AdditonalGameComponents;
	std::map<int, std::string> ComponentNameMap;
	Component* Internal_CreateAdditonalComponent(int id);
	ExtraComponentRegister* ECR;
	Component * Internal_CreateBaseComponent(BaseComponentTypes id);
};

//todo: map of componenet class ids