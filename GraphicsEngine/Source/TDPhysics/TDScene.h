#pragma once
#include "TDActor.h"
namespace TD
{
	class TDQuadTree;
	class TDRigidDynamic;
	class TDScene
	{
	public:
		TDScene();
		~TDScene();
		TD_API void AddToScene(TDActor* Actor);
		std::vector<TDActor*>& GetActors() { return SceneActors; };
		std::vector<TDRigidDynamic*>& GetDynamicActors() { return DynamicActors; };
		glm::vec3 GetGravity()const { return GravityForce; }
	private:
		glm::vec3 GravityForce = glm::vec3(0, -9.81, 0);
		TDQuadTree* AcclerationTree = nullptr;
		//todo: remove
		std::vector<TDActor*> SceneActors;
		std::vector<TDRigidDynamic*> DynamicActors;
	};

}