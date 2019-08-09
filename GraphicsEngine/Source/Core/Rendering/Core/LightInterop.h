#if __cplusplus
#define row_major
#include "glm\gtx\compatibility.hpp"
namespace glm
{
struct _LightEntry
#else
struct Light
#endif
{
	float3 LPosition;
	float F;
	float3 color;
	float F2;
	float3 Direction;
	float f3;
	int type;//type 1 == point, type 0 == directional, type 2 == spot
	int ShadowID;
	int DirShadowID;
	int HasShadow;
	int4 PreSampled;
	float Range;
	//float3 T;
};
#if __cplusplus
}
typedef glm::_LightEntry LightEntry;
#endif