#pragma once

class Scene;
class SceneSeraliser
{
public:
	void SaveScene(Scene* target, std::string path);
	void LoadScene(Scene* target, std::string path);
};

