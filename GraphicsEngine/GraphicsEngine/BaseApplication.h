#pragma once
#include "EngineGlobals.h"
#if BUILD_WINDOW_ENGINE

#define IDM_RENDER_D3D11 1
#define IDM_RENDER_OGL 2
#define IDM_FILE_QUIT 3

class RenderWindow;
class PhysicsEngine;
class Engine;
//Define BaseApplication as a singleton
class BaseApplication
{
	public:

		
	private:
		RenderWindow*				m_appwnd;
		static BaseApplication*		s_oglapp;
		Engine*		m_engine;
		BOOL						m_terminate;
									BaseApplication();
		virtual						~BaseApplication();		
		BOOL						MyRegisterClass(HINSTANCE hinst);
		int mwidth, mheight;
	
	public:

		HINSTANCE					m_hInst;
		bool						isWindowVaild = false;
		static BaseApplication*		CreateApplication(HINSTANCE hInst, LPSTR args, int nshow);
		static HWND GetHWND();
		static void					DestroyApplication();
		static BaseApplication*		GetApplication();

		bool IsRenderSwitch = false;
		int							Run();
		void						Kill();

		void AddMenus(HWND hwnd);

		inline RenderWindow*			GetApplicationWindow();


		static	LRESULT CALLBACK	WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
};
#endif