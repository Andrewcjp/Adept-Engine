#pragma once
#include "TDPCH.h"
#include "TDTypes.h"

namespace TD
{
	class TDScene;
	class TDActor;
	class TDAABB;
	class SweepAndPrune;
	//!This class handles executing the Broadphase and holds the results for the TDsolver to use.
	class TDBroadphase
	{
	public:
		TDBroadphase();
		~TDBroadphase();
		void ConstructPairs();
		std::vector<CollisionPair> NarrowPhasePairs;
		///Register an actor with the current BroadPhase 
		void AddToPhase(TDActor* actor);
		void RemoveFromPhase(TDActor * actor);
		void UpdateActor(TDActor * actor);
	private:
		std::vector<TDActor*> actors;
		SweepAndPrune* SAP = nullptr;
	};
	struct SAPBox;
	struct SAPEndPoint
	{
		SAPEndPoint() {};
		SAPEndPoint(float value, bool ismin, SAPBox* owner)
		{
			Value = value;
			IsMinPoint = ismin;
			Owner = owner;
		}
		int ActorId = 0;//todo: map
		SAPBox* Owner = nullptr;
		float Value = 0.0f;
		bool IsMinPoint = false;//todo: could encode in actor id!
	};
	struct SAPBox
	{
		SAPEndPoint* Min[3];
		SAPEndPoint* Max[3];
		int Id = 0;
		TDAABB* Owner = nullptr;
		SAPBox() {};
		SAPBox(TDAABB* bb);
		void Update(TDAABB* AABB);
	};
	struct BPCollisionPair
	{
		BPCollisionPair(SAPEndPoint* a, SAPEndPoint* b);
		TDAABB* A;
		TDAABB* B;
	};
	class SweepAndPrune
	{
	public:
		void AddObject(TDAABB* box);
		void UpdateObject(SAPBox* box, TDAABB* AABB);
		void RemoveObject(TDAABB* box);
		void Sort();
		void SAPSortAxis(std::vector<SAPEndPoint *>& AxisPoints);
		
		std::vector<BPCollisionPair*> Pairs;
	private:
		std::vector<SAPEndPoint*> Xpoints;
		std::vector<SAPEndPoint*> Ypoints;
		std::vector<SAPEndPoint*> Zpoints;
		std::vector<SAPBox*>Bodies;
	};
};

