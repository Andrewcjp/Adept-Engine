#include "RenderingUtils.h"
#include "RHI\RHICommandList.h"
#include "Defaults.h"

void RenderingUtils::RenderScreenQuad(RHICommandList * list)
{
	list->SetVertexBuffer(Defaults::GetQuadBuffer());
	list->DrawPrimitive(6, 1, 0, 0);
}
