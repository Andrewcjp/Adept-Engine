#pragma once
#include "Core\Module\ModuleInterface.h"
class EditorModule : public IEditorModule
{
public:
	EditorModule();
	~EditorModule();

	virtual BaseWindow* GetEditorWindow() override;

};

