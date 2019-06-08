#pragma once
class ComponentRegistry
{
public:
	CORE_API ComponentRegistry();
	CORE_API ~ComponentRegistry();
	CORE_API void RegisterComponent(std::string name, int id);
	CORE_API static ComponentRegistry* GetInstance();

	enum BaseComponentTypes
	{
		LightComp,
		CameraComp,
		ParticleComp,
		RigidComp,
		MeshComp,
		GrassPatchComp,
		ColliderComp,
		SpawnMarkerComp,
		Limit
	};

	static class Component* CreateAdditonalComponent(int id);
	static Component * CreateBaseComponent(BaseComponentTypes id);
	CORE_API void RegisterExtraComponents(class ExtraComponentRegister* ECR);
	std::string GetNameById(int id);
	int GetCount();
private:
	std::vector<int> AdditonalGameComponents;
	std::map<int, std::string> ComponentNameMap;
	Component* Internal_CreateAdditonalComponent(int id);
	ExtraComponentRegister* ECR = nullptr;
	Component * Internal_CreateBaseComponent(BaseComponentTypes id);
};

//todo: map of componenet class ids