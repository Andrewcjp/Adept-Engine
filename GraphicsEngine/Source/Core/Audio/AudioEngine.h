#pragma once
#include "Core/Types/FString.h"
class AudioEngine
{
public:
	static void Startup();
	static void Shutdown();
	CORE_API static AudioEngine* Get();
	static void ProcessAudio();
	CORE_API static void PostEvent(FString name, class GameObject* Obj = nullptr);
	void LoadBanks();
	CORE_API void MakeDefaultListener(GameObject * g);

	static void RegisterObject(GameObject * obj);
	static void DeRegisterObject(GameObject * obj);
	static void StopAll();
	static int GetNextAudioId();
	void LoadBank(std::string Name);
	static void UpdateWiseTransfrom(GameObject* go);
private:
	AudioEngine();
	~AudioEngine();
	bool Init();

	void Terminate();
	GameObject* CurrnetMainListener = nullptr;
	static AudioEngine* Instance;
	int NextAudioId = 0;
};

