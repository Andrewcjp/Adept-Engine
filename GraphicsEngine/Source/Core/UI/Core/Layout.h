#pragma once

#include "UI/Core/UIWidget.h"
#include "UI/UIManager.h"

namespace UIUtils
{
	//todo min size with widget value
	template <class T>
	static void ArrangeHorizontal(int w, int h, int x, int y, std::vector<T*> &widgets, float MinHeight = -1, int Gap = 0, int MaxHeight = -1)
	{
		UNUSED_PARAM(MinHeight);
		if (widgets.size() > 0)
		{
			int Currenty = y;
			for (int i = 0; i < widgets.size(); i++)
			{
				int height = int(widgets[i]->AligmentStruct.SizeMax);
				if (MaxHeight != -1 && height > MaxHeight)
				{
					height = MaxHeight;
				}
				widgets[i]->SetRootSpaceSize(w, height, x, Currenty);
				Currenty += height + Gap;
			}
		}
	}
	static void ArrangeVertical(int w, int h, int x, int y, std::vector<UIWidget*> &widgets, int Gap = 0)
	{
		if (widgets.size() > 0)
		{
			int Xstride = w / (int)widgets.size();
			if (Xstride > UIManager::GetScaledWidth(0.05f))
			{
				Xstride = UIManager::GetScaledWidth(0.05f);
			}
			int width = Xstride;
			int Currentx = x;
			for (int i = 0; i < widgets.size(); i++)
			{
				widgets[i]->SetAbsoluteSize(width, h, Currentx, y);
				Currentx += Xstride + Gap;
			}
		}
	}
	static void ArrangeGrid(int w, int h, int x, int y, std::vector<UIWidget*> &widgets, int Gap = 0)
	{
		float MinSize = 0.05f;
		if (widgets.size() > 0)
		{
			int YStride = h / (int)widgets.size();
			if (YStride > UIManager::GetScaledHeight(MinSize))
			{
				YStride = UIManager::GetScaledHeight(MinSize);
			}

			int Xstride = w / (int)widgets.size();
			if (Xstride > UIManager::GetScaledWidth(MinSize))
			{
				Xstride = UIManager::GetScaledWidth(MinSize);
			}
			int width = Xstride;
			int Currentx = x;
			int height = YStride;
			int Currenty = (y - YStride) + h;
			int CurrectItemWidth = 0;
			for (int i = 0; i < widgets.size(); i++)
			{
				widgets[i]->SetAbsoluteSize(width, height, Currentx, Currenty);
				CurrectItemWidth++;
				if (CurrectItemWidth >= 3)
				{
					Currenty -= YStride + Gap;
					Currentx = x;
					CurrectItemWidth = 0;
				}
				else
				{
					Currentx += Xstride + Gap;
				}

			}
		}
	}
}