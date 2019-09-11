#pragma once


namespace ManagedDll
{
	public ref class Container
	{
	public:void CallInit()
	{
		CSharpCore::Container::Init();
	}
	public:void CallTick()
	{
		CSharpCore::Container::Tick();
	}
	public:void CallShutdown()
	{
		CSharpCore::Container::ShutDown();
	}
	};
}