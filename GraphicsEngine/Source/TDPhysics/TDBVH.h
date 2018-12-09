#pragma once

namespace TD { struct RayCast; }

namespace TD { class TDSphere; }

namespace TD
{
	class TDMesh;
	class TDTriangle;
	class TDAABB;
	struct BVHNode;
	class TDBVH
	{
	public:
		TDBVH();
		~TDBVH();
		void BuildAccelerationStructure(TDMesh* Shape);
		void SplitBVH(BVHNode* node, TDMesh* model, int depth);
		void FreeBVHNode(BVHNode* node);
		bool TraverseForSphere(TDSphere * A, glm::vec3 & out_Contact, float & depth, TDTriangle** Out_TRi);
		void Render();
		bool TraverseForRay(RayCast * ray);
		TDMesh* TargetMesh = nullptr;
		BVHNode* Root = nullptr;
	private:
		int MaxDepth = 10;
		int Targetcount = 100;
	};

	struct BVHNode
	{
		TDAABB* bounds;
		int numTriangles;
		std::vector<BVHNode> children;
		std::vector<int> TrianglesIndexs;
	};
};