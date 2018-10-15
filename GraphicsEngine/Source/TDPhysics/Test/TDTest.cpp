#include "TDPCH.h"
#include "TDTest.h"
#include "TDRigidStatic.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDPlane.h"
#include "TDCollisionHandlers.h"
#include "TDAsserts.h"
//remove
#include "TDSolver.h"
namespace TD {
	TDTest::TDTest()
	{
	}


	TDTest::~TDTest()
	{
	}

	void TDTest::RunAllTests()
	{
		TDRigidStatic* rb = new TDRigidStatic();
		rb->GetTransfrom()->SetPos(glm::vec3(0, 0.9, 0));
		rb->AttachShape(new TDSphere());
		TDRigidStatic* rbB = new TDRigidStatic();
		rbB->GetTransfrom()->SetPos(glm::vec3(0, 0, 0));
		rbB->AttachShape(new TDPlane());
		ContactData data;
		DebugEnsure(TDCollisionHandlers::CollideSpherePlane( (TDSphere*)rb->GetAttachedShapes()[0], (TDPlane*)rbB->GetAttachedShapes()[0], &data));

		rb->GetTransfrom()->SetPos(glm::vec3(0, 2, 0));
		DebugEnsure(TDCollisionHandlers::CollideSpherePlane( (TDSphere*)rb->GetAttachedShapes()[0], (TDPlane*)rbB->GetAttachedShapes()[0], &data) == false);

		TDSolver::ProcessCollisions(rbB->GetAttachedShapes()[0], rb->GetAttachedShapes()[0]);
	}
}