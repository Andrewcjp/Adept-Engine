#include "Screen.h"
#include "Core\Platform\Logger.h"

Screen* Screen::Instance = nullptr;

Screen::Screen()
{}


Screen::~Screen()
{}

Screen * Screen::Get()
{
	if (Instance == nullptr)
	{
		Instance = new Screen();
	}
	return Instance;
}


int Screen::GetScaledWidth()
{
	if (RHI::GetRenderSettings()->LockBackBuffer)
	{
		return glm::iround(RHI::GetRenderSettings()->LockedWidth*RHI::GetRenderSettings()->GetCurrentRenderScale());
	}
	else
	{
		return (int)(Get()->m_width * RHI::GetRenderSettings()->GetCurrentRenderScale());
	}
}

int Screen::GetScaledHeight()
{
	if (RHI::GetRenderSettings()->LockBackBuffer)
	{
		return glm::iround(RHI::GetRenderSettings()->LockedHeight*RHI::GetRenderSettings()->GetCurrentRenderScale());
	}
	else
	{
		return (int)(Get()->m_height * RHI::GetRenderSettings()->GetCurrentRenderScale());
	}
}

glm::ivec2 Screen::GetScaledRes()
{
	return glm::ivec2(GetScaledWidth(), GetScaledHeight());
}

void Screen::Resize(int width, int height)
{
	Get()->m_width = width;
	Get()->m_height = height;
	Log::LogMessage("Window resized to " + std::to_string(Screen::GetWindowWidth()) + " X " + std::to_string(Screen::GetWindowHeight()) + " (~" + std::to_string(Screen::GetWindowHeight()) + "p)");
}

int Screen::GetWindowWidth()
{
	return Get()->m_width;
}

int Screen::GetWindowHeight()
{
	return Get()->m_height;
}

glm::ivec2 Screen::GetWindowRes()
{
	return glm::ivec2(GetScaledWidth(), GetWindowHeight());
}

bool Screen::IsValidForRender()
{
	return Get()->m_height > 0 && Get()->m_width > 0;
}

bool Screen::NeedsWindowUpdate(int newwidth, int newHeight)
{
	return Get()->m_height != newHeight || Get()->m_width != newwidth;
}


