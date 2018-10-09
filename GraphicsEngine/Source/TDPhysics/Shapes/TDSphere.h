#pragma once
#include "TDShape.h"
namespace TD {
	class TDSphere :public TDShape
	{
	public:
		TDSphere();
		~TDSphere();
		float Radius = 1;
	};

}