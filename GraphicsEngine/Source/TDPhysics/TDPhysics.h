#pragma once
namespace TD
{
	class TDPhysics
	{
	public:
		static TDPhysics* CreatePhysics();
		void StartUp();
		void StartStep(float deltaTime);
		void ShutDown();
	private:
		TDPhysics();
		~TDPhysics();
	};

}