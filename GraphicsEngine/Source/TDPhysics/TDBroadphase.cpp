
#include "TDBroadphase.h"
#include "Shapes/TDAABB.h"
#include "TDCollisionHandlers.h"
#include <algorithm>
#include "TDPhysics.h"
#include "TDSimConfig.h"
#include "Utils/MemoryUtils.h"
#include "Core/Utils/VectorUtils.h"
#include "Utils/MathUtils.h"
#include "TDBFBE.h"
#include "TDScene.h"

namespace TD
{
	using namespace MemoryUtils::VectorUtils;
	TDBroadphase::TDBroadphase()
	{
		if (TDPhysics::GetCurrentSimConfig()->BroadphaseMethod == TDBroadphaseMethod::SAP)
		{
			SAP = new SweepAndPrune();
		}
		else if (TDPhysics::GetCurrentSimConfig()->BroadphaseMethod == TDBroadphaseMethod::BFBE)
		{
			BFBE = new TDBFBE();
		}
		else
		{
			__debugbreak();
		}
	}

	TDBroadphase::~TDBroadphase()
	{
		SafeDelete(SAP);
		SafeDelete(BFBE);
	}

	void TDBroadphase::ConstructPairs()
	{
		if (TDPhysics::GetCurrentSimConfig()->BroadphaseMethod == TDBroadphaseMethod::SAP)
		{
			SapGetPairs();
		}
		else if (TDPhysics::GetCurrentSimConfig()->BroadphaseMethod == TDBroadphaseMethod::BFBE)
		{
			BFBEGetPairs();
		}
#if !BUILD_FULLRELEASE
		Validate();
#endif
	}
	void TDBroadphase::BFBEGetPairs()
	{
		NarrowPhasePairs.clear();
		for (int i = 0; i < TargetScene->GetActors().size(); i++)
		{
			for (int j = i; j < TargetScene->GetActors().size(); j++)
			{
				TDActor* Actor = TargetScene->GetActors()[i];
				TDActor* Actorb = TargetScene->GetActors()[j];
				if (TargetScene->GetActors()[i] == TargetScene->GetActors()[j])
				{
					continue;
				}
				if (TargetScene->GetActors()[i]->GetActorType() == TDActorType::RigidStatic && TargetScene->GetActors()[j]->GetActorType() == TDActorType::RigidStatic)
				{
					continue;
				}

				NarrowPhasePairs.push_back(ActorCollisionPair(Actor, Actorb));
			}
		}
	}
	void TDBroadphase::SapGetPairs()
	{
		SAP->Sort();
		NarrowPhasePairs.clear();
		for (int i = 0; i < SAP->Pairs.size(); i++)
		{
			if (SAP->Pairs[i]->A->Owner == SAP->Pairs[i]->B->Owner)
			{
				continue;//How? Just HOW?
			}
			if (SAP->Pairs[i]->A->Owner->GetActorType() == TDActorType::RigidStatic && SAP->Pairs[i]->B->Owner->GetActorType() == TDActorType::RigidStatic)
			{
				continue;//static - static collisions make no sense.
			}
			ActorCollisionPair newpair = ActorCollisionPair(SAP->Pairs[i]->A->Owner, SAP->Pairs[i]->B->Owner);
			//DebugEnsure(newpair.ShapePairs.size() != 0);
			if (!VectorUtils::Contains(NarrowPhasePairs, newpair))
			{
				NarrowPhasePairs.push_back(newpair);
			}
		}
	}
	bool CheckBB(SAPEndPoint* a, SAPEndPoint* b)
	{
		return TDCollisionHandlers::CollideAABBAABB(a->Owner->Owner, b->Owner->Owner);
	}
#if !BUILD_FULLRELEASE
	void TDBroadphase::Validate()
	{
		return;
		int CorrectedCount = 0;
		for (int i = 0; i < TargetScene->GetActors().size(); i++)
		{
			for (int j = i; j < TargetScene->GetActors().size(); j++)
			{
				TDActor* Actor = TargetScene->GetActors()[i];
				TDActor* Actorb = TargetScene->GetActors()[j];
				if (TargetScene->GetActors()[i] == TargetScene->GetActors()[j])
				{
					continue;
				}
				if (Actor->GetActorType() == TDActorType::RigidStatic && Actorb->GetActorType() == TDActorType::RigidStatic)
				{
					continue;
				}
				if (TDCollisionHandlers::CollideAABBAABB(Actor->AABB, Actorb->AABB))
				{
					if (!VectorUtils::Contains(NarrowPhasePairs, ActorCollisionPair(Actor, Actorb)))
					{
					//	NarrowPhasePairs.push_back(ActorCollisionPair(Actor, Actorb));
						CorrectedCount++;
						//__debugbreak();
					}
				}
				/*NarrowPhasePairs.push_back(ActorCollisionPair(Actor, Actorb));*/
			}
		}
		for (int i = 0; i < NarrowPhasePairs.size(); i++)
		{
			//TDPhysics::DrawDebugLine(NarrowPhasePairs[i].first->GetTransfrom()->GetPos(), NarrowPhasePairs[i].second->GetTransfrom()->GetPos(), glm::vec3(0, 1, 0), 0.0f);
		}
		if (CorrectedCount > 0)
		{
			printf("CorrectedCount %d\n", CorrectedCount);
		}
	}
#endif
	void TDBroadphase::AddToPhase(TDActor * actor)
	{
		if (SAP != nullptr)
		{
			SAP->AddObject(actor->AABB);
		}
	}

