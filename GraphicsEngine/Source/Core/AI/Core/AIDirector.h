#pragma once
#include "Core/Types/WeakObjectPtr.h"
#include "Core/GameObject.h"

class Scene;
class SpawnMarker;
/*!  \addtogroup AI
* @{ */
class AIDirector
{
public:
	CORE_API AIDirector();
	CORE_API virtual ~AIDirector();
	void SetScene(Scene* newscene);
	CORE_API virtual void Tick();
	CORE_API GameObject* GetPlayerObject();
	CORE_API void SetPlayer(GameObject* player);
	std::vector<SpawnMarker*> spawnmarkers;
protected:
	Scene* scene = nullptr;
	WeakObjectPtr<GameObject> Player;


};

