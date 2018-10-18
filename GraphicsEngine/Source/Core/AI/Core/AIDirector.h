#pragma once
class Scene;
class AIDirector
{
public:
	AIDirector();
	~AIDirector();
	void SetScene(Scene* newscene);
private:
	Scene* scene = nullptr;
};

