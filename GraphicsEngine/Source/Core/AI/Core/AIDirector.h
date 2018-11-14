#pragma once
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
class GameObject;
class AIDirector
{
public:
	CORE_API AIDirector();
	CORE_API ~AIDirector();
	void SetScene(Scene* newscene);
	CORE_API virtual void Tick();
	CORE_API GameObject* GetPlayerObject();
	CORE_API void SetPlayer(GameObject* player);
protected:
	Scene* scene = nullptr;
	GameObject* Player = nullptr;

};

