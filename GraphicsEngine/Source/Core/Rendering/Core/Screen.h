#pragma once
//This handles everything related to resolutions
class Screen
{
public:
	Screen();
	~Screen();
	static Screen* Get();
	static int GetScaledWidth();
	static int GetScaledHeight();
	static glm::ivec2 GetScaledRes();
	//Only updates the store size.
	static void Resize(int width, int height);
	static int GetWindowWidth();
	static int GetWindowHeight();
	static glm::ivec2 GetWindowRes();
	static bool IsValidForRender();
	//base window handles renderers and render scale.
	//ResizeSwapChain Handles swapchain etc.
	static bool NeedsWindowUpdate(int newwidth, int newHeight);
private:
	static Screen* Instance;
	int m_width = 0;
	int m_height = 0;
};

