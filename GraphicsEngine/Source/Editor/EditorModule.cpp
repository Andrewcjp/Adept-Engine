#include "EditorModule.h"
#include "Core\Module\ModuleManager.h"
#include "Core\BaseWindow.h"

EditorModule::EditorModule()
{}


EditorModule::~EditorModule()
{}

BaseWindow* EditorModule::GetEditorWindow()
{
	return nullptr;
}

#ifdef EDITOR_EXPORT
IMPLEMENT_MODULE_DYNAMIC(EditorModule);
#endif