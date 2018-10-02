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
	private:
		TDQuadTree* AcclerationTree = nullptr;
		//todo: remove
		std::vector<TDActor*> SceneActors;
	};

}