#pragma once
#include "TDActor.h"
namespace TD
{
	class TDQuadTree;
	class TDScene
	{
	public:
		TDScene();
		~TDScene();
		TD_API void AddToScene(TDActor* Actor);
		std::vector<TDActor*>& GetActors() { return SceneActors; };
		glm::vec3 GetGravity()const { return GravityForce; }
	private:
		glm::vec3 GravityForce = glm::vec3(0, -9.81, 0);
		TDQuadTree* AcclerationTree = nullptr;
		//todo: remove
		std::vector<TDActor*> SceneActors;
	};

}