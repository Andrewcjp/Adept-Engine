
#include "AudioEngine.h"
#include <AK/SoundEngine/Common/AkSoundEngine.h>
#include <AK/IBytes.h>
#include <AK/SoundEngine/Common/AkMemoryMgr.h>
#include <AK/SoundEngine/Common/AkModule.h>  
#include <AK/SoundEngine/Common/IAkStreamMgr.h>                 // Streaming Manager
#include <AK/Tools/Common/AkPlatformFuncs.h>                    // Thread defines                  
#include <AK/SoundEngine/Common/AkSoundEngine.h>  
#ifndef AK_OPTIMIZED
#include <AK/Comm/AkCommunication.h>
#endif // AK_OPTIMIZED
#include "ThirdParty/Audio/Win32/AkFilePackageLowLevelIOBlocking.h"// Sample low-level I/O implementation
#include "AKHooks.h"
#include "Core/Platform/Logger.h"
#include "Core/GameObject.h"
#include "Core/Assets/AssetManager.h"
//wwise Libs
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "ws2_32.lib")


AudioEngine* AudioEngine::Instance = nullptr;

void AudioEngine::Startup()
{
	Instance = new AudioEngine();
	Instance->Init();
}

void AudioEngine::Shutdown()
{
	if (Instance != nullptr)
	{
		Instance->Terminate();
		SafeDelete(Instance);
	}
}

AudioEngine * AudioEngine::Get()
{
	return Instance;
}

AudioEngine::AudioEngine()
{}
AudioEngine::~AudioEngine()
{}

CAkFilePackageLowLevelIOBlocking g_lowLevelIO;
bool AudioEngine::Init()
{
	AkMemSettings memSettings;
	memSettings.uMaxNumPools = 20;

	if (AK::MemoryMgr::Init(&memSettings) != AK_Success)
	{
		assert(!"Could not create the memory manager.");
		return false;
	}
	//
// Create and initialize an instance of the default streaming manager. Note
// that you can override the default streaming manager with your own. Refer
// to the SDK documentation for more information.
//

	AkStreamMgrSettings stmSettings;
	AK::StreamMgr::GetDefaultSettings(stmSettings);

	// Customize the Stream Manager settings here.

	if (!AK::StreamMgr::Create(stmSettings))
	{
		assert(!"Could not create the Streaming Manager");
		return false;
	}

	//
	// Create a streaming device with blocking low-level I/O handshaking.
	// Note that you can override the default low-level I/O module with your own. Refer
	// to the SDK documentation for more information.      
	//
	AkDeviceSettings deviceSettings;
	AK::StreamMgr::GetDefaultDeviceSettings(deviceSettings);

	// Customize the streaming device settings here.

	// CAkFilePackageLowLevelIOBlocking::Init() creates a streaming device
	// in the Stream Manager, and registers itself as the File Location Resolver.
	if (g_lowLevelIO.Init(deviceSettings) != AK_Success)
	{
		assert(!"Could not create the streaming device and Low-Level I/O system");
		return false;
	}
	AkInitSettings initSettings;
	AkPlatformInitSettings platformInitSettings;
	AK::SoundEngine::GetDefaultInitSettings(initSettings);
	AK::SoundEngine::GetDefaultPlatformInitSettings(platformInitSettings);

	if (AK::SoundEngine::Init(&initSettings, &platformInitSettings) != AK_Success)
	{
		assert(!"Could not initialize the Sound Engine.");
		return false;
	}
#ifndef AK_OPTIMIZED
	//
	// Initialize communications (not in release build!)
	//
	AkCommSettings commSettings;
	AK::Comm::GetDefaultInitSettings(commSettings);
	if (AK::Comm::Init(commSettings) != AK_Success)
	{
		assert(!"Could not initialize communication.");
		return false;
	}
#endif // AK_OPTIMIZED
	Log::LogMessage("WWise Initalised");
	LoadBanks();
	return true;
}
void AudioEngine::Terminate()
{
	AK::SoundEngine::ClearBanks();
#ifndef AK_OPTIMIZED
	//
	// Terminate Communication Services
	//
	AK::Comm::Term();
#endif // AK_OPTIMIZED
	AK::SoundEngine::Term();
	g_lowLevelIO.Term();

	if (AK::IAkStreamMgr::Get())
	{
		AK::IAkStreamMgr::Get()->Destroy();
	}
	AK::MemoryMgr::Term();

}
void AudioEngine::ProcessAudio()
{
	// Process bank requests, events, positions, RTPC, etc.
	AK::SoundEngine::RenderAudio();
}
const AkGameObjectID GAME_OBJECT_ID_DEFAULT = 0;
void AudioEngine::PostEvent(FString name, GameObject * Obj)
{
	int outputid = 0;
	if (Obj == nullptr)
	{
		outputid = AK::SoundEngine::PostEvent(name.ToWideString().c_str(), GAME_OBJECT_ID_DEFAULT);
	}
	else
	{
		outputid = AK::SoundEngine::PostEvent(name.ToWideString().c_str(), Obj->GetAudioId());
	}
	if (outputid == AK_INVALID_PLAYING_ID)
	{
		Log::LogMessage("Failed to post event \"" + name.ToSString() + "\"", Log::Severity::Error);
	}
}

void AudioEngine::LoadBanks()
{
	LoadBank("Init.bnk");
	LoadBank("Core.bnk");
	AK::SoundEngine::RegisterGameObj(GAME_OBJECT_ID_DEFAULT, "DEFAULT");
}
void AudioEngine::RegisterObject(GameObject* obj)
{
	AK::SoundEngine::RegisterGameObj(obj->GetAudioId(), obj->GetName().c_str());
}
void AudioEngine::DeRegisterObject(GameObject* obj)
{
	AK::SoundEngine::UnregisterGameObj(obj->GetAudioId());
}
void AudioEngine::StopAll()
{
	AK::SoundEngine::StopAll();
}
int AudioEngine::GetNextAudioId()
{
	Instance->NextAudioId++;
	return Instance->NextAudioId;
}
void AudioEngine::LoadBank(std::string Name)
{
	const std::string BankLocation = AssetManager::GetContentPath() + "Banks\\" + Name;
	AkBankID bankID; // Not used. These banks can be unloaded with their file name.
	AKRESULT eResult = AK::SoundEngine::LoadBank(BankLocation.c_str(), AK_DEFAULT_POOL_ID, bankID);
	assert(eResult == AK_Success);	
}
