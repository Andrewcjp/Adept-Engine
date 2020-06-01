#pragma once

class HighLevelAccelerationStructure;
class RTAccelerationStructureCache;
namespace ERTSceneEntityState
{
	enum Type
	{
		NeedsBuild,
		Inactive,
		Active,
		Limit
	};
}

struct RTSceneEntry
{
	ERTSceneEntityState::Type CurrnetState = ERTSceneEntityState::NeedsBuild;
	GameObject* Object;
	MeshEntity* E = nullptr;
	LowLevelAccelerationStructure* Structure;
	ShaderHitGroupInstance* Instance;
};

class RayTracingScene
{
public:
	void Create(Scene* scene);
	void Update();


	RayTracingScene();
	~RayTracingScene();
	void ConsiderObjects();
	void UpdateStateObject(ShaderBindingTable* table);
	void UpdateShaderTable(ShaderBindingTable * table);
	void ProcessUpdatesToAcclerationStuctures(RHICommandList * list);
	void OnObjectAddedToScene(GameObject* object);
	HighLevelAccelerationStructure* GetTopLevel() const { return mTopLevel; }
	RHIStateObject* GetSceneStateObject() const { return SceneStateObject; }
private:
	std::vector<RTSceneEntry*> ActiveEntitys;
	std::vector<RTSceneEntry*> ActiveEntitys_NeedBuild;
	std::vector<RTSceneEntry*> RTObjects;
	HighLevelAccelerationStructure* mTopLevel = nullptr;
	RTAccelerationStructureCache* mCache = nullptr;
	RHIStateObject* SceneStateObject = nullptr;
};

