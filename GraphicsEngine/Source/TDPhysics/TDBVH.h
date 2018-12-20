#pragma once

namespace TD
{
	class TDMesh;
	class TDTriangle;
	class TDAABB;
	class TDSphere;
	struct BVHNode;
	struct RayCast;
	struct TriangleInterection
	{
		glm::vec3 Point = glm::vec3();
		TDTriangle* Tri = nullptr;
		float depth = 0;
	};	
	class TDBVH
	{
	public:
		TDBVH();
		~TDBVH();
		void BuildAccelerationStructure(TDMesh* Shape);
		void SplitBVH(BVHNode* node, TDMesh* model, int depth);
		void FreeBVHNode(BVHNode* node);
		bool TraverseForSphere(TDSphere * A, std::vector<TriangleInterection>& contacts, int MaxContactCount);
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