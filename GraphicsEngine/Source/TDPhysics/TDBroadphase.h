#pragma once

#include "TDTypes.h"

namespace TD
{
	class TDScene;
	class TDActor;
	class TDAABB;
	class SweepAndPrune;
	struct SAPBox;
	///This class handles executing the Broadphase and holds the results for the TDsolver to use.
	class TDBroadphase
	{
	public:
		TDBroadphase();
		~TDBroadphase();
		///Populate the NarrowPhasePairs List from the Currnet Selected BroadPhase
		void ConstructPairs();
		std::vector<CollisionPair> NarrowPhasePairs;
		///Register an actor with the current BroadPhase 
		void AddToPhase(TDActor* actor);
		///Remove an Actor from the currnet BroadPhase
		void RemoveFromPhase(TDActor * actor);
		///Update the Actors position the 
		void UpdateActor(TDActor * actor);
	private:
		SweepAndPrune* SAP = nullptr;
	};
	//A Point On a SAP axis
	struct SAPEndPoint
	{
		SAPEndPoint() {};
		SAPEndPoint(float value, bool ismin, SAPBox* owner)
		{
			Value = value;
			IsMinPoint = ismin;
			Owner = owner;
		}
		///PTR to the owning SAPBox
		SAPBox* Owner = nullptr;
		///Position on the Axis
		float Value = 0.0f;
		bool IsMinPoint = false;//todo: could encode in actor id for speed!
	};
	//The SAP reprsentation of a AABB
	struct SAPBox
	{
		SAPEndPoint* Min[3];
		SAPEndPoint* Max[3];
		//int Id = 0;
		TDAABB* Owner = nullptr;
		SAPBox() {};
		SAPBox(TDAABB* bb);
		///Update the Positions of this object on all of the SAP axes (not to be confused with an AXE) 
		void Update(TDAABB* AABB);
	};
	///Container for Pairs of SAP end points
	struct BPCollisionPair
	{
		BPCollisionPair(SAPEndPoint* a, SAPEndPoint* b);
		TDAABB* A = nullptr;
		TDAABB* B = nullptr;
		SAPEndPoint* Apoint = nullptr;
		SAPEndPoint* BPoint = nullptr;
	};
	///This class holds all data relating to the SAP broadPhase method 
	class SweepAndPrune
	{
	public:
		void AddObject(TDAABB* box);
		void UpdateObject(SAPBox* box, TDAABB* AABB);
		void RemoveObject(TDAABB* box);
		///Sorts all the Axes
		void Sort();
		///List of Generated Pairs
		std::vector<BPCollisionPair*> Pairs;
	private:
		void SAPSortAxis(std::vector<SAPEndPoint *>& AxisPoints);
		std::vector<SAPEndPoint*> Xpoints;
		std::vector<SAPEndPoint*> Ypoints;
		std::vector<SAPEndPoint*> Zpoints;
		std::vector<SAPBox*> Bodies;
	};
};

