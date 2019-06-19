struct Plane
{
	float3	N;		// Plane normal.
	float	d;		// Distance to origin.
};
// Compute a plane from 3 noncollinear points that form a triangle.
// This equation assumes a right-handed (counter-clockwise winding order) 
// coordinate system to determine the direction of the plane normal.
Plane ComputePlane(float3 p0, float3 p1, float3 p2)
{
	Plane plane;

	float3 v0 = p1 - p0;
	float3 v2 = p2 - p0;

	plane.N = normalize(cross(v0, v2));

	// Compute the distance to the origin using p0.
	plane.d = dot(plane.N, p0);

	return plane;
}
struct Sphere
{
	float3 c;	 // Center point.
	float r;	// Radius.
};
// Four planes of a view frustum (in view space).
// The planes are:
//  * Left,
//  * Right,
//  * Top,
//  * Bottom.
// The back and/or front planes can be computed from depth values in the 
// light culling compute shader.
struct Frustum
{
	Plane planes[4];	// left, right, top, bottom frustum planes.
};
bool SphereInsidePlane(Sphere sphere, Plane plane)
{
	return dot(plane.N, sphere.c) - plane.d < -sphere.r;
}
// Check to see of a light is partially contained within the frustum.
bool SphereInsideFrustum(Sphere sphere, Frustum frustum, float zNear, float zFar) // this can only be used in view space
{
	bool result = true;

	//if (sphere.c.z + sphere.r < zNear || sphere.c.z - sphere.r > zFar)
	//{
	//	result = false;
	//}

	//// Then check frustum planes
	//for (int i = 0; i < 4 && result; i++)
	//{
	//	if (SphereInsidePlane(sphere, frustum.planes[i]))
	//	{
	//		result = false;
	//	}
	//}

	// Better to just unroll:
	result = ((sphere.c.z + sphere.r < zNear || sphere.c.z - sphere.r > zFar) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[0])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[1])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[2])) ? false : result);
	result = ((SphereInsidePlane(sphere, frustum.planes[3])) ? false : result);

	return result;
}