	void TDBroadphase::RemoveFromPhase(TDActor * actor)
	{
		if (SAP != nullptr)
		{
			SAP->RemoveObject(actor->AABB);
		}
	}

	void TDBroadphase::UpdateActor(TDActor* actor)
	{
	//	if (!MathUtils::AlmostEqual(actor->AABB->Position, actor->GetTransfrom()->GetPos(), 0.001f))
		{
			actor->UpdateAABBPos(actor->GetTransfrom()->GetPos());
			if (SAP != nullptr)
			{
				SAP->UpdateObject(actor->AABB);
			}
		}
	}

	void SweepAndPrune::AddObject(TDAABB * AABB)
	{
		SAPBox* box = new SAPBox(AABB);
		Bodies.push_back(box);
		Xpoints.push_back(box->Min[0]);
		Xpoints.push_back(box->Max[0]);

		Ypoints.push_back(box->Min[1]);
		Ypoints.push_back(box->Max[1]);

		Zpoints.push_back(box->Min[2]);
		Zpoints.push_back(box->Max[2]);
	}

	void SweepAndPrune::UpdateObject(TDAABB* AABB)
	{
		for (int i = 0; i < Bodies.size(); i++)
		{
			if (Bodies[i]->Owner == AABB)
			{
				Bodies[i]->Update(AABB);
				return;
			}
		}
		DebugEnsure(false);
	}

	int RemovePair(BPCollisionPair* point, std::vector<BPCollisionPair*> & points, TDActor* owner)
	{
		int removecount = 0;
		//for (int i = 0; i < points.size(); i++)
		//{
		//	if (points[i]->A == point->A || points[i]->B == point->B || points[i]->A == point->B || points[i]->B == point->A)
		//	{
		//		points.erase(points.begin() + i);
		//		removecount++;
		//	}
		//}
		for (int i = (int)points.size() - 1; i >= 0; i--)
		{
			if (points[i]->A->Owner == owner || points[i]->B->Owner == owner)
			{
				points.erase(points.begin() + i);
				removecount++;
			}
		}
		for (int i = (int)points.size() - 1; i >= 0; i--)
		{
			if (points[i]->A->IsDead || points[i]->B->IsDead)
			{
				points.erase(points.begin() + i);
				removecount++;
			}
		}
		return removecount;
	}

