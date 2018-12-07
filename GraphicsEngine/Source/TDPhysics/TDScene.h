#pragma once
#include "TDActor.h"
#include "TDTypes.h"

namespace TD { class TDBroadphase; }
namespace TD
{
	class TDOctTree;
	class TDRigidDynamic;
	struct RaycastData;
	class TDConstraint;
	class TDScene
	{
	public:
		TDScene();
		~TDScene();
#if !BUILD_FULLRELEASE
		void DebugRender();
#endif
		TD_API void AddToScene(TDActor* Actor);
		void AddConstraint(TDConstraint* con);
		std::vector<TDActor*>& GetActors() { return SceneActors; };
		std::vector<TDRigidDynamic*>& GetDynamicActors() { return DynamicActors; };
		std::vector<TDConstraint*>& GetConstraints() { return Constraints; };
		glm::vec3 GetGravity()const { return GravityForce; }
		TD_API void RemoveActor(TDActor* Actor);
		TD_API bool RayCastScene(glm::vec3 Origin, glm::vec3 Dir, float Distance, RaycastData* HitData);
		void UpdateBroadPhase();
		std::vector<CollisionPair>& GetPairs();
	private:
		bool RayCastSceneInternal(RayCast* ray);
		glm::vec3 GravityForce = glm::vec3(0, -9.81, 0);
		TDOctTree* AcclerationTree = nullptr;
		//todo: remove
		std::vector<TDActor*> SceneActors;
		std::vector<TDRigidDynamic*> DynamicActors;
		std::vector<TDConstraint*> Constraints;
		TDBroadphase* Broadphase = nullptr;
	};

}