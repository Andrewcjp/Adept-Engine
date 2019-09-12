#pragma once
#include "Core\CSharpInterOp\ICSharpContainerModule.h"
using namespace System;


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
	public:int CallCreateObject(CSObjectCreationArgs* Args);
	};
}