	void SweepAndPrune::RemoveObject(TDAABB* bb)
	{
		SAPBox* box = nullptr;
		for (int i = 0; i < Bodies.size(); i++)
		{
			if (Bodies[i]->Owner == bb)
			{
				box = Bodies[i];
			}
		}
		if (box == nullptr)
		{
			return;
		}
		RemoveItemO(box->Min[0], Xpoints);
		RemoveItemO(box->Max[0], Xpoints);
		RemoveItemO(box->Min[1], Ypoints);
		RemoveItemO(box->Max[1], Ypoints);
		RemoveItemO(box->Min[2], Zpoints);
		RemoveItemO(box->Max[2], Zpoints);
		bb->IsDead = true;
		int Count = RemovePair(new BPCollisionPair(bb, bb), Pairs, bb->Owner);
		Count += RemovePair(new BPCollisionPair(box->Min[1]->Owner->Owner, box->Max[1]->Owner->Owner), Pairs, bb->Owner);
		//Count += RemovePair(new BPCollisionPair(box->Min[2], box->Max[2]), Pairs, bb->Owner);
		RemoveItemO(box, Bodies);
		box->IsDead = true;
		//	SafeDelete(box);
	}

	void SweepAndPrune::Sort()
	{
		//todo: local sort on insert?
		SAPSortAxis(Xpoints);
		SAPSortAxis(Ypoints);
		SAPSortAxis(Zpoints);
	}


	void RemoveItem(BPCollisionPair* point, std::vector<BPCollisionPair*> & points)
	{
		for (int i = (int)points.size() - 1; i >= 0; i--)
		{
			if ((points[i]->A == point->A && points[i]->B == point->B) /*|| (points[i]->A == point->B && points[i]->B == point->A)*/)
			{
				points.erase(points.begin() + i);
			}
		}
	}


	void SweepAndPrune::SAPSortAxis(std::vector<SAPEndPoint*>& AxisPoints)
	{
		for (int j = 1; j < AxisPoints.size(); j++)
		{
			SAPEndPoint* KeyPoint = AxisPoints[j];
			const float Key = KeyPoint->Value;
			int i = j - 1;
			while (i >= 0 && AxisPoints[i]->Value > Key)
			{
				SAPEndPoint* spoint = AxisPoints[i];
				if (spoint->Owner->IsDead || KeyPoint->Owner->IsDead)
				{
					__debugbreak();
				}
				if (KeyPoint->IsMinPoint && !spoint->IsMinPoint)
				{
					//pair!
					if (CheckBB(KeyPoint, spoint))
					{
						Pairs.push_back(new BPCollisionPair(spoint->Owner->Owner, KeyPoint->Owner->Owner));
					}
					else
					{
						/*	spoint->Owner->Owner->DebugRender(glm::vec3(1, 0, 0),10.5f);
							KeyPoint->Owner->Owner->DebugRender(glm::vec3(1, 0, 0),10.5f);*/
					}
				}
				if (!KeyPoint->IsMinPoint && spoint->IsMinPoint)
				{
					RemoveItem(new BPCollisionPair(spoint->Owner->Owner, KeyPoint->Owner->Owner), Pairs);
				}
				AxisPoints[i + 1] = spoint;
				i = i - 1;
			}
			AxisPoints[i + 1] = KeyPoint;
		}
	}

	SAPBox::SAPBox(TDAABB * bb)
	{
		Min[0] = new SAPEndPoint(bb->GetMin().x, true, this);
		Min[1] = new SAPEndPoint(bb->GetMin().y, true, this);
		Min[2] = new SAPEndPoint(bb->GetMin().z, true, this);

		Max[0] = new SAPEndPoint(bb->GetMax().x, false, this);
		Max[1] = new SAPEndPoint(bb->GetMax().y, false, this);
		Max[2] = new SAPEndPoint(bb->GetMax().z, false, this);
		Owner = bb;
	}

	void SAPBox::Update(TDAABB * AABB)
	{
		Min[0]->Value = AABB->GetMin().x;
		Min[1]->Value = AABB->GetMin().y;
		Min[2]->Value = AABB->GetMin().z;

		Max[0]->Value = AABB->GetMax().x;
		Max[1]->Value = AABB->GetMax().y;
		Max[2]->Value = AABB->GetMax().z;
	}

	/*BPCollisionPair::BPCollisionPair(SAPEndPoint * a, SAPEndPoint * b)
	{
		Apoint = a;
		BPoint = b;
		A = a->Owner->Owner;
		B = b->Owner->Owner;
	}*/
	BPCollisionPair::BPCollisionPair(TDAABB * a, TDAABB * b)
	{
		A = a;
		B = b;
	}
}