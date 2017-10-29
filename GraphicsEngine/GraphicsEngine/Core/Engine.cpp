#include "Engine.h"
#include "OpenGL/OGLWindow.h"
#include "D3D11/D3D11Window.h"
#include "Resource.h"
#include "EngineGlobals.h"
#include "Physics/PhysicsEngine.h"
#include <Windows.h>
#if PHYSX_ENABLED
#include "Physics/PhysxEngine.h"
#endif
#include "RHI/RHI.h"
#include "Editor/EditorWindow.h"
PhysicsEngine* Engine::PhysEngine = NULL;
#include "Core/Assets/AssetManager.h"
#include "Game.h"
float Engine::StartTime = 0;
EditorWindow * Engine::GetEditorWindow()
{
	return reinterpret_cast<EditorWindow*>(m_appwnd);
}

std::string Engine::GetRootDir()
{
	wchar_t buffer[MAX_PATH];
	int bytes = GetModuleFileName(NULL, buffer, MAX_PATH);
	std::wstring ws(buffer);
	return std::string(ws.begin(), ws.end());
}

Engine::Engine()
{
	StartTime = (float)get_nanos();
	std::cout << "Starting In " << GetRootDir() << std::endl;
	std::cout << "Loading Engine v0.1" << std::endl;
#if PHYSX_ENABLED
	PhysEngine = new PhysxEngine();
#else
	PhysEngine = new PhysicsEngine();
	std::cout << "WARNING: Physx Disabled" << std::endl;
#endif
	if (PhysEngine != nullptr)
	{
		PhysEngine->initPhysics(false);
	}
	AssetManager::StartAssetManager();
}


Engine::~Engine()
{

}
void Engine::Destory()
{
	PhysEngine->cleanupPhysics(false);
}

//
void Engine::CreateApplication(HINSTANCE hinst, LPSTR args, int nCmdShow)
{
	if (nCmdShow > 0)
	{
		std::string input = args;
		if (input.compare("-deferred") == 0)
		{
			std::cout << "Starting in Deferred Rendering mode" << std::endl;
			Deferredmode = true;
		}
	}
	CreateApplicationWindow(1280, 720, ERenderSystemType::RenderSystemOGL);

}
void Engine::SetContextData(HGLRC hglrc, HWND hwnd, HDC hdc)
{
	mhglrc = hglrc;
	mhwnd = hwnd;
	mhdc = hdc;
}

void Engine::SetGame(Game * game)
{
	mgame = game;
}

void Engine::CreateApplicationWindow(int width, int height, ERenderSystemType type)
{
	if (m_appwnd == nullptr)
	{

		mwidth = width;
		mheight = height;
		CurrentRenderSystem = type;
		if (type == RenderSystemD3D11)
		{

#if BUILD_D3D11
			RHI::InitRHI(RenderSystemD3D11);
			m_appwnd = new D3D11Window();
			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height);
#else
			isWindowVaild = false;
#endif

		}
		else if (type == RenderSystemOGL)
		{
#if BUILD_OPENGL
			RHI::InitRHI(RenderSystemOGL);
#if WITH_EDITOR
			m_appwnd = new EditorWindow(Deferredmode);
#else 
			m_appwnd = new OGLWindow(Deferredmode);
#endif 

			isWindowVaild = m_appwnd->CreateRenderWindow(m_hInst, width, height);
#else 
			isWindowVaild = false;
#endif			
		}
		if (!isWindowVaild)
		{
			std::cout << "Fatal Error: Window Invalid" << std::endl;
		}
#if !NO_GEN_CONTEXT
		m_appwnd->SetVisible(TRUE);
#endif

	}
}
void Engine::setVSync(bool sync)
{
	// Function pointer for the wgl extention function we need to enable/disable
	// vsync
	typedef BOOL(APIENTRY *PFNWGLSWAPINTERVALPROC)(int);
	PFNWGLSWAPINTERVALPROC wglSwapIntervalEXT = 0;

	const char *extensions = (char*)glGetString(GL_EXTENSIONS);

	if (strstr(extensions, "WGL_EXT_swap_control") == 0)
	{
		std::cout << "WGL_EXT_swap_control Not Avalible" << std::endl;
		return;
	}
	else
	{
		wglSwapIntervalEXT = (PFNWGLSWAPINTERVALPROC)wglGetProcAddress("wglSwapIntervalEXT");

		if (wglSwapIntervalEXT != nullptr)
		{
			wglSwapIntervalEXT(sync);
		}
		else
		{
			std::cout << "wglSwapIntervalEXT Not valid" << std::endl;
		}
	}
}
bool Engine::SwitchRenderAPI(ERenderSystemType type)
{
	if (type == CurrentRenderSystem)
	{
		return false;
	}
	isWindowVaild = false;
	m_appwnd->DestroyRenderWindow();
	delete m_appwnd;
	m_appwnd = nullptr;
	RHI::InitRHI(type);
	CreateApplicationWindow(mwidth, mheight, type);
	return true;
}
