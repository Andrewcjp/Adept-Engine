#pragma once

#include "TDTypes.h"

namespace TD { class TDBFBE; }

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
		///Populate the NarrowPhasePairs List from the current Selected BroadPhase
		void ConstructPairs();

		void BFBEGetPairs();

		void SapGetPairs();
#if !BUILD_FULLRELEASE
		///Development only validates the Broadphase is not missing any interactions 
		void Validate();
#endif
		std::vector<ActorCollisionPair> NarrowPhasePairs;
		///Register an actor with the current BroadPhase 
		void AddToPhase(TDActor* actor);
		///Remove an Actor from the current BroadPhase
		void RemoveFromPhase(TDActor * actor);
		///Update the Actors position the 
		void UpdateActor(TDActor * actor);

		TDScene* TargetScene = nullptr;
	private:
		SweepAndPrune* SAP = nullptr;
		TDBFBE* BFBE = nullptr;
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
	//The SAP representation of a AABB
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
		bool IsDead = false;
	};
	///Container for Pairs of SAP end points
	struct BPCollisionPair
	{
		BPCollisionPair(TDAABB* a, TDAABB* b);
		//BPCollisionPair(SAPEndPoint* a, SAPEndPoint* b);
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
		void UpdateObject(TDAABB* AABB);
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

