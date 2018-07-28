#include "stdafx.h"
#include "EditorCore.h"
#include "EditorWindow.h"

EditorCore::EditorCore()
{}


EditorCore::~EditorCore()
{}

void EditorCore::SetSelectedObjectIndex(int index)
{
	SelectedObject = (*EditorWindow::GetInstance()->GetCurrentScene()->GetObjects())[index];
}

GameObject * EditorCore::GetSelectedObject()
{
	return SelectedObject;
}

void EditorCore::SetSelectedObject(GameObject * newobj)
{
	SelectedObject = newobj;
}
