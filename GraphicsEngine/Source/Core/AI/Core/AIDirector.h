#pragma once
#include "Core/Types/WeakObjectPtr.h"
#include "Core/GameObject.h"
namespace EAIDirectorDifficulty
{
	enum Type
	{
		Easy,
		Normal,
		Hard,
		Extreme,
		Limit
	};
}
class Scene;
class SpawnMarker;
class AIDirector
{
public:
	CORE_API AIDirector();
	CORE_API ~AIDirector();
	void SetScene(Scene* newscene);
	CORE_API virtual void Tick();
	CORE_API GameObject* GetPlayerObject();
	CORE_API void SetPlayer(GameObject* player);
	std::vector<SpawnMarker*> spawnmarkers;
protected:
	Scene* scene = nullptr;
	WeakObjectPtr<GameObject> Player;
	

};

