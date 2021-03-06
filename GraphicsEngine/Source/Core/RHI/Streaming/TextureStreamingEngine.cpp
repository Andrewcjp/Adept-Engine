#include "TextureStreamingEngine.h"
#include "Core/Input/Input.h"

TextureStreamingEngine* TextureStreamingEngine::Instance = nullptr;

TextureStreamingEngine::TextureStreamingEngine()
{
	Instance = this;
	m_StreamingMode = EGPUSteamMode::TiledTexture;
	m_StreamingMode = EGPUSteamMode::SamplerFeedBack;
}


TextureStreamingEngine::~TextureStreamingEngine()
{
}

void TextureStreamingEngine::Update()
{
	UpdateSceneTextureStreaming();
}

void TextureStreamingEngine::UpdateSceneTextureStreaming()
{
	//find relevant 
	//and load to memory
	while (NewHandles.size())
	{
		TextureHandle* h = NewHandles.front();
		NewHandles.pop();
		h->LoadToCPUMemory();
		for (int i = 0; i < Streamers.size(); i++)
		{
			Streamers[i]->RealiseHandle(h);
		}
	}

	for (int i = 0; i < Streamers.size(); i++)
	{
		Streamers[i]->Update();
	}
	if (Input::GetKeyDown(KeyCode::I))
	{
		for (int i = 0; i < Handles.size(); i++)
		{
			Handles[i]->SetAllTargetMip(8);
		}
	}
	if (Input::GetKeyDown(KeyCode::O))
	{
		for (int i = 0; i < Handles.size(); i++)
		{
			Handles[i]->SetAllTargetMip(0);
		}
	}
}

void TextureStreamingEngine::RegisterGPUStreamer(GPUTextureStreamer* Stream)
{
	Streamers.push_back(Stream);
}

TextureHandle* TextureStreamingEngine::RequestTexture(std::string File)
{
	for (int i = 0; i < Get()->Handles.size(); i++)
	{
		if (Get()->Handles[i]->GetFilePath() == File)
		{
			return Get()->Handles[i];
		}
	}
	TextureHandle* handle = new TextureHandle();
	handle->InitFromFile(File);
	Get()->Handles.push_back(handle);
	Get()->NewHandles.emplace(handle);
	return handle;
}

void TextureStreamingEngine::ResolveTextureStreaming(RHICommandList * list)
{
	if (m_StreamingMode != EGPUSteamMode::SamplerFeedBack)
	{
		return;
	}
	for (int i = 0; i < Get()->Handles.size(); i++)
	{
		Get()->Handles[i]->ResolveStreamingMaps(list);
	}
}

