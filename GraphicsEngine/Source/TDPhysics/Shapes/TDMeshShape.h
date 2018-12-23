#pragma once
#include "TDShape.h"

namespace TD { class TDBox; }

namespace TD { struct RayCast; }

namespace TD { class TDBVH; }

namespace TD
{
	class TDTriangle;
	class TDPlane;
	class TDMesh;
	class TDMeshShape :public TDShape
	{
	public:
		TD_API TDMeshShape(TDMesh* mesh);
		TD_API ~TDMeshShape();

		virtual glm::vec3 GetBoundBoxHExtents() override;
		TDAABB* GetAABB();
		bool MeshSphere(TDSphere * s, ContactData* contactbuffer);
		bool MeshBox(TDBox * Box, ContactData * Contacts);
		bool IntersectTriangle(RayCast* ray);
	private:
		TDMesh* Mesh = nullptr;
	};
	struct ArrayEntryDesc
	{
		size_t Count = 0;
		size_t Stride = 0;
		void* DataPtr = 0;
		template<class T>
		T* GetFromArray(int index) const
		{
			return (T*)(((unsigned char*)DataPtr) + Stride * index);
		}
	};
	class TDTriangleMeshDesc
	{
	public:
		ArrayEntryDesc Points;
		ArrayEntryDesc Normals;
		ArrayEntryDesc Indices;
		bool HasPerVertexNormals = true;
	};
	class TDMesh
	{
	public:
		TD_API TDMesh(const TDTriangleMeshDesc& desc);
		~TDMesh();
		TD_API std::vector<TDTriangle*> & GetTriangles() { return Triangles; }
		static TDAABB * FromMinMax(const glm::vec3 & min, const glm::vec3 & max);
		TD_API virtual void CookMesh();
		glm::vec3 Max;
		glm::vec3 Min;
		TDBVH* GetBVH() { return BVH; }		
	private:
		TDBVH* BVH = nullptr;
		std::vector<TDTriangle*> Triangles;
	};
	class TDTriangle
	{
	public:
		TDTriangle() {};
		TDTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);
		TDTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 normal);
		glm::vec3 Points[3];
		glm::vec3 Normal = glm::vec3(0);
		TDPlane MakeFromTriangle();
		bool PointInTriangle(const glm::vec3 & p);
		glm::vec3 GetBarycentric(const glm::vec3 & p);
		bool Intersect(RayCast* ray, TDMeshShape* owner);
		bool TriangleSphere(TDSphere * s, glm::vec3 & out_Contact, float & depth);
		glm::vec3 ClosestPoint(const glm::vec3 & p);
		void DebugDraw(float time);
		bool TriangleAABB(const TDAABB * a);
		bool TriangleBox(TDBox * Box);
		glm::vec3 GetPos();
	private:
		glm::vec3 posAVG;
	};
	struct Edge
	{
		glm::vec3 pointa;
		glm::vec3 pointb;
		Edge() {};
		Edge(glm::vec3 a, glm::vec3 b)
		{
			pointa = a;
			pointb = b;
		}
		static glm::vec3 ClosestPoint(const Edge & line, const glm::vec3 & point);
	};
}