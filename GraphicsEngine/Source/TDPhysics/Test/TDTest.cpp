
#include "TDTest.h"
#include "TDRigidStatic.h"
#include "Shapes/TDSphere.h"
#include "Shapes/TDPlane.h"
#include "TDCollisionHandlers.h"
//remove
#include "TDSolver.h"
#include "Shapes/TDAABB.h"
namespace TD
{
	TDTest::TDTest()
	{}


	TDTest::~TDTest()
	{}
	void TDTest::TypeTest()
	{
		TDShapeFlags t;
		t.SetFlagValue(TDShapeFlags::ESimulation, true);
		DebugEnsure(t.GetFlagValue(TDShapeFlags::ESimulation));
		DebugEnsure(t.GetFlagValue(TDShapeFlags::Limit) == false);
	}
	void TDTest::RunAllTests()
	{
		TDAABB* A = new TDAABB();
		A->Position = glm::vec3(5, 0, 0);
		A->HalfExtends = glm::vec3(1.25);

		TDAABB* B = new TDAABB();
		B->Position = glm::vec3(5.4, 0.6, 1);
		B->HalfExtends = glm::vec3(4);

		bool collide = TDCollisionHandlers::CollideAABBAABB(A, B);
		glm::vec3 colour = collide ? glm::vec3(0, 1, 0) : glm::vec3(1, 0, 0);
		/*B->DebugRender(colour, 100);
		A->DebugRender(colour, 100);*/
		B->Position = glm::vec3(0, 0.7f, 0);
		//DebugEnsure(TDCollisionHandlers::CollideAABBAABB(A, B));
		return;
		TDRigidStatic* rb = new TDRigidStatic();
		rb->GetTransfrom()->SetPos(glm::vec3(0, 0.9, 0));
		rb->AttachShape(new TDSphere());
		TDRigidStatic* rbB = new TDRigidStatic();
		rbB->GetTransfrom()->SetPos(glm::vec3(0, 0, 0));
		rbB->AttachShape(new TDPlane());
		ContactData data;
		DebugEnsure(TDCollisionHandlers::CollideSpherePlane((TDSphere*)rb->GetAttachedShapes()[0], (TDPlane*)rbB->GetAttachedShapes()[0], &data));

		rb->GetTransfrom()->SetPos(glm::vec3(0, 2, 0));
		DebugEnsure(TDCollisionHandlers::CollideSpherePlane((TDSphere*)rb->GetAttachedShapes()[0], (TDPlane*)rbB->GetAttachedShapes()[0], &data) == false);

		//		TDSolver::ProcessCollisions(rbB->GetAttachedShapes()[0]);
	}
}