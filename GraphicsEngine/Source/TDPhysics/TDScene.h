#pragma once
#include "TDActor.h"

namespace TD
{
	class TDOctTree;
	class TDRigidDynamic;
	struct RaycastData;
	class TDScene
	{
	public:
		TDScene();
		~TDScene();
		TD_API void AddToScene(TDActor* Actor);
		std::vector<TDActor*>& GetActors() { return SceneActors; };
		std::vector<TDRigidDynamic*>& GetDynamicActors() { return DynamicActors; };
		glm::vec3 GetGravity()const { return GravityForce; }
		TD_API void RemoveActor(TDActor* Actor);
		TD_API bool RayCastScene(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData* HitData);		
	private:
		bool RayCastSceneInternal(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData * HitData);
		glm::vec3 GravityForce = glm::vec3(0, -9.81, 0);
		TDOctTree* AcclerationTree = nullptr;
		//todo: remove
		std::vector<TDActor*> SceneActors;
		std::vector<TDRigidDynamic*> DynamicActors;
	};

}