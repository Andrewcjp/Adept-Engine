#include "SceneSeraliser.h"
#include "Core/Assets/Scene.h"
#include "BinaryArchive.h"


void SceneSeraliser::SaveScene(Scene* target, std::string path)
{
	BinaryArchive * SceneArchive = new BinaryArchive();
	SceneArchive->Open(path, true);
	int Count = target->GetObjects().size();
	//Count = 1;
	SceneArchive->LinkHeader(Count);
	for (int i = 0; i < Count; i++)
	{
		target->GetObjects()[i]->Serialize(SceneArchive);
	}
	SceneArchive->Close();
}

void SceneSeraliser::LoadScene(Scene* target, std::string path)
{
	BinaryArchive * SceneArchive = new BinaryArchive();
	SceneArchive->Open(path);
	int Count = target->GetObjects().size();
	SceneArchive->LinkHeader(Count);
	
	for (int i = 0; i < Count; i++)
	{
		GameObject* newobj = new GameObject();
		newobj->Serialize(SceneArchive);
		target->AddGameobjectToScene(newobj);
	}
	SceneArchive->Close();
}