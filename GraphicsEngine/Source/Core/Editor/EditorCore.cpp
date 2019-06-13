
#include "EditorCore.h"
#include "EditorWindow.h"
#include "Core/Assets/Scene.h"
#if WITH_EDITOR
EditorCore::EditorCore()
{}


EditorCore::~EditorCore()
{}

void EditorCore::SetSelectedObjectIndex(int index)
{
	SelectedObject = BaseWindow::GetScene()->GetObjects()[index];
}

GameObject * EditorCore::GetSelectedObject()
{
	return SelectedObject;
}

void EditorCore::SetSelectedObject(GameObject * newobj)
{
	SelectedObject = newobj;
}
#endif