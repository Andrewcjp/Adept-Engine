#pragma once

class BaseWindow;

class IModuleInterface
{

public:
	virtual ~IModuleInterface()
	{}
	virtual bool StartupModule()
	{
		return true;
	}
	virtual void ShutdownModule()
	{}
};

class IEditorModule : public IModuleInterface
{
public:
	virtual ~IEditorModule()
	{}
	virtual BaseWindow* GetEditorWindow()
	{
		return nullptr;
	}
};