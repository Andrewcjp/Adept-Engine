#include "RenderEngine.h"


RenderEngine::~RenderEngine()
{
}

void RenderEngine::SetRenderSettings(RenderSettings set)
{
	settings = set;
}

void RenderEngine::SetScene(Scene * sc)
{
	mainscene = sc;
}

void RenderEngine::SetEditorCamera(Editor_Camera * cam)
{
	EditorCam = cam;
}
