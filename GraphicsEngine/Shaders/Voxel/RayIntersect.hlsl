struct AABB
{
	float3 Position;
	float3 HalfExtends;
	float3 ClosestPoint(float3 Pos)
	{
		float3 result = Pos;
		float3 min = GetMin();
		float3 max = GetMax();

		result.x = (result.x < min.x) ? min.x : result.x;
		result.y = (result.y < min.y) ? min.y : result.y;
		result.z = (result.z < min.z) ? min.z : result.z;

		result.x = (result.x > max.x) ? max.x : result.x;
		result.y = (result.y > max.y) ? max.y : result.y;
		result.z = (result.z > max.z) ? max.z : result.z;

		return result;
	}
	void Init(float3 pos, float3 Halfextend)
	{
		Position = pos;
		HalfExtends = Halfextend;
	}
	float3 GetMin()
	{
		float3 point1 = Position + HalfExtends;
		float3 point2 = Position - HalfExtends;
		return float3(min(point1.x, point2.x), min(point1.y, point2.y), min(point1.z, point2.z));
	}

	float3 GetMax()
	{
		float3 point1 = Position + HalfExtends;
		float3 point2 = Position - HalfExtends;
		return float3(max(point1.x, point2.x), max(point1.y, point2.y), max(point1.z, point2.z));
	}

};
#define FLT_EPSILON 1.192092896e-07
bool VectorEqual(float3 A, float3 B)
{
	return A.x == B.x && A.y == B.y && A.z == B.z;
}
struct Ray 
{
	float3 Origin;
	float3 Dir;
};
#define MAX_HITS 10
struct RayHitResult
{
	float3 Hit[MAX_HITS];
	int HitCount;
	float T;
	void Init()
	{
		T = 0.0f;
		HitCount = 0;
	}
	void AddHit(float3 pos)
	{
		Hit[HitCount] = pos;
		HitCount++;
	}
};
bool AlmostEqual(float a, float b, float epsilon)
{
	return abs(a - b) <= epsilon;
}
bool RayAABB(in AABB aabb,in Ray inray,inout RayHitResult result)
{
	result.Init();
	float3 bmin = aabb.GetMin();
	float3 bmax = aabb.GetMax();
	//It is possible for some of these denominators top be 0.0f so Min with a Small number
	const float MinValue = 0.00001f;
	float t1 = (bmin.x - inray.Origin.x) / max(inray.Dir.x, MinValue);
	float t2 = (bmax.x - inray.Origin.x) / max(inray.Dir.x, MinValue);
	float t3 = (bmin.y - inray.Origin.y) / max(inray.Dir.y, MinValue);
	float t4 = (bmax.y - inray.Origin.y) / max(inray.Dir.y, MinValue);
	float t5 = (bmin.z - inray.Origin.z) / max(inray.Dir.z, MinValue);
	float t6 = (bmax.z - inray.Origin.z) / max(inray.Dir.z, MinValue);

	float tmin = max(max(min(t1, t2), min(t3, t4)), min(t5, t6));
	float tmax = min(min(max(t1, t2), max(t3, t4)), max(t5, t6));

	if (tmax < 0.0f)
	{
		return false;
	}
	//No possible intersect
	if (tmin > tmax)
	{
		return false;
	}

	float Distance = tmin;

	// If tmin is < 0, tmax is closer
	if (tmin < 0.0f)
	{
		Distance = tmax;
	}
	result.T = Distance;
#if 1
	{
		//vec3 normals[] = //all possible normals
		//{
		//	vec3(-1, 0, 0),
		//	vec3(1, 0, 0),
		//	vec3(0, -1, 0),
		//	vec3(0, 1, 0),
		//	vec3(0, 0, -1),
		//	vec3(0, 0, 1)
		//};
		float t[] = { t1, t2, t3, t4, t5, t6 };//All the computed mins and maxes
		const int BOX_SIDES = 6;
		for (int i = 0; i < BOX_SIDES; i++)
		{
			if (AlmostEqual(Distance, t[i], FLT_EPSILON))
			{
				//Ray.HitData.AddContact(Ray.Origin + Ray.Dir* Distance, normals[i], Distance, Shape);
				result.AddHit(inray.Origin + inray.Dir* Distance);
				return true;
			}
		}
	}
#endif
	return true;
}


#if 0
bool GetSamplePos(float3 OurPos, InstanceDesc desc, out uint3 Pos)
{
	float3 Point = desc.ClosestPoint(OurPos);
	if (!VectorEqual(Point, OurPos))
	{
		return false;
	}
	float3 LocalPos = OurPos - desc.GetMin();
	float3 NRM = LocalPos / (desc.HalfExtends * 2);
	Pos = NRM * desc.Size / 0.1f;
	Pos = LocalPos / 0.1f;
	return true;
}
#endif