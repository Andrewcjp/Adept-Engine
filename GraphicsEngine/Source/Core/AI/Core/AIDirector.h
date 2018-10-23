#pragma once
namespace EAIDirectorDifficulty {
	enum Type {
		Easy,
		Normal,
		Hard,
		Extreme,
		Limit
	};
}
class Scene;
class AIDirector
{
public:
	CORE_API AIDirector();
	CORE_API ~AIDirector();
	void SetScene(Scene* newscene);
	CORE_API virtual void Tick();
private:
	Scene* scene = nullptr;

};

