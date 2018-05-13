#pragma once
#if 0
#include "RHI/RenderWindow.h"

#include <vector>

#include "Core/Transform.h"
#include "Core/GameObject.h"
#include "../Rendering/Core/Light.h"

#include <ctime>
#include <time.h>


#include "Core/Input.h"
#include <memory>
#include "Core/Input.h"
#include "UI/TextRenderer.h"

#include "../OpenGL/OGLTexture.h"
#include "../Rendering/Core/Material.h"
#include "../Rendering/Renderers/ForwardRenderer.h"
#include "../Rendering/Renderers/DeferredRenderer.h"
#include "UI\UIManager.h"
class EditorGizmos;
//class ForwardRenderer;
class OGLWindow : public RenderWindow
{


private:

	int itemsrender = 0;
	float	    m_euler[3];
	HDC			m_hdc;				//handle to a device context
	HGLRC		m_hglrc;			//handle to a gl rendering context

	int			m_width;
	int			m_height;

	clock_t tstart;
	const int MaxPhysicsObjects = 1000;

	long lasttime = 0;
	float deltatime = 1;
	double accumilatePhysxdeltatime = 0;
	double accumrendertime = 0;
	double currenfps = 0;
	double avgtime = 0;
	int framecount = 0;
	int avgaccum = 0;
	double fpsnexttime = 0;
	float timesincestat = 0;
	double fpsaccumtime = 0.25f;
	double startms = 0;
	double physxtime = 0;
	double ShadowTime = 0;
	double FinalTime = 0;
	double RenderTime = 0;
	double sleeptimeMS = 0;
	double ShadowRendertime = 0;

	RenderEngine* Renderer;

	EditorGizmos* gizmos;
	Scene* mainscene;
	std::vector<GameObject*> PhysicsObjects;

	bool RenderedReflection = false;
	int FrameBufferRatio = 1;
	bool IsDeferredMode = false;
	std::unique_ptr<Input> input;
	std::unique_ptr<UIManager> UI;
protected:

	HGLRC CreateOGLContext(HDC hdc);
	BOOL DestroyOGLContext();
public:

	OGLWindow(HINSTANCE hInstance, int width, int height);
	OGLWindow(bool Isdef);
	virtual ~OGLWindow();
	bool IsFullscreen = false;
	void SwitchFullScreen(HINSTANCE hInstance);
	void SetFullScreenState(bool state);
	bool CreateRenderWindow(HINSTANCE hInstance, int width, int height, bool Fullscreen = false);
	BOOL InitWindow(HGLRC hglrc, HWND hwnd, HDC hdc, int width, int height);

	void RenderText();
	void SetDeferredState(bool state)
	{
		IsDeferredMode = state;
	}
	bool ShowHud = true;
	bool ExtendedPerformanceStats = false;
	

	void		Render();
	void		Resize(int width, int height);
	RenderEngine* GetCurrentRenderer()override
	{
		return Renderer;
	}

	void		DestroyRenderWindow();

	BOOL		MouseLBDown(int x, int y);
	BOOL		MouseLBUp(int x, int y);
	BOOL		MouseRBDown(int x, int y);
	BOOL		MouseRBUp(int x, int y);
	BOOL		MouseMove(int x, int y);
	BOOL KeyDown(WPARAM key);


	


	// Inherited via RenderWindow
	virtual void ProcessMenu(WORD command) override;

};
#endif