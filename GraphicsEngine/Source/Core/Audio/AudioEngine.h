#pragma once
#include "Core/Types/FString.h"
class AudioEngine
{
public:
	static void Startup();
	static void Shutdown();
	static AudioEngine* Get();
	static void ProcessAudio();
	CORE_API static void PostEvent(FString name, class GameObject* Obj = nullptr);
	void LoadBanks();
	static void RegisterObject(GameObject * obj);
	static void DeRegisterObject(GameObject * obj);
	static void StopAll();
	static int GetNextAudioId();
	void LoadBank(std::string Name);
private:
	AudioEngine();
	~AudioEngine();
	bool Init();

	void Terminate();

	static AudioEngine* Instance;
	int NextAudioId = 0;
